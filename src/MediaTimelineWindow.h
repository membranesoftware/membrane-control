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
// Panel that shows a horizontal bar with position indicators, representing the timeline of a MediaItem or StreamItem

#ifndef MEDIA_TIMELINE_WINDOW_H
#define MEDIA_TIMELINE_WINDOW_H

#include <list>
#include <vector>
#include "StdString.h"
#include "LabelWindow.h"
#include "Panel.h"

class MediaTimelineWindow : public Panel {
public:
	// recordId must reference a MediaItem or StreamItem record
	MediaTimelineWindow (float barWidth, const StdString &recordId);
	virtual ~MediaTimelineWindow ();

	// Read-write data members
	Widget::EventCallbackContext positionHoverCallback;
	Widget::EventCallbackContext positionClickCallback;

	// Read-only data members
	StdString recordId;
	StdString agentId;
	int recordType;
	int highlightedMarkerIndex;
	float hoverPosition; // A negative value indicates that the mouse is positioned outside the timeline bar
	float clickPosition;

	// Set the highlight marker's text color
	void setHighlightColor (const Color &color);

	// Set the bar to highlight the marker at the specified index. A negative index value indicates that any existing highlight should be cleared.
	void setHighlightedMarker (int markerIndex);

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);

private:
	static const int GuideSegmentCount;

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
};

#endif
