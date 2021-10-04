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
// Widget that shows a menu of action items

#ifndef MENU_H
#define MENU_H

#include <list>
#include <map>
#include "StdString.h"
#include "Sprite.h"
#include "Label.h"
#include "Image.h"
#include "Panel.h"

class Menu : public Panel {
public:
	Menu ();
	virtual ~Menu ();

	// Read-write data members
	bool isClickDestroyEnabled;

	// Add a menu item containing the provided name and an optional sprite icon. If a selection group number of zero or greater is specified, selecting the item causes the menu to show a check mark next to it while clearing any check marks from other items in the same group.
	void addItem (const StdString &name, Sprite *sprite = NULL, Widget::EventCallback callback = NULL, void *callbackData = NULL, int selectionGroup = -1, bool isSelected = false);

	// Add a divider line to the menu
	void addDivider ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static Menu *castWidget (Widget *widget);

protected:
	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	// Clear the isSelected state for each item in the specified group
	void unselectItemGroup (int selectionGroup);

	struct Item {
		bool isChoice;
		bool isDivider;
		bool isSelected;
		int selectionGroup;
		Panel *panel;
		Label *label;
		Image *image;
		Widget::EventCallback callback;
		void *callbackData;
		Item (): isChoice (false), isDivider (false), isSelected (false), selectionGroup (-1), panel (NULL), label (NULL), image (NULL), callback (NULL), callbackData (NULL) { }
	};
	std::list<Menu::Item> itemList;

	// A map of selection group numbers to checkmark images
	std::map<int, Image *> checkmarkImageMap;

	float selectionMarginSize;
	Panel *focusBackgroundPanel;
	float itemHeight;
	bool isItemFocused;
	Panel *lastFocusPanel;
};

#endif
