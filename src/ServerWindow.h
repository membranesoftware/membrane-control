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
// Panel that shows an agent's state and a menu of actions appropriate for its available servers

#ifndef SERVER_WINDOW_H
#define SERVER_WINDOW_H

#include "StdString.h"
#include "Image.h"
#include "Json.h"
#include "Image.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "StatsWindow.h"
#include "IconLabelWindow.h"
#include "Panel.h"

class ServerWindow : public Panel {
public:
	ServerWindow (const StdString &agentId);
	virtual ~ServerWindow ();

	// Read-only data members
	bool isExpanded;
	StdString agentId;
	StdString agentDisplayName;
	StdString applicationId;
	int serverType;
	bool isRecordLoaded;
	bool isAgentDisabled;
	int agentTaskCount;

	// Set a callback that should be invoked when the expand toggle's checked state changes
	void setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the window's displayed server status changes
	void setStatusChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the window's check for updates button is pressed
	void setCheckForUpdatesClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the window's admin button is pressed
	void setAdminClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the window's detach button is pressed
	void setDetachClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the window's remove button is pressed
	void setRemoveClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static ServerWindow *castWidget (Widget *widget);

	// Callback functions
	static void expandToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void checkForUpdatesButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void adminButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void detachButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void removeButtonClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Reset the visibility of window controls as appropriate for server state
	void resetVisibility ();

	// Reset status icons to show the specified sprite and color
	void setStatusIcons (int spriteType, int textString, const Color &color);

	Image *iconImage;
	Label *nameLabel;
	Label *descriptionLabel;
	IconLabelWindow *statusIcon;
	Image *unexpandedStatusIcon;
	IconLabelWindow *storageIcon;
	IconLabelWindow *mediaCountIcon;
	IconLabelWindow *streamCountIcon;
	IconLabelWindow *taskCountIcon;
	StatsWindow *statsWindow;
	Toggle *expandToggle;
	Button *checkForUpdatesButton;
	Button *adminButton;
	Button *detachButton;
	Button *removeButton;
	Widget::EventCallback expandStateChangeCallback;
	void *expandStateChangeCallbackData;
	Widget::EventCallback statusChangeCallback;
	void *statusChangeCallbackData;
	Widget::EventCallback checkForUpdatesClickCallback;
	void *checkForUpdatesClickCallbackData;
	Widget::EventCallback adminClickCallback;
	void *adminClickCallbackData;
	Widget::EventCallback detachClickCallback;
	void *detachClickCallbackData;
	Widget::EventCallback removeClickCallback;
	void *removeClickCallbackData;
	int statusSpriteType;
	int statusTextString;
	Color statusColor;
};

#endif
