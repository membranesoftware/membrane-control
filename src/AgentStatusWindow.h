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
// Panel that shows status of an agent and its active servers

#ifndef AGENT_STATUS_WINDOW_H
#define AGENT_STATUS_WINDOW_H

#include <list>
#include "StdString.h"
#include "SpriteGroup.h"
#include "Image.h"
#include "Json.h"
#include "HashMap.h"
#include "Label.h"
#include "Button.h"
#include "TextArea.h"
#include "IconLabelWindow.h"
#include "ToggleWindow.h"
#include "StatsWindow.h"
#include "ActionWindow.h"
#include "HyperlinkWindow.h"
#include "Panel.h"

class AgentStatusWindow : public Panel {
public:
	// An empty agentId value indicates that the window should display its empty state, appropriate for use when no agents are available
	AgentStatusWindow (const StdString &agentId, SpriteGroup *linkUiSpriteGroup);
	virtual ~AgentStatusWindow ();

	// Read-write data members
	StdString itemId;

	// Read-only data members
	StdString agentId;

	// Set a callback function that should be invoked when the window's connect button is pressed
	void setConnectCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback function that should be invoked when the window's disconnect button is pressed
	void setDisconnectCallback (Widget::EventCallback callback, void *callbackData);

	// Set the window's state as appropriate when its agent is connecting
	void setConnecting ();

	// Set the window's state as appropriate when its agent is connected
	void setConnected ();

	// Set the window's state as appropriate when its agent is disconnected
	void setDisconnected ();

	// Execute operations appropriate to sync widget state with records present in the provided RecordStore object, which has been locked prior to invocation
	void syncRecordStore (RecordStore *store);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static AgentStatusWindow *castWidget (Widget *widget);

	// Callback functions
	static void connectButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void disconnectButtonClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void resetLayout ();

private:
	struct ServerItem {
		StdString itemId;
		Image *iconImage;
		StatsWindow *statsWindow;
		ServerItem (): iconImage (NULL), statsWindow (NULL) { }
	};

	// Populate widgets as appropriate for the window's initial state
	void populate ();

	// Create and update elements to reflect any media server status contained in the provided AgentStatus record
	void syncMediaServerStatus (Json *record);

	// Create and update elements to reflect any stream server status contained in the provided AgentStatus record
	void syncStreamServerStatus (Json *record);

	// Create and update elements to reflect any display server status contained in the provided AgentStatus record
	void syncDisplayServerStatus (Json *record);

	// Create and update elements to reflect any master server status contained in the provided AgentStatus record
	void syncMasterServerStatus (Json *record);

	// Create and update elements to reflect any monitor server status contained in the provided AgentStatus record
	void syncMonitorServerStatus (Json *record);

	// Create and update elements to reflect any link server status contained in the provided AgentStatus record
	void syncLinkServerStatus (Json *record);

	// Return an iterator positioned at the server item with the specified ID, or at the end of the list if the item wasn't found
	std::list<AgentStatusWindow::ServerItem>::iterator findServerItem (const StdString &itemId);

	SpriteGroup *spriteGroup;
	Image *iconImage;
	Label *nameLabel;
	TextArea *descriptionText;
	std::list<AgentStatusWindow::ServerItem> serverList;
	HyperlinkWindow *helpLinkWindow;
	StatsWindow *statsWindow;
	Button *connectButton;
	Widget::EventCallback connectCallback;
	void *connectCallbackData;
	Button *disconnectButton;
	Widget::EventCallback disconnectCallback;
	void *disconnectCallbackData;
};

#endif
