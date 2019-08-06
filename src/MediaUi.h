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
// UI for browsing and manipulating media items

#ifndef MEDIA_UI_H
#define MEDIA_UI_H

#include <map>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "HashMap.h"
#include "Json.h"
#include "CardView.h"
#include "Button.h"
#include "WidgetHandle.h"
#include "StreamPlaylistWindow.h"
#include "HelpWindow.h"
#include "Ui.h"

class MediaUi : public Ui {
public:
	// Constants to use for sprite indexes
	enum {
		ConfigureStreamButtonSprite = 0,
		CreatePlaylistButtonSprite = 1,
		AddPlaylistItemButtonSprite = 2,
		BrowserPlayButtonSprite = 3,
		BreadcrumbIconSprite = 4,
		SearchButtonSprite = 5,
		CacheButtonSprite = 6,
		PlayButtonSprite = 7,
		StreamButtonSprite = 8,
		WritePlaylistButtonSprite = 9,
		StopButtonSprite = 10,
		SortButtonSprite = 11
	};

	// Constants to use for card view row numbers
	enum {
		AgentRow = 0,
		PlaylistRow = 1,
		EmptyMediaRow = 2,
		MediaRow = 3,
		MediaLoadingRow = 4
	};

	// Constants to use for toolbar modes
	enum {
		MonitorMode = 0,
		StreamMode = 1,
		PlaylistMode = 2,
		ModeCount = 3
	};

	// Constants to use as state values in findMediaStreamsMap
	enum {
		StreamsRequestedState = 0,
		StreamsReceivedState = 1
	};

	// Constants to use for empty state types
	enum {
		EmptyAgentState = 0,
		EmptyMediaState = 1,
		EmptyMediaStreamState = 2
	};

	MediaUi ();
	~MediaUi ();

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

	// Execute operations appropriate when an agent control link client becomes connected
	void handleLinkClientConnect (const StdString &agentId);

	// Execute actions appropriate for a command received from an agent control link client
	void handleLinkClientCommand (const StdString &agentId, int commandId, Json *command);

	// Clear the provided string if the widget is of the correct type and matches its content by name
	static void matchPlaylistName (void *stringPtr, Widget *widgetPtr);

	// Callback functions
	static void processMediaServerAgent (void *uiPtr, Json *record, const StdString &recordId);
	static void processMonitorAgent (void *uiPtr, Json *record, const StdString &recordId);
	static void processMediaItem (void *uiPtr, Json *record, const StdString &recordId);
	static bool matchMediaItem (void *idPtr, Widget *widget);
	static void appendSelectedAgentId (void *stringListPtr, Widget *widgetPtr);
	static void appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr);
	static void imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadAgent (void *uiPtr, Widget *widgetPtr);
	static void cardExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void mediaWindowImageClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaWindowViewButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaWindowSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void searchFieldEdited (void *uiPtr, Widget *widgetPtr);
	static void searchButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void appendMediaIdWithoutStream (void *stringListPtr, Widget *widgetPtr);
	static void sortButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void showMediaWithoutStreamsActionClicked (void *uiPtr, Widget *widgetPtr);
	static void sortByNameActionClicked (void *uiPtr, Widget *widgetPtr);
	static void sortByNewestActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaLibraryMenuClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaLibraryScanActionClicked (void *uiPtr, Widget *widgetPtr);
	static void monitorSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void monitorCacheButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void itemUiThumbnailClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaItemUiMediaRemoved (void *uiPtr, Widget *widgetPtr);
	static void modeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void monitorModeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void streamModeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistModeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseExited (void *uiPtr, Widget *widgetPtr);
	static void playButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void stopButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void browserPlayButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void configureStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void configureStreamActionClosed (void *uiPtr, Widget *widgetPtr);
	static void configureMediaStreamComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand);
	static void cacheStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void deleteStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void addPlaylistItemButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void deletePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistItemsChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistRenameActionClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistNameEdited (void *uiPtr, Widget *widgetPtr);
	static void appendPlaylistJson (void *stringListPtr, Widget *widgetPtr);

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

	// Execute subclass-specific actions appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

private:
	struct MediaServerInfo {
		int resultOffset;
		int setSize;
		int recordCount;
		MediaServerInfo (): resultOffset (0), setSize (0), recordCount (0) { }
	};

	// Return a mediaServerMap iterator positioned at the specified entry, creating it if it doesn't already exist. This method must be invoked only while holding a lock on mediaServerMapMutex.
	std::map<StdString, MediaUi::MediaServerInfo>::iterator getMediaServerInfo (const StdString &agentId);

	// Clear all media items and request search result sets from servers
	void loadSearchResults ();

	// Set the control mode for the secondary toolbar, optionally forcing the reset even if the requested mode matches the mode already active
	void setToolbarMode (int mode, bool forceReset = false);

	// Return a string containing the set of selected monitor agent names, appropriate for use in a command popup and truncated to fit within the specified maximum width, or an empty string if no monitor agents are selected
	StdString getSelectedMonitorNames (float maxWidth);

	// Return a string containing the set of selected media item names, appropriate for use in a command popup and truncated to fit within the specified maximum width, or an empty string if no media items are selected
	StdString getSelectedMediaNames (float maxWidth, bool isStreamRequired = false);

	// Return a newly created StreamPlaylistWindow widget, suitable for use as a card view item
	StreamPlaylistWindow *createStreamPlaylistWindow ();

	// Return a generated string that does not match any existing playlist name
	StdString getAvailablePlaylistName ();

	static const int pageSize;

	int toolbarMode;
	CardView *cardView;
	Button *playButton;
	Button *writePlaylistButton;
	Button *stopButton;
	Button *browserPlayButton;
	Button *configureStreamButton;
	Button *cacheStreamButton;
	Button *deleteStreamButton;
	Button *addPlaylistItemButton;
	Button *createPlaylistButton;
	Button *deletePlaylistButton;
	WidgetHandle searchField;
	WidgetHandle emptyStateWindow;
	WidgetHandle commandPopup;
	WidgetHandle commandPopupSource;
	WidgetHandle lastSelectedMediaWindow;
	WidgetHandle selectedPlaylistWindow;
	int cardDetail;
	int emptyStateType;
	bool isShowingMediaWithoutStreams;
	int mediaSortOrder;
	int mediaServerCount;
	int mediaItemCount;
	int mediaStreamCount;
	bool findMediaComplete;
	bool isLoadingMedia;
	StdString searchKey;
	int recordReceiveCount;
	int64_t nextRecordSyncTime;
	std::map<StdString, MediaUi::MediaServerInfo> mediaServerMap;
	SDL_mutex *mediaServerMapMutex;
	HashMap selectedMonitorMap;
	HashMap selectedMediaMap;
	HashMap findMediaStreamsMap;
	SDL_mutex *findMediaStreamsMapMutex;
};

#endif
