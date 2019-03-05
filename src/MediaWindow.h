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
// Panel that contains elements representing a media item on a card view

#ifndef MEDIA_WINDOW_H
#define MEDIA_WINDOW_H

#include "StdString.h"
#include "Json.h"
#include "Label.h"
#include "LabelWindow.h"
#include "ImageWindow.h"
#include "TextArea.h"
#include "Panel.h"

class MediaWindow : public Panel {
public:
	// Constants to use for layout types
	enum {
		LOW_DETAIL = 0,
		MEDIUM_DETAIL = 1,
		HIGH_DETAIL = 2
	};

	MediaWindow (Json *mediaItem, int layoutType = MediaWindow::LOW_DETAIL, float maxMediaImageWidth = 64.0f);
	virtual ~MediaWindow ();

	// Read-only data members
	StdString mediaId;
	StdString mediaName;
	StdString agentId;
	StdString mediaPath;
	StdString thumbnailPath;
	int thumbnailCount;
	int mediaWidth, mediaHeight;
	float mediaDuration;
	float mediaFrameRate;
	int64_t mediaSize;
	int64_t mediaBitrate;
	StdString streamId;
	StdString streamAgentId;
	StdString streamAgentName;

	// Set the card's layout type and reset widgets to show the specified content
	void setLayout (int layoutType, float maxImageWidth);

	// Return a boolean value indicating if the window is configured to load thumbnail images
	bool hasThumbnails ();

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static MediaWindow *castWidget (Widget *widget);

	// Callback functions
	static bool matchStreamSourceId (void *idStringPtr, Json *record);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations appropriate when the widget receives new mouse state
	void doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	ImageWindow *mediaImage;
	Label *nameLabel;
	TextArea *detailText;
	LabelWindow *mouseoverLabel;
	LabelWindow *detailNameLabel;
};

#endif
