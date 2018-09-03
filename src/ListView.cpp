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
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "HashMap.h"
#include "Ui.h"
#include "Widget.h"
#include "Label.h"
#include "Panel.h"
#include "UiConfiguration.h"
#include "ListView.h"

ListView::ListView (float viewWidth, int minItemHeight, int itemFontType, const StdString &titleText, const StdString &emptyStateText)
: Panel ()
, viewWidth (viewWidth)
, minItemHeight (minItemHeight)
, itemFontType (itemFontType)
, titleLabel (NULL)
, emptyStateLabel (NULL)
, deleteButton (NULL)
, isItemFocused (false)
, lastFocusPanel (NULL)
, listChangeCallback (NULL)
, listChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	typeName.assign ("ListView");

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	setFillBg (true, uiconfig->lightBackgroundColor);

	if (! titleText.empty ()) {
		titleLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (titleText, UiConfiguration::TITLE, uiconfig->inverseTextColor)));
		titleLabel->setFillBg (true, uiconfig->mediumPrimaryColor);
		titleLabel->setPadding (uiconfig->paddingSize, uiconfig->textLineHeightMargin * 2.0f);
		titleLabel->setWindowWidth (viewWidth);
	}

	if (! emptyStateText.empty ()) {
		emptyStateLabel = (Label *) addWidget (new Label (emptyStateText, UiConfiguration::CAPTION, uiconfig->primaryTextColor));
		emptyStateLabel->isVisible = false;
	}

	deleteButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON)));
	deleteButton->setMouseClickCallback (ListView::deleteButtonClicked, this);
	deleteButton->zLevel = 2;
	deleteButton->setImageColor (uiconfig->flatButtonTextColor);
	deleteButton->setMouseHoverTooltip (uitext->remove.capitalized ());
	deleteButton->isVisible = false;

	resetLayout ();
}

ListView::~ListView () {
	clearItems ();
}

void ListView::setViewWidth (float fixedWidth) {
	viewWidth = fixedWidth;
	resetLayout ();
}

void ListView::setListChangeCallback (Widget::EventCallback callback, void *callbackData) {
	listChangeCallback = callback;
	listChangeCallbackData = callbackData;
}

void ListView::clearItems () {
	std::list<ListView::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->panel) {
			i->panel->isDestroyed = true;
		}
		++i;
	}
	itemList.clear ();
}

void ListView::setItems (StringList *itemList) {
	StringList::iterator i, end;
	Widget::EventCallback callback;
	void *callbackdata;

	callback = listChangeCallback;
	callbackdata = listChangeCallbackData;
	listChangeCallback = NULL;
	listChangeCallbackData = NULL;

	clearItems ();
	i = itemList->begin ();
	end = itemList->end ();
	while (i != end) {
		addItem (*i);
		++i;
	}

	listChangeCallback = callback;
	listChangeCallbackData = callbackdata;
	if (listChangeCallback) {
		listChangeCallback (listChangeCallbackData, this);
	}
}

void ListView::getItems (StringList *destList) {
	std::list<ListView::Item>::iterator i, end;

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

void ListView::addItem (const StdString &itemText) {
	ListView::Item item;
	Panel *panel;
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);

	item.text.assign (itemText);

	panel = new Panel ();
	addWidget (panel);
	item.panel = panel;

	item.label = (Label *) panel->addWidget (new Label (itemText, itemFontType, uiconfig->primaryTextColor));
	item.label->position.assign (uiconfig->paddingSize, uiconfig->paddingSize);
	panel->setFixedSize (true, viewWidth, uiconfig->fonts[itemFontType]->maxLineHeight + uiconfig->paddingSize);

	itemList.push_back (item);
	resetLayout ();

	if (listChangeCallback) {
		listChangeCallback (listChangeCallbackData, this);
	}
}

void ListView::resetLayout () {
	UiConfiguration *uiconfig;
	float x, y, h;
	std::list<ListView::Item>::iterator i, end;
	int sz;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = 0.0f;
	y = 0.0f;
	if (titleLabel) {
		titleLabel->position.assign (x, y);
		y += titleLabel->height;
	}
	else {
		y += heightPadding;
	}
	x += widthPadding;

	if (! itemList.empty ()) {
		if (emptyStateLabel) {
			emptyStateLabel->isVisible = false;
		}
		i = itemList.begin ();
		end = itemList.end ();
		while (i != end) {
			i->panel->position.assign (x, y);
			y += i->panel->height;
			++i;
		}
	}

	if (isItemFocused && lastFocusPanel) {
		deleteButton->position.assign (width - (uiconfig->paddingSize * 2.0f) - deleteButton->maxImageWidth, lastFocusPanel->position.y + (lastFocusPanel->height / 2.0f) - (deleteButton->maxImageHeight / 2.0f));
		deleteButton->isVisible = true;
	}
	else {
		deleteButton->isVisible = false;
	}

	h = uiconfig->fonts[itemFontType]->maxLineHeight + uiconfig->paddingSize;
	sz = (int) itemList.size ();
	if ((minItemHeight > 0) && (sz < minItemHeight)) {
		sz = minItemHeight;
	}
	h *= sz;

	setFixedSize (true, viewWidth, h);

	if (itemList.empty ()) {
		if (emptyStateLabel) {
			emptyStateLabel->position.assign ((width / 2.0f) - (emptyStateLabel->width / 2.0f), (h / 2.0f) - (emptyStateLabel->height / 2.0f));
			emptyStateLabel->isVisible = true;
		}
	}
}

void ListView::deleteButtonClicked (void *listViewPtr, Widget *widgetPtr) {
	ListView *view;
	std::list<ListView::Item>::iterator i, end;

	view = (ListView *) listViewPtr;
	if (! view->lastFocusPanel) {
		return;
	}

	i = view->itemList.begin ();
	end = view->itemList.end ();
	while (i != end) {
		if (i->panel == view->lastFocusPanel) {
			i->panel->isDestroyed = true;
			view->itemList.erase (i);
			break;
		}
		++i;
	}

	view->isItemFocused = false;
	view->lastFocusPanel = NULL;
	view->resetLayout ();
	if (view->listChangeCallback) {
		view->listChangeCallback (view->listChangeCallbackData, view);
	}
}

void ListView::doProcessMouseState (const Widget::MouseState &mouseState) {
	bool shouldreset, found;
	std::list<ListView::Item>::iterator i, end;

	shouldreset = false;
	if (! mouseState.isEntered) {
		if (isItemFocused) {
			isItemFocused = false;
			lastFocusPanel = NULL;
			shouldreset = true;
		}
	}
	else {
		found = false;
		i = itemList.begin ();
		end = itemList.end ();
		while (i != end) {
			if ((mouseState.enterDeltaY >= i->panel->position.y) && (mouseState.enterDeltaY < (i->panel->position.y + i->panel->height))) {
				found = true;
				if ((! isItemFocused) || (i->panel != lastFocusPanel)) {
					shouldreset = true;
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
			shouldreset = true;
		}
	}

	if (shouldreset) {
		resetLayout ();
	}

	Panel::doProcessMouseState (mouseState);
}

void ListView::doRefresh () {
	UiConfiguration *uiconfig;
	float h;
	std::list<ListView::Item>::iterator i, end;

	uiconfig = &(App::getInstance ()->uiConfig);
	if (titleLabel) {
		titleLabel->setWindowWidth (viewWidth);
	}
	h = uiconfig->fonts[itemFontType]->maxLineHeight + uiconfig->paddingSize;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		i->panel->setFixedSize (true, viewWidth, h);
		++i;
	}

	Panel::doRefresh ();
}
