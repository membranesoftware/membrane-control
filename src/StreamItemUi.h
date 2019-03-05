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
// UI that shows details and controls for a stream item

#ifndef STREAM_ITEM_UI_H
#define STREAM_ITEM_UI_H

#include "StdString.h"
#include "Json.h"
#include "WidgetHandle.h"
#include "Toolbar.h"
#include "Button.h"
#include "HelpWindow.h"
#include "TimelineWindow.h"
#include "StreamWindow.h"
#include "IconCardWindow.h"
#include "CardView.h"
#include "Ui.h"

class StreamItemUi : public Ui {
public:
	StreamItemUi (StreamWindow *streamWindow, const StdString &captionText = StdString (""));
	~StreamItemUi ();

	// Constants to use for sprite indexes
	enum {
		TIME_ICON = 0,
		LARGE_THUMBNAILS_ICON = 1,
		MEDIUM_THUMBNAILS_ICON = 2,
		SMALL_THUMBNAILS_ICON = 3,
		THUMBNAIL_SIZE_BUTTON = 4
	};

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

	// Set a callback that should be invoked when a ThumbnailWindow item is clicked on the UI
	void setThumbnailClickCallback (Widget::EventCallback callback, void *callbackData);

	// Callback functions
	static void thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void viewSmallActionClicked (void *uiPtr, Widget *widgetPtr);
	static void viewMediumActionClicked (void *uiPtr, Widget *widgetPtr);
	static void viewLargeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void resetCardLayout (void *uiPtr, Widget *widgetPtr);
	static void thumbnailClicked (void *uiPtr, Widget *widgetPtr);
	static void thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void thumbnailMouseExited (void *uiPtr, Widget *widgetPtr);
	static void timelineWindowPositionHovered (void *uiPtr, Widget *widgetPtr);
	static void timelineWindowPositionClicked (void *uiPtr, Widget *widgetPtr);
	static bool matchThumbnailIndex (void *intPtr, Widget *widgetPtr);

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

	// Remove and destroy any subclass-specific popup widgets that have been created by the UI
	void doClearPopupWidgets ();

	// Execute subclass-specific operations to refresh the interface's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Reload subclass-specific interface resources as needed to account for a new application window size
	void doResize ();

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

private:
	// Execute actions appropriate when the view button is clicked
	void handleThumbnailSizeButtonClick (Widget *buttonWidget);

	WidgetHandle sourceStreamWindow;
	StdString captionText;
	CardView *cardView;
	int cardLayout;
	float cardMaxImageWidth;
	TimelineWindow *timelineWindow;
	WidgetHandle thumbnailSizeMenu;
	int lastTimelineHoverPosition;
	Widget::EventCallback thumbnailClickCallback;
	void *thumbnailClickCallbackData;
};

#endif
