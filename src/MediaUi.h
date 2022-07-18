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
// UI for browsing and manipulating media items

#ifndef MEDIA_UI_H
#define MEDIA_UI_H

#include <map>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "HashMap.h"
#include "Json.h"
#include "Button.h"
#include "WidgetHandle.h"
#include "StreamPlaylistWindow.h"
#include "HelpWindow.h"
#include "Ui.h"

class MediaUi : public Ui {
public:
	// Sprite indexes
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
		SortButtonSprite = 11,
		StartPositionIconSprite = 12,
		DurationIconSprite = 13,
		ShuffleIconSprite = 14,
		PauseButtonSprite = 15,
		AddTagButtonSprite = 16,
		SearchStatusIconSprite = 17
	};

	// Card view row numbers
	enum {
		AgentToggleRow = 0,
		UnexpandedAgentRow = 1,
		ExpandedAgentRow = 2,
		PlaylistToggleRow = 3,
		UnexpandedPlaylistRow = 4,
		ExpandedPlaylistRow = 5,
		EmptyMediaRow = 6,
		MediaRow = 7,
		MediaLoadingRow = 8
	};

	// Toolbar modes
	enum {
		MonitorMode = 0,
		StreamMode = 1,
		TagMode = 2,
		ModeCount = 3
	};

	// State values for findMediaStreamsMap
	enum {
		StreamsRequestedState = 0,
		StreamsReceivedState = 1
	};

	// Empty state types
	enum {
		EmptyAgentState = 0,
		EmptyMediaState = 1,
		EmptyMediaStreamState = 2
	};

	// Prefs keys
	static const char *ImageSizeKey;
	static const char *SortOrderKey;
	static const char *SelectedAgentsKey;
	static const char *ExpandedAgentsKey;
	static const char *PlaylistsKey;
	static const char *ToolbarModeKey;
	static const char *VideoQualityKey;
	static const char *ShowMediaWithoutStreamsKey;

	MediaUi ();
	~MediaUi ();

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

	// Execute operations appropriate when an agent control link client becomes connected
	void handleLinkClientConnect (const StdString &agentId);

	// Execute an interface action to open the named widget and return a boolean value indicating if the widget was found
	bool openWidget (const StdString &targetName);

	// Execute an interface action to select the named widget and return a boolean value indicating if the widget was found
	bool selectWidget (const StdString &targetName);

	// Execute an interface action to unselect the named widget and return a boolean value indicating if the widget was found
	bool unselectWidget (const StdString &targetName);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	StdString getSpritePath ();

	// Return a newly created widget for use as a main toolbar breadcrumb item
	Widget *createBreadcrumbWidget ();

	// Load subclass-specific resources and return a result value
	OsUtil::Result doLoad ();

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
	static void doSyncRecordStore_processMediaServerAgent (void *uiPtr, Json *record, const StdString &recordId);
	static void doSyncRecordStore_processMonitorAgent (void *uiPtr, Json *record, const StdString &recordId);
	static void doSyncRecordStore_processMediaItem (void *uiPtr, Json *record, const StdString &recordId);

private:
	// Callback functions
	static bool matchMediaItem (void *idPtr, Widget *widget);
	static void imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void expandAgentsToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void expandPlaylistsToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void cardExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void mediaWindowImageClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaWindowViewButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaWindowBrowserPlayButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaWindowSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void searchFieldEdited (void *uiPtr, Widget *widgetPtr);
	static void searchButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void sortButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void showMediaWithoutStreamsActionClicked (void *uiPtr, Widget *widgetPtr);
	static void sortByNameActionClicked (void *uiPtr, Widget *widgetPtr);
	static void sortByNewestActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaLibraryMenuClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaLibraryScanActionClicked (void *uiPtr, Widget *widgetPtr);
	static void monitorSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void monitorScreenshotLoaded (void *uiPtr, Widget *widgetPtr);
	static void monitorCacheButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void itemUiThumbnailClicked (void *uiPtr, Widget *widgetPtr);
	static void mediaItemUiMediaRemoved (void *uiPtr, Widget *widgetPtr);
	static void modeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void monitorModeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void streamModeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void tagModeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseExited (void *uiPtr, Widget *widgetPtr);
	static void playButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void pauseButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void stopButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void configureStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void configureStreamActionClosed (void *uiPtr, Widget *widgetPtr);
	static void configureStreamOptionChanged (void *uiPtr, Widget *widgetPtr);
	static void configureMediaStreamComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess);
	static void cacheStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void cacheStreamActionClosed (void *uiPtr, Widget *widgetPtr);
	static void removeStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void removeStreamActionClosed (void *uiPtr, Widget *widgetPtr);
	static void removeStreamComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess);
	static void selectAllButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void selectMediaWindow (void *uiPtr, Widget *widgetPtr);
	static void createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistItemsChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistRenameActionClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistNameEdited (void *uiPtr, Widget *widgetPtr);
	static void playlistNameEditEnterButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistRemoveActionClicked (void *uiPtr, Widget *widgetPtr);
	static void removePlaylistActionClosed (void *uiPtr, Widget *widgetPtr);
	static void playlistAddItemActionClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistAddItemMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void playlistAddItemMouseExited (void *uiPtr, Widget *widgetPtr);
	static void addTagButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void addTagActionClosed (void *uiPtr, Widget *widgetPtr);
	static void removeTagButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void removeTagActionClosed (void *uiPtr, Widget *widgetPtr);
	static void receiveFindMediaItemsResult (void *uiPtr, const StdString &agentId, Json *command);
	static void receiveMediaItem (void *uiPtr, const StdString &agentId, Json *command);
	static void receiveFindMediaStreamsResult (void *uiPtr, const StdString &agentId, Json *command);
	static void receiveStreamItem (void *uiPtr, const StdString &agentId, Json *command);
	static void invokeMonitorCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess);
	static void invokeMediaCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess);

	struct MediaServerInfo {
		int resultOffset;
		int setSize;
		int recordCount;
		MediaServerInfo ():
			resultOffset (0),
			setSize (0),
			recordCount (0) { }
	};

	// Return a mediaServerMap iterator positioned at the specified entry, creating it if it doesn't already exist. This method must be invoked only while holding a lock on mediaServerMapMutex.
	std::map<StdString, MediaUi::MediaServerInfo>::iterator getMediaServerInfo (const StdString &agentId);

	// Clear all media items and request search result sets from servers
	void loadSearchResults ();

	// Set the control mode for the secondary toolbar, optionally forcing the reset even if the requested mode matches the mode already active
	void setToolbarMode (int mode, bool forceReset = false);

	// Return a string containing the set of selected monitor agent names, appropriate for use in a command popup, or an empty string if no monitor agents are selected
	StdString getSelectedMonitorNames ();

	// Return a string containing the set of selected media item names, appropriate for use in a command popup, or an empty string if no media items are selected
	StdString getSelectedMediaNames (bool isStreamRequired = false, bool isCreateStreamRequired = false);

	// Set selected state for all media items
	void selectAllMedia ();

	// Clear selected state from all media items
	void unselectAllMedia ();

	// Return a newly created StreamPlaylistWindow widget, suitable for use as a card view item
	StreamPlaylistWindow *createStreamPlaylistWindow ();

	// Return the provided base value, after appending suffixes as needed to generate an unused playlist name
	StdString getAvailablePlaylistName (const StdString &baseName = StdString (""));

	// Reset checked states for row expand toggles, as appropriate for item expand state
	void resetExpandToggles ();

	// Reset state of search status widgets
	void resetSearchStatus ();

	// Set the time of the next record sync if it isn't already assigned
	void resetNextRecordSyncTime ();

	// Return the total stream data size for all selected media items
	int64_t getSelectedStreamSize ();

	// Return the number of selected media items
	int getSelectedMediaCount ();

	// Return the number of selected media items that have an available playback stream
	int getSelectedStreamCount ();

	// Return the number of selected media items that are available for stream creation
	int getSelectedCreateStreamCount ();

	// Return the estimated total storage size required for streams of all selected media items at the specified profile, in bytes
	int64_t getSelectedCreateStreamSize (int profile);

	static const int PageSize;
	static const float TextTruncateWidthScale;
	static const float SearchFieldWidthScale;
	static const float BottomPaddingHeightScale;

	int toolbarMode;
	Button *playButton;
	Button *writePlaylistButton;
	Button *pauseButton;
	Button *stopButton;
	Button *configureStreamButton;
	Button *cacheStreamButton;
	Button *removeStreamButton;
	Button *selectAllButton;
	Button *addTagButton;
	Button *removeTagButton;
	WidgetHandle searchField;
	WidgetHandle searchStatusIcon;
	WidgetHandle emptyStateWindow;
	WidgetHandle commandPopup;
	WidgetHandle commandPopupSource;
	WidgetHandle targetMediaWindow;
	WidgetHandle lastSelectedMediaWindow;
	WidgetHandle selectedPlaylistWindow;
	WidgetHandle expandAgentsToggle;
	WidgetHandle expandPlaylistsToggle;
	WidgetHandle configureStreamSizeIcon;
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
