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
// Panel that shows a stored command and controls for management and execution

#ifndef COMMAND_WINDOW_H
#define COMMAND_WINDOW_H

#include <vector>
#include "StdString.h"
#include "SpriteGroup.h"
#include "StringList.h"
#include "Image.h"
#include "Json.h"
#include "Image.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "IconLabelWindow.h"
#include "CommandRecord.h"
#include "Panel.h"

class CommandWindow : public Panel {
public:
	CommandWindow (int64_t storeId, SpriteGroup *commandUiSpriteGroup);
	virtual ~CommandWindow ();

	// Read-write data members
	StdString itemId;

	// Read-only data members
	int64_t storeId;
	int commandId;
	CommandRecord commandRecord;
	StdString commandName;

	// Set a callback that should be invoked when the window's save button is pressed
	void setSaveClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the window's execute button is pressed
	void setExecuteClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the window's delete button is pressed
	void setDeleteClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the window's name is clicked
	void setNameClickCallback (Widget::EventCallback callback, void *callbackData);

	// Refresh displayed data values using stored agent control state
	void refreshCommandData ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static CommandWindow *castWidget (Widget *widget);

	// Callback functions
	static void nameLabelClicked (void *windowPtr, Widget *widgetPtr);
	static void saveButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void executeButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void deleteButtonClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	SpriteGroup *sprites;
	Image *iconImage;
	Label *nameLabel;
	IconLabelWindow *targetAgentsIcon;
	IconLabelWindow *parameterIcon;
	Button *saveButton;
	Button *executeButton;
	Button *deleteButton;
	Widget::EventCallback nameClickCallback;
	void *nameClickCallbackData;
	Widget::EventCallback saveClickCallback;
	void *saveClickCallbackData;
	Widget::EventCallback executeClickCallback;
	void *executeClickCallbackData;
	Widget::EventCallback deleteClickCallback;
	void *deleteClickCallbackData;
};

#endif
