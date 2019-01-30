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
#include <string.h>
#include "SDL2/SDL.h"
#include "libwebsockets.h"
#include "App.h"
#include "Result.h"
#include "StdString.h"
#include "Log.h"
#include "Util.h"
#include "Json.h"
#include "SystemInterface.h"
#include "LinkClient.h"

static bool initLibDone = false;

const int LinkClient::defaultPingInterval = 25000; // milliseconds
const int LinkClient::reconnectPeriod = 7000; // milliseconds
const int LinkClient::maxCommandSize = (256 * 1024); // bytes

static const struct lws_extension libwebsocketExts[] = {
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

static const struct lws_protocols libwebsocketProtocols[] = {
	{
		"ws",
		LinkClient::protocolCallback,
		0,
		8192
	},
	{ NULL, NULL, 0, 0 }
};

LinkClient::LinkClient (const StdString &serverUrl, const StdString &agentId, void *callbackData, LinkClient::ConnectCallback connectCallback, LinkClient::DisconnectCallback disconnectCallback, LinkClient::CommandCallback commandCallback)
: usageCount (0)
, isConnected (false)
, isRunning (false)
, isStopped (false)
, isShutdown (false)
, isClosing (false)
, isClosed (false)
, url (serverUrl)
, agentId (agentId)
, callbackData (callbackData)
, connectCallback (connectCallback)
, disconnectCallback (disconnectCallback)
, commandCallback (commandCallback)
, runMutex (NULL)
, runCond (NULL)
, context (NULL)
, lws (NULL)
, urlParseBuffer (NULL)
, writeQueueMutex (NULL)
, isWriteReady (false)
, writeBuffer (NULL)
, writeBufferSize (0)
, pingInterval (0)
, nextPingTime (0)
, reconnectClock (0)
, authorizeSecretIndex (0)
, isAuthorizing (false)
, isAuthorizeComplete (false)
{
	runMutex = SDL_CreateMutex ();
	runCond = SDL_CreateCond ();
	writeQueueMutex = SDL_CreateMutex ();
}

LinkClient::~LinkClient () {
	stop ();
	clear ();
	if (context) {
		lws_context_destroy (context);
		context = NULL;
	}
	if (runCond) {
		SDL_DestroyCond (runCond);
		runCond = NULL;
	}
	if (runMutex) {
		SDL_DestroyMutex (runMutex);
		runMutex = NULL;
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

void LinkClient::clear () {
	SDL_LockMutex (writeQueueMutex);
	while (! writeQueue.empty ()) {
		writeQueue.pop ();
	}
	SDL_UnlockMutex (writeQueueMutex);

	if (urlParseBuffer) {
		free (urlParseBuffer);
		urlParseBuffer = NULL;
	}

	lws = NULL;
	isConnected = false;
	isStopped = false;
	isClosing = false;
	isClosed = false;
	pingInterval = 0;
	nextPingTime = 0;
	commandBuffer.reset ();
	sessionId.assign ("");
}

void LinkClient::update (int msElapsed) {
	if ((! isShutdown) && (! isRunning)) {
		reconnectClock -= msElapsed;
		if (reconnectClock <= 0) {
			start ();
			reconnectClock = LinkClient::reconnectPeriod;
		}
	}
}

void LinkClient::setUrl (const StdString &linkUrl) {
	if (url.equals (linkUrl)) {
		return;
	}

	url.assign (linkUrl);
	if (isRunning) {
		stop ();
	}
	reconnectClock = 0;
}

void LinkClient::shutdown () {
	isShutdown = true;
	stop (false);
}

void LinkClient::start () {
	struct lws_context_creation_info contextinfo;
	SDL_Thread *thread;
	bool shouldrun;

	LinkClient::initLib ();
	shouldrun = false;
	SDL_LockMutex (runMutex);
	if (! isRunning) {
		shouldrun = true;
		isRunning = true;
	}
	SDL_UnlockMutex (runMutex);

	if (! shouldrun) {
		return;
	}

	if (! context) {
		memset (&contextinfo, 0, sizeof contextinfo);
		contextinfo.port = CONTEXT_PORT_NO_LISTEN;
		contextinfo.protocols = libwebsocketProtocols;
		contextinfo.gid = -1;
		contextinfo.uid = -1;
		contextinfo.ws_ping_pong_interval = 0;
		contextinfo.extensions = libwebsocketExts;
		contextinfo.user = this;

		if (App::getInstance ()->isHttpsEnabled) {
			contextinfo.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		}

		context = lws_create_context (&contextinfo);
		if (! context) {
			Log::write (Log::ERR, "Failed to create libwebsockets context; url=\"%s\"", url.c_str ());
			isRunning = false;
			return;
		}
	}

	clear ();
	thread = SDL_CreateThread (LinkClient::runThread, "LinkClient::runThread", (void *) this);
	if (! thread) {
		Log::write (Log::ERR, "WebSocket client start failed; err=\"thread create failed\"");
		isRunning = false;
		return;
	}
	SDL_DetachThread (thread);
}

void LinkClient::stop (bool shouldWait) {
	SDL_LockMutex (runMutex);
	if (isRunning && (! isStopped)) {
		isStopped = true;
	}
	if (shouldWait) {
		while (isRunning) {
			SDL_CondWait (runCond, runMutex);
		}
	}
	SDL_UnlockMutex (runMutex);
}

void LinkClient::initLib () {
	int level;

	if (initLibDone) {
		return;
	}

	level = 0;
	lws_set_log_level (level, LinkClient::logCallback);
	initLibDone = true;
}

void LinkClient::logCallback (int level, const char *line) {
}

int LinkClient::runThread (void *clientPtr) {
	LinkClient *client;

	client = (LinkClient *) clientPtr;

	client->run ();

	client->clear ();

	SDL_LockMutex (client->runMutex);
	client->isRunning = false;
	SDL_CondBroadcast (client->runCond);
	SDL_UnlockMutex (client->runMutex);
	return (0);
}

void LinkClient::run () {
	App *app;
	lws_client_connect_info clientinfo;
	int result, port, sz;
	const char *protocol, *address, *path;
	int64_t now;

	if (url.empty ()) {
		return;
	}

	app = App::getInstance ();
	sz = url.length () + 1;
	urlParseBuffer = (char *) malloc (sz);
	if (! urlParseBuffer) {
		Log::write (Log::ERR, "Failed to allocate memory for url parse buffer; length=%i", sz);
		return;
	}
	memset (urlParseBuffer, 0, sz);
	memcpy (urlParseBuffer, url.c_str (), sz);
	result = lws_parse_uri (urlParseBuffer, &protocol, &address, &port, &path);
	if (result != 0) {
		Log::write (Log::ERR, "Failed to parse WebSocket URL; url=\"%s\" err=%i", url.c_str (), result);
		return;
	}

	urlPath.assign ("");
	if (path[0] != '/') {
		urlPath.append ("/");
	}
	urlPath.append (path);
	memset (&clientinfo, 0, sizeof (clientinfo));
	clientinfo.context = context;
	clientinfo.address = address;
	clientinfo.port = port;
	clientinfo.path = urlPath.c_str ();
	clientinfo.host = clientinfo.address;
	clientinfo.origin = clientinfo.address;
	clientinfo.ietf_version_or_minus_one = -1;

	if (app->isHttpsEnabled) {
		// TODO: Possibly enable certificate validation (currently disabled to allow use of self-signed certificates)
		clientinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK | LCCSCF_ALLOW_EXPIRED;
	}
	else {
		clientinfo.ssl_connection = 0;
	}

	lws = lws_client_connect_via_info (&clientinfo);
	if (! lws) {
		Log::write (Log::ERR, "Failed to establish WebSocket connection; url=\"%s\"", url.c_str ());
		return;
	}

	while (true) {
		if (isStopped || isClosing || isClosed) {
			break;
		}

		if ((! isWriteReady) && (! writeQueue.empty ())) {
			isWriteReady = true;
			lws_callback_on_writable (lws);
		}

		if ((pingInterval > 0) && (nextPingTime > 0)) {
			now = Util::getTime ();
			if (nextPingTime <= now) {
				write (StdString ("2"));
				nextPingTime = 0;
			}
		}

		lws_service (context, app->minUpdateFrameDelay);
	}

	isConnected = false;
	while (true) {
		if (isClosed) {
			break;
		}

		isClosing = true;
		lws_callback_on_writable (lws);
		lws_service (context, app->minUpdateFrameDelay);
	}
}

void LinkClient::authorize () {
	App *app;
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

	app = App::getInstance ();
	sent = false;
	authorizeToken.assign ("");
	params = new Json ();
	params->set ("token", app->getRandomString (64));
	cmd = app->createCommand ("Authorize", SystemInterface::Constant_DefaultCommandType, params);
	if (cmd) {
		if (app->agentControl.setCommandAuthorization (cmd, authorizeSecretIndex)) {
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

void LinkClient::write (const StdString &message) {
	SDL_LockMutex (writeQueueMutex);
	writeQueue.push (message);
	SDL_UnlockMutex (writeQueueMutex);
}

void LinkClient::writeCommand (Json *sourceCommand) {
	App *app;
	Json *cmd;

	app = App::getInstance ();
	if (authorizeToken.empty ()) {
		writeCommand (sourceCommand->toString ());
	}
	else {
		cmd = new Json ();
		cmd->copy (sourceCommand);
		if (! app->agentControl.setCommandAuthorization (cmd, authorizeSecretIndex, authorizeToken)) {
			authorizeToken.assign ("");
			writeCommand (sourceCommand->toString ());
		}
		else {
			writeCommand (cmd->toString ());
		}
		delete (cmd);
	}
}

void LinkClient::writeCommand (const StdString &commandJson) {
	// This output string specifies packet types 4 (message) and 2 (event) for the engine.io / socket.io protocols
	write (StdString::createSprintf ("42[\"%s\",%s]", SystemInterface::Constant_WebSocketEvent, commandJson.c_str ()));
}

void LinkClient::writeNextMessage () {
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
				Log::write (Log::ERR, "Failed to allocate memory for WebSocket write buffer; length=%i", buflen);
				return;
			}
		}

		memcpy (writeBuffer + LWS_PRE, message.c_str (), message.length ());
		result = lws_write (lws, writeBuffer + LWS_PRE, message.length (), LWS_WRITE_TEXT);
		if (result < 0) {
			Log::write (Log::ERR, "Failed to write WebSocket messge; err=%i", result);
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

void LinkClient::receiveData (char *data, int dataLength) {
	Json *json;
	char *d, packettype;
	int result, len;

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
			result = json->parse (d, len);
			if (result != Result::SUCCESS) {
				Log::write (Log::ERR, "WebSocket connection error; url=\"%s\" err=\"Failed to parse open packet, %i\"", url.c_str (), result);
			}
			else {
				sessionId = json->getString ("sid", "");
				pingInterval = json->getNumber ("pingInterval", LinkClient::defaultPingInterval);
				if (pingInterval <= 0) {
					nextPingTime = 0;
				}
				else {
					nextPingTime = Util::getTime () + pingInterval;
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
				nextPingTime = Util::getTime () + pingInterval;
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

void LinkClient::processCommandMessage (char *data, int dataLength) {
	App *app;
	StdString s, prefix;
	size_t pos1, pos2;
	Json *cmd;
	int commandid;

	if ((! data) || (dataLength <= 0)) {
		return;
	}

	app = App::getInstance ();
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
		if (commandBuffer.length > LinkClient::maxCommandSize) {
			commandBuffer.reset ();
		}
		return;
	}
	--pos2;

	s.assign (s.substr (pos1, pos2 - pos1 + 1));
	if (! App::getInstance ()->systemInterface.parseCommand (s, &cmd)) {
		if (commandBuffer.empty ()) {
			commandBuffer.add ((uint8_t *) data, dataLength);
		}
		if (commandBuffer.length > LinkClient::maxCommandSize) {
			commandBuffer.reset ();
		}
		return;
	}
	commandBuffer.reset ();

	commandid = app->systemInterface.getCommandId (cmd);
	switch (commandid) {
		case SystemInterface::Command_AuthorizationRequired: {
			authorize ();
			break;
		}
		case SystemInterface::Command_AuthorizeResult: {
			authorizeToken = app->systemInterface.getCommandStringParam (cmd, "token", "");
			break;
		}
		case SystemInterface::Command_LinkSuccess: {
			if (! isConnected) {
				isConnected = true;
				if (connectCallback) {
					connectCallback (callbackData, this);
				}
			}
			break;
		}
		default: {
			if (commandCallback) {
				commandCallback (callbackData, this, cmd);
			}
			break;
		}
	}
	delete (cmd);
}

int LinkClient::protocolCallback (struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
	LinkClient *client;
	int result;

	client = (LinkClient *) lws_context_user (lws_get_context (wsi));
	if (! client) {
		return (-1);
	}

	result = 0;
	switch (reason) {
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
			Log::write (Log::DEBUG, "WebSocket connection error; url=\"%s\" err=\"%s\"", client->url.c_str (), in ? StdString ((char *) in, len).c_str () : "unspecified connection failure");
			break;
		}
		case LWS_CALLBACK_CLIENT_ESTABLISHED: {
			break;
		}
		case LWS_CALLBACK_CLOSED: {
			client->isConnected = false;
			client->isClosing = true;
			if (client->disconnectCallback) {
				// TODO: Possibly set an error description string here
				client->disconnectCallback (client->callbackData, client, StdString (""));
			}
			break;
		}
		case LWS_CALLBACK_CLIENT_WRITEABLE: {
			if (client->isClosing) {
				result = -1;
			}
			else {
				client->writeNextMessage ();
			}
			break;
		}
		case LWS_CALLBACK_RECEIVE: {
			client->receiveData ((char *) in, len);
			break;
		}
		case LWS_CALLBACK_CLIENT_RECEIVE: {
			client->receiveData ((char *) in, len);
			break;
		}
		case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: {
			client->isConnected = false;
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
			client->isClosed = true;
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
		default: {
			break;
		}
	}

	return (result);
}
