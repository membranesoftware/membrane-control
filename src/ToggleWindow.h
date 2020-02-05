/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Panel that holds a Toggle widget and a label or image

#ifndef TOGGLE_WINDOW_H
#define TOGGLE_WINDOW_H

#include "StdString.h"
#include "Toggle.h"
#include "Label.h"
#include "Image.h"
#include "Sprite.h"
#include "Panel.h"

class ToggleWindow : public Panel {
public:
	ToggleWindow (Toggle *toggle);
	virtual ~ToggleWindow ();

	// Read-only data members
	bool isChecked;
	bool isRightAligned;

	// Set the toggle to show a text label
	void setText (const StdString &text);

	// Set the toggle to show an icon image
	void setIcon (Sprite *iconSprite);

	// Set the window's right-aligned option. If enabled, the window places the toggle on the right side instead of the left side.
	void setRightAligned (bool enable);

	// Set the draw color for the toggle's button images
	void setImageColor (const Color &imageColor);

	// Set a callback that should be invoked when the toggle button's checked state changes
	void setStateChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Set the toggle's checked state
	void setChecked (bool checked);

	// Callback functions
	static void mouseEntered (void *windowPtr, Widget *widgetPtr);
	static void mouseExited (void *windowPtr, Widget *widgetPtr);
	static void mousePressed (void *windowPtr, Widget *widgetPtr);
	static void mouseReleased (void *windowPtr, Widget *widgetPtr);
	static void mouseClicked (void *windowPtr, Widget *widgetPtr);
	static void toggleStateChanged (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	Toggle *toggle;
	Label *label;
	Image *iconImage;
	Widget::EventCallback stateChangeCallback;
	void *stateChangeCallbackData;
};

#endif
