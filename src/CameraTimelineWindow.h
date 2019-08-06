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
// Panel that shows a horizontal bar representing the timeline of a camera's capture store

#ifndef CAMERA_TIMELINE_WINDOW_H
#define CAMERA_TIMELINE_WINDOW_H

#include <list>
#include <vector>
#include "StdString.h"
#include "LabelWindow.h"
#include "Panel.h"

class CameraTimelineWindow : public Panel {
public:
	CameraTimelineWindow (float barWidth);
	virtual ~CameraTimelineWindow ();

	// Read-only data members
	bool isDisabled;
	int64_t startTime;
	int64_t endTime;
	int64_t highlightTime;
	int64_t selectTime;
	float hoverPosition;
	float clickPosition;

	// Set a callback that should be invoked when the mouse hovers on a timeline position. The hoverPosition field stores the hovered value, with a negative value indicating that the mouse is outside the timeline bar.
	void setPositionHoverCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the mouse clicks on a timeline position, with the clickPosition field storing the clicked value
	void setPositionClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set the window's disabled state, appropriate for use when the window becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the lower and upper bounds of the time period shown by the window
	void setTimespan (int64_t spanStartTime, int64_t spanEndTime);

	// Set a time value that should be highlighted by the window
	void setHighlightedTime (int64_t highlightTimeValue);

	// Set a time value that should be marked as selected by the window
	void setSelectedTime (int64_t selectTimeValue, bool isSpanDescending);

protected:
	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

	// Execute operations appropriate when the widget receives new mouse state
	void doProcessMouseState (const Widget::MouseState &mouseState);

private:
	float barWidth;
	float barHeight;
	LabelWindow *startTimeLabel;
	LabelWindow *endTimeLabel;
	LabelWindow *highlightTimeLabel;
	Panel *highlightMarker;
	LabelWindow *selectTimeLabel;
	Panel *selectMarker;
	Widget::EventCallback positionHoverCallback;
	void *positionHoverCallbackData;
	Widget::EventCallback positionClickCallback;
	void *positionClickCallbackData;
};

#endif
