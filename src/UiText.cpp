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
#include "StdString.h"
#include "OsUtil.h"
#include "Resource.h"
#include "UiText.h"

UiText *UiText::instance = NULL;
const StdString UiText::DefaultLanguage = StdString ("en");

UiText::UiText () {

}

UiText::~UiText () {

}

OsUtil::Result UiText::load (const StdString &language) {
	Buffer *buffer;
	StdString path, text, s;
	size_t curpos, pos1, pos2;

	path.sprintf ("text/%s.txt", language.c_str ());
	buffer = Resource::instance->loadFile (path);
	if ((! buffer) && (! language.equals (UiText::DefaultLanguage))) {
		path.sprintf ("text/%s.txt", UiText::DefaultLanguage.c_str ());
		buffer = Resource::instance->loadFile (path);
	}
	if (! buffer) {
		return (OsUtil::FileOpenFailedError);
	}
	text.assignBuffer (buffer);
	textStrings.clear ();
	curpos = 0;
	while (true) {
		pos1 = text.find (' ', curpos);
		pos2 = text.find ('\n', curpos);
		if ((pos1 == StdString::npos) || (pos2 == StdString::npos) || (pos1 >= pos2)) {
			break;
		}

		s = text.substr (pos1 + 1, pos2 - pos1 - 1);
		s.replace (StdString ("\\n"), StdString ("\n"));
		textStrings.push_back (s);
		curpos = pos2 + 1;
	}

	Resource::instance->unloadFile (path);
	return (OsUtil::Success);
}

StdString UiText::getText (int stringIndex) {
	int pos;

	pos = ((int) stringIndex) - 1;
	if ((pos < 0) || (pos >= (int) textStrings.size ())) {
		return (StdString (""));
	}
	return (textStrings.at (pos));
}

StdString UiText::getCountText (int64_t amount, int singularStringIndex, int pluralStringIndex) {
	StdString s;

	if ((amount == 1) || (pluralStringIndex < 0)) {
		s = getText (singularStringIndex);
	}
	else {
		s = getText (pluralStringIndex);
	}
	return (StdString::createSprintf ("%lli %s", (long long int) amount, s.c_str ()));
}

StdString UiText::getCountText (int amount, int singularStringIndex, int pluralStringIndex) {
	return (getCountText ((int64_t) amount, singularStringIndex, pluralStringIndex));
}
