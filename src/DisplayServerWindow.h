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
// Panel that contains elements representing a display server item on a card view

#ifndef DISPLAY_SERVER_WINDOW_H
#define DISPLAY_SERVER_WINDOW_H

#include "StdString.h"
#include "Sprite.h"
#include "Json.h"
#include "Image.h"
#include "Label.h"
#include "TextArea.h"
#include "Button.h"
#include "ToggleWindow.h"
#include "StatsWindow.h"
#include "Panel.h"

class DisplayServerWindow : public Panel {
public:
	// Constants to use for layout types
	enum {
		LINK_UI_CARD = 0,
		DISPLAY_UI_CARD = 1,
		INTENT_UI_CARD = 2
	};

	DisplayServerWindow (Json *agentStatus, Sprite *iconSprite, int cardLayout = DisplayServerWindow::LINK_UI_CARD, const StdString &referenceIntentId = StdString (""));
	virtual ~DisplayServerWindow ();

	// Read-only data members
	StdString agentId;
	StdString agentName;
	StdString agentAddress;
	StdString agentVersion;
	StdString agentUptime;
	StdString controllerId;
	bool isSelected;

	// Set the card's layout type and reset widgets to show the specified content
	void setLayout (int cardLayout);

	// Execute operations appropriate to sync widget state with records present in the provided RecordStore object, which has been locked prior to invocation
	void syncRecordStore (RecordStore *store);

	// Set a callback that should be invoked when the select toggle's checked state changes
	void setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static DisplayServerWindow *castWidget (Widget *widget);

	// Callback functions
	static void selectToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void assignButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void setControllerComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);
	static void unassignButtonClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void resetLayout ();

private:
	StdString referenceIntentId;
	StdString controllerName;
	Image *iconImage;
	Label *nameLabel;
	TextArea *descriptionText;
	StatsWindow *statsWindow;
	ToggleWindow *selectToggle;
	Widget::EventCallback selectStateChangeCallback;
	void *selectStateChangeCallbackData;
	Button *assignButton;
	Button *unassignButton;
};

#endif
