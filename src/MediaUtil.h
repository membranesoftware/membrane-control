/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Utility methods for operations related to media files and streams

#ifndef MEDIA_UTIL_H
#define MEDIA_UTIL_H

#include "StdString.h"

class MediaUtil {
public:
	static const float AspectRatioMatchEpsilon;

	// Return a string containing the name of the specified aspect ratio, or an empty string if no such name was found.
	static StdString getAspectRatioDisplayString (int width, int height);
	static StdString getAspectRatioDisplayString (float ratio);

	// Return a string containing text representing the specified bitrate in readable format
	static StdString getBitrateDisplayString (int64_t bitsPerSecond);

	// Return a string containing the name of the specified frame size, or an empty string if no such name was found.
	static StdString getFrameSizeName (int width, int height);

	// Return a string containing a description of the specified stream profile constant, or an empty string if no such name was found
	static StdString getStreamProfileDescription (int streamProfile);

	// Return the stream profile constant matching the provided description text, or SystemInterface::Constant_DefaultStreamProfile if no matching constant was found
	static int getStreamProfile (const StdString &description);

	// Return the estimated stream data size for the specified media size and profile, in bytes
	static int64_t getStreamSize (int64_t mediaSize, int streamProfile);
};

#endif
