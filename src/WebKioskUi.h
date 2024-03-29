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
// UI for showing browser URLs on monitors

#ifndef WEB_KIOSK_UI_H
#define WEB_KIOSK_UI_H

#include "StdString.h"
#include "StringList.h"
#include "Json.h"
#include "HashMap.h"
#include "WidgetHandle.h"
#include "Toolbar.h"
#include "TextFieldWindow.h"
#include "WebPlaylistWindow.h"
#include "HelpWindow.h"
#include "Ui.h"

class WebKioskUi : public Ui {
public:
	// Sprite indexes
	enum {
		BreadcrumbIconSprite = 0,
		ClearDisplayButtonSprite = 1,
		BrowseUrlButtonSprite = 2,
		ShowUrlButtonSprite = 3,
		CreatePlaylistButtonSprite = 4,
		PlaylistIconSprite = 5,
		AddUrlButtonSprite = 6,
		WritePlaylistButtonSprite = 7,
		ShuffleIconSprite = 8,
		SpeedIconSprite = 9
	};

	// Card view row numbers
	enum {
		AgentToggleRow = 0,
		UnexpandedAgentRow = 1,
		ExpandedAgentRow = 2,
		PlaylistToggleRow = 3,
		UnexpandedPlaylistRow = 4,
		ExpandedPlaylistRow = 5
	};

	// Prefs keys
	static const char *SelectedAgentsKey;
	static const char *ExpandedAgentsKey;
	static const char *PlaylistsKey;

	WebKioskUi ();
	~WebKioskUi ();

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

	// Execute operations appropriate when an agent control link client becomes connected
	void handleLinkClientConnect (const StdString &agentId);

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

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();
	static void doSyncRecordStore_processAgentStatus (void *uiPtr, Json *record, const StdString &recordId);

private:
	// Callback functions
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void agentSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void agentExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void agentScreenshotLoaded (void *uiPtr, Widget *widgetPtr);
	static void playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void expandAgentsToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void expandPlaylistsToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void browseUrlButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void showUrlButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistNameClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistNameEdited (void *uiPtr, Widget *widgetPtr);
	static void playlistNameEditEnterButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistUrlListChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistRemoveActionClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistRemoveActionClosed (void *uiPtr, Widget *widgetPtr);
	static void playlistAddItemActionClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistAddItemMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void playlistAddItemMouseExited (void *uiPtr, Widget *widgetPtr);
	static void clearDisplayButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseExited (void *uiPtr, Widget *widgetPtr);
	static void invokeCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess);

	// Reset checked states for row expand toggles, as appropriate for item expand state
	void resetExpandToggles ();

	// Return the provided base value, after appending suffixes as needed to generate an unused playlist name
	StdString getAvailablePlaylistName (const StdString &baseName = StdString (""));

	// Return a newly created WebPlaylistWindow object that has been initialized for use with the UI
	WebPlaylistWindow *createWebPlaylistWindow ();

	// Return a string containing the set of selected agent names, appropriate for use in a command popup and truncated to fit within the specified maximum width, or an empty string if no agents are selected
	StdString getSelectedAgentNames (float maxWidth);

	int agentCount;
	HashMap selectedAgentMap;
	StdString selectedPlaylistId;
	TextFieldWindow *addressField;
	Button *browseUrlButton;
	Button *showUrlButton;
	Button *writePlaylistButton;
	Button *clearDisplayButton;
	WidgetHandle commandPopup;
	WidgetHandle commandPopupSource;
	WidgetHandle expandAgentsToggle;
	WidgetHandle expandPlaylistsToggle;
};

#endif
