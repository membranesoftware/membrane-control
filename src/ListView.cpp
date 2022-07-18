/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include <vector>
#include "App.h"
#include "StdString.h"
#include "HashMap.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Label.h"
#include "Panel.h"
#include "ScrollBar.h"
#include "ScrollView.h"
#include "ListView.h"

ListView::ListView (float viewWidth, int minViewItems, int maxViewItems, UiConfiguration::FontType itemFontType, const StdString &emptyStateText)
: ScrollView ()
, isDisabled (false)
, focusItemIndex (-1)
, viewWidth (viewWidth)
, minViewItems (minViewItems)
, maxViewItems (maxViewItems)
, itemFontType (itemFontType)
, scrollBar (NULL)
, emptyStateLabel (NULL)
, deleteButton (NULL)
, isItemFocused (false)
, lastFocusPanel (NULL)
, focusHighlightPanel (NULL)
{
	isMouseWheelScrollEnabled = true;
	if (minViewItems < 1) {
		minViewItems = 1;
	}
	if (maxViewItems < minViewItems) {
		maxViewItems = minViewItems;
	}

	setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	setBorder (true, UiConfiguration::instance->darkBackgroundColor);

	if (! emptyStateText.empty ()) {
		emptyStateLabel = (Label *) addWidget (new Label (emptyStateText, UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
		emptyStateLabel->isVisible = false;
	}

	scrollBar = (ScrollBar *) addWidget (new ScrollBar (1.0f));
	scrollBar->positionChangeCallback = Widget::EventCallbackContext (ListView::scrollBarPositionChanged, this);
	scrollBar->zLevel = 3;
	scrollBar->isVisible = false;

	deleteButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	deleteButton->mouseClickCallback = Widget::EventCallbackContext (ListView::deleteButtonClicked, this);
	deleteButton->zLevel = 2;
	deleteButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	deleteButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Remove).capitalized ());
	deleteButton->isVisible = false;

	focusHighlightPanel = (Panel *) addWidget (new Panel ());
	focusHighlightPanel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	focusHighlightPanel->zLevel = -1;
	focusHighlightPanel->isVisible = false;

	refreshLayout ();
}

ListView::~ListView () {
	clearItems ();
}

void ListView::freeItem (std::vector<ListView::Item>::iterator item) {
	if (item->panel) {
		item->panel->isDestroyed = true;
		item->panel = NULL;
	}
	if (item->data && item->dataFree) {
		item->dataFree (item->data);
	}
	item->data = NULL;
	item->dataFree = NULL;
}

void ListView::setViewWidth (float fixedWidth) {
	viewWidth = fixedWidth;
	refreshLayout ();
}

void ListView::setDisabled (bool disabled) {
	if (isDisabled == disabled) {
		return;
	}
	isDisabled = disabled;
	refreshLayout ();
}

void ListView::setEmptyStateText (const StdString &text) {
	if (! emptyStateLabel) {
		emptyStateLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	}
	emptyStateLabel->setText (text);
	refreshLayout ();
}

void ListView::clearItems () {
	std::vector<ListView::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		freeItem (i);
		++i;
	}
	itemList.clear ();
}

void ListView::setItems (StringList *itemList, bool shouldSkipChangeCallback) {
	StringList::iterator i, end;

	clearItems ();
	i = itemList->begin ();
	end = itemList->end ();
	while (i != end) {
		addItem (*i, NULL, NULL, true);
		++i;
	}
	if (! shouldSkipChangeCallback) {
		eventCallback (listChangeCallback);
	}
}

void ListView::getItems (StringList *destList) {
	std::vector<ListView::Item>::iterator i, end;

	destList->clear ();
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		destList->push_back (i->text);
		++i;
	}
}

int ListView::getItemCount () {
	return ((int) itemList.size ());
}

bool ListView::contains (const StdString &itemText) {
	std::vector<ListView::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->text.equals (itemText)) {
			return (true);
		}
		++i;
	}
	return (false);
}

StdString ListView::getItemText (int itemIndex) {
	if ((itemIndex < 0) || (itemIndex >= (int) itemList.size ())) {
		return (StdString (""));
	}
	return (itemList.at (itemIndex).text);
}

void *ListView::getItemData (int itemIndex) {
	if ((itemIndex < 0) || (itemIndex >= (int) itemList.size ())) {
		return (NULL);
	}
	return (itemList.at (itemIndex).data);
}

void ListView::addItem (const StdString &itemText, void *itemData, Widget::FreeFunction itemFree, bool shouldSkipChangeCallback) {
	ListView::Item item;
	Panel *panel;

	item.text.assign (itemText);
	item.data = itemData;
	item.dataFree = itemFree;

	panel = new Panel ();
	addWidget (panel);
	item.panel = panel;

	item.label = (Label *) panel->addWidget (new Label (itemText, itemFontType, UiConfiguration::instance->primaryTextColor));
	item.label->position.assign (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	panel->setFixedSize (true, viewWidth, UiConfiguration::instance->fonts[itemFontType]->maxLineHeight + UiConfiguration::instance->paddingSize);
	if (isDisabled) {
		item.label->textColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
	}

	itemList.push_back (item);
	refreshLayout ();
	if (! shouldSkipChangeCallback) {
		eventCallback (listChangeCallback);
	}
}

void ListView::removeItem (int itemIndex, bool shouldSkipChangeCallback) {
	std::vector<ListView::Item>::iterator pos;

	if ((itemIndex < 0) || (itemIndex >= (int) itemList.size ())) {
		return;
	}
	pos = itemList.begin () + itemIndex;
	freeItem (pos);
	itemList.erase (pos);
	if (itemIndex == focusItemIndex) {
		isItemFocused = false;
		lastFocusPanel = NULL;
		focusItemIndex = -1;
	}

	refreshLayout ();
	if (! shouldSkipChangeCallback) {
		eventCallback (listChangeCallback);
	}
}

void ListView::scrollToBottom () {
	setViewOrigin (0.0f, maxViewOriginY);
	scrollBar->setPosition (viewOriginY, true);
	scrollBar->position.assignY (viewOriginY);
}

void ListView::refreshLayout () {
	float x, y, itemh, h;
	Panel *panel;
	Label *label;
	std::vector<ListView::Item>::iterator i, end;
	int sz;

	x = widthPadding;
	y = heightPadding;
	if (! itemList.empty ()) {
		if (emptyStateLabel) {
			emptyStateLabel->isVisible = false;
		}
		i = itemList.begin ();
		end = itemList.end ();
		while (i != end) {
			panel = i->panel;
			label = i->label;

			if (isDisabled || (panel == lastFocusPanel)) {
				label->textColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
			}
			else {
				label->textColor.assign (UiConfiguration::instance->primaryTextColor);
			}

			panel->position.assign (x, y);
			y += panel->height;
			++i;
		}
	}

	itemh = UiConfiguration::instance->fonts[itemFontType]->maxLineHeight + UiConfiguration::instance->paddingSize;
	sz = (int) itemList.size ();
	if (sz < minViewItems) {
		sz = minViewItems;
	}
	if (sz > maxViewItems) {
		sz = maxViewItems;
	}
	h = itemh * sz;
	h += (UiConfiguration::instance->paddingSize * 2.0f);
	setViewSize (viewWidth, h);

	y -= (h - UiConfiguration::instance->paddingSize);
	if (y < 0.0f) {
		y = 0.0f;
	}
	scrollBar->setMaxTrackLength (h);
	scrollBar->setScrollBounds (itemh * sz, itemh * itemList.size ());

	setVerticalScrollBounds (0.0f, y);
	if (viewOriginY < 0.0f) {
		setViewOrigin (0.0f, 0.0f);
	}
	else if (viewOriginY > y) {
		setViewOrigin (0.0f, y);
	}

	if (scrollBar->maxScrollPosition <= 0.0f) {
		scrollBar->isVisible = false;
		isMouseWheelScrollEnabled = false;
	}
	else {
		scrollBar->position.assign (viewWidth - scrollBar->width, viewOriginY);
		scrollBar->isVisible = true;
		isMouseWheelScrollEnabled = true;
	}

	if (emptyStateLabel && itemList.empty ()) {
		emptyStateLabel->position.assign ((width / 2.0f) - (emptyStateLabel->width / 2.0f), (h / 2.0f) - (emptyStateLabel->height / 2.0f));
		emptyStateLabel->isVisible = true;
	}

	if (isItemFocused && lastFocusPanel) {
		x = width - (UiConfiguration::instance->paddingSize * 2.0f) - deleteButton->maxImageWidth;
		if (scrollBar->isVisible) {
			x -= scrollBar->width;
		}
		deleteButton->position.assign (x, lastFocusPanel->position.y + (lastFocusPanel->height / 2.0f) - (deleteButton->maxImageHeight / 2.0f));
		deleteButton->isVisible = true;

		focusHighlightPanel->position.assign (0.0f, lastFocusPanel->position.y);
		focusHighlightPanel->setFixedSize (true, width, itemh + UiConfiguration::instance->paddingSize);
		focusHighlightPanel->isVisible = true;
	}
	else {
		deleteButton->isVisible = false;
		focusHighlightPanel->isVisible = false;
	}
}

void ListView::scrollBarPositionChanged (void *listViewPtr, Widget *widgetPtr) {
	ListView *listview;
	ScrollBar *scrollbar;

	listview = (ListView *) listViewPtr;
	scrollbar = (ScrollBar *) widgetPtr;

	listview->setViewOrigin (0.0f, scrollbar->scrollPosition);
	scrollbar->position.assignY (listview->viewOriginY);
}

void ListView::deleteButtonClicked (void *listViewPtr, Widget *widgetPtr) {
	ListView *view;
	std::vector<ListView::Item>::iterator i, end;
	int index;

	view = (ListView *) listViewPtr;
	if (! view->lastFocusPanel) {
		return;
	}
	index = 0;
	i = view->itemList.begin ();
	end = view->itemList.end ();
	while (i != end) {
		if (i->panel == view->lastFocusPanel) {
			break;
		}
		++index;
		++i;
	}
	view->focusItemIndex = index;
	if (view->eventCallback (view->itemDeleteCallback)) {
		return;
	}

	view->removeItem (index, true);
	view->isItemFocused = false;
	view->lastFocusPanel = NULL;
	view->focusItemIndex = -1;
	view->refreshLayout ();
	view->eventCallback (view->listChangeCallback);
}

bool ListView::doProcessMouseState (const Widget::MouseState &mouseState) {
	bool consumed, shouldrefresh, found;
	float y1;
	std::vector<ListView::Item>::iterator i, end;

	y1 = viewOriginY;
	consumed = ScrollView::doProcessMouseState (mouseState);
	if (! FLOAT_EQUALS (y1, viewOriginY)) {
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY);
	}

	shouldrefresh = false;
	if (isDisabled || (! mouseState.isEntered)) {
		if (isItemFocused) {
			isItemFocused = false;
			lastFocusPanel = NULL;
			shouldrefresh = true;
		}
	}
	else {
		found = false;
		i = itemList.begin ();
		end = itemList.end ();
		while (i != end) {
			if ((mouseState.enterDeltaY >= (i->panel->position.y - viewOriginY)) && (mouseState.enterDeltaY < (i->panel->position.y - viewOriginY + i->panel->height))) {
				found = true;
				if ((! isItemFocused) || (i->panel != lastFocusPanel)) {
					shouldrefresh = true;
					isItemFocused = true;
					lastFocusPanel = i->panel;
				}
				break;
			}
			++i;
		}
		if ((! found) && isItemFocused) {
			isItemFocused = false;
			lastFocusPanel = NULL;
			shouldrefresh = true;
		}
	}
	if (shouldrefresh) {
		refreshLayout ();
	}

	return (consumed);
}

void ListView::doRefresh () {
	float h;
	std::vector<ListView::Item>::iterator i, end;

	h = UiConfiguration::instance->fonts[itemFontType]->maxLineHeight + UiConfiguration::instance->paddingSize;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		i->panel->setFixedSize (true, viewWidth, h);
		++i;
	}
	Panel::doRefresh ();
}
