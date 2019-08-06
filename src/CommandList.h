/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Class that holds a list of commands to be invoked on remote agents

#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

#include <list>
#include <map>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "HashMap.h"
#include "SharedBuffer.h"
#include "Json.h"

class CommandList {
public:
	CommandList (const StdString &name);
	~CommandList ();

	typedef void (*InvokeCallback) (void *data, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand);

	struct Context {
		StdString hostname;
		int tcpPort;
		Json *command;
		bool isAuthorizing;
		bool isAuthorizeComplete;
		int authorizeSecretIndex;
		CommandList::InvokeCallback callback;
		void *callbackData;
		Context (): tcpPort (0), command (NULL), isAuthorizing (false), isAuthorizeComplete (false), authorizeSecretIndex (0), callback (NULL), callbackData (NULL) { }
	};

	// Read-only data members
	StdString name;
	int lastInvokeResult;

	// Return a boolean value indicating if the command list is empty
	bool empty ();

	// Return a boolean value indicating if the command list is idle according to the specified millisecond timeout
	bool isIdle (int timeout = 60000);

	// Add a command to the end of the list
	void addCommand (const StdString &hostname, int tcpPort, Json *command, CommandList::InvokeCallback callback, void *callbackData);

	// Invoke an Authorize command against the target agent from the last executed command, using the next available secret that hasn't yet been tried. Returns a boolean value indicating if any new authorize attempt was made.
	bool authorizeLastCommand ();

	// Callback functions
	static void invokeComplete (void *commandListPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);
	static void authorizeComplete (void *commandListPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);

private:
	struct Token {
		StdString token;
		int authorizeSecretIndex;
		Token (): authorizeSecretIndex (0) { }
	};
	// Remove all items from contextList
	void clearContextList ();

	// Free the provided context struct
	void freeContext (CommandList::Context *ctx);

	// Execute actions appropriate when a command invoke operation completes
	void endInvoke (int invokeResult, Json *responseCommand);

	// Execute actions appropriate when an authorize invoke operation completes
	void endAuthorize (int invokeResult, Json *responseCommand);

	// Read the topmost command in the list and store its request data into the provided objects. Returns a boolean value indicating if a next command was found. This method should be invoked only while holding a lock on mutex.
	bool getNextCommand (StdString *url, StdString *postData);

	bool isInvoking;
	int64_t lastIdleTime;
	std::list<CommandList::Context *> contextList;
	std::map<StdString, CommandList::Token> tokenMap;
	SDL_mutex *mutex;
};

#endif
