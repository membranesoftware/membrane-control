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
// UI for contacting and configuring remote server applications

#ifndef SERVER_UI_H
#define SERVER_UI_H

#include "StdString.h"
#include "StringList.h"
#include "Json.h"
#include "WidgetHandle.h"
#include "Toolbar.h"
#include "AdminSecretWindow.h"
#include "HelpWindow.h"
#include "Ui.h"

class ServerUi : public Ui {
public:
	// Sprite indexes
	enum {
		BreadcrumbIconSprite = 0,
		AddressButtonSprite = 1,
		BroadcastButtonSprite = 2,
		AddressIconSprite = 3,
		ConfigureServerButtonSprite = 4
	};

	// Card view row numbers
	enum {
		AttachedServerToggleRow = 0,
		UnexpandedAttachedServerRow = 1,
		ExpandedAttachedServerRow = 2,
		UnattachedServerRow = 3,
		ServerPasswordsRow = 4
	};

	// Prefs keys
	static const char *UnexpandedAgentsKey;

	ServerUi ();
	~ServerUi ();

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

	// Execute an interface action to open the named widget and return a boolean value indicating if the widget was found
	bool openWidget (const StdString &targetName);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	StdString getSpritePath ();

	// Return a newly created widget for use as a main toolbar breadcrumb item
	Widget *createBreadcrumbWidget ();

	// Load subclass-specific resources and return a result value
	OsUtil::Result doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Remove and destroy any subclass-specific popup widgets that have been created by the UI
	void doClearPopupWidgets ();

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);
	static void doUpdate_findDeletedWindows (void *uiPtr, Widget *widgetPtr);

	// Reload subclass-specific interface resources as needed to account for a new application window size
	void doResize ();

	// Add subclass-specific items to the provided main toolbar object
	void doAddMainToolbarItems (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	void doAddSecondaryToolbarItems (Toolbar *toolbar);

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

private:
	// Callback functions
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void serverContactWindowStateChanged (void *uiPtr, Widget *widgetPtr);
	static void broadcastButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void addressToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void addressTextFieldEdited (void *uiPtr, Widget *widgetPtr);
	static void addressSnackbarHelpClicked (void *uiPtr, Widget *widgetPtr);
	static void serverExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void expandServersToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void serverStatusChanged (void *uiPtr, Widget *widgetPtr);
	static void serverAttachActionClicked (void *uiPtr, Widget *widgetPtr);
	static void serverCheckForUpdatesActionClicked (void *uiPtr, Widget *widgetPtr);
	static void serverAdminActionClicked (void *uiPtr, Widget *widgetPtr);
	static void serverDetachActionClicked (void *uiPtr, Widget *widgetPtr);
	static void serverRemoveActionClicked (void *uiPtr, Widget *widgetPtr);
	static void serverRemoveActionClosed (void *uiPtr, Widget *widgetPtr);
	static void adminSecretAddButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void adminSecretAddActionClosed (void *uiPtr, Widget *widgetPtr);
	static void adminSecretExpandStateChanged (void *uiPtr, Widget *widgetPtr);

	// Reset checked states for row expand toggles, as appropriate for item expand state
	void resetExpandToggles ();

	AdminSecretWindow *adminSecretWindow;
	WidgetHandle addressToggle;
	WidgetHandle addressTextFieldWindow;
	WidgetHandle emptyServerWindow;
	WidgetHandle expandServersToggle;
	StringList agentIdList;
	StringList cardIdList;
	int attachedServerCount;
	int unattachedServerCount;
};

#endif
