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
// UI that shows details and controls for a stream item

#ifndef STREAM_ITEM_UI_H
#define STREAM_ITEM_UI_H

#include <vector>
#include "StdString.h"
#include "Json.h"
#include "WidgetHandle.h"
#include "Toolbar.h"
#include "HelpWindow.h"
#include "Ui.h"

class StreamItemUi : public Ui {
public:
	// Sprite indexes
	enum {
		TimeIconSprite = 0,
		AttributesIconSprite = 1,
		DurationIconSprite = 2
	};

	// Card view row numbers
	enum {
		InfoRow = 0,
		ImageRow = 1
	};

	// Prefs keys
	static const char *ImageSizeKey;

	StreamItemUi (const StdString &streamId, const StdString &streamName);
	~StreamItemUi ();

	// Read-write data members
	Widget::EventCallbackContext thumbnailClickCallback;

	// Read-only data members
	StdString streamId;
	StdString streamName;
	StdString agentId;
	int64_t duration;
	int frameWidth;
	int frameHeight;
	StdString thumbnailPath;
	int segmentCount;
	std::vector<double> segmentPositions;

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	StdString getSpritePath ();

	// Return a newly created widget for use as a main toolbar breadcrumb item
	Widget *createBreadcrumbWidget ();

	// Load subclass-specific resources and return a result value
	int doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Add subclass-specific items to the provided main toolbar object
	void doAddMainToolbarItems (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	void doAddSecondaryToolbarItems (Toolbar *toolbar);

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

private:
	// Callback functions
	static void imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void thumbnailClicked (void *uiPtr, Widget *widgetPtr);
	static void thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void thumbnailMouseExited (void *uiPtr, Widget *widgetPtr);
	static void timelineWindowPositionHovered (void *uiPtr, Widget *widgetPtr);
	static void timelineWindowPositionClicked (void *uiPtr, Widget *widgetPtr);
	static void selectToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static bool matchThumbnailIndex (void *intPtr, Widget *widgetPtr);

	// Find the source StreamItem record and populate the interface based on its fields
	void syncStreamItem ();

	WidgetHandle timelineWindow;
	WidgetHandle commandCaption;
	bool isRecordSynced;
	int cardDetail;
	int lastTimelineHoverPosition;
	bool isSelectingPlayPosition;
};

#endif
