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
#include <string.h>
#include "OsUtil.h"
#include "Buffer.h"

const int Buffer::DefaultSizeIncrement = 1024; // bytes

Buffer::Buffer ()
: data (NULL)
, length (0)
, size (0)
, sizeIncrement (Buffer::DefaultSizeIncrement)
{

}

Buffer *Buffer::copy () {
	Buffer *buffer;

	buffer = new Buffer ();
	if (! empty ()) {
		buffer->add (data, length);
	}
	return (buffer);
}

Buffer::~Buffer () {
	if (data) {
		free (data);
		data = NULL;
	}
}

void Buffer::reset () {
	if (data) {
		free (data);
		data = NULL;
	}
	length = 0;
	size = 0;
}

bool Buffer::empty () const {
	if (data && (length > 0)) {
		return (false);
	}
	return (true);
}

OsUtil::Result Buffer::add (uint8_t *dataPtr, int dataLength) {
	int sz, diff, blocks, incr, sz2;

	sz = length + dataLength;
	diff = sz - size;
	if (diff > 0) {
		incr = sizeIncrement;
		blocks = (diff / incr);
		if (diff % incr) {
			++blocks;
		}

		sz2 = size + (blocks * incr);
		data = (uint8_t *) realloc (data, sz2);
		if (! data) {
			return (OsUtil::OutOfMemoryError);
		}
		size = sz2;
	}

	memcpy (data + length, dataPtr, dataLength);
	length += dataLength;
	return (OsUtil::Success);
}

OsUtil::Result Buffer::add (const char *str) {
	return (add ((uint8_t *) str, (int) strlen (str)));
}

void Buffer::setDataLength (int dataLength) {
	if ((dataLength < 0) || (dataLength >= length)) {
		return;
	}
	length = dataLength;
}

void Buffer::advanceRead (int advanceSize) {
	int len;

	if (advanceSize <= 0) {
		return;
	}
	if (advanceSize >= length) {
		length = 0;
		return;
	}
	len = length - advanceSize;
	memmove (data, data + advanceSize, len);
	length = len;
}
