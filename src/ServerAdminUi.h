/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// UI that shows admin controls for a remote agent

#ifndef SERVER_ADMIN_UI_H
#define SERVER_ADMIN_UI_H

#include "StdString.h"
#include "Json.h"
#include "StringList.h"
#include "RecordStore.h"
#include "WidgetHandle.h"
#include "Toolbar.h"
#include "Button.h"
#include "AdminSecretWindow.h"
#include "HelpWindow.h"
#include "Ui.h"

class ServerAdminUi : public Ui {
public:
	// Sprite indexes
	enum {
		LockButtonSprite = 0
	};

	// Card view row numbers
	enum {
		ServerRow = 0,
		ServerPasswordsRow = 1,
		TaskRow = 2
	};

	ServerAdminUi (const StdString &agentId, const StdString &agentDisplayName);
	~ServerAdminUi ();

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
	int doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Add subclass-specific items to the provided secondary toolbar object
	void doAddSecondaryToolbarItems (Toolbar *toolbar);

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

private:
	// Callback functions
	static void cardExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void serverStatusChanged (void *uiPtr, Widget *widgetPtr);
	static void adminSecretAddButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void adminSecretAddActionClosed (void *uiPtr, Widget *widgetPtr);
	static void lockButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void setAdminPasswordActionClosed (void *uiPtr, Widget *widgetPtr);
	static void processTaskItem (void *uiPtr, Json *record, const StdString &recordId);
	static void setAdminSecretComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand);

	StdString agentId;
	StdString agentDisplayName;
	StringList watchIdList;
	Button *lockButton;
	AdminSecretWindow *adminSecretWindow;
	WidgetHandle emptyTaskWindow;
	int taskCount;
	StdString emptyPasswordName;
	StdString lastSecretName;
};

#endif
