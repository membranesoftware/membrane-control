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
#include <ctype.h>
#include <list>
#include "StdString.h"
#include "StringList.h"

StringList::StringList ()
: std::list<StdString> ()
{

}

StringList::StringList (const StringList &copySource)
: StringList ()
{
	insertStringList (copySource);
}

StringList::StringList (const StdString &item)
: StringList ()
{
	push_back (item);
}

StringList::~StringList () {

}

StdString StringList::toString () const {
	StdString s;
	StringList::const_iterator i1, i2;
	char delim[2];

	delim[0] = '\0';
	delim[1] = '\0';
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		s.appendSprintf ("%s%s", delim, (*i1).c_str ());
		delim[0] = ',';
		++i1;
	}
	return (s);
}

StdString StringList::toJsonString () const {
	StdString s, item;
	StringList::const_iterator i1, i2;
	char delim[2];

	delim[0] = '\0';
	delim[1] = '\0';
	s.assign ("[");
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		item = *i1;
		s.appendSprintf ("%s\"%s\"", delim, item.jsonEscaped ().c_str ());
		delim[0] = ',';
		++i1;
	}
	s.append ("]");
	return (s);
}

StringList *StringList::copy () const {
	StringList *stringlist;

	stringlist = new StringList ();
	stringlist->insertStringList (*this);
	return (stringlist);
}

bool StringList::parseJsonString (const StdString &jsonString) {
	int i, len, stage;
	char c;
	bool success, err, escaped, ended;
	StdString item;

	clear ();
	if (jsonString.empty ()) {
		return (true);
	}

	success = false;
	err = false;
	escaped = false;
	ended = false;
	stage = 0;
	len = jsonString.length ();
	i = 0;
	while (i < len) {
		c = jsonString.at (i);
		switch (stage) {
			case 0: {
				if (c == '[') {
					stage = 1;
				}
				else {
					err = true;
				}
				break;
			}
			case 1: {
				if (c == '"') {
					item.assign ("");
					escaped = false;
					stage = 2;
				}
				else if (c == ']') {
					ended = true;
				}
				else if (! isspace (c)) {
					err = true;
				}
				break;
			}
			case 2: {
				if (escaped) {
					item.append (1, c);
					escaped = false;
				}
				else {
					if (c == '\\') {
						escaped = true;
					}
					else if (c == '"') {
						push_back (item);
						stage = 3;
					}
					else {
						item.append (1, c);
					}
				}
				break;
			}
			case 3: {
				if (c == ']') {
					ended = true;
				}
				else if (c == ',') {
					stage = 1;
				}
				else if (! isspace (c)) {
					err = true;
				}
				break;
			}
		}

		if (err || ended) {
			break;
		}
		++i;
	}

	if (! err) {
		if (! ended) {
			err = true;
		}
	}

	if (err) {
		clear ();
	}
	else {
		success = true;
	}
	return (success);
}

void StringList::insertInOrder (const StdString &item) {
	StringList::iterator i1, i2;
	bool found;

	found = false;
	i1 = begin ();
	i2 = end ();
	while (i1 != i2) {
		if (item.compare (*i1) < 0) {
			found = true;
			insert (i1, item);
			break;
		}
		++i1;
	}

	if (! found) {
		push_back (item);
	}
}

void StringList::insertStringList (const StringList &sourceList) {
	StringList::const_iterator i1, i2;

	i1 = sourceList.cbegin ();
	i2 = sourceList.cend ();
	while (i1 != i2) {
		push_back (*i1);
		++i1;
	}
}

bool StringList::contains (const StdString &item) const {
	StringList::const_iterator i1, i2;
	bool found;

	found = false;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		if (item.equals (*i1)) {
			found = true;
			break;
		}
		++i1;
	}
	return (found);
}

int StringList::indexOf (const StdString &item) const {
	StringList::const_iterator i1, i2;
	int result, count;

	result = -1;
	count = 0;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		if (item.equals (*i1)) {
			result = count;
			break;
		}
		++count;
		++i1;
	}
	return (result);
}

int StringList::indexOf (const char *item) const {
	return (indexOf (StdString (item)));
}

bool StringList::equals (const StringList &stringList) const {
	StringList::const_iterator i, iend, j, jend;

	if (size () != stringList.size ()) {
		return (false);
	}
	i = cbegin ();
	iend = cend ();
	j = stringList.cbegin ();
	jend = stringList.cend ();
	while (i != iend) {
		if (j == jend) {
			return (false);
		}
		if (! (*i).equals (*j)) {
			return (false);
		}
		++i;
		++j;
	}
	return (true);
}

void StringList::sort (StringList::SortFunction sortFunction) {
	std::list<StdString>::sort (sortFunction);
}

bool StringList::compareAscending (const StdString &a, const StdString &b) {
	return (a.compare (b) < 0);
}

bool StringList::compareDescending (const StdString &a, const StdString &b) {
	return (a.compare (b) > 0);
}

bool StringList::compareCaseInsensitiveAscending (const StdString &a, const StdString &b) {
	return (a.lowercased ().compare (b.lowercased ()) < 0);
}

bool StringList::compareCaseInsensitiveDescending (const StdString &a, const StdString &b) {
	return (a.lowercased ().compare (b.lowercased ()) > 0);
}

StdString StringList::join (const StdString &delimiter) const {
	StdString s;
	StringList::const_iterator i1, i2;
	bool first;

	first = true;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		if (first) {
			first = false;
			s.append (*i1);
		}
		else {
			s.appendSprintf ("%s%s", delimiter.c_str (), (*i1).c_str ());
		}
		++i1;
	}
	return (s);
}

StdString StringList::join (const char *delimiter) const {
	return (join (StdString (delimiter)));
}

StdString StringList::loopNext (StringList::const_iterator *pos) const {
	StdString s;

	if (*pos == cend ()) {
		*pos = cbegin ();
	}
	if (*pos != cend ()) {
		s.assign (**pos);
		++(*pos);
	}
	return (s);
}
