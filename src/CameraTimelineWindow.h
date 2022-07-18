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
// Panel that shows a horizontal bar representing the timeline of a camera's capture store

#ifndef CAMERA_TIMELINE_WINDOW_H
#define CAMERA_TIMELINE_WINDOW_H

#include "StdString.h"
#include "Sprite.h"
#include "LabelWindow.h"
#include "Panel.h"

class CameraTimelineWindow : public Panel {
public:
	CameraTimelineWindow (float barWidth);
	virtual ~CameraTimelineWindow ();

	// Read-write data members
	Widget::EventCallbackContext positionHoverCallback;
	Widget::EventCallbackContext positionClickCallback;

	// Read-only data members
	bool isDisabled;
	int64_t startTime;
	int64_t endTime;
	int64_t highlightTime;
	int64_t selectTime;
	int64_t timeRate; // Milliseconds per pixel of bar width
	float hoverPosition; // A negative value indicates that the mouse is positioned outside the timeline bar
	float clickPosition;

	// Set the width of the window's timeline bar
	void setBarWidth (float barWidthValue);

	// Set the window's disabled state, appropriate for use when the window becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the lower and upper bounds of the time period shown by the window
	void setTimespan (int64_t spanStartTime, int64_t spanEndTime);

	// Set a time value that should be highlighted by the window
	void setHighlightedTime (int64_t highlightTimeValue);

	// Set a time value that should be marked as selected by the window
	void setSelectedTime (int64_t selectTimeValue, bool isSpanDescending);

	// Clear timeline state data
	void clearTimeline ();

	// Store a point in timeline state, indicating an available capture image at that position
	void addTimelinePoint (int64_t t);

	// Mark a time range as covered within timeline state, indicating it has been scanned for capture images
	void addTimelineCoverRange (int64_t rangeMin, int64_t rangeMax);

protected:
	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Add subclass-specific draw commands for execution by the App. If targetTexture is non-NULL, it has been set as the render target and draw commands should adjust coordinates as appropriate.
	virtual void doDraw (SDL_Texture *targetTexture, float originX, float originY);

	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);

private:
	// Callback functions
	static void createBarTexture (void *windowPtr);

	// Allocate barPixels as a memory buffer holding bar state pixel data and return a boolean value indicating if the operation succeeded
	bool createBarPixels ();

	float barWidth;
	float barHeight;
	uint8_t *barState;
	int barStateSize;
	bool shouldRefreshBarTexture;
	Uint32 *barPixels;
	Sprite *barSprite;
	LabelWindow *startTimeLabel;
	LabelWindow *endTimeLabel;
	LabelWindow *highlightTimeLabel;
	Panel *highlightMarker;
	LabelWindow *selectTimeLabel;
	Panel *selectMarker;
};

#endif
