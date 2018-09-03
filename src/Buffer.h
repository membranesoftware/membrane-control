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
// Object that holds a growable data buffer

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

class Buffer {
public:
	Buffer ();
	~Buffer ();

	static const int defaultSizeIncrement;

	// Set the buffer to persistent mode. If enabled, the buffer's underlying data is not freed when the buffer object is destroyed.
	void setPersistent ();

	// Free the buffer's underlying memory and reset its size to zero
	void reset ();

	// Return a boolean value indicating if the buffer is empty
	bool empty () const;

	// Add data to the buffer. Returns a Result value.
	int add (uint8_t *dataPtr, int dataLength);
	int add (const char *str);

	// Return the buffer's data and length using the provided pointers
	void getData (uint8_t **dataPtr, int *dataLength);
	void getData (char **dataPtr, int *dataLength);

	// Truncate the buffer's data length to the provided value, which must be less than the buffer's current length
	void setDataLength (int dataLength);

	// Advance the buffer read position
	void advanceRead (int advanceSize);

protected:
	uint8_t *data;
	int length;
	int size;
	int sizeIncrement;
	bool isPersistent;
};

#endif
