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
// Panel that contains elements representing a web kiosk item on a card view

#ifndef WEB_KIOSK_WINDOW_H
#define WEB_KIOSK_WINDOW_H

#include "StdString.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Image.h"
#include "Label.h"
#include "TextArea.h"
#include "Button.h"
#include "Toggle.h"
#include "StatsWindow.h"
#include "Panel.h"

class WebKioskWindow : public Panel {
public:
	WebKioskWindow (const StdString &agentId, SpriteGroup *webKioskUiSpriteGroup);
	virtual ~WebKioskWindow ();

	// Read-only data members
	bool isSelected;
	StdString agentId;
	StdString agentName;

	// Set a callback that should be invoked when the select toggle's checked state changes
	void setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Execute operations appropriate to sync widget state with records present in the provided RecordStore object, which has been locked prior to invocation
	void syncRecordStore (RecordStore *store);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static WebKioskWindow *castWidget (Widget *widget);

	// Callback functions
	static void selectToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void setControllerComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void resetLayout ();

private:
	// Populate widgets as appropriate for the window's initial state
	void populate ();

	SpriteGroup *spriteGroup;
	Image *iconImage;
	Label *nameLabel;
	TextArea *descriptionText;
	StatsWindow *statsWindow;
	Toggle *selectToggle;
	Widget::EventCallback selectStateChangeCallback;
	void *selectStateChangeCallbackData;
};

#endif
