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
// Class that maintains a set of records received from remote agents, indexed by ID

#ifndef RECORD_STORE_H
#define RECORD_STORE_H

#include <map>
#include <list>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "HashMap.h"
#include "Json.h"

class RecordStore {
public:
	typedef bool (*FindMatchFunction) (void *data, Json *record);
	typedef void (*ProcessAgentRecordFunction) (void *data, Json *record, const StdString &agentId);
	typedef void (*ProcessRecordFunction) (void *data, Json *record, const StdString &recordId);
	RecordStore ();
	~RecordStore ();
	static RecordStore *instance;

	// Copy the provided Json object and add the copy to the record store. If recordId is not provided, the command must include a params.id field for use as a record ID.
	void addRecord (Json *record, const StdString &recordId = StdString (""));

	// Remove all records from the store
	void clear ();

	// Lock the record store. This lock should be obtained before executing certain other methods, as specified in their descriptions.
	void lock ();

	// Release a previously acquired lock
	void unlock ();

	// Find a record matching the specified ID and type and return the associated Json object, or NULL if no such record was found. This method should be invoked only while the store is locked; if a Json object is returned by this method, it remains valid only as long as the store lock is held.
	Json *findRecord (const StdString &recordId, int recordType);

	// Find the first available record that passes a match predicate function and return the resulting Json object, or NULL if no such record was found. This method should be invoked only while the store is locked; if Json objects are returned by this method, they remain valid only as long as the store lock is held.
	Json *findRecord (RecordStore::FindMatchFunction matchFn, void *matchData);

	// Find records using a match predicate function and insert them into the provided list, optionally clearing the list before doing so. This method should be invoked only while the store is locked; if Json objects are returned by this method, they remain valid only as long as the store lock is held.
	void findRecords (RecordStore::FindMatchFunction matchFn, void *matchData, std::list<Json *> *destList, bool shouldClear = false);

	// Find records using a match predicate function invoke the provided process function for each one. This method should be invoked only while the store is locked.
	void processRecords (RecordStore::FindMatchFunction matchFn, void *matchData, RecordStore::ProcessAgentRecordFunction processFn, void *processFnData = NULL);

	// Find AgentStatus records containing a status field of the specified name and invoke the provided function for each one. This method should be invoked only while the store is locked.
	void processAgentRecords (const char *agentStatusFieldName, RecordStore::ProcessAgentRecordFunction processFn, void *processFnData = NULL);
	void processAgentRecords (const StdString &agentStatusFieldName, RecordStore::ProcessAgentRecordFunction processFn, void *processFnData = NULL);

	// Find records with the specified command ID and a non-empty record ID value, and invoke the provided function for each one. This method should be invoked only while the store is locked.
	void processCommandRecords (int commandId, RecordStore::ProcessRecordFunction processFn, void *processFnData = NULL);

	// Clear a HashMap and populate it with name / ID pairs for AgentStatus records containing the specified status object. This method should be invoked only while the store is locked.
	void populateAgentMap (HashMap *destMap, const char *statusFieldName);
	void populateAgentMap (HashMap *destMap, const StdString &statusFieldName);

	// Remove the record with the specified ID
	void removeRecord (const StdString &recordId);

	// Remove all records with the specified command ID
	void removeRecords (int commandId);

	// Return the number of AgentStatus records containing a status field of the specified name. This method should be invoked only while the store is locked.
	int countAgentRecords (const char *agentStatusFieldName);
	int countAgentRecords (const StdString &agentStatusFieldName);

	// Return the number of records matching the specified command ID. This method should be invoked only while the store is locked.
	int countCommandRecords (int commandId);

	// Match functions for use with find methods
	static bool matchCommandId (void *intPtr, Json *record);
	static bool matchAgentStatusSource (void *agentIdStringPtr, Json *record);
	static bool matchAgentStatusObjectExists (void *objectFieldNameStringPtr, Json *record);

private:
	std::map<StdString, Json *> recordMap;
	SDL_mutex *mutex;
};

#endif
