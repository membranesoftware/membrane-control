/*
* Copyright 2018 Membrane Software <author@membranesoftware.com>
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
// Class that runs a systemagent link client using libwebsockets and the engine.io protocol

#ifndef LINK_CLIENT_H
#define LINK_CLIENT_H

#include <list>
#include <queue>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "Buffer.h"
#include "Json.h"
#include "libwebsockets.h"

class LinkClient {
public:
	typedef void (*ConnectCallback) (void *data, LinkClient *client);
	typedef void (*DisconnectCallback) (void *data, LinkClient *client, const StdString &errorDescription);
	typedef void (*CommandCallback) (void *data, LinkClient *client, Json *command);

	LinkClient (const StdString &serverUrl, const StdString &agentId = StdString (""), void *callbackData = NULL, LinkClient::ConnectCallback connectCallback = NULL, LinkClient::DisconnectCallback disconnectCallback = NULL, LinkClient::CommandCallback commandCallback = NULL);
	~LinkClient ();

	static const int defaultPingInterval;
	static const int reconnectPeriod;
	static const int maxCommandSize;

	// Read-write data members
	int usageCount;

	// Read-only data members
	bool isConnected;
	bool isRunning;
	bool isStopped;
	bool isShutdown;
	bool isClosing;
	bool isClosed;
	StdString url;
	StdString agentId;

	// Update client state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Set the client's target URL and trigger a reset for its underlying websocket connection if needed
	void setUrl (const StdString &linkUrl);

	// Notify the client that it should stop any active connection and cease reconnection attempts
	void shutdown ();

	// Stop the client's operation, optionally waiting for its run thread to end before returning
	void stop (bool shouldWait = true);

	// Establish the client connection, then service protocol events until the connection ends
	void run ();

	// Write a message to the client
	void write (const StdString &message);

	// Write a command to the client
	void writeCommand (Json *command);
	void writeCommand (const StdString &commandJson);

	// Parse the provided data buffer, as received from the WebSocket/engine.io server, and execute actions appropriate for the received message
	void receiveData (char *data, int dataLength);

	// Callback function for use as a libwebsockets protocol
	static int protocolCallback (struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

	// Execute initialization routines for libwebsockets
	static void initLib ();

	// Callback function for use with libwebsockets logging
	static void logCallback (int level, const char *line);

	// Run a thread to establish the client connection, then service protocol events until the connection ends
	static int runThread (void *networkPtr);

private:
	// Clear data members as needed for a new client connection
	void clear ();

	// Start the client's operation by launching a background thread that connects to the configured URL
	void start ();

	// Write the next message from the queue
	void writeNextMessage ();

	// Parse a SystemInterface command from the provided socket.io message string and execute actions appropriate for that command
	void processCommandMessage (char *data, int dataLength);

	void *callbackData;
	LinkClient::ConnectCallback connectCallback;
	LinkClient::DisconnectCallback disconnectCallback;
	LinkClient::CommandCallback commandCallback;
	SDL_mutex *runMutex;
	SDL_cond *runCond;
	struct lws_context *context;
	struct lws *lws;
	char *urlParseBuffer;
	std::queue<StdString> writeQueue;
	SDL_mutex *writeQueueMutex;
	bool isWriteReady;
	unsigned char *writeBuffer;
	int writeBufferSize;
	Buffer commandBuffer;
	StdString sessionId;
	int pingInterval; // ms
	int64_t nextPingTime;
	int reconnectClock;
};

#endif
