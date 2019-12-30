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
#include "Result.h"
#include "Log.h"
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
, toggle (toggle)
, label (NULL)
, iconImage (NULL)
, stateChangeCallback (NULL)
, stateChangeCallbackData (NULL)
{
	addWidget (toggle);
	toggle->isInputSuspended = true;
	toggle->setStateChangeCallback (ToggleWindow::toggleStateChanged, this);

	setMouseEnterCallback (ToggleWindow::mouseEntered, this);
	setMouseExitCallback (ToggleWindow::mouseExited, this);
	setMousePressCallback (ToggleWindow::mousePressed, this);
	setMouseReleaseCallback (ToggleWindow::mouseReleased, this);
	setMouseClickCallback (ToggleWindow::mouseClicked, this);

	refreshLayout ();
}

ToggleWindow::~ToggleWindow () {

}

StdString ToggleWindow::toStringDetail () {
	return (StdString::createSprintf (" ToggleWindow text=\"%s\"", label ? label->text.c_str () : ""));
}

void ToggleWindow::setText (const StdString &text) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	if (! label) {
		label = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
		label->isInputSuspended = true;
	}
	label->setText (text);

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

void ToggleWindow::setImageColor (const Color &imageColor) {
	toggle->setImageColor (imageColor);
}

void ToggleWindow::setStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	stateChangeCallback = callback;
	stateChangeCallbackData = callbackData;
}

void ToggleWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	x = 0.0f;
	y = 0.0f;

	if (isRightAligned) {
		if (label || iconImage) {
			x += uiconfig->paddingSize;
		}
	}
	else {
		toggle->flowRight (&x, y);
	}
	if (label) {
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
		width += uiconfig->paddingSize;
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

void ToggleWindow::setChecked (bool checked) {
	toggle->setChecked (checked);
}

void ToggleWindow::toggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *window;
	Toggle *toggle;

	window = (ToggleWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->isChecked = toggle->isChecked;
	if (window->stateChangeCallback) {
		window->stateChangeCallback (window->stateChangeCallbackData, window);
	}
}
