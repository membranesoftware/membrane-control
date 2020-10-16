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
, isDisabled (false)
, isInverseColor (false)
, isPromptErrorColor (false)
, isObscured (false)
, isOvertype (false)
, cursorPosition (0)
, promptLabel (NULL)
, valueLabel (NULL)
, cursorPanel (NULL)
, isFocused (false)
, cursorClock (0)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);

	normalBgColor.assign (uiconfig->lightBackgroundColor);
	normalBorderColor.assign (uiconfig->darkBackgroundColor);
	focusBgColor.assign (uiconfig->darkBackgroundColor);
	focusBorderColor.assign (uiconfig->lightPrimaryColor);
	disabledBgColor.assign (uiconfig->darkBackgroundColor);
	disabledBorderColor.assign (uiconfig->mediumBackgroundColor);
	editBgColor.assign (uiconfig->lightBackgroundColor);
	editBorderColor.assign (uiconfig->darkPrimaryColor);
	normalValueTextColor.assign (uiconfig->lightPrimaryColor);
	editValueTextColor.assign (uiconfig->primaryTextColor);
	disabledValueTextColor.assign (uiconfig->lightPrimaryTextColor);
	promptTextColor.assign (uiconfig->lightPrimaryColor);

	setPadding (uiconfig->paddingSize, uiconfig->paddingSize / 2.0f);
	setFillBg (true, normalBgColor);
	setBorder (true, normalBorderColor);

	if (! promptText.empty ()) {
		promptLabel = (Label *) addWidget (new Label (promptText, UiConfiguration::CaptionFont, promptTextColor), widthPadding, heightPadding);
	}

	valueLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, normalValueTextColor), widthPadding, heightPadding);

	cursorPanel = (Panel *) addWidget (new Panel ());
	cursorPanel->setFixedSize (true, uiconfig->textFieldInsertCursorWidth, valueLabel->maxLineHeight);
	cursorPanel->setFillBg (true, uiconfig->darkPrimaryColor);
	cursorPanel->zLevel = 1;
	cursorPanel->isVisible = false;

	setFixedSize (true, fieldWidth, valueLabel->maxLineHeight + (heightPadding * 2.0f));

	refreshLayout ();
}

TextField::~TextField () {

}

void TextField::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}
	isDisabled = disabled;
	if (isDisabled) {
		setFocused (false);
		setKeyFocus (false);
	}
	refreshLayout ();
}

void TextField::setInverseColor (bool inverse) {
	UiConfiguration *uiconfig;

	if (isInverseColor == inverse) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isInverseColor = inverse;
	if (isInverseColor) {
		normalBgColor.assign (uiconfig->darkInverseBackgroundColor);
		normalBorderColor.assign (uiconfig->mediumInverseBackgroundColor);
		focusBgColor.assign (uiconfig->lightInverseBackgroundColor);
		focusBorderColor.assign (uiconfig->darkInverseBackgroundColor);
		disabledBgColor.assign (uiconfig->lightInverseBackgroundColor);
		disabledBorderColor.assign (uiconfig->darkInverseBackgroundColor);
		editBgColor.assign (uiconfig->lightInverseBackgroundColor);
		editBorderColor.assign (uiconfig->darkInverseBackgroundColor);
		normalValueTextColor.assign (uiconfig->darkInverseTextColor);
		editValueTextColor.assign (uiconfig->inverseTextColor);
		disabledValueTextColor.assign (uiconfig->darkInverseTextColor);
		if (isPromptErrorColor) {
			promptTextColor.assign (uiconfig->errorTextColor);
		}
		else {
			promptTextColor.assign (uiconfig->darkInverseTextColor);
		}
	}
	else {
		normalBgColor.assign (uiconfig->lightBackgroundColor);
		normalBorderColor.assign (uiconfig->darkBackgroundColor);
		focusBgColor.assign (uiconfig->darkBackgroundColor);
		focusBorderColor.assign (uiconfig->lightPrimaryColor);
		disabledBgColor.assign (uiconfig->darkBackgroundColor);
		disabledBorderColor.assign (uiconfig->mediumBackgroundColor);
		editBgColor.assign (uiconfig->lightBackgroundColor);
		editBorderColor.assign (uiconfig->darkPrimaryColor);
		normalValueTextColor.assign (uiconfig->lightPrimaryColor);
		editValueTextColor.assign (uiconfig->primaryTextColor);
		disabledValueTextColor.assign (uiconfig->lightPrimaryTextColor);
		if (isPromptErrorColor) {
			promptTextColor.assign (uiconfig->errorTextColor);
		}
		else {
			promptTextColor.assign (uiconfig->lightPrimaryColor);
		}
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

void TextField::setPromptErrorColor (bool enable) {
	UiConfiguration *uiconfig;

	if (isPromptErrorColor == enable) {
		return;
	}
	uiconfig = &(App::instance->uiConfig);
	isPromptErrorColor = enable;
	if (isPromptErrorColor) {
		promptTextColor.assign (uiconfig->errorTextColor);
	}
	else {
		if (isInverseColor) {
			promptTextColor.assign (uiconfig->darkInverseTextColor);
		}
		else {
			promptTextColor.assign (uiconfig->lightPrimaryColor);
		}
	}
	if (promptLabel) {
		promptLabel->textColor.translate (promptTextColor, uiconfig->shortColorTranslateDuration);
	}
}

void TextField::setObscured (bool enable) {
	if (isObscured == enable) {
		return;
	}
	isObscured = enable;
	if (isObscured) {
		valueLabel->setObscured (true);
	}
	else {
		valueLabel->setObscured (false);
	}
	refreshLayout ();
}

void TextField::setOvertype (bool enable) {
	if (isOvertype == enable) {
		return;
	}
	isOvertype = enable;
	if (isOvertype) {
		cursorPanel->setFixedSize (true, valueLabel->maxGlyphWidth * App::instance->uiConfig.textFieldOvertypeCursorScale, valueLabel->maxLineHeight);
	}
	else {
		cursorPanel->setFixedSize (true, App::instance->uiConfig.textFieldInsertCursorWidth, valueLabel->maxLineHeight);
	}
}

StdString TextField::getValue () {
	return (valueLabel->text);
}

void TextField::setValue (const StdString &valueText, bool shouldSkipChangeCallback, bool shouldSkipEditCallback) {
	if (valueText.equals (valueLabel->text)) {
		refreshLayout ();
		return;
	}

	valueLabel->setText (valueText);
	clipCursorPosition ();
	refreshLayout ();
	if ((! shouldSkipChangeCallback) && valueChangeCallback.callback) {
		valueChangeCallback.callback (valueChangeCallback.callbackData, this);
	}
	if ((! shouldSkipEditCallback) && valueEditCallback.callback) {
		valueEditCallback.callback (valueEditCallback.callbackData, this);
	}
}

void TextField::appendClipboardText () {
	char *text;
	int textlen;
	StdString val;

	if (! SDL_HasClipboardText ()) {
		return;
	}
	text = SDL_GetClipboardText ();
	if (! text) {
		return;
	}

	textlen = (int) StdString (text).length ();
	clipCursorPosition ();
	val.assign (valueLabel->text);
	if (isOvertype) {
		val.replace ((size_t) cursorPosition, (size_t) textlen, text);
	}
	else {
		val.insert ((size_t) cursorPosition, text);
	}
	SDL_free (text);
	cursorPosition += textlen;
	setValue (val, false, isKeyFocused);
}

void TextField::setFieldWidth (float widthValue) {
	fieldWidth = widthValue;
	setFixedSize (true, fieldWidth, valueLabel->maxLineHeight + (heightPadding * 2.0f));
}

void TextField::setLineWidth (int lineLength) {
	fieldWidth = valueLabel->maxGlyphWidth * (float) lineLength;
	setFixedSize (true, fieldWidth, valueLabel->maxLineHeight + (heightPadding * 2.0f));
}

void TextField::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, cursorx, dx;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;

	if (promptLabel) {
		promptLabel->position.assign (x + (promptLabel->spaceWidth * 2.0f), promptLabel->getLinePosition (y - (heightPadding / 2.0f)));
	}

	if (valueLabel->text.empty () || (cursorPosition <= 0)) {
		cursorx = 0.0f;
		dx = 0.0f;
	}
	else {
		cursorx = valueLabel->getCharacterPosition (cursorPosition);
		dx = fieldWidth - (widthPadding * 2.0f) - (cursorx + cursorPanel->width);
		if (dx >= 0.0f) {
			dx = 0.0f;
		}
	}
	valueLabel->position.assign (x + dx, valueLabel->getLinePosition (y - (heightPadding / 2.0f)));
	cursorPanel->position.assign (x + cursorx + dx, (height / 2.0f) - (cursorPanel->height / 2.0f));

	if (isKeyFocused) {
		bgColor.translate (editBgColor, uiconfig->shortColorTranslateDuration);
		borderColor.translate (editBorderColor, uiconfig->shortColorTranslateDuration);
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
			valueLabel->textColor.translate (editValueTextColor, uiconfig->shortColorTranslateDuration);
			valueLabel->isVisible = true;
		}
	}
	else {
		if (isDisabled) {
			bgColor.translate (disabledBgColor, uiconfig->shortColorTranslateDuration);
			borderColor.translate (disabledBorderColor, uiconfig->shortColorTranslateDuration);
		}
		else if (isFocused) {
			bgColor.translate (focusBgColor, uiconfig->shortColorTranslateDuration);
			borderColor.translate (focusBorderColor, uiconfig->shortColorTranslateDuration);
		}
		else {
			bgColor.translate (normalBgColor, uiconfig->shortColorTranslateDuration);
			borderColor.translate (normalBorderColor, uiconfig->shortColorTranslateDuration);
		}

		if (valueLabel->text.empty ()) {
			valueLabel->isVisible = false;

			if (promptLabel) {
				if (isDisabled) {
					promptLabel->isVisible = false;
				}
				else {
					promptLabel->isVisible = true;
				}
			}
		}
		else {
			if (promptLabel) {
				promptLabel->isVisible = false;
			}
			if (isDisabled) {
				valueLabel->textColor.translate (disabledValueTextColor, uiconfig->shortColorTranslateDuration);
			}
			else {
				valueLabel->textColor.translate (normalValueTextColor, uiconfig->shortColorTranslateDuration);
			}
			valueLabel->isVisible = true;
		}
	}
}

bool TextField::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	StdString val;
	char c;
	int len;

	if (isDisabled || (! isKeyFocused)) {
		return (false);
	}

	switch (keycode) {
		case SDLK_ESCAPE: {
			setValue (lastValue, false, true);
			setKeyFocus (false);
			return (true);
		}
		case SDLK_RETURN: {
			setKeyFocus (false);
			return (true);
		}
		case SDLK_LEFT: {
			if (cursorPosition > 0) {
				--cursorPosition;
				refreshLayout ();
			}
			return (true);
		}
		case SDLK_RIGHT: {
			if (cursorPosition < (int) valueLabel->text.length ()) {
				++cursorPosition;
				refreshLayout ();
			}
			return (true);
		}
		case SDLK_HOME: {
			if (cursorPosition != 0) {
				cursorPosition = 0;
				refreshLayout ();
			}
			return (true);
		}
		case SDLK_END: {
			len = (int) valueLabel->text.length ();
			if (cursorPosition != len) {
				cursorPosition = len;
				refreshLayout ();
			}
			return (true);
		}
		case SDLK_INSERT: {
			setOvertype (! isOvertype);
			return (true);
		}
		case SDLK_BACKSPACE: {
			clipCursorPosition ();
			val.assign (valueLabel->text);
			len = val.length ();
			if ((len > 0) && (cursorPosition > 0)) {
				val.erase (cursorPosition - 1, 1);
				--cursorPosition;
				setValue (val, false, true);
			}
			return (true);
		}
		case SDLK_DELETE: {
			clipCursorPosition ();
			val.assign (valueLabel->text);
			if (cursorPosition < (int) val.length ()) {
				val.erase ((size_t) cursorPosition, 1);
				setValue (val, false, true);
			}
			return (true);
		}
	}

	if (isControlDown && (keycode == SDLK_v)) {
		appendClipboardText ();
		return (true);
	}
	if (! isControlDown) {
		c = App::instance->input.getKeyCharacter (keycode, isShiftDown);
		if (c > 0) {
			clipCursorPosition ();
			val.assign (valueLabel->text);
			if (isOvertype) {
				val.replace ((size_t) cursorPosition, 1, 1, c);
			}
			else {
				val.insert ((size_t) cursorPosition, 1, c);
			}
			++cursorPosition;
			setValue (val, false, true);
			return (true);
		}
	}

	return (false);
}

void TextField::doUpdate (int msElapsed) {
	UiConfiguration *uiconfig;

	Panel::doUpdate (msElapsed);
	uiconfig = &(App::instance->uiConfig);

	if (isKeyFocused && (! isDisabled) && (uiconfig->blinkDuration > 0)) {
		cursorClock -= msElapsed;
		if (cursorClock <= 0) {
			cursorPanel->isVisible = (! cursorPanel->isVisible);
			if (cursorPanel->isVisible) {
				clipCursorPosition ();
				refreshLayout ();
			}
			cursorClock %= uiconfig->blinkDuration;
			cursorClock += uiconfig->blinkDuration;
		}
	}
	else {
		cursorPanel->isVisible = false;
	}
}

bool TextField::doProcessMouseState (const Widget::MouseState &mouseState) {
	if (isDisabled) {
		return (false);
	}

	if (mouseState.isEntered) {
		setFocused (true);
		if (mouseState.isLeftClickReleased && mouseState.isLeftClickEntered) {
			App::instance->uiStack.setKeyFocusTarget (this);
		}
	}
	else {
		setFocused (false);
	}

	return (false);
}

void TextField::setFocused (bool enable) {
	if (enable == isFocused) {
		return;
	}
	isFocused = enable;
	refreshLayout ();
}

void TextField::setKeyFocus (bool enable) {
	if (enable == isKeyFocused) {
		return;
	}
	if (enable) {
		isKeyFocused = true;
		lastValue.assign (valueLabel->text);
		cursorPosition = (int) valueLabel->text.length ();
		setOvertype (false);
	}
	else {
		isKeyFocused = false;
		if (! lastValue.equals (valueLabel->text)) {
			if (valueEditCallback.callback) {
				valueEditCallback.callback (valueEditCallback.callbackData, this);
			}
		}
	}
	refreshLayout ();
}

void TextField::clipCursorPosition () {
	int len;

	if (cursorPosition < 0) {
		cursorPosition = 0;
	}
	len = (int) valueLabel->text.length ();
	if (cursorPosition > len) {
		cursorPosition = len;
	}
}
