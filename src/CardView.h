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
// ScrollView that arranges widgets as rows of cards

#ifndef CARD_VIEW_H
#define CARD_VIEW_H

#include <list>
#include <map>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "HashMap.h"
#include "Label.h"
#include "Panel.h"
#include "ScrollBar.h"
#include "ScrollView.h"

class CardView : public ScrollView {
public:
	// Row item detail types
	enum {
		LowDetail = 0,
		MediumDetail = 1,
		HighDetail = 2
	};

	CardView (float viewWidth, float viewHeight);
	~CardView ();

	typedef bool (*MatchFunction) (void *data, Widget *widget);
	static const float SmallItemScale;
	static const int AnimateScaleDuration;

	// Read-only data members
	float cardAreaWidth;

	// Set the size of the viewable area
	virtual void setViewSize (float viewWidth, float viewHeight);

	// Set the size of margin space that should be inserted between items in the view
	void setItemMarginSize (float marginSize);

	// Set a panel that should be shown as a header widget, immediately preceding items in the specified row number
	void setRowHeader (int row, Panel *headerPanel);

	// Set the margin size that should be used for items in the specified row, overriding any default item margin size that might have been set
	void setRowItemMarginSize (int row, float marginSize);

	// Set the reverse sort option for the specified row. If enabled, the view sorts items from that row in descending order.
	void setRowReverseSorted (int row, bool enable);

	// Set the animated selection option for the specified row. If enabled, the view uses scaling effects to animate selection of items in the row.
	void setRowSelectionAnimated (int row, bool enable);

	// Return a boolean value indicating whether the specified row has enabled the animated selection option
	bool isRowSelectionAnimated (int row);

	// Reset selection animation states for all items in the specified row
	void resetRowSelection (int row);

	// Set the detail option for the specified row, indicating a level of detail that should apply to contained items
	void setRowDetail (int row, int detailType);

	// Return a boolean value indicating if the card view contains no items
	bool empty ();

	// Return a boolean value indicating if the specified item exists in the view
	bool contains (const StdString &itemId);
	bool contains (const char *itemId);

	// Return a string value, suitable for use as a new item ID in the view
	StdString getAvailableItemId ();

	// Add an item to the view. Returns the Widget pointer that was added. If an empty itemId value is provided, the CardView generates one of its own. If the provided row value is zero or greater, assign it for use when positioning items in the view. After adding the item, invoke refreshLayout unless shouldSkipRefreshLayout is true.
	Widget *addItem (Panel *itemPanel, const StdString &itemId = StdString (""), int row = 0, bool shouldSkipRefreshLayout = false);
	Widget *addItem (Panel *itemPanel, const char *itemId, int row = 0, bool shouldSkipRefreshLayout = false);

	// Return a pointer to the item widget with the specified ID, or NULL if the item wasn't found
	Widget *getItem (const StdString &itemId);
	Widget *getItem (const char *itemId);

	// Return the number of items in the view
	int getItemCount ();

	// Return the number of items in the specified row
	int getRowItemCount (int row);

	// Return a pointer to the first item widget reported matching by the provided function, or NULL if the item wasn't found
	Widget *findItem (CardView::MatchFunction fn, void *fnData);

	// Remove the specified item from the view and destroy its underlying widget. After removing the item, invoke refreshLayout unless shouldSkipRefreshLayout is true.
	void removeItem (const StdString &itemId, bool shouldSkipRefreshLayout = false);
	void removeItem (const char *itemId, bool shouldSkipRefreshLayout = false);

	// Remove all items in the specified row from the view and destroy their underlying widgets
	void removeRowItems (int row);

	// Move an item in the view to the specified row, then invoke refreshLayout unless shouldSkipRefreshLayout is true.
	void setItemRow (const StdString &itemId, int targetRow, bool shouldSkipRefreshLayout = false);

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
	// Execute subclass-specific operations to update object state as appropriate for an elapsed millisecond time period
	virtual void doUpdate (int msElapsed);

	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	struct Item {
		StdString id;
		Panel *panel;
		int row;
		bool isHighlighted;
		bool isAnimationStarted;
		Item (): panel (NULL), row (-1), isHighlighted (false), isAnimationStarted (false) { }
	};

	struct Row {
		Panel *headerPanel;
		float itemMarginSize;
		bool isReverseSorted;
		bool isSelectionAnimated;
		int itemCount;
		int layout;
		float maxItemWidth;
		Row (): headerPanel (NULL), itemMarginSize (-1.0f), isReverseSorted (false), isSelectionAnimated (false), itemCount (0), layout (-1), maxItemWidth (0.0f) { }
	};

	// Sort the item list and populate secondary data structures. This method must be invoked only while holding a lock on itemMutex.
	void doSort ();

	// Reset the contents of itemIdMap to match item state. This method must be invoked only while holding a lock on itemMutex.
	void resetItemIdMap ();

	// Return an iterator positioned at the specified item in the item list, or the end of the item list if the item wasn't found. This method must be invoked only while holding a lock on itemMutex.
	std::list<CardView::Item>::iterator findItemPosition (const StdString &itemId);

	// Return an iterator positioned at the specified item in the row map, creating the item if it doesn't already exist
	std::map<int, CardView::Row>::iterator getRow (int rowNumber);

	static bool compareItemsAscending (const CardView::Item &a, const CardView::Item &b);
	static bool compareItemsDescending (const CardView::Item &a, const CardView::Item &b);

	float itemMarginSize;
	SDL_mutex *itemMutex;
	std::list<CardView::Item> itemList;

	// A map of item ID strings to numbers indicating the item's position in itemList
	HashMap itemIdMap;

	StdString highlightedItemId;
	std::map<int, CardView::Row> rowMap;
	bool isSorted;
	ScrollBar *scrollBar;
};

#endif
