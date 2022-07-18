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
// Class that accepts callback functions for invocation when commands are received

#ifndef COMMAND_LISTENER_H
#define COMMAND_LISTENER_H

#include <list>
#include <map>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "Json.h"

class CommandListener {
public:
	CommandListener ();
	~CommandListener ();
	static CommandListener *instance;

	typedef void (*CommandCallback) (void *data, const StdString &agentId, Json *command);
	struct CommandCallbackContext {
		CommandListener::CommandCallback callback;
		void *callbackData;
		CommandCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		CommandCallbackContext (CommandListener::CommandCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};
	// Add a callback that should be executed when a command with commandId is received
	void subscribe (int commandId, const CommandListener::CommandCallbackContext &context);

	// Remove all previously added callbacks matching commandId and context
	void unsubscribe (int commandId, const CommandListener::CommandCallbackContext &context);

	// Remove all previously added callbacks for which callbackData matches the stored context.callbackData value
	void unsubscribeContext (void *callbackData);

	// Invoke subscribed callbacks as appropriate for a received command
	void emit (const StdString &agentId, Json *command);

private:
	typedef std::list<CommandListener::CommandCallbackContext> CallbackList;
	std::map<int, CommandListener::CallbackList *> commandMap;
	SDL_mutex *commandMapMutex;
};

#endif
