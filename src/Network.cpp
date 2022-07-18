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
#include <errno.h>
#include <sys/types.h>
#if PLATFORM_LINUX
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#endif
#if PLATFORM_MACOS
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>
#endif
#if PLATFORM_WINDOWS
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <Iphlpapi.h>
#include <Ipifcons.h>
#endif
#include "curl/curl.h"
#include "openssl/ssl.h"
#include "App.h"
#include "Log.h"
#include "Buffer.h"
#include "OsUtil.h"
#include "StdString.h"
#include "Ipv4Address.h"
#include "Network.h"

Network *Network::instance = NULL;
const StdString Network::LocalhostAddress = StdString ("127.0.0.1");
const int Network::DefaultMaxRequestThreads = 8;
const int Network::MaxDatagramSize = 1500; // bytes

Network::Network ()
: maxRequestThreads (Network::DefaultMaxRequestThreads)
, enableDatagramSocket (false)
, isStarted (false)
, isStopped (false)
, datagramPort (0)
, datagramSendThread (NULL)
, datagramReceiveThread (NULL)
, datagramSendMutex (NULL)
, datagramSendCond (NULL)
, datagramSocket (-1)
, httpRequestQueueMutex (NULL)
, httpRequestQueueCond (NULL)
, httpRequestThreadStopCount (0)
#if PLATFORM_WINDOWS
, isWsaStarted (false)
#endif
{
	datagramSendMutex = SDL_CreateMutex ();
	datagramSendCond = SDL_CreateCond ();
	httpRequestQueueMutex = SDL_CreateMutex ();
	httpRequestQueueCond = SDL_CreateCond ();
}

Network::~Network () {
	stop ();

	if (httpRequestQueueCond) {
		SDL_DestroyCond (httpRequestQueueCond);
		httpRequestQueueCond = NULL;
	}
	if (httpRequestQueueMutex) {
		SDL_DestroyMutex (httpRequestQueueMutex);
		httpRequestQueueMutex = NULL;
	}
	if (datagramSendCond) {
		SDL_DestroyCond (datagramSendCond);
		datagramSendCond = NULL;
	}
	if (datagramSendMutex) {
		SDL_DestroyMutex (datagramSendMutex);
		datagramSendMutex = NULL;
	}
}

void Network::clearDatagramQueue () {
	Network::Datagram item;

	SDL_LockMutex (datagramSendMutex);
	while (! datagramQueue.empty ()) {
		item = datagramQueue.front ();
		if (item.messageData) {
			delete (item.messageData);
			item.messageData = NULL;
		}
		datagramQueue.pop ();
	}
	SDL_CondBroadcast (datagramSendCond);
	SDL_UnlockMutex (datagramSendMutex);
}

void Network::clearHttpRequestQueue () {
	SDL_LockMutex (httpRequestQueueMutex);
	while (! httpRequestQueue.empty ()) {
		httpRequestQueue.pop ();
	}
	SDL_CondBroadcast (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);
}

void Network::waitHttpRequestThreads () {
	std::list<SDL_Thread *>::iterator i, end;
	int result;

	SDL_LockMutex (httpRequestQueueMutex);
	SDL_CondBroadcast (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);

	i = httpRequestThreadList.begin ();
	end = httpRequestThreadList.end ();
	while (i != end) {
		SDL_WaitThread (*i, &result);
		++i;
	}
	httpRequestThreadList.clear ();
}

OsUtil::Result Network::start () {
	struct sockaddr_in saddr;
	OsUtil::Result result;
	int i, cresult;
	socklen_t namelen;
	SDL_Thread *thread;
#if PLATFORM_LINUX || PLATFORM_MACOS
	int sockopt;
	struct protoent *proto;
#endif
#if PLATFORM_WINDOWS
	char sockopt;
	WORD versionrequested;
	WSADATA wsadata;
#endif

	if (isStarted) {
		return (OsUtil::Success);
	}
	if (maxRequestThreads <= 0) {
		Log::warning ("Invalid preferences value %s %i, ignored", App::NetworkThreadsKey, maxRequestThreads);
		maxRequestThreads = Network::DefaultMaxRequestThreads;
	}

#if PLATFORM_WINDOWS
	if (! isWsaStarted) {
		versionrequested = MAKEWORD (2, 2);
		cresult = WSAStartup (versionrequested, &wsadata);
		if (cresult != 0) {
			Log::err ("Network start failed; err=\"WSAStartup: %i\"", cresult);
			return (OsUtil::SocketOperationFailedError);
		}
		Log::debug ("WSAStartup; wsaVersion=%i.%i", HIBYTE (wsadata.wVersion), LOBYTE (wsadata.wVersion));
		isWsaStarted = true;
	}
#endif

	SSL_library_init ();
	cresult = curl_global_init (CURL_GLOBAL_ALL);
	if (cresult != 0) {
		return (OsUtil::LibcurlOperationFailedError);
	}
	result = resetInterfaces ();
	if (result != OsUtil::Success) {
		return (result);
	}

	datagramSocket = -1;
	datagramPort = 0;
	if (enableDatagramSocket) {
#if PLATFORM_LINUX || PLATFORM_MACOS
		proto = getprotobyname ("udp");
		if (! proto) {
			Log::err ("Network start failed; err=\"getprotobyname: %s\"", strerror (errno));
			return (OsUtil::SocketOperationFailedError);
		}
		datagramSocket = socket (PF_INET, SOCK_DGRAM, proto->p_proto);
		endprotoent ();
#endif
#if PLATFORM_WINDOWS
		datagramSocket = socket (AF_INET, SOCK_DGRAM, 0);
#endif
		if (datagramSocket < 0) {
			Log::err ("Network start failed; err=\"socket: %s\"", strerror (errno));
			return (OsUtil::SocketOperationFailedError);
		}

		sockopt = 1;
		if (setsockopt (datagramSocket, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof (sockopt)) < 0) {
			Log::err ("Network start failed; err=\"setsockopt SO_REUSEADDR: %s\"", strerror (errno));
			return (OsUtil::SocketOperationFailedError);
		}

		sockopt = 1;
		if (setsockopt (datagramSocket, SOL_SOCKET, SO_BROADCAST, &sockopt, sizeof (sockopt)) < 0) {
			Log::err ("Network start failed; err=\"setsockopt SO_BROADCAST: %s\"", strerror (errno));
			return (OsUtil::SocketOperationFailedError);
		}

		memset (&saddr, 0, sizeof (struct sockaddr_in));
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = htonl (INADDR_ANY);
		if (bind (datagramSocket, (struct sockaddr *) (&saddr), sizeof (struct sockaddr_in)) < 0) {
			Log::err ("Network start failed; err=\"bind: %s\"", strerror (errno));
			return (OsUtil::SocketOperationFailedError);
		}

		memset (&saddr, 0, sizeof (struct sockaddr_in));
		namelen = sizeof (struct sockaddr_in);
		if (getsockname (datagramSocket, (struct sockaddr *) &saddr, &namelen) < 0) {
			Log::err ("Network start failed; err=\"getsockname: %s\"", strerror (errno));
			return (OsUtil::SocketOperationFailedError);
		}
		datagramPort = (int) ntohs (saddr.sin_port);

		datagramReceiveThread = SDL_CreateThread (Network::runDatagramReceiveThread, "runDatagramReceiveThread", (void *) this);
		if (! datagramReceiveThread) {
			Log::err ("Network start failed; err=\"thread create failed\"");
			return (OsUtil::ThreadCreateFailedError);
		}

		datagramSendThread = SDL_CreateThread (Network::runDatagramSendThread, "runDatagramSendThread", (void *) this);
		if (! datagramSendThread) {
			Log::err ("Network start failed; err=\"thread create failed\"");
			return (OsUtil::ThreadCreateFailedError);
		}
	}

	for (i = 0; i < maxRequestThreads; ++i) {
		thread = SDL_CreateThread (Network::runHttpRequestThread, StdString::createSprintf ("runHttpRequestThread_%i", i).c_str (), (void *) this);
		if (! thread) {
			return (OsUtil::ThreadCreateFailedError);
		}
		httpRequestThreadList.push_back (thread);
	}

	isStarted = true;
	Log::debug ("Network start; datagramSocket=%i datagramPort=%i maxRequestThreads=%i", datagramSocket, datagramPort, maxRequestThreads);

	return (OsUtil::Success);
}

void Network::stop () {
	Network::HttpRequestContext item;

#if PLATFORM_WINDOWS
	if (isWsaStarted) {
		Log::debug ("WSACleanup");
		isWsaStarted = false;
		WSACleanup ();
	}
#endif
	if ((! isStarted) || isStopped) {
		return;
	}
	isStopped = true;
	if (datagramSocket >= 0) {
		shutdown (datagramSocket, SHUT_RDWR);
#if PLATFORM_WINDOWS
		closesocket (datagramSocket);
#else
		close (datagramSocket);
#endif
		datagramSocket = -1;
	}
	clearDatagramQueue ();

	httpShutdownList.clear ();
	SDL_LockMutex (httpRequestQueueMutex);
	while (! httpRequestQueue.empty ()) {
		item = httpRequestQueue.front ();
		httpShutdownList.push_back (item);
		httpRequestQueue.pop ();
	}
	SDL_CondBroadcast (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);

	curl_global_cleanup ();
}

void Network::waitThreads () {
	std::list<Network::HttpRequestContext>::iterator i, end;
	int result;

	clearDatagramQueue ();
	clearHttpRequestQueue ();

	i = httpShutdownList.begin ();
	end = httpShutdownList.end ();
	while (i != end) {
		if (i->callback.callback) {
			i->callback.callback (i->callback.callbackData, i->url, 0, NULL);
		}
		++i;
	}
	httpShutdownList.clear ();

	waitHttpRequestThreads ();
	if (datagramReceiveThread) {
		SDL_WaitThread (datagramReceiveThread, &result);
		datagramReceiveThread = NULL;
	}
	if (datagramSendThread) {
		SDL_WaitThread (datagramSendThread, &result);
		datagramSendThread = NULL;
	}
}

bool Network::isStopComplete () {
	return (isStopped && (httpRequestThreadStopCount >= maxRequestThreads));
}

OsUtil::Result Network::resetInterfaces () {
#if PLATFORM_LINUX
	struct ifreq req, *i, *end;
	struct ifconf conf;
	struct sockaddr_in *addr;
	char confbuf[1024], addrbuf[1024];
	int fd, id;
	OsUtil::Result result;
	StdString name;
	Network::Interface interface;

	fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd < 0) {
		Log::err ("Failed to detect network interfaces; err=\"socket: %s\"", strerror (errno));
		close (fd);
		return (OsUtil::SocketOperationFailedError);
	}

	conf.ifc_len = sizeof (confbuf);
	conf.ifc_buf = confbuf;
	if (ioctl (fd, SIOCGIFCONF, &conf) < 0) {
		Log::err ("Failed to detect network interfaces; err=\"ioctl SIOCGIFCONF: %s\"", strerror (errno));
		close (fd);
		return (OsUtil::SocketOperationFailedError);
	}

	interfaceMap.clear ();
	id = 0;
	result = OsUtil::Success;
	i = conf.ifc_req;
	end = i + (conf.ifc_len / sizeof (struct ifreq));
	while (i != end) {
		strncpy (req.ifr_name, i->ifr_name, sizeof (req.ifr_name));
		++i;

		name.assign (req.ifr_name);
		if (ioctl (fd, SIOCGIFADDR, &req) < 0) {
			Log::warning ("Failed to read network interface; name=\"%s\" err=\"ioctl SIOCGIFADDR: %s\"", name.c_str (), strerror (errno));
			continue;
		}
		if (req.ifr_addr.sa_family != AF_INET) {
			Log::debug ("Skip network interface (not AF_INET); name=\"%s\"", name.c_str ());
			continue;
		}

		addr = (struct sockaddr_in *) &(req.ifr_addr);
		if (! inet_ntop (AF_INET, &(addr->sin_addr), addrbuf, sizeof (addrbuf))) {
			Log::warning ("Failed to read network interface; name=\"%s\" err=\"inet_ntop: %s\"", name.c_str (), strerror (errno));
			continue;
		}
		interface.address.assign (addrbuf);

		if (ioctl (fd, SIOCGIFFLAGS, &req) < 0) {
			Log::warning ("Failed to read network interface; name=\"%s\" err=\"ioctl SIOCGIFFLAGS: %s\"", name.c_str (), strerror (errno));
			continue;
		}
		interface.isUp = (req.ifr_flags & IFF_UP) ? true : false;
		interface.isBroadcast = (req.ifr_flags & IFF_BROADCAST) ? true : false;
		interface.isLoopback = (req.ifr_flags & IFF_LOOPBACK) ? true : false;

		if (! interface.isLoopback) {
			if (! interface.isBroadcast) {
				interface.broadcastAddress.assign ("");
			}
			else {
				if (ioctl (fd, SIOCGIFBRDADDR, &req) < 0) {
					Log::warning ("Failed to read network interface; name=\"%s\" err=\"ioctl SIOCGIFBRDADDR: %s\"", name.c_str (), strerror (errno));
					continue;
				}
				addr = (struct sockaddr_in *) &(req.ifr_broadaddr);
				if (! inet_ntop (AF_INET, &(addr->sin_addr), addrbuf, sizeof (addrbuf))) {
					Log::warning ("Failed to read network interface; name=\"%s\" err=\"inet_ntop: %s\"", name.c_str (), strerror (errno));
					continue;
				}
				interface.broadcastAddress.assign (addrbuf);
			}
		}

		interface.id = id;
		++id;
		Log::debug ("Detected network interface; id=%i name=\"%s\" isUp=%s isBroadcast=%s isLoopback=%s address=%s broadcastAddress=%s", interface.id, name.c_str (), BOOL_STRING (interface.isUp), BOOL_STRING (interface.isBroadcast), BOOL_STRING (interface.isLoopback), interface.address.c_str (), interface.broadcastAddress.c_str ());
		interfaceMap.insert (std::pair<StdString, Network::Interface> (name, interface));
	}

	close (fd);
	return (result);
#endif
#if PLATFORM_MACOS
	StdString name;
	Network::Interface interface;
	struct ifaddrs *ifp, *item;
	struct sockaddr_in *addr;
	char addrbuf[1024];
	int result, id;

	result = getifaddrs (&ifp);
	if (result != 0) {
		Log::warning ("Failed to detect network interfaces; err=\"getifaddrs: %s\"", strerror (errno));
		return (OsUtil::SocketOperationFailedError);
	}

	id = 0;
	item = ifp;
	while (item) {
		name.assign (item->ifa_name);

		if (item->ifa_addr->sa_family != AF_INET) {
			Log::debug ("Skip network interface (not AF_INET); name=\"%s\"", name.c_str ());
			item = item->ifa_next;
			continue;
		}

		addr = (struct sockaddr_in *) item->ifa_addr;
		if (! inet_ntop (AF_INET, &(addr->sin_addr), addrbuf, sizeof (addrbuf))) {
			Log::warning ("Failed to read network interface; name=\"%s\" err=\"inet_ntop: %s\"", name.c_str (), strerror (errno));
			item = item->ifa_next;
			continue;
		}
		interface.address.assign (addrbuf);

		interface.isUp = (item->ifa_flags & IFF_UP) ? true : false;
		interface.isBroadcast = (item->ifa_flags & IFF_BROADCAST) ? true : false;
		interface.isLoopback = (item->ifa_flags & IFF_LOOPBACK) ? true : false;

		if (! interface.isLoopback) {
			if (! interface.isBroadcast) {
				interface.broadcastAddress.assign ("");
			}
			else {
				// TODO: Check if this should use ifa_broadaddr instead
				addr = (struct sockaddr_in *) item->ifa_dstaddr;
				if (! inet_ntop (AF_INET, &(addr->sin_addr), addrbuf, sizeof (addrbuf))) {
					Log::warning ("Failed to read network interface; name=\"%s\" err=\"inet_ntop: %s\"", name.c_str (), strerror (errno));
					item = item->ifa_next;
					continue;
				}
				interface.broadcastAddress.assign (addrbuf);
			}
		}

		interface.id = id;
		++id;
		Log::debug ("Detected network interface; id=%i name=\"%s\" isUp=%s isBroadcast=%s isLoopback=%s address=%s broadcastAddress=%s", interface.id, name.c_str (), BOOL_STRING (interface.isUp), BOOL_STRING (interface.isBroadcast), BOOL_STRING (interface.isLoopback), interface.address.c_str (), interface.broadcastAddress.c_str ());
		interfaceMap.insert (std::pair<StdString, Network::Interface> (name, interface));

		item = item->ifa_next;
	}

	freeifaddrs (ifp);
	return (OsUtil::Success);
#endif
#if PLATFORM_WINDOWS
	PMIB_IPADDRTABLE table;
	DWORD sz, retval;
	Ipv4Address ipaddr;
	StdString name;
	Network::Interface interface;
	IN_ADDR inaddr;
	int i, id;

	sz = 0;
	retval = 0;
	table = (MIB_IPADDRTABLE *) malloc (sizeof (MIB_IPADDRTABLE));
	if (! table) {
		Log::err ("Failed to detect network interfaces (out of memory)");
		return (OsUtil::OutOfMemoryError);
	}

	retval = GetIpAddrTable (table, &sz, 0);
	if (retval == ERROR_INSUFFICIENT_BUFFER) {
		free (table);
		table = (MIB_IPADDRTABLE *) malloc (sz);
		if (! table) {
			Log::err ("Failed to detect network interfaces (out of memory)");
			return (OsUtil::OutOfMemoryError);
		}
		retval = GetIpAddrTable (table, &sz, 0);
	}

	if (retval != NO_ERROR) {
		free (table);
		Log::err ("Failed to detect network interfaces (GetIpAddrTable); result=%i", (int) retval);
		return (OsUtil::SystemOperationFailedError);
	}

	id = 0;
	for (i = 0; i < (int) table->dwNumEntries; i++) {
		name.sprintf ("%i", table->table[i].dwIndex);
		inaddr.S_un.S_addr = (u_long) table->table[i].dwAddr;
		ipaddr.parse (inet_ntoa (inaddr));
		inaddr.S_un.S_addr = (u_long) table->table[i].dwMask;
		ipaddr.parseNetmask (inet_ntoa (inaddr));

		interface.address.assign (ipaddr.toString ());
		interface.broadcastAddress.assign (ipaddr.getBroadcastAddress ());

		interface.isUp = false;
		interface.isLoopback = ipaddr.isLocalhost ();
		interface.isBroadcast = (! interface.broadcastAddress.equals (interface.address));
		if (table->table[i].wType & MIB_IPADDR_PRIMARY) {
			interface.isUp = true;
		}

		interface.id = id;
		++id;
		Log::debug ("Detected network interface; id=%i name=\"%s\" isUp=%s isBroadcast=%s isLoopback=%s address=%s broadcastAddress=%s", interface.id, name.c_str (), BOOL_STRING (interface.isUp), BOOL_STRING (interface.isBroadcast), BOOL_STRING (interface.isLoopback), interface.address.c_str (), interface.broadcastAddress.c_str ());
		interfaceMap.insert (std::pair<StdString, Network::Interface> (name, interface));
	}

	if (table) {
		free (table);
		table = NULL;
	}
	return (OsUtil::Success);
#endif
}

StdString Network::getPrimaryInterfaceAddress () {
	std::map<StdString, Network::Interface>::iterator i, end;
	StdString address;
	Network::Interface *interface;
	int minid;

	minid = -1;
	i = interfaceMap.begin ();
	end = interfaceMap.end ();
	while (i != end) {
		interface = &(i->second);
		if (interface->isUp && (! interface->isLoopback) && interface->isBroadcast && (! interface->address.empty ())) {
			if ((minid < 0) || (interface->id < minid)) {
				minid = interface->id;
				address = interface->address;
			}
		}
		++i;
	}
	return (address);
}

int Network::runDatagramReceiveThread (void *networkPtr) {
	Network *network;
	struct sockaddr_in srcaddr;
	socklen_t addrlen;
	int msglen;
	char buf[Network::MaxDatagramSize], host[NI_MAXHOST];

	network = (Network *) networkPtr;
	while (true) {
		if (network->isStopped || (network->datagramSocket < 0)) {
			break;
		}
		addrlen = sizeof (struct sockaddr_in);
		msglen = recvfrom (network->datagramSocket, buf, sizeof (buf), 0, (struct sockaddr *) &srcaddr, &addrlen);
		if (msglen < 0) {
			break;
		}
		if (msglen == 0) {
			break;
		}
		if (getnameinfo ((struct sockaddr *) &srcaddr, addrlen, host, sizeof (host), NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
			memset (host, 0, sizeof (host));
		}
		if (network->datagramCallback.callback) {
			network->datagramCallback.callback (network->datagramCallback.callbackData, buf, msglen, host, (int) srcaddr.sin_port);
		}
	}

	return (0);
}

int Network::runDatagramSendThread (void *networkPtr) {
	Network *network;
	Network::Datagram item;
	int result;

	network = (Network *) networkPtr;
	SDL_LockMutex (network->datagramSendMutex);
	while (true) {
		if (network->isStopped || (network->datagramSocket < 0)) {
			break;
		}
		if (network->datagramQueue.empty ()) {
			SDL_CondWait (network->datagramSendCond, network->datagramSendMutex);
			continue;
		}

		item = network->datagramQueue.front ();
		network->datagramQueue.pop ();
		SDL_UnlockMutex (network->datagramSendMutex);

		if (! item.messageData) {
			Log::warning ("Discard queued datagram (no message data provided)");
		}
		else {
			if (item.isBroadcast) {
				result = network->broadcastSendTo (item.targetPort, item.messageData);
			}
			else {
				result = network->sendTo (item.targetHostname, item.targetPort, item.messageData);
			}

			if (result != OsUtil::Success) {
				Log::debug3 ("Failed to send datagram; err=%i", result);
			}
			delete (item.messageData);
			item.messageData = NULL;
		}

		SDL_LockMutex (network->datagramSendMutex);
	}
	SDL_UnlockMutex (network->datagramSendMutex);

	return (0);
}

void Network::sendDatagram (const StdString &targetHostname, int targetPort, Buffer *messageData) {
	Network::Datagram item;

	item.targetHostname.assign (targetHostname);
	item.targetPort = targetPort;
	item.messageData = messageData;
	SDL_LockMutex (datagramSendMutex);
	datagramQueue.push (item);
	SDL_CondSignal (datagramSendCond);
	SDL_UnlockMutex (datagramSendMutex);
}

void Network::sendBroadcastDatagram (int targetPort, Buffer *messageData) {
	Network::Datagram item;

	item.targetPort = targetPort;
	item.messageData = messageData;
	item.isBroadcast = true;
	SDL_LockMutex (datagramSendMutex);
	datagramQueue.push (item);
	SDL_CondSignal (datagramSendCond);
	SDL_UnlockMutex (datagramSendMutex);
}

void Network::sendHttpGet (const StdString &targetUrl, Network::HttpRequestCallbackContext callback, const StdString &targetServerName) {
	Network::HttpRequestContext item;

	item.method.assign ("GET");
	item.url.assign (targetUrl);
	item.callback = callback;
	item.serverName.assign (targetServerName);
	SDL_LockMutex (httpRequestQueueMutex);
	httpRequestQueue.push (item);
	SDL_CondSignal (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);
}

void Network::sendHttpPost (const StdString &targetUrl, const StdString &postData, Network::HttpRequestCallbackContext callback, const StdString &targetServerName) {
	Network::HttpRequestContext item;

	item.method.assign ("POST");
	item.url.assign (targetUrl);
	item.postData.assign (postData);
	item.serverName.assign (targetServerName);
	item.callback = callback;
	SDL_LockMutex (httpRequestQueueMutex);
	httpRequestQueue.push (item);
	SDL_CondSignal (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);
}

int Network::sendTo (const StdString &targetHostname, int targetPort, Buffer *messageData) {
	StdString portstr;
	struct addrinfo hints;
	struct addrinfo *addr;
	int result;

	if ((! isStarted) || (datagramSocket < 0)) {
		return (OsUtil::SocketNotConnectedError);
	}
	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	portstr.sprintf ("%i", targetPort);
	result = getaddrinfo (targetHostname.c_str (), portstr.c_str (), &hints, &addr);
	if (result != 0) {
		return (OsUtil::UnknownHostnameError);
	}
	if (! addr) {
		return (OsUtil::SocketOperationFailedError);
	}

	result = sendto (datagramSocket, (char *) messageData->data, messageData->length, 0, addr->ai_addr, addr->ai_addrlen);
	freeaddrinfo (addr);
	if (result < 0) {
		return (OsUtil::SocketOperationFailedError);
	}
	return (OsUtil::Success);
}

int Network::broadcastSendTo (int targetPort, Buffer *messageData) {
	std::map<StdString, Network::Interface>::iterator i, end;
	Network::Interface *interface;
	int result, sendtoresult, successcount;

	result = OsUtil::Success;
	successcount = 0;
	i = interfaceMap.begin ();
	end = interfaceMap.end ();
	while (i != end) {
		interface = &(i->second);
		if (interface->isBroadcast && interface->isUp && (! interface->isLoopback) && (! interface->broadcastAddress.empty ()) && (! interface->broadcastAddress.equals ("0.0.0.0"))) {
			sendtoresult = sendTo (interface->broadcastAddress, targetPort, messageData);
			if (sendtoresult == OsUtil::Success) {
				++successcount;
			}
			else {
				result = sendtoresult;
			}
		}
		++i;
	}

	if ((result != OsUtil::Success) && (successcount > 0)) {
		result = OsUtil::Success;
	}
	return (result);
}

int Network::runHttpRequestThread (void *networkPtr) {
	Network *network;
	Network::HttpRequestContext item;
	int result, statuscode;
	SharedBuffer *responsebuffer;

	network = (Network *) networkPtr;

	SDL_LockMutex (network->httpRequestQueueMutex);
	while (true) {
		if (network->isStopped) {
			break;
		}
		if (network->httpRequestQueue.empty ()) {
			SDL_CondWait (network->httpRequestQueueCond, network->httpRequestQueueMutex);
			continue;
		}

		item = network->httpRequestQueue.front ();
		network->httpRequestQueue.pop ();
		SDL_UnlockMutex (network->httpRequestQueueMutex);

		statuscode = 0;
		responsebuffer = NULL;
		result = network->sendHttpRequest (&item, &statuscode, &responsebuffer);
		if (result != OsUtil::Success) {
			statuscode = 0;
		}
		if (item.callback.callback) {
			item.callback.callback (item.callback.callbackData, item.url, statuscode, responsebuffer);
		}
		if (responsebuffer) {
			responsebuffer->release ();
			responsebuffer = NULL;
		}

		SDL_LockMutex (network->httpRequestQueueMutex);
	}
	++(network->httpRequestThreadStopCount);
	SDL_UnlockMutex (network->httpRequestQueueMutex);

	return (0);
}

OsUtil::Result Network::sendHttpRequest (Network::HttpRequestContext *item, int *statusCode, SharedBuffer **responseBuffer) {
	CURL *curl;
	struct curl_slist *headers;
	CURLcode code;
	SharedBuffer *buffer;
	long responsecode;
	OsUtil::Result result;

	curl = curl_easy_init ();
	if (! curl) {
		return (OsUtil::LibcurlOperationFailedError);
	}
	result = OsUtil::Success;
	responsecode = 0;
	headers = NULL;
	code = CURLE_UNKNOWN_OPTION;
	buffer = new SharedBuffer ();
	buffer->retain ();
	curl_easy_setopt (curl, CURLOPT_VERBOSE, 0);
	curl_easy_setopt (curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, Network::curlWrite);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, buffer);
	curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt (curl, CURLOPT_PROGRESSFUNCTION, Network::curlProgress);

	if (! item->serverName.empty ()) {
		headers = curl_slist_append (headers, StdString::createSprintf ("Host: %s", item->serverName.c_str ()).c_str ());
		curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
	}

	curl_easy_setopt (curl, CURLOPT_URL, item->url.c_str ());
	if (! httpUserAgent.empty ()) {
		curl_easy_setopt (curl, CURLOPT_USERAGENT, httpUserAgent.c_str ());
	}

	if (App::instance->isHttpsEnabled && item->url.startsWith ("https://")) {
		curl_easy_setopt (curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

		// TODO: Possibly enable certificate validation (currently disabled to allow use of self-signed certificates)
		curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);
	}

	if (item->method.equals ("GET")) {
		code = curl_easy_perform (curl);
	}
	else if (item->method.equals ("POST")) {
		curl_easy_setopt (curl, CURLOPT_POST, 1);
		curl_easy_setopt (curl, CURLOPT_POSTFIELDS, item->postData.c_str ());
		curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, item->postData.length ());
		code = curl_easy_perform (curl);
	}
	else {
		result = OsUtil::UnknownMethodError;
	}

	if (result == OsUtil::Success) {
		if (code != CURLE_OK) {
			result = OsUtil::LibcurlOperationFailedError;
		}
	}

	if (result != OsUtil::Success) {
		delete (buffer);
	}
	else {
		code = curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &responsecode);
		if (code == CURLE_OK) {
			if (statusCode) {
				*statusCode = (int) responsecode;
			}
		}
		if (responseBuffer) {
			*responseBuffer = buffer;
		}
		else {
			delete (buffer);
		}
	}

	curl_easy_cleanup (curl);
	if (headers) {
		curl_slist_free_all (headers);
		headers = NULL;
	}
	return (result);
}

size_t Network::curlWrite (char *ptr, size_t size, size_t nmemb, void *userdata) {
	SharedBuffer *buffer;
	size_t total;

	buffer = (SharedBuffer *) userdata;
	total = size * nmemb;
	buffer->add ((uint8_t *) ptr, (int) total);
	return (total);
}

int Network::curlProgress (void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
	if (App::instance->isShuttingDown || App::instance->isShutdown) {
		return (-1);
	}
	return (0);
}
