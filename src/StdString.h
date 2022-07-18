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
// String class (extends std::string)

#ifndef STD_STRING_H
#define STD_STRING_H

#include <stdarg.h>
#include <string>
#include <list>

class Buffer;

class StdString : public std::string {
public:
	// Default constructor that creates an empty string
	StdString ();

	// Constructor that initializes the string to the provided value
	StdString (const char *s);

	// Constructor that initializes the string to the provided value
	StdString (const char *s, const int sLength);

	// Constructor that initializes the string to the provided value
	StdString (const std::string &s);

	virtual ~StdString ();

	static const int MaxSprintfLength;

	// Set the string's content using a format string. If the final length is greater than MaxSprintfLength, the content is truncated to fit.
	void sprintf (const char *str, ...) __attribute__((format(printf, 2, 3)));

	// Set the string's content using a format string and an arg list. If the final length is greater than MaxSprintfLength, the content is truncated to fit.
	void vsprintf (const char *str, va_list ap);

	// Append to the string's content using a format string. If the final length of the string to append is greater than MaxSprintfLength, the content is truncated to fit.
	void appendSprintf (const char *str, ...) __attribute__((format(printf, 2, 3)));

	// Append to the string's content using a format string and an arg list. If the final length is greater than MaxSprintfLength, the content is truncated to fit.
	void appendVsprintf (const char *str, va_list ap);

	// Return a boolean value indicating if the string is equal to the provided value
	bool equals (const StdString &value) const;
	bool equals (const char *value) const;
	bool equals (size_t pos, size_t len, const StdString &value) const;
	bool equals (size_t pos, size_t len, const char *value) const;
	bool equals (size_t pos, size_t len, const StdString &value, size_t subpos, size_t sublen) const;

	// Return a boolean value indicating if the string starts with the provided value
	bool startsWith (const StdString &value) const;
	bool startsWith (const char *value) const;

	// Return a boolean value indicating if the string ends with the provided value
	bool endsWith (const StdString &value) const;
	bool endsWith (const char *value) const;

	// Return a boolean value indicating if the string contains the provided value
	bool contains (const StdString &value) const;
	bool contains (const char *value) const;

	// Change all uppercase characters in the string's content to their matching lowercase values
	void lowercase ();

	// Change all uppercase characters in the string's content to their matching lowercase values and return the resulting string
	StdString lowercased () const;

	// Change all lowercase characters in the string's content to their matching uppercase values
	void uppercase ();

	// Change all lowercase characters in the string's content to their matching uppercase values and return the resulting string
	StdString uppercased () const;

	// If the first character of the string's content is a lowercase letter, change it to the matching uppercase value
	void capitalize ();

	// If the first character of the string's content is a lowercase letter, change it to the matching uppercase value and return the resulting string
	StdString capitalized () const;

	// Reduce the string's length to the specified value, appending a truncation suffix if needed
	void truncate (int maxLength, const StdString &suffix = StdString ("..."));

	// Reduce the string's length to the specified value, appending a truncation suffix if needed, and return the resulting value
	StdString truncated (int maxLength, const StdString &suffix = StdString ("...")) const;

	// Remove all instances of oldText and replace them with newText
	void replace (const StdString &oldText, const StdString &newText);

	// Replace the portion of the string that starts at pos and spans len
	void replace (size_t pos, size_t len, const StdString &str);
	void replace (size_t pos, size_t len, size_t n, char c);

	// Return a string resulting from the specified replace operation
	StdString replaced (const StdString &oldText, const StdString &newText) const;
	StdString replaced (size_t pos, size_t len, const StdString &str) const;
	StdString replaced (size_t pos, size_t len, size_t n, char c) const;

	// URL decode the string's content and assign it to the resulting value. Returns a boolean value indicating if the decode succeeded.
	bool urlDecode ();

	// URL decode the string's content and return the resulting value, or an empty string if the URL decode failed
	StdString urlDecoded () const;

	// URL encode the string's content and assign it to the resulting value
	void urlEncode ();

	// URL encode the string's content and return the resulting value
	StdString urlEncoded () const;

	// Base64 encode the provided data and assign the string to the resulting value
	void assignBase64 (const unsigned char *data, int dataLength);

	// Base64 encode the string's content and assign it to the resulting value
	void base64Encode ();

	// Base64 encode the string's content and return the resulting value
	StdString base64Encoded () const;

	// Escape the string's content for use as a JSON string value and assign it to the resulting value
	void jsonEscape ();

	// Escape the string's content for use as a JSON string value and return the resulting value
	StdString jsonEscaped () const;

	// Replace the string's content as needed for use as an identifier and assign it to the resulting value
	void idTranslate ();

	// Replace the string's content as needed for use as an identifier and return the resulting value
	StdString idTranslated () const;

	// Return a newly created Buffer object containing the string's value
	Buffer *createBuffer ();

	// Assign the string's value from data held in a Buffer object
	void assignBuffer (Buffer *buffer);

	// Assign the string's value to a set of hex numbers, as read from the provided data
	void assignHex (const unsigned char *hexData, int hexDataLength);

	// Split the string by the specified delimiter and store the resulting parts into the provided list, clearing the list before doing so
	void split (const char *delimiter, std::list<StdString> *destList) const;
	void split (const StdString &delimiter, std::list<StdString> *destList) const;

	// Overwrite all characters in the string and clear its contents
	void wipe ();

	// Parse the string's content as an integer and store its value in the provided pointer. Returns a boolean value indicating if the parse was successful.
	bool parseInt (int *value) const;

	// Parse the string's content as a hex integer and store its value in the provided pointer. Returns a boolean value indicating if the parse was successful.
	bool parseHex (int *value) const;

	// Parse the string's content as a float and store its value in the provided pointer. Returns a boolean value indicating if the parse was successful.
	bool parseFloat (float *value) const;
	bool parseFloat (double *value) const;

	// Parse the string's content as a network address and store its component values in the provided pointers. Returns a boolean value indicating if the parse was successful. If no port value is found in the address, assign the specified default port value to portValue.
	bool parseAddress (StdString *hostnameValue = NULL, int *portValue = NULL, int defaultPortValue = 0) const;

	// Return a boolean value indicating if the string contains a UUID value
	bool isUuid () const;

	// Return a newly created string with content generated from a format string
	static StdString createSprintf (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Return a newly created string with hex number content generated from the provided data
	static StdString createHex (const unsigned char *hexData, int hexDataLength);
};

#endif
