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
#include "Config.h"
#include <stdlib.h>
#include <list>
#include <map>
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "Ui.h"
#include "Widget.h"
#include "HashMap.h"
#include "Panel.h"
#include "Label.h"
#include "UiConfiguration.h"
#include "ScrollBar.h"
#include "ScrollView.h"
#include "CardView.h"

CardView::CardView (float viewWidth, float viewHeight)
: ScrollView (viewWidth, viewHeight)
, cardAreaWidth (viewWidth)
, itemMarginSize (0.0f)
, isSorted (false)
, scrollBar (NULL)
{
	UiConfiguration *uiconfig;

	typeName.assign ("CardView");

	uiconfig = &(App::getInstance ()->uiConfig);
	itemMarginSize = uiconfig->marginSize;
	scrollBar = (ScrollBar *) addWidget (new ScrollBar (viewHeight - (uiconfig->paddingSize * 2.0f)));
	scrollBar->setPositionChangeCallback (CardView::scrollBarPositionChanged, this);
	scrollBar->zLevel = 2;
	scrollBar->isVisible = false;

	cardAreaWidth = width - scrollBar->width - uiconfig->paddingSize - (uiconfig->marginSize * 0.25f);
}

CardView::~CardView () {
	std::list<CardView::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->widget) {
			i->widget->isDestroyed = true;
			i->widget->release ();
		}
		++i;
	}
	itemList.clear ();
}

void CardView::setViewSize (float viewWidth, float viewHeight) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setFixedSize (true, viewWidth, viewHeight);
	scrollBar->setMaxTrackLength (viewHeight - (uiconfig->paddingSize * 2.0f));
	cardAreaWidth = width - scrollBar->width - uiconfig->paddingSize - (uiconfig->marginSize * 0.25f);
	refreshLayout ();
}

void CardView::setItemMarginSize (float marginSize) {
	if (FLOAT_EQUALS (marginSize, itemMarginSize)) {
		return;
	}
	itemMarginSize = marginSize;
	refreshLayout ();
}

void CardView::setRowHeader (int row, const StdString &headerText, int headerFontType, const Color &color) {
	UiConfiguration *uiconfig;
	std::map<int, CardView::Row>::iterator pos;
	LabelWindow *label;

	uiconfig = &(App::getInstance ()->uiConfig);
	label = (LabelWindow *) addWidget (new LabelWindow (new Label (headerText, headerFontType, color)));
	label->isVisible = false;
	label->zLevel = 1;
	label->setFillBg (true, 0.0f, 0.0f, 0.0f);
	label->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);

	pos = getRow (row);
	if (pos->second.headerLabel) {
		pos->second.headerLabel->isDestroyed = true;
	}
	pos->second.headerLabel = label;

	refreshLayout ();
}

void CardView::setRowItemMarginSize (int row, float marginSize) {
	std::map<int, CardView::Row>::iterator pos;

	pos = getRow (row);
	if (FLOAT_EQUALS (marginSize, pos->second.itemMarginSize)) {
		return;
	}
	pos->second.itemMarginSize = marginSize;
	refreshLayout ();
}

void CardView::doProcessMouseState (const Widget::MouseState &mouseState) {
	float y;

	y = viewOriginY;
	ScrollView::doProcessMouseState (mouseState);
	if (! FLOAT_EQUALS (y, viewOriginY)) {
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY + App::getInstance ()->uiConfig.paddingSize);
	}
}

bool CardView::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	float y;
	bool result;

	y = viewOriginY;
	result = ScrollView::doProcessKeyEvent (keycode, isShiftDown, isControlDown);
	if (! FLOAT_EQUALS (y, viewOriginY)) {
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY + App::getInstance ()->uiConfig.paddingSize);
	}

	return (result);
}

bool CardView::empty () {
	return (itemList.empty ());
}

bool CardView::contains (const StdString &itemId) {
	return (itemIdMap.exists (itemId));
}

bool CardView::contains (const char *itemId) {
	return (contains (StdString (itemId)));
}

StdString CardView::getAvailableItemId () {
	App *app;
	StdString id;

	app = App::getInstance ();
	while (true) {
		id.sprintf ("CardView_item_%lli", (long long int) app->getUniqueId ());
		if (! contains (id)) {
			break;
		}
	}

	return (id);
}

Widget *CardView::addItem (Widget *itemWidget, const char *itemId, int row, bool shouldSkipRefreshLayout) {
	return (addItem (itemWidget, StdString (itemId ? itemId : ""), row, shouldSkipRefreshLayout));
}

Widget *CardView::addItem (Widget *itemWidget, const StdString &itemId, int row, bool shouldSkipRefreshLayout) {
	CardView::Item item;
	StdString id;

	if (itemId.empty ()) {
		id.assign (getAvailableItemId ());
	}
	else {
		id.assign (itemId);
	}

	addWidget (itemWidget);

	isSorted = false;
	item.id.assign (id);
	item.widget = itemWidget;
	item.widget->retain ();

	if (row < 0) {
		row = 0;
	}
	item.row = row;
	itemList.push_back (item);

	if (! shouldSkipRefreshLayout) {
		refreshLayout ();
	}

	return (itemWidget);
}

void CardView::removeItem (const StdString &itemId) {
	std::list<CardView::Item>::iterator pos;

	pos = findItemPosition (itemId);
	if (pos == itemList.end ()) {
		return;
	}

	pos->widget->isDestroyed = true;
	pos->widget->release ();
	itemList.erase (pos);
	isSorted = false;
	refreshLayout ();
}

void CardView::removeItem (const char *itemId) {
	removeItem (StdString (itemId));
}

void CardView::removeRowItems (int row) {
	std::list<CardView::Item>::iterator i, end;
	bool found;

	while (true) {
		found = false;
		i = itemList.begin ();
		end = itemList.end ();
		while (i != end) {
			if (i->row == row) {
				i->widget->isDestroyed = true;
				i->widget->release ();
				itemList.erase (i);
				found = true;
				break;
			}
			++i;
		}

		if (! found) {
			break;
		}
	}

	isSorted = false;
	refreshLayout ();
}

void CardView::removeAllItems () {
	std::list<CardView::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		i->widget->isDestroyed = true;
		i->widget->release ();
		++i;
	}
	itemList.clear ();
	itemIdMap.clear ();
	isSorted = true;
	refreshLayout ();
}

void CardView::processItems (Widget::EventCallback fn, void *fnData, bool shouldRefreshLayout) {
	std::list<CardView::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		fn (fnData, i->widget);
		++i;
	}

	if (shouldRefreshLayout) {
		refreshLayout ();
	}
}

void CardView::processRowItems (int row, Widget::EventCallback fn, void *fnData, bool shouldRefreshLayout) {
	std::list<CardView::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->row == row) {
			fn (fnData, i->widget);
		}
		++i;
	}

	if (shouldRefreshLayout) {
		refreshLayout ();
	}
}

void CardView::scrollToItem (const StdString &itemId) {
	std::list<CardView::Item>::iterator pos;
	UiConfiguration *uiconfig;
	Widget *widget;

	pos = findItemPosition (itemId);
	if (pos == itemList.end ()) {
		return;
	}

	widget = pos->widget;
	uiconfig = &(App::getInstance ()->uiConfig);
	setViewOrigin (0.0f, widget->position.y - (height / 2.0f) + (widget->height / 2.0f));
	scrollBar->setPosition (viewOriginY, true);
	scrollBar->position.assignY (viewOriginY + uiconfig->paddingSize);
}

void CardView::refreshLayout () {
	UiConfiguration *uiconfig;
	std::list<CardView::Item>::iterator i, end;
	std::map<int, CardView::Row>::iterator rowpos;
	Widget *widget;
	LabelWindow *label;
	float x, y, x0, rowh, rowmargin;
	int row;

	uiconfig = &(App::getInstance ()->uiConfig);
	if (! isSorted) {
		doSort ();
	}

	row = -1;
	x0 = uiconfig->paddingSize;
	y = uiconfig->paddingSize;
	x = x0;
	rowmargin = itemMarginSize;
	rowh = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		widget = i->widget;
		if (row != i->row) {
			if (row >= 0) {
				x = x0;
				y += rowh + itemMarginSize;
				rowh = 0.0f;
			}
			row = i->row;

			if (row >= 0) {
				rowpos = getRow (row);
				label = rowpos->second.headerLabel;
				if (label && label->isVisible) {
					label->position.assign (x0, y);
					y += label->height + itemMarginSize;
				}
				rowmargin = rowpos->second.itemMarginSize;
				if (rowmargin < 0.0f) {
					rowmargin = itemMarginSize;
				}
			}
		}

		if ((x + widget->width) >= cardAreaWidth) {
			x = x0;
			y += rowh + rowmargin;
			rowh = 0.0f;
		}
		if (widget->height > rowh) {
			rowh = widget->height;
		}

		widget->position.assign (x, y);
		x += widget->width + rowmargin;

		++i;
	}

	y += rowh;
	y += (uiconfig->paddingSize * 2.0f);
	scrollBar->setScrollBounds (height, y);
	y -= height;
	if (y < 0.0f) {
		y = 0.0f;
	}
	setVerticalScrollBounds (0.0f, y);
	if (viewOriginY < 0.0f) {
		setViewOrigin (0.0f, 0.0f);
	}
	else if (viewOriginY > y) {
		setViewOrigin (0.0f, y);
	}

	if (scrollBar->maxScrollPosition <= 0.0f) {
		scrollBar->isVisible = false;
	}
	else {
		scrollBar->position.assign (width - uiconfig->paddingSize - scrollBar->width, viewOriginY + uiconfig->paddingSize);
		scrollBar->isVisible = true;
	}
}

void CardView::doSort () {
	std::map<int, CardView::Row>::iterator ri, rend;
	std::list<CardView::Item> outlist, rowlist;
	std::list<CardView::Item>::iterator i, iend;
	int row, nextrow, index;

	ri = rowMap.begin ();
	rend = rowMap.end ();
	while (ri != rend) {
		ri->second.itemCount = 0;
		++ri;
	}

	row = -1;
	i = itemList.begin ();
	iend = itemList.end ();
	while (i != iend) {
		if ((row < 0) || (i->row < row)) {
			row = i->row;
		}
		ri = getRow (i->row);
		++(ri->second.itemCount);
		++i;
	}

	while (true) {
		rowlist.clear ();
		nextrow = -1;
		i = itemList.begin ();
		iend = itemList.end ();
		while (i != iend) {
			if (i->row == row) {
				rowlist.push_back (*i);
			}
			else if (i->row > row) {
				if ((nextrow < 0) || (i->row < nextrow)) {
					nextrow = i->row;
				}
			}
			++i;
		}

		rowlist.sort (CardView::compareItems);
		i = rowlist.begin ();
		iend = rowlist.end ();
		while (i != iend) {
			outlist.push_back (*i);
			++i;
		}

		if (nextrow < 0) {
			break;
		}
		row = nextrow;
	}

	itemList.swap (outlist);

	itemIdMap.clear ();
	index = 0;
	i = itemList.begin ();
	iend = itemList.end ();
	while (i != iend) {
		itemIdMap.insert (i->id, index);
		++index;
		++i;
	}

	ri = rowMap.begin ();
	rend = rowMap.end ();
	while (ri != rend) {
		if (ri->second.headerLabel) {
			if (ri->second.itemCount <= 0) {
				ri->second.headerLabel->isVisible = false;
			}
			else {
				ri->second.headerLabel->isVisible = true;
			}
		}
		++ri;
	}
	isSorted = true;
}

bool CardView::compareItems (const CardView::Item &a, const CardView::Item &b) {
	return (a.widget->sortKey.compare (b.widget->sortKey) <= 0);
}

std::list<CardView::Item>::iterator CardView::findItemPosition (const StdString &itemId) {
	std::list<CardView::Item>::iterator i, end, pos;
	int index, curindex;

	index = itemIdMap.find (itemId, -1);
	if ((index < 0) || (index >= (int) itemList.size ())) {
		return (itemList.end ());
	}

	// TODO: Possibly modify this logic and associated data structures to avoid the need for iteration here (i.e. by directly accessing the element index instead)
	curindex = 0;
	i = itemList.begin ();
	end = itemList.end ();
	pos = end;
	while (i != end) {
		if (curindex == index) {
			pos = i;
			break;
		}
		++curindex;
		++i;
	}

	return (pos);
}

std::map<int, CardView::Row>::iterator CardView::getRow (int rowNumber) {
	std::map<int, CardView::Row>::iterator pos;
	CardView::Row row;

	pos = rowMap.find (rowNumber);
	if (pos == rowMap.end ()) {
		rowMap.insert (std::pair<int, CardView::Row> (rowNumber, row));
		pos = rowMap.find (rowNumber);
	}

	return (pos);
}

Widget *CardView::getItem (const StdString &itemId) {
	std::list<CardView::Item>::iterator pos;

	pos = findItemPosition (itemId);
	if (pos == itemList.end ()) {
		return (NULL);
	}
	return (pos->widget);
}

Widget *CardView::getItem (const char *itemId) {
	return (getItem (StdString (itemId)));
}

int CardView::getItemCount () {
	return ((int) itemList.size ());
}

Widget *CardView::findItem (CardView::MatchFunction fn, void *fnData) {
	std::list<CardView::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->widget) {
			if (fn (fnData, i->widget)) {
				return (i->widget);
			}
		}
		++i;
	}

	return (NULL);
}

void CardView::scrollBarPositionChanged (void *windowPtr, Widget *widgetPtr) {
	CardView *window;
	ScrollBar *scrollbar;
	UiConfiguration *uiconfig;

	window = (CardView *) windowPtr;
	scrollbar = (ScrollBar *) widgetPtr;
	uiconfig = &(App::getInstance ()->uiConfig);

	window->setViewOrigin (0.0f, scrollbar->scrollPosition);
	scrollbar->position.assignY (window->viewOriginY + uiconfig->paddingSize);
}
