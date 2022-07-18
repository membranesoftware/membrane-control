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
// Class that stores a history log of executed commands

#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H

#include <map>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "StringList.h"
#include "HashMap.h"
#include "OsUtil.h"
#include "Json.h"

class CommandHistory {
public:
	CommandHistory ();
	~CommandHistory ();
	static CommandHistory *instance;

	// Prefs keys
	static const char *CommandHistoryKey;
	static const char *MaxCommandHistorySizeKey;

	enum CommandStatus {
		Unknown = 0,
		InProgress = 1,
		Complete = 2,
		CompleteWithError = 3,
		Failed = 4
	};

	struct CommandAgent {
		StdString agentId;
		StdString invokeHostname;
		int invokeTcpPort1;
		StdString displayName;
		CommandHistory::CommandStatus status;
		int successCount;
		int failureCount;
		CommandAgent ():
			invokeTcpPort1 (-1),
			status (CommandHistory::Unknown),
			successCount (0),
			failureCount (0) { }
	};
	typedef std::map<StdString, CommandHistory::CommandAgent> CommandAgentMap;
	struct Record {
		StdString recordId;
		int commandId;
		int64_t commandTime;
		StdString name;
		StdString detailText;
		bool isExecutable;
		bool isSaved;
		CommandHistory::CommandAgentMap agentMap;
		HashMap commandParams;
		int agentIconType;
		CommandHistory::CommandStatus status;
		int commandCount;
		Record ():
			commandId (-1),
			commandTime (0),
			isExecutable (false),
			isSaved (false),
			agentIconType (-1),
			status (CommandHistory::Unknown),
			commandCount (0) { }
	};

	// Read-only data members
	int activeCommandCount;

	// Add a new record containing details from an executed command or set of commands
	StdString setAdminSecret (const StringList &agentIdList);
	StdString addMediaTag (const StringList &agentIdList, int commandCount, const StdString &mediaName);
	StdString removeMediaTag (const StringList &agentIdList, int commandCount, const StdString &mediaName);
	StdString clearDisplay (const StringList &agentIdList);
	StdString showWebUrl (const StringList &agentIdList, const StdString &url);
	StdString createWebDisplayIntent (const StringList &agentIdList, const StdString &playlistName);
	StdString playMedia (const StringList &agentIdList, const StdString &mediaName, const StdString &streamUrl, const StdString &streamId, float startPosition, const StdString &thumbnailUrl);
	StdString pauseMedia (const StringList &agentIdList);
	StdString createMediaDisplayIntent (const StringList &agentIdList, const StdString &playlistName);
	StdString scanMediaItems (const StringList &agentIdList);
	StdString configureMediaStream (const StringList &agentIdList, int commandCount, const StdString &mediaName);
	StdString createCacheStream (const StringList &agentIdList, int commandCount, const StdString &mediaName);
	StdString removeStream (const StringList &agentIdList, int commandCount, const StdString &mediaName);
	StdString removeMedia (const StringList &agentIdList, int commandCount, const StdString &mediaName);
	StdString playCacheStream (const StringList &agentIdList, const StdString &mediaName, const StdString &streamId, float startPosition);
	StdString createStreamCacheDisplayIntent (const StringList &agentIdList, int minStartPositionDelta, int maxStartPositionDelta, int minItemDisplayDuration, int maxItemDisplayDuration);
	StdString configureCamera (const StringList &agentIdList);
	StdString clearTimelapse (const StringList &agentIdList);
	StdString showCameraImage (const StringList &agentIdList);
	StdString playCameraStream (const StringList &agentIdList);
	StdString createCameraImageDisplayIntent (const StringList &agentIdList, const StdString &playlistName);

	// Prepare a repeat invocation of a command record, reset its status to InProgress, store invoke fields in agentIdList and invokeCommand, and return an OsUtil::Result value
	OsUtil::Result executeRepeatCommand (const StdString &recordId, StringList *agentIdList, Json **invokeCommand);

	// Remove all command records
	void clear ();

	// Remove all unsaved command records
	void clearUnsaved ();

	// Write an application preferences value containing command history records
	void writePrefs ();

	// Read previously cached command history data from preferences
	void readPrefs ();

	// Return the command name of the specified command record, or an empty string if no name was found
	StdString getCommandName (const StdString &recordId);

	// Return the status field for a command record
	CommandHistory::CommandStatus getCommandStatus (const StdString &recordId);

	// Set the status field for a command record
	void setCommandStatus (const StdString &recordId, CommandHistory::CommandStatus status);

	// Update a command record to reflect a received invoke result
	void addCommandInvokeResult (const StdString &recordId, const StdString &agentId, bool success);

	// Set the isSaved field for a command record
	void setCommandSaved (const StdString &recordId, bool saved);

	typedef void (*ProcessRecordFunction) (void *data, CommandHistory::Record *record);
	void processRecords (CommandHistory::ProcessRecordFunction processFn, void *processFnData = NULL);

	// Return a string representation of a CommandHistory record
	StdString toString (const CommandHistory::Record &record);

	// Clear destList and insert agentId strings from a CommandHistory record
	void getCommandAgentIdList (const CommandHistory::Record &record, StringList *destList);

	// Clear destList and insert agent display name strings from a CommandHistory record
	void getCommandAgentNameList (const CommandHistory::Record &record, StringList *destList);

private:
	// Object field names
	static const char *CommandIdKey;
	static const char *CommandTimeKey;
	static const char *DetailTextKey;
	static const char *StatusKey;
	static const char *IsExecutableKey;
	static const char *IsSavedKey;
	static const char *CommandParamsKey;
	static const char *AgentIdKey;
	static const char *InvokeHostnameKey;
	static const char *InvokeTcpPort1Key;
	static const char *DisplayNameKey;
	static const char *AgentListKey;
	static const char *StreamUrlKey;
	static const char *StreamIdKey;
	static const char *StartPositionKey;
	static const char *ThumbnailUrlKey;
	static const char *MinStartPositionDeltaKey;
	static const char *MaxStartPositionDeltaKey;
	static const char *MinItemDisplayDurationKey;
	static const char *MaxItemDisplayDurationKey;

	// Initialize fields in a command record and return the recordId value that was assigned
	StdString initRecord (CommandHistory::Record *record, int commandId, const StringList &agentIdList, int commandCount);

	// Insert a command record into recordMap
	void insertRecord (const CommandHistory::Record &record);

	// Return a newly created Json object containing command record fields
	Json *createRecordState (const CommandHistory::Record &record);

	// Read command records fields from a state object and return a boolean value indicating if the read was successful
	bool readRecordState (CommandHistory::Record *record, Json *state);

	// Reset command stats based on the contents of recordMap. This method must only be invoked while holding a lock on mutex.
	void resetStats ();

	// Remove command records as needed to prevent the item count from exceeding maxSize. This method must only be invoked while holding a lock on mutex.
	void compact ();

	// Return the text value for the command name associated with commandId, or -1 if no name text was found
	StdString getCommandNameText (int commandId);

	// Return the UiConfiguration string index for the agent icon type associated with commandId
	int getCommandAgentIconType (int commandId);

	// Create a Json object containing the repeat command for a command record, store it in destJson, and return an OsUtil::Result value
	OsUtil::Result createCommand (const CommandHistory::Record &record, Json **destJson);

	std::map<StdString, CommandHistory::Record> recordMap;
	SDL_mutex *mutex;
	int maxSize;
};

#endif
