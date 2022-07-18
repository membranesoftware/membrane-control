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
// Class that handles JSON parsing functions

#ifndef JSON_H
#define JSON_H

#include <stdarg.h>
#include <vector>
#include <list>
#include "StdString.h"
#include "StringList.h"
#include "json-parser.h"
#include "json-builder.h"

class JsonList;

class Json {
public:
	Json ();
	~Json ();

	// Free the provided Json object pointer
	static void freeObject (void *jsonPtr);

	// Return a boolean value indicating whether the Json object holds a value
	bool isAssigned ();

	// Clear any stored json pointer, leaving the Json object with an unassigned value
	void unassign ();

	// Reassign the Json object to a newly created empty object, clearing any pointer that might already be present
	void setEmpty ();

	// Parse a JSON string containing key-value pairs and store the resulting data. Returns a boolean value indicating if the parse succeeded.
	bool parse (const StdString &data);
	bool parse (const char *data, const int dataLength);

	// Return a JSON string containing object fields
	StdString toString ();

	// Replace the Json object's content with another object's json pointer and free the other object
	void assign (Json *otherJson);

	// Replace the Json object's content with a copy of the provided source object
	void copyValue (Json *sourceJson);

	// Return a newly created Json object with contents copied from this object
	Json *copy ();

	// Return a boolean value indicating if the object's content matches that of another
	bool deepEquals (Json *other);

	// Store a list of currently available map keys in the provided list, optionally clearing the list before doing so
	void getKeys (std::vector<StdString> *keyVector, bool shouldClear = false);
	void getKeys (StringList *destList, bool shouldClear = false);

	// Return a boolean value indicating if the specified key exists in the object
	bool exists (const StdString &key) const;
	bool exists (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is a null value
	bool isNull (const StdString &key) const;
	bool isNull (const char *key) const;

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

	// Get a string item from the named array key return its value, or the specified default value if no such item was found
	StdString getArrayString (const StdString &key, int index, const StdString &defaultValue) const;
	StdString getArrayString (const char *key, int index, const StdString &defaultValue) const;

	// Get a boolean item from the named array key return its value, or the specified default value if no such item was found
	bool getArrayBoolean (const StdString &key, int index, bool defaultValue) const;
	bool getArrayBoolean (const char *key, int index, bool defaultValue) const;

	// Get an object item from the named array key and store its value in the provided Json object. Returns a boolean value indicating if the object was found.
	bool getArrayObject (const StdString &key, int index, Json *destJson);
	bool getArrayObject (const char *key, int index, Json *destJson);

	// Set a key-value pair in the map and return the this pointer
	Json *set (const StdString &key, const char *value);
	Json *set (const char *key, const char *value);
	Json *set (const StdString &key, const StdString &value);
	Json *set (const char *key, const StdString &value);
	Json *set (const StdString &key, const int value);
	Json *set (const char *key, const int value);
	Json *set (const StdString &key, const int64_t value);
	Json *set (const char *key, const int64_t value);
	Json *set (const StdString &key, const float value);
	Json *set (const char *key, const float value);
	Json *set (const StdString &key, const double value);
	Json *set (const char *key, const double value);
	Json *set (const StdString &key, const bool value);
	Json *set (const char *key, const bool value);
	Json *set (const StdString &key, Json *value);
	Json *set (const char *key, Json *value);
	Json *set (const StdString &key, const StringList &value);
	Json *set (const char *key, const StringList &value);
	Json *set (const StdString &key, std::list<int> *value);
	Json *set (const char *key, std::list<int> *value);
	Json *set (const StdString &key, std::list<int64_t> *value);
	Json *set (const char *key, std::list<int64_t> *value);
	Json *set (const StdString &key, std::list<float> *value);
	Json *set (const char *key, std::list<float> *value);
	Json *set (const StdString &key, std::list<double> *value);
	Json *set (const char *key, std::list<double> *value);
	Json *set (const StdString &key, std::list<bool> *value);
	Json *set (const char *key, std::list<bool> *value);
	Json *set (const StdString &key, std::vector<Json *> *value);
	Json *set (const char *key, std::vector<Json *> *value);
	Json *set (const StdString &key, std::list<Json *> *value);
	Json *set (const char *key, std::list<Json *> *value);
	Json *set (const StdString &key, JsonList *value);
	Json *set (const char *key, JsonList *value);

	// Set a string value in the map using a format string and return the this pointer
	Json *setSprintf (const StdString &key, const char *str, ...) __attribute__((format(printf, 3, 4)));
	Json *setSprintf (const char *key, const char *str, ...) __attribute__((format(printf, 3, 4)));

	// Set a key in the map to a null value and return the this pointer
	Json *setNull (const StdString &key);
	Json *setNull (const char *key);

private:
	// Set the json value to a newly created builder object
	void resetBuilder ();

	// Use the json_object_push method to set a key in the json object, creating the object if needed
	void jsonObjectPush (const json_char *name, json_value *value);

	// Reassign the json pointer, clearing any pointer that might already be present
	void setJsonValue (json_value *value, bool isJsonBuilder);

	// Return a newly created json_value object containing a copy of the provided source value's data
	json_value *copyJsonValue (json_value *sourceValue);

	// Return a boolean value indicating if a value's content matches that of a value from another object
	bool deepEqualsValue (json_value *thisValue, json_value *otherValue);

	json_value *json;
	bool shouldFreeJson;
	bool isJsonBuilder;
};

// Json list class that extends std::list<Json *> and frees all contained Json objects when destroyed
class JsonList : public std::list<Json *> {
public:
	JsonList ();
	virtual ~JsonList ();

	// Remove all elements from the list and free all contained Json objects
	void clear ();

	// Return a string containing the list as a JSON array
	StdString toString ();

	// Remove all elements from the list and add copies of all objects in sourceList
	void copyValues (JsonList *sourceList);

	// Return a newly created JsonList object with contents copied from this object
	JsonList *copy ();

	// Return the Json object at the specified list index, or NULL if no object was found
	Json *at (int index);
};

#endif
