/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "StdString.h"
#include "StringList.h"
#include "Json.h"
#include "SystemInterface.h"
#include "Agent.h"
#include "RecordStore.h"

RecordStore *RecordStore::instance = NULL;

RecordStore::RecordStore ()
: mutex (NULL)
{
	mutex = SDL_CreateMutex ();
}

RecordStore::~RecordStore () {
	clear ();
	if (mutex) {
		SDL_DestroyMutex (mutex);
		mutex = NULL;
	}
}

void RecordStore::clear () {
	std::map<StdString, Json *>::iterator i, end;

	lock ();
	i = recordMap.begin ();
	end = recordMap.end ();
	while (i != end) {
		if (i->second) {
			delete (i->second);
		}
		++i;
	}
	recordMap.clear ();
	unlock ();
}

void RecordStore::addRecord (Json *record, const StdString &recordId) {
	std::map<StdString, Json *>::iterator pos;
	StdString id;
	Json *item;

	if (! record) {
		return;
	}
	if (! recordId.empty ()) {
		id.assign (recordId);
	}
	else {
		id = SystemInterface::instance->getCommandRecordId (record);
		if (id.empty ()) {
			return;
		}
	}

	item = new Json ();
	item->copyValue (record);
	lock ();
	pos = recordMap.find (id);
	if (pos != recordMap.end ()) {
		if (pos->second) {
			delete (pos->second);
		}
		pos->second = item;
	}
	else {
		recordMap.insert (std::pair<StdString, Json *> (id, item));
	}
	unlock ();
}

void RecordStore::removeRecord (const StdString &recordId) {
	std::map<StdString, Json *>::iterator pos;

	lock ();
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		if (pos->second) {
			delete (pos->second);
		}
		recordMap.erase (pos);
	}
	unlock ();
}

void RecordStore::removeRecords (int commandId) {
	StringList idlist;
	std::map<StdString, Json *>::iterator i, iend, pos;
	StringList::iterator j, jend;

	lock ();
	i = recordMap.begin ();
	iend = recordMap.end ();
	while (i != iend) {
		if (SystemInterface::instance->getCommandId (i->second) == commandId) {
			idlist.push_back (i->first);
		}
		++i;
	}

	j = idlist.begin ();
	jend = idlist.end ();
	while (j != jend) {
		pos = recordMap.find (*j);
		if (pos != recordMap.end ()) {
			if (pos->second) {
				delete (pos->second);
			}
			recordMap.erase (pos);
		}
		++j;
	}
	unlock ();
}

void RecordStore::lock () {
	SDL_LockMutex (mutex);
}

void RecordStore::unlock () {
	SDL_UnlockMutex (mutex);
}

Json *RecordStore::findRecord (const StdString &recordId, int recordType) {
	std::map<StdString, Json *>::iterator pos;
	
	pos = recordMap.find (recordId);
	if (pos == recordMap.end ()) {
		return (NULL);
	}
	if (SystemInterface::instance->getCommandId (pos->second) != recordType) {
		return (NULL);
	}
	return (pos->second);
}

Json *RecordStore::findRecord (RecordStore::FindMatchFunction matchFn, void *matchData) {
	std::map<StdString, Json *>::iterator i, end;

	i = recordMap.begin ();
	end = recordMap.end ();
	while (i != end) {
		if (matchFn (matchData, i->second)) {
			return (i->second);
		}
		++i;
	}
	return (NULL);
}

void RecordStore::findRecords (RecordStore::FindMatchFunction matchFn, void *matchData, std::list<Json *> *destList, bool shouldClear) {
	std::map<StdString, Json *>::iterator i, end;

	if (shouldClear) {
		destList->clear ();
	}
	i = recordMap.begin ();
	end = recordMap.end ();
	while (i != end) {
		if (matchFn (matchData, i->second)) {
			destList->push_back (i->second);
		}
		++i;
	}
}

void RecordStore::processRecords (RecordStore::FindMatchFunction matchFn, void *matchData, RecordStore::ProcessAgentRecordFunction processFn, void *processFnData) {
	std::map<StdString, Json *>::iterator i, end;

	i = recordMap.begin ();
	end = recordMap.end ();
	while (i != end) {
		if (matchFn (matchData, i->second)) {
			processFn (processFnData, i->second, i->first);
		}
		++i;
	}
}

void RecordStore::processAgentRecords (const char *agentStatusFieldName, RecordStore::ProcessAgentRecordFunction processFn, void *processFnData) {
	std::list<Json *> records;
	std::list<Json *>::iterator i, end;
	StdString fieldname, agentid;
	Json *record;

	fieldname.assign (agentStatusFieldName);
	findRecords (RecordStore::matchAgentStatusObjectExists, &fieldname, &records, true);
	i = records.begin ();
	end = records.end ();
	while (i != end) {
		record = *i;
		++i;

		agentid = SystemInterface::instance->getCommandStringParam (record, "id", "");
		if (agentid.empty ()) {
			continue;
		}
		processFn (processFnData, record, agentid);
	}
}

void RecordStore::processAgentRecords (const StdString &agentStatusFieldName, RecordStore::ProcessAgentRecordFunction processFn, void *processFnData) {
	RecordStore::processAgentRecords (agentStatusFieldName.c_str (), processFn, processFnData);
}

void RecordStore::processCommandRecords (int commandId, RecordStore::ProcessRecordFunction processFn, void *processFnData) {
	std::list<Json *> records;
	std::list<Json *>::iterator i, end;
	StdString fieldname, recordid;
	Json *record;

	findRecords (RecordStore::matchCommandId, &commandId, &records, true);
	i = records.begin ();
	end = records.end ();
	while (i != end) {
		record = *i;
		++i;

		recordid = SystemInterface::instance->getCommandRecordId (record);
		if (recordid.empty ()) {
			recordid = SystemInterface::instance->getCommandStringParam (record, "id", "");
		}
		if (recordid.empty ()) {
			continue;
		}
		processFn (processFnData, record, recordid);
	}
}

void RecordStore::populateAgentMap (HashMap *destMap, const StdString &statusFieldName) {
	RecordStore::populateAgentMap (destMap, statusFieldName.c_str ());
}

void RecordStore::populateAgentMap (HashMap *destMap, const char *statusFieldName) {
	std::list<Json *> records;
	std::list<Json *>::iterator i, end;
	StdString fieldname, agentid, agentname;

	fieldname.assign (statusFieldName);
	findRecords (RecordStore::matchAgentStatusObjectExists, &fieldname, &records, true);
	destMap->clear ();
	i = records.begin ();
	end = records.end ();
	while (i != end) {
		agentid = SystemInterface::instance->getCommandAgentId (*i);
		agentname = Agent::getCommandAgentName (*i);
		if ((! agentid.empty () && (! agentname.empty ()))) {
			destMap->insert (agentname, agentid);
		}
		++i;
	}
}

int RecordStore::countAgentRecords (const char *agentStatusFieldName) {
	StdString fieldname;
	std::list<Json *> records;

	fieldname.assign (agentStatusFieldName);
	findRecords (RecordStore::matchAgentStatusObjectExists, &fieldname, &records);
	return ((int) records.size ());
}

int RecordStore::countAgentRecords (const StdString &agentStatusFieldName) {
	return (countAgentRecords (agentStatusFieldName.c_str ()));
}

int RecordStore::countCommandRecords (int commandId) {
	std::list<Json *> records;
	int id;

	id = commandId;
	findRecords (RecordStore::matchCommandId, &id, &records);
	return ((int) records.size ());
}

bool RecordStore::matchCommandId (void *intPtr, Json *record) {
	int *type;

	type = (int *) intPtr;
	return (SystemInterface::instance->getCommandId (record) == *type);
}

bool RecordStore::matchAgentStatusSource (void *agentIdStringPtr, Json *record) {
	StdString *agentid;

	if (SystemInterface::instance->getCommandId (record) != SystemInterface::CommandId_AgentStatus) {
		return (false);
	}
	agentid = (StdString *) agentIdStringPtr;
	return (agentid->equals (SystemInterface::instance->getCommandAgentId (record)));
}

bool RecordStore::matchAgentStatusObjectExists (void *objectFieldNameStringPtr, Json *record) {
	StdString *fieldname;

	if (SystemInterface::instance->getCommandId (record) != SystemInterface::CommandId_AgentStatus) {
		return (false);
	}
	fieldname = (StdString *) objectFieldNameStringPtr;
	return (SystemInterface::instance->getCommandObjectParam (record, fieldname->c_str (), NULL));
}
