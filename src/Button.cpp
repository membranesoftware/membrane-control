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
#include "SDL2/SDL.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Sprite.h"
#include "Label.h"
#include "Image.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "Button.h"

Button::Button (const StdString &labelText, Sprite *sprite, bool shouldDestroySprite)
: Panel ()
, shortcutKey (SDLK_UNKNOWN)
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
	UiConfiguration *uiconfig;

	widgetType.assign ("Button");

	uiconfig = &(App::getInstance ()->uiConfig);
	if (! labelText.empty ()) {
		normalTextColor.assign (uiconfig->flatButtonTextColor);
		label = (Label *) addWidget (new Label (labelText, UiConfiguration::BUTTON, normalTextColor));
	}
	if (sprite) {
		image = (Image *) addWidget (new Image (sprite, UiConfiguration::WHITE_BUTTON_FRAME, shouldDestroySprite));
		image->drawAlpha = uiconfig->activeFocusedIconAlpha;
		maxImageWidth = image->maxSpriteWidth;
		maxImageHeight = image->maxSpriteHeight;
	}

	widthPadding = uiconfig->paddingSize;
	heightPadding = uiconfig->paddingSize;

	setMouseEnterCallback (Button::mouseEntered, this);
	setMouseExitCallback (Button::mouseExited, this);
	setMousePressCallback (Button::mousePressed, this);
	setMouseReleaseCallback (Button::mouseReleased, this);

	refreshLayout ();
}

Button::~Button () {

}

bool Button::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	return (widget->widgetType.equals ("Button"));
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
			label = (Label *) addWidget (new Label (text, UiConfiguration::BUTTON, normalTextColor));
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
	if (isDisabled || isPressed) {
		return (false);
	}

	if ((shortcutKey != SDLK_UNKNOWN) && (keycode == shortcutKey)) {
		setPressed (true);
		pressClock = App::getInstance ()->uiConfig.blinkDuration;
		refreshLayout ();
		if (mouseClickCallback) {
			mouseClickCallback (mouseClickCallbackData, this);
		}
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

void Button::doUpdate (int msElapsed, float originX, float originY) {
	Panel::doUpdate (msElapsed, originX, originY);
	if (pressClock > 0) {
		pressClock -= msElapsed;
		if (pressClock <= 0) {
			pressClock = 0;
		}
	}

	if (! isInputSuspended) {
		if (isPressed && (pressClock <= 0)) {
			if ((! isMouseEntered) || (! App::getInstance ()->input.isMouseLeftButtonDown)) {
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
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	widthPadding = uiconfig->paddingSize;
	heightPadding = uiconfig->paddingSize;
	Panel::doRefresh ();
	if (image) {
		maxImageWidth = image->maxSpriteWidth;
		maxImageHeight = image->maxSpriteHeight;
	}
	refreshLayout ();
}

void Button::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, h, spacew, paddingh, bgalpha;
	bool shouldfillbg, iswhiteframe;
	Color bgcolor, bordercolor, shadecolor;

	// TODO: Handle borders / shadows in this operation

	uiconfig = &(App::getInstance ()->uiConfig);

	shouldfillbg = false;
	bgalpha = -1.0f;
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
			image->setFrame (iswhiteframe ? UiConfiguration::WHITE_BUTTON_FRAME : UiConfiguration::BLACK_BUTTON_FRAME);
			image->drawAlpha = uiconfig->activeUnfocusedIconAlpha;
		}

		if (isRaised) {
			bgalpha = uiconfig->buttonDisabledShadeAlpha;
		}
	}
	else if (isPressed) {
		if (image) {
			image->setFrame (iswhiteframe ? UiConfiguration::WHITE_LARGE_BUTTON_FRAME : UiConfiguration::BLACK_LARGE_BUTTON_FRAME);
			image->drawAlpha = uiconfig->activeFocusedIconAlpha;
		}

		shouldfillbg = true;
		if (isRaised) {
			bgcolor.blend (shadecolor, uiconfig->buttonPressedShadeAlpha);
		}
		else {
			bgalpha = uiconfig->buttonPressedShadeAlpha;
			bgcolor.assign (shadecolor);
		}
	}
	else if (isFocused) {
		if (image) {
			image->setFrame (iswhiteframe ? UiConfiguration::WHITE_LARGE_BUTTON_FRAME : UiConfiguration::BLACK_LARGE_BUTTON_FRAME);
			image->drawAlpha = uiconfig->activeFocusedIconAlpha;
		}

		shouldfillbg = true;
		if (isRaised) {
			bgcolor.blend (shadecolor, uiconfig->buttonFocusedShadeAlpha);
		}
		else {
			bgalpha = uiconfig->buttonFocusedShadeAlpha;
			bgcolor.assign (shadecolor);
		}
	}
	else {
		if (image) {
			image->setFrame (iswhiteframe ? UiConfiguration::WHITE_BUTTON_FRAME : UiConfiguration::BLACK_BUTTON_FRAME);
			image->drawAlpha = uiconfig->activeUnfocusedIconAlpha;
		}
	}

	if (shouldfillbg) {
		if (bgalpha >= 0.0f) {
			setAlphaBlend (true, bgalpha);
		}
		else {
			setAlphaBlend (false);
		}

		setFillBg (true, bgcolor);
	}
	else {
		setFillBg (false);
		setBorder (false);
	}

	if (label) {
		if (isDisabled) {
			label->textColor.assign (uiconfig->lightInverseBackgroundColor);
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
		spacew = uiconfig->marginSize;
		if (image->height > h) {
			h = image->height;
		}
	}
	if (label) {
		x += spacew;
		label->position.assign (x, y);
		x += label->width;
		spacew = uiconfig->marginSize;
		if (label->height > h) {
			h = label->height;
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
