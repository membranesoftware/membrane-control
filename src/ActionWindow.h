/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "TextFlow.h"
#include "Button.h"
#include "ComboBox.h"
#include "TextField.h"
#include "TextFieldWindow.h"
#include "Toggle.h"
#include "SliderWindow.h"
#include "WidgetHandle.h"
#include "Panel.h"

class ActionWindow : public Panel {
public:
	ActionWindow ();
	virtual ~ActionWindow ();

	// Read-write data members
	Widget::EventCallbackContext optionChangeCallback;
	Widget::EventCallbackContext closeCallback;

	// Read-only data members
	bool isOptionDataValid;
	bool isConfirmed;
	bool isInverseColor;

	// Set the visible state for the window's cancel and confirm buttons (visible by default)
	void setButtonsVisible (bool visible);

	// Set the window's inverse color option
	void setInverseColor (bool inverse);

	// Set the window's title text (empty by default)
	void setTitleText (const StdString &text);

	// Set the window's description text (empty by default)
	void setDescriptionText (const StdString &text);

	// Set the tooltip text that should be shown on the window's confirm button (defaults to "Confirm")
	void setConfirmTooltipText (const StdString &text);

	// Add the provided panel to the window as a footer element
	void setFooterPanel (Panel *panel);

	// Add the provided widget to the window as an option item
	void addOption (const StdString &optionName, ComboBox *comboBox, const StdString &descriptionText = StdString (""));
	void addOption (const StdString &optionName, TextField *textField, const StdString &descriptionText = StdString (""));
	void addOption (const StdString &optionName, TextFieldWindow *textFieldWindow, const StdString &descriptionText = StdString (""));
	void addOption (const StdString &optionName, Toggle *toggle, const StdString &descriptionText = StdString (""));
	void addOption (const StdString &optionName, SliderWindow *slider, const StdString &descriptionText = StdString (""));

	// Set description text for the named option
	void setOptionDescriptionText (const StdString &optionName, const StdString &descriptionText);

	// Set the named option to evaluate as invalid if its value is an empty string
	void setOptionNotEmptyString (const StdString &optionName);

	// Set the disabled state for the named option
	void setOptionDisabled (const StdString &optionName, bool disable);

	// Set the value of the named option
	void setOptionValue (const StdString &optionName, const char *optionValue, bool shouldSkipChangeCallback = false);
	void setOptionValue (const StdString &optionName, const StdString &optionValue, bool shouldSkipChangeCallback = false);
	void setOptionValue (const StdString &optionName, int optionValue, bool shouldSkipChangeCallback = false);
	void setOptionValue (const StdString &optionName, float optionValue, bool shouldSkipChangeCallback = false);
	void setOptionValue (const StdString &optionName, bool optionValue, bool shouldSkipChangeCallback = false);

	// Return the string value of the named option, or the specified default value if no such option was found
	StdString getStringValue (const StdString &optionName, const StdString &defaultValue);
	StdString getStringValue (const StdString &optionName, const char *defaultValue);

	// Return the number value of the named option, or the specified default value if no such option was found
	int getNumberValue (const StdString &optionName, int defaultValue);
	float getNumberValue (const StdString &optionName, float defaultValue);

	// Return the boolean value of the named option, or the specified default value if no such option was found
	bool getBooleanValue (const StdString &optionName, bool defaultValue);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static ActionWindow *castWidget (Widget *widget);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void confirmButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void cancelButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void optionValueChanged (void *windowPtr, Widget *widgetPtr);

	// Item types
	enum {
		ComboBoxItem = 1,
		TextFieldItem = 2,
		TextFieldWindowItem = 3,
		ToggleItem = 4,
		SliderItem = 5
	};
	struct Item {
		StdString name;
		int type;
		Label *nameLabel;
		TextFlow *descriptionText;
		Widget *optionWidget;
		bool isNotEmptyString;
		bool isDisabled;
		Item (): type (0), nameLabel (NULL), descriptionText (NULL), optionWidget (NULL), isNotEmptyString (false), isDisabled (false) { }
	};

	// Return an iterator positioned at the specified item in itemList, or the end of itemList if the item wasn't found
	std::list<ActionWindow::Item>::iterator findItem (const StdString &optionName, bool createNewItem = false);

	// Utility method that executes item add operations as needed by addOption variants
	void doAddOption (int itemType, const StdString &optionName, Widget *optionWidget, const StdString &descriptionText);

	// Check the validity of all option values and reset isOptionDataValid
	void verifyOptions ();

	std::list<ActionWindow::Item> itemList;
	Label *titleLabel;
	TextFlow *headerDescriptionText;
	Panel *footerPanel;
	Button *confirmButton;
	StdString confirmButtonTooltipText;
	Button *cancelButton;
};

#endif
