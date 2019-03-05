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
// UI that shows controls for a set of Membrane Monitor agents

#ifndef MONITOR_UI_H
#define MONITOR_UI_H

#include "StdString.h"
#include "Json.h"
#include "WidgetHandle.h"
#include "Toolbar.h"
#include "Button.h"
#include "HashMap.h"
#include "CardView.h"
#include "HelpWindow.h"
#include "TextFieldWindow.h"
#include "StreamWindow.h"
#include "StreamPlaylistWindow.h"
#include "Ui.h"

class MonitorUi : public Ui {
public:
	MonitorUi ();
	~MonitorUi ();

	// Constants to use for sprite indexes
	enum {
		BREADCRUMB_ICON = 0,
		THUMBNAIL_SIZE_BUTTON = 1,
		LARGE_THUMBNAIL_BUTTON = 2,
		MEDIUM_THUMBNAIL_BUTTON = 3,
		SMALL_THUMBNAIL_BUTTON = 4,
		ADD_CACHE_STREAM_BUTTON = 5,
		PLAY_BUTTON = 6,
		STOP_BUTTON = 7,
		SMALL_MONITOR_ICON = 8,
		SMALL_STREAM_ICON = 9,
		WRITE_PLAYLIST_BUTTON = 10,
		CREATE_PLAYLIST_BUTTON = 11,
		ADD_PLAYLIST_ITEM_BUTTON = 12,
		SMALL_PROGRAM_ICON = 13,
		SEARCH_BUTTON = 14,
		CACHE_BUTTON = 15
	};

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

	// Execute operations appropriate when an agent control link client becomes connected
	void handleLinkClientConnect (const StdString &agentId);

	// Execute actions appropriate for a command received from an agent control link client
	void handleLinkClientCommand (const StdString &agentId, int commandId, Json *command);

	// Callback functions
	static void processMonitorAgentStatus (void *uiPtr, Json *record, const StdString &recordId);
	static void processStreamServerAgentStatus (void *uiPtr, Json *record, const StdString &recordId);
	static void processStreamItem (void *uiPtr, Json *record, const StdString &recordId);
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadAgent (void *uiPtr, Widget *widgetPtr);
	static void monitorAgentSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void agentExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallThumbnailActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediumThumbnailActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeThumbnailActionClicked (void *uiPtr, Widget *widgetPtr);
	static void resetStreamWindowLayout (void *uiPtr, Widget *widgetPtr);
	static void streamWindowImageClicked (void *uiPtr, Widget *widgetPtr);
	static void streamWindowViewButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void streamItemUiThumbnailClicked (void *uiPtr, Widget *widgetPtr);
	static void unselectStreamWindow (void *uiPtr, Widget *widgetPtr);
	static void searchFieldEdited (void *uiPtr, Widget *widgetPtr);
	static void searchButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseExited (void *uiPtr, Widget *widgetPtr);
	static void stopButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void addPlaylistItemButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void matchPlaylistName (void *stringPtr, Widget *widgetPtr);
	static void playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistItemsChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistMenuClicked (void *uiPtr, Widget *widgetPtr);
	static void renamePlaylistActionClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistNameEdited (void *uiPtr, Widget *widgetPtr);
	static void removePlaylistActionClicked (void *uiPtr, Widget *widgetPtr);
	static void appendPlaylistJson (void *stringListPtr, Widget *widgetPtr);
	static void appendSelectedAgentId (void *stringListPtr, Widget *widgetPtr);
	static void appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr);
	static void monitorCacheButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void monitorStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void monitorStreamButtonMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void monitorCommandButtonMouseExited (void *uiPtr, Widget *widgetPtr);

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

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Execute subclass-specific operations to refresh the interface's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Reload subclass-specific interface resources as needed to account for a new application window size
	void doResize ();

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

private:
	// Return a newly created StreamPlaylistWindow widget, suitable for use as a card view item
	StreamPlaylistWindow *createStreamPlaylistWindow ();

	// Set the interface's selected StreamWindow item at the specified timestamp, for use in executing remote commands
	void setSelectedStream (StreamWindow *streamWindow, float timestamp);

	// Return a string containing the set of selected agent names, appropriate for use in a command popup and truncated to fit within the specified maximum width, or an empty string if no agents are selected
	StdString getSelectedAgentNames (float maxWidth);

	// Return a generated string that does not match any existing playlist name
	StdString getAvailablePlaylistName ();

	// Clear all stream items and request search result sets from servers
	void loadSearchResults ();

	static const int pageSize;

	HashMap selectedAgentMap;
	CardView *cardView;
	TextFieldWindow *searchField;
	Button *searchButton;
	Button *stopButton;
	Button *playButton;
	Button *createPlaylistButton;
	Button *writePlaylistButton;
	Button *addPlaylistItemButton;
	StdString searchKey;
	WidgetHandle emptyStateWindow;
	WidgetHandle selectedStreamWindow;
	WidgetHandle selectedPlaylistWindow;
	WidgetHandle commandPopup;
	WidgetHandle commandPopupSource;
	int cardLayout;
	float cardMaxImageWidth;
	int recordReceiveCount;
	int64_t nextRecordSyncTime;
	StdString streamSearchKey;
	HashMap streamServerAgentMap;
	HashMap streamServerResultOffsetMap;
	HashMap streamServerSetSizeMap;
	HashMap streamServerRecordCountMap;
	int monitorCount;
	int streamServerCount;
	int streamCount;
	bool findStreamsComplete;
};

#endif
