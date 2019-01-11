/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
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
// Panel that holds a set of item rows

#ifndef LIST_VIEW_H
#define LIST_VIEW_H

#include <vector>
#include "StdString.h"
#include "StringList.h"
#include "Button.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Panel.h"

class ListView : public Panel {
public:
	ListView (float viewWidth, int minItemHeight = 0, int itemFontType = UiConfiguration::BODY, const StdString &titleText = StdString (""), const StdString &emptyStateText = StdString (""));
	~ListView ();

	// Set the view's width
	void setViewWidth (float fixedWidth);

	// Set a callback that should be invoked when the list content changes
	void setListChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Remove all items from the view
	void clearItems ();

	// Set the view's item list
	void setItems (StringList *itemList);

	// Clear the provided StringList object and insert items from the view's list
	void getItems (StringList *destList);

	// Return the number of items in the view's list
	int getItemCount ();

	// Return the itemData pointer associated with the specified item index, or NULL if no such data was found
	void *getItemData (int itemIndex);

	// Add an item to the view's list
	void addItem (const StdString &itemText, void *itemData = NULL, Widget::FreeFunction itemFree = NULL);

	// Callback functions
	static void deleteButtonClicked (void *listViewPtr, Widget *widgetPtr);

protected:
	// Execute operations appropriate when the widget receives new mouse state
	virtual void doProcessMouseState (const Widget::MouseState &mouseState);

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	struct Item {
		StdString text;
		Panel *panel;
		Label *label;
		void *data;
		Widget::FreeFunction dataFree;
		Item (): panel (NULL), label (NULL), data (NULL), dataFree (NULL) { }
	};

	float viewWidth;
	int minItemHeight;
	int itemFontType;
	std::vector<ListView::Item> itemList;
	LabelWindow *titleLabel;
	Label *emptyStateLabel;
	Button *deleteButton;
	bool isItemFocused;
	Panel *lastFocusPanel;
	Widget::EventCallback listChangeCallback;
	void *listChangeCallbackData;
};

#endif
