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
#include "Config.h"
#include <stdlib.h>
#include "App.h"
#include "Result.h"
#include "StdString.h"
#include "Json.h"
#include "SystemInterface.h"
#include "CommandRecord.h"

const char *CommandRecord::NameKey = "a";
const char *CommandRecord::TargetAgentIdsKey = "b";
const char *CommandRecord::CommandKey = "c";

CommandRecord::CommandRecord ()
: command (NULL)
{
}

CommandRecord::~CommandRecord () {
	if (command) {
		delete (command);
		command = NULL;
	}
}

StdString CommandRecord::toString () {
	StdString s;

	s.sprintf ("<CommandRecord name=\"%s\" targetAgentIds=", name.c_str ());
	s.append (targetAgentIds.toString ());
	if (command) {
		s.append (" command=");
		s.append (command->toString ());
	}
	s.append (">");

	return (s);
}

StdString CommandRecord::toPrefsJsonString () {
	StdString s;
	Json *obj;

	obj = new Json ();
	if (! name.empty ()) {
		obj->set (CommandRecord::NameKey, name);
	}
	obj->set (CommandRecord::TargetAgentIdsKey, &targetAgentIds);
	obj->set (CommandRecord::CommandKey, command->copy ());
	s = obj->toString ();
	delete (obj);

	return (s);
}

int CommandRecord::readPrefsJson (const StdString &prefsJson) {
	Json *obj, cmd;
	int result;

	obj = new Json ();
	result = Result::Success;
	if (! obj->parse (prefsJson)) {
		result = Result::JsonParseFailedError;
	}
	else {
		name = obj->getString (CommandRecord::NameKey, "");

		targetAgentIds.clear ();
		obj->getStringList (CommandRecord::TargetAgentIdsKey, &targetAgentIds);

		if (obj->getObject (CommandRecord::CommandKey, &cmd)) {
			if (command) {
				delete (command);
			}
			command = cmd.copy ();
		}
	}
	delete (obj);

	return (result);
}

void CommandRecord::copyValue (CommandRecord *commandRecord) {
	name.assign (commandRecord->name);
	targetAgentIds.clear ();
	targetAgentIds.insertStringList (&(commandRecord->targetAgentIds));
	if (command) {
		delete (command);
	}
	command = commandRecord->command->copy ();
}
