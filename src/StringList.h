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
// String list class (extends std::list<StdString>)

#ifndef STRING_LIST_H
#define STRING_LIST_H

#include <list>
#include "StdString.h"

class StringList : public std::list<StdString> {
public:
	StringList ();
	StringList (const StringList &copySource);
	StringList (const StdString &item);
	virtual ~StringList ();

	typedef bool (*SortFunction) (const StdString &a, const StdString &b);

	// Return a string containing the items in the list
	StdString toString () const;

	// Return a JSON array string containing the items in the list
	StdString toJsonString () const;

	// Return a newly created StringList object containing copies of all items in the list
	StringList *copy () const;

	// Parse the provided JSON array string and replace the list content with the resulting items. Returns a boolean value indicating if the parse was successful.
	bool parseJsonString (const StdString &jsonString);

	// Add the provided string to the list, choosing a position that maintains sorted order
	void insertInOrder (const StdString &item);

	// Add all items from a source StringList object to the list
	void insertStringList (const StringList &sourceList);

	// Return a boolean value indicating if the list contains an item matching the specified value
	bool contains (const StdString &item) const;

	// Return the index of the specified item in the list, or -1 if the item was not found
	int indexOf (const StdString &item) const;
	int indexOf (const char *item) const;

	// Return a boolean value indicating if the list contains all items from another list, in the same order
	bool equals (const StringList &stringList) const;

	// Sort the items in the list
	void sort (StringList::SortFunction sortFunction = StringList::compareAscending);

	// Return a string composed by joining all list items with the specified delimiter
	StdString join (const StdString &delimiter = StdString ("")) const;
	StdString join (const char *delimiter) const;

	// Return the string value from the list at the provided position, or an empty string if no values are available. If a string value was found, advance pos and loop it to the list start if the end has been reached.
	StdString loopNext (StringList::const_iterator *pos) const;

	static bool compareAscending (const StdString &a, const StdString &b);
	static bool compareDescending (const StdString &a, const StdString &b);
	static bool compareCaseInsensitiveAscending (const StdString &a, const StdString &b);
	static bool compareCaseInsensitiveDescending (const StdString &a, const StdString &b);
};

#endif
