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
// Panel that represents a media item on a card view

#ifndef MEDIA_WINDOW_H
#define MEDIA_WINDOW_H

#include "StdString.h"
#include "Json.h"
#include "Widget.h"
#include "SpriteGroup.h"
#include "Label.h"
#include "LabelWindow.h"
#include "ImageWindow.h"
#include "TextFlow.h"
#include "Button.h"
#include "Panel.h"

class MediaWindow : public Panel {
public:
	MediaWindow (Json *mediaItem, SpriteGroup *mediaUiSprites);
	virtual ~MediaWindow ();

	// Read-write data members
	Widget::EventCallbackContext mediaImageClickCallback;
	Widget::EventCallbackContext viewButtonClickCallback;
	Widget::EventCallbackContext browserPlayButtonClickCallback;
	Widget::EventCallbackContext selectStateChangeCallback;

	// Read-only data members
	StdString mediaId;
	StdString mediaName;
	StdString mediaSortKey;
	StdString agentId;
	StdString mediaPath;
	StdString thumbnailPath;
	StdString hlsStreamPath;
	StdString htmlPlayerPath;
	int thumbnailCount;
	int mediaWidth, mediaHeight;
	float mediaDuration;
	float mediaFrameRate;
	int64_t mediaSize;
	int64_t mediaBitrate;
	bool isCreateStreamAvailable;
	StdString streamId;
	int64_t streamSize;
	StdString streamAgentId;
	StdString streamAgentName;
	StdString streamThumbnailPath;
	StdString playThumbnailUrl;
	int playThumbnailIndex;
	float displayTimestamp;
	bool isSelected;

	// Set the layout type that should be used to arrange the panel's widgets, as specified by a CardView detail constant
	void setLayout (int layoutType, float maxPanelWidth);

	// Set the timestamp that should be shown for the media item, with a negative value specifying that no timestamp should be shown
	void setDisplayTimestamp (float timestamp);

	// Set the window's select state, then execute any select state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setSelected (bool selected, bool shouldSkipStateChangeCallback = false);

	// Return a boolean value indicating if the window is configured to load thumbnail images
	bool hasThumbnails ();

	// Set the URL that should be used to load the window's thumbnail image, along with the thumbnail's stream index
	void setThumbnail (const StdString &imageUrl, int thumbnailIndex);

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static MediaWindow *castWidget (Widget *widget);

	// Match a StreamItem record by sourceId using the provided StdString pointer
	static bool matchStreamSourceId (void *idStringPtr, Json *record);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void mediaImageClicked (void *windowPtr, Widget *widgetPtr);
	static void mediaImageLoaded (void *windowPtr, Widget *widgetPtr);
	static void mediaImageLongPressed (void *windowPtr, Widget *widgetPtr);
	static void viewButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void browserPlayButtonClicked (void *windowPtr, Widget *widgetPtr);

	SpriteGroup *sprites;
	ImageWindow *mediaImage;
	Label *nameLabel;
	TextFlow *detailText;
	LabelWindow *mouseoverLabel;
	LabelWindow *detailNameLabel;
	Button *viewButton;
	Button *browserPlayButton;
	LabelWindow *timestampLabel;
	ImageWindow *streamIconImage;
	ImageWindow *createStreamUnavailableIconImage;
};

#endif
