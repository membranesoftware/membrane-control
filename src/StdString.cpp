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
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include "Result.h"
#include "Buffer.h"
#include "StdString.h"

const int StdString::maxSprintfLength = (64 * 1024); // bytes

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
	char buf[StdString::maxSprintfLength];

	va_start (ap, str);
	vsnprintf (buf, sizeof (buf), str, ap);
	va_end (ap);
	assign (buf);
}

void StdString::vsprintf (const char *str, va_list ap) {
	char buf[StdString::maxSprintfLength];

	vsnprintf (buf, sizeof (buf), str, ap);
	assign (buf);
}

void StdString::appendSprintf (const char *str, ...) {
	va_list ap;
	char buf[StdString::maxSprintfLength];

	va_start (ap, str);
	vsnprintf (buf, sizeof (buf), str, ap);
	va_end (ap);
	append (buf);
}

void StdString::appendVsprintf (const char *str, va_list ap) {
	char buf[StdString::maxSprintfLength];

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
	return (find (value) == (length () - value.length ()));
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

StdString StdString::replaced (const StdString &oldText, const StdString &newText) {
	StdString s;

	s.assign (c_str ());
	s.replace (oldText, newText);

	return (s);
}

int StdString::urlDecode () {
	StdString s;
	char *d, *end, c;
	int rval, code, codechars;

	rval = Result::SUCCESS;
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
				rval = Result::ERROR_MALFORMED_DATA;
				break;
			}

			if (codechars >= 3) {
				s.append (1, (char) (code & 0xFF));
				codechars = 0;
			}

		}
		++d;
	}

	if (rval == Result::SUCCESS) {
		assign (s.c_str ());
	}
	return (rval);
}

StdString StdString::urlDecoded () const {
	StdString s;
	int result;

	s.assign (c_str ());
	result = s.urlDecode ();
	if (result != Result::SUCCESS) {
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
	char buf[StdString::maxSprintfLength];

	va_start (ap, str);
	vsnprintf (buf, sizeof (buf), str, ap);
	va_end (ap);

	return (StdString (buf));
}

bool StdString::parseInt (const char *str, int *value) {
	char *s, c;

	s = (char *) str;
	c = *s;
	while (c) {
		if (! isdigit (c) && (c != '-')) {
			return (false);
		}

		++s;
		c = *s;
	}

	if (value) {
		*value = atoi (str);
	}

	return (true);
}

bool StdString::parseFloat (const char *str, float *value) {
	char *s, c;

	s = (char *) str;
	c = *s;
	while (c) {
		if ((! isdigit (c)) && (c != '.') && (c != '-')) {
			return (false);
		}

		++s;
		c = *s;
	}

	if (value) {
		*value = strtof (str, NULL);
	}

	return (true);
}

bool StdString::parseAddress (const char *str, StdString *hostnameValue, int *portValue, int defaultPortValue) {
	StdString s, hostname;
	int port;
	size_t pos;

	s.assign (str);
	if (s.empty ()) {
		return (false);
	}

	port = defaultPortValue;
	pos = s.find (":");
	if (pos == StdString::npos) {
		hostname.assign (s);
	}
	else {
		if (! StdString::parseInt (s.substr (pos + 1).c_str (), &port)) {
			return (false);
		}
		if ((port <= 0) || (port > 65535)) {
			return (false);
		}

		hostname.assign (s.substr (0, pos));
	}

	if (hostnameValue) {
		hostnameValue->assign (hostname);
	}
	if (portValue) {
		*portValue = port;
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
	assign ((char *) buffer->data, buffer->length);
}

void StdString::split (const char *delimiter, std::list<StdString> *destList) {
	split (StdString (delimiter), destList);
}

void StdString::split (const StdString &delimiter, std::list<StdString> *destList) {
	size_t curpos, pos, delimlen, len;

	destList->clear ();
	curpos = 0;
	delimlen = delimiter.length ();
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
