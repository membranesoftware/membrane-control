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
// Class that holds a map of key-value pairs

#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <map>
#include <list>
#include "StdString.h"
#include "OsUtil.h"
#include "StringList.h"
#include "Json.h"
#include "Buffer.h"

class HashMap {
public:
	HashMap ();
	~HashMap ();

	struct Iterator {
		std::list<StdString>::iterator listIterator;
		std::map<StdString, StdString>::iterator mapIterator;
	};
	typedef bool (*SortFunction) (const StdString &a, const StdString &b);

	// Read-only data members
	bool isWriteDirty;

	// Remove all items from the map
	void clear ();

	// Return the number of items in the map
	int size ();

	// Return a boolean value indicating if the map is empty
	bool empty ();

	// Return a string representation of the map
	StdString toString ();

	// Set the sort function that should be used to order items in generated map traversals
	void sort (HashMap::SortFunction fn);

	// Insert the map's keys into the provided list, optionally clearing the list before doing so
	void getKeys (StringList *destList, bool shouldClear = false);

	// Set a key-value pair in the map
	void insert (const StdString &key, const StdString &value);
	void insert (const char *key, const StdString &value);
	void insert (const StdString &key, const char *value);
	void insert (const char *key, const char *value);
	void insert (const StdString &key, bool value);
	void insert (const char *key, bool value);
	void insert (const StdString &key, int value);
	void insert (const char *key, int value);
	void insert (const StdString &key, int64_t value);
	void insert (const char *key, int64_t value);

	// Set a key-value pair in the map to store a StringList. If value is an empty list, instead remove the named key.
	void insert (const StdString &key, StringList *value);
	void insert (const char *key, StringList *value);

	// Set key-value pairs in the map to store a JsonList, freeing all contained Json objects in the process. If value is an empty list, instead remove all key-value pairs for JsonList objects associated with the named base key.
	void insert (const StdString &key, JsonList *value);
	void insert (const char *key, JsonList *value);

	// Remove the specified key or list of keys from the map
	void remove (const StdString &key);
	void remove (const char *key);
	void remove (StringList *keys);

	// Read values from configuration file data and store the resulting items in the map, optionally clearing the map before doing so. Returns a Result value.
	OsUtil::Result read (const StdString &filename, bool shouldClear = false);
	OsUtil::Result read (Buffer *buffer, bool shouldClear = false);

	// Write values from the map to the specified file. Returns a Result value.
	OsUtil::Result write (const StdString &filename);

	// Return a boolean value indicating if the provided key exists in the map
	bool exists (const StdString &key);
	bool exists (const char *key);

	// Return a string containing a value from the map, or the specified default if no such value exists
	StdString find (const StdString &key, const StdString &defaultValue);
	StdString find (const StdString &key, const char *defaultValue);
	StdString find (const char *key, const char *defaultValue);
	int find (const StdString &key, int defaultValue);
	int find (const char *key, int defaultValue);
	int64_t find (const StdString &key, int64_t defaultValue);
	int64_t find (const char *key, int64_t defaultValue);
	bool find (const StdString &key, bool defaultValue);
	bool find (const char *key, bool defaultValue);

	// Clear the provided StringList and fill it with items from the specified string list key if found
	void find (const StdString &key, StringList *destList);
	void find (const char *key, StringList *destList);

	// Clear the provided JsonList and free all contained Json objects, then fill it with items from the specified object list key if found
	void find (const StdString &key, JsonList *destList);
	void find (const char *key, JsonList *destList);

	// Return an iterator positioned at the map's first element
	HashMap::Iterator begin ();

	// Return a boolean value indicating if the provided iterator contains a next element
	bool hasNext (HashMap::Iterator *i);

	// Return the string key referenced by the provided iterator, or an empty string if the iterator has ended. If an item is found, advance the iterator.
	StdString next (HashMap::Iterator *i);

	// Sort functions
	static bool sortAscending (const StdString &a, const StdString &b);
	static bool sortDescending (const StdString &a, const StdString &b);

private:
	// Clear keyList and populate it with key items in sorted order
	void doSort ();

	std::map<StdString, StdString> valueMap;
	std::list<StdString> keyList;
	HashMap::SortFunction sortFunction;
	bool isSorted;
};

#endif
