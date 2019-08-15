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
// Class that holds information about a stored command record

#ifndef COMMAND_RECORD_H
#define COMMAND_RECORD_H

#include "StringList.h"
#include "Json.h"

class CommandRecord {
public:
	CommandRecord ();
	~CommandRecord ();

	// Read-write data members
	StdString name;
	StringList targetAgentIds;
	Json *command;

	// Return a string representation of the command record
	StdString toString ();

	// Return a JSON string containing the command record's data, suitable for storage in application preferences
	StdString toPrefsJsonString ();

	// Update the command record's fields with information from the provided prefs JSON string. Returns a Result value.
	int readPrefsJson (const StdString &prefsJson);

	// Replace the command record's fields with copies of fields from another record
	void copyValue (CommandRecord *commandRecord);

private:
	// Constants to use as object field names
	static const char *NameKey;
	static const char *TargetAgentIdsKey;
	static const char *CommandKey;
};

#endif
