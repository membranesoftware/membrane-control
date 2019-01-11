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
// Widget that shows a horizontal bar with time position indicators

#ifndef TIMELINE_BAR_H
#define TIMELINE_BAR_H

#include <list>
#include "StdString.h"
#include "LabelWindow.h"
#include "Panel.h"

class TimelineBar : public Panel {
public:
	// recordId must reference a MediaItem or StreamItem record
	TimelineBar (float barWidth, const StdString &recordId);
	virtual ~TimelineBar ();

	// Read-only data members
	StdString recordId;
	StdString agentId;
	int recordType;
	int highlightedMarkerIndex;
	float hoverPosition;
	float clickPosition;

	// Set a callback that should be invoked when the mouse hovers on a timeline position. The hoverPosition field stores the hovered value, with a negative value indicating that the mouse is outside the timeline bar.
	void setPositionHoverCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the mouse clicks on a timeline position, with the clickPosition field storing the clicked value
	void setPositionClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set the bar to highlight the marker at the specified index. A negative index value indicates that any existing highlight should be cleared.
	void setHighlightedMarker (int markerIndex);

	// Execute operations appropriate to sync widget state with records present in the provided RecordStore object, which has been locked prior to invocation
	void syncRecordStore (RecordStore *store);

protected:
	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

	// Execute operations appropriate when the widget receives new mouse state
	virtual void doProcessMouseState (const Widget::MouseState &mouseState);

private:
	static const int guideSegmentCount;

	// If markerList is empty, populate it with a set of Panel widgets meant to highlight positions on the timeline
	void populateMarkers ();

	std::list<Panel *> markerList;
	std::vector<double> segmentPositionList;
	Panel *streamHighlightMarker;
	LabelWindow *startTimeLabel;
	LabelWindow *endTimeLabel;
	LabelWindow *highlightTimeLabel;
	float barWidth;
	float barHeight;
	float duration;
	int minDurationUnitType;
	int thumbnailCount;
	Widget::EventCallback positionHoverCallback;
	void *positionHoverCallbackData;
	Widget::EventCallback positionClickCallback;
	void *positionClickCallbackData;
};

#endif
