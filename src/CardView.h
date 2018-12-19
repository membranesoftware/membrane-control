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
// ScrollView that arranges widgets as rows of cards

#ifndef CARD_VIEW_H
#define CARD_VIEW_H

#include <list>
#include <map>
#include "StdString.h"
#include "HashMap.h"
#include "Label.h"
#include "LabelWindow.h"
#include "ScrollBar.h"
#include "ScrollView.h"

class CardView : public ScrollView {
public:
	CardView (float viewWidth, float viewHeight);
	~CardView ();

	typedef bool (*MatchFunction) (void *data, Widget *widget);

	// Read-only data members
	float cardAreaWidth;

	// Set the view's size
	void setViewSize (float viewWidth, float viewHeight);

	// Set the size of margin space that should be inserted between items in the view
	void setItemMarginSize (float marginSize);

	// Set a text string that should be shown as a header label immediately preceding items in the specified row number
	void setRowHeader (int row, const StdString &headerText, int headerFontType = UiConfiguration::BODY, const Color &color = Color (0.0f, 0.0f, 0.0f));

	// Set the margin size that should be used for items in the specified row, overriding any default item margin size that might have been set
	void setRowItemMarginSize (int row, float marginSize);

	// Return a boolean value indicating if the card view contains no items
	bool empty ();

	// Return a boolean value indicating if the specified item exists in the view
	bool contains (const StdString &itemId);
	bool contains (const char *itemId);

	// Return a string value, suitable for use as a new item ID in the view
	StdString getAvailableItemId ();

	// Add an item to the view. Returns the Widget pointer that was added. If an empty itemId value is provided, the CardView generates one of its own. If the provided row value is zero or greater, assign it for use when positioning items in the view. After adding the item, invoke refreshLayout unless shouldSkipRefreshLayout is true.
	Widget *addItem (Widget *itemWidget, const StdString &itemId = StdString (""), int row = 0, bool shouldSkipRefreshLayout = false);
	Widget *addItem (Widget *itemWidget, const char *itemId, int row = 0, bool shouldSkipRefreshLayout = false);

	// Return a pointer to the item widget with the specified ID, or NULL if the item wasn't found
	Widget *getItem (const StdString &itemId);
	Widget *getItem (const char *itemId);

	// Return the number of items in the view's list
	int getItemCount ();

	// Return a pointer to the first item widget reported matching by the provided function, or NULL if the item wasn't found
	Widget *findItem (CardView::MatchFunction fn, void *fnData);

	// Remove the specified item from the view and destroy its underlying widget
	void removeItem (const StdString &itemId);
	void removeItem (const char *itemId);

	// Remove all items in the specified row from the view and destroy their underlying widgets
	void removeRowItems (int row);

	// Remove all items from the view and destroy their underlying widgets
	void removeAllItems ();

	// Process all items in the view by executing the provided function, optionally resetting widget positions afterward
	void processItems (Widget::EventCallback fn, void *fnData, bool shouldRefreshLayout = false);

	// Process all items in the specified row of the view by executing the provided function, optionally resetting widget positions afterward
	void processRowItems (int row, Widget::EventCallback fn, void *fnData, bool shouldRefreshLayout = false);

	// Change the view's vertical scroll position to display the specified item
	void scrollToItem (const StdString &itemId);

	// Callback functions
	static void scrollBarPositionChanged (void *windowPtr, Widget *widgetPtr);

protected:
	// Execute operations appropriate when the widget receives new mouse state
	virtual void doProcessMouseState (const Widget::MouseState &mouseState);

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	struct Item {
		StdString id;
		Widget *widget;
		int row;
		Item (): widget (NULL), row (-1) { }
	};

	struct Row {
		LabelWindow *headerLabel;
		float itemMarginSize;
		int itemCount;
		Row (): headerLabel (NULL), itemMarginSize (-1.0f), itemCount (0) { }
	};

	// Sort the item list and populate secondary data structures
	void doSort ();

	// Return an iterator positioned at the specified item in the item list, or the end of the item list if the item wasn't found
	std::list<CardView::Item>::iterator findItemPosition (const StdString &itemId);

	// Return an iterator positioned at the specified item in the row map, creating the item if it doesn't already exist
	std::map<int, CardView::Row>::iterator getRow (int rowNumber);

	static bool compareItems (const CardView::Item &a, const CardView::Item &b);

	float itemMarginSize;
	std::list<CardView::Item> itemList;
	std::map<int, CardView::Row> rowMap;

	// A map of item ID strings to numbers indicating the item's position in itemList
	HashMap itemIdMap;

	bool isSorted;
	ScrollBar *scrollBar;
};

#endif
