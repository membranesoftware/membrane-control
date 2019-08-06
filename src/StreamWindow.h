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
// Panel that contains elements representing a stream item on a card view

#ifndef STREAM_WINDOW_H
#define STREAM_WINDOW_H

#include <vector>
#include "StdString.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Button.h"
#include "TextArea.h"
#include "Json.h"
#include "Panel.h"

class StreamWindow : public Panel {
public:
	StreamWindow (Json *streamItem);
	virtual ~StreamWindow ();

	// Read-only data members
	StdString streamId;
	StdString agentId;
	StdString streamName;
	StdString thumbnailPath;
	StdString hlsStreamPath;
	bool isSelected;
	float displayTimestamp;
	int thumbnailIndex;
	int segmentCount;
	std::vector<double> segmentPositions;
	int frameWidth;
	int frameHeight;
	int64_t duration;
	float frameRate;
	int64_t bitrate;

	// Set the layout type that should be used to arrange the panel's widgets, as specified by a CardView detail constant
	void setLayout (int layoutType, float maxPanelWidth);

	// Set a callback that should be invoked when the stream image is clicked
	void setStreamImageClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the view button is clicked
	void setViewButtonClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set the index that should be used to load the window's thumbnail image
	void setThumbnailIndex (int index);

	// Set the window's selected state
	void setSelected (bool selected);

	// Set the timestamp that should be shown for the media item, with a negative value specifying that no timestamp should be shown
	void setDisplayTimestamp (float timestamp);

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static StreamWindow *castWidget (Widget *widget);

	// Callback functions
	static void streamImageClicked (void *windowPtr, Widget *widgetPtr);
	static void streamImageLoaded (void *windowPtr, Widget *widgetPtr);
	static void viewButtonClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations appropriate when the widget receives new mouse state
	void doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	ImageWindow *streamImage;
	Label *nameLabel;
	TextArea *detailText;
	LabelWindow *mouseoverLabel;
	LabelWindow *detailNameLabel;
	LabelWindow *timestampLabel;
	Button *viewButton;
	Widget::EventCallback streamImageClickCallback;
	void *streamImageClickCallbackData;
	Widget::EventCallback viewButtonClickCallback;
	void *viewButtonClickCallbackData;
};

#endif
