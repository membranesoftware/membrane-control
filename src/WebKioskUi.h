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
// UI for control of the Membrane Web Kiosk application

#ifndef WEB_KIOSK_UI_H
#define WEB_KIOSK_UI_H

#include "StdString.h"
#include "Json.h"
#include "WidgetHandle.h"
#include "CardView.h"
#include "TextFieldWindow.h"
#include "WebPlaylistWindow.h"
#include "Toolbar.h"
#include "HashMap.h"
#include "Ui.h"

class WebKioskUi : public Ui {
public:
	// Constants to use for sprite indexes
	enum {
		BREADCRUMB_ICON = 0,
		DISPLAY_ICON = 1,
		BROWSE_URL_BUTTON = 2,
		SHOW_URL_BUTTON = 3,
		ADD_INTENT_BUTTON = 4,
		INTENT_ICON = 5,
		ADD_URL_BUTTON = 6,
		WRITE_INTENT_BUTTON = 7,
		CLEAR_DISPLAY_BUTTON = 8,
		SMALL_DISPLAY_ICON = 9,
		SMALL_INTENT_ICON = 10
	};

	WebKioskUi ();
	~WebKioskUi ();

	static const StdString serverApplicationName;

	// Return text that should be used to identify the UI in a set of breadcrumb actions, or an empty string if no such title exists
	StdString getBreadcrumbTitle ();

	// Return a newly created Sprite object that should be used to identify the UI in a set of breadcrumb actions, or NULL if no such Sprite exists. If a Sprite is returned by this method, the caller becomes responsible for destroying it when no longer needed.
	Sprite *getBreadcrumbSprite ();

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (Widget *helpWindowPtr);

	// Callback functions
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadAgent (void *uiPtr, Widget *widgetPtr);
	static void processAgentStatus (void *uiPtr, Json *record, const StdString &recordId);
	static void appendPlaylistJson (void *stringListPtr, Widget *widgetPtr);
	static void appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr);
	static void matchPlaylistName (void *stringPtr, Widget *widgetPtr);
	static void agentSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void agentExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void addUrlButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void browseUrlButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void showUrlButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void addPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static bool matchWebKioskAgentStatus (void *ptr, Json *record);
	static void addPlaylistUrl (void *urlStringPtr, Widget *widgetPtr);
	static void writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void writePlaylistComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);
	static void playlistNameClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistNameEdited (void *uiPtr, Widget *widgetPtr);
	static void playlistUrlListChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistMenuClicked (void *uiPtr, Widget *widgetPtr);
	static void renamePlaylistActionClicked (void *uiPtr, Widget *widgetPtr);
	static void removePlaylistActionClicked (void *uiPtr, Widget *widgetPtr);
	static void clearDisplayButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseExited (void *uiPtr, Widget *widgetPtr);

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

	// Execute subclass-specific operations to sync state with records present in the provided RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore (RecordStore *store);

private:
	// Return the provided base value, after appending suffixes as needed to generate an unused playlist name
	StdString getAvailablePlaylistName ();

	// Return a newly created WebPlaylistWindow object that has been initialized for use with the UI
	WebPlaylistWindow *createWebPlaylistWindow ();

	// Return a string containing the set of selected agent names, appropriate for use in a command popup and truncated to fit within the specified maximum width, or an empty string if no agents are selected
	StdString getSelectedAgentNames (float maxWidth);

	int agentCount;
	HashMap selectedAgentMap;
	StdString selectedPlaylistId;
	CardView *cardView;
	TextFieldWindow *addressField;
	Button *addUrlButton;
	Button *browseUrlButton;
	Button *showUrlButton;
	Button *addPlaylistButton;
	Button *writePlaylistButton;
	Button *clearDisplayButton;
	WidgetHandle actionWidget;
	WidgetHandle actionTarget;
	WidgetHandle commandPopup;
	WidgetHandle commandButton;
};

#endif
