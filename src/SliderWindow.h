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
// Panel that holds a Slider widget and related text elements

#ifndef SLIDER_WINDOW_H
#define SLIDER_WINDOW_H

#include "StdString.h"
#include "Slider.h"
#include "Label.h"
#include "Panel.h"

class SliderWindow : public Panel {
public:
	SliderWindow (Slider *slider);
	virtual ~SliderWindow ();

	typedef StdString (*ValueNameFunction) (float sliderValue);

	// Read-only data members
	float value;
	bool isHovering;

	// Set a callback function that should be invoked when the slider's value changes
	void setValueChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Set a function that should be invoked to convert the slider's numeric value to a name string
	void setValueNameFunction (SliderWindow::ValueNameFunction fn);

	// Set the slider's value
	void setValue (float sliderValue);

	// Callback functions
	static void sliderValueChanged (void *windowPtr, Widget *widgetPtr);
	static void sliderValueHovered (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void resetLayout ();

private:
	Slider *slider;
	Label *valueLabel;
	SliderWindow::ValueNameFunction valueNameFunction;
	Widget::EventCallback valueChangeCallback;
	void *valueChangeCallbackData;
};
#endif
