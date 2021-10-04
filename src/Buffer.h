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
// Object that holds a growable data buffer

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include "OsUtil.h"

class Buffer {
public:
	Buffer ();
	virtual ~Buffer ();

	// Read-only data members
	uint8_t *data;
	int length;

	// Return a newly created Buffer object that has been populated with a copy of this buffer's data
	Buffer *copy ();

	// Free the buffer's underlying memory and reset its size to zero
	void reset ();

	// Return a boolean value indicating if the buffer is empty
	bool empty () const;

	// Add data to the buffer and return a Result value
	OsUtil::Result add (uint8_t *dataPtr, int dataLength);
	OsUtil::Result add (const char *str);

	// Truncate the buffer's data length to the provided value, which must be less than the buffer's current length
	void setDataLength (int dataLength);

	// Advance the buffer read position
	void advanceRead (int advanceSize);

protected:
	static const int DefaultSizeIncrement;

	int size;
	int sizeIncrement;
};

#endif
