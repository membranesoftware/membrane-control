/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Class that holds information about a remote system agent

#ifndef AGENT_H
#define AGENT_H

#include "StdString.h"
#include "Json.h"

class Agent {
public:
	Agent ();
	~Agent ();

	// Read-write data members
	StdString id;
	bool isAttached;
	int64_t lastStatusTime;
	StdString invokeHostname;
	int invokeTcpPort1;
	int invokeTcpPort2;
	StdString authorizePath;
	StdString authorizeSecret;
	StdString authorizeToken;

	// Read-only data members
	int serverType;
	StdString linkPath;
	StdString displayName;
	StdString applicationName;
	StdString urlHostname;
	int tcpPort1;
	int tcpPort2;
	StdString version;
	StdString platform;

	// Return a string representation of the agent
	StdString toString ();

	// Update the agent's fields with information from the provided AgentStatus command
	void readCommand (Json *command);

	// Return a newly created Json object containing the agent's data, suitable for storage in application preferences
	Json *createState ();

	// Update the agent's fields with state information from the provided Json object and return a Result value
	int readState (Json *state);

	// Return the URL that should be used for invoke operations targeting the agent
	StdString getInvokeUrl ();

	// Return the URL that should be used for secondary invoke operations targeting the agent
	StdString getSecondaryUrl ();

	// Return the URL that should be used for link operations targeting the agent
	StdString getLinkUrl ();

	// Return the URL that should be used for retrieving news related to the agent's application, or an empty string if no URL is available
	StdString getApplicationNewsUrl ();

private:
	// Object field names
	static const char *AgentIdKey;
	static const char *InvokeHostnameKey;
	static const char *InvokeTcpPort1Key;
	static const char *InvokeTcpPort2Key;
	static const char *LinkPathKey;
	static const char *DisplayNameKey;
	static const char *UrlHostnameKey;
	static const char *TcpPort1Key;
	static const char *TcpPort2Key;
	static const char *IsAttachedKey;
	static const char *ApplicationNameKey;
	static const char *ServerTypeKey;
};

#endif
