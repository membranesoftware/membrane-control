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
#include "ClassId.h"
#include "App.h"
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
, shouldRetainFocusOnReturnKey (false)
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
	classId = ClassId::TextField;

	normalBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
	normalBorderColor.assign (UiConfiguration::instance->darkBackgroundColor);
	focusBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
	focusBorderColor.assign (UiConfiguration::instance->lightPrimaryColor);
	disabledBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
	disabledBorderColor.assign (UiConfiguration::instance->mediumBackgroundColor);
	editBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
	editBorderColor.assign (UiConfiguration::instance->darkPrimaryColor);
	normalValueTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
	editValueTextColor.assign (UiConfiguration::instance->primaryTextColor);
	disabledValueTextColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
	promptTextColor.assign (UiConfiguration::instance->lightPrimaryColor);

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize / 2.0f);
	setFillBg (true, normalBgColor);
	setBorder (true, normalBorderColor);

	if (! promptText.empty ()) {
		promptLabel = (Label *) addWidget (new Label (promptText, UiConfiguration::CaptionFont, promptTextColor), widthPadding, heightPadding);
	}

	valueLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, normalValueTextColor), widthPadding, heightPadding);

	cursorPanel = (Panel *) addWidget (new Panel ());
	cursorPanel->setFixedSize (true, UiConfiguration::instance->textFieldInsertCursorWidth, valueLabel->maxLineHeight);
	cursorPanel->setFillBg (true, UiConfiguration::instance->darkPrimaryColor);
	cursorPanel->zLevel = 1;
	cursorPanel->isVisible = false;

	setFixedSize (true, fieldWidth, valueLabel->maxLineHeight + (heightPadding * 2.0f));

	refreshLayout ();
}

TextField::~TextField () {

}

bool TextField::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::TextField));
}

TextField *TextField::castWidget (Widget *widget) {
	return (TextField::isWidgetType (widget) ? (TextField *) widget : NULL);
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
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	if (isInverseColor) {
		normalBgColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		normalBorderColor.assign (UiConfiguration::instance->mediumInverseBackgroundColor);
		focusBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		focusBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		disabledBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		disabledBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		editBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		editBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		normalValueTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		editValueTextColor.assign (UiConfiguration::instance->inverseTextColor);
		disabledValueTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		if (isPromptErrorColor) {
			promptTextColor.assign (UiConfiguration::instance->errorTextColor);
		}
		else {
			promptTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		}
	}
	else {
		normalBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
		normalBorderColor.assign (UiConfiguration::instance->darkBackgroundColor);
		focusBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
		focusBorderColor.assign (UiConfiguration::instance->lightPrimaryColor);
		disabledBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
		disabledBorderColor.assign (UiConfiguration::instance->mediumBackgroundColor);
		editBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
		editBorderColor.assign (UiConfiguration::instance->darkPrimaryColor);
		normalValueTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
		editValueTextColor.assign (UiConfiguration::instance->primaryTextColor);
		disabledValueTextColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
		if (isPromptErrorColor) {
			promptTextColor.assign (UiConfiguration::instance->errorTextColor);
		}
		else {
			promptTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
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
	if (isPromptErrorColor == enable) {
		return;
	}
	isPromptErrorColor = enable;
	if (isPromptErrorColor) {
		promptTextColor.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		if (isInverseColor) {
			promptTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		}
		else {
			promptTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
		}
	}
	if (promptLabel) {
		promptLabel->textColor.translate (promptTextColor, UiConfiguration::instance->shortColorTranslateDuration);
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
		cursorPanel->setFixedSize (true, valueLabel->maxGlyphWidth * UiConfiguration::instance->textFieldOvertypeCursorScale, valueLabel->maxLineHeight);
	}
	else {
		cursorPanel->setFixedSize (true, UiConfiguration::instance->textFieldInsertCursorWidth, valueLabel->maxLineHeight);
	}
}

StdString TextField::getValue () {
	return (valueLabel->text);
}

void TextField::setText (const StdString &valueText) {
	if (valueText.equals (valueLabel->text)) {
		refreshLayout ();
		return;
	}
	valueLabel->setText (valueText);
	clipCursorPosition ();
	refreshLayout ();
}

void TextField::setValue (const StdString &valueText, bool shouldSkipChangeCallback, bool shouldSkipEditCallback) {
	setText (valueText);
	lastValue.assign (valueText);
	cursorPosition = (int) valueText.length ();
	if (! shouldSkipChangeCallback) {
		eventCallback (valueChangeCallback);
	}
	if (! shouldSkipEditCallback) {
		eventCallback (valueEditCallback);
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
	setText (val);
	eventCallback (valueChangeCallback);
	if (! isKeyFocused) {
		eventCallback (valueEditCallback);
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

void TextField::refreshLayout () {
	float x, y, cursorx, dx;

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
		bgColor.translate (editBgColor, UiConfiguration::instance->shortColorTranslateDuration);
		borderColor.translate (editBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
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
			valueLabel->textColor.translate (editValueTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			valueLabel->isVisible = true;
		}
	}
	else {
		if (isDisabled) {
			bgColor.translate (disabledBgColor, UiConfiguration::instance->shortColorTranslateDuration);
			borderColor.translate (disabledBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
		}
		else if (isFocused) {
			bgColor.translate (focusBgColor, UiConfiguration::instance->shortColorTranslateDuration);
			borderColor.translate (focusBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
		}
		else {
			bgColor.translate (normalBgColor, UiConfiguration::instance->shortColorTranslateDuration);
			borderColor.translate (normalBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
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
				valueLabel->textColor.translate (disabledValueTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			}
			else {
				valueLabel->textColor.translate (normalValueTextColor, UiConfiguration::instance->shortColorTranslateDuration);
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
			setText (lastValue);
			eventCallback (valueChangeCallback);
			setKeyFocus (false);
			return (true);
		}
		case SDLK_RETURN: {
			if (shouldRetainFocusOnReturnKey) {
				if (! lastValue.equals (valueLabel->text)) {
					eventCallback (valueEditCallback);
				}
			}
			else {
				setKeyFocus (false);
			}
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
				setText (val);
				eventCallback (valueChangeCallback);
			}
			return (true);
		}
		case SDLK_DELETE: {
			clipCursorPosition ();
			val.assign (valueLabel->text);
			if (cursorPosition < (int) val.length ()) {
				val.erase ((size_t) cursorPosition, 1);
				setText (val);
				eventCallback (valueChangeCallback);
			}
			return (true);
		}
	}

	if (isControlDown && (keycode == SDLK_v)) {
		appendClipboardText ();
		return (true);
	}
	if (! isControlDown) {
		c = Input::instance->getKeyCharacter (keycode, isShiftDown);
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
			setText (val);
			eventCallback (valueChangeCallback);
			return (true);
		}
	}

	return (false);
}

void TextField::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);

	if (isKeyFocused && (! isDisabled) && (UiConfiguration::instance->blinkDuration > 0)) {
		cursorClock -= msElapsed;
		if (cursorClock <= 0) {
			cursorPanel->isVisible = (! cursorPanel->isVisible);
			if (cursorPanel->isVisible) {
				clipCursorPosition ();
				refreshLayout ();
			}
			cursorClock %= UiConfiguration::instance->blinkDuration;
			cursorClock += UiConfiguration::instance->blinkDuration;
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
			eventCallback (valueEditCallback);
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
