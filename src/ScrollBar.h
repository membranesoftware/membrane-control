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
// Widget that allows control over a scrolling view

#ifndef SCROLL_BAR_H
#define SCROLL_BAR_H

#include <list>
#include "StdString.h"
#include "Color.h"
#include "Image.h"
#include "Panel.h"

class ScrollBar : public Panel {
public:
	ScrollBar (float maxScrollTrackLength);
	virtual ~ScrollBar ();

	// Read-only data members
	float scrollPosition;
	float maxScrollPosition;

	// Set the scrollbar's position and invoke any configured change callback unless shouldSkipCallback is true
	void setPosition (float positionValue, bool shouldSkipCallback = false);

	// Set a callback function that should be invoked when the scroll bar's position changes
	void setPositionChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Set the maximum track length of the scrollbar
	void setMaxTrackLength (float maxScrollTrackLength);

	// Set the visible height and total scrollable area that should be used to determine the scroll bar's appearance
	void setScrollBounds (float scrollViewHeight, float scrollAreaHeight);

protected:
	// Execute subclass-specific operations to update object state as appropriate for an elapsed millisecond time period and origin position
	virtual void doUpdate (int msElapsed, float originX, float originY);

	// Execute operations appropriate when the widget receives new mouse state
	virtual void doProcessMouseState (const Widget::MouseState &mouseState);

	// Execute operations appropriate when the widget's input state is reset
	virtual void doResetInputState ();

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	float maxTrackLength;
	float trackLength;
	float trackWidth;
	Panel *arrowPanel;
	Image *upArrowImage;
	Image *downArrowImage;
	Widget::EventCallback positionChangeCallback;
	void *positionChangeCallbackData;
	bool isFollowingMouse;
};

#endif