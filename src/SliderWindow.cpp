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
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "UiConfiguration.h"
#include "Slider.h"
#include "SliderWindow.h"

SliderWindow::SliderWindow (Slider *slider)
: Panel ()
, value (0.0f)
, isHovering (false)
, slider (slider)
, valueLabel (NULL)
, valueNameFunction (NULL)
, valueChangeCallback (NULL)
, valueChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);

	value = slider->value;
	addWidget (slider);
	valueLabel = (Label *) addWidget (new Label (StdString::createSprintf ("%.2f", slider->value), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));

	slider->setValueChangeCallback (SliderWindow::sliderValueChanged, this);
	slider->setValueHoverCallback (SliderWindow::sliderValueHovered, this);

	resetLayout ();
}

SliderWindow::~SliderWindow () {

}

StdString SliderWindow::toStringDetail () {
	return (StdString (" SliderWindow"));
}

void SliderWindow::resetLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = 0.0f;
	y = 0.0f;
	valueLabel->position.assign (x, y);
	if (isHovering) {
		valueLabel->textColor.rotate (uiconfig->raisedButtonTextColor, uiconfig->colorRotateDuration);
	}
	else {
		valueLabel->textColor.rotate (uiconfig->lightPrimaryTextColor, uiconfig->colorRotateDuration);
	}

	y += valueLabel->maxLineHeight;
	slider->position.assign (x, y);

	resetSize ();
}

void SliderWindow::setValueChangeCallback (Widget::EventCallback callback, void *callbackData) {
	valueChangeCallback = callback;
	valueChangeCallbackData = callbackData;
}

void SliderWindow::setValueNameFunction (SliderWindow::ValueNameFunction fn) {
	valueNameFunction = fn;
	if (valueNameFunction) {
		valueLabel->setText (valueNameFunction (slider->value));
	}
}

void SliderWindow::setValue (float sliderValue) {
	slider->setValue (sliderValue);
}

void SliderWindow::sliderValueChanged (void *windowPtr, Widget *widgetPtr) {
	SliderWindow *window;
	Slider *slider;

	window = (SliderWindow *) windowPtr;
	slider = (Slider *) widgetPtr;

	window->value = slider->value;
	if (window->valueNameFunction) {
		window->valueLabel->setText (window->valueNameFunction (slider->value));
	}
	else {
		window->valueLabel->setText (StdString::createSprintf ("%.2f", slider->value));
	}

	if (window->valueChangeCallback) {
		window->valueChangeCallback (window->valueChangeCallbackData, window);
	}
}

void SliderWindow::sliderValueHovered (void *windowPtr, Widget *widgetPtr) {
	SliderWindow *window;
	Slider *slider;
	float val;

	window = (SliderWindow *) windowPtr;
	slider = window->slider;
	window->isHovering = slider->isHovering;

	if (slider->isHovering) {
		val = slider->hoverValue;
	}
	else {
		val = slider->value;
	}

	if (window->valueNameFunction) {
		window->valueLabel->setText (window->valueNameFunction (val));
	}
	else {
		window->valueLabel->setText (StdString::createSprintf ("%.2f", val));
	}
	window->resetLayout ();
}
