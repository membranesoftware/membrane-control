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
#include <map>
#include "SDL2/SDL.h"
#include "App.h"
#include "Log.h"
#include "OsUtil.h"
#include "StdString.h"
#include "StringList.h"
#include "HashMap.h"
#include "Json.h"
#include "UiText.h"
#include "Agent.h"
#include "AgentControl.h"
#include "SystemInterface.h"
#include "WebKioskUi.h"
#include "WebPlaylistWindow.h"
#include "MediaUi.h"
#include "StreamPlaylistWindow.h"
#include "CommandHistory.h"

CommandHistory *CommandHistory::instance = NULL;

const char *CommandHistory::CommandHistoryKey = "CommandLog";
const char *CommandHistory::MaxCommandHistorySizeKey = "CommandLogSize";
const char *CommandHistory::CommandIdKey = "a";
const char *CommandHistory::CommandTimeKey = "b";
const char *CommandHistory::DetailTextKey = "c";
const char *CommandHistory::StatusKey = "d";
const char *CommandHistory::IsExecutableKey = "e";
const char *CommandHistory::IsSavedKey = "f";
const char *CommandHistory::CommandParamsKey = "g";
const char *CommandHistory::AgentIdKey = "h";
const char *CommandHistory::InvokeHostnameKey = "i";
const char *CommandHistory::InvokeTcpPort1Key = "j";
const char *CommandHistory::DisplayNameKey = "k";
const char *CommandHistory::AgentListKey = "l";
const char *CommandHistory::StreamUrlKey = "m";
const char *CommandHistory::StreamIdKey = "n";
const char *CommandHistory::StartPositionKey = "o";
const char *CommandHistory::ThumbnailUrlKey = "p";
const char *CommandHistory::MinStartPositionDeltaKey = "q";
const char *CommandHistory::MaxStartPositionDeltaKey = "r";
const char *CommandHistory::MinItemDisplayDurationKey = "s";
const char *CommandHistory::MaxItemDisplayDurationKey = "t";

const int defaultMaxCommandHistorySize = 100;

CommandHistory::CommandHistory ()
: activeCommandCount (0)
, mutex (NULL)
, maxSize (defaultMaxCommandHistorySize)
{
	mutex = SDL_CreateMutex ();
}

CommandHistory::~CommandHistory () {
	if (mutex) {
		SDL_DestroyMutex (mutex);
		mutex = NULL;
	}
}

StdString CommandHistory::toString (const CommandHistory::Record &record) {
	StringList ids, names;

	getCommandAgentIdList (record, &ids);
	getCommandAgentNameList (record, &names);
	return (StdString::createSprintf ("<CommandHistory #%s commandId=%i commandTime=%lli name=\"%s\" detailText=\"%s\" status=%i isExecutable=%s isSaved=%s agentIconType=%i commandParams=%s agentIds=%s agentNames=%s commandCount=%i>", record.recordId.c_str (), record.commandId, (long long int) record.commandTime, record.name.c_str (), record.detailText.c_str (), record.status, BOOL_STRING (record.isExecutable), BOOL_STRING (record.isSaved), record.agentIconType, record.commandParams.toString ().c_str (), ids.toString ().c_str (), names.toString ().c_str (), record.commandCount));
}

void CommandHistory::getCommandAgentIdList (const CommandHistory::Record &record, StringList *destList) {
	CommandHistory::CommandAgentMap::const_iterator i, end;

	destList->clear ();
	i = record.agentMap.cbegin ();
	end = record.agentMap.cend ();
	while (i != end) {
		destList->push_back (i->second.agentId);
		++i;
	}
}

void CommandHistory::getCommandAgentNameList (const CommandHistory::Record &record, StringList *destList) {
	CommandHistory::CommandAgentMap::const_iterator i, end;

	destList->clear ();
	i = record.agentMap.cbegin ();
	end = record.agentMap.cend ();
	while (i != end) {
		destList->push_back (i->second.displayName);
		++i;
	}
}

void CommandHistory::clear () {
	SDL_LockMutex (mutex);
	recordMap.clear ();
	activeCommandCount = 0;
	SDL_UnlockMutex (mutex);
	writePrefs ();
}

void CommandHistory::clearUnsaved () {
	std::map<StdString, CommandHistory::Record>::iterator i, iend;
	StringList idlist;
	StringList::iterator j, jend;

	SDL_LockMutex (mutex);
	i = recordMap.begin ();
	iend = recordMap.end ();
	while (i != iend) {
		if ((! i->second.isSaved) && (i->second.status != CommandHistory::InProgress)) {
			idlist.push_back (i->first);
		}
		++i;
	}
	j = idlist.begin ();
	jend = idlist.end ();
	while (j != jend) {
		recordMap.erase (*j);
		++j;
	}
	resetStats ();
	SDL_UnlockMutex (mutex);
	writePrefs ();
}

StdString CommandHistory::getCommandName (const StdString &recordId) {
	std::map<StdString, CommandHistory::Record>::iterator pos;
	StdString result;

	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		result.assign (pos->second.name);
	}
	SDL_UnlockMutex (mutex);
	return (result);
}

CommandHistory::CommandStatus CommandHistory::getCommandStatus (const StdString &recordId) {
	std::map<StdString, CommandHistory::Record>::iterator pos;
	CommandHistory::CommandStatus result;

	result = CommandHistory::Unknown;
	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		result = pos->second.status;
	}
	SDL_UnlockMutex (mutex);
	return (result);
}

void CommandHistory::setCommandStatus (const StdString &recordId, CommandHistory::CommandStatus status) {
	std::map<StdString, CommandHistory::Record>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		found = true;
		pos->second.status = status;
		resetStats ();
	}
	SDL_UnlockMutex (mutex);

	if (found) {
		writePrefs ();
	}
}

void CommandHistory::addCommandInvokeResult (const StdString &recordId, const StdString &agentId, bool success) {
	std::map<StdString, CommandHistory::Record>::iterator pos;
	CommandHistory::CommandAgentMap::iterator agent, i, end;
	bool found;
	int successcount, failcount;

	found = false;
	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		agent = pos->second.agentMap.find (agentId);
		if (agent != pos->second.agentMap.end ()) {
			found = true;
			if (success) {
				++(agent->second.successCount);
			}
			else {
				++(agent->second.failureCount);
			}

			successcount = 0;
			failcount = 0;
			i = pos->second.agentMap.begin ();
			end = pos->second.agentMap.end ();
			while (i != end) {
				successcount += i->second.successCount;
				failcount += i->second.failureCount;
				++i;
			}
			if ((successcount + failcount) < pos->second.commandCount) {
				pos->second.status = CommandHistory::InProgress;
			}
			else {
				i = pos->second.agentMap.begin ();
				end = pos->second.agentMap.end ();
				while (i != end) {
					if (i->second.successCount <= 0) {
						i->second.status = CommandHistory::Failed;
					}
					else if (i->second.failureCount > 0) {
						i->second.status = CommandHistory::CompleteWithError;
					}
					else {
						i->second.status = CommandHistory::Complete;
					}
					++i;
				}

				if (successcount <= 0) {
					pos->second.status = CommandHistory::Failed;
				}
				else if (failcount > 0) {
					pos->second.status = CommandHistory::CompleteWithError;
				}
				else {
					pos->second.status = CommandHistory::Complete;
				}
			}
			resetStats ();
		}
	}
	SDL_UnlockMutex (mutex);

	if (found) {
		writePrefs ();
	}
}

void CommandHistory::setCommandSaved (const StdString &recordId, bool saved) {
	std::map<StdString, CommandHistory::Record>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		found = true;
		pos->second.isSaved = saved;
	}
	SDL_UnlockMutex (mutex);

	if (found) {
		writePrefs ();
	}
}

void CommandHistory::processRecords (CommandHistory::ProcessRecordFunction processFn, void *processFnData) {
	std::map<StdString, CommandHistory::Record>::iterator i, end;

	SDL_LockMutex (mutex);
	i = recordMap.begin ();
	end = recordMap.end ();
	while (i != end) {
		processFn (processFnData, &(i->second));
		++i;
	}
	SDL_UnlockMutex (mutex);
}

StdString CommandHistory::initRecord (CommandHistory::Record *record, int commandId, const StringList &agentIdList, int commandCount) {
	StringList::const_iterator i, end;
	StdString agentname;
	CommandHistory::CommandAgent agent;
	CommandHistory::CommandAgentMap::iterator pos;
	Agent a;

	record->recordId.sprintf ("%016llx", (long long int) App::instance->getUniqueId ());
	record->commandId = commandId;
	record->commandTime = OsUtil::getTime ();
	record->name.assign (getCommandNameText (commandId));
	record->agentIconType = getCommandAgentIconType (commandId);
	record->status = CommandHistory::InProgress;
	record->commandCount = commandCount;

	record->agentMap.clear ();
	i = agentIdList.cbegin ();
	end = agentIdList.cend ();
	while (i != end) {
		pos = record->agentMap.find (*i);
		if (pos == record->agentMap.end ()) {
			if (AgentControl::instance->findTargetAgent (*i, &a) == OsUtil::Success) {
				agent = CommandHistory::CommandAgent ();
				agent.agentId.assign (a.id);
				agent.invokeHostname.assign (a.invokeHostname);
				agent.invokeTcpPort1 = a.invokeTcpPort1;
				agent.displayName.assign (a.displayName);
				agent.status = CommandHistory::InProgress;
				record->agentMap.insert (std::pair<StdString, CommandHistory::CommandAgent> (*i, agent));
			}
		}
		++i;
	}
	return (record->recordId);
}

void CommandHistory::insertRecord (const CommandHistory::Record &record) {
	SDL_LockMutex (mutex);
	recordMap.insert (std::pair<StdString, CommandHistory::Record> (record.recordId, record));
	compact ();
	resetStats ();
	SDL_UnlockMutex (mutex);
	writePrefs ();
}

void CommandHistory::readPrefs () {
	HashMap *prefs;
	JsonList items;
	JsonList::iterator i, end;
	CommandHistory::Record record;

	prefs = App::instance->lockPrefs ();
	prefs->find (CommandHistory::CommandHistoryKey, &items);
	maxSize = prefs->find (CommandHistory::MaxCommandHistorySizeKey, defaultMaxCommandHistorySize);
	App::instance->unlockPrefs ();

	SDL_LockMutex (mutex);
	recordMap.clear ();
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		if (readRecordState (&record, *i)) {
			recordMap.insert (std::pair<StdString, CommandHistory::Record> (record.recordId, record));
		}
		++i;
	}
	compact ();
	resetStats ();
	SDL_UnlockMutex (mutex);
}

void CommandHistory::writePrefs () {
	HashMap *prefs;
	std::map<StdString, CommandHistory::Record>::iterator i, end;
	JsonList items;

	SDL_LockMutex (mutex);
	i = recordMap.begin ();
	end = recordMap.end ();
	while (i != end) {
		items.push_back (createRecordState (i->second));
		++i;
	}
	SDL_UnlockMutex (mutex);

	prefs = App::instance->lockPrefs ();
	prefs->insert (CommandHistory::CommandHistoryKey, &items);
	App::instance->unlockPrefs ();
}

Json *CommandHistory::createRecordState (const CommandHistory::Record &record) {
	Json *state, *item;
	CommandHistory::CommandAgentMap::const_iterator i, end;
	JsonList items;

	state = new Json ();
	state->set (CommandHistory::CommandIdKey, record.commandId);
	state->set (CommandHistory::CommandTimeKey, record.commandTime);
	state->set (CommandHistory::DetailTextKey, record.detailText);
	state->set (CommandHistory::StatusKey, (int) record.status);
	state->set (CommandHistory::IsExecutableKey, record.isExecutable);
	state->set (CommandHistory::IsSavedKey, record.isSaved);
	state->set (CommandHistory::CommandParamsKey, record.commandParams.toJson ());

	i = record.agentMap.cbegin ();
	end = record.agentMap.cend ();
	while (i != end) {
		item = new Json ();
		item->set (CommandHistory::AgentIdKey, i->second.agentId);
		item->set (CommandHistory::InvokeHostnameKey, i->second.invokeHostname);
		item->set (CommandHistory::InvokeTcpPort1Key, i->second.invokeTcpPort1);
		item->set (CommandHistory::DisplayNameKey, i->second.displayName);
		item->set (CommandHistory::StatusKey, (int) i->second.status);
		items.push_back (item);
		++i;
	}
	state->set (CommandHistory::AgentListKey, &items);

	return (state);
}

bool CommandHistory::readRecordState (CommandHistory::Record *record, Json *state) {
	Json json, item;
	CommandHistory::CommandAgent agent;
	int i, count;

	*record = CommandHistory::Record ();
	record->commandId = state->getNumber (CommandHistory::CommandIdKey, -1);
	if (record->commandId < 0) {
		return (false);
	}
	record->name.assign (getCommandNameText (record->commandId));
	record->agentIconType = getCommandAgentIconType (record->commandId);
	record->commandTime = state->getNumber (CommandHistory::CommandTimeKey, (int64_t) 0);
	record->detailText = state->getString (CommandHistory::DetailTextKey, "");

	record->status = (CommandHistory::CommandStatus) state->getNumber (CommandHistory::StatusKey, (int) CommandHistory::Failed);
	if (record->status == CommandHistory::InProgress) {
		record->status = CommandHistory::Failed;
	}

	record->isExecutable = state->getBoolean (CommandHistory::IsExecutableKey, false);
	record->isSaved = state->getBoolean (CommandHistory::IsSavedKey, false);
	if (state->getObject (CommandHistory::CommandParamsKey, &json)) {
		record->commandParams.readJson (&json);
	}

	record->agentMap.clear ();
	count = state->getArrayLength (CommandHistory::AgentListKey);
	for (i = 0; i < count; ++i) {
		if (state->getArrayObject (CommandHistory::AgentListKey, i, &item)) {
			agent.agentId = item.getString (CommandHistory::AgentIdKey, "");
			if (! agent.agentId.empty ()) {
				agent.invokeHostname = item.getString (CommandHistory::InvokeHostnameKey, "");
				agent.invokeTcpPort1 = item.getNumber (CommandHistory::InvokeTcpPort1Key, 0);
				agent.displayName = item.getString (CommandHistory::DisplayNameKey, "");
				agent.status = (CommandHistory::CommandStatus) item.getNumber (CommandHistory::StatusKey, CommandHistory::Failed);
				record->agentMap.insert (std::pair<StdString, CommandHistory::CommandAgent> (agent.agentId, agent));
			}
		}
	}

	record->recordId.sprintf ("%016llx", (long long int) App::instance->getUniqueId ());
	return (true);
}

void CommandHistory::resetStats () {
	std::map<StdString, CommandHistory::Record>::iterator i, end;
	int activecount;

	activecount = 0;
	i = recordMap.begin ();
	end = recordMap.end ();
	while (i != end) {
		if (i->second.status == CommandHistory::InProgress) {
			++activecount;
		}
		++i;
	}
	activeCommandCount = activecount;
}

void CommandHistory::compact () {
	std::map<StdString, CommandHistory::Record>::iterator i, end, pos;
	int count;

	if (maxSize < 0) {
		return;
	}
	count = (int) recordMap.size ();
	while (count > maxSize) {
		i = recordMap.begin ();
		end = recordMap.end ();
		pos = end;
		while (i != end) {
			if ((! i->second.isSaved) && (i->second.status != CommandHistory::InProgress)) {
				if (pos == end) {
					pos = i;
				}
				else {
					if (i->second.commandTime < pos->second.commandTime) {
						pos = i;
					}
				}
			}
			++i;
		}
		if (pos == end) {
			break;
		}
		recordMap.erase (pos);
		--count;
	}
}

StdString CommandHistory::getCommandNameText (int commandId) {
	switch (commandId) {
		case SystemInterface::CommandId_SetAdminSecret: {
			return (UiText::instance->getText (UiTextString::SetAdminPassword).capitalized ());
		}
		case SystemInterface::CommandId_AddMediaTag: {
			return (UiText::instance->getText (UiTextString::AddMediaTagCommandName));
		}
		case SystemInterface::CommandId_RemoveMediaTag: {
			return (UiText::instance->getText (UiTextString::RemoveMediaTagCommandName));
		}
		case SystemInterface::CommandId_ClearDisplay: {
			return (UiText::instance->getText (UiTextString::ClearDisplayCommandName));
		}
		case SystemInterface::CommandId_ShowWebUrl: {
			return (UiText::instance->getText (UiTextString::ShowWebUrlCommandName));
		}
		case SystemInterface::CommandId_CreateWebDisplayIntent: {
			return (UiText::instance->getText (UiTextString::CreateWebDisplayIntentCommandName));
		}
		case SystemInterface::CommandId_PlayMedia: {
			return (UiText::instance->getText (UiTextString::PlayMediaCommandName));
		}
		case SystemInterface::CommandId_PauseMedia: {
			return (UiText::instance->getText (UiTextString::PauseMediaCommandName));
		}
		case SystemInterface::CommandId_CreateMediaDisplayIntent: {
			return (UiText::instance->getText (UiTextString::CreateMediaDisplayIntentCommandName));
		}
		case SystemInterface::CommandId_ScanMediaItems: {
			return (UiText::instance->getText (UiTextString::ScanForMedia).capitalized ());
		}
		case SystemInterface::CommandId_ConfigureMediaStream: {
			return (UiText::instance->getText (UiTextString::ConfigureMediaStreamCommandName));
		}
		case SystemInterface::CommandId_CreateCacheStream: {
			return (UiText::instance->getText (UiTextString::CreateCacheStreamCommandName));
		}
		case SystemInterface::CommandId_RemoveStream: {
			return (UiText::instance->getText (UiTextString::RemoveStreamCommandName));
		}
		case SystemInterface::CommandId_RemoveMedia: {
			return (UiText::instance->getText (UiTextString::RemoveMediaCommandName));
		}
		case SystemInterface::CommandId_PlayCacheStream: {
			return (UiText::instance->getText (UiTextString::PlayCacheStreamCommandName));
		}
		case SystemInterface::CommandId_CreateStreamCacheDisplayIntent: {
			return (UiText::instance->getText (UiTextString::CreateStreamCacheDisplayIntentCommandName));
		}
		case SystemInterface::CommandId_ConfigureCamera: {
			return (UiText::instance->getText (UiTextString::ConfigureCamera).capitalized ());
		}
		case SystemInterface::CommandId_ClearTimelapse: {
			return (UiText::instance->getText (UiTextString::ClearTimelapse).capitalized ());
		}
		case SystemInterface::CommandId_ShowCameraImage: {
			return (UiText::instance->getText (UiTextString::ShowCameraImageCommandName));
		}
		case SystemInterface::CommandId_PlayCameraStream: {
			return (UiText::instance->getText (UiTextString::PlayCameraStreamCommandName));
		}
		case SystemInterface::CommandId_CreateCameraImageDisplayIntent: {
			return (UiText::instance->getText (UiTextString::CreateCameraImageDisplayIntentCommandName));
		}
	}
	return (StdString (""));
}

int CommandHistory::getCommandAgentIconType (int commandId) {
	switch (commandId) {
		case SystemInterface::CommandId_AddMediaTag:
		case SystemInterface::CommandId_RemoveMediaTag:
		case SystemInterface::CommandId_ScanMediaItems:
		case SystemInterface::CommandId_ConfigureMediaStream:
		case SystemInterface::CommandId_RemoveStream:
		case SystemInterface::CommandId_RemoveMedia: {
			return (UiConfiguration::SmallMediaIconSprite);
		}
		case SystemInterface::CommandId_ClearDisplay:
		case SystemInterface::CommandId_ShowWebUrl:
		case SystemInterface::CommandId_CreateWebDisplayIntent:
		case SystemInterface::CommandId_PlayMedia:
		case SystemInterface::CommandId_PauseMedia:
		case SystemInterface::CommandId_CreateMediaDisplayIntent:
		case SystemInterface::CommandId_CreateCacheStream:
		case SystemInterface::CommandId_PlayCacheStream:
		case SystemInterface::CommandId_CreateStreamCacheDisplayIntent:
		case SystemInterface::CommandId_ShowCameraImage:
		case SystemInterface::CommandId_PlayCameraStream:
		case SystemInterface::CommandId_CreateCameraImageDisplayIntent: {
			return (UiConfiguration::SmallDisplayIconSprite);
		}
		case SystemInterface::CommandId_ConfigureCamera:
		case SystemInterface::CommandId_ClearTimelapse: {
			return (UiConfiguration::SmallCameraIconSprite);
		}
	}
	return (UiConfiguration::SmallServerIconSprite);
}

OsUtil::Result CommandHistory::executeRepeatCommand (const StdString &recordId, StringList *agentIdList, Json **invokeCommand) {
	std::map<StdString, CommandHistory::Record>::iterator pos;
	CommandHistory::CommandAgentMap::iterator i, end;
	Json *command;
	OsUtil::Result result;

	if (invokeCommand) {
		*invokeCommand = NULL;
	}
	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos == recordMap.end ()) {
		result = OsUtil::KeyNotFoundError;
	}
	else if (! pos->second.isExecutable) {
		result = OsUtil::NotImplementedError;
	}
	else {
		result = createCommand (pos->second, &command);
		if (result == OsUtil::Success) {
			// TODO: Check if record agentMap entries exist as connected agents

			if (agentIdList) {
				agentIdList->clear ();
			}
			i = pos->second.agentMap.begin ();
			end = pos->second.agentMap.end ();
			while (i != end) {
				i->second.status = CommandHistory::InProgress;
				if (agentIdList) {
					agentIdList->push_back (i->second.agentId);
				}
				++i;
			}

			pos->second.status = CommandHistory::InProgress;
			pos->second.commandTime = OsUtil::getTime ();

			if (invokeCommand) {
				*invokeCommand = command;
			}
			else {
				delete (command);
			}
		}
	}
	SDL_UnlockMutex (mutex);

	return (result);
}

OsUtil::Result CommandHistory::createCommand (const CommandHistory::Record &record, Json **destJson) {
	Json *params, *cmd;
	JsonList items;
	JsonList::iterator i, end;
	HashMap *prefs;
	WebPlaylistWindow *webplaylist;
	StreamPlaylistWindow *streamplaylist;

	*destJson = NULL;
	cmd = NULL;
	params = NULL;
	switch (record.commandId) {
		case SystemInterface::CommandId_ClearDisplay: {
			params = new Json ();
			break;
		}
		case SystemInterface::CommandId_ShowWebUrl: {
			params = new Json ();
			params->set ("url", record.detailText);
			break;
		}
		case SystemInterface::CommandId_CreateWebDisplayIntent: {
			prefs = App::instance->lockPrefs ();
			prefs->find (WebKioskUi::PlaylistsKey, &items);
			App::instance->unlockPrefs ();

			i = items.begin ();
			end = items.end ();
			while (i != end) {
				webplaylist = new WebPlaylistWindow (NULL);
				webplaylist->readState (*i);
				if (webplaylist->playlistName.equals (record.detailText)) {
					cmd = webplaylist->createCommand ();
				}
				delete (webplaylist);
				if (cmd) {
					break;
				}
				++i;
			}
			if (! cmd) {
				return (OsUtil::ProgramNotFoundError);
			}
			break;
		}
		case SystemInterface::CommandId_PlayMedia: {
			params = new Json ();
			params->set ("mediaName", record.detailText);
			params->set ("streamUrl", record.commandParams.find (CommandHistory::StreamUrlKey, ""));
			params->set ("streamId", record.commandParams.find (CommandHistory::StreamIdKey, ""));
			params->set ("startPosition", record.commandParams.find (CommandHistory::StartPositionKey, 0.0f));
			if (record.commandParams.exists (CommandHistory::ThumbnailUrlKey)) {
				params->set ("thumbnailUrl", record.commandParams.find (CommandHistory::ThumbnailUrlKey, ""));
			}
			break;
		}
		case SystemInterface::CommandId_PauseMedia: {
			params = new Json ();
			break;
		}
		case SystemInterface::CommandId_CreateMediaDisplayIntent: {
			prefs = App::instance->lockPrefs ();
			prefs->find (MediaUi::PlaylistsKey, &items);
			App::instance->unlockPrefs ();

			i = items.begin ();
			end = items.end ();
			while (i != end) {
				streamplaylist = new StreamPlaylistWindow (NULL);
				streamplaylist->readState (*i);
				if (streamplaylist->playlistName.equals (record.detailText)) {
					cmd = streamplaylist->createCommand ();
				}
				delete (streamplaylist);
				if (cmd) {
					break;
				}
				++i;
			}
			if (! cmd) {
				return (OsUtil::ProgramNotFoundError);
			}
			break;
		}
		case SystemInterface::CommandId_PlayCacheStream: {
			params = new Json ();
			params->set ("streamId", record.commandParams.find (CommandHistory::StreamIdKey, ""));
			params->set ("startPosition", record.commandParams.find (CommandHistory::StartPositionKey, 0.0f));
			break;
		}
		case SystemInterface::CommandId_CreateStreamCacheDisplayIntent: {
			params = new Json ();
			params->set ("displayName", UiText::instance->getText (UiTextString::Cache).capitalized ());
			params->set ("isShuffle", true);
			params->set ("minStartPositionDelta", record.commandParams.find (CommandHistory::MinStartPositionDeltaKey, 0));
			params->set ("maxStartPositionDelta", record.commandParams.find (CommandHistory::MaxStartPositionDeltaKey, 0));
			params->set ("minItemDisplayDuration", record.commandParams.find (CommandHistory::MinItemDisplayDurationKey, 0));
			params->set ("maxItemDisplayDuration", record.commandParams.find (CommandHistory::MaxItemDisplayDurationKey, 0));
			break;
		}
		case SystemInterface::CommandId_CreateCameraImageDisplayIntent: {
			// TODO: Implement this
			break;
		}
	}

	if (cmd) {
		*destJson = cmd;
		return (OsUtil::Success);
	}

	if (! params) {
		return (OsUtil::NotImplementedError);
	}
	cmd = App::instance->createCommand (record.commandId, params);
	if (! cmd) {
		return (OsUtil::InternalApplicationFailureError);
	}
	*destJson = cmd;
	return (OsUtil::Success);
}

StdString CommandHistory::setAdminSecret (const StringList &agentIdList) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_SetAdminSecret, agentIdList, (int) agentIdList.size ());
	insertRecord (record);
	return (id);
}

StdString CommandHistory::addMediaTag (const StringList &agentIdList, int commandCount, const StdString &mediaName) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_AddMediaTag, agentIdList, commandCount);
	record.detailText.assign (mediaName);
	insertRecord (record);
	return (id);
}

StdString CommandHistory::removeMediaTag (const StringList &agentIdList, int commandCount, const StdString &mediaName) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_RemoveMediaTag, agentIdList, commandCount);
	record.detailText.assign (mediaName);
	insertRecord (record);
	return (id);
}

StdString CommandHistory::clearDisplay (const StringList &agentIdList) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_ClearDisplay, agentIdList, (int) agentIdList.size ());
	record.isExecutable = true;
	insertRecord (record);
	return (id);
}

StdString CommandHistory::showWebUrl (const StringList &agentIdList, const StdString &url) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_ShowWebUrl, agentIdList, (int) agentIdList.size ());
	record.detailText.assign (url);
	record.isExecutable = true;
	insertRecord (record);
	return (id);
}

StdString CommandHistory::createWebDisplayIntent (const StringList &agentIdList, const StdString &playlistName) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_CreateWebDisplayIntent, agentIdList, (int) agentIdList.size ());
	record.detailText.assign (playlistName);
	record.isExecutable = true;
	insertRecord (record);
	return (id);
}

StdString CommandHistory::playMedia (const StringList &agentIdList, const StdString &mediaName, const StdString &streamUrl, const StdString &streamId, float startPosition, const StdString &thumbnailUrl) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_PlayMedia, agentIdList, (int) agentIdList.size ());
	record.detailText.assign (mediaName);
	record.commandParams.insert (CommandHistory::StreamUrlKey, streamUrl);
	record.commandParams.insert (CommandHistory::StreamIdKey, streamId);
	record.commandParams.insert (CommandHistory::StartPositionKey, startPosition);
	if (! thumbnailUrl.empty ()) {
		record.commandParams.insert (CommandHistory::ThumbnailUrlKey, thumbnailUrl);
	}
	record.isExecutable = true;
	insertRecord (record);
	return (id);
}

StdString CommandHistory::pauseMedia (const StringList &agentIdList) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_PauseMedia, agentIdList, (int) agentIdList.size ());
	record.isExecutable = true;
	insertRecord (record);
	return (id);
}

StdString CommandHistory::createMediaDisplayIntent (const StringList &agentIdList, const StdString &playlistName) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_CreateMediaDisplayIntent, agentIdList, (int) agentIdList.size ());
	record.detailText.assign (playlistName);
	record.isExecutable = true;
	insertRecord (record);
	return (id);
}

StdString CommandHistory::scanMediaItems (const StringList &agentIdList) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_ScanMediaItems, agentIdList, (int) agentIdList.size ());
	insertRecord (record);
	return (id);
}

StdString CommandHistory::configureMediaStream (const StringList &agentIdList, int commandCount, const StdString &mediaName) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_ConfigureMediaStream, agentIdList, commandCount);
	record.detailText.assign (mediaName);
	insertRecord (record);
	return (id);
}

StdString CommandHistory::createCacheStream (const StringList &agentIdList, int commandCount, const StdString &mediaName) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_CreateCacheStream, agentIdList, commandCount);
	record.detailText.assign (mediaName);
	insertRecord (record);
	return (id);
}

StdString CommandHistory::removeStream (const StringList &agentIdList, int commandCount, const StdString &mediaName) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_RemoveStream, agentIdList, commandCount);
	record.detailText.assign (mediaName);
	insertRecord (record);
	return (id);
}

StdString CommandHistory::removeMedia (const StringList &agentIdList, int commandCount, const StdString &mediaName) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_RemoveMedia, agentIdList, commandCount);
	record.detailText.assign (mediaName);
	insertRecord (record);
	return (id);
}

StdString CommandHistory::playCacheStream (const StringList &agentIdList, const StdString &mediaName, const StdString &streamId, float startPosition) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_PlayCacheStream, agentIdList, (int) agentIdList.size ());
	record.detailText.assign (mediaName);
	record.commandParams.insert (CommandHistory::StreamIdKey, streamId);
	record.commandParams.insert (CommandHistory::StartPositionKey, startPosition);
	record.isExecutable = true;
	insertRecord (record);
	return (id);
}

StdString CommandHistory::createStreamCacheDisplayIntent (const StringList &agentIdList, int minStartPositionDelta, int maxStartPositionDelta, int minItemDisplayDuration, int maxItemDisplayDuration) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_CreateStreamCacheDisplayIntent, agentIdList, (int) agentIdList.size ());
	record.commandParams.insert (CommandHistory::MinStartPositionDeltaKey, minStartPositionDelta);
	record.commandParams.insert (CommandHistory::MaxStartPositionDeltaKey, maxStartPositionDelta);
	record.commandParams.insert (CommandHistory::MinItemDisplayDurationKey, minItemDisplayDuration);
	record.commandParams.insert (CommandHistory::MaxItemDisplayDurationKey, maxItemDisplayDuration);
	record.isExecutable = true;
	insertRecord (record);
	return (id);
}

StdString CommandHistory::configureCamera (const StringList &agentIdList) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_ConfigureCamera, agentIdList, (int) agentIdList.size ());
	insertRecord (record);
	return (id);
}

StdString CommandHistory::clearTimelapse (const StringList &agentIdList) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_ClearTimelapse, agentIdList, (int) agentIdList.size ());
	insertRecord (record);
	return (id);
}

StdString CommandHistory::showCameraImage (const StringList &agentIdList) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_ShowCameraImage, agentIdList, (int) agentIdList.size ());
	insertRecord (record);
	return (id);
}

StdString CommandHistory::playCameraStream (const StringList &agentIdList) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_PlayCameraStream, agentIdList, (int) agentIdList.size ());
	insertRecord (record);
	return (id);
}

StdString CommandHistory::createCameraImageDisplayIntent (const StringList &agentIdList, const StdString &playlistName) {
	CommandHistory::Record record;
	StdString id;

	id = initRecord (&record, SystemInterface::CommandId_CreateCameraImageDisplayIntent, agentIdList, (int) agentIdList.size ());
	record.detailText.assign (playlistName);
	record.isExecutable = true;
	insertRecord (record);
	return (id);
}
