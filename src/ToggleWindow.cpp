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
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "Toggle.h"
#include "ToggleWindow.h"

ToggleWindow::ToggleWindow (Toggle *toggle, const StdString &labelText)
: Panel ()
, isChecked (false)
, toggle (toggle)
, label (NULL)
, stateChangeCallback (NULL)
, stateChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);

	addWidget (toggle);
	toggle->isInputSuspended = true;
	toggle->setStateChangeCallback (ToggleWindow::toggleStateChanged, this);

	if (! labelText.empty ()) {
		label = (Label *) addWidget (new Label (labelText, UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
		label->isInputSuspended = true;
	}

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

void ToggleWindow::setPadding (float widthPadding, float heightPadding) {
	Panel::setPadding (widthPadding, heightPadding);
	refreshLayout ();
}

void ToggleWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;
	toggle->position.assign (x, y);
	x += toggle->width;
	if (label) {
		x += (uiconfig->marginSize / 2.0f);
		label->position.assign (x, y);
		x += label->width + uiconfig->marginSize;
	}

	resetSize ();
	if (label) {
		label->position.assignY ((height / 2.0f) - (label->height / 2.0f));
	}
	toggle->position.assignY ((height / 2.0f) - (toggle->height / 2.0f));
	resetSize ();
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

void ToggleWindow::setImageColor (const Color &imageColor) {
	toggle->setImageColor (imageColor);
}

void ToggleWindow::setStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	stateChangeCallback = callback;
	stateChangeCallbackData = callbackData;
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
