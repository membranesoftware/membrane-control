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
#include <list>
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "Ui.h"
#include "Input.h"
#include "Sprite.h"
#include "Color.h"
#include "Widget.h"
#include "Label.h"
#include "Font.h"
#include "Panel.h"
#include "UiConfiguration.h"
#include "TextArea.h"

TextArea::TextArea (int fontType, const Color &textColor, int maxTextLineLength, float maxTextLineWidth)
: Panel ()
, textFontType (fontType)
, textColor (textColor)
, maxLineLength (maxTextLineLength)
, maxLineWidth (maxTextLineWidth)
{
	UiConfiguration *uiconfig;

	typeName.assign ("TextArea");
	uiconfig = &(App::getInstance ()->uiConfig);
	if (maxLineLength <= 0) {
		maxLineLength = uiconfig->textAreaMediumLineLength;
	}
}

TextArea::~TextArea () {

}

void TextArea::clear () {
	std::list<Label *>::iterator i, end;

	i = lineList.begin ();
	end = lineList.end ();
	while (i != end) {
		(*i)->isDestroyed = true;
		++i;
	}
	lineList.clear ();
}

void TextArea::setMaxLineWidth (float maxTextLineWidth) {
	std::list<Label *>::iterator i, end;
	StdString text;

	if (FLOAT_EQUALS (maxTextLineWidth, maxLineWidth)) {
		return;
	}

	maxLineWidth = maxTextLineWidth;
	i = lineList.begin ();
	end = lineList.end ();
	while (i != end) {
		if (! text.empty ()) {
			text.append (1, ' ');
		}
		text.append ((*i)->text);
		++i;
	}
	setText (text);
}

void TextArea::setPadding (float widthPaddingSize, float heightPaddingSize) {
	Panel::setPadding (widthPaddingSize, heightPaddingSize);
	refreshLayout ();
}

void TextArea::setTextColor (const Color &color) {
	std::list<Label *>::iterator i, end;

	textColor.assign (color);
	i = lineList.begin ();
	end = lineList.end ();
	while (i != end) {
		(*i)->textColor.assign (textColor);
		++i;
	}
}

void TextArea::setText (const StdString &text) {
	Label *label;
	StdString s, line;
	size_t pos;

	clear ();

	s.assign (text);
	while (true) {
		if (s.empty ()) {
			break;
		}

		line.assign ("");
		pos = s.find ('\n');
		if (pos != StdString::npos) {
			if ((maxLineLength > 0) && ((int) pos > maxLineLength)) {
				pos = StdString::npos;
			}
		}

		if (pos != StdString::npos) {
			line.assign (s.substr (0, pos));
			s.assign (s.substr (pos + 1));
		}
		else {
			if ((maxLineLength <= 0) || ((int) s.length () < maxLineLength)) {
				line.assign (s);
				s.assign ("");
			}
			else {
				pos = s.find_last_of (' ', maxLineLength);
				if (pos == StdString::npos) {
					line.assign (s);
					s.assign ("");
				}
				else {
					line.assign (s.substr (0, pos));
					s.assign (s.substr (pos + 1));
				}
			}
		}

		label = (Label *) addWidget (new Label (line, textFontType, textColor));
		if (maxLineWidth > 0.0f) {
			while (true) {
				if (label->width <= maxLineWidth) {
					break;
				}

				pos = line.find_last_of (' ');
				if (pos == StdString::npos) {
					break;
				}

				if (s.find (' ') != 0) {
					s.insert (0, 1, ' ');
				}
				s.insert (0, line.substr (pos));
				line.assign (line.substr (0, pos));
				label->setText (line);
			}

			if (s.find (' ') == 0) {
				s.assign (s.substr (1));
			}
		}

		lineList.push_back (label);
	}

	refreshLayout ();
}

void TextArea::refreshLayout () {
	UiConfiguration *uiconfig;
	std::list<Label *>::iterator i, end;
	Label *label;
	float x, y, dh;

	uiconfig = &(App::getInstance ()->uiConfig);
	dh = 0.0f;
	x = widthPadding;
	y = heightPadding;
	i = lineList.begin ();
	end = lineList.end ();
	while (i != end) {
		label = *i;
		label->position.assign (x, label->getLinePosition (y));
		y += label->maxLineHeight + uiconfig->textLineHeightMargin;
		dh = label->maxLineHeight - label->height + uiconfig->textLineHeightMargin;
		++i;
	}

	resetSize ();
	height += dh;
}
