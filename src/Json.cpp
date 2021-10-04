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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <map>
#include <vector>
#include <list>
#include "json-parser.h"
#include "json-builder.h"
#include "Log.h"
#include "StdString.h"
#include "Json.h"

Json::Json ()
: json (NULL)
, shouldFreeJson (false)
, isJsonBuilder (false)
{

}

Json::~Json () {
	unassign ();
}

void Json::freeObject (void *jsonPtr) {
	delete ((Json *) jsonPtr);
}

bool Json::isAssigned () {
	return (json ? true : false);
}

void Json::unassign () {
	if (json) {
		if (shouldFreeJson) {
			if (isJsonBuilder) {
				json_builder_free (json);
			}
			else {
				json_value_free (json);
			}
		}
		json = NULL;
	}
	shouldFreeJson = false;
	isJsonBuilder = false;
}

void Json::resetBuilder () {
	unassign ();
	json = json_object_new (0);
	shouldFreeJson = true;
	isJsonBuilder = true;
}

void Json::setEmpty () {
	unassign ();
	resetBuilder ();
}

void Json::setJsonValue (json_value *value, bool isJsonBuilder) {
	unassign ();
	json = value;
	this->isJsonBuilder = isJsonBuilder;
}

void Json::jsonObjectPush (const json_char *name, json_value *value) {
	if ((! json) || (! isJsonBuilder)) {
		resetBuilder ();
	}
	json_object_push (json, name, value);
}

bool Json::exists (const StdString &key) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (false);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			return (true);
		}
	}

	return (false);
}

bool Json::exists (const char *key) const {
	return (exists (StdString (key)));
}

void Json::getKeys (std::vector<StdString> *destVector, bool shouldClear) {
	int i, len;

	if (shouldClear) {
		destVector->clear ();
	}
	if (! json) {
		return;
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		destVector->push_back (StdString (json->u.object.values[i].name, json->u.object.values[i].name_length));
	}
}

void Json::getKeys (StringList *destList, bool shouldClear) {
	int i, len;

	if (shouldClear) {
		destList->clear ();
	}
	if (! json) {
		return;
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		destList->push_back (StdString (json->u.object.values[i].name, json->u.object.values[i].name_length));
	}
}

bool Json::isNull (const StdString &key) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (false);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			return ((entry->value->type == json_null) ? true : false);
		}
	}

	return (false);
}

bool Json::isNull (const char *key) const {
	return (isNull (StdString (key)));
}

bool Json::isNumber (const StdString &key) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (false);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_integer: {
					return (true);
				}
				case json_double: {
					return (true);
				}
				default: {
					return (false);
				}
			}
			break;
		}
	}

	return (false);
}

bool Json::isNumber (const char *key) const {
	return (isNumber (StdString (key)));
}

bool Json::isBoolean (const StdString &key) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (false);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			return ((entry->value->type == json_boolean) ? true : false);
		}
	}

	return (false);
}

bool Json::isBoolean (const char *key) const {
	return (isBoolean (StdString (key)));
}

bool Json::isString (const StdString &key) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (false);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			return ((entry->value->type == json_string) ? true : false);
		}
	}

	return (false);
}

bool Json::isString (const char *key) const {
	return (isString (StdString (key)));
}

bool Json::isArray (const StdString &key) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (false);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			if (entry->value->type == json_array) {
				return (true);
			}

			break;
		}
	}

	return (false);
}

bool Json::isArray (const char *key) const {
	return (isArray (StdString (key)));
}

bool Json::parse (const char *data, const int dataLength) {
	json_settings settings;
	json_value *value;
	char buf[json_error_max];

	memset (&settings, 0, sizeof (settings));
	settings.value_extra = json_builder_extra;
	value = json_parse_ex (&settings, data, dataLength, buf);
	if (! value) {
		return (false);
	}

	unassign ();
	json = value;
	shouldFreeJson = true;
	isJsonBuilder = false;
	return (true);
}

bool Json::parse (const StdString &data) {
	return (parse (data.c_str (), data.length ()));
}

void Json::assign (Json *otherJson) {
	if (! otherJson) {
		unassign ();
		return;
	}

	if (otherJson->json) {
		setJsonValue (otherJson->json, otherJson->isJsonBuilder);
		otherJson->json = NULL;
		shouldFreeJson = true;
	}
	else {
		unassign ();
	}
	delete (otherJson);
}

void Json::copyValue (Json *sourceJson) {
	setEmpty ();
	if (! sourceJson->json) {
		return;
	}

	setJsonValue (copyJsonValue (sourceJson->json), true);
	shouldFreeJson = true;
}

Json *Json::copy () {
	Json *j;

	j = new Json ();
	j->copyValue (this);

	return (j);
}

json_value *Json::copyJsonValue (json_value *sourceValue) {
	json_value *value;
	json_object_entry *entry;
	StdString s;
	int i, len;

	switch (sourceValue->type) {
		case json_integer: {
			value = json_integer_new (sourceValue->u.integer);
			break;
		}
		case json_double: {
			value = json_double_new (sourceValue->u.dbl);
			break;
		}
		case json_boolean: {
			value = json_boolean_new (sourceValue->u.boolean);
			break;
		}
		case json_string: {
			s.assign (sourceValue->u.string.ptr, sourceValue->u.string.length);
			value = json_string_new (s.c_str ());
			break;
		}
		case json_array: {
			value = json_array_new (0);
			len = sourceValue->u.array.length;
			for (i = 0; i < len; ++i) {
				json_array_push (value, copyJsonValue (sourceValue->u.array.values[i]));
			}
			break;
		}
		case json_object: {
			value = json_object_new (0);
			len = sourceValue->u.object.length;
			for (i = 0; i < len; ++i) {
				entry = &(sourceValue->u.object.values[i]);
				s.assign (entry->name, entry->name_length);
				json_object_push (value, s.c_str (), copyJsonValue (entry->value));
			}
			break;
		}
		default: {
			value = json_null_new ();
			break;
		}
	}

	return (value);
}

bool Json::deepEquals (Json *other) {
	if ((! json) && (! other->json)) {
		return (true);
	}
	if ((! json) || (! other->json)) {
		return (false);
	}

	return (deepEqualsValue (json, other->json));
}

bool Json::deepEqualsValue (json_value *thisValue, json_value *otherValue) {
	std::map<StdString, int> keymap;
	std::map<StdString, int>::iterator pos;
	int i, len;
	bool result;

	if (thisValue->type != otherValue->type) {
		return (false);
	}

	result = false;
	switch (thisValue->type) {
		case json_integer: {
			result = (thisValue->u.integer == otherValue->u.integer);
			break;
		}
		case json_double: {
			result = FLOAT_EQUALS (thisValue->u.dbl, otherValue->u.dbl);
			break;
		}
		case json_boolean: {
			result = (thisValue->u.boolean == otherValue->u.boolean);
			break;
		}
		case json_string: {
			result = StdString (thisValue->u.string.ptr, thisValue->u.string.length).equals (StdString (otherValue->u.string.ptr, otherValue->u.string.length));
			break;
		}
		case json_array: {
			len = thisValue->u.array.length;
			if (len != (int) otherValue->u.array.length) {
				result = false;
				break;
			}

			result = true;
			for (i = 0; i < len; ++i) {
				if (! deepEqualsValue (thisValue->u.array.values[i], otherValue->u.array.values[i])) {
					result = false;
					break;
				}
			}
			break;
		}
		case json_object: {
			len = thisValue->u.object.length;
			if (len != (int) otherValue->u.object.length) {
				result = false;
				break;
			}
			if (len == 0) {
				result = true;
				break;
			}

			keymap.clear ();
			for (i = 0; i < len; ++i) {
				keymap.insert (std::pair<StdString, int> (StdString (thisValue->u.object.values[i].name, thisValue->u.object.values[i].name_length), i));
			}

			result = true;
			for (i = 0; i < len; ++i) {
				pos = keymap.find (StdString (otherValue->u.object.values[i].name, otherValue->u.object.values[i].name_length));
				if (pos == keymap.end ()) {
					result = false;
					break;
				}
				if (! deepEqualsValue (thisValue->u.object.values[pos->second].value, otherValue->u.object.values[i].value)) {
					result = false;
					break;
				}
			}
			break;
		}
		default: {
			result = true;
			break;
		}
	}

	return (result);
}

int Json::getNumber (const StdString &key, int defaultValue) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_integer: {
					return (entry->value->u.integer);
				}
				case json_double: {
					return ((int) entry->value->u.dbl);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

int Json::getNumber (const char *key, int defaultValue) const {
	return (getNumber (StdString (key), defaultValue));
}

int64_t Json::getNumber (const StdString &key, int64_t defaultValue) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_integer: {
					return ((int64_t) entry->value->u.integer);
				}
				case json_double: {
					return ((int64_t) entry->value->u.dbl);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

int64_t Json::getNumber (const char *key, int64_t defaultValue) const {
	return (getNumber (StdString (key), defaultValue));
}

double Json::getNumber (const StdString &key, double defaultValue) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_integer: {
					return ((double) entry->value->u.integer);
				}
				case json_double: {
					return (entry->value->u.dbl);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

double Json::getNumber (const char *key, double defaultValue) const {
	return (getNumber (StdString (key), defaultValue));
}

float Json::getNumber (const StdString &key, float defaultValue) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_integer: {
					return ((float) entry->value->u.integer);
				}
				case json_double: {
					return ((float) entry->value->u.dbl);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

float Json::getNumber (const char *key, float defaultValue) const {
	return (getNumber (StdString (key), defaultValue));
}

bool Json::getBoolean (const StdString &key, bool defaultValue) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_boolean: {
					return (entry->value->u.boolean);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

bool Json::getBoolean (const char *key, bool defaultValue) const {
	return (getBoolean (StdString (key), defaultValue));
}

StdString Json::getString (const StdString &key, const StdString &defaultValue) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_string: {
					return (StdString (entry->value->u.string.ptr, entry->value->u.string.length));
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

StdString Json::getString (const char *key, const StdString &defaultValue) const {
	return (getString (StdString (key), defaultValue));
}

StdString Json::getString (const StdString &key, const char *defaultValue) const {
	return (getString (key, StdString (defaultValue)));
}

StdString Json::getString (const char *key, const char *defaultValue) const {
	return (getString (StdString (key), StdString (defaultValue)));
}

bool Json::getStringList (const StdString &key, StringList *destList) const {
	int i, len;

	destList->clear ();
	if (! isArray (key)) {
		return (false);
	}

	len = getArrayLength (key);
	if (len <= 0) {
		return (true);
	}

	for (i = 0; i < len; ++i) {
		destList->push_back (getArrayString (key, i, StdString ("")));
	}

	return (true);
}

bool Json::getStringList (const char *key, StringList *destList) const {
	return (getStringList (StdString (key), destList));
}

bool Json::getObject (const StdString &key, Json *destJson) {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (false);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_object: {
					if (destJson) {
						destJson->setJsonValue (entry->value, isJsonBuilder);
					}
					return (true);
				}
				default: {
					return (false);
				}
			}
			break;
		}
	}

	return (false);
}

bool Json::getObject (const char *key, Json *destJson) {
	return (getObject (StdString (key), destJson));
}

int Json::getArrayLength (const StdString &key) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (0);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_array: {
					return (entry->value->u.array.length);
				}
				default: {
					return (0);
				}
			}
			break;
		}
	}

	return (0);
}

int Json::getArrayLength (const char *key) const {
	return (getArrayLength (StdString (key)));
}

int Json::getArrayNumber (const StdString &key, int index, int defaultValue) const {
	int i, len;
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_array: {
					if (index >= (int) entry->value->u.array.length) {
						return (defaultValue);
					}
					item = entry->value->u.array.values[index];
					if (item->type == json_integer) {
						return (item->u.integer);
					}
					else if (item->type == json_double) {
						return ((int) item->u.dbl);
					}

					return (defaultValue);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

int Json::getArrayNumber (const char *key, int index, int defaultValue) const {
	return (getArrayNumber (StdString (key), index, defaultValue));
}

int64_t Json::getArrayNumber (const StdString &key, int index, int64_t defaultValue) const {
	int i, len;
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_array: {
					if (index >= (int) entry->value->u.array.length) {
						return (defaultValue);
					}
					item = entry->value->u.array.values[index];
					if (item->type == json_integer) {
						return ((int64_t) item->u.integer);
					}
					else if (item->type == json_double) {
						return ((int64_t) item->u.dbl);
					}

					return (defaultValue);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

int64_t Json::getArrayNumber (const char *key, int index, int64_t defaultValue) const {
	return (getArrayNumber (StdString (key), index, defaultValue));
}

double Json::getArrayNumber (const StdString &key, int index, double defaultValue) const {
	int i, len;
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_array: {
					if (index >= (int) entry->value->u.array.length) {
						return (defaultValue);
					}
					item = entry->value->u.array.values[index];
					if (item->type == json_integer) {
						return ((double) item->u.integer);
					}
					else if (item->type == json_double) {
						return (item->u.dbl);
					}

					return (defaultValue);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

double Json::getArrayNumber (const char *key, int index, double defaultValue) const {
	return (getArrayNumber (StdString (key), index, defaultValue));
}

float Json::getArrayNumber (const StdString &key, int index, float defaultValue) const {
	int i, len;
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_array: {
					if (index >= (int) entry->value->u.array.length) {
						return (defaultValue);
					}
					item = entry->value->u.array.values[index];
					if (item->type == json_integer) {
						return ((float) item->u.integer);
					}
					else if (item->type == json_double) {
						return ((float) item->u.dbl);
					}

					return (defaultValue);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

float Json::getArrayNumber (const char *key, int index, float defaultValue) const {
	return (getArrayNumber (StdString (key), index, defaultValue));
}

StdString Json::getArrayString (const StdString &key, int index, const StdString &defaultValue) const {
	int i, len;
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_array: {
					if (index >= (int) entry->value->u.array.length) {
						return (defaultValue);
					}
					item = entry->value->u.array.values[index];
					if (item->type == json_string) {
						return (StdString (item->u.string.ptr, item->u.string.length));
					}

					return (defaultValue);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

StdString Json::getArrayString (const char *key, int index, const StdString &defaultValue) const {
	return (getArrayString (StdString (key), index, defaultValue));
}

bool Json::getArrayBoolean (const StdString &key, int index, bool defaultValue) const {
	int i, len;
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_array: {
					if (index >= (int) entry->value->u.array.length) {
						return (defaultValue);
					}
					item = entry->value->u.array.values[index];
					if (item->type == json_boolean) {
						return (item->u.boolean);
					}

					return (defaultValue);
				}
				default: {
					return (defaultValue);
				}
			}
			break;
		}
	}

	return (defaultValue);
}

bool Json::getArrayBoolean (const char *key, int index, bool defaultValue) const {
	return (getArrayBoolean (StdString (key), index, defaultValue));
}

bool Json::getArrayObject (const StdString &key, int index, Json *destJson) {
	int i, len;
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (false);
	}

	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			switch (entry->value->type) {
				case json_array: {
					if (index >= (int) entry->value->u.array.length) {
						return (false);
					}
					item = entry->value->u.array.values[index];
					if (item->type != json_object) {
						return (false);
					}

					if (destJson) {
						destJson->setJsonValue (item, isJsonBuilder);
					}
					return (true);
				}
				default: {
					return (false);
				}
			}
			break;
		}
	}

	return (false);
}

bool Json::getArrayObject (const char *key, int index, Json *destJson) {
	return (getArrayObject (StdString (key), index, destJson));
}

void Json::set (const StdString &key, const char *value) {
	jsonObjectPush (key.c_str (), json_string_new (value));
}

void Json::set (const char *key, const char *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, const StdString &value) {
	jsonObjectPush (key.c_str (), json_string_new (value.c_str ()));
}

void Json::set (const char *key, const StdString &value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, const int value) {
	jsonObjectPush (key.c_str (), json_integer_new (value));
}

void Json::set (const char *key, const int value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, const int64_t value) {
	jsonObjectPush (key.c_str (), json_double_new ((double) value));
}

void Json::set (const char *key, const int64_t value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, const float value) {
	jsonObjectPush (key.c_str (), json_double_new ((double) value));
}

void Json::set (const char *key, const float value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, const double value) {
	jsonObjectPush (key.c_str (), json_double_new (value));
}

void Json::set (const char *key, const double value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, const bool value) {
	jsonObjectPush (key.c_str (), json_boolean_new (value));
}

void Json::set (const char *key, const bool value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, Json *value) {
	if (value->json) {
		jsonObjectPush (key.c_str (), value->json);
		value->json = NULL;
	}
	else {
		jsonObjectPush (key.c_str (), json_object_new (0));
	}
	delete (value);
}

void Json::set (const char *key, Json *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, StringList *value) {
	json_value *a;
	StringList::iterator i, end;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		json_array_push (a, json_string_new (i->c_str ()));
		++i;
	}

	jsonObjectPush (key.c_str (), a);
}

void Json::set (const char *key, StringList *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, std::list<int> *value) {
	json_value *a;
	std::list<int>::iterator i, end;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		json_array_push (a, json_integer_new (*i));
		++i;
	}

	jsonObjectPush (key.c_str (), a);
}

void Json::set (const char *key, std::list<int> *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, std::list<int64_t> *value) {
	json_value *a;
	std::list<int64_t>::iterator i, end;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		json_array_push (a, json_double_new ((double) *i));
		++i;
	}

	jsonObjectPush (key.c_str (), a);
}

void Json::set (const char *key, std::list<int64_t> *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, std::list<float> *value) {
	json_value *a;
	std::list<float>::iterator i, end;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		json_array_push (a, json_double_new (*i));
		++i;
	}

	jsonObjectPush (key.c_str (), a);
}

void Json::set (const char *key, std::list<float> *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, std::list<double> *value) {
	json_value *a;
	std::list<double>::iterator i, end;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		json_array_push (a, json_double_new (*i));
		++i;
	}

	jsonObjectPush (key.c_str (), a);
}

void Json::set (const char *key, std::list<double> *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, std::list<bool> *value) {
	json_value *a;
	std::list<bool>::iterator i, end;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		json_array_push (a, json_boolean_new (*i));
		++i;
	}

	jsonObjectPush (key.c_str (), a);
}

void Json::set (const char *key, std::list<bool> *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, std::vector<Json *> *value) {
	json_value *a;
	std::vector<Json *>::iterator i, end;
	Json *item;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		item = *i;
		if (! item->json) {
			item->setEmpty ();
		}
		json_array_push (a, item->json);
		item->json = NULL;
		delete (item);
		++i;
	}

	jsonObjectPush (key.c_str (), a);
	value->clear ();
}

void Json::set (const char *key, std::vector<Json *> *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, std::list<Json *> *value) {
	json_value *a;
	std::list<Json *>::iterator i, end;
	Json *item;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		item = *i;
		if (! item->json) {
			item->setEmpty ();
		}
		json_array_push (a, item->json);
		item->json = NULL;
		delete (item);
		++i;
	}

	jsonObjectPush (key.c_str (), a);
	value->clear ();
}

void Json::set (const char *key, std::list<Json *> *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, JsonList *value) {
	json_value *a;
	JsonList::iterator i, end;
	Json *item;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		item = *i;
		if (! item->json) {
			item->setEmpty ();
		}
		json_array_push (a, item->json);
		item->json = NULL;
		++i;
	}

	jsonObjectPush (key.c_str (), a);
	value->clear ();
}

void Json::set (const char *key, JsonList *value) {
	set (StdString (key), value);
}

void Json::setSprintf (const StdString &key, const char *str, ...) {
	va_list ap;
	StdString s;

	va_start (ap, str);
	s.vsprintf (str, ap);
	va_end (ap);

	jsonObjectPush (key.c_str (), json_string_new (s.c_str ()));
}

void Json::setSprintf (const char *key, const char *str, ...) {
	va_list ap;
	StdString s;

	va_start (ap, str);
	s.vsprintf (str, ap);
	va_end (ap);

	jsonObjectPush (key, json_string_new (s.c_str ()));
}

void Json::setNull (const StdString &key) {
	jsonObjectPush (key.c_str (), json_null_new ());
}

void Json::setNull (const char *key) {
	setNull (StdString (key));
}

StdString Json::toString () {
	StdString s;
	char *buf;
	json_serialize_opts opts;
	int len;

	if (! json) {
		return (StdString (""));
	}

	// TODO: Possibly employ a locking mechanism here (json_measure_ex modifies json data structures while measuring)

	opts.mode = json_serialize_mode_packed;
	opts.opts = json_serialize_opt_no_space_after_colon | json_serialize_opt_no_space_after_comma | json_serialize_opt_pack_brackets;
	opts.indent_size = 0;
	len = json_measure_ex (json, opts);
	if (len <= 0) {
		return (StdString (""));
	}

	buf = (char *) malloc (len);
	if (! buf) {
		Log::err ("Out of memory in Json::toString; len=%i", len);
		return (StdString (""));
	}

	json_serialize_ex (buf, json, opts);
	s.assign (buf);
	free (buf);

	return (s);
}

JsonList::JsonList ()
: std::list<Json *> ()
{

}

JsonList::~JsonList () {
	clear ();
}

void JsonList::clear () {
	JsonList::iterator i, iend;

	i = begin ();
	iend = end ();
	while (i != iend) {
		delete (*i);
		++i;
	}
	std::list<Json *>::clear ();
}

StdString JsonList::toString () {
	JsonList::iterator i, iend;
	StdString s;
	bool first;

	s.assign ("[");
	first = true;
	i = begin ();
	iend = end ();
	while (i != iend) {
		if (first) {
			first = false;
		}
		else {
			s.append (",");
		}
		s.append ((*i)->toString ());
		++i;
	}
	s.append ("]");

	return (s);
}

void JsonList::copyValues (JsonList *sourceList) {
	JsonList::iterator i, iend;

	clear ();
	i = sourceList->begin ();
	iend = sourceList->end ();
	while (i != iend) {
		push_back ((*i)->copy ());
		++i;
	}
}

JsonList *JsonList::copy () {
	JsonList *j;

	j = new JsonList ();
	j->copyValues (this);

	return (j);
}

Json *JsonList::at (int index) {
	JsonList::iterator i, iend;
	int listindex;

	if ((index < 0) || (index >= (int) size ())) {
		return (NULL);
	}
	listindex = 0;
	i = begin ();
	iend = end ();
	while (i != iend) {
		if (index == listindex) {
			return (*i);
		}
		++listindex;
		++i;
	}
	return (NULL);
}
