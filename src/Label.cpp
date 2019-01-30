/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
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
#include <stdio.h>
#include <string.h>
#include "SDL2/SDL.h"
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "Sprite.h"
#include "Resource.h"
#include "Font.h"
#include "Widget.h"
#include "Label.h"

static const char OBSCURE_CHARACTER = '*';

Label::Label (const StdString &text, int fontType, const Color &color)
: Widget ()
, textColor (color)
, textFontType (-1)
, textFont (NULL)
, textFontSize (0)
, spaceWidth (0.0f)
, maxGlyphWidth (0.0f)
, maxLineHeight (0.0f)
, maxCharacterHeight (0.0f)
, descenderHeight (0.0f)
, isUnderlined (false)
, isObscured (false)
, maxGlyphTopBearing (0)
, underlineMargin (0.0f)
, textMutex (NULL)
{
	widgetType.assign ("Label");
	textMutex = SDL_CreateMutex ();
	setText (text, fontType);
}

Label::~Label () {
	Resource *resource;

	if (isObscured) {
		text.wipe ();
	}

	if (textMutex) {
		SDL_LockMutex (textMutex);
		glyphList.clear ();
		kerningList.clear ();
		SDL_UnlockMutex (textMutex);
	}

	if (textFont) {
		resource = &(App::getInstance ()->resource);
		resource->unloadFont (textFontName, textFontSize);
		textFont = NULL;
	}

	if (textMutex) {
		SDL_DestroyMutex (textMutex);
		textMutex = NULL;
	}
}

StdString Label::toStringDetail () {
	return (StdString::createSprintf (" \"%s\" fontName=\"%s\" fontSize=%i", text.c_str (), textFontName.c_str (), textFontSize));
}

float Label::getLinePosition (float targetY) {
	float y;

	y = targetY + maxLineHeight - maxCharacterHeight + descenderHeight;

	return (y);
}

void Label::setUnderlined (bool enable) {
	UiConfiguration *uiconfig;

	if (isUnderlined == enable) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	isUnderlined = enable;
	underlineMargin = uiconfig->textUnderlineMargin;
	if (isUnderlined) {
		height = maxGlyphTopBearing + underlineMargin + 1.0f;
	}
	else {
		height = maxCharacterHeight;
	}
}

void Label::setObscured (bool enable) {
	if (isObscured == enable) {
		return;
	}
	isObscured = enable;
	setText (text, textFontType, true);
}

void Label::doDraw () {
	App *app;
	Font::Glyph *glyph;
	std::list<Font::Glyph *>::iterator i, end;
	std::list<int>::iterator ki, kend;
	SDL_Rect rect;
	int x, y, kerning;
	bool first;

	SDL_LockMutex (textMutex);
	if (glyphList.empty ()) {
		SDL_UnlockMutex (textMutex);
		return;
	}

	app = App::getInstance ();
	x = 0;
	y = 0;
	kerning = 0;
	first = true;
	ki = kerningList.begin ();
	kend = kerningList.end ();
	i = glyphList.begin ();
	end = glyphList.end ();
	while (i != end) {
		glyph = *i;

		if ((! first) && (ki != kend)) {
			kerning = *ki;
		}
		else {
			kerning = 0;
		}

		if (! glyph) {
			x += (int) spaceWidth;
		}
		else {
			rect.x = x + (int) drawX + glyph->leftBearing + kerning;
			rect.y = y + (int) drawY + maxGlyphTopBearing - glyph->topBearing;
			if (((rect.x + glyph->advanceWidth) >= 0) && (rect.x < app->windowWidth) && ((rect.y + maxGlyphTopBearing) >= 0) && (rect.y < app->windowHeight)) {
				rect.w = glyph->width;
				rect.h = glyph->height;
				SDL_SetTextureColorMod (glyph->texture, textColor.rByte, textColor.gByte, textColor.bByte);
				SDL_RenderCopy (app->render, glyph->texture, NULL, &rect);
			}

			x += glyph->advanceWidth;
		}

		++i;

		if (first) {
			first = false;
		}
		else {
			if (ki != kend) {
				++ki;
			}
		}
	}

	if (isUnderlined) {
		y = (int) (drawY + maxGlyphTopBearing + underlineMargin);
		SDL_SetRenderDrawColor (app->render, textColor.rByte, textColor.gByte, textColor.bByte, 255);
		SDL_RenderDrawLine (app->render, (int) drawX, y, (int) (drawX + width), y);
	}
	SDL_UnlockMutex (textMutex);
}

void Label::doRefresh () {
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	if (textFont && (textFontType >= 0)) {
		if ((! textFontName.equals (uiconfig->fontNames[textFontType])) || (textFontSize != uiconfig->fontSizes[textFontType])) {
			setText (text, textFontType, true);
		}
	}
}

void Label::doUpdate (int msElapsed, float originX, float originY) {
	textColor.update (msElapsed);
}

void Label::setText (const StdString &labelText, int fontType, bool forceFontReload) {
	UiConfiguration *uiconfig;
	Resource *resource;
	Font *font;
	Font::Glyph *glyph;
	int i, maxbearing, textlen, kerning, maxh, h, descenderh;
	float x;
	char *buf, c, lastc;

	uiconfig = &(App::getInstance ()->uiConfig);
	font = NULL;
	if (fontType >= 0) {
		if (forceFontReload || (! textFont) || (textFontType != fontType)) {
			resource = &(App::getInstance ()->resource);
			font = resource->loadFont (uiconfig->fontNames[fontType], uiconfig->fontSizes[fontType]);
			if (font) {
				if (textFont) {
					resource->unloadFont (textFontName, textFontSize);
				}
				textFont = font;
				textFontType = fontType;
				textFontName.assign (uiconfig->fontNames[fontType]);
				textFontSize = uiconfig->fontSizes[fontType];
				spaceWidth = (float) textFont->spaceWidth;
				maxGlyphWidth = (float) textFont->maxGlyphWidth;
				maxLineHeight = (float) textFont->maxLineHeight;
			}
		}
	}
	if (! textFont) {
		return;
	}
	if ((! font) && text.equals (labelText)) {
		return;
	}

	SDL_LockMutex (textMutex);
	text.assign (labelText);
	glyphList.clear ();
	kerningList.clear ();
	textlen = text.length ();
	if (textlen <= 0) {
		width = 0.0f;
		height = 0.0f;
		maxCharacterHeight = 0.0f;
		descenderHeight = 0.0f;
		SDL_UnlockMutex (textMutex);
		return;
	}

	lastc = 0;
	x = 0.0f;
	descenderh = 0;
	maxbearing = -1;
	buf = (char *) text.c_str ();
	for (i = 0; i < textlen; ++i) {
		c = isObscured ? OBSCURE_CHARACTER : buf[i];
		glyph = textFont->getGlyph (c);
		glyphList.push_back (glyph);

		if (i > 0) {
			kerning = textFont->getKerning (lastc, c);
			kerningList.push_back (kerning);
			x += kerning;
		}
		lastc = c;

		if (! glyph) {
			if (i < (textlen - 1)) {
				x += spaceWidth;
			}
		}
		else {
			if ((maxbearing < 0) || (glyph->topBearing > maxbearing)) {
				maxbearing = glyph->topBearing;
			}

			if (i == (textlen - 1)) {
				x += glyph->leftBearing;
				x += glyph->width;
			}
			else {
				x += glyph->advanceWidth;
			}

			h = glyph->height - glyph->topBearing;
			if (h > descenderh) {
				descenderh = h;
			}
		}
	}
	maxGlyphTopBearing = maxbearing;
	descenderHeight = (float) descenderh;

	maxh = 0;
	for (i = 0; i < textlen; ++i) {
		c = isObscured ? OBSCURE_CHARACTER : buf[i];
		glyph = textFont->getGlyph (c);
		if (glyph) {
			h = maxGlyphTopBearing - glyph->topBearing + glyph->height;
			if (h > maxh) {
				maxh = h;
			}
		}
	}

	width = x;
	maxCharacterHeight = (float) maxh;
	if (isUnderlined) {
		height = maxGlyphTopBearing + underlineMargin + 1.0f;
	}
	else {
		height = maxCharacterHeight;
	}
	SDL_UnlockMutex (textMutex);
}

void Label::setFont (int fontType) {
	if (fontType == textFontType) {
		return;
	}

	setText (text, fontType);
}

void Label::truncateText (StdString *text, int fontType, float maxWidth, const StdString &truncateSuffix) {
	StdString s;
	Label *label;
	bool istruncated;

	istruncated = false;
	s.assign (*text);
	label = new Label (s, fontType);
	while (label->width > maxWidth) {
		if (s.empty ()) {
			break;
		}

		istruncated = true;
		s = s.substr (0, s.length () - 1);
		label->setText (StdString::createSprintf ("%s%s", s.c_str (), truncateSuffix.c_str ()));
	}

	if (istruncated) {
		text->sprintf ("%s%s", s.c_str (), truncateSuffix.c_str ());
	}
	delete (label);
}

StdString Label::getTruncatedText (const StdString &text, int fontType, float maxWidth, const StdString &truncateSuffix) {
	StdString s;

	s.assign (text);
	Label::truncateText (&s, fontType, maxWidth, truncateSuffix);

	return (s);
}

void Label::flowRight (float *positionX, float positionY, float *rightExtent, float *bottomExtent) {
	UiConfiguration *uiconfig;
	float pos;

	uiconfig = &(App::getInstance ()->uiConfig);
	position.assign (*positionX, getLinePosition (positionY));
	*positionX += width + uiconfig->marginSize;
	if (rightExtent) {
		pos = position.x + width;
		if (pos > *rightExtent) {
			*rightExtent = pos;
		}
	}
	if (bottomExtent) {
		pos = positionY + maxLineHeight;
		if (pos > *bottomExtent) {
			*bottomExtent = pos;
		}
	}
}

void Label::flowDown (float positionX, float *positionY, float *rightExtent, float *bottomExtent) {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	y = *positionY;
	position.assign (positionX, getLinePosition (y));
	*positionY += maxLineHeight + uiconfig->marginSize;
	if (rightExtent) {
		x = position.x + width;
		if (x > *rightExtent) {
			*rightExtent = x;
		}
	}
	if (bottomExtent) {
		y = y + maxLineHeight;
		if (y > *bottomExtent) {
			*bottomExtent = y;
		}
	}
}

void Label::centerVertical (float topExtent, float bottomExtent) {
	position.assignY (topExtent + ((bottomExtent - topExtent) / 2.0f) - (maxLineHeight / 2.0f));
}
