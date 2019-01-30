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
// Base class for objects that manage application interface elements and associated resources

#ifndef UI_H
#define UI_H

#include "SDL2/SDL.h"
#include "StdString.h"
#include "StringList.h"
#include "Widget.h"
#include "WidgetHandle.h"
#include "Panel.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "RecordStore.h"
#include "Toolbar.h"

class Ui {
public:
	Ui ();
	virtual ~Ui ();

	// Read-only data members
	Panel *rootPanel;
	bool isLoaded;
	bool isFirstResumeComplete;
	bool isLinkConnected;

	// Increase the object's refcount
	void retain ();

	// Decrease the object's refcount. If this reduces the refcount to zero or less, delete the object.
	void release ();

	// Return text that should be used to identify the UI in a set of breadcrumb actions, or an empty string if no such title exists
	virtual StdString getBreadcrumbTitle ();

	// Return a newly created Sprite object that should be used to identify the UI in a set of breadcrumb actions, or NULL if no such Sprite exists. If a Sprite is returned by this method, the caller becomes responsible for destroying it when no longer needed.
	virtual Sprite *getBreadcrumbSprite ();

	// Execute operations appropriate when an agent control link client becomes connected
	virtual void handleLinkClientConnect (const StdString &agentId);

	// Execute operations appropriate when an agent control link client becomes disconnected
	virtual void handleLinkClientDisconnect (const StdString &agentId, const StdString &errorDescription);

	// Execute actions appropriate for a command received from an agent control link client
	virtual void handleLinkClientCommand (const StdString &agentId, int commandId, Json *command);

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	virtual void setHelpWindowContent (Widget *helpWindowPtr);

	// Load resources as needed to prepare the UI and return a result value
	int load ();

	// Free resources allocated by any previous load operation
	void unload ();

	// Change the provided main toolbar object to contain items appropriate for the UI
	void resetMainToolbar (Toolbar *toolbar);

	// Change the provided secondary toolbar object to contain items appropriate for the UI
	void resetSecondaryToolbar (Toolbar *toolbar);

	// Remove and destroy any popup widgets that have been created by the UI
	void clearPopupWidgets ();

	// Show the application shutdown window and disable further user interaction with UI widgets
	void showShutdownWindow ();

	// Reset interface elements as appropriate when the UI becomes active
	void resume ();

	// Reset interface elements as appropriate when the UI becomes inactive
	void pause ();

	// Update interface state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Add draw commands for execution by the application
	void draw ();

	// Refresh the interface's layout as appropriate for the current set of UiConfiguration values
	void refresh ();

	// Reload interface resources as needed to account for a new application window size
	void resize ();

	// Execute operations to sync state with records present in the provided RecordStore object, which has been locked prior to invocation
	void syncRecordStore (RecordStore *store);

	// Add a widget to the UI. Returns the widget pointer.
	Widget *addWidget (Widget *widget, float positionX = 0.0f, float positionY = 0.0f, int zLevel = 0);

	// Return a boolean value indicating if one of the UI's side windows is open
	bool isSideWindowOpen ();

	// Callback functions
	static void backButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void mainMenuButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void newsButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void settingsActionClicked (void *uiPtr, Widget *widgetPtr);
	static void aboutActionClicked (void *uiPtr, Widget *widgetPtr);
	static void updateActionClicked (void *uiPtr, Widget *widgetPtr);
	static void feedbackActionClicked (void *uiPtr, Widget *widgetPtr);
	static void helpActionClicked (void *uiPtr, Widget *widgetPtr);
	static void exitActionClicked (void *uiPtr, Widget *widgetPtr);
	static bool matchOpenTaskItem (void *ptr, Json *record);
	static bool keyEvent (void *uiPtr, SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	virtual StdString getSpritePath ();

	// Load subclass-specific resources and return a result value
	virtual int doLoad ();

	// Unload subclass-specific resources
	virtual void doUnload ();

	// Add subclass-specific items to the provided main toolbar object
	virtual void doResetMainToolbar (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	virtual void doResetSecondaryToolbar (Toolbar *toolbar);

	// Remove and destroy any subclass-specific popup widgets that have been created by the UI
	virtual void doClearPopupWidgets ();

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	virtual void doResume ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	virtual void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	virtual void doUpdate (int msElapsed);

	// Add subclass-specific draw commands for execution by the application
	virtual void doDraw ();

	// Execute subclass-specific operations to refresh the interface's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

	// Reload subclass-specific interface resources as needed to account for a new application window size
	virtual void doResize ();

	// Execute subclass-specific operations to sync state with records present in the provided RecordStore object, which has been locked prior to invocation
	virtual void doSyncRecordStore (RecordStore *store);

	// Execute operations appropriate when mouseHoverWidget has held its current value beyond the hover threshold
	void activateMouseHover ();

	// Deactivate any previously activated mouse hover widgets
	void deactivateMouseHover ();

	// Deactivate the mouse hover widget and prevent reactivation until a new mouse hover widget is acquired
	void suspendMouseHover ();

	// Add a back button to the provided toolbar and associate it with a click handler that pops the top UI from the application
	void addMainToolbarBackButton (Toolbar *toolbar);

	// Set the link client connection state that should be maintained for agents with ID values appearing in linkAgentIds
	void setLinkConnected (bool connected);

	// Add the specified agent ID to the set that should be maintained with a link client connection
	void addLinkAgent (const StdString &agentId);

	SpriteGroup sprites;
	WidgetHandle breadcrumbChip;
	WidgetHandle tooltip;
	WidgetHandle darkenPanel;
	WidgetHandle mainMenu;
	WidgetHandle settingsWindow;
	WidgetHandle helpWindow;
	WidgetHandle mouseHoverWidget;
	WidgetHandle actionWidget;
	WidgetHandle actionTarget;
	int mouseHoverClock;
	bool isMouseHoverActive;
	bool isMouseHoverSuspended;
	StringList linkAgentIds;

private:
	// Execute actions appropriate when the main menu button has been clicked
	void handleMainMenuButtonClick (Widget *buttonWidget);

	// Toggle the visible state of the settings window
	void toggleSettingsWindow ();

	// Toggle the visible state of the help window
	void toggleHelpWindow ();

	int refcount;
	SDL_mutex *refcountMutex;
};

#endif
