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
// Panel that contains elements for manipulating application-wide settings

#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include "StdString.h"
#include "Sprite.h"
#include "ImageWindow.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Button.h"
#include "SliderWindow.h"
#include "ToggleWindow.h"
#include "Json.h"
#include "Panel.h"

class SettingsWindow : public Panel {
public:
	SettingsWindow (float windowWidth, float windowHeight);
	virtual ~SettingsWindow ();

	// Set the window's size
	void setWindowSize (float windowWidth, float windowHeight);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	void doUpdate (int msElapsed);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static StdString windowSizeSliderValueName (float sliderValue);
	static StdString textSizeSliderValueName (float sliderValue);
	static void closeButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void windowSizeSliderChanged (void *windowPtr, Widget *widgetPtr);
	static void textSizeSliderChanged (void *windowPtr, Widget *widgetPtr);
	static void showClockToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void showInterfaceAnimationsToggleStateChanged (void *windowPtr, Widget *widgetPtr);

	ImageWindow *headerImage;
	bool isHeaderImageLoaded;
	LabelWindow *titleLabel;
	Button *closeButton;
	Label *windowSizeLabel;
	SliderWindow *windowSizeSlider;
	Label *textSizeLabel;
	SliderWindow *textSizeSlider;
	ToggleWindow *showClockToggle;
	ToggleWindow *showInterfaceAnimationsToggle;
};

#endif
