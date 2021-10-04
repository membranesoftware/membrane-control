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
#include <math.h>
#include "App.h"
#include "Log.h"
#include "StdString.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "Input.h"
#include "Sprite.h"
#include "Color.h"
#include "Widget.h"
#include "Label.h"
#include "Font.h"
#include "Panel.h"
#include "ScrollBar.h"
#include "ScrollView.h"
#include "TextArea.h"

TextArea::TextArea (float viewWidth, int viewLineCount, UiConfiguration::FontType fontType, bool isScrollEnabled)
: ScrollView ()
, textFlow (NULL)
, scrollBar (NULL)
{
	float textw;

	setViewSize (viewWidth, (UiConfiguration::instance->fonts[fontType]->maxLineHeight + UiConfiguration::instance->textLineHeightMargin) * (float) viewLineCount);

	textw = viewWidth;
	if (isScrollEnabled) {
		isMouseWheelScrollEnabled = true;
		scrollBar = (ScrollBar *) addWidget (new ScrollBar (1.0f));
		scrollBar->setMaxTrackLength (height);
		scrollBar->positionChangeCallback = Widget::EventCallbackContext (TextArea::scrollBarPositionChanged, this);
		scrollBar->zLevel = 3;
		textw -= (scrollBar->width + UiConfiguration::instance->marginSize);
	}
	textFlow = (TextFlow *) addWidget (new TextFlow (textw, fontType));
	textFlow->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);

	setInverseColor (false);
	refreshLayout ();
}

TextArea::~TextArea () {

}

void TextArea::setFont (UiConfiguration::FontType fontType) {
	textFlow->setFont (fontType);
	refreshLayout ();
}

void TextArea::setTextColor (const Color &color) {
	textFlow->setTextColor (color);
}

void TextArea::setInverseColor (bool isInverseColor) {
	if (isInverseColor) {
		setFillBg (true, UiConfiguration::instance->mediumInverseBackgroundColor);
		setBorder (true, UiConfiguration::instance->lightInverseBackgroundColor);
		setTextColor (UiConfiguration::instance->inverseTextColor);
	}
	else {
		setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		setBorder (true, UiConfiguration::instance->darkBackgroundColor);
		setTextColor (UiConfiguration::instance->primaryTextColor);
	}
}

void TextArea::setMaxLineCount (int max) {
	if (max < 0) {
		max = 0;
	}
	textFlow->maxLineCount = max;
}

void TextArea::setText (const StdString &textContent, UiConfiguration::FontType fontType, bool forceFontReload) {
	textFlow->setText (textContent, fontType, forceFontReload);
	refreshLayout ();
}

void TextArea::appendText (const StdString &textContent, bool scrollToBottom) {
	textFlow->appendText (textContent);
	refreshLayout ();
	if (scrollBar && scrollToBottom) {
		setViewOrigin (0.0f, maxViewOriginY);
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY);
	}
}

void TextArea::refreshLayout () {
	float x, y;

	x = widthPadding;
	y = heightPadding;
	textFlow->position.assign (x, y);

	y += textFlow->height;
	y -= height;
	if (y < 0.0f) {
		y = 0.0f;
	}
	setVerticalScrollBounds (0.0f, y);
	if (viewOriginY < 0.0f) {
		setViewOrigin (0.0f, 0.0f);
	}
	else if (viewOriginY > y) {
		setViewOrigin (0.0f, y);
	}

	if (scrollBar) {
		scrollBar->setScrollBounds (height, textFlow->height + (heightPadding * 2.0f));
		scrollBar->position.assign (width - scrollBar->width, viewOriginY);
		if (scrollBar->maxScrollPosition <= 0.0f) {
			isMouseWheelScrollEnabled = false;
			scrollBar->isInputSuspended = true;
		}
		else {
			isMouseWheelScrollEnabled = true;
			scrollBar->isInputSuspended = false;
		}
	}
}

bool TextArea::doProcessMouseState (const Widget::MouseState &mouseState) {
	bool consumed;
	float y1;

	y1 = viewOriginY;
	consumed = ScrollView::doProcessMouseState (mouseState);
	if (scrollBar) {
		if (! FLOAT_EQUALS (y1, viewOriginY)) {
			scrollBar->setPosition (viewOriginY, true);
			scrollBar->position.assignY (viewOriginY);
		}
	}

	return (consumed);
}

void TextArea::scrollBarPositionChanged (void *textAreaPtr, Widget *widgetPtr) {
	TextArea *textarea;
	ScrollBar *scrollbar;

	textarea = (TextArea *) textAreaPtr;
	scrollbar = (ScrollBar *) widgetPtr;

	textarea->setViewOrigin (0.0f, scrollbar->scrollPosition);
	scrollbar->position.assignY (textarea->viewOriginY);
}
