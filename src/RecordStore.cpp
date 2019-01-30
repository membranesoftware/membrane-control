/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
*                 https://membranesoftware.com
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
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "StringList.h"
#include "App.h"
#include "Util.h"
#include "SystemInterface.h"
#include "Json.h"
#include "RecordStore.h"

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
	SystemInterface *interface;
	std::map<StdString, Json *>::iterator pos;
	StdString id;
	Json *item;

	interface = &(App::getInstance ()->systemInterface);
	if (! recordId.empty ()) {
		id.assign (recordId);
	}
	else {
		id = interface->getCommandRecordId (record);
		if (id.empty ()) {
			return;
		}
	}

	item = new Json ();
	item->copy (record);
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
	App *app;
	SystemInterface *interface;
	StringList idlist;
	std::map<StdString, Json *>::iterator i, iend, pos;
	StringList::iterator j, jend;

	app = App::getInstance ();
	interface = &(app->systemInterface);

	lock ();

	i = recordMap.begin ();
	iend = recordMap.end ();
	while (i != iend) {
		if (interface->getCommandId (i->second) == commandId) {
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

void RecordStore::compact () {
	std::map<StdString, Json *>::iterator i, iend;
	std::list<StdString> idlist;
	std::list<StdString>::iterator j, jend;
	SystemInterface *interface;

	interface = &(App::getInstance ()->systemInterface);
	i = recordMap.begin ();
	iend = recordMap.end ();
	while (i != iend) {
		if (interface->isRecordClosed (i->second)) {
			idlist.push_back (i->first);
		}
		++i;
	}
	if (idlist.empty ()) {
		return;
	}

	j = idlist.begin ();
	jend = idlist.end ();
	while (j != jend) {
		i = recordMap.find (*j);
		if (i != recordMap.end ()) {
			if (i->second) {
				delete (i->second);
			}
			recordMap.erase (i);
		}
		++j;
	}
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

	if (App::getInstance ()->systemInterface.getCommandId (pos->second) != recordType) {
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
	SystemInterface *interface;
	Json *record;

	interface = &(App::getInstance ()->systemInterface);
	fieldname.assign (agentStatusFieldName);
	findRecords (RecordStore::matchAgentStatusObjectExists, &fieldname, &records, true);
	i = records.begin ();
	end = records.end ();
	while (i != end) {
		record = *i;
		++i;

		agentid = interface->getCommandStringParam (record, "id", "");
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
	SystemInterface *interface;
	Json *record;

	interface = &(App::getInstance ()->systemInterface);
	findRecords (RecordStore::matchCommandId, &commandId, &records, true);
	i = records.begin ();
	end = records.end ();
	while (i != end) {
		record = *i;
		++i;

		recordid = interface->getCommandRecordId (record);
		if (recordid.empty ()) {
			recordid = interface->getCommandStringParam (record, "id", "");
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
	SystemInterface *interface;
	StdString fieldname, agentid, agentname;

	interface = &(App::getInstance ()->systemInterface);
	fieldname.assign (statusFieldName);
	findRecords (RecordStore::matchAgentStatusObjectExists, &fieldname, &records, true);
	destMap->clear ();
	i = records.begin ();
	end = records.end ();
	while (i != end) {
		agentid = interface->getCommandAgentId (*i);
		agentname = interface->getCommandAgentName (*i);
		if ((! agentid.empty () && (! agentname.empty ()))) {
			destMap->insert (agentname, agentid);
		}
		++i;
	}
}

int RecordStore::countRecords (RecordStore::FindMatchFunction matchFn, void *matchData, int64_t maxRecordAge) {
	std::map<StdString, Json *>::iterator i, end;
	int count;
	bool match, skip;

	count = 0;
	i = recordMap.begin ();
	end = recordMap.end ();
	while (i != end) {
		match = false;
		skip = false;

		// TODO: Check record age here

		if (! skip) {
			if (matchFn (matchData, i->second)) {
				match = true;
			}
		}

		if (match) {
			++count;
		}
		++i;
	}

	return (count);
}

int RecordStore::countAgentRecords (const char *agentStatusFieldName, int64_t maxRecordAge) {
	StdString fieldname;
	std::list<Json *> records;
	std::list<Json *>::iterator i, end;
	int count;

	fieldname.assign (agentStatusFieldName);
	findRecords (RecordStore::matchAgentStatusObjectExists, &fieldname, &records);

	if (maxRecordAge <= 0) {
		count = (int) records.size ();
	}
	else {
		count = 0;
		i = records.begin ();
		end = records.end ();
		while (i != end) {
			// TODO: Check record age here
			++count;
			++i;
		}
	}

	return (count);
}

int RecordStore::countAgentRecords (const StdString &agentStatusFieldName, int64_t maxRecordAge) {
	return (countAgentRecords (agentStatusFieldName.c_str (), maxRecordAge));
}

int RecordStore::countCommandRecords (int commandId, int64_t maxRecordAge) {
	std::list<Json *> records;
	std::list<Json *>::iterator i, end;
	int id, count;

	id = commandId;
	findRecords (RecordStore::matchCommandId, &id, &records);
	if (maxRecordAge <= 0) {
		return ((int) records.size ());
	}

	count = 0;
	i = records.begin ();
	end = records.end ();
	while (i != end) {
		// TODO: Check record age here
		++count;
		++i;
	}

	return (count);
}

bool RecordStore::matchCommandId (void *intPtr, Json *record) {
	int *type;

	type = (int *) intPtr;
	return (App::getInstance ()->systemInterface.getCommandId (record) == *type);
}

bool RecordStore::matchAgentStatusSource (void *agentIdStringPtr, Json *record) {
	StdString *agentid;
	SystemInterface *interface;

	interface = &(App::getInstance ()->systemInterface);
	if (interface->getCommandId (record) != SystemInterface::Command_AgentStatus) {
		return (false);
	}

	agentid = (StdString *) agentIdStringPtr;
	return (agentid->equals (interface->getCommandAgentId (record)));
}

bool RecordStore::matchAgentStatusObjectExists (void *objectFieldNameStringPtr, Json *record) {
	StdString *fieldname;
	SystemInterface *interface;

	interface = &(App::getInstance ()->systemInterface);
	if (interface->getCommandId (record) != SystemInterface::Command_AgentStatus) {
		return (false);
	}

	fieldname = (StdString *) objectFieldNameStringPtr;
	return (interface->getCommandObjectParam (record, fieldname->c_str (), NULL));
}
