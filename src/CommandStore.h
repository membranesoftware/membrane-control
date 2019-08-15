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
// Class that stores commands for repeat execution

#ifndef COMMAND_STORE_H
#define COMMAND_STORE_H

#include <map>
#include <vector>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "StringList.h"
#include "Json.h"
#include "CommandRecord.h"

class CommandStore {
public:
	CommandStore ();
	~CommandStore ();

	// Remove all items from the recent command map
	void clearRecentCommands ();

	// Remove all items from the stored command map
	void clearStoredCommands ();

	// Write an application preferences value containing stored command data
	void writePrefs ();

	// Read previously cached stored command data from preferences
	void readPrefs ();

	// Read the provided values and store them as a recent command entry if eligible
	void readRecentCommand (const StdString &targetAgentId, Json *command);
	void readRecentCommand (StringList *targetAgentIds, Json *command);

	// Clear the provided vector and insert all ID values from the recent command map
	void getRecentCommandIds (std::vector<int64_t> *destVector);

	// Return the number of items in the recent command map
	int getRecentCommandCount ();

	// Move an item from the recent command list to the stored command list. Returns a boolean value indicating if the operation was successful.
	bool storeRecentCommand (int64_t id);

	// Clear the provided vector and insert all ID values from the stored command map
	void getStoredCommandIds (std::vector<int64_t> *destVector);

	// Return the number of items in the stored command map
	int getStoredCommandCount ();

	// Find the command item matching the specified ID value and store copies of its data into the provided record. Returns a boolean value indicating if the item was found. If a command Json object is returned by this method, the caller is responsible for deleting it when it's no longer needed.
	bool getCommand (int64_t id, CommandRecord *commandRecord);

	// Remove the stored command with the specified ID value
	void removeStoredCommand (int64_t id);

	// Execute the stored command with the specified ID value. Returns the number of command invocations that were successfully queued.
	int executeStoredCommand (int64_t id);

	// Assign the provided name to the stored command with the specified ID value
	void setStoredCommandName (int64_t id, const StdString &commandName);

private:
	static const int MaxRecentCommands;

	std::map<int64_t, CommandRecord *> recentCommandMap;
	std::map<int64_t, CommandRecord *> storedCommandMap;
	SDL_mutex *commandMapMutex;
};

#endif
