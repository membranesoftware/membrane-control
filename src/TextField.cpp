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
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "Ui.h"
#include "Input.h"
#include "UiConfiguration.h"
#include "Sprite.h"
#include "Widget.h"
#include "Label.h"
#include "Panel.h"
#include "TextField.h"

TextField::TextField (float fieldWidth, const StdString &promptText)
: Panel ()
, fieldWidth (fieldWidth)
, isEditing (false)
, isInverseColor (false)
, valueChangeCallback (NULL)
, valueChangeCallbackData (NULL)
, promptLabel (NULL)
, valueLabel (NULL)
, cursorPanel (NULL)
, isFocused (false)
, cursorClock (0)
{
	UiConfiguration *uiconfig;

	typeName.assign ("TextField");

	uiconfig = &(App::getInstance ()->uiConfig);

	normalBgColor.assign (uiconfig->lightBackgroundColor);
	normalBorderColor.assign (uiconfig->mediumBackgroundColor);
	focusBgColor.assign (uiconfig->darkBackgroundColor);
	focusBorderColor.assign (uiconfig->lightBackgroundColor);
	editBgColor.assign (uiconfig->lightBackgroundColor);
	editBorderColor.assign (uiconfig->darkBackgroundColor);
	normalValueTextColor.assign (uiconfig->lightPrimaryTextColor);
	editValueTextColor.assign (uiconfig->primaryTextColor);
	promptTextColor.assign (uiconfig->lightPrimaryColor);

	setPadding (uiconfig->paddingSize, uiconfig->paddingSize / 2.0f);
	setFillBg (true, normalBgColor);
	setBorder (true, normalBorderColor);

	if (! promptText.empty ()) {
		promptLabel = (Label *) addWidget (new Label (promptText, UiConfiguration::CAPTION, promptTextColor), widthPadding, heightPadding);
	}

	valueLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CAPTION, normalValueTextColor), widthPadding, heightPadding);

	cursorPanel = (Panel *) addWidget (new Panel ());
	cursorPanel->setFixedSize (true, valueLabel->maxGlyphWidth * 0.9f, valueLabel->maxLineHeight);
	cursorPanel->setFillBg (true, normalValueTextColor);
	cursorPanel->zLevel = 1;
	cursorPanel->isVisible = false;

	setFixedSize (true, fieldWidth, valueLabel->maxLineHeight + (heightPadding * 2.0f));

	refreshLayout ();
}

TextField::~TextField () {

}

void TextField::setInverseColor (bool inverse) {
	UiConfiguration *uiconfig;

	if (isInverseColor == inverse) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	isInverseColor = inverse;
	if (isInverseColor) {
		normalBgColor.assign (uiconfig->darkInverseBackgroundColor);
		normalBorderColor.assign (uiconfig->mediumInverseBackgroundColor);
		focusBgColor.assign (uiconfig->lightInverseBackgroundColor);
		focusBorderColor.assign (uiconfig->darkInverseBackgroundColor);
		editBgColor.assign (uiconfig->lightInverseBackgroundColor);
		editBorderColor.assign (uiconfig->darkInverseBackgroundColor);
		normalValueTextColor.assign (uiconfig->darkInverseTextColor);
		editValueTextColor.assign (uiconfig->inverseTextColor);
		promptTextColor.assign (uiconfig->lightPrimaryColor);
	}
	else {
		normalBgColor.assign (uiconfig->lightBackgroundColor);
		normalBorderColor.assign (uiconfig->mediumBackgroundColor);
		focusBgColor.assign (uiconfig->darkBackgroundColor);
		focusBorderColor.assign (uiconfig->lightBackgroundColor);
		editBgColor.assign (uiconfig->lightBackgroundColor);
		editBorderColor.assign (uiconfig->darkBackgroundColor);
		normalValueTextColor.assign (uiconfig->lightPrimaryTextColor);
		editValueTextColor.assign (uiconfig->primaryTextColor);
		promptTextColor.assign (uiconfig->lightPrimaryColor);
	}

	setFillBg (true, normalBgColor);
	setBorder (true, normalBorderColor);
	if (promptLabel) {
		promptLabel->textColor.assign (promptTextColor);
	}
	valueLabel->textColor.assign (normalValueTextColor);
	cursorPanel->setFillBg (true, normalValueTextColor);
	refreshLayout ();
}

StdString TextField::getValue () {
	return (valueLabel->text);
}

void TextField::clearValue () {
	valueLabel->setText (StdString (""));
	refreshLayout ();
}

void TextField::setValue (const StdString &valueText, bool shouldSkipChangeCallback) {
	if (valueText.equals (valueLabel->text)) {
		return;
	}

	valueLabel->setText (valueText);
	refreshLayout ();
	if (valueChangeCallback && (! shouldSkipChangeCallback)) {
		valueChangeCallback (valueChangeCallbackData, this);
	}
}

void TextField::appendClipboardText () {
	char *text;
	StdString val;

	if (SDL_HasClipboardText ()) {
		text = SDL_GetClipboardText ();
		if (text) {
			val = valueLabel->text;
			val.append (text);
			SDL_free (text);
			setValue (val, isEditing);
		}
	}
}

void TextField::setFieldWidth (float widthValue) {
	fieldWidth = widthValue;
	setFixedSize (true, fieldWidth, valueLabel->maxLineHeight + (heightPadding * 2.0f));
}

void TextField::setLineWidth (int lineLength) {
	fieldWidth = valueLabel->maxGlyphWidth * (float) lineLength;
	setFixedSize (true, fieldWidth, valueLabel->maxLineHeight + (heightPadding * 2.0f));
}

void TextField::setValueChangeCallback (Widget::EventCallback callback, void *callbackData) {
	valueChangeCallback = callback;
	valueChangeCallbackData = callbackData;
}

void TextField::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, w;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = widthPadding;
	y = heightPadding;
	valueLabel->position.assign (x, valueLabel->getLinePosition (y - (heightPadding / 2.0f)));
	if (promptLabel) {
		promptLabel->position.assign (x + (promptLabel->spaceWidth * 2.0f), promptLabel->getLinePosition (y - (heightPadding / 2.0f)));
	}

	if (! valueLabel->text.empty ()) {
		x += valueLabel->width;
		w = valueLabel->spaceWidth;
		w /= 2.0f;
		if (w < 2.0f) {
			w = 2.0f;
		}
		else if (w > 5.0f) {
			w = 5.0f;
		}
		x += w;
	}
	cursorPanel->position.assign (x, (height / 2.0f) - (cursorPanel->height / 2.0f));

	if (isEditing) {
		bgColor.rotate (editBgColor, uiconfig->shortColorRotateDuration);
		borderColor.rotate (editBorderColor, uiconfig->shortColorRotateDuration);
		if (valueLabel->text.empty ()) {
			if (promptLabel) {
				promptLabel->isVisible = true;
			}
			valueLabel->isVisible = false;
		}
		else {
			if (promptLabel) {
				promptLabel->isVisible = false;
			}
			valueLabel->textColor.rotate (editValueTextColor, uiconfig->shortColorRotateDuration);
			valueLabel->isVisible = true;
		}
	}
	else {
		if (isFocused) {
			bgColor.rotate (focusBgColor, uiconfig->shortColorRotateDuration);
			borderColor.rotate (focusBorderColor, uiconfig->shortColorRotateDuration);
		}
		else {
			bgColor.rotate (normalBgColor, uiconfig->shortColorRotateDuration);
			borderColor.rotate (normalBorderColor, uiconfig->shortColorRotateDuration);
		}

		if (valueLabel->text.empty ()) {
			valueLabel->isVisible = false;
			if (promptLabel) {
				promptLabel->isVisible = true;
			}
		}
		else {
			if (promptLabel) {
				promptLabel->isVisible = false;
			}

			valueLabel->textColor.rotate (normalValueTextColor, uiconfig->shortColorRotateDuration);
			valueLabel->isVisible = true;
		}
	}
}

bool TextField::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	StdString val;
	char c;
	int len;

	if (! isEditing) {
		return (false);
	}

	if (keycode == SDLK_BACKSPACE) {
		val = valueLabel->text;
		len = val.length ();
		if (len > 0) {
			val.erase (len - 1, 1);
			setValue (val, true);
		}
		return (true);
	}

	if (isControlDown && (keycode == SDLK_v)) {
		appendClipboardText ();
		return (true);
	}

	if (keycode == SDLK_ESCAPE) {
		setValue (lastValue, true);
		setEditing (false);
		return (true);
	}

	if (keycode == SDLK_RETURN) {
		setEditing (false);
		return (true);
	}

	c = App::getInstance ()->input.getKeyCharacter (keycode, isShiftDown);
	if (c > 0) {
		val = valueLabel->text;
		val.append (1, c);
		setValue (val, true);
		return (true);
	}

	return (false);
}

void TextField::doUpdate (int msElapsed, float originX, float originY) {
	UiConfiguration *uiconfig;

	Panel::doUpdate (msElapsed, originX, originY);

	uiconfig = &(App::getInstance ()->uiConfig);
	if (isEditing && (uiconfig->blinkDuration > 0)) {
		cursorClock -= msElapsed;
		if (cursorClock <= 0) {
			cursorPanel->isVisible = (! cursorPanel->isVisible);
			while (cursorClock <= 0) {
				cursorClock += uiconfig->blinkDuration;
			}
		}
	}
	else {
		cursorPanel->isVisible = false;
	}
}

void TextField::doProcessMouseState (const Widget::MouseState &mouseState) {
	if (mouseState.isEntered) {
		setFocused (true);
		if (mouseState.isLeftClickReleased && mouseState.isLeftClickEntered) {
			setEditing (true);
		}
	}
	else {
		setFocused (false);
		if (mouseState.isLeftClickReleased) {
			setEditing (false);
		}
	}
}

void TextField::setFocused (bool enable) {
	if (enable == isFocused) {
		return;
	}
	isFocused = enable;
	refreshLayout ();
}

void TextField::setEditing (bool enable) {
	if (enable == isEditing) {
		return;
	}
	if (enable) {
		isEditing = true;
		lastValue.assign (valueLabel->text);
	}
	else {
		isEditing = false;
		if (! lastValue.equals (valueLabel->text)) {
			if (valueChangeCallback) {
				valueChangeCallback (valueChangeCallbackData, this);
			}
		}
	}
	refreshLayout ();
}
