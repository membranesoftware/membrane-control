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
// Class that tracks the state of remote system agents and executes commands to control them

#ifndef AGENT_CONTROL_H
#define AGENT_CONTROL_H

#include <map>
#include <vector>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "StringList.h"
#include "Json.h"
#include "RecordStore.h"
#include "Agent.h"
#include "LinkClient.h"
#include "CommandList.h"
#include "Ui.h"

class AgentControl {
public:
	AgentControl ();
	~AgentControl ();

	static const StdString localHostname;
	static const int commandListIdleTimeout;

	// Read-write data members
	int agentDatagramPort;
	StdString agentId;
	StdString urlHostname;
	RecordStore recordStore;

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

	// Return a boolean value indicating if the specified agent has an established link connection
	bool isLinkClientConnected (const StdString &agentId);

	// Write a command to the specified link client. An empty agentId value causes the command to be written to all connected link clients. This method becomes responsible for deleting the command object when it's no longer needed.
	void writeLinkCommand (Json *command, const StdString &agentId = StdString (""));

	// Return a boolean value indicating if an agent has been contacted at the specified hostname and port
	bool isContacted (const StdString &invokeHostname, int invokePort);

	// Return a boolean value indicating if the specified hostname and port is currently being contacted
	bool isContacting (const StdString &invokeHostname, int invokePort);

	// Return a boolean value indicating if the specified hostname and port has failed contact due to missing authorization credentials
	bool isUnauthorized (const StdString &invokeHostname, int invokePort);

	// Return a string containing the specified agent's display name, or an empty string if no such agent was found
	StdString getAgentDisplayName (const StdString &agentId);

	// Return a string containing the specified agent's invoke URL with an optional command parameter, or an empty string if no such agent was found. If command is not NULL, this method becomes responsible for freeing the object when it's no longer needed.
	StdString getAgentInvokeUrl (const StdString &agentId, Json *command = NULL, const StdString &path = StdString ("/"));

	// Return a string containing the specified agent's secondary URL with an optional command parameter, or an empty string if no such agent was found. If command is not NULL, this method becomes responsible for freeing the object when it's no longer needed.
	StdString getAgentSecondaryUrl (const StdString &agentId, Json *command = NULL, const StdString &path = StdString ("/"));

	// Return a string containing the specified agent's link URL, or an empty string if no such agent was found
	StdString getAgentLinkUrl (const StdString &agentId);

	// Store record data from a received AgentStatus command
	void storeAgentStatus (Json *agentStatusCommand, const StdString &invokeHostname = StdString (""), int invokeTcpPort = 0);

	// Send a network message to attempt contact with an agent at the specified address
	void contactAgent (const StdString &hostname, int tcpPort);

	// Attempt to gather status from all previously known agents that have not yet been successfully contacted
	void retryAgents ();

	// Return a boolean value indicating whether the specified agent ID is currently being contacted to invoke one or more commands
	bool isAgentInvoking (const StdString &agentId);

	// Invoke the GetStatus command from the specified agent and update its record if successful
	void refreshAgentStatus (const StdString &agentId, const StdString &queueId = StdString (""));
	void refreshAgentStatus (const StdString &invokeHostname, int invokeTcpPort, const StdString &queueId = StdString (""));

	// Remove any previously stored records associated with the specified agent
	void removeAgent (const StdString &agentId);

	// Invoke a command from a remote agent, gather response data, and execute the provided callback when complete. A non-empty queueId value indicates that the command should be executed serially with others of the same queueId. This class becomes responsible for freeing the submitted command object when it's no longer needed. Returns a result value indicating if the command was accepted.
	int invokeCommand (const StdString &hostname, int tcpPort, Json *command, CommandList::InvokeCallback callback = NULL, void *callbackData = NULL, const StdString &queueId = StdString (""));
	int invokeCommand (const StdString &agentId, Json *command, CommandList::InvokeCallback callback = NULL, void *callbackData = NULL, const StdString &queueId = StdString (""));

	// Parse the provided message data as a command payload received from a remote agent
	void receiveMessage (const char *messageData, int messageLength);

	// Add an entry to the list of secrets
	void addAdminSecret (const StdString &entryName, const StdString &entrySecret);

	// Remove an entry from the list of secrets
	void removeAdminSecret (const StdString &entryName);

	// Return the secret value associated with the specified entry name, or an empty string if the entry wasn't found
	StdString getAdminSecret (const StdString &entryName);

	// Clear the provided list and populate it with entry names from the list of secrets
	void getAdminSecretNames (StringList *destList);

	// Set authorization prefix fields in the provided command object using the specified secret and an optional token, then store the associated authorization path in the provided string. Returns a boolean value indicating if the authorization fields were successfully applied.
	bool setCommandAuthorization (Json *command, int secretIndex, const StdString &authToken = StdString (""), StdString *authPath = NULL);

	// Return the base invoke URL associated with the provided hostname / port pair
	StdString getHostInvokeUrl (const StdString &hostname, int tcpPort, const StdString &path = StdString ("/"));

	// Return the map key associated with the provided hostname / port pair
	StdString getMapKey (const StdString &hostname, int tcpPort);

	// Return a hash value computed from the provided source string, or an empty string if the hash operation failed
	StdString getStringHash (const StdString &sourceString);

	// Callback functions
	static void linkClientConnect (void *agentControlPtr, const StdString &agentId);
	static void linkClientDisconnect (void *agentControlPtr, const StdString &agentId, const StdString &errorDescription);
	static void linkClientCommand (void *agentControlPtr, const StdString &agentId, Json *command);
	static void invokeGetStatusComplete (void *agentControlPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand);
	static void hashUpdate (void *contextPtr, unsigned char *data, int dataLength);
	static StdString hashDigest (void *contextPtr);

private:
	// Constants to use as object field names
	static const char *NameKey;
	static const char *SecretKey;

	// Remove all items from the command map
	void clearCommandMap ();

	// Write an application preferences value containing cached agent status data
	void writePrefs ();

	// Read previously cached agent status data from preferences
	void readPrefs ();

	// Return the CommandList object of the specified name, or NULL if the object wasn't found. If createNew is true, create the named list if it doesn't already exist. This method should be invoked only while holding a lock on commandMapMutex.
	CommandList *findCommandList (const StdString &listName, bool createNew = false);

	// Return an iterator positioned at the specified agentMap entry, or at the map end if the entry wasn't found. If createNew is true, create the entry if it doesn't already exist. This method should be invoked only while holding a lock on agentMapMutex.
	std::map<StdString, Agent>::iterator findAgent (const StdString &agentId, bool createNew = false);
	std::map<StdString, Agent>::iterator findAgent (const StdString &invokeHostname, int invokePort);

	bool isStarted;
	LinkClient linkClient;

	std::map<StdString, Agent> agentMap;
	SDL_mutex *agentMapMutex;

	std::map<StdString, CommandList *> commandMap;
	SDL_mutex *commandMapMutex;

	std::vector<StdString> adminSecretList;
	SDL_mutex *adminSecretMutex;
};

#endif
