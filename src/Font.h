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
// Class that handles operations with fonts (uses freetype)

#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <map>
#include "SDL2/SDL.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include "StdString.h"
#include "Buffer.h"

#define FONT_CHARACTERS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_=+[]{}\\\"';:,.<>/?!@#$%^&*()|"

class Font {
public:
	struct _glyph {
		SDL_Texture *texture;
		StdString texturePath;
		int width, height;
		int leftBearing;
		int topBearing;
		int advanceWidth;
	};
	typedef struct _glyph Glyph;

	Font (FT_Library freetype, const StdString &name);
	~Font ();

	// Read-only data members
	StdString name;
	int spaceWidth;
	int maxGlyphWidth;
	int maxLineHeight;

	// Load a font using the specified data buffer and point size. Returns a Result value.
	int load (Buffer *fontData, int pointSize);

	// Return a pointer to a Font::Glyph struct for the specified character, or NULL if no such glyph was found
	Font::Glyph *getGlyph (char glyphCharacter);

	// Return the kerning value that should be used between the two specified characters
	int getKerning (char leftCharacter, char rightCharacter);

private:
	// Remove all items from the glyph map
	void clearGlyphMap ();

	FT_Library freetype;
	FT_Face face;
	bool isLoaded;
	std::map<char, Font::Glyph> glyphMap;
};

#endif
