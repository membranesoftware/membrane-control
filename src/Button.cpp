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
#include "SDL2/SDL.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Input.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Sprite.h"
#include "Label.h"
#include "Image.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "Button.h"

const float Button::FocusTextOffset = 2.0f;

Button::Button (Sprite *sprite, const StdString &labelText)
: Panel ()
, shortcutKey (SDLK_UNKNOWN)
, isFocusDropShadowDisabled (false)
, maxImageWidth (0.0f)
, maxImageHeight (0.0f)
, isFocused (false)
, isPressed (false)
, isDisabled (false)
, isRaised (false)
, isInverseColor (false)
, isImageColorEnabled (false)
, label (NULL)
, image (NULL)
, pressClock (0)
{
	classId = ClassId::Button;

	if (! labelText.empty ()) {
		normalTextColor.assign (UiConfiguration::instance->flatButtonTextColor);
		label = (Label *) addWidget (new Label (labelText, UiConfiguration::ButtonFont, normalTextColor));
	}
	if (sprite) {
		image = (Image *) addWidget (new Image (sprite, UiConfiguration::WhiteButtonFrame));
		image->drawAlpha = UiConfiguration::instance->activeFocusedIconAlpha;
		maxImageWidth = image->maxSpriteWidth;
		maxImageHeight = image->maxSpriteHeight;
	}

	widthPadding = UiConfiguration::instance->paddingSize;
	heightPadding = UiConfiguration::instance->paddingSize;

	mouseEnterCallback = Widget::EventCallbackContext (Button::mouseEntered, this);
	mouseExitCallback = Widget::EventCallbackContext (Button::mouseExited, this);
	mousePressCallback = Widget::EventCallbackContext (Button::mousePressed, this);
	mouseReleaseCallback = Widget::EventCallbackContext (Button::mouseReleased, this);

	refreshLayout ();
}

Button::~Button () {

}

bool Button::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::Button));
}

Button *Button::castWidget (Widget *widget) {
	return (Button::isWidgetType (widget) ? (Button *) widget : NULL);
}

void Button::setPadding (float widthPaddingSize, float heightPaddingSize) {
	widthPadding = widthPaddingSize;
	heightPadding = heightPaddingSize;
	refreshLayout ();
}

void Button::setPressed (bool pressed) {
	if (pressed == isPressed) {
		return;
	}
	isPressed = pressed;
	if (isPressed) {
		setFocused (false);
	}
	refreshLayout ();
}

void Button::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}
	isDisabled = disabled;
	if (isDisabled) {
		setFocused (false);
		isInputSuspended = true;
	}
	else {
		isInputSuspended = false;
	}
	refreshLayout ();
}

void Button::setText (const StdString &text) {
	if (text.empty ()) {
		if (label) {
			label->isDestroyed = true;
			label = NULL;
			refreshLayout ();
		}
	}
	else {
		if (! label) {
			label = (Label *) addWidget (new Label (text, UiConfiguration::ButtonFont, normalTextColor));
		}
		label->setText (text);
		refreshLayout ();
	}
}

void Button::setTextColor (const Color &textColor) {
	normalTextColor.assign (textColor);
	if (label) {
		label->textColor.assign (textColor);
	}
}

void Button::setRaised (bool raised, const Color &normalBgColor) {
	isRaised = raised;
	if (isRaised) {
		raiseNormalBgColor = normalBgColor;
	}
	refreshLayout ();
}

void Button::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	refreshLayout ();
}

void Button::setFocused (bool focused) {
	if (focused == isFocused) {
		return;
	}
	isFocused = focused;
	refreshLayout ();
}

void Button::setImageColor (const Color &imageColor) {
	isImageColorEnabled = true;
	if (image) {
		image->setDrawColor (true, imageColor);
	}
	refreshLayout ();
}

bool Button::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (isShiftDown || isControlDown) {
		return (false);
	}
	if (isDisabled || isPressed) {
		return (false);
	}

	if ((shortcutKey != SDLK_UNKNOWN) && (keycode == shortcutKey)) {
		setPressed (true);
		pressClock = UiConfiguration::instance->blinkDuration;
		refreshLayout ();
		mouseClick ();
		return (true);
	}

	return (false);
}

void Button::mouseEntered (void *buttonPtr, Widget *widgetPtr) {
	Button *button;

	button = (Button *) buttonPtr;
	if (button->isDisabled) {
		return;
	}
	button->setFocused (true);
}

void Button::mouseExited (void *buttonPtr, Widget *widgetPtr) {
	Button *button;

	button = (Button *) buttonPtr;
	if (button->isDisabled) {
		return;
	}
	button->setFocused (false);
}

void Button::mousePressed (void *buttonPtr, Widget *widgetPtr) {
	Button *button;

	button = (Button *) buttonPtr;
	if (button->isDisabled) {
		return;
	}
	button->setPressed (true);
}

void Button::mouseReleased (void *buttonPtr, Widget *widgetPtr) {
	Button *button;

	button = (Button *) buttonPtr;
	if (button->isDisabled) {
		return;
	}
	button->setPressed (false);
	button->setFocused (button->isMouseEntered);
}

void Button::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (pressClock > 0) {
		pressClock -= msElapsed;
		if (pressClock <= 0) {
			pressClock = 0;
		}
	}

	if (! isInputSuspended) {
		if (isPressed && (pressClock <= 0)) {
			if ((! isMouseEntered) || (! Input::instance->isMouseLeftButtonDown)) {
				setPressed (false);
				if (isMouseEntered) {
					setFocused (true);
				}
			}
		}

		if (isFocused) {
			if (! isMouseEntered) {
				setFocused (false);
			}
		}
	}
}

void Button::doRefresh () {
	widthPadding = UiConfiguration::instance->paddingSize;
	heightPadding = UiConfiguration::instance->paddingSize;
	Panel::doRefresh ();
	if (image) {
		maxImageWidth = image->maxSpriteWidth;
		maxImageHeight = image->maxSpriteHeight;
	}
	refreshLayout ();
}

void Button::refreshLayout () {
	float x, y, h, spacew, paddingh, bgalpha;
	bool shouldfillbg, iswhiteframe, isdropshadowed;
	Color bgcolor, bordercolor, shadecolor;

	shouldfillbg = false;
	isdropshadowed = false;
	bgalpha = 1.0f;
	if (isRaised) {
		shouldfillbg = true;
		bgcolor.assign (raiseNormalBgColor);
	}

	if (isInverseColor) {
		shadecolor.assign (0.89f, 0.89f, 0.89f);
	}
	else {
		shadecolor.assign (0.0f, 0.0f, 0.0f);
	}

	iswhiteframe = (isInverseColor || isImageColorEnabled);
	if (isDisabled) {
		if (image) {
			image->setFrame (iswhiteframe ? UiConfiguration::WhiteButtonFrame : UiConfiguration::BlackButtonFrame);
			image->drawAlpha = UiConfiguration::instance->inactiveIconAlpha;
		}

		if (isRaised) {
			bgalpha = UiConfiguration::instance->buttonDisabledShadeAlpha;
		}
	}
	else if (isPressed) {
		if (image) {
			image->setFrame (iswhiteframe ? UiConfiguration::WhiteLargeButtonFrame : UiConfiguration::BlackLargeButtonFrame);
			image->drawAlpha = UiConfiguration::instance->activeFocusedIconAlpha;
		}

		shouldfillbg = true;
		if (isRaised) {
			bgcolor.blend (shadecolor, UiConfiguration::instance->buttonPressedShadeAlpha);
		}
		else {
			bgalpha = UiConfiguration::instance->buttonPressedShadeAlpha;
			bgcolor.assign (shadecolor);
		}
	}
	else if (isFocused) {
		if (image) {
			image->setFrame (iswhiteframe ? UiConfiguration::WhiteLargeButtonFrame : UiConfiguration::BlackLargeButtonFrame);
			image->drawAlpha = UiConfiguration::instance->activeFocusedIconAlpha;
		}

		isdropshadowed = true;
		shouldfillbg = true;
		if (isRaised) {
			bgcolor.blend (shadecolor, UiConfiguration::instance->buttonFocusedShadeAlpha);
		}
		else {
			bgalpha = UiConfiguration::instance->buttonFocusedShadeAlpha;
			bgcolor.assign (shadecolor);
		}
	}
	else {
		if (image) {
			image->setFrame (iswhiteframe ? UiConfiguration::WhiteButtonFrame : UiConfiguration::BlackButtonFrame);
			image->drawAlpha = UiConfiguration::instance->activeUnfocusedIconAlpha;
		}
	}

	if (shouldfillbg) {
		bgcolor.assign (bgcolor.r, bgcolor.g, bgcolor.b, bgalpha);
		setFillBg (true, bgcolor);
	}
	else {
		setFillBg (false);
	}

	if (isdropshadowed && (! isFocusDropShadowDisabled)) {
		setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	}
	else {
		setDropShadow (false);
	}

	if (label) {
		if (isDisabled) {
			if (isInverseColor) {
				label->textColor.assign (normalTextColor.copy (UiConfiguration::instance->buttonDisabledShadeAlpha));
			}
			else {
				label->textColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
			}
		}
		else {
			label->textColor.assign (normalTextColor);
		}
	}

	paddingh = (heightPadding * 2.0f);
	if (image && (! label)) {
		paddingh /= 2.0f;
	}
	x = widthPadding;
	y = paddingh / 2.0f;
	spacew = 0.0f;
	h = 0.0f;
	if (image && image->isVisible) {
		x += spacew;
		image->position.assign (x, y);
		x += maxImageWidth;
		spacew = UiConfiguration::instance->marginSize;
		if (image->height > h) {
			h = image->height;
		}
	}
	if (label) {
		x += spacew;
		label->position.assign (x, y);
		x += label->width;
		spacew = UiConfiguration::instance->marginSize;
		if (label->height > h) {
			h = label->height;
		}

		if (isFocused && (! isFocusDropShadowDisabled)) {
			label->position.move (-(Button::FocusTextOffset), -(Button::FocusTextOffset));
		}
	}

	x += widthPadding;
	if (image && image->isVisible) {
		h += (maxImageHeight - image->height);
	}
	setFixedSize (true, x, h + paddingh);
	if (image) {
		image->position.assign (image->position.x + ((maxImageWidth - image->width) / 2.0f), (height / 2.0f) - (image->height / 2.0f));
		image->setFixedCenter (true);
	}
	if (label) {
		label->position.assign (label->position.x, (height / 2.0f) - (label->height / 2.0f) + (label->descenderHeight / 2.0f));
	}
}
