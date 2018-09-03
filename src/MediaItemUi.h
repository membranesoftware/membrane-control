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
// UI that shows details and controls for a media item

#ifndef MEDIA_ITEM_UI_H
#define MEDIA_ITEM_UI_H

#include "StdString.h"
#include "Json.h"
#include "RecordStore.h"
#include "WidgetHandle.h"
#include "Button.h"
#include "Toolbar.h"
#include "HashMap.h"
#include "TimelineBar.h"
#include "MediaWindow.h"
#include "IconCardWindow.h"
#include "CardView.h"
#include "Ui.h"

class MediaItemUi : public Ui {
public:
	MediaItemUi (MediaWindow *card);
	~MediaItemUi ();

	// Constants to use for sprite indexes
	enum {
		LOADING_IMAGE_ICON = 0,
		LARGE_THUMBNAILS_ICON = 1,
		MEDIUM_THUMBNAILS_ICON = 2,
		SMALL_THUMBNAILS_ICON = 3,
		VIEW_BUTTON = 4,
		TIME_ICON = 5,
		MEDIA_ICON = 6
	};

	// Return text that should be used to identify the UI in a set of breadcrumb actions, or an empty string if no such title exists
	StdString getBreadcrumbTitle ();

	// Return a newly created Sprite object that should be used to identify the UI in a set of breadcrumb actions, or NULL if no such Sprite exists. If a Sprite is returned by this method, the caller becomes responsible for destroying it when no longer needed.
	Sprite *getBreadcrumbSprite ();

	// Callback functions
	static void viewButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void viewSmallActionClicked (void *uiPtr, Widget *widgetPtr);
	static void viewMediumActionClicked (void *uiPtr, Widget *widgetPtr);
	static void viewLargeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void resetCardLayout (void *uiPtr, Widget *widgetPtr);
	static bool sortCards (Widget *a, Widget *b);
	static void createStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void removeStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void createStreamActionClosed (void *uiPtr, Widget *widgetPtr);
	static void removeStreamActionClosed (void *uiPtr, Widget *widgetPtr);
	static void createMediaStreamComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);
	static void removeMediaStreamComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	StdString getSpritePath ();

	// Load subclass-specific resources and return a result value
	int doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Add subclass-specific items to the provided main toolbar object
	void doResetMainToolbar (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	void doResetSecondaryToolbar (Toolbar *toolbar);

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

	// Execute subclass-specific operations to sync state with records present in the provided RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore (RecordStore *store);

private:
	// Execute actions appropriate when the view button is clicked
	void handleViewButtonClick (Widget *buttonWidget);

	// Send a CreateMediaStream command to a remote agent, as specified by the currently held actionWindow widget
	void invokeCreateMediaStream ();

	// Send a RemoveMediaStream command to a remote agent, as specified by the currently held actionWindow widget
	void invokeRemoveMediaStream ();

	static const float smallImageScale;
	static const float mediumImageScale;
	static const float largeImageScale;

	StdString mediaId;
	StdString mediaName;
	StdString mediaUrl;
	StdString thumbnailUrl;
	int thumbnailCount;
	int mediaWidth, mediaHeight;
	float mediaDuration;
	float mediaFrameRate;
	int64_t mediaSize;
	int64_t mediaBitrate;
	StdString streamId;
	StdString streamAgentId;
	StdString streamAgentName;

	CardView *cardView;
	int cardLayout;
	float cardMaxImageWidth;
	IconCardWindow *detailCard;
	WidgetHandle timelineBar;
	WidgetHandle viewMenu;
	WidgetHandle actionWindow;

	// A map of agent names to ID values
	HashMap streamServerAgentMap;
};

#endif
