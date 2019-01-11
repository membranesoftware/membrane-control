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
// Class that handles JSON parsing functions

#ifndef JSON_H
#define JSON_H

#include <stdarg.h>
#include <map>
#include <vector>
#include <list>
#include "StdString.h"
#include "StringList.h"
#include "json-parser.h"
#include "json-builder.h"

class Json {
public:
	Json ();
	~Json ();

	// Reassign the Json object's underlying json_value pointer, clearing any pointer that might already be present
	void setJsonValue (Json *value);
	void setJsonValue (json_value *value, bool isJsonBuilder);

	// Reassign the Json object to a newly created empty object, clearing any pointer that might already be present
	void setEmpty ();

	// Parse a JSON string containing key-value pairs and store the resulting data. Returns a Result value.
	int parse (const StdString &data);
	int parse (const char *data, const int dataLength);

	// Return a JSON string containing object fields
	StdString toString ();

	// Replace the Json object's content with a copy of the provided source object. Returns a Result value.
	int copy (Json *sourceJson);

	// Store a list of currently available map keys in the provided list, optionally clearing the list before doing so
	void getKeys (std::vector<StdString> *keyVector, bool shouldClear = false);
	void getKeys (StringList *destList, bool shouldClear = false);

	// Return a boolean value indicating if the specified key exists in the object
	bool exists (const StdString &key) const;
	bool exists (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is a number
	bool isNumber (const StdString &key) const;
	bool isNumber (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is a boolean
	bool isBoolean (const StdString &key) const;
	bool isBoolean (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is a string
	bool isString (const StdString &key) const;
	bool isString (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is an array
	bool isArray (const StdString &key) const;
	bool isArray (const char *key) const;

	// Return the value of the named number key, or the provided default if no such key was found
	int getNumber (const StdString &key, int defaultValue) const;
	int getNumber (const char *key, int defaultValue) const;
	int64_t getNumber (const StdString &key, int64_t defaultValue) const;
	int64_t getNumber (const char *key, int64_t defaultValue) const;
	double getNumber (const StdString &key, double defaultValue) const;
	double getNumber (const char *key, double defaultValue) const;
	float getNumber (const StdString &key, float defaultValue) const;
	float getNumber (const char *key, float defaultValue) const;

	// Return the value of the named boolean key, or the provided default if no such key was found
	bool getBoolean (const StdString &key, bool defaultValue) const;
	bool getBoolean (const char *key, bool defaultValue) const;

	// Return the value of the named string key, or the provided default if no such key was found
	StdString getString (const StdString &key, const StdString &defaultValue) const;
	StdString getString (const char *key, const StdString &defaultValue) const;
	StdString getString (const StdString &key, const char *defaultValue) const;
	StdString getString (const char *key, const char *defaultValue) const;

	// Get the named string array key and store it into the provided StringList object, clearing it first. Returns a boolean value indicating if the object was found.
	bool getStringList (const StdString &key, StringList *destList) const;
	bool getStringList (const char *key, StringList *destList) const;

	// Get the named object key and store its value in the provided Json object. Returns a boolean value indicating if the object was found.
	bool getObject (const StdString &key, Json *destJson);
	bool getObject (const char *key, Json *destJson);

	// Return the length of the named array key. A return value of zero indicates an empty or nonexistent array.
	int getArrayLength (const StdString &key) const;
	int getArrayLength (const char *key) const;

	// Return an item from a number array, or the provided default if no such item was found
	int getArrayNumber (const StdString &key, int index, int defaultValue) const;
	int getArrayNumber (const char *key, int index, int defaultValue) const;
	int64_t getArrayNumber (const StdString &key, int index, int64_t defaultValue) const;
	int64_t getArrayNumber (const char *key, int index, int64_t defaultValue) const;
	double getArrayNumber (const StdString &key, int index, double defaultValue) const;
	double getArrayNumber (const char *key, int index, double defaultValue) const;
	float getArrayNumber (const StdString &key, int index, float defaultValue) const;
	float getArrayNumber (const char *key, int index, float defaultValue) const;

	// Get a string item from the named array key and store its value in the provided Json object. Returns a boolean value indicating if the string was found.
	StdString getArrayString (const StdString &key, int index, const StdString &defaultValue) const;
	StdString getArrayString (const char *key, int index, const StdString &defaultValue) const;

	// Get an object item from the named array key and store its value in the provided Json object. Returns a boolean value indicating if the object was found.
	bool getArrayObject (const StdString &key, int index, Json *destJson);
	bool getArrayObject (const char *key, int index, Json *destJson);

	// Set a key-value pair in the map
	void set (const StdString &key, const char *value);
	void set (const char *key, const char *value);
	void set (const StdString &key, const StdString &value);
	void set (const char *key, const StdString &value);
	void set (const StdString &key, const int value);
	void set (const char *key, const int value);
	void set (const StdString &key, const int64_t value);
	void set (const char *key, const int64_t value);
	void set (const StdString &key, const float value);
	void set (const char *key, const float value);
	void set (const StdString &key, const double value);
	void set (const char *key, const double value);
	void set (const StdString &key, const bool value);
	void set (const char *key, const bool value);
	void set (const StdString &key, Json *value);
	void set (const char *key, Json *value);
	void set (const StdString &key, StringList *value);
	void set (const char *key, StringList *value);
	void set (const StdString &key, std::vector<Json *> *value);
	void set (const char *key, std::vector<Json *> *value);
	void set (const StdString &key, std::list<Json *> *value);
	void set (const char *key, std::list<Json *> *value);

	// Set a string value in the map using a format string
	void setSprintf (const StdString &key, const char *str, ...) __attribute__((format(printf, 3, 4)));
	void setSprintf (const char *key, const char *str, ...) __attribute__((format(printf, 3, 4)));

	// Set a key in the map to a null value
	void setNull (const StdString &key);
	void setNull (const char *key);

private:
	// Clear the json pointer
	void clear ();

	// Set the json value to a newly created builder object
	void resetBuilder ();

	// Use the json_object_push method to set a key in the json object, creating the object if needed
	void jsonObjectPush (const json_char *name, json_value *value);

	json_value *json;
	bool shouldFreeJson;
	bool isJsonBuilder;
};

#endif
