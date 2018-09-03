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
// Panel that shows controls for executing an action with a set of options

#ifndef ACTION_WINDOW_H
#define ACTION_WINDOW_H

#include <list>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "StringList.h"
#include "HashMap.h"
#include "Label.h"
#include "Button.h"
#include "WidgetHandle.h"
#include "Panel.h"

class ActionWindow : public Panel {
public:
	ActionWindow (const StdString &titleText = StdString (""), const StdString &confirmButtonText = StdString ("OK"));
	virtual ~ActionWindow ();

	// Read-write data members
	WidgetHandle sourceWidget;

	// Read-only data members
	bool isConfirmed;

	// Set a function that should be invoked when the action window is closed
	void setCloseCallback (Widget::EventCallback callback, void *callbackData);

	// Add a combo box option to the window using the provided item list. If a HashMap is provided, it is treated as mapping item name to item data strings.
	void addComboBoxOption (const StdString &optionName, StringList *optionItemList, const StdString &optionValue = StdString (""));
	void addComboBoxOption (const StdString &optionName, HashMap *optionItemMap, const StdString &optionValue = StdString (""));

	// Add a text field option to the window
	void addTextFieldOption (const StdString &optionName, const StdString &promptText = StdString (""), const StdString &optionValue = StdString (""));

	// Add a toggle option to the window
	void addToggleOption (const StdString &optionName, bool optionValue = false);

	// Return the value of the named option, or the specified default value if no such option was found
	StdString getOptionValue (const StdString &optionName, const StdString &defaultValue);
	StdString getOptionValue (const StdString &optionName, const char *defaultValue);
	bool getOptionValue (const StdString &optionName, bool defaultValue);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static ActionWindow *castWidget (Widget *widget);

	// Callback functions
	static void confirmButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void cancelButtonClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void resetLayout ();

private:
	// Constants to use for item types
	enum {
		COMBO_BOX = 1,
		TEXT_FIELD = 2,
		TOGGLE = 3
	};
	struct Item {
		StdString name;
		int type;
		Label *nameLabel;
		Widget *optionWidget;
		Item (): type (0), nameLabel (NULL), optionWidget (NULL) { }
	};

	// Return an iterator positioned at the specified item in itemList, or the end of itemList if the item wasn't found
	std::list<ActionWindow::Item>::iterator findItem (const StdString &optionName);

	Widget::EventCallback closeCallback;
	void *closeCallbackData;
	std::list<ActionWindow::Item> itemList;
	Label *titleLabel;
	Button *confirmButton;
	Button *cancelButton;
};

#endif
