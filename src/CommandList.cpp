/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
*                 https://membranesoftware.com
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
#include "Result.h"
#include "Log.h"
#include "Util.h"
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
	return ((lastIdleTime > 0) && ((Util::getTime () - lastIdleTime) >= timeout));
}

void CommandList::addCommand (const StdString &hostname, int tcpPort, Json *command, CommandList::InvokeCallback callback, void *callbackData) {
	App *app;
	CommandList::Context *ctx;
	StdString url, postdata;
	bool shouldinvoke;

	app = App::getInstance ();
	shouldinvoke = false;

	SDL_LockMutex (mutex);
	ctx = new CommandList::Context ();
	ctx->hostname.assign (hostname);
	ctx->tcpPort = tcpPort;
	ctx->command = command;
	ctx->callback = callback;
	ctx->callbackData = callbackData;

	contextList.push_back (ctx);
	if ((! isInvoking) && getNextCommand (&url, &postdata)) {
		shouldinvoke = true;
		isInvoking = true;
	}
	SDL_UnlockMutex (mutex);

	if (shouldinvoke) {
		app->network.sendHttpPost (url, postdata, CommandList::invokeComplete, this);
	}
}

void CommandList::invokeComplete (void *commandListPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	CommandList *cmdlist;
	App *app;
	StdString resp;
	Json *responsecmd;
	int result;

	cmdlist = (CommandList *) commandListPtr;
	app = App::getInstance ();
	if (statusCode == Network::HTTP_UNAUTHORIZED) {
		if (cmdlist->authorizeLastCommand ()) {
			return;
		}
	}

	result = Result::SUCCESS;
	responsecmd = NULL;

	if ((statusCode != Network::HTTP_OK) || (! responseData)) {
		switch (statusCode) {
			case Network::HTTP_UNAUTHORIZED: {
				result = Result::ERROR_UNAUTHORIZED;
				break;
			}
			default: {
				result = Result::ERROR_HTTP_OPERATION_FAILED;
				break;
			}
		}
	}

	if (result == Result::SUCCESS) {
		if (responseData->empty ()) {
			result = Result::ERROR_MALFORMED_RESPONSE;
		}
	}

	if (result == Result::SUCCESS) {
		resp.assignBuffer (responseData);
		if (! app->systemInterface.parseCommand (resp, &responsecmd)) {
			result = Result::ERROR_MALFORMED_RESPONSE;
		}
	}

	cmdlist->endInvoke (result, responsecmd);
	if (responsecmd) {
		delete (responsecmd);
	}
}

void CommandList::endInvoke (int invokeResult, Json *responseCommand) {
	App *app;
	CommandList::Context *ctx;
	StdString agentid, url, postdata;
	bool shouldinvoke;

	app = App::getInstance ();
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
		if (ctx->callback) {
			if (responseCommand) {
				agentid = app->systemInterface.getCommandAgentId (responseCommand);
			}

			ctx->callback (ctx->callbackData, invokeResult, ctx->hostname, ctx->tcpPort, agentid, ctx->command, responseCommand);
		}
		freeContext (ctx);
	}

	SDL_LockMutex (mutex);
	if (! getNextCommand (&url, &postdata)) {
		isInvoking = false;
		lastIdleTime = Util::getTime ();
	}
	else {
		shouldinvoke = true;
		isInvoking = true;
	}
	SDL_UnlockMutex (mutex);

	if (shouldinvoke) {
		app->network.sendHttpPost (url, postdata, CommandList::invokeComplete, this);
	}
	else {
		if (! isInvoking) {
			clearContextList ();
		}
	}
}

bool CommandList::getNextCommand (StdString *url, StdString *postData) {
	App *app;
	CommandList::Context *ctx;
	std::map<StdString, CommandList::Token>::iterator pos;
	Json *cmd;

	if (contextList.empty ()) {
		return (false);
	}

	app = App::getInstance ();
	ctx = contextList.front ();
	if (ctx->hostname.empty () || (ctx->tcpPort <= 0)) {
		return (false);
	}

	url->assign (app->agentControl.getHostInvokeUrl (ctx->hostname, ctx->tcpPort));
	pos = tokenMap.find (app->agentControl.getMapKey (ctx->hostname, ctx->tcpPort));
	if (pos == tokenMap.end ()) {
		postData->assign (ctx->command->toString ());
		return (true);
	}

	cmd = new Json ();
	cmd->copy (ctx->command);
	if (! app->agentControl.setCommandAuthorization (cmd, pos->second.authorizeSecretIndex, pos->second.token)) {
		Log::write (Log::DEBUG, "Failed to generate command authorization; err=index %i not found", ctx->authorizeSecretIndex);
		postData->assign (ctx->command->toString ());
	}
	else {
		postData->assign (cmd->toString ());
	}
	delete (cmd);

	return (true);
}

bool CommandList::authorizeLastCommand () {
	App *app;
	CommandList::Context *ctx;
	Json *cmd, *params;
	StdString path, url, postdata;
	bool result;

	app = App::getInstance ();
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
			params->set ("token", app->getRandomString (64));
			cmd = app->createCommand ("Authorize", SystemInterface::Constant_DefaultCommandType, params);
			if (cmd) {
				if (app->agentControl.setCommandAuthorization (cmd, ctx->authorizeSecretIndex, StdString (""), &path)) {
					result = true;
					ctx->isAuthorizing = true;
					url = app->agentControl.getHostInvokeUrl (ctx->hostname, ctx->tcpPort, path);
					postdata = cmd->toString ();
					tokenMap.erase (app->agentControl.getMapKey (ctx->hostname, ctx->tcpPort));
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
		app->network.sendHttpPost (url, postdata, CommandList::authorizeComplete, this);
	}
	return (result);
}

void CommandList::authorizeComplete (void *commandListPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	CommandList *cmdlist;
	App *app;
	SystemInterface *interface;
	StdString resp;
	Json *responsecmd;
	int result;

	cmdlist = (CommandList *) commandListPtr;
	app = App::getInstance ();
	interface = &(app->systemInterface);

	result = Result::SUCCESS;
	responsecmd = NULL;

	if ((statusCode == Network::HTTP_OK) && (! responseData->empty ())) {
		resp.assignBuffer (responseData);
		if (! interface->parseCommand (resp, &responsecmd)) {
			result = Result::ERROR_MALFORMED_RESPONSE;
		}
	}

	if (result == Result::SUCCESS) {
		if (! responsecmd) {
			result = Result::ERROR_HTTP_OPERATION_FAILED;
		}
	}

	cmdlist->endAuthorize (result, responsecmd);
	if (responsecmd) {
		delete (responsecmd);
	}
}

void CommandList::endAuthorize (int invokeResult, Json *responseCommand) {
	App *app;
	SystemInterface *interface;
	CommandList::Context *ctx;
	CommandList::Token item;
	StdString token, url, postdata;
	bool shouldinvoke, shouldcallback;

	app = App::getInstance ();
	interface = &(app->systemInterface);

	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_AuthorizeResult)) {
		token = interface->getCommandStringParam (responseCommand, "token", "");
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
					lastIdleTime = Util::getTime ();
				}
			}
		}
		else {
			ctx->isAuthorizeComplete = true;
			item.token.assign (token);
			item.authorizeSecretIndex = ctx->authorizeSecretIndex;
			tokenMap.insert (std::pair<StdString, CommandList::Token> (app->agentControl.getMapKey (ctx->hostname, ctx->tcpPort), item));
			if (getNextCommand (&url, &postdata)) {
				shouldinvoke = true;
			}
		}
	}
	SDL_UnlockMutex (mutex);

	if (shouldinvoke) {
		app->network.sendHttpPost (url, postdata, CommandList::invokeComplete, this);
	}
	if (ctx && shouldcallback) {
		lastInvokeResult = Result::ERROR_UNAUTHORIZED;
		if (ctx->callback) {
			ctx->callback (ctx->callbackData, Result::ERROR_UNAUTHORIZED, ctx->hostname, ctx->tcpPort, StdString (""), ctx->command, NULL);
		}
		freeContext (ctx);
	}
}
