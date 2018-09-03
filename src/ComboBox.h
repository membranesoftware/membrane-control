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
// Widget that holds a text value selected from a list of options

#ifndef COMBO_BOX_H
#define COMBO_BOX_H

#include <list>
#include "StdString.h"
#include "StringList.h"
#include "HashMap.h"
#include "LabelWindow.h"
#include "Panel.h"

class ComboBox : public Panel {
public:
	ComboBox ();
	~ComboBox ();

	// Read-only data members
	StdString selectedItemValue;
	StdString selectedItemData;

	// Add an item to the combo box
	void addItem (const StdString &itemValue, const StdString &itemData = StdString (""));

	// Add a set of items to the combo box. If a HashMap is provided, it is treated as mapping itemName to itemData strings.
	void addItems (StringList *nameList);
	void addItems (HashMap *itemMap);

	// Set the combo box's value to the item matching the specified string
	void setValue (const StdString &value);

	// Return the combo box's current value, or the corresponding data string if non-empty
	StdString getValue ();

	// Expand the combo box, causing its parent UI to populate an item panel
	void expand ();

	// Clear a previously enabled expand state, optionally setting the combo box's current value in the process
	void unexpand (const StdString &value = StdString (""));

	// Set the isDestroyed state for all child widgets, causing them to be removed during the next update cycle
	void destroyAllChildWidgets ();

	// Callback functions
	static void expandItemClicked (void *expandPanelPtr, Widget *labelWindowPtr);

protected:
	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	virtual void doUpdate (int msElapsed, float originX, float originY);

	// Execute operations appropriate when the widget receives new mouse state
	virtual void doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void resetLayout ();

private:
	struct Item {
		StdString value;
		StdString itemData;
		LabelWindow *label;
		Item (): label (NULL) { }
	};

	// Set the combo box highlight state
	void setFocused (bool enable);

	// Find the active expand panel and return its pointer, or NULL if the panel was not found
	Panel *findExpandPanel ();

	std::list<ComboBox::Item> itemList;
	bool isExpanded;
	uint64_t expandPanelId;
	float expandDrawX, expandDrawY;
	bool isFocused;
	LabelWindow *selectedItemLabel;
	float maxTextWidth;
};

#endif
