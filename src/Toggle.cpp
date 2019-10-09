/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "ClassId.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Sprite.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "Toggle.h"

Toggle::Toggle (Sprite *uncheckedButtonSprite, Sprite *checkedButtonSprite)
: Panel ()
, shortcutKey (SDLK_UNKNOWN)
, isFocusDropShadowDisabled (false)
, isChecked (false)
, isDisabled (false)
, isFocused (false)
, uncheckedButton (NULL)
, checkedButton (NULL)
, stateChangeCallback (NULL)
, stateChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	classId = ClassId::Toggle;

	// TODO: Add an indeterminate toggle state (for use when disabled)

	uiconfig = &(App::instance->uiConfig);
	if (uncheckedButtonSprite && checkedButtonSprite) {
		uncheckedButton = (Button *) addWidget (new Button (StdString (""), uncheckedButtonSprite));
		checkedButton = (Button *) addWidget (new Button (StdString (""), checkedButtonSprite));
	}
	else {
		uncheckedButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::ToggleCheckboxOutlineSprite)));
		checkedButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::ToggleCheckboxSprite)));
	}
	uncheckedButton->isInputSuspended = true;
	checkedButton->isInputSuspended = true;
	uncheckedButton->isFocusDropShadowDisabled = true;
	checkedButton->isFocusDropShadowDisabled = true;
	checkedButton->isVisible = false;

	setMouseEnterCallback (Toggle::mouseEntered, this);
	setMouseExitCallback (Toggle::mouseExited, this);
	setMousePressCallback (Toggle::mousePressed, this);
	setMouseReleaseCallback (Toggle::mouseReleased, this);
	setMouseClickCallback (Toggle::mouseClicked, this);

	refreshLayout ();
}

Toggle::~Toggle () {

}

bool Toggle::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::Toggle));
}

Toggle *Toggle::castWidget (Widget *widget) {
	return (Toggle::isWidgetType (widget) ? (Toggle *) widget : NULL);
}

void Toggle::refreshLayout () {
	resetSize ();
}

void Toggle::setChecked (bool checked, bool shouldSkipChangeCallback) {
	if (checked == isChecked) {
		return;
	}

	isChecked = checked;
	if (isChecked) {
		uncheckedButton->isVisible = false;
		uncheckedButton->setFocused (false);
		checkedButton->setFocused (isFocused);
		checkedButton->isVisible = true;
	}
	else {
		checkedButton->isVisible = false;
		checkedButton->setFocused (false);
		uncheckedButton->setFocused (isFocused);
		uncheckedButton->isVisible = true;
	}
	refreshLayout ();
	if (stateChangeCallback && (! shouldSkipChangeCallback)) {
		stateChangeCallback (stateChangeCallbackData, this);
	}
}

void Toggle::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}

	isDisabled = disabled;
	uncheckedButton->setDisabled (isDisabled);
	checkedButton->setDisabled (isDisabled);
	if (isDisabled) {
		isFocused = false;
		setDropShadow (false);
	}
	refreshLayout ();
}

void Toggle::setInverseColor (bool inverse) {
	uncheckedButton->setInverseColor (inverse);
	checkedButton->setInverseColor (inverse);
}

void Toggle::setImageColor (const Color &imageColor) {
	uncheckedButton->setImageColor (imageColor);
	checkedButton->setImageColor (imageColor);
}

void Toggle::setStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	stateChangeCallback = callback;
	stateChangeCallbackData = callbackData;
}

void Toggle::setStateMouseHoverTooltips (const StdString &uncheckedTooltip, const StdString &checkedTooltip, int alignment) {
	uncheckedButton->setMouseHoverTooltip (uncheckedTooltip, alignment);
	checkedButton->setMouseHoverTooltip (checkedTooltip, alignment);
}

void Toggle::doResetInputState () {
	Panel::doResetInputState ();
	isFocused = false;
	setDropShadow (false);
	uncheckedButton->setFocused (false);
	checkedButton->setFocused (false);
}

bool Toggle::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (isDisabled || isShiftDown || isControlDown) {
		return (false);
	}

	if ((shortcutKey != SDLK_UNKNOWN) && (keycode == shortcutKey)) {
		setChecked (! isChecked);
		return (true);
	}

	return (false);
}

void Toggle::mouseEntered (void *togglePtr, Widget *widgetPtr) {
	Toggle *toggle;
	UiConfiguration *uiconfig;

	toggle = (Toggle *) togglePtr;
	uiconfig = &(App::instance->uiConfig);
	if (toggle->isDisabled) {
		return;
	}

	toggle->isFocused = true;
	toggle->checkedButton->mouseEnter ();
	toggle->uncheckedButton->mouseEnter ();
	if (! toggle->isFocusDropShadowDisabled) {
		toggle->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	}
}

void Toggle::mouseExited (void *togglePtr, Widget *widgetPtr) {
	Toggle *toggle;

	toggle = (Toggle *) togglePtr;
	if (toggle->isDisabled) {
		return;
	}

	toggle->isFocused = false;
	toggle->checkedButton->mouseExit ();
	toggle->uncheckedButton->mouseExit ();
	if (! toggle->isFocusDropShadowDisabled) {
		toggle->setDropShadow (false);
	}
}

void Toggle::mousePressed (void *togglePtr, Widget *widgetPtr) {
	Toggle *toggle;

	toggle = (Toggle *) togglePtr;
	if (toggle->isDisabled) {
		return;
	}

	toggle->checkedButton->mousePress ();
	toggle->uncheckedButton->mousePress ();
}

void Toggle::mouseReleased (void *togglePtr, Widget *widgetPtr) {
	Toggle *toggle;

	toggle = (Toggle *) togglePtr;
	if (toggle->isDisabled) {
		return;
	}

	toggle->checkedButton->mouseRelease ();
	toggle->uncheckedButton->mouseRelease ();
}

void Toggle::mouseClicked (void *togglePtr, Widget *widgetPtr) {
	Toggle *toggle;

	toggle = (Toggle *) togglePtr;
	if (toggle->isDisabled) {
		return;
	}

	toggle->setChecked (! toggle->isChecked);
}
