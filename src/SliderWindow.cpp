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
#include "Widget.h"
#include "Sprite.h"
#include "Panel.h"
#include "Label.h"
#include "UiConfiguration.h"
#include "Slider.h"
#include "SliderWindow.h"

SliderWindow::SliderWindow (Slider *slider)
: Panel ()
, isDisabled (false)
, isInverseColor (false)
, value (0.0f)
, isHovering (false)
, slider (slider)
, iconImage (NULL)
, valueLabel (NULL)
, valueNameFunction (NULL)
{
	value = slider->value;
	addWidget (slider);
	valueLabel = (Label *) addWidget (new Label (StdString::createSprintf ("%.2f", slider->value), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));

	slider->valueChangeCallback = Widget::EventCallbackContext (SliderWindow::sliderValueChanged, this);
	slider->valueHoverCallback = Widget::EventCallbackContext (SliderWindow::sliderValueHovered, this);

	refreshLayout ();
}

SliderWindow::~SliderWindow () {

}

StdString SliderWindow::toStringDetail () {
	return (StdString (" SliderWindow"));
}

void SliderWindow::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}

	isDisabled = disabled;
	slider->setDisabled (isDisabled);
	refreshLayout ();
}

void SliderWindow::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	slider->setInverseColor (isInverseColor);
	refreshLayout ();
}

void SliderWindow::setPadding (float widthPadding, float heightPadding) {
	Panel::setPadding (widthPadding, heightPadding);
	refreshLayout ();
}

void SliderWindow::setIcon (Sprite *iconSprite) {
	if (iconImage) {
		iconImage->isDestroyed = true;
	}
	iconImage = (Image *) addWidget (new Image (iconSprite));
	iconImage->isInputSuspended = true;
	refreshLayout ();
}

void SliderWindow::refreshLayout () {
	Color color;
	float x, y, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x2 = 0.0f;
	y2 = 0.0f;
	if (iconImage) {
		iconImage->flowRight (&x, y, &x2, &y2);
	}
	valueLabel->flowRight (&x, y, &x2, &y2);
	slider->position.assign (valueLabel->position.x, y + valueLabel->maxLineHeight + (UiConfiguration::instance->marginSize / 2.0f));
	resetSize ();
	if (iconImage) {
		iconImage->centerVertical (0.0f, height);
	}

	if (isDisabled) {
		color.assign (isInverseColor ? UiConfiguration::instance->darkInverseTextColor : UiConfiguration::instance->lightPrimaryTextColor);
	}
	else {
		color.assign (isInverseColor ? UiConfiguration::instance->darkBackgroundColor : UiConfiguration::instance->lightPrimaryColor);
	}
	valueLabel->textColor.translate (color, UiConfiguration::instance->shortColorTranslateDuration);
}

void SliderWindow::setValueNameFunction (SliderWindow::ValueNameFunction fn) {
	valueNameFunction = fn;
	if (valueNameFunction) {
		valueLabel->setText (valueNameFunction (slider->value));
		refreshLayout ();
	}
}

void SliderWindow::setValue (float sliderValue, bool shouldSkipChangeCallback) {
	slider->setValue (sliderValue, shouldSkipChangeCallback);
	value = slider->value;
	if (shouldSkipChangeCallback) {
		if (valueNameFunction) {
			valueLabel->setText (valueNameFunction (slider->value));
		}
		else {
			valueLabel->setText (StdString::createSprintf ("%.2f", slider->value));
		}
		refreshLayout ();
	}
}

void SliderWindow::setTrackWidthScale (float scale) {
	slider->setTrackWidthScale (scale);
	refreshLayout ();
}

void SliderWindow::addSnapValue (float snapValue) {
	slider->addSnapValue (snapValue);
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
	window->refreshLayout ();
	window->eventCallback (window->valueChangeCallback);
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
	window->refreshLayout ();
}
