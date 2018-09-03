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
// Class that tracks the state of remote system agents and executes commands to control them

#ifndef AGENT_CONTROL_H
#define AGENT_CONTROL_H

#include <map>
#include <list>
#include <queue>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "Json.h"
#include "Buffer.h"
#include "RecordStore.h"
#include "HashMap.h"
#include "LinkClient.h"
#include "Ui.h"

class AgentControl {
public:
	AgentControl ();
	~AgentControl ();

	typedef void (*InvokeCommandCallback) (void *data, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);

	static const StdString localHostname;

	// Read-write data members
	int agentDatagramPort;
	StdString agentId;
	StdString urlHostname;
	RecordStore recordStore;

	// Read-only data members
	int linkClientConnectionCount;

	// Start the agent control's operation. Returns a Result value.
	int start ();

	// Stop the agent control's operation
	void stop ();

	// Update agent control state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Broadcast a message requesting that remote agents report contact information to the control
	void broadcastContactMessage ();

	// Request that the agent control maintain a link client connection to the specified agent
	void connectLinkClient (const StdString &agentId);

	// Remove a previously requested link client connection
	void disconnectLinkClient (const StdString &agentId);

	// Request that the agent control contact an agent at the specified address and maintain a link client connection to it if available
	void connectLinkClientToAddress (const StdString &address);

	// Write a command to the specified link client. An empty agentId value causes the command to be written to all connected link clients.
	void writeLinkCommand (Json *command, const StdString &agentId = StdString (""));
	void writeLinkCommand (const StdString &commandJson, const StdString &agentId = StdString (""));

	// Return a boolean value indicating if the specified link client is connected
	bool isLinkClientConnected (const StdString &agentId);

	// Return a boolean value indicating if a connect attempt to the specified link address is in progress
	bool isLinkClientConnectingToAddress (const StdString &address);

	// Return the number of active link client connections
	int getLinkClientCount ();

	// Return a string value containing the specified agent's display name, or an empty string if no such agent was found
	StdString getAgentDisplayName (const StdString &agentId);

	// Store record data from a received AgentStatus command
	void storeAgentStatus (Json *agentStatusCommand);

	// Invoke a command from a remote agent, gather response data, and invoke the provided callback when complete. A non-empty queueId value indicates that the command should be executed serially with others of the same name. This class becomes responsible for freeing the submitted command object when it's no longer needed. Returns a result value indicating if the command was accepted. If successful, the method stores a jobId value in the provided pointer.
	int invokeCommand (const StdString &agentId, Json *command, AgentControl::InvokeCommandCallback callback = NULL, void *callbackData = NULL, int64_t *jobId = NULL, const StdString &queueId = StdString (""));
	int invokeCommand (const StdString &hostname, int tcpPort, Json *command, AgentControl::InvokeCommandCallback callback = NULL, void *callbackData = NULL, int64_t *jobId = NULL, const StdString &queueId = StdString (""));

	// Parse the provided message data as a command payload received from a remote agent
	void receiveMessage (const char *messageData, int messageLength);

	// Read configuration fields and an agent ID value from the locally installed system agent and store them in the provided objects. Returns a Result value.
	static int readSystemAgentConfiguration (HashMap *destMap, StdString *agentId);

	// Callback functions
	static void sendHttpPostComplete (void *contextPtr, const StdString &targetUrl, int statusCode, Buffer *responseData);
	static void linkClientConnect (void *agentControlPtr, LinkClient *client);
	static void linkClientDisconnect (void *agentControlPtr, LinkClient *client, const StdString &errorDescription);
	static void linkClientCommand (void *agentControlPtr, LinkClient *client, Json *command);
	static void invokeAgentContactGetStatusComplete (void *agentControlPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);
	static void invokeLinkServerGetStatusComplete (void *agentControlPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);

private:
	struct CommandContext {
		StdString agentId;
		StdString queueId;
		StdString url;
		Json *command;
		int64_t jobId;
		AgentControl::InvokeCommandCallback callback;
		void *callbackData;
		bool isActive;
		CommandContext (): agentId (""), queueId (""), url (""), command (NULL), jobId (0), callback (NULL), callbackData (NULL), isActive (false) { }
	};
	typedef std::queue<AgentControl::CommandContext *> CommandContextQueue;

	// Destroy the provided CommandContext object
	void destroyCommandContext (AgentControl::CommandContext *ctx);

	// Remove all items from the link client map
	void clearLinkClients ();

	// Remove all items from the command queue map
	void clearCommandQueues ();

	// Set entries in agentInvokeUrlMap and agentLinkUrlMap as appropriate for the provided AgentStatus or AgentContact command
	void storeAgentUrls (Json *command);

	// Write an application preferences value containing cached agent status data
	void writePrefs ();

	// Read previously cached agent status data from preferences
	void readPrefs ();

	// Add a CommandContext for execution in serial within the specified queue
	void addQueueCommand (const StdString &queueId, AgentControl::CommandContext *ctx);

	// If the specified command queue is populated but not executing a request, execute the next stored item. If removeTop is true, remove the top item from the queue before attempting to execute the next one.
	void processQueueCommand (const StdString &queueId, bool removeTop = false);

	bool isStarted;
	std::map<StdString, LinkClient *> linkClientMap;
	std::list<LinkClient *> linkClientCloseList;
	SDL_mutex *linkClientMutex;
	std::map<StdString, AgentControl::CommandContextQueue> commandQueueMap;
	SDL_mutex *commandQueueMutex;

	// A map of agent ID values to invoke URLs
	HashMap agentInvokeUrlMap;

	// A map of agent ID values to link server URLs
	HashMap agentLinkUrlMap;

	// A map of agent ID values to display name strings
	HashMap agentDisplayNameMap;

	// A map of connect address values to job ID values associated with a connect attempt in progress
	HashMap linkClientConnectAddressMap;
};

#endif
