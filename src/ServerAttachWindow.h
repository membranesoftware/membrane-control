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
// Panel that shows an unattached agent record and controls to move it to the attached state

#ifndef SERVER_ATTACH_WINDOW_H
#define SERVER_ATTACH_WINDOW_H

#include "StdString.h"
#include "Image.h"
#include "Json.h"
#include "Label.h"
#include "Button.h"
#include "StatsWindow.h"
#include "Panel.h"

class ServerAttachWindow : public Panel {
public:
	ServerAttachWindow (const StdString &agentId);
	virtual ~ServerAttachWindow ();

	// Read-only data members
	StdString agentId;
	StdString agentDisplayName;
	int serverType;

	// Set a callback that should be invoked when the window's attach button is pressed
	void setAttachClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the window's remove button is pressed
	void setRemoveClickCallback (Widget::EventCallback callback, void *callbackData);

	// Refresh displayed data values using stored agent control state
	void refreshAgentData ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static ServerAttachWindow *castWidget (Widget *widget);

	// Callback functions
	static void attachButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void removeButtonClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	Image *iconImage;
	Label *nameLabel;
	Label *descriptionLabel;
	StatsWindow *statsWindow;
	Button *attachButton;
	Button *removeButton;
	Widget::EventCallback attachClickCallback;
	void *attachClickCallbackData;
	Widget::EventCallback removeClickCallback;
	void *removeClickCallbackData;
};

#endif
