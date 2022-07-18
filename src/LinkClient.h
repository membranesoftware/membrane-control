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
// Class that runs a set of systemagent link clients using libwebsockets and the engine.io protocol

#ifndef LINK_CLIENT_H
#define LINK_CLIENT_H

#include <map>
#include <list>
#include <queue>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "Buffer.h"
#include "Json.h"
#include "libwebsockets.h"

class LinkContext {
public:
	typedef void (*ConnectCallback) (void *data, const StdString &agentId);
	typedef void (*DisconnectCallback) (void *data, const StdString &agentId, const StdString &errorDescription);
	typedef void (*CommandCallback) (void *data, const StdString &agentId, Json *command);

	LinkContext ();
	~LinkContext ();

	// Read-write data members
	StdString agentId;
	int usageCount;
	StdString linkUrl;
	StdString serverName;
	bool isConnected;
	bool isClosing;
	bool isClosed;
	int reconnectClock;
	struct lws_context *context;
	struct lws *lws;
	char *urlParseBuffer;
	StdString urlPath;
	bool isWriteReady;
	std::queue<StdString> writeQueue;
	SDL_mutex *writeQueueMutex;
	unsigned char *writeBuffer;
	int writeBufferSize;
	Buffer commandBuffer;
	StdString sessionId;
	int pingInterval; // ms
	int64_t nextPingTime;
	StdString authorizeToken;
	int authorizeSecretIndex;
	bool isAuthorizing;
	bool isAuthorizeComplete;
	void *callbackData;
	LinkContext::ConnectCallback connectCallback;
	LinkContext::DisconnectCallback disconnectCallback;
	LinkContext::CommandCallback commandCallback;

	// Parse the provided data buffer, as received from the WebSocket/engine.io server, and execute actions appropriate for the received message
	void receiveData (char *data, int dataLength);

	// Add a message to the context's write queue
	void writeMessage (const StdString &message);

	// Add a command message to the context's write queue
	void writeCommand (Json *sourceCommand);
	void writeCommand (const StdString &commandJson);

	// Write the next message from the queue
	void sendNextMessage ();

	// Parse a SystemInterface command from the provided socket.io message string and execute actions appropriate for that command
	void processCommandMessage (char *data, int dataLength);

	// Execute an authorize command using the next available authorize secret, if any
	void authorize ();
};

class LinkClient {
public:
	LinkClient (void *callbackData = NULL, LinkContext::ConnectCallback connectCallback = NULL, LinkContext::DisconnectCallback disconnectCallback = NULL, LinkContext::CommandCallback commandCallback = NULL);
	~LinkClient ();

	static const int DefaultPingInterval;
	static const int ReconnectPeriod;
	static const int MaxCommandSize;

	// Start the link client's operation
	void start ();

	// Disconnect all link client contexts
	void clear ();

	// Update link client state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Create a context that maintains a link connection to the specified agent
	void connect (const StdString &agentId, const StdString &linkUrl);

	// Remove a previously created connection context
	void disconnect (const StdString &agentId);

	// Return a boolean value indicating if the specified agent has an established link connection
	bool isConnected (const StdString &agentId);

	// Reset the link URL for an active agent context
	void setLinkUrl (const StdString &agentId, const StdString &linkUrl);

	// Write a command to the specified agent if it's connected. An empty agentId value causes the command to be written to all connected link clients. This method becomes responsible for deleting the command object when it's no longer needed.
	void writeCommand (Json *command, const StdString &agentId = StdString (""));

	// Callback function for use as a libwebsockets protocol
	static int protocolCallback (struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

	// Callback function for use with libwebsockets logging
	static void logCallback (int level, const char *line);

private:
	// Update link client context state as appropriate for an elapsed millisecond time period
	void updateActiveContext (LinkContext *ctx, int msElapsed);

	void *callbackData;
	LinkContext::ConnectCallback connectCallback;
	LinkContext::DisconnectCallback disconnectCallback;
	LinkContext::CommandCallback commandCallback;

	std::map<StdString, LinkContext *> contextMap;
	std::list<LinkContext *> closeContextList;
	SDL_mutex *contextMutex;
};

#endif
