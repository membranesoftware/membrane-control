/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "Config.h"
#include <stdlib.h>
#include <list>
#include <map>
#include "SDL2/SDL.h"
#include "App.h"
#include "Log.h"
#include "OsUtil.h"
#include "StdString.h"
#include "Json.h"
#include "Network.h"
#include "SystemInterface.h"
#include "CommandList.h"

CommandList::CommandList (const StdString &name)
: name (name)
, lastInvokeResult (0)
, isInvoking (false)
, lastIdleTime (0)
, mutex (NULL)
{
	mutex = SDL_CreateMutex ();
}

CommandList::~CommandList () {
	clearContextList ();
	if (mutex) {
		SDL_DestroyMutex (mutex);
		mutex = NULL;
	}
}

void CommandList::clearContextList () {
	std::list<CommandList::Context *>::iterator i, end;

	SDL_LockMutex (mutex);
	i = contextList.begin ();
	end = contextList.end ();
	while (i != end) {
		freeContext (*i);
		++i;
	}
	contextList.clear ();
	SDL_UnlockMutex (mutex);
}

void CommandList::freeContext (CommandList::Context *ctx) {
	if (ctx->command) {
		delete (ctx->command);
		ctx->command = NULL;
	}
	delete (ctx);
}

bool CommandList::empty () {
	bool result;

	SDL_LockMutex (mutex);
	if ((! isInvoking) && contextList.empty ()) {
		result = true;
	}
	else {
		result = false;
	}
	SDL_UnlockMutex (mutex);

	return (result);
}

bool CommandList::isIdle (int timeout) {
	return ((lastIdleTime > 0) && ((OsUtil::getTime () - lastIdleTime) >= timeout));
}

void CommandList::addCommand (const StdString &hostname, int tcpPort, Json *command, CommandList::InvokeCallbackContext callback) {
	CommandList::Context *ctx;
	StdString url, postdata;
	bool shouldinvoke;

	shouldinvoke = false;

	SDL_LockMutex (mutex);
	ctx = new CommandList::Context ();
	ctx->hostname.assign (hostname);
	ctx->tcpPort = tcpPort;
	ctx->command = command;
	ctx->callback = callback;

	contextList.push_back (ctx);
	if ((! isInvoking) && getNextCommand (&url, &postdata)) {
		shouldinvoke = true;
		isInvoking = true;
	}
	SDL_UnlockMutex (mutex);

	if (shouldinvoke) {
		Network::instance->sendHttpPost (url, postdata, Network::HttpRequestCallbackContext (CommandList::invokeComplete, this), AgentControl::instance->agentServerName);
	}
}

void CommandList::invokeComplete (void *commandListPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	CommandList *cmdlist;
	StdString resp;
	Json *responsecmd;
	int result;

	cmdlist = (CommandList *) commandListPtr;
	if (statusCode == Network::HttpUnauthorizedCode) {
		if (cmdlist->authorizeLastCommand ()) {
			return;
		}
	}

	result = OsUtil::Success;
	responsecmd = NULL;

	if ((statusCode != Network::HttpOkCode) || (! responseData)) {
		switch (statusCode) {
			case Network::HttpUnauthorizedCode: {
				result = OsUtil::UnauthorizedError;
				break;
			}
			default: {
				result = OsUtil::HttpOperationFailedError;
				break;
			}
		}
	}

	if (result == OsUtil::Success) {
		if (responseData->empty ()) {
			result = OsUtil::MalformedResponseError;
		}
	}

	if (result == OsUtil::Success) {
		resp.assignBuffer (responseData);
		if (! SystemInterface::instance->parseCommand (resp, &responsecmd)) {
			result = OsUtil::MalformedResponseError;
		}
	}

	cmdlist->endInvoke (result, responsecmd);
	if (responsecmd) {
		delete (responsecmd);
	}
}

void CommandList::endInvoke (int invokeResult, Json *responseCommand) {
	CommandList::Context *ctx;
	StdString agentid, url, postdata;
	bool shouldinvoke;

	ctx = NULL;
	shouldinvoke = false;

	SDL_LockMutex (mutex);
	if (! contextList.empty ()) {
		ctx = contextList.front ();
		contextList.pop_front ();
	}
	SDL_UnlockMutex (mutex);

	if (ctx) {
		lastInvokeResult = invokeResult;
		if (ctx->callback.callback) {
			if (responseCommand) {
				agentid = SystemInterface::instance->getCommandAgentId (responseCommand);
			}
			if (agentid.empty ()) {
				agentid.assign (ctx->callback.agentId);
			}
			ctx->callback.callback (ctx->callback.callbackData, invokeResult, ctx->hostname, ctx->tcpPort, agentid, ctx->command, responseCommand, ctx->callback.invokeId);
		}
		freeContext (ctx);
	}

	SDL_LockMutex (mutex);
	if (! getNextCommand (&url, &postdata)) {
		isInvoking = false;
		lastIdleTime = OsUtil::getTime ();
	}
	else {
		shouldinvoke = true;
		isInvoking = true;
	}
	SDL_UnlockMutex (mutex);

	if (shouldinvoke) {
		Network::instance->sendHttpPost (url, postdata, Network::HttpRequestCallbackContext (CommandList::invokeComplete, this), AgentControl::instance->agentServerName);
	}
	else {
		if (! isInvoking) {
			clearContextList ();
		}
	}
}

bool CommandList::getNextCommand (StdString *url, StdString *postData) {
	CommandList::Context *ctx;
	std::map<StdString, CommandList::Token>::iterator pos;
	Json *cmd;

	if (contextList.empty ()) {
		return (false);
	}

	ctx = contextList.front ();
	if (ctx->hostname.empty () || (ctx->tcpPort <= 0)) {
		return (false);
	}

	url->assign (AgentControl::instance->getHostInvokeUrl (ctx->hostname, ctx->tcpPort));
	pos = tokenMap.find (AgentControl::instance->getMapKey (ctx->hostname, ctx->tcpPort));
	if (pos == tokenMap.end ()) {
		postData->assign (ctx->command->toString ());
		return (true);
	}

	cmd = new Json ();
	cmd->copyValue (ctx->command);
	if (! AgentControl::instance->setCommandAuthorization (cmd, pos->second.authorizeSecretIndex, pos->second.token)) {
		Log::debug ("Failed to generate command authorization; err=index %i not found", ctx->authorizeSecretIndex);
		postData->assign (ctx->command->toString ());
	}
	else {
		postData->assign (cmd->toString ());
	}
	delete (cmd);

	return (true);
}

bool CommandList::authorizeLastCommand () {
	CommandList::Context *ctx;
	Json *cmd, *params;
	StdString path, url, postdata;
	bool result;

	result = false;
	SDL_LockMutex (mutex);
	if (! contextList.empty ()) {
		ctx = contextList.front ();
		if (! ctx->isAuthorizing) {
			if (! ctx->isAuthorizeComplete) {
				ctx->isAuthorizing = true;
				ctx->authorizeSecretIndex = 0;
			}
		}
		else {
			++(ctx->authorizeSecretIndex);
		}

		if (ctx->isAuthorizing) {
			params = new Json ();
			params->set ("token", App::instance->getRandomString (64));
			cmd = App::instance->createCommand (SystemInterface::Command_Authorize, params);
			if (cmd) {
				if (AgentControl::instance->setCommandAuthorization (cmd, ctx->authorizeSecretIndex, StdString (""), &path)) {
					result = true;
					ctx->isAuthorizing = true;
					url = AgentControl::instance->getHostInvokeUrl (ctx->hostname, ctx->tcpPort, path);
					postdata = cmd->toString ();
					tokenMap.erase (AgentControl::instance->getMapKey (ctx->hostname, ctx->tcpPort));
				}
				delete (cmd);
			}

			if (! result) {
				ctx->isAuthorizing = false;
				ctx->isAuthorizeComplete = true;
			}
		}
	}
	SDL_UnlockMutex (mutex);

	if (result) {
		Network::instance->sendHttpPost (url, postdata, Network::HttpRequestCallbackContext (CommandList::authorizeComplete, this), AgentControl::instance->agentServerName);
	}
	return (result);
}

void CommandList::authorizeComplete (void *commandListPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	CommandList *cmdlist;
	StdString resp;
	Json *responsecmd;
	int result;

	cmdlist = (CommandList *) commandListPtr;
	result = OsUtil::Success;
	responsecmd = NULL;

	if ((statusCode == Network::HttpOkCode) && responseData && (! responseData->empty ())) {
		resp.assignBuffer (responseData);
		if (! SystemInterface::instance->parseCommand (resp, &responsecmd)) {
			result = OsUtil::MalformedResponseError;
		}
	}

	if (result == OsUtil::Success) {
		if (! responsecmd) {
			result = OsUtil::HttpOperationFailedError;
		}
	}

	cmdlist->endAuthorize (result, responsecmd);
	if (responsecmd) {
		delete (responsecmd);
	}
}

void CommandList::endAuthorize (int invokeResult, Json *responseCommand) {
	CommandList::Context *ctx;
	CommandList::Token item;
	StdString token, url, postdata;
	bool shouldinvoke, shouldcallback;

	if (responseCommand && (SystemInterface::instance->getCommandId (responseCommand) == SystemInterface::CommandId_AuthorizeResult)) {
		token = SystemInterface::instance->getCommandStringParam (responseCommand, "token", "");
	}

	ctx = NULL;
	shouldinvoke = false;
	shouldcallback = false;
	SDL_LockMutex (mutex);
	if (! contextList.empty ()) {
		ctx = contextList.front ();
		if (token.empty ()) {
			if (! authorizeLastCommand ()) {
				shouldcallback = true;
				contextList.pop_front ();
				if (getNextCommand (&url, &postdata)) {
					shouldinvoke = true;
				}
				else {
					isInvoking = false;
					lastIdleTime = OsUtil::getTime ();
				}
			}
		}
		else {
			ctx->isAuthorizeComplete = true;
			item.token.assign (token);
			item.authorizeSecretIndex = ctx->authorizeSecretIndex;
			tokenMap.insert (std::pair<StdString, CommandList::Token> (AgentControl::instance->getMapKey (ctx->hostname, ctx->tcpPort), item));
			if (getNextCommand (&url, &postdata)) {
				shouldinvoke = true;
			}
		}
	}
	SDL_UnlockMutex (mutex);

	if (ctx && (! token.empty ())) {
		AgentControl::instance->setHostAuthorization (ctx->hostname, ctx->tcpPort, ctx->authorizeSecretIndex, token);
	}
	if (shouldinvoke) {
		Network::instance->sendHttpPost (url, postdata, Network::HttpRequestCallbackContext (CommandList::invokeComplete, this), AgentControl::instance->agentServerName);
	}
	if (ctx && shouldcallback) {
		lastInvokeResult = OsUtil::UnauthorizedError;
		if (ctx->callback.callback) {
			ctx->callback.callback (ctx->callback.callbackData, OsUtil::UnauthorizedError, ctx->hostname, ctx->tcpPort, ctx->callback.agentId, ctx->command, NULL, ctx->callback.invokeId);
		}
		freeContext (ctx);
	}
}
