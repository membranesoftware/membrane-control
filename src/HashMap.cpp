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
#include "OsUtil.h"
#include "StdString.h"
#include "Buffer.h"
#include "Json.h"
#include "HashMap.h"

HashMap::HashMap ()
: isWriteDirty (false)
, sortFunction (NULL)
, isSorted (false)
{

}

HashMap::~HashMap () {

}

void HashMap::clear () {
	valueMap.clear ();
	keyList.clear ();
}

int HashMap::size () const {
	return (valueMap.size ());
}

bool HashMap::empty () const {
	return (valueMap.empty ());
}

bool HashMap::equals (const HashMap &other) const {
	std::map<StdString, StdString>::const_iterator i, end;

	if (size () != other.size ()) {
		return (false);
	}
	i = valueMap.cbegin ();
	end = valueMap.cend ();
	while (i != end) {
		if (! other.find (i->first, "").equals (i->second)) {
			return (false);
		}
		++i;
	}
	return (true);
}

StdString HashMap::toString () const {
	StdString s;
	std::map<StdString, StdString>::const_iterator i, end;

	s.assign ("{");
	i = valueMap.cbegin ();
	end = valueMap.cend ();
	while (i != end) {
		s.append (" ");
		s.append (i->first);
		s.append ("=\"");
		s.append (i->second);
		s.append ("\"");
		++i;
	}
	s.append (" }");
	return (s);
}

Json *HashMap::toJson () const {
	Json *json;
	std::map<StdString, StdString>::const_iterator i, end;

	json = new Json ();
	json->setEmpty ();
	i = valueMap.cbegin ();
	end = valueMap.cend ();
	while (i != end) {
		json->set (i->first, i->second);
		++i;
	}
	return (json);
}

void HashMap::readJson (Json *json) {
	StringList keys;
	StringList::iterator i, end;
	StdString s;

	clear ();
	json->getKeys (&keys);
	i = keys.begin ();
	end = keys.end ();
	while (i != end) {
		s = json->getString (*i, "");
		if (! s.empty ()) {
			insert (*i, s);
		}
		++i;
	}
}

void HashMap::sort (HashMap::SortFunction fn) {
	sortFunction = fn;
	isSorted = false;
}

void HashMap::doSort () {
	std::map<StdString, StdString>::const_iterator i, end;

	keyList.clear ();
	if (sortFunction) {
		i = valueMap.cbegin ();
		end = valueMap.cend ();
		while (i != end) {
			keyList.push_back (i->first);
			++i;
		}
		keyList.sort (sortFunction);
	}
	isSorted = true;
}

void HashMap::getKeys (StringList *destList, bool shouldClear) {
	HashMap::Iterator i;

	if (shouldClear) {
		destList->clear ();
	}
	i = begin ();
	while (hasNext (&i)) {
		destList->push_back (next (&i));
	}
}

bool HashMap::sortAscending (const StdString &a, const StdString &b) {
	if (a.lowercased ().compare (b.lowercased ()) < 0) {
		return (true);
	}
	return (false);
}

bool HashMap::sortDescending (const StdString &a, const StdString &b) {
	if (a.lowercased ().compare (b.lowercased ()) > 0) {
		return (true);
	}
	return (false);
}

OsUtil::Result HashMap::read (const StdString &filename, bool shouldClear) {
	FILE *fp;
	char data[8192];
	Buffer *buffer;
	OsUtil::Result result;

	fp = fopen (filename.c_str (), "rb");
	if (! fp) {
		return (OsUtil::FileOpenFailedError);
	}
	buffer = new Buffer ();
	while (1) {
		if (! fgets (data, sizeof (data), fp)) {
			break;
		}
		buffer->add (data);
	}
	fclose (fp);

	result = read (buffer, shouldClear);
	delete (buffer);

	return (result);
}

OsUtil::Result HashMap::read (Buffer *buffer, bool shouldClear) {
	uint8_t *data, *end, *key1, *key2, *val1, *val2;
	char c;
	bool iscomment;
	StdString key, val;

	if (shouldClear) {
		clear ();
	}
	data = buffer->data;
	end = data + buffer->length;
	iscomment = false;
	key1 = NULL;
	key2 = NULL;
	val1 = NULL;
	val2 = NULL;
	while (data < end) {
		c = (char) *data;
		++data;

		if (iscomment) {
			if (c == '\n') {
				iscomment = false;
			}
			continue;
		}
		if (! key1) {
			if (c == '#') {
				iscomment = true;
				key1 = NULL;
				key2 = NULL;
				val1 = NULL;
				val2 = NULL;
				continue;
			}
			if (! isspace (c)) {
				key1 = (data - 1);
			}
		}
		else if (! key2) {
			if (isspace (c)) {
				key2 = (data - 2);
			}
		}
		else if (! val1) {
			if (! isspace (c)) {
				val1 = (data - 1);
			}
		}
		else {
			if ((c == '\r') || (c == '\n')) {
				val2 = (data - 2);
			}
		}

		if (key1 && key2 && val1 && val2) {
			key.assign ((char *) key1, key2 - key1 + 1);
			val.assign ((char *) val1, val2 - val1 + 1);
			if ((key.length () > 0) && (val.length () > 0)) {
				valueMap.insert (std::pair<StdString, StdString> (key, val));
			}
		}

		if ((c == '\r') || (c == '\n')) {
			key1 = NULL;
			key2 = NULL;
			val1 = NULL;
			val2 = NULL;
		}
	}
	return (OsUtil::Success);
}

OsUtil::Result HashMap::write (const StdString &filename) {
	std::map<StdString, StdString>::iterator i, end;
	StdString out;
	FILE *fp;

	isWriteDirty = false;
	out.assign ("");
	i = valueMap.begin ();
	end = valueMap.end ();
	while (i != end) {
		out.append (i->first);
		out.append (" ");
		out.append (i->second);
		out.append ("\n");
		++i;
	}

	fp = fopen (filename.c_str (), "wb");
	if (! fp) {
		return (OsUtil::FileOpenFailedError);
	}
	fprintf (fp, "%s", out.c_str ());
	fclose (fp);
	return (OsUtil::Success);
}

bool HashMap::exists (const StdString &key) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	return (i != valueMap.cend ());
}

bool HashMap::exists (const char *key) const {
	return (exists (StdString (key)));
}

void HashMap::insert (const StdString &key, const StdString &value) {
	std::map<StdString, StdString>::iterator i;

	i = valueMap.find (key);
	if (i != valueMap.end ()) {
		if (! i->second.equals (value)) {
			i->second.assign (value);
			isWriteDirty = true;
		}
	}
	else {
		valueMap.insert (std::pair<StdString, StdString> (key, value));
		isWriteDirty = true;
		isSorted = false;
	}
}

void HashMap::insert (const char *key, const StdString &value) {
	insert (StdString (key), value);
}

void HashMap::insert (const StdString &key, const char *value) {
	insert (key, StdString (value));
}

void HashMap::insert (const char *key, const char *value) {
	insert (StdString (key), StdString (value));
}

void HashMap::insert (const StdString &key, bool value) {
	insert (key, value ? StdString ("true") : StdString ("false"));
}

void HashMap::insert (const char *key, bool value) {
	insert (StdString (key), value ? StdString ("true") : StdString ("false"));
}

void HashMap::insert (const StdString &key, int value) {
	insert (key, StdString::createSprintf ("%i", value));
}

void HashMap::insert (const char *key, int value) {
	insert (StdString (key), StdString::createSprintf ("%i", value));
}

void HashMap::insert (const StdString &key, int64_t value) {
	insert (key, StdString::createSprintf ("%lli", (long long int) value));
}

void HashMap::insert (const char *key, int64_t value) {
	insert (StdString (key), StdString::createSprintf ("%lli", (long long int) value));
}

void HashMap::insert (const StdString &key, float value) {
	insert (key, StdString::createSprintf ("%f", value));
}

void HashMap::insert (const char *key, float value) {
	insert (StdString (key), StdString::createSprintf ("%f", value));
}

void HashMap::insert (const StdString &key, double value) {
	insert (key, StdString::createSprintf ("%f", value));
}

void HashMap::insert (const char *key, double value) {
	insert (StdString (key), StdString::createSprintf ("%f", value));
}

void HashMap::insert (const StdString &key, const StringList &value) {
	if (value.empty ()) {
		remove (key);
	}
	else {
		insert (key, value.toJsonString ());
	}
}

void HashMap::insert (const char *key, const StringList &value) {
	insert (StdString (key), value);
}

void HashMap::insert (const StdString &key, JsonList *value) {
	std::map<StdString, StdString>::iterator mi, mend;
	JsonList::iterator ji, jend;
	Json *j;
	StringList keys;
	StdString s;
	bool match;
	char *c;
	int index;

	mi = valueMap.begin ();
	mend = valueMap.end ();
	while (mi != mend) {
		s.assign (mi->first);
		match = false;
		if ((s.length () > (key.length () + 1)) && s.startsWith (key)) {
			c = (char *) s.c_str ();
			c += key.length ();
			if ((*c) == '_') {
				while (true) {
					++c;
					if (*c == '\0') {
						match = true;
						break;
					}
					if ((*c < '0') || (*c > '9')) {
						break;
					}
				}
			}
		}
		if (match) {
			keys.push_back (s);
		}
		++mi;
	}
	remove (keys);

	index = 1;
	ji = value->begin ();
	jend = value->end ();
	while (ji != jend) {
		j = *ji;
		if (! j->isAssigned ()) {
			j->setEmpty ();
		}
		insert (StdString::createSprintf ("%s_%i", key.c_str (), index), j->toString ());
		++index;
		++ji;
	}
	value->clear ();
}

void HashMap::insert (const char *key, JsonList *value) {
	insert (StdString (key), value);
}

void HashMap::remove (const StdString &key) {
	std::map<StdString, StdString>::iterator i;

	i = valueMap.find (key);
	if (i != valueMap.end ()) {
		valueMap.erase (i);
		isWriteDirty = true;
		isSorted = false;
	}
}

void HashMap::remove (const char *key) {
	remove (StdString (key));
}

void HashMap::remove (const StringList &keys) {
	StringList::const_iterator i, end;

	i = keys.cbegin ();
	end = keys.cend ();
	while (i != end) {
		remove (*i);
		++i;
	}
}

StdString HashMap::find (const StdString &key, const StdString &defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (i->second);
}

StdString HashMap::find (const StdString &key, const char *defaultValue) const {
	return (find (key, StdString (defaultValue)));
}

StdString HashMap::find (const char *key, const char *defaultValue) const {
	return (find (StdString (key), StdString (defaultValue)));
}

int HashMap::find (const StdString &key, int defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (atoi (i->second.c_str ()));
}

int HashMap::find (const char *key, int defaultValue) const {
	return (find (StdString (key), defaultValue));
}

int64_t HashMap::find (const StdString &key, int64_t defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (atoll (i->second.c_str ()));
}

int64_t HashMap::find (const char *key, int64_t defaultValue) const {
	return (find (StdString (key), defaultValue));
}

bool HashMap::find (const StdString &key, bool defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (i->second.lowercased ().equals ("true") || i->second.lowercased ().equals ("yes"));
}

bool HashMap::find (const char *key, bool defaultValue) const {
	return (find (StdString (key), defaultValue));
}

float HashMap::find (const StdString &key, float defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return ((float) atof (i->second.c_str ()));
}

float HashMap::find (const char *key, float defaultValue) const {
	return (find (StdString (key), defaultValue));
}

double HashMap::find (const StdString &key, double defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (atof (i->second.c_str ()));
}

double HashMap::find (const char *key, double defaultValue) const {
	return (find (StdString (key), defaultValue));
}

void HashMap::find (const StdString &key, StringList *destList) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		destList->clear ();
		return;
	}
	if (! destList->parseJsonString (i->second)) {
		destList->clear ();
	}
}

void HashMap::find (const char *key, StringList *destList) const {
	find (StdString (key), destList);
}

void HashMap::find (const StdString &key, JsonList *destList) const {
	Json *json;
	StdString s;
	int index;

	destList->clear ();
	index = 1;
	while (true) {
		s = find (StdString::createSprintf ("%s_%i", key.c_str (), index), "");
		if (s.empty ()) {
			break;
		}
		json = new Json ();
		if (! json->parse (s)) {
			delete (json);
			break;
		}
		destList->push_back (json);
		++index;
	}
}

void HashMap::find (const char *key, JsonList *destList) const {
	find (StdString (key), destList);
}

HashMap::Iterator HashMap::begin () {
	HashMap::Iterator i;

	if (sortFunction) {
		if (! isSorted) {
			doSort ();
		}
		i.listIterator = keyList.begin ();
	}
	else {
		i.mapIterator = valueMap.begin ();
	}
	return (i);
}

bool HashMap::hasNext (HashMap::Iterator *i) {
	if (sortFunction) {
		return (i->listIterator != keyList.end ());
	}
	return (i->mapIterator != valueMap.end ());
}

StdString HashMap::next (HashMap::Iterator *i) {
	StdString val;

	if (sortFunction) {
		if (i->listIterator != keyList.end ()) {
			val = *(i->listIterator);
			++(i->listIterator);
		}
	}
	else {
		if (i->mapIterator != valueMap.end ()) {
			val = i->mapIterator->first;
			++(i->mapIterator);
		}
	}
	return (val);
}
