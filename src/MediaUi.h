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
// UI for browsing and manipulating media items

#ifndef MEDIA_UI_H
#define MEDIA_UI_H

#include "StdString.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "RecordStore.h"
#include "CardView.h"
#include "TextFieldWindow.h"
#include "Button.h"
#include "WidgetHandle.h"
#include "Ui.h"

class MediaUi : public Ui {
public:
	// Constants to use for sprite indexes
	enum {
		CREATE_STREAM_BUTTON = 0,
		LARGE_THUMBNAIL_BUTTON = 1,
		MEDIUM_THUMBNAIL_BUTTON = 2,
		SMALL_THUMBNAIL_BUTTON = 3,
		BREADCRUMB_ICON = 4,
		SEARCH_BUTTON = 5,
		THUMBNAIL_SIZE_BUTTON = 6
	};

	MediaUi ();
	~MediaUi ();

	// Return text that should be used to identify the UI in a set of breadcrumb actions, or an empty string if no such title exists
	StdString getBreadcrumbTitle ();

	// Return a newly created Sprite object that should be used to identify the UI in a set of breadcrumb actions, or NULL if no such Sprite exists. If a Sprite is returned by this method, the caller becomes responsible for destroying it when no longer needed.
	Sprite *getBreadcrumbSprite ();

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (Widget *helpWindowPtr);

	// Execute operations appropriate when an agent control link client becomes connected
	void handleLinkClientConnect (const StdString &agentId);

	// Execute actions appropriate for a command received from an agent control link client
	void handleLinkClientCommand (const StdString &agentId, int commandId, Json *command);

	// Callback functions
	static void processAgentStatus (void *uiPtr, Json *record, const StdString &recordId);
	static void processMediaItem (void *uiPtr, Json *record, const StdString &recordId);
	static void appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr);
	static void thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallThumbnailActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediumThumbnailActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeThumbnailActionClicked (void *uiPtr, Widget *widgetPtr);
	static void resetMediaCardLayout (void *uiPtr, Widget *widgetPtr);
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadAgent (void *uiPtr, Widget *widgetPtr);
	static void agentExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void mediaWindowClicked (void *uiPtr, Widget *widgetPtr);
	static void searchFieldEdited (void *uiPtr, Widget *widgetPtr);
	static void searchButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaLibraryMenuClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaLibraryScanActionClicked (void *uiPtr, Widget *widgetPtr);

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
	// Execute actions appropriate when the thumbnail size button is clicked
	void handleThumbnailSizeButtonClick (Widget *buttonWidget);

	// Clear all media items and request search result sets from servers
	void loadSearchResults ();

	static const int pageSize;

	CardView *cardView;
	TextFieldWindow *searchField;
	Button *searchButton;
	WidgetHandle thumbnailSizeMenu;
	WidgetHandle actionWidget;
	WidgetHandle actionTarget;
	WidgetHandle emptyStateWindow;
	int cardLayout;
	float cardMaxImageWidth;
	int agentCount;
	int mediaCount;
	bool findMediaComplete;
	StdString searchKey;
	int recordReceiveCount;
	int64_t nextRecordSyncTime;

	HashMap mediaServerResultOffsetMap;
	HashMap mediaServerSetSizeMap;
	HashMap mediaServerRecordCountMap;
};

#endif
