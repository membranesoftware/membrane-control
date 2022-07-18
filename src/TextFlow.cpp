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
#include <list>
#include "StdString.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "Resource.h"
#include "Widget.h"
#include "Label.h"
#include "Font.h"
#include "Panel.h"
#include "TextFlow.h"

TextFlow::TextFlow (float viewWidth, UiConfiguration::FontType fontType)
: Panel ()
, maxLineCount (0)
, viewWidth (viewWidth)
, lineCount (0)
, textFontType (UiConfiguration::NoFont)
, textFont (NULL)
, textFontSize (0)
{
	setTextColor (UiConfiguration::instance->primaryTextColor);
	setText (StdString (""), fontType);
}

TextFlow::~TextFlow () {
	if (textFont) {
		Resource::instance->unloadFont (textFontName, textFontSize);
		textFont = NULL;
	}
}

void TextFlow::setViewWidth (float targetWidth) {
	if (FLOAT_EQUALS (viewWidth, targetWidth)) {
		return;
	}
	viewWidth = targetWidth;
	setText (text, textFontType, true);
}

void TextFlow::setFont (UiConfiguration::FontType fontType) {
	if (fontType == textFontType) {
		return;
	}
	setText (text, fontType);
}

void TextFlow::setTextColor (const Color &color) {
	std::list<Label *>::iterator i, end;

	textColor.assign (color);
	i = lineList.begin ();
	end = lineList.end ();
	while (i != end) {
		(*i)->textColor.assign (textColor);
		++i;
	}
}

void TextFlow::setText (const StdString &textContent, UiConfiguration::FontType fontType, bool forceFontReload) {
	std::list<Label *>::iterator i, end;
	Font *loadfont;

	loadfont = NULL;
	if (fontType >= 0) {
		if (forceFontReload || (! textFont) || (textFontType != fontType)) {
			loadfont = Resource::instance->loadFont (UiConfiguration::instance->fontNames[fontType], UiConfiguration::instance->fontSizes[fontType]);
			if (loadfont) {
				if (textFont) {
					Resource::instance->unloadFont (textFontName, textFontSize);
				}
				textFont = loadfont;
				textFontType = fontType;
				textFontName.assign (UiConfiguration::instance->fontNames[fontType]);
				textFontSize = UiConfiguration::instance->fontSizes[fontType];
			}
		}
	}
	if (! textFont) {
		return;
	}
	if ((! loadfont) && text.equals (textContent)) {
		return;
	}

	i = lineList.begin ();
	end = lineList.end ();
	while (i != end) {
		(*i)->isDestroyed = true;
		++i;
	}
	lineList.clear ();

	text.assign (textContent);
	addLines (text);
}

void TextFlow::appendText (const StdString &textContent) {
	Label *label;
	size_t pos;

	text.appendSprintf ("\n%s", textContent.c_str ());
	addLines (textContent);
	if ((maxLineCount > 0) && (lineCount > maxLineCount)) {
		while (lineCount > maxLineCount) {
			label = lineList.front ();
			pos = text.find (label->text);
			if (pos != StdString::npos) {
				text = text.substr (pos + label->text.length ());
			}
			label->isDestroyed = true;
			lineList.pop_front ();
			--lineCount;
		}
		refreshLayout ();
	}
}

void TextFlow::addLines (const StdString &linesText) {
	Font::Metrics metrics;
	Label *label;
	float maxw;
	int breakpos;
	char lastc;

	if ((! textFont) || linesText.empty ()) {
		return;
	}
	maxw = viewWidth - (widthPadding * 2.0f);
	textFont->resetMetrics (&metrics, linesText, 0);
	lastc = '\0';
	breakpos = -1;
	while (! metrics.isComplete) {
		textFont->advanceMetrics (&metrics);
		if ((metrics.lastCharacter == ' ') && (lastc != ' ')) {
			breakpos = metrics.textPosition;
		}
		if ((metrics.textWidth > maxw) || (metrics.lastCharacter == '\n')) {
			if ((breakpos < 0) || (metrics.lastCharacter == '\n')) {
				breakpos = metrics.textPosition;
			}
			label = (Label *) addWidget (new Label (metrics.text.substr (0, breakpos), textFontType, textColor));
			lineList.push_back (label);

			textFont->resetMetrics (&metrics, metrics.text.substr (breakpos), 0);
			breakpos = -1;
		}

		lastc = metrics.lastCharacter;
	}
	if (metrics.textPosition > 0) {
		label = (Label *) addWidget (new Label (metrics.text, textFontType, textColor));
		lineList.push_back (label);
	}

	refreshLayout ();
}

void TextFlow::refreshLayout () {
	std::list<Label *>::iterator i, end;
	Label *label;
	float x, y, h;

	x = widthPadding;
	y = heightPadding;
	h = 0.0f;
	i = lineList.begin ();
	end = lineList.end ();
	while (i != end) {
		label = *i;
		label->position.assign (x, label->getLinePosition (y));
		h = label->position.y + label->height;
		y += label->maxLineHeight + UiConfiguration::instance->textLineHeightMargin;
		++i;
	}

	h += heightPadding;
	setFixedSize (true, viewWidth, h);
	lineCount = (int) lineList.size ();
}
