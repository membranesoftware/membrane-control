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
#include "StdString.h"
#include "App.h"
#include "Sprite.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Toggle.h"
#include "ToggleWindow.h"

ToggleWindow::ToggleWindow (Toggle *toggle)
: Panel ()
, isChecked (false)
, isRightAligned (false)
, isInverseColor (false)
, toggle (toggle)
, label (NULL)
, iconImage (NULL)
{
	addWidget (toggle);
	toggle->isInputSuspended = true;
	toggle->stateChangeCallback = Widget::EventCallbackContext (ToggleWindow::toggleStateChanged, this);

	mouseEnterCallback = Widget::EventCallbackContext (ToggleWindow::mouseEntered, this);
	mouseExitCallback = Widget::EventCallbackContext (ToggleWindow::mouseExited, this);
	mousePressCallback = Widget::EventCallbackContext (ToggleWindow::mousePressed, this);
	mouseReleaseCallback = Widget::EventCallbackContext (ToggleWindow::mouseReleased, this);
	mouseClickCallback = Widget::EventCallbackContext (ToggleWindow::mouseClicked, this);

	refreshLayout ();
}

ToggleWindow::~ToggleWindow () {

}

StdString ToggleWindow::toStringDetail () {
	return (StdString::createSprintf (" ToggleWindow checked=%s text=\"%s\"", BOOL_STRING (isChecked), label ? label->text.c_str () : ""));
}

void ToggleWindow::setText (const StdString &text) {
	if (! label) {
		label = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
		label->isInputSuspended = true;
	}
	label->setText (text);
	label->textColor.assign (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);

	if (iconImage) {
		iconImage->isDestroyed = true;
		iconImage = NULL;
	}
	refreshLayout ();
}

void ToggleWindow::setIcon (Sprite *iconSprite) {
	if (iconImage) {
		iconImage->isDestroyed = true;
	}
	iconImage = (Image *) addWidget (new Image (iconSprite));
	iconImage->isInputSuspended = true;

	if (label) {
		label->isDestroyed = true;
		label = NULL;
	}
	refreshLayout ();
}

void ToggleWindow::setRightAligned (bool enable) {
	if (enable == isRightAligned) {
		return;
	}
	isRightAligned = enable;
	refreshLayout ();
}

void ToggleWindow::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	toggle->setInverseColor (isInverseColor);
	if (isInverseColor) {
		if (isFilledBg) {
			setFillBg (true, UiConfiguration::instance->lightPrimaryColor);
		}
		if (label) {
			label->textColor.assign (UiConfiguration::instance->inverseTextColor);
		}
	}
	else {
		if (isFilledBg) {
			setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		}
		if (label) {
			label->textColor.assign (UiConfiguration::instance->primaryTextColor);
		}
	}
}

void ToggleWindow::setImageColor (const Color &imageColor) {
	toggle->setImageColor (imageColor);
}

void ToggleWindow::refreshLayout () {
	float x, y;

	x = 0.0f;
	y = 0.0f;

	if (isRightAligned) {
		if (label || iconImage) {
			x += UiConfiguration::instance->paddingSize;
		}
	}
	else {
		toggle->flowRight (&x, y);
	}
	if (label) {
		x -= (UiConfiguration::instance->marginSize / 2.0f);
		label->flowRight (&x, y);
	}
	if (iconImage) {
		iconImage->flowRight (&x, y);
	}
	if (isRightAligned) {
		toggle->flowRight (&x, y);
	}

	resetSize ();
	if (label) {
		label->centerVertical (0.0f, height);
	}
	if (iconImage) {
		iconImage->centerVertical (0.0f, height);
	}
	toggle->centerVertical (0.0f, height);
	if ((! isRightAligned) && (label || iconImage)) {
		width += UiConfiguration::instance->paddingSize;
	}
}

void ToggleWindow::mouseEntered (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *window;

	window = (ToggleWindow *) windowPtr;
	window->toggle->mouseEnter ();
}

void ToggleWindow::mouseExited (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *window;

	window = (ToggleWindow *) windowPtr;
	window->toggle->mouseExit ();
}

void ToggleWindow::mousePressed (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *window;

	window = (ToggleWindow *) windowPtr;
	window->toggle->mousePress ();
}

void ToggleWindow::mouseReleased (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *window;

	window = (ToggleWindow *) windowPtr;
	window->toggle->mouseRelease ();
}

void ToggleWindow::mouseClicked (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *window;

	window = (ToggleWindow *) windowPtr;
	window->toggle->mouseClick ();
}

void ToggleWindow::setChecked (bool checked, bool shouldSkipChangeCallback) {
	toggle->setChecked (checked, shouldSkipChangeCallback);
	isChecked = toggle->isChecked;
}

void ToggleWindow::toggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *window;
	Toggle *toggle;

	window = (ToggleWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->isChecked = toggle->isChecked;
	window->eventCallback (window->stateChangeCallback);
}
