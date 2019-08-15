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
#include "Config.h"
#include <stdlib.h>
#include <map>
#include <vector>
#include "SDL2/SDL.h"
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "Json.h"
#include "SystemInterface.h"
#include "CommandRecord.h"
#include "CommandStore.h"

const int CommandStore::MaxRecentCommands = 50;

CommandStore::CommandStore ()
: commandMapMutex (NULL)
{
	commandMapMutex = SDL_CreateMutex ();
}

CommandStore::~CommandStore () {
	clearRecentCommands ();
	clearStoredCommands ();

	if (commandMapMutex) {
		SDL_DestroyMutex (commandMapMutex);
		commandMapMutex = NULL;
	}
}

void CommandStore::clearRecentCommands () {
	std::map<int64_t, CommandRecord *>::iterator i, end;

	SDL_LockMutex (commandMapMutex);
	i = recentCommandMap.begin ();
	end = recentCommandMap.end ();
	while (i != end) {
		if (i->second) {
			delete (i->second);
			i->second = NULL;
		}
		++i;
	}
	recentCommandMap.clear ();
	SDL_UnlockMutex (commandMapMutex);
}

void CommandStore::clearStoredCommands () {
	std::map<int64_t, CommandRecord *>::iterator i, end;

	SDL_LockMutex (commandMapMutex);
	i = storedCommandMap.begin ();
	end = storedCommandMap.end ();
	while (i != end) {
		if (i->second) {
			delete (i->second);
			i->second = NULL;
		}
		++i;
	}
	storedCommandMap.clear ();
	SDL_UnlockMutex (commandMapMutex);
}

void CommandStore::writePrefs () {
	std::map<int64_t, CommandRecord *>::iterator i, end;
	StringList items;

	SDL_LockMutex (commandMapMutex);
	i = storedCommandMap.begin ();
	end = storedCommandMap.end ();
	while (i != end) {
		items.push_back (i->second->toPrefsJsonString ());
		++i;
	}
	SDL_UnlockMutex (commandMapMutex);

	if (items.empty ()) {
		App::instance->prefsMap.remove (App::StoredCommandsKey);
	}
	else {
		App::instance->prefsMap.insert (App::StoredCommandsKey, &items);
	}
}

void CommandStore::readPrefs () {
	StringList items;
	StringList::iterator i, end;
	CommandRecord *item;
	int64_t id;

	App::instance->prefsMap.find (App::StoredCommandsKey, &items);
	clearStoredCommands ();

	SDL_LockMutex (commandMapMutex);
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		item = new CommandRecord ();
		if (item->readPrefsJson (*i) != Result::Success) {
			delete (item);
		}
		else {
			id = App::instance->getUniqueId ();
			storedCommandMap.insert (std::pair<int64_t, CommandRecord *> (id, item));
		}
		++i;
	}
	SDL_UnlockMutex (commandMapMutex);
}

void CommandStore::readRecentCommand (const StdString &targetAgentId, Json *command) {
	StringList idlist;

	idlist.push_back (targetAgentId);
	readRecentCommand (&idlist, command);
}

void CommandStore::readRecentCommand (StringList *targetAgentIds, Json *command) {
	SystemInterface *interface;
	CommandRecord *item;
	std::map<int64_t, CommandRecord *>::iterator i, iend;
	std::list<int64_t> keys;
	std::list<int64_t>::iterator j, jend;
	StringList idlist;
	Json params1, params2;
	int64_t id;
	int cmd, removecount;
	bool shouldstore;

	if (targetAgentIds->empty () || (! command)) {
		return;
	}

	interface = &(App::instance->systemInterface);
	shouldstore = false;
	cmd = interface->getCommandId (command);
	switch (cmd) {
		case SystemInterface::CommandId_ClearDisplay:
		case SystemInterface::CommandId_PlayMedia:
		case SystemInterface::CommandId_ShowWebUrl:
		case SystemInterface::CommandId_CreateMediaDisplayIntent:
		case SystemInterface::CommandId_CreateWebDisplayIntent: {
			idlist.insertStringList (targetAgentIds);
			idlist.sort (StringList::compareAscending);
			shouldstore = true;
			break;
		}
		default: {
			break;
		}
	}

	if (shouldstore && interface->getCommandParams (command, &params1)) {
		SDL_LockMutex (commandMapMutex);
		i = storedCommandMap.begin ();
		iend = storedCommandMap.end ();
		while (i != iend) {
			if ((interface->getCommandId (i->second->command) == cmd) && i->second->targetAgentIds.equals (&idlist)) {
				if (interface->getCommandParams (i->second->command, &params2) && params1.deepEquals (&params2)) {
					shouldstore = false;
					break;
				}
			}
			++i;
		}

		if (shouldstore) {
			i = recentCommandMap.begin ();
			iend = recentCommandMap.end ();
			while (i != iend) {
				if ((interface->getCommandId (i->second->command) == cmd) && i->second->targetAgentIds.equals (&idlist)) {
					if (interface->getCommandParams (i->second->command, &params2) && params1.deepEquals (&params2)) {
						shouldstore = false;
						break;
					}
				}
				++i;
			}
		}
		SDL_UnlockMutex (commandMapMutex);
	}

	if (shouldstore) {
		id = App::instance->getUniqueId ();
		item = new CommandRecord ();
		item->targetAgentIds.insertStringList (&idlist);
		item->command = command->copy ();

		SDL_LockMutex (commandMapMutex);
		recentCommandMap.insert (std::pair<int64_t, CommandRecord *> (id, item));

		removecount = (int) recentCommandMap.size () - CommandStore::MaxRecentCommands;
		if (removecount > 0) {
			i = recentCommandMap.begin ();
			iend = recentCommandMap.end ();
			while (i != iend) {
				if (removecount <= 0) {
					break;
				}
				keys.push_back (i->first);
				--removecount;
				++i;
			}

			j = keys.begin ();
			jend = keys.end ();
			while (j != jend) {
				i = recentCommandMap.find (*j);
				if (i != recentCommandMap.end ()) {
					if (i->second) {
						delete (i->second);
						i->second = NULL;
					}
					recentCommandMap.erase (i);
				}
				++j;
			}
		}
		SDL_UnlockMutex (commandMapMutex);
	}
}

void CommandStore::getRecentCommandIds (std::vector<int64_t> *destVector) {
	std::map<int64_t, CommandRecord *>::reverse_iterator i, end;

	destVector->clear ();
	SDL_LockMutex (commandMapMutex);
	i = recentCommandMap.rbegin ();
	end = recentCommandMap.rend ();
	while (i != end) {
		destVector->push_back (i->first);
		++i;
	}
	SDL_UnlockMutex (commandMapMutex);
}

int CommandStore::getRecentCommandCount () {
	int count;

	SDL_LockMutex (commandMapMutex);
	count = (int) recentCommandMap.size ();
	SDL_UnlockMutex (commandMapMutex);

	return (count);
}

bool CommandStore::storeRecentCommand (int64_t id) {
	std::map<int64_t, CommandRecord *>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (commandMapMutex);
	pos = recentCommandMap.find (id);
	if (pos != recentCommandMap.end ()) {
		found = true;
		storedCommandMap.insert (std::pair<int64_t, CommandRecord *> (pos->first, pos->second));
		recentCommandMap.erase (pos);
	}
	SDL_UnlockMutex (commandMapMutex);

	if (found) {
		writePrefs ();
	}

	return (found);
}

void CommandStore::getStoredCommandIds (std::vector<int64_t> *destVector) {
	std::map<int64_t, CommandRecord *>::iterator i, end;

	destVector->clear ();
	SDL_LockMutex (commandMapMutex);
	i = storedCommandMap.begin ();
	end = storedCommandMap.end ();
	while (i != end) {
		destVector->push_back (i->first);
		++i;
	}
	SDL_UnlockMutex (commandMapMutex);
}

int CommandStore::getStoredCommandCount () {
	int count;

	SDL_LockMutex (commandMapMutex);
	count = (int) storedCommandMap.size ();
	SDL_UnlockMutex (commandMapMutex);

	return (count);
}

bool CommandStore::getCommand (int64_t id, CommandRecord *commandRecord) {
	std::map<int64_t, CommandRecord *>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (commandMapMutex);
	pos = recentCommandMap.find (id);
	if (pos != recentCommandMap.end ()) {
		found = true;
	}
	else {
		pos = storedCommandMap.find (id);
		if (pos != storedCommandMap.end ()) {
			found = true;
		}
	}

	if (found) {
		if (commandRecord) {
			commandRecord->copyValue (pos->second);
		}
	}
	SDL_UnlockMutex (commandMapMutex);

	return (found);
}

void CommandStore::removeStoredCommand (int64_t id) {
	std::map<int64_t, CommandRecord *>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (commandMapMutex);
	pos = storedCommandMap.find (id);
	if (pos != storedCommandMap.end ()) {
		found = true;
		if (pos->second) {
			delete (pos->second);
			pos->second = NULL;
		}
		storedCommandMap.erase (pos);
	}
	SDL_UnlockMutex (commandMapMutex);

	if (found) {
		writePrefs ();
	}
}

int CommandStore::executeStoredCommand (int64_t id) {
	std::map<int64_t, CommandRecord *>::iterator pos;
	StringList idlist;
	Json *command;
	int count;

	count = 0;
	command = NULL;
	SDL_LockMutex (commandMapMutex);
	pos = storedCommandMap.find (id);
	if (pos != storedCommandMap.end ()) {
		idlist.insertStringList (&(pos->second->targetAgentIds));
		command = pos->second->command->copy ();
	}
	SDL_UnlockMutex (commandMapMutex);

	if ((! idlist.empty ()) && command) {
		count = App::instance->agentControl.invokeCommand (&idlist, command);
	}

	return (count);
}

void CommandStore::setStoredCommandName (int64_t id, const StdString &commandName) {
	std::map<int64_t, CommandRecord *>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (commandMapMutex);
	pos = storedCommandMap.find (id);
	if (pos != storedCommandMap.end ()) {
		found = true;
		pos->second->name.assign (commandName);
	}
	SDL_UnlockMutex (commandMapMutex);

	if (found) {
		writePrefs ();
	}
}
