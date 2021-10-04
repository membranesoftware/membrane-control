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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <map>
#include "SDL2/SDL.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include "OsUtil.h"
#include "App.h"
#include "Log.h"
#include "StdString.h"
#include "Resource.h"
#include "Buffer.h"
#include "Font.h"

const char *Font::GlyphCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_=+[]{}\\\"';:,.<>/?!@#$%^&*()|";
const StdString Font::DotTruncateSuffix = StdString ("...");

Font::Font (FT_Library freetype, const StdString &name)
: name (name)
, spaceWidth (0)
, maxGlyphWidth (0)
, maxLineHeight (0)
, freetype (freetype)
, isLoaded (false)
{

}

Font::~Font () {
	clearGlyphMap ();
	if (isLoaded) {
		FT_Done_Face (face);
		isLoaded = false;
	}
}

void Font::clearGlyphMap () {
	std::map<char, Font::Glyph>::iterator i, end;

	i = glyphMap.begin ();
	end = glyphMap.end ();
	while (i != end) {
		if (i->second.texture) {
			App::instance->resource.unloadTexture (i->second.texturePath);
			i->second.texture = NULL;
		}
		++i;
	}
	glyphMap.clear ();
}

OsUtil::Result Font::load (Buffer *fontData, int pointSize) {
	Font::Glyph glyph;
	FT_GlyphSlot slot;
	SDL_Surface *surface;
	char *s, c;
	int result, charindex, x, y, w, h, pitch, maxw, maxtopbearing;
	uint8_t *row, *bitmap, alpha;
	Uint32 *pixels, *dest, color, rmask, gmask, bmask, amask;
	std::map<char, Font::Glyph>::iterator i, end;

	result = FT_New_Memory_Face (freetype, (FT_Byte *) fontData->data, fontData->length, 0, &face);
	if (result != 0) {
		Log::err ("Failed to load font; name=\"%s\" err=\"FT_New_Memory_Face: %i\"", name.c_str (), result);
		return (OsUtil::Result::FreetypeOperationFailedError);
	}
	result = FT_Set_Char_Size (face, pointSize << 6, 0, 100, 0);
	if (result != 0) {
		Log::err ("Failed to load font; name=\"%s\" err=\"FT_Set_Char_Size: %i\"", name.c_str (), result);
		return (OsUtil::Result::FreetypeOperationFailedError);
	}

	maxw = 0;
	maxtopbearing = 0;
	s = (char *) Font::GlyphCharacters;
	while (1) {
		c = *s;
		if (c == 0) {
			break;
		}
		++s;

		charindex = FT_Get_Char_Index (face, c);
		result = FT_Load_Glyph (face, charindex, FT_LOAD_RENDER);
		if (result != 0) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"FT_Load_Glyph: %i\"", name.c_str (), c, result);
			continue;
		}

		slot = face->glyph;
		w = slot->bitmap.width;
		h = slot->bitmap.rows;
		if ((w <= 0) || (h <= 0)) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"Invalid bitmap dimensions %ix%i\"", name.c_str (), c, w, h);
			continue;
		}

		pixels = (Uint32 *) malloc (w * h * sizeof (Uint32));
		if (! pixels) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"Out of memory, bitmap dimensions %ix%i\"", name.c_str (), c, w, h);
			continue;
		}

		dest = pixels;
		row = (uint8_t *) slot->bitmap.buffer;
		pitch = slot->bitmap.pitch;
		y = 0;
		while (y < h) {
			bitmap = row;
			x = 0;
			while (x < w) {
				alpha = *bitmap;
				++bitmap;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				color = 0xFFFFFF00 | (alpha & 0xFF);
#else
				color = 0x00FFFFFF | (((Uint32) (alpha & 0xFF)) << 24);
#endif
				*dest = color;
				++dest;
				++x;
			}
			row += pitch;
			++y;
		}
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xFF000000;
		gmask = 0x00FF0000;
		bmask = 0x0000FF00;
		amask = 0x000000FF;
#else
		rmask = 0x000000FF;
		gmask = 0x0000FF00;
		bmask = 0x00FF0000;
		amask = 0xFF000000;
#endif
		surface = SDL_CreateRGBSurfaceFrom (pixels, w, h, 32, w * sizeof (Uint32), rmask, gmask, bmask, amask);
		if (! surface) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"SDL_CreateRGBSurfaceFrom, %s\"", name.c_str (), c, SDL_GetError ());
			free (pixels);
			continue;
		}

		glyph.texturePath.sprintf ("*_Font_%s_%i_%i", name.c_str (), pointSize, (int) c);
		glyph.texture = App::instance->resource.createTexture (glyph.texturePath, surface);
		SDL_FreeSurface (surface);
		free (pixels);
		if (! glyph.texture) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"SDL_CreateTextureFromSurface, %s\"", name.c_str (), c, SDL_GetError ());
			continue;
		}

		glyph.width = w;
		glyph.height = h;
		glyph.leftBearing = (int) slot->bitmap_left;
		glyph.topBearing = (int) slot->bitmap_top;
		glyph.advanceWidth = (int) ((FT_CeilFix (slot->linearHoriAdvance) >> 16) & 0xFFFF);
		glyphMap.insert (std::pair<char, Font::Glyph> (c, glyph));
		if (w > maxw) {
			maxw = w;
		}
		if ((maxtopbearing <= 0) || (glyph.topBearing > maxtopbearing)) {
			maxtopbearing = glyph.topBearing;
		}
	}
	if (face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) {
		spaceWidth = maxw;
	}
	else {
		spaceWidth = (maxw / 3);
	}

	maxGlyphWidth = 0;
	maxLineHeight = 0;
	i = glyphMap.begin ();
	end = glyphMap.end ();
	while (i != end) {
		if (i->second.width > maxGlyphWidth) {
			maxGlyphWidth = i->second.width;
		}

		h = maxtopbearing - i->second.topBearing + i->second.height;
		if (h > maxLineHeight) {
			maxLineHeight = h;
		}

		++i;
	}

	isLoaded = true;
	return (OsUtil::Result::Success);
}

Font::Glyph *Font::getGlyph (char glyphCharacter) {
	std::map<char, Font::Glyph>::iterator i;

	i = glyphMap.find (glyphCharacter);
	if (i == glyphMap.end ()) {
		return (NULL);
	}
	return (&(i->second));
}

int Font::getKerning (char leftCharacter, char rightCharacter) {
	int leftindex, rightindex;
	FT_Vector vector;

	leftindex = FT_Get_Char_Index (face, leftCharacter);
	rightindex = FT_Get_Char_Index (face, rightCharacter);
	FT_Get_Kerning (face, leftindex, rightindex, FT_KERNING_DEFAULT, &vector);
	return (vector.x >> 6);
}

void Font::resetMetrics (Font::Metrics *metrics, const StdString &text, int textPosition) {
	metrics->text.assign (text);
	metrics->textLength = (int) metrics->text.length ();
	metrics->textPosition = 0;
	metrics->lastCharacter = '\0';
	metrics->textWidth = 0.0f;
	metrics->isComplete = false;

	if (textPosition < 0) {
		textPosition = metrics->textLength;
	}
	if (textPosition > 0) {
		advanceMetrics (metrics, textPosition);
	}
	if (metrics->textPosition >= metrics->textLength) {
		metrics->isComplete = true;
	}
}

void Font::advanceMetrics (Font::Metrics *metrics, int advanceLength) {
	Font::Glyph *glyph;
	int i, kerning;
	char *buf, c;

	if (advanceLength <= 0) {
		return;
	}
	if (metrics->isComplete || (metrics->textPosition < 0) || (metrics->textPosition >= metrics->textLength)) {
		return;
	}

	buf = (char *) metrics->text.c_str ();
	for (i = 0; i < advanceLength; ++i) {
		if (metrics->textPosition >= metrics->textLength) {
			break;
		}
		c = buf[metrics->textPosition];
		glyph = getGlyph (c);

		if (metrics->lastCharacter != '\0') {
			kerning = getKerning (metrics->lastCharacter, c);
			metrics->textWidth += (float) kerning;
		}
		metrics->lastCharacter = c;

		if (! glyph) {
			metrics->textWidth += (float) spaceWidth;
		}
		else {
			if (metrics->textPosition == (metrics->textLength - 1)) {
				metrics->textWidth += (float) glyph->leftBearing;
				metrics->textWidth += (float) glyph->width;
			}
			else {
				metrics->textWidth += (float) glyph->advanceWidth;
			}
		}

		++(metrics->textPosition);
	}

	if (metrics->textPosition >= metrics->textLength) {
		metrics->isComplete = true;
	}
}

void Font::truncateText (StdString *text, float maxWidth, const StdString &truncateSuffix) {
	Font::Glyph *glyph;
	float x, spacew, suffixw;
	char *buf, c, lastc, suffixc;
	int i, textlen, truncatepos, suffixkerning;

	spacew = (float) spaceWidth;
	suffixc = 0;
	x = 0.0f;
	lastc = 0;
	buf = (char *) truncateSuffix.c_str ();
	textlen = truncateSuffix.length ();
	for (i = 0; i < textlen; ++i) {
		c = buf[i];
		if (suffixc <= 0) {
			suffixc = c;
		}
		glyph = getGlyph (c);
		if (i > 0) {
			x += getKerning (lastc, c);
		}
		lastc = c;

		if (! glyph) {
			if (i < (textlen - 1)) {
				x += spacew;
			}
		}
		else {
			if (i == (textlen - 1)) {
				x += glyph->leftBearing;
				x += glyph->width;
			}
			else {
				x += glyph->advanceWidth;
			}
		}
	}
	suffixw = x;

	truncatepos = 0;
	x = 0.0f;
	lastc = 0;
	buf = (char *) text->c_str ();
	textlen = text->length ();
	for (i = 0; i < textlen; ++i) {
		c = buf[i];
		glyph = getGlyph (c);
		if (i > 0) {
			x += getKerning (lastc, c);
		}
		lastc = c;

		if (! glyph) {
			if (i < (textlen - 1)) {
				x += spacew;
			}
		}
		else {
			if (i == (textlen - 1)) {
				x += glyph->leftBearing;
				x += glyph->width;
			}
			else {
				x += glyph->advanceWidth;
			}
		}

		suffixkerning = 0;
		if (suffixc > 0) {
			suffixkerning = getKerning (c, suffixc);
		}

		if ((x + suffixkerning + suffixw) <= maxWidth) {
			truncatepos = i;
		}
		if (x > maxWidth) {
			text->assign (text->substr (0, truncatepos + 1));
			text->append (truncateSuffix);
			break;
		}
	}
}

StdString Font::truncatedText (const StdString &text, float maxWidth, const StdString &truncateSuffix) {
	StdString s;

	s.assign (text);
	truncateText (&s, maxWidth, truncateSuffix);
	return (s);
}
