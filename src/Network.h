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
// Class that handles network interfaces and sockets

#ifndef NETWORK_H
#define NETWORK_H

#include <map>
#include <queue>
#include <list>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "OsUtil.h"
#include "Buffer.h"
#include "SharedBuffer.h"

class Network {
public:
	Network ();
	~Network ();
	static Network *instance;

	static const StdString LocalhostAddress;
	static const int DefaultMaxRequestThreads;
	static const int MaxDatagramSize;

	// HTTP status codes
	enum {
		HttpOkCode = 200,
		HttpUnauthorizedCode = 401
	};

	typedef void (*DatagramCallback) (void *callbackData, const char *messageData, int messageLength, const char *sourceAddress, int sourcePort);
	typedef void (*HttpRequestCallback) (void *callbackData, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);

	struct DatagramCallbackContext {
		Network::DatagramCallback callback;
		void *callbackData;
		DatagramCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		DatagramCallbackContext (Network::DatagramCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};
	struct HttpRequestCallbackContext {
		Network::HttpRequestCallback callback;
		void *callbackData;
		HttpRequestCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		HttpRequestCallbackContext (Network::HttpRequestCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};

	// Read-write data members
	int maxRequestThreads;
	StdString httpUserAgent;
	bool enableDatagramSocket;
	Network::DatagramCallbackContext datagramCallback;

	// Read-only data members
	bool isStarted;
	bool isStopped;
	int datagramPort;

	// Initialize networking functionality and acquire resources as needed. Returns a Result value.
	OsUtil::Result start ();

	// Stop the networking engine and release acquired resources
	void stop ();

	// Return a boolean value indicating if the networking engine has completed its stop operation and its child threads have terminated
	bool isStopComplete ();

	// Join the networking engine's child threads, blocking until the operation completes
	void waitThreads ();

	// Return a string containing the address of the primary network interface, or an empty string if no such address was found
	StdString getPrimaryInterfaceAddress ();

	// Send a datagram packet to a remote host using data from the provided buffer. This class becomes responsible for freeing messageData when it's no longer needed.
	void sendDatagram (const StdString &targetHostname, int targetPort, Buffer *messageData);

	// Send a datagram packet to all known broadcast addresses using data from the provided buffer. This class becomes responsible for freeing messageData when it's no longer needed.
	void sendBroadcastDatagram (int targetPort, Buffer *messageData);

	// Send an HTTP GET request and invoke the provided callback when complete
	void sendHttpGet (const StdString &targetUrl, Network::HttpRequestCallbackContext callback, const StdString &targetServerName = StdString (""));

	// Send an HTTP POST request and invoke the provided callback when complete
	void sendHttpPost (const StdString &targetUrl, const StdString &postData, Network::HttpRequestCallbackContext callback, const StdString &targetServerName = StdString (""));

private:
	// Run a thread that sends datagrams submitted by outside callers
	static int runDatagramSendThread (void *networkPtr);

	// Run a thread that receives messages from datagramSocket
	static int runDatagramReceiveThread (void *networkPtr);

	// Run a thread that sends HTTP requests submitted by outside callers
	static int runHttpRequestThread (void *networkPtr);

	struct Interface {
		int id;
		bool isUp;
		bool isBroadcast;
		bool isLoopback;
		StdString address;
		StdString broadcastAddress;
		Interface ():
			id (0),
			isUp (false),
			isBroadcast (false),
			isLoopback (false),
			address (""),
			broadcastAddress ("") { }
	};
	struct Datagram {
		StdString targetHostname;
		int targetPort;
		Buffer *messageData;
		bool isBroadcast;
		Datagram ():
			targetHostname (""),
			targetPort (0),
			messageData (NULL),
			isBroadcast (false) { }
	};
	struct HttpRequestContext {
		StdString method;
		StdString url;
		StdString postData;
		StdString serverName;
		Network::HttpRequestCallbackContext callback;
		HttpRequestContext ():
			method ("GET"),
			url (""),
			postData (""),
			serverName ("") { }
	};

	// Populate the interface map with data regarding available network interfaces. Returns a Result value.
	OsUtil::Result resetInterfaces ();

	// Remove all items from the datagram queue
	void clearDatagramQueue ();

	// Remove all items from the HTTP request queue
	void clearHttpRequestQueue ();

	// Wait all HTTP request threads
	void waitHttpRequestThreads ();

	// Execute a sendto call to transmit a datagram packet
	int sendTo (const StdString &targetHostname, int targetPort, Buffer *messageData);

	// Execute sendto calls to transmit a datagram packet to each available broadcast address
	int broadcastSendTo (int targetPort, Buffer *messageData);

	// Execute operations to send an HTTP request and gather the response data. Returns a Result value. If successful, this method stores values in the provided pointers, and the caller is responsible for releasing any created SharedBuffer object.
	OsUtil::Result sendHttpRequest (Network::HttpRequestContext *item, int *statusCode, SharedBuffer **responseBuffer);

	// Callback functions for use with libcurl
	static size_t curlWrite (char *ptr, size_t size, size_t nmemb, void *userdata);
	static int curlProgress (void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

	std::map<StdString, Network::Interface> interfaceMap;
	SDL_Thread *datagramSendThread;
	SDL_Thread *datagramReceiveThread;
	std::queue<Network::Datagram> datagramQueue;
	SDL_mutex *datagramSendMutex;
	SDL_cond *datagramSendCond;
	int datagramSocket;
	std::queue<Network::HttpRequestContext> httpRequestQueue;
	std::list<Network::HttpRequestContext> httpShutdownList;
	SDL_mutex *httpRequestQueueMutex;
	SDL_cond *httpRequestQueueCond;
	std::list<SDL_Thread *> httpRequestThreadList;
	int httpRequestThreadStopCount;
#if PLATFORM_WINDOWS
	bool isWsaStarted;
#endif
};

#endif
