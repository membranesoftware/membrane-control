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
#include <list>
#include <map>
#include "SDL2/SDL.h"
#include "App.h"
#include "Log.h"
#include "OsUtil.h"
#include "StdString.h"
#include "Json.h"
#include "SystemInterface.h"
#include "CommandListener.h"

CommandListener *CommandListener::instance = NULL;

CommandListener::CommandListener ()
: commandMapMutex (NULL)
{
	commandMapMutex = SDL_CreateMutex ();
}

CommandListener::~CommandListener () {
	std::map<int, CommandListener::CallbackList *>::iterator i, end;

	SDL_LockMutex (commandMapMutex);
	i = commandMap.begin ();
	end = commandMap.end ();
	while (i != end) {
		if (i->second) {
			delete (i->second);
			i->second = NULL;
		}
		++i;
	}
	commandMap.clear ();
	SDL_UnlockMutex (commandMapMutex);

	if (commandMapMutex) {
		SDL_DestroyMutex (commandMapMutex);
		commandMapMutex = NULL;
	}
}

void CommandListener::subscribe (int commandId, const CommandListener::CommandCallbackContext &context) {
	std::map<int, CommandListener::CallbackList *>::iterator pos;
	CommandListener::CallbackList *cblist;

	SDL_LockMutex (commandMapMutex);
	pos = commandMap.find (commandId);
	if (pos != commandMap.end ()) {
		cblist = pos->second;
	}
	else {
		cblist = new CommandListener::CallbackList ();
		commandMap.insert (std::pair<int, CommandListener::CallbackList *> (commandId, cblist));
	}
	cblist->push_back (context);
	SDL_UnlockMutex (commandMapMutex);
}

void CommandListener::unsubscribe (int commandId, const CommandListener::CommandCallbackContext &context) {
	std::map<int, CommandListener::CallbackList *>::iterator pos;
	CommandListener::CallbackList *cblist;
	CommandListener::CallbackList::iterator i, end;
	bool found;

	SDL_LockMutex (commandMapMutex);
	pos = commandMap.find (commandId);
	if (pos != commandMap.end ()) {
		cblist = pos->second;
		while (true) {
			found = false;
			i = cblist->begin ();
			end = cblist->end ();
			while (i != end) {
				if ((i->callback == context.callback) && (i->callbackData == context.callbackData)) {
					cblist->erase (i);
					found = true;
					break;
				}
				++i;
			}
			if (! found) {
				break;
			}
		}
		if (cblist->empty ()) {
			delete (cblist);
			commandMap.erase (pos);
		}
	}
	SDL_UnlockMutex (commandMapMutex);
}

void CommandListener::unsubscribeContext (void *callbackData) {
	std::map<int, CommandListener::CallbackList *>::iterator mi, mend;
	CommandListener::CallbackList *cblist;
	CommandListener::CallbackList::iterator i, end;
	std::list<int> emptyids;
	std::list<int>::iterator j, jend;
	bool found;

	SDL_LockMutex (commandMapMutex);
	mi = commandMap.begin ();
	mend = commandMap.end ();
	while (mi != mend) {
		cblist = mi->second;
		while (true) {
			found = false;
			i = cblist->begin ();
			end = cblist->end ();
			while (i != end) {
				if (i->callbackData == callbackData) {
					cblist->erase (i);
					found = true;
					break;
				}
				++i;
			}
			if (! found) {
				break;
			}
		}
		if (cblist->empty ()) {
			emptyids.push_back (mi->first);
		}
		++mi;
	}

	j = emptyids.begin ();
	jend = emptyids.end ();
	while (j != jend) {
		mi = commandMap.find (*j);
		if (mi != commandMap.end ()) {
			delete (mi->second);
			mi->second = NULL;
			commandMap.erase (mi);
		}
		++j;
	}
	SDL_UnlockMutex (commandMapMutex);
}

void CommandListener::emit (const StdString &agentId, Json *command) {
	std::map<int, CommandListener::CallbackList *>::iterator pos;
	CommandListener::CallbackList *cblist;
	CommandListener::CallbackList::iterator i, end;
	int commandid;

	commandid = SystemInterface::instance->getCommandId (command);
	if (commandid < 0) {
		return;
	}

	SDL_LockMutex (commandMapMutex);
	pos = commandMap.find (commandid);
	if (pos != commandMap.end ()) {
		cblist = pos->second;
		i = cblist->begin ();
		end = cblist->end ();
		while (i != end) {
			if (i->callback) {
				i->callback (i->callbackData, agentId, command);
			}
			++i;
		}
	}
	SDL_UnlockMutex (commandMapMutex);
}
