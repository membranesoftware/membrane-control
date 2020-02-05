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
#include "Config.h"
#include <stdlib.h>
#include <math.h>
#include <list>
#include <map>
#include "SDL2/SDL.h"
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

const float CardView::smallItemScale = 0.83f;
const int CardView::animateScaleDuration = 80; // ms

CardView::CardView (float viewWidth, float viewHeight)
: ScrollView (viewWidth, viewHeight)
, cardAreaWidth (viewWidth)
, itemMarginSize (0.0f)
, itemMutex (NULL)
, isSorted (false)
, scrollBar (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	itemMarginSize = uiconfig->marginSize;
	itemMutex = SDL_CreateMutex ();
	scrollBar = (ScrollBar *) addWidget (new ScrollBar (viewHeight - (uiconfig->paddingSize * 2.0f)));
	scrollBar->setPositionChangeCallback (CardView::scrollBarPositionChanged, this);
	scrollBar->zLevel = 2;
	scrollBar->isVisible = false;

	cardAreaWidth = width - scrollBar->width - (uiconfig->paddingSize * 2.0f) - (uiconfig->marginSize * 0.25f);
}

CardView::~CardView () {
	std::list<CardView::Item>::iterator i, end;

	if (itemMutex) {
		SDL_LockMutex (itemMutex);
	}
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->panel) {
			i->panel->isDestroyed = true;
			i->panel->release ();
		}
		++i;
	}
	itemList.clear ();
	if (itemMutex) {
		SDL_UnlockMutex (itemMutex);
	}

	if (itemMutex) {
		SDL_DestroyMutex (itemMutex);
		itemMutex = NULL;
	}
}

void CardView::setViewSize (float viewWidth, float viewHeight) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	setFixedSize (true, viewWidth, viewHeight);
	scrollBar->setMaxTrackLength (viewHeight - (uiconfig->paddingSize * 2.0f));
	cardAreaWidth = width - scrollBar->width - (uiconfig->paddingSize * 2.0f) - (uiconfig->marginSize * 0.25f);
	refreshLayout ();
}

void CardView::setItemMarginSize (float marginSize) {
	if (FLOAT_EQUALS (marginSize, itemMarginSize)) {
		return;
	}
	itemMarginSize = marginSize;
	refreshLayout ();
}

void CardView::setRowHeader (int row, Panel *headerPanel) {
	std::map<int, CardView::Row>::iterator pos;
	bool visible;

	visible = true;
	pos = getRow (row);
	if (pos->second.headerPanel) {
		visible = pos->second.headerPanel->isVisible;
		pos->second.headerPanel->isDestroyed = true;
	}
	addWidget (headerPanel);
	pos->second.headerPanel = headerPanel;
	pos->second.headerPanel->zLevel = 1;
	pos->second.headerPanel->isVisible = visible;

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

void CardView::setRowReverseSorted (int row, bool enable) {
	std::map<int, CardView::Row>::iterator pos;

	pos = getRow (row);
	if (enable == pos->second.isReverseSorted) {
		return;
	}
	pos->second.isReverseSorted = enable;
	refreshLayout ();
}

void CardView::setRowSelectionAnimated (int row, bool enable) {
	std::map<int, CardView::Row>::iterator pos;

	if (! App::instance->isInterfaceAnimationEnabled) {
		return;
	}

	pos = getRow (row);
	if (enable == pos->second.isSelectionAnimated) {
		return;
	}
	pos->second.isSelectionAnimated = enable;
	refreshLayout ();
}

bool CardView::isRowSelectionAnimated (int row) {
	std::map<int, CardView::Row>::iterator rowpos;

	if (rowMap.count (row) <= 0) {
		return (false);
	}
	rowpos = getRow (row);
	return (rowpos->second.isSelectionAnimated);
}

void CardView::setRowDetail (int row, int detailType) {
	UiConfiguration *uiconfig;
	std::map<int, CardView::Row>::iterator pos;
	std::list<CardView::Item>::iterator i, end;
	float scale;

	if ((detailType < 0) || (detailType > CardView::HighDetail)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	pos = getRow (row);
	if (detailType == pos->second.layout) {
		return;
	}
	pos->second.layout = detailType;
	switch (pos->second.layout) {
		case CardView::LowDetail: {
			scale = uiconfig->smallThumbnailImageScale;
			break;
		}
		case CardView::HighDetail: {
			scale = uiconfig->largeThumbnailImageScale;
			break;
		}
		default: {
			scale = uiconfig->mediumThumbnailImageScale;
			break;
		}
	}
	if (pos->second.isSelectionAnimated) {
		scale /= CardView::smallItemScale;
	}
	pos->second.maxItemWidth = cardAreaWidth * scale;
	if (pos->second.maxItemWidth < 1.0f) {
		pos->second.maxItemWidth = 1.0f;
	}

	SDL_LockMutex (itemMutex);
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->row == row) {
			i->panel->setLayout (pos->second.layout, pos->second.maxItemWidth);
		}
		++i;
	}
	SDL_UnlockMutex (itemMutex);

	if (isRowSelectionAnimated (row)) {
		resetRowSelection (row);
	}

	refreshLayout ();
}

void CardView::doUpdate (int msElapsed) {
	std::map<int, CardView::Row>::iterator i, iend;
	std::list<CardView::Item>::iterator j, jend;
	bool refresh;

	ScrollView::doUpdate (msElapsed);

	refresh = false;
	i = rowMap.begin ();
	iend = rowMap.end ();
	while (i != iend) {
		if (i->second.isSelectionAnimated) {
			SDL_LockMutex (itemMutex);
			j = itemList.begin ();
			jend = itemList.end ();
			while (j != jend) {
				if (j->row == i->first) {
					if (! j->panel->isVisible) {
						j->panel->animateScale (0.99f, CardView::smallItemScale, CardView::animateScaleDuration);
						j->panel->isVisible = true;
						refresh = true;
					}
				}
				++j;
			}
			SDL_UnlockMutex (itemMutex);
		}
		++i;
	}

	if (refresh) {
		refreshLayout ();
	}
}

void CardView::doProcessMouseState (const Widget::MouseState &mouseState) {
	std::map<int, CardView::Row>::iterator i, iend;
	std::list<CardView::Item>::iterator j, jend, item;
	int mousex, mousey;
	float x1, y1, x2, y2, dx, dy;
	bool highlight;

	y1 = viewOriginY;
	ScrollView::doProcessMouseState (mouseState);
	if (! FLOAT_EQUALS (y1, viewOriginY)) {
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY + App::instance->uiConfig.paddingSize);
	}

	mousex = App::instance->input.mouseX;
	mousey = App::instance->input.mouseY;
	if (! highlightedItemId.empty ()) {
		SDL_LockMutex (itemMutex);
		item = findItemPosition (highlightedItemId);
		if (item == itemList.end ()) {
			highlightedItemId.assign ("");
		}
		else {
			highlight = true;
			if (! mouseState.isEntered) {
				highlight = false;
			}
			else {
				x1 = item->panel->screenX;
				y1 = item->panel->screenY;
				x2 = x1 + item->panel->width;
				y2 = y1 + item->panel->height;
				if (!((mousex >= (int) x1) && (mousey >= (int) y1) && (mousex <= (int) x2) && (mousey <= (int) y2))) {
					highlight = false;
				}
			}

			if (! highlight) {
				item->isHighlighted = false;
				item->panel->zLevel = -1;
				item->panel->animateScale (1.0f, CardView::smallItemScale, CardView::animateScaleDuration * 2);
				highlightedItemId.assign ("");
			}
		}
		SDL_UnlockMutex (itemMutex);
	}

	if (highlightedItemId.empty ()) {
		i = rowMap.begin ();
		iend = rowMap.end ();
		while (i != iend) {
			if (i->second.isSelectionAnimated) {
				SDL_LockMutex (itemMutex);
				j = itemList.begin ();
				jend = itemList.end ();
				while (j != jend) {
					if ((j->row == i->first) && j->panel->isVisible) {
						highlight = false;
						if (mouseState.isEntered && highlightedItemId.empty ()) {
							x1 = j->panel->screenX;
							y1 = j->panel->screenY;
							x2 = x1 + j->panel->width;
							y2 = y1 + j->panel->height;
							dx = (1.0f - CardView::smallItemScale) * j->panel->width;
							dy = (1.0f - CardView::smallItemScale) * j->panel->height;
							x1 += (dx / 2.0f);
							y1 += (dy / 2.0f);
							x2 -= (dx / 2.0f);
							y2 -= (dy / 2.0f);
							if ((mousex >= (int) x1) && (mousey >= (int) y1) && (mousex <= (int) x2) && (mousey <= (int) y2)) {
								highlight = true;
							}
						}

						if (highlight) {
							j->isHighlighted = true;
							j->panel->zLevel = 1;
							j->panel->animateScale (CardView::smallItemScale, 1.0f, CardView::animateScaleDuration);
							highlightedItemId.assign (j->id);
						}
						else {
							if (j->isHighlighted) {
								j->isHighlighted = false;
								j->panel->zLevel = -1;
								j->panel->animateScale (1.0f, CardView::smallItemScale, CardView::animateScaleDuration * 2);
							}
						}
					}
					++j;
				}
				SDL_UnlockMutex (itemMutex);
			}
			++i;
		}
	}
}

bool CardView::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	float y;
	bool result;

	y = viewOriginY;
	result = ScrollView::doProcessKeyEvent (keycode, isShiftDown, isControlDown);
	if (! FLOAT_EQUALS (y, viewOriginY)) {
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY + App::instance->uiConfig.paddingSize);
	}

	return (result);
}

void CardView::resetRowSelection (int row) {
	std::list<CardView::Item>::iterator i, end;

	if (! isRowSelectionAnimated (row)) {
		return;
	}

	SDL_LockMutex (itemMutex);
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->row == row) {
			i->panel->isVisible = false;
			i->panel->setTextureRender (false);
		}
		++i;
	}
	SDL_UnlockMutex (itemMutex);
}

bool CardView::empty () {
	bool result;

	SDL_LockMutex (itemMutex);
	result = itemList.empty ();
	SDL_UnlockMutex (itemMutex);

	return (result);
}

bool CardView::contains (const StdString &itemId) {
	bool result;

	SDL_LockMutex (itemMutex);
	result = itemIdMap.exists (itemId);
	SDL_UnlockMutex (itemMutex);

	return (result);
}

bool CardView::contains (const char *itemId) {
	return (contains (StdString (itemId)));
}

StdString CardView::getAvailableItemId () {
	StdString id;

	while (true) {
		id.sprintf ("CardView_item_%016llx", (long long int) App::instance->getUniqueId ());
		if (! contains (id)) {
			break;
		}
	}

	return (id);
}

Widget *CardView::addItem (Panel *itemPanel, const char *itemId, int row, bool shouldSkipRefreshLayout) {
	return (addItem (itemPanel, StdString (itemId ? itemId : ""), row, shouldSkipRefreshLayout));
}

Widget *CardView::addItem (Panel *itemPanel, const StdString &itemId, int row, bool shouldSkipRefreshLayout) {
	CardView::Item item;
	std::map<int, CardView::Row>::iterator rowpos;
	StdString id;

	if (itemId.empty ()) {
		id.assign (getAvailableItemId ());
	}
	else {
		id.assign (itemId);
	}
	if (row < 0) {
		row = 0;
	}

	addWidget (itemPanel);

	item.id.assign (id);
	item.panel = itemPanel;
	item.panel->retain ();
	item.row = row;
	rowpos = getRow (row);
	if (rowpos->second.isSelectionAnimated) {
		item.panel->isVisible = false;
	}
	if (rowpos->second.layout >= 0) {
		item.panel->setLayout (rowpos->second.layout, rowpos->second.maxItemWidth);
	}

	SDL_LockMutex (itemMutex);
	itemList.push_back (item);
	isSorted = false;
	SDL_UnlockMutex (itemMutex);

	if (! shouldSkipRefreshLayout) {
		refreshLayout ();
	}
	return (itemPanel);
}

void CardView::removeItem (const StdString &itemId, bool shouldSkipRefreshLayout) {
	std::list<CardView::Item>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (itemMutex);
	pos = findItemPosition (itemId);
	if (pos != itemList.end ()) {
		found = true;
		pos->panel->isDestroyed = true;
		pos->panel->release ();
		isSorted = false;
		itemList.erase (pos);
		resetItemIdMap ();
	}
	SDL_UnlockMutex (itemMutex);

	if (found && (! shouldSkipRefreshLayout)) {
		refreshLayout ();
	}
}

void CardView::removeItem (const char *itemId, bool shouldSkipRefreshLayout) {
	removeItem (StdString (itemId), shouldSkipRefreshLayout);
}

void CardView::removeRowItems (int row) {
	std::list<CardView::Item>::iterator i, end;
	bool found;

	SDL_LockMutex (itemMutex);
	while (true) {
		found = false;
		i = itemList.begin ();
		end = itemList.end ();
		while (i != end) {
			if (i->row == row) {
				i->panel->isDestroyed = true;
				i->panel->release ();
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
	SDL_UnlockMutex (itemMutex);

	refreshLayout ();
}

void CardView::setItemRow (const StdString &itemId, int targetRow, bool shouldSkipRefreshLayout) {
	std::list<CardView::Item>::iterator pos;

	SDL_LockMutex (itemMutex);
	pos = findItemPosition (itemId);
	if ((pos != itemList.end ()) && (pos->row != targetRow)) {
		pos->row = targetRow;
		isSorted = false;
	}
	SDL_UnlockMutex (itemMutex);

	if (! shouldSkipRefreshLayout) {
		refreshLayout ();
	}
}

void CardView::removeAllItems () {
	std::list<CardView::Item>::iterator i, end;

	SDL_LockMutex (itemMutex);
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		i->panel->isDestroyed = true;
		i->panel->release ();
		++i;
	}
	itemList.clear ();
	itemIdMap.clear ();
	isSorted = true;
	SDL_UnlockMutex (itemMutex);

	refreshLayout ();
}

void CardView::processItems (Widget::EventCallback fn, void *fnData, bool shouldRefreshLayout) {
	std::list<CardView::Item>::iterator i, end;

	SDL_LockMutex (itemMutex);
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		fn (fnData, i->panel);
		++i;
	}
	SDL_UnlockMutex (itemMutex);

	if (shouldRefreshLayout) {
		refreshLayout ();
	}
}

void CardView::processRowItems (int row, Widget::EventCallback fn, void *fnData, bool shouldRefreshLayout) {
	std::list<CardView::Item>::iterator i, end;

	SDL_LockMutex (itemMutex);
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->row == row) {
			fn (fnData, i->panel);
		}
		++i;
	}
	SDL_UnlockMutex (itemMutex);

	if (shouldRefreshLayout) {
		refreshLayout ();
	}
}

void CardView::scrollToItem (const StdString &itemId) {
	std::list<CardView::Item>::iterator pos;
	UiConfiguration *uiconfig;

	SDL_LockMutex (itemMutex);
	pos = findItemPosition (itemId);
	if (pos != itemList.end ()) {
		uiconfig = &(App::instance->uiConfig);
		setViewOrigin (0.0f, pos->panel->position.y - (height / 2.0f) + (pos->panel->height / 2.0f));
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY + uiconfig->paddingSize);
	}
	SDL_UnlockMutex (itemMutex);
}

void CardView::refreshLayout () {
	UiConfiguration *uiconfig;
	std::list<CardView::Item>::iterator i, end;
	std::map<int, CardView::Row>::iterator rowpos;
	Panel *itempanel, *headerpanel;
	float x, y, dx, dy, x0, itemw, itemh, rowh, rowmargin;
	int row;

	uiconfig = &(App::instance->uiConfig);
	row = -1;
	x0 = uiconfig->paddingSize;
	y = uiconfig->paddingSize;
	x = x0;
	rowmargin = itemMarginSize;
	rowh = 0.0f;

	SDL_LockMutex (itemMutex);
	if (! isSorted) {
		doSort ();
	}
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		itempanel = i->panel;
		itemw = itempanel->width;
		itemh = itempanel->height;
		dx = 0.0f;
		dy = 0.0f;

		if (row != i->row) {
			if (row >= 0) {
				x = x0;
				y += rowh + itemMarginSize;
				rowh = 0.0f;
			}
			row = i->row;

			if (row >= 0) {
				rowpos = getRow (row);
				headerpanel = rowpos->second.headerPanel;
				if (headerpanel && headerpanel->isVisible) {
					headerpanel->position.assign (x0, y);
					y += headerpanel->height + itemMarginSize;
				}
				rowmargin = rowpos->second.itemMarginSize;
				if (rowmargin < 0.0f) {
					rowmargin = itemMarginSize;
				}
			}
		}

		if (row >= 0) {
			rowpos = getRow (row);
			if (rowpos->second.isSelectionAnimated) {
				itemw *= CardView::smallItemScale;
				itemh *= CardView::smallItemScale;
				dx = (itempanel->width - itemw) / -4.0f;
				dy = (itempanel->height - itemh) / -4.0f;
			}
		}

		if ((x + itemw) >= cardAreaWidth) {
			x = x0;
			y += rowh + rowmargin;
			rowh = 0.0f;
		}
		if (itemh > rowh) {
			rowh = itemh;
		}

		itempanel->position.assign (x + dx, y + dy);
		x += itemw + rowmargin;

		++i;
	}
	SDL_UnlockMutex (itemMutex);

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
	int row, nextrow;

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

		ri = getRow (row);
		rowlist.sort (ri->second.isReverseSorted ? CardView::compareItemsDescending : CardView::compareItemsAscending);

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
	resetItemIdMap ();

	ri = rowMap.begin ();
	rend = rowMap.end ();
	while (ri != rend) {
		if (ri->second.headerPanel) {
			if (ri->second.itemCount <= 0) {
				ri->second.headerPanel->isVisible = false;
			}
			else {
				ri->second.headerPanel->isVisible = true;
			}
		}
		++ri;
	}
	isSorted = true;
}

void CardView::resetItemIdMap () {
	std::list<CardView::Item>::iterator i, end;
	int index;

	itemIdMap.clear ();
	index = 0;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		itemIdMap.insert (i->id, index);
		++index;
		++i;
	}
}

bool CardView::compareItemsAscending (const CardView::Item &a, const CardView::Item &b) {
	return (a.panel->sortKey.compare (b.panel->sortKey) <= 0);
}

bool CardView::compareItemsDescending (const CardView::Item &a, const CardView::Item &b) {
	return (a.panel->sortKey.compare (b.panel->sortKey) > 0);
}

std::list<CardView::Item>::iterator CardView::findItemPosition (const StdString &itemId) {
	std::list<CardView::Item>::iterator pos;
	int index;

	index = itemIdMap.find (itemId, -1);
	if ((index < 0) || (index >= (int) itemList.size ())) {
		return (itemList.end ());
	}

	pos = itemList.begin ();
	while (index > 0) {
		++pos;
		--index;
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
	Widget *result;

	result = NULL;
	SDL_LockMutex (itemMutex);
	pos = findItemPosition (itemId);
	if (pos != itemList.end ()) {
		result = pos->panel;
	}
	SDL_UnlockMutex (itemMutex);

	return (result);
}

Widget *CardView::getItem (const char *itemId) {
	return (getItem (StdString (itemId)));
}

int CardView::getItemCount () {
	int result;

	SDL_LockMutex (itemMutex);
	result = (int) itemList.size ();
	SDL_UnlockMutex (itemMutex);

	return (result);
}

int CardView::getRowItemCount (int row) {
	std::map<int, CardView::Row>::iterator rowpos;

	if (rowMap.count (row) <= 0) {
		return (0);
	}
	rowpos = getRow (row);
	return (rowpos->second.itemCount);
}

Widget *CardView::findItem (CardView::MatchFunction fn, void *fnData) {
	std::list<CardView::Item>::iterator i, end;
	Widget *result;

	result = NULL;
	SDL_LockMutex (itemMutex);
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->panel) {
			if (fn (fnData, i->panel)) {
				result = i->panel;
				break;
			}
		}
		++i;
	}
	SDL_UnlockMutex (itemMutex);

	return (result);
}

void CardView::scrollBarPositionChanged (void *windowPtr, Widget *widgetPtr) {
	CardView *window;
	ScrollBar *scrollbar;
	UiConfiguration *uiconfig;

	window = (CardView *) windowPtr;
	scrollbar = (ScrollBar *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);

	window->setViewOrigin (0.0f, scrollbar->scrollPosition);
	scrollbar->position.assignY (window->viewOriginY + uiconfig->paddingSize);
}
