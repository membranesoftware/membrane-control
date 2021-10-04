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
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include "Buffer.h"
#include "StdString.h"

const int StdString::MaxSprintfLength = (64 * 1024); // bytes

StdString::StdString ()
: std::string ()
{

}

StdString::StdString (const char *s)
: std::string (s)
{

}

StdString::StdString (const char *s, const int sLength)
: std::string (s, sLength)
{

}

StdString::StdString (const std::string &s)
: std::string (s)
{

}

StdString::~StdString () {

}

void StdString::sprintf (const char *str, ...) {
	va_list ap;
	char buf[StdString::MaxSprintfLength];

	va_start (ap, str);
	vsnprintf (buf, sizeof (buf), str, ap);
	va_end (ap);
	assign (buf);
}

void StdString::vsprintf (const char *str, va_list ap) {
	char buf[StdString::MaxSprintfLength];

	vsnprintf (buf, sizeof (buf), str, ap);
	assign (buf);
}

void StdString::appendSprintf (const char *str, ...) {
	va_list ap;
	char buf[StdString::MaxSprintfLength];

	va_start (ap, str);
	vsnprintf (buf, sizeof (buf), str, ap);
	va_end (ap);
	append (buf);
}

void StdString::appendVsprintf (const char *str, va_list ap) {
	char buf[StdString::MaxSprintfLength];

	vsnprintf (buf, sizeof (buf), str, ap);
	append (buf);
}

bool StdString::equals (const StdString &value) const {
	return (! compare (value));
}

bool StdString::equals (const char *value) const {
	return (! compare (value));
}

bool StdString::equals (size_t pos, size_t len, const StdString &value) const {
	return (! compare (pos, len, value));
}

bool StdString::equals (size_t pos, size_t len, const char *value) const {
	return (! compare (pos, len, value));
}

bool StdString::equals (size_t pos, size_t len, const StdString &value, size_t subpos, size_t sublen) const {
	return (! compare (pos, len, value, subpos, sublen));
}

bool StdString::startsWith (const StdString &value) const {
	return (find (value) == 0);
}

bool StdString::startsWith (const char *value) const {
	return (find (value) == 0);
}

bool StdString::endsWith (const StdString &value) const {
	return ((value.length () <= length ()) && (find (value) == (length () - value.length ())));
}

bool StdString::endsWith (const char *value) const {
	return (endsWith (StdString (value)));
}

bool StdString::contains (const StdString &value) const {
	return (find (value) != StdString::npos);
}

bool StdString::contains (const char *value) const {
	return (find (value) != StdString::npos);
}

void StdString::lowercase () {
	assign (lowercased ());
}

StdString StdString::lowercased () const {
	StdString s;
	int len;
	char c, *ptr, *end;

	len = length ();
	ptr = (char *) c_str ();
	end = ptr + len;
	while (ptr < end) {
		c = *ptr;
		s.append (1, tolower (c));
		++ptr;
	}

	return (s);
}

void StdString::uppercase () {
	assign (uppercased ());
}

StdString StdString::uppercased () const {
	StdString s;
	int len;
	char c, *ptr, *end;

	len = length ();
	ptr = (char *) c_str ();
	end = ptr + len;
	while (ptr < end) {
		c = *ptr;
		s.append (1, toupper (c));
		++ptr;
	}

	return (s);
}

void StdString::capitalize () {
	char c;

	if (length () < 1) {
		return;
	}
	c = at (0);
	if (! islower (c)) {
		return;
	}

	std::string::replace (0, 1, 1, toupper (c));
}

StdString StdString::capitalized () const {
	StdString s;

	s.assign (c_str ());
	s.capitalize ();

	return (s);
}

void StdString::truncate (int maxLength, const StdString &suffix) {
	int len;

	len = (int) length ();
	if (len <= maxLength) {
		return;
	}

	len = maxLength;
	len -= suffix.length ();
	if (len <= 0) {
		assign (suffix);
		return;
	}

	assign (substr (0, len));
	append (suffix);
}

StdString StdString::truncated (int maxLength, const StdString &suffix) const {
	StdString s;

	s.assign (c_str ());
	s.truncate (maxLength, suffix);

	return (s);
}

void StdString::replace (const StdString &oldText, const StdString &newText) {
	size_t curpos, pos, oldtextlen, newtextlen;

	oldtextlen = oldText.length ();
	if (oldtextlen <= 0) {
		return;
	}

	newtextlen = newText.length ();
	curpos = 0;
	while (true) {
		pos = find (oldText, curpos);
		if (pos == StdString::npos) {
			break;
		}

		std::string::replace (pos, oldtextlen, newText);
		curpos = pos + newtextlen;
	}
}

void StdString::replace (size_t pos, size_t len, const StdString &str) {
	std::string::replace (pos, len, str);
}

void StdString::replace (size_t pos, size_t len, size_t n, char c) {
	std::string::replace (pos, len, n, c);
}

StdString StdString::replaced (const StdString &oldText, const StdString &newText) const {
	StdString s;

	s.assign (c_str ());
	s.replace (oldText, newText);

	return (s);
}

StdString StdString::replaced (size_t pos, size_t len, const StdString &str) const {
	StdString s;

	s.assign (c_str ());
	s.replace (pos, len, str);

	return (s);
}

StdString StdString::replaced (size_t pos, size_t len, size_t n, char c) const {
	StdString s;

	s.assign (c_str ());
	s.replace (pos, len, n, c);

	return (s);
}

bool StdString::urlDecode () {
	StdString s;
	char *d, *end, c;
	int code, codechars;
	bool result;

	result = true;
	s.assign ("");
	d = (char *) c_str ();
	end = d + length ();
	code = 0;
	codechars = 0;
	while (d < end) {
		c = *d;

		if (codechars <= 0) {
			if (c == '%') {
				codechars = 1;
			}
			else {
				s.append (1, c);
			}
		}
		else {
			code <<= 4;
			if ((c >= '0') && (c <= '9')) {
				code |= ((c - '0') & 0x0F);
				++codechars;
			}
			else if ((c >= 'a') && (c <= 'f')) {
				code |= ((c - 'a' + 10) & 0x0F);
				++codechars;
			}
			else if ((c >= 'A') && (c <= 'F')) {
				code |= ((c - 'A' + 10) & 0x0F);
				++codechars;
			}
			else {
				result = false;
				break;
			}

			if (codechars >= 3) {
				s.append (1, (char) (code & 0xFF));
				codechars = 0;
			}

		}
		++d;
	}

	if (result) {
		assign (s.c_str ());
	}
	return (result);
}

StdString StdString::urlDecoded () const {
	StdString s;

	s.assign (c_str ());
	if (! s.urlDecode ()) {
		return (StdString (""));
	}
	return (s);
}

void StdString::urlEncode () {
	StdString s;
	char *d, *end, c;

	s.assign ("");
	d = (char *) c_str ();
	end = d + length ();
	while (d < end) {
		c = *d;

		switch (c) {
			case '!':
			case '#':
			case '$':
			case '&':
			case '\'':
			case '(':
			case ')':
			case '*':
			case '+':
			case ',':
			case '/':
			case ':':
			case ';':
			case '=':
			case '?':
			case '@':
			case '[':
			case ']':
			case '%':
			case '"':
			case '{':
			case '}':
			case ' ':
				s.appendSprintf ("%%%02X", c);
				break;
			default:
				s.append (1, c);
				break;
		}

		++d;
	}

	assign (s.c_str ());
}

StdString StdString::urlEncoded () const {
	StdString s;

	s.assign (c_str ());
	s.urlEncode ();

	return (s);
}

void StdString::jsonEscape () {
	StdString s;
	char *d, *end, c;

	s.assign ("");
	d = (char *) c_str ();
	end = d + length ();
	while (d < end) {
		c = *d;

		if (c == '"') {
			s.append (1, '\\');
		}
		s.append (1, c);

		++d;
	}

	assign (s.c_str ());
}

StdString StdString::jsonEscaped () const {
	StdString s;

	s.assign (c_str ());
	s.jsonEscape ();

	return (s);
}

void StdString::idTranslate () {
	assign (idTranslated ());
}

StdString StdString::idTranslated () const {
	StdString s;
	char c, *ptr, *end;
	int len;

	len = (int) length ();
	s.reserve (len);
	ptr = (char *) c_str ();
	end = ptr + len;
	while (ptr < end) {
		c = *ptr;
		if (! isalnum (c)) {
			c = '_';
		}
		else {
			c = tolower (c);
		}
		s.append (1, c);
		++ptr;
	}

	return (s);
}

StdString StdString::createSprintf (const char *str, ...) {
	va_list ap;
	char buf[StdString::MaxSprintfLength];

	va_start (ap, str);
	vsnprintf (buf, sizeof (buf), str, ap);
	va_end (ap);

	return (StdString (buf));
}

StdString StdString::createHex (const unsigned char *hexData, int hexDataLength) {
	StdString s;

	s.assignHex (hexData, hexDataLength);

	return (s);
}

bool StdString::parseInt (int *value) const {
	char *s, c;

	if (length () <= 0) {
		return (false);
	}

	s = (char *) c_str ();
	c = *s;
	while (c) {
		if (! isdigit (c) && (c != '-')) {
			return (false);
		}

		++s;
		c = *s;
	}

	if (value) {
		*value = atoi (c_str ());
	}

	return (true);
}

bool StdString::parseHex (int *value) const {
	int i;
	char *s, c;

	if (length () <= 0) {
		return (false);
	}

	i = 0;
	s = (char *) c_str ();
	while (1) {
		c = *s;
		if (! c) {
			break;
		}

		i <<= 4;
		if ((c >= '0') && (c <= '9')) {
			i |= ((c - '0') & 0x0F);
		}
		else if ((c >= 'a') && (c <= 'f')) {
			i |= ((c - 'a' + 10) & 0x0F);
		}
		else if ((c >= 'A') && (c <= 'F')) {
			i |= ((c - 'A' + 10) & 0x0F);
		}
		else {
			return (false);
		}

		++s;
	}

	if (value) {
		*value = i;
	}

	return (true);
}

bool StdString::parseFloat (float *value) const {
	char *s, c;

	if (length () <= 0) {
		return (false);
	}

	s = (char *) c_str ();
	c = *s;
	while (c) {
		if ((! isdigit (c)) && (c != '.') && (c != '-')) {
			return (false);
		}

		++s;
		c = *s;
	}

	if (value) {
		*value = strtof (c_str (), NULL);
	}

	return (true);
}

bool StdString::parseFloat (double *value) const {
	char *s, c;

	if (length () <= 0) {
		return (false);
	}

	s = (char *) c_str ();
	c = *s;
	while (c) {
		if ((! isdigit (c)) && (c != '.') && (c != '-')) {
			return (false);
		}

		++s;
		c = *s;
	}

	if (value) {
		*value = strtod (c_str (), NULL);
	}

	return (true);
}

bool StdString::parseAddress (StdString *hostnameValue, int *portValue, int defaultPortValue) const {
	StdString hostname;
	int port;
	size_t pos;

	if (length () <= 0) {
		return (false);
	}

	port = defaultPortValue;
	pos = find (":");
	if (pos == StdString::npos) {
		hostname.assign (c_str ());
	}
	else {
		if (! StdString (substr (pos + 1)).parseInt (&port)) {
			return (false);
		}
		if ((port <= 0) || (port > 65535)) {
			return (false);
		}

		hostname.assign (substr (0, pos));
	}

	if (hostnameValue) {
		hostnameValue->assign (hostname);
	}
	if (portValue) {
		*portValue = port;
	}
	return (true);
}

bool StdString::isUuid () const {
	int i, len;
	char c;

	len = (int) length ();
	if (len != 36) {
		return (false);
	}
	for (i = 0; i < len; ++i) {
		c = at (i);
		if ((i == 8) || (i == 13) || (i == 18) || (i == 23)) {
			if (c != '-') {
				return (false);
			}
		}
		else {
			if ((c >= '0') && (c <= '9')) {
				continue;
			}
			if ((c >= 'a') && (c <= 'f')) {
				continue;
			}
			if ((c >= 'A') && (c <= 'F')) {
				continue;
			}
			return (false);
		}
	}
	return (true);
}

Buffer *StdString::createBuffer () {
	Buffer *buffer;

	buffer = new Buffer ();
	buffer->add ((uint8_t *) c_str (), length ());

	return (buffer);
}

void StdString::assignBuffer (Buffer *buffer) {
	if (! buffer) {
		assign ("");
		return;
	}
	assign ((char *) buffer->data, buffer->length);
}

void StdString::assignHex (const unsigned char *hexData, int hexDataLength) {
	unsigned char *d, *end;

	if ((! hexData) || (hexDataLength <= 0)) {
		return;
	}
	assign ("");
	d = (unsigned char *) hexData;
	end = d + hexDataLength;
	while (d < end) {
		appendSprintf ("%02x", *d);
		++d;
	}
}

void StdString::wipe () {
	StdString::iterator i1, i2;

	i1 = begin ();
	i2 = end ();
	while (i1 != i2) {
		*i1 = ' ';
		++i1;
	}

	clear ();
}

void StdString::split (const char *delimiter, std::list<StdString> *destList) const {
	split (StdString (delimiter), destList);
}

void StdString::split (const StdString &delimiter, std::list<StdString> *destList) const {
	size_t curpos, pos, delimlen, len;

	if (! destList) {
		return;
	}

	destList->clear ();
	curpos = 0;
	delimlen = delimiter.length ();
	if (delimlen <= 0) {
		destList->push_back (StdString (c_str ()));
		return;
	}

	len = length ();
	while (true) {
		if (curpos >= len) {
			break;
		}
		pos = find (delimiter, curpos);
		if (pos == StdString::npos) {
			break;
		}
		destList->push_back (StdString (substr (curpos, pos - curpos)));
		curpos = pos + delimlen;
	}

	if (curpos <= len) {
		destList->push_back (StdString (substr (curpos)));
	}
}
