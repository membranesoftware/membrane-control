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
, sortFunction (NULL)
, isSorted (false)
, itemClickCallback (NULL)
, itemClickCallbackData (NULL)
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
			i->widget->release ();
		}
		++i;
	}
	itemList.clear ();
}

void CardView::sort (CardView::SortFunction fn) {
	if (sortFunction == fn) {
		return;
	}
	sortFunction = fn;
	isSorted = false;
	resetLayout ();
}

void CardView::setViewSize (float viewWidth, float viewHeight) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setFixedSize (true, viewWidth, viewHeight);
	scrollBar->setMaxTrackLength (viewHeight - (uiconfig->paddingSize * 2.0f));
	cardAreaWidth = width - scrollBar->width - uiconfig->paddingSize - (uiconfig->marginSize * 0.25f);
	resetLayout ();
}

void CardView::setItemMarginSize (float marginSize) {
	if (FLOAT_EQUALS (marginSize, itemMarginSize)) {
		return;
	}
	itemMarginSize = marginSize;
	resetLayout ();
}

void CardView::setItemClickCallback (Widget::EventCallback callback, void *callbackData) {
	itemClickCallback = callback;
	itemClickCallbackData = callbackData;
}

void CardView::setRowHeader (int row, const StdString &headerText, int headerFontType, const Color &color) {
	UiConfiguration *uiconfig;
	std::map<int, LabelWindow *>::iterator pos;
	LabelWindow *label;

	uiconfig = &(App::getInstance ()->uiConfig);
	label = (LabelWindow *) addWidget (new LabelWindow (new Label (headerText, headerFontType, color)));
	label->isVisible = false;
	label->zLevel = 1;
	label->setFillBg (true, 0.0f, 0.0f, 0.0f);
	label->setAlphaBlend (true, uiconfig->imageTextScrimAlpha);

	pos = rowHeaderLabelMap.find (row);
	if (pos != rowHeaderLabelMap.end ()) {
		pos->second->isDestroyed = true;
		pos->second = label;
	}
	else {
		rowHeaderLabelMap.insert (std::pair<int, LabelWindow *> (row, label));
	}

	resetLayout ();
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

Widget *CardView::addItem (Widget *itemWidget, const char *itemId, int row, bool shouldSkipResetLayout) {
	return (addItem (itemWidget, StdString (itemId ? itemId : ""), row, shouldSkipResetLayout));
}

Widget *CardView::addItem (Widget *itemWidget, const StdString &itemId, int row, bool shouldSkipResetLayout) {
	CardView::Item item;
	StdString id;

	if (itemId.empty ()) {
		id.assign (getAvailableItemId ());
	}
	else {
		id.assign (itemId);
	}

	itemWidget->setMouseClickCallback (CardView::itemClicked, this);
	addWidget (itemWidget);

	isSorted = false;
	item.id.assign (id);
	item.widget = itemWidget;
	item.widget->retain ();
	item.row = row;
	itemList.push_back (item);

	if (! shouldSkipResetLayout) {
		resetLayout ();
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
	resetLayout ();
}

void CardView::removeItem (const char *itemId) {
	removeItem (StdString (itemId));
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
	resetLayout ();
}

void CardView::processItems (Widget::EventCallback fn, void *fnData, bool shouldResetLayout) {
	std::list<CardView::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		fn (fnData, i->widget);
		++i;
	}

	if (shouldResetLayout) {
		resetLayout ();
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
	setViewOrigin (0.0f, widget->position.y + (height / 2.0f) - (widget->height / 2.0f));
	scrollBar->setPosition (viewOriginY, true);
	scrollBar->position.assignY (viewOriginY + uiconfig->paddingSize);
}

void CardView::resetLayout () {
	UiConfiguration *uiconfig;
	std::list<CardView::Item>::iterator i, end;
	std::map<int, LabelWindow *>::iterator hpos;
	Widget *widget;
	LabelWindow *label;
	float x, y, x0, rowh;
	int row;

	uiconfig = &(App::getInstance ()->uiConfig);
	if (! isSorted) {
		doSort ();
	}

	row = -1;
	x0 = uiconfig->paddingSize;
	y = uiconfig->paddingSize;
	x = x0;
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
				hpos = rowHeaderLabelMap.find (row);
				if (hpos != rowHeaderLabelMap.end ()) {
					label = hpos->second;
					if (label->isVisible) {
						label->position.assign (x0, y);
						y += label->height + itemMarginSize;
					}
				}
			}
		}

		if ((x + widget->width) >= cardAreaWidth) {
			x = x0;
			y += rowh + itemMarginSize;
			rowh = 0.0f;
		}
		if (widget->height > rowh) {
			rowh = widget->height;
		}

		widget->position.assign (x, y);
		x += widget->width + itemMarginSize;

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
	std::list<CardView::Item>::iterator i, iend, j, jend, pos;
	std::list<CardView::Item> sortlist;
	std::map<int, bool> rowmap;
	std::map<int, bool>::iterator rowpos;
	std::map<int, LabelWindow *>::iterator hi, hend;
	int index;

	i = itemList.begin ();
	iend = itemList.end ();
	while (i != iend) {
		if (i->row >= 0) {
			rowpos = rowmap.find (i->row);
			if (rowpos == rowmap.end ()) {
				rowmap.insert (std::pair<int, bool> (i->row, true));
			}
		}

		j = sortlist.begin ();
		jend = sortlist.end ();
		pos = jend;
		while (j != jend) {
			if ((i->row >= 0) && (j->row < 0)) {
				pos = j;
				break;
			}

			if ((i->row < 0) && (j->row >= 0)) {
				++j;
				continue;
			}

			if (i->row < j->row) {
				pos = j;
				break;
			}

			if (i->row > j->row) {
				++j;
				continue;
			}

			if (sortFunction && sortFunction (i->widget, j->widget)) {
				pos = j;
				break;
			}

			++j;
		}
		if (pos == jend) {
			sortlist.push_back (*i);
		}
		else {
			sortlist.insert (pos, *i);
		}

		++i;
	}

	itemList.swap (sortlist);

	itemIdMap.clear ();
	index = 0;
	i = itemList.begin ();
	iend = itemList.end ();
	while (i != iend) {
		itemIdMap.insert (i->id, index);
		++index;
		++i;
	}

	hi = rowHeaderLabelMap.begin ();
	hend = rowHeaderLabelMap.end ();
	while (hi != hend) {
		rowpos = rowmap.find (hi->first);
		if (rowpos == rowmap.end ()) {
			hi->second->isVisible = false;
		}
		else {
			hi->second->isVisible = true;
		}
		++hi;
	}

	isSorted = true;
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

void CardView::itemClicked (void *windowPtr, Widget *widgetPtr) {
	CardView *window;

	window = (CardView *) windowPtr;
	if (window->itemClickCallback) {
		window->itemClickCallback (window->itemClickCallbackData, widgetPtr);
	}
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
