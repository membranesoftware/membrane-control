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
#include <string.h>
#include <map>
#include <list>
#include <queue>
#include "SDL2/SDL.h"
#include "libwebsockets.h"
#include "App.h"
#include "StdString.h"
#include "StringList.h"
#include "Log.h"
#include "OsUtil.h"
#include "Json.h"
#include "SystemInterface.h"
#include "AgentControl.h"
#include "LinkClient.h"

const int LinkClient::DefaultPingInterval = 25000; // milliseconds
const int LinkClient::ReconnectPeriod = 7000; // milliseconds
const int LinkClient::MaxCommandSize = (256 * 1024); // bytes

static const struct lws_extension LibwebsocketExts[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_no_context_takeover"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL }
};

static const struct lws_protocols LibwebsocketProtocols[] = {
	{
		"ws",
		LinkClient::protocolCallback,
		0,
		8192
	},
	{ NULL, NULL, 0, 0 }
};

LinkClient::LinkClient (void *callbackData, LinkContext::ConnectCallback connectCallback, LinkContext::DisconnectCallback disconnectCallback, LinkContext::CommandCallback commandCallback)
: callbackData (callbackData)
, connectCallback (connectCallback)
, disconnectCallback (disconnectCallback)
, commandCallback (commandCallback)
, contextMutex (NULL)
{
	contextMutex = SDL_CreateMutex ();
}

LinkClient::~LinkClient () {
	clear ();
	if (contextMutex) {
		SDL_DestroyMutex (contextMutex);
		contextMutex = NULL;
	}
}

LinkContext::LinkContext ()
: usageCount (0)
, isConnected (false)
, isClosing (false)
, isClosed (false)
, reconnectClock (0)
, context (NULL)
, lws (NULL)
, urlParseBuffer (NULL)
, isWriteReady (false)
, writeQueueMutex (NULL)
, writeBuffer (NULL)
, writeBufferSize (0)
, pingInterval (0)
, nextPingTime (0)
, authorizeSecretIndex (0)
, isAuthorizing (false)
, isAuthorizeComplete (false)
, callbackData (NULL)
, connectCallback (NULL)
, disconnectCallback (NULL)
, commandCallback (NULL)
{
	writeQueueMutex = SDL_CreateMutex ();
}

LinkContext::~LinkContext () {
	if (context) {
		lws_context_destroy (context);
		context = NULL;
	}
	lws = NULL;
	if (urlParseBuffer) {
		free (urlParseBuffer);
		urlParseBuffer = NULL;
	}
	if (writeQueueMutex) {
		SDL_DestroyMutex (writeQueueMutex);
		writeQueueMutex = NULL;
	}
	if (writeBuffer) {
		free (writeBuffer);
		writeBuffer = NULL;
	}
}

void LinkClient::start () {
	int level;

	lws_set_log_level (level, LinkClient::logCallback);
}

void LinkClient::logCallback (int level, const char *line) {
}

void LinkClient::clear () {
	std::map<StdString, LinkContext *>::iterator i, iend;
	std::list<LinkContext *>::iterator j, jend;

	SDL_LockMutex (contextMutex);
	i = contextMap.begin ();
	iend = contextMap.end ();
	while (i != iend) {
		delete (i->second);
		++i;
	}
	contextMap.clear ();

	j = closeContextList.begin ();
	jend = closeContextList.end ();
	while (j != jend) {
		delete (*j);
		++j;
	}
	closeContextList.clear ();

	SDL_UnlockMutex (contextMutex);
}

void LinkClient::update (int msElapsed) {
	StringList idlist;
	LinkContext *ctx;
	std::map<StdString, LinkContext *>::iterator i, iend;
	std::list<LinkContext *>::iterator j, jend;
	StringList::iterator k, kend;
	bool found;

	SDL_LockMutex (contextMutex);
	i = contextMap.begin ();
	iend = contextMap.end ();
	while (i != iend) {
		ctx = i->second;
		updateActiveContext (ctx, msElapsed);
		if (ctx->isClosing || ctx->isClosed) {
			idlist.push_back (i->first);
		}
		else {
			if (ctx->isConnected) {
				if (! ctx->disconnectCallback) {
					ctx->disconnectCallback = disconnectCallback;
				}
			}
		}
		++i;
	}

	k = idlist.begin ();
	kend = idlist.end ();
	while (k != kend) {
		i = contextMap.find (*k);
		if (i != contextMap.end ()) {
			ctx = i->second;
			closeContextList.push_back (ctx);

			if (ctx->usageCount <= 0) {
				contextMap.erase (i);
			}
			else {
				i->second = new LinkContext ();
				i->second->agentId.assign (ctx->agentId);
				i->second->usageCount = ctx->usageCount;
				i->second->linkUrl.assign (ctx->linkUrl);
				i->second->serverName.assign (ctx->serverName);
				i->second->callbackData = callbackData;
				i->second->connectCallback = connectCallback;
				i->second->commandCallback = commandCallback;
				i->second->reconnectClock = LinkClient::ReconnectPeriod;
			}

			if (ctx->disconnectCallback) {
				// TODO: Possibly set an error description string here
				ctx->disconnectCallback (ctx->callbackData, ctx->agentId, StdString (""));
				ctx->disconnectCallback = NULL;
			}
		}
		++k;
	}

	while (true) {
		found = false;
		j = closeContextList.begin ();
		jend = closeContextList.end ();
		while (j != jend) {
			ctx = *j;
			if ((! ctx->context) || (! ctx->lws) || ctx->isClosed) {
				found = true;
				closeContextList.erase (j);
				delete (ctx);
				break;
			}
			++j;
		}

		if (! found) {
			break;
		}
	}

	j = closeContextList.begin ();
	jend = closeContextList.end ();
	while (j != jend) {
		ctx = *j;
		ctx->isClosing = true;
		lws_callback_on_writable (ctx->lws);
		lws_service (ctx->context, 0);
		++j;
	}
	SDL_UnlockMutex (contextMutex);
}

void LinkClient::updateActiveContext (LinkContext *ctx, int msElapsed) {
	struct lws_context_creation_info contextinfo;
	lws_client_connect_info clientinfo;
	int result, sz, port;
	int64_t now;
	const char *protocol, *address, *path;

	if (ctx->isClosing) {
		return;
	}

	if (ctx->reconnectClock > 0) {
		ctx->reconnectClock -= msElapsed;
		if (ctx->reconnectClock > 0) {
			return;
		}
	}

	if (! ctx->context) {
		memset (&contextinfo, 0, sizeof contextinfo);
		contextinfo.port = CONTEXT_PORT_NO_LISTEN;
		contextinfo.protocols = LibwebsocketProtocols;
		contextinfo.gid = -1;
		contextinfo.uid = -1;
		contextinfo.ws_ping_pong_interval = 0;
		if (! ctx->serverName.empty ()) {
			contextinfo.vhost_name = ctx->serverName.c_str ();
		}
		contextinfo.extensions = LibwebsocketExts;
		contextinfo.user = ctx;
		if (App::instance->isHttpsEnabled) {
			contextinfo.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		}

		ctx->context = lws_create_context (&contextinfo);
		if (! ctx->context) {
			ctx->isClosing = true;
			Log::err ("Failed to create libwebsockets context; url=\"%s\"", ctx->linkUrl.c_str ());
			return;
		}
	}

	if (! ctx->lws) {
		sz = (int) ctx->linkUrl.length () + 1;
		ctx->urlParseBuffer = (char *) realloc (ctx->urlParseBuffer, sz);
		if (! ctx->urlParseBuffer) {
			ctx->isClosing = true;
			Log::err ("Failed to allocate memory for link url parse buffer; length=%i", sz);
			return;
		}

		memset (ctx->urlParseBuffer, 0, sz);
		memcpy (ctx->urlParseBuffer, ctx->linkUrl.c_str (), ctx->linkUrl.length ());
		result = lws_parse_uri (ctx->urlParseBuffer, &protocol, &address, &port, &path);
		if (result != 0) {
			ctx->isClosing = true;
			Log::err ("Failed to parse WebSocket URL; url=\"%s\" err=%i", ctx->linkUrl.c_str (), result);
			return;
		}

		ctx->urlPath.assign ("");
		if (path[0] != '/') {
			ctx->urlPath.append ("/");
		}
		ctx->urlPath.append (path);
		memset (&clientinfo, 0, sizeof (clientinfo));
		clientinfo.context = ctx->context;
		clientinfo.address = address;
		clientinfo.port = port;
		clientinfo.path = ctx->urlPath.c_str ();
		clientinfo.host = clientinfo.address;
		clientinfo.origin = clientinfo.address;
		clientinfo.ietf_version_or_minus_one = -1;

		if (App::instance->isHttpsEnabled) {
			// TODO: Possibly enable certificate validation (currently disabled to allow use of self-signed certificates)
			clientinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK | LCCSCF_ALLOW_EXPIRED;
		}
		else {
			clientinfo.ssl_connection = 0;
		}

		ctx->lws = lws_client_connect_via_info (&clientinfo);
		if (! ctx->lws) {
			ctx->isClosing = true;
			Log::err ("Failed to establish WebSocket connection; url=\"%s\" err=\"lws_client_connect_via_info failed\"", ctx->linkUrl.c_str ());
			return;
		}
	}

	if ((! ctx->isWriteReady) && (! ctx->writeQueue.empty ())) {
		ctx->isWriteReady = true;
		lws_callback_on_writable (ctx->lws);
	}

	if ((ctx->pingInterval > 0) && (ctx->nextPingTime > 0)) {
		now = OsUtil::getTime ();
		if (ctx->nextPingTime <= now) {
			ctx->writeMessage (StdString ("2"));
			ctx->nextPingTime = 0;
		}
	}

	lws_service (ctx->context, 0);
}

void LinkClient::connect (const StdString &agentId, const StdString &linkUrl) {
	std::map<StdString, LinkContext *>::iterator pos;
	LinkContext *ctx;

	if (agentId.empty () || linkUrl.empty ()) {
		return;
	}
	SDL_LockMutex (contextMutex);
	pos = contextMap.find (agentId);
	if (pos != contextMap.end ()) {
		ctx = pos->second;
		if (! ctx->linkUrl.equals (linkUrl)) {
			// TODO: Restart the context targeting the new URL
		}
		++(ctx->usageCount);
	}
	else {
		ctx = new LinkContext ();
		ctx->agentId.assign (agentId);
		ctx->usageCount = 1;
		ctx->linkUrl.assign (linkUrl);
		ctx->serverName.assign (AgentControl::instance->agentServerName);
		ctx->callbackData = callbackData;
		ctx->connectCallback = connectCallback;
		ctx->commandCallback = commandCallback;
		contextMap.insert (std::pair<StdString, LinkContext *> (agentId, ctx));
	}
	SDL_UnlockMutex (contextMutex);
}

void LinkClient::disconnect (const StdString &agentId) {
	std::map<StdString, LinkContext *>::iterator pos;
	LinkContext *ctx;

	SDL_LockMutex (contextMutex);
	pos = contextMap.find (agentId);
	if (pos != contextMap.end ()) {
		ctx = pos->second;
		--(ctx->usageCount);
		if (ctx->usageCount <= 0) {
			ctx->isClosing = true;
			closeContextList.push_back (ctx);
			contextMap.erase (pos);
			if (ctx->disconnectCallback) {
				// TODO: Possibly set an error description string here
				ctx->disconnectCallback (ctx->callbackData, ctx->agentId, StdString (""));
				ctx->disconnectCallback = NULL;
			}
		}
	}
	SDL_UnlockMutex (contextMutex);
}

bool LinkClient::isConnected (const StdString &agentId) {
	std::map<StdString, LinkContext *>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (contextMutex);
	pos = contextMap.find (agentId);
	if (pos != contextMap.end ()) {
		result = pos->second->isConnected;
	}
	SDL_UnlockMutex (contextMutex);

	return (result);
}

void LinkClient::setLinkUrl (const StdString &agentId, const StdString &linkUrl) {
	std::map<StdString, LinkContext *>::iterator pos;
	LinkContext *ctx;

	SDL_LockMutex (contextMutex);
	pos = contextMap.find (agentId);
	if (pos != contextMap.end ()) {
		ctx = pos->second;
		if (! ctx->linkUrl.equals (linkUrl)) {
			ctx->isClosing = true;
			closeContextList.push_back (ctx);
			if (ctx->disconnectCallback) {
				// TODO: Possibly set an error description string here
				ctx->disconnectCallback (ctx->callbackData, ctx->agentId, StdString (""));
				ctx->disconnectCallback = NULL;
			}

			pos->second = new LinkContext ();
			pos->second->agentId.assign (ctx->agentId);
			pos->second->usageCount = ctx->usageCount;
			pos->second->linkUrl.assign (linkUrl);
			pos->second->serverName.assign (ctx->serverName);
			pos->second->callbackData = callbackData;
			pos->second->connectCallback = connectCallback;
			pos->second->commandCallback = commandCallback;
		}
	}
	SDL_UnlockMutex (contextMutex);
}

void LinkClient::writeCommand (Json *command, const StdString &agentId) {
	std::map<StdString, LinkContext *>::iterator i, end;
	LinkContext *ctx;

	if (! command) {
		return;
	}
	SDL_LockMutex (contextMutex);
	end = contextMap.end ();
	if (agentId.empty ()) {
		i = contextMap.begin ();
		while (i != end) {
			ctx = i->second;
			if (ctx->isConnected) {
				ctx->writeCommand (command);
			}
			++i;
		}
	}
	else {
		i = contextMap.find (agentId);
		if (i != end) {
			ctx = i->second;
			if (ctx->isConnected) {
				ctx->writeCommand (command);
			}
		}
	}
	SDL_UnlockMutex (contextMutex);

	delete (command);
}

int LinkClient::protocolCallback (struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
	LinkContext *ctx;
	int result;

	ctx = (LinkContext *) lws_context_user (lws_get_context (wsi));
	if (! ctx) {
		return (-1);
	}

	result = 0;
	switch (reason) {
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
			Log::debug ("WebSocket connection error; url=\"%s\" err=\"%s\"", ctx->linkUrl.c_str (), in ? StdString ((char *) in, len).c_str () : "unspecified connection failure");
			break;
		}
		case LWS_CALLBACK_CLIENT_ESTABLISHED: {
			break;
		}
		case LWS_CALLBACK_CLOSED: {
			ctx->isConnected = false;
			ctx->isClosing = true;
			break;
		}
		case LWS_CALLBACK_CLIENT_WRITEABLE: {
			if (ctx->isClosing) {
				result = -1;
			}
			else {
				ctx->sendNextMessage ();
			}
			break;
		}
		case LWS_CALLBACK_RECEIVE: {
			ctx->receiveData ((char *) in, len);
			break;
		}
		case LWS_CALLBACK_CLIENT_RECEIVE: {
			ctx->receiveData ((char *) in, len);
			break;
		}
		case LWS_CALLBACK_CLIENT_CLOSED: {
			break;
		}
		case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: {
			ctx->isConnected = false;
			break;
		}
		case LWS_CALLBACK_PROTOCOL_INIT: {
			break;
		}
		case LWS_CALLBACK_PROTOCOL_DESTROY: {
			break;
		}
		case LWS_CALLBACK_WSI_CREATE: {
			break;
		}
		case LWS_CALLBACK_WSI_DESTROY: {
			ctx->isClosed = true;
			break;
		}
		case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED: {
			break;
		}
		case LWS_CALLBACK_WS_EXT_DEFAULTS: {
			break;
		}
		case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: {
			break;
		}
		case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: {
			break;
		}
		case LWS_CALLBACK_LOCK_POLL: {
			break;
		}
		case LWS_CALLBACK_UNLOCK_POLL: {
			break;
		}
		case LWS_CALLBACK_ADD_POLL_FD: {
			break;
		}
		case LWS_CALLBACK_CHANGE_MODE_POLL_FD: {
			break;
		}
		case LWS_CALLBACK_DEL_POLL_FD: {
			break;
		}
		case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION: {
			break;
		}
		default: {
			break;
		}
	}

	return (result);
}

void LinkContext::receiveData (char *data, int dataLength) {
	Json *json;
	char *d, packettype;
	int len;

	d = data;
	len = dataLength;
	if ((! d) || (len <= 0)) {
		return;
	}

	if (! commandBuffer.empty ()) {
		processCommandMessage (d, len);
		return;
	}

	packettype = *d;
	++d;
	--len;
	switch (packettype) {
		case '0': { // open
			if (len <= 0) {
				break;
			}
			json = new Json ();
			if (! json->parse (d, len)) {
				Log::err ("WebSocket connection error; url=\"%s\" err=\"Failed to parse open packet\"", linkUrl.c_str ());
			}
			else {
				sessionId = json->getString ("sid", "");
				pingInterval = json->getNumber ("pingInterval", LinkClient::DefaultPingInterval);
				if (pingInterval <= 0) {
					nextPingTime = 0;
				}
				else {
					nextPingTime = OsUtil::getTime () + pingInterval;
				}
			}
			delete (json);
			break;
		}
		case '1': { // close
			isClosing = true;
			break;
		}
		case '3': { // pong
			if (pingInterval > 0) {
				nextPingTime = OsUtil::getTime () + pingInterval;
			}
			break;
		}
		case '4': { // message
			if (len <= 0) {
				break;
			}
			if (*d != '2') {
				break;
			}
			++d;
			--len;
			if (len <= 0) {
				break;
			}

			processCommandMessage (d, len);
			break;
		}
		default: {
			break;
		}
	}
}

void LinkContext::writeMessage (const StdString &message) {
	if (isClosing || isClosed) {
		return;
	}
	SDL_LockMutex (writeQueueMutex);
	writeQueue.push (message);
	SDL_UnlockMutex (writeQueueMutex);
}

void LinkContext::writeCommand (Json *sourceCommand) {
	Json *cmd;

	if (authorizeToken.empty ()) {
		writeCommand (sourceCommand->toString ());
	}
	else {
		cmd = new Json ();
		cmd->copyValue (sourceCommand);
		if (! AgentControl::instance->setCommandAuthorization (cmd, authorizeSecretIndex, authorizeToken)) {
			authorizeToken.assign ("");
			writeCommand (sourceCommand->toString ());
		}
		else {
			writeCommand (cmd->toString ());
		}
		delete (cmd);
	}
}

void LinkContext::writeCommand (const StdString &commandJson) {
	// This output string specifies packet types 4 (message) and 2 (event) for the engine.io / socket.io protocols
	writeMessage (StdString::createSprintf ("42[\"%s\",%s]", SystemInterface::Constant_WebSocketEvent, commandJson.c_str ()));
}

void LinkContext::sendNextMessage () {
	StdString message;
	bool found, shouldrequestcallback;
	int buflen, result;

	found = false;
	shouldrequestcallback = false;
	SDL_LockMutex (writeQueueMutex);
	if (! writeQueue.empty ()) {
		found = true;
		message = writeQueue.front ();
		writeQueue.pop ();
		if (! writeQueue.empty ()) {
			shouldrequestcallback = true;
		}
	}
	SDL_UnlockMutex (writeQueueMutex);

	if (found) {
		buflen = message.length () + LWS_PRE;
		if (buflen > writeBufferSize) {
			writeBuffer = (unsigned char *) realloc (writeBuffer, buflen);
			if (! writeBuffer) {
				isWriteReady = false;
				Log::err ("Failed to allocate memory for WebSocket write buffer; length=%i", buflen);
				return;
			}
		}

		memcpy (writeBuffer + LWS_PRE, message.c_str (), message.length ());
		result = lws_write (lws, writeBuffer + LWS_PRE, message.length (), LWS_WRITE_TEXT);
		if (result < 0) {
			Log::err ("Failed to write WebSocket message; err=%i", result);
		}
	}
	if (shouldrequestcallback) {
		isWriteReady = true;
		lws_callback_on_writable (lws);
	}
	else {
		isWriteReady = false;
	}
}

void LinkContext::processCommandMessage (char *data, int dataLength) {
	StdString s, prefix;
	size_t pos1, pos2;
	Json *cmd;
	int commandid;

	if ((! data) || (dataLength <= 0)) {
		return;
	}

	if (! commandBuffer.empty ()) {
		commandBuffer.add ((uint8_t *) data, dataLength);
		s.assignBuffer (&commandBuffer);
	}
	else {
		s.assign (data, dataLength);
	}
	prefix.sprintf ("[\"%s\",{", SystemInterface::Constant_WebSocketEvent);
	pos1 = s.find (prefix);
	if (pos1 != 0) {
		commandBuffer.reset ();
		return;
	}
	pos1 += prefix.length () - 1;

	pos2 = s.find_last_of ("}]");
	if (pos2 == StdString::npos) {
		if (commandBuffer.empty ()) {
			commandBuffer.add ((uint8_t *) data, dataLength);
		}
		if (commandBuffer.length > LinkClient::MaxCommandSize) {
			commandBuffer.reset ();
		}
		return;
	}
	--pos2;

	s.assign (s.substr (pos1, pos2 - pos1 + 1));
	if (! SystemInterface::instance->parseCommand (s, &cmd)) {
		if (commandBuffer.empty ()) {
			commandBuffer.add ((uint8_t *) data, dataLength);
		}
		if (commandBuffer.length > LinkClient::MaxCommandSize) {
			commandBuffer.reset ();
		}
		return;
	}
	commandBuffer.reset ();

	commandid = SystemInterface::instance->getCommandId (cmd);
	switch (commandid) {
		case SystemInterface::CommandId_AuthorizationRequired: {
			authorize ();
			break;
		}
		case SystemInterface::CommandId_AuthorizeResult: {
			authorizeToken = SystemInterface::instance->getCommandStringParam (cmd, "token", "");
			break;
		}
		case SystemInterface::CommandId_LinkSuccess: {
			if (! isConnected) {
				isConnected = true;
				if (connectCallback) {
					connectCallback (callbackData, agentId);
				}
			}
			break;
		}
		default: {
			if (commandCallback) {
				commandCallback (callbackData, agentId, cmd);
			}
			break;
		}
	}
	delete (cmd);
}

void LinkContext::authorize () {
	Json *cmd, *params;
	bool sent;

	if (! isAuthorizing) {
		if (! isAuthorizeComplete) {
			authorizeSecretIndex = 0;
			isAuthorizing = true;
		}
	}
	else {
		++authorizeSecretIndex;
	}
	if (! isAuthorizing) {
		// TODO: Possibly disconnect the client here (expecting to retry authorization on the next scheduled reconnect)
		return;
	}

	sent = false;
	authorizeToken.assign ("");
	params = new Json ();
	params->set ("token", App::instance->getRandomString (64));
	cmd = App::instance->createCommand (SystemInterface::Command_Authorize, params);
	if (cmd) {
		if (AgentControl::instance->setCommandAuthorization (cmd, authorizeSecretIndex)) {
			writeCommand (cmd);
			sent = true;
		}
		delete (cmd);
	}

	if (! sent) {
		isAuthorizeComplete = true;
		isAuthorizing = false;
		// TODO: Possibly disconnect the client here (expecting to retry authorization on the next scheduled reconnect)
	}
}
