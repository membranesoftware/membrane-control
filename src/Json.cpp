/*
* Copyright 2018 Membrane Software <author@membranesoftware.com>
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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <map>
#include <vector>
#include <list>
#include "json-parser.h"
#include "json-builder.h"
#include "Result.h"
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
	clear ();
}

void Json::clear () {
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
	clear ();
	json = json_object_new (0);
	shouldFreeJson = true;
	isJsonBuilder = true;
}

void Json::setEmpty () {
	clear ();
	resetBuilder ();
}

void Json::setJsonValue (Json *value) {
	setJsonValue (value->json, value->isJsonBuilder);
}

void Json::setJsonValue (json_value *value, bool isJsonBuilder) {
	clear ();
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

int Json::parse (const char *data, const int dataLength) {
	json_settings settings;
	json_value *value;
	char buf[json_error_max];

	memset (&settings, 0, sizeof (settings));
	settings.value_extra = json_builder_extra;
	value = json_parse_ex (&settings, data, dataLength, buf);
	if (! value) {
		return (Result::ERROR_JSON_PARSE_FAILED);
	}

	clear ();
	json = value;
	shouldFreeJson = true;
	isJsonBuilder = false;
	return (Result::SUCCESS);
}

int Json::parse (const StdString &data) {
	return (parse (data.c_str (), data.length ()));
}

int Json::copy (Json *sourceJson) {
	StdString s;

	s.assign (sourceJson->toString ());
	return (parse (s));
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
						// TODO: Possibly copy this item to enable clearing of its "parent" pointer, found to cause toString operations with this item to append extra content
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
	jsonObjectPush (key.c_str (), value->json);
	value->json = NULL;
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

void Json::set (const StdString &key, std::vector<Json *> *value) {
	json_value *a;
	std::vector<Json *>::iterator i, end;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		json_array_push (a, (*i)->json);
		(*i)->json = NULL;
		delete (*i);
		++i;
	}

	jsonObjectPush (key.c_str (), a);
}

void Json::set (const char *key, std::vector<Json *> *value) {
	set (StdString (key), value);
}

void Json::set (const StdString &key, std::list<Json *> *value) {
	json_value *a;
	std::list<Json *>::iterator i, end;

	a = json_array_new (0);
	i = value->begin ();
	end = value->end ();
	while (i != end) {
		json_array_push (a, (*i)->json);
		(*i)->json = NULL;
		delete (*i);
		++i;
	}

	jsonObjectPush (key.c_str (), a);
}

void Json::set (const char *key, std::list<Json *> *value) {
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
		return (StdString ("{}"));
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
		Log::write (Log::ERR, "Out of memory in Json::toString; len=%i", len);
		return (StdString (""));
	}
	
	json_serialize_ex (buf, json, opts);
	s.assign (buf);
	free (buf);

	return (s);
}