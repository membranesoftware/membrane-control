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
#include <list>
#include "ClassId.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Sprite.h"
#include "Label.h"
#include "Image.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "Menu.h"

Menu::Menu ()
: Panel ()
, isClickDestroyEnabled (false)
, selectionMarginSize (0.0f)
, focusBackgroundPanel (NULL)
, itemHeight (0.0f)
, isItemFocused (false)
, lastFocusPanel (NULL)
{
	classId = ClassId::Menu;

	setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);

	focusBackgroundPanel = (Panel *) addWidget (new Panel ());
	focusBackgroundPanel->zLevel = Widget::MinZLevel;
	focusBackgroundPanel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	focusBackgroundPanel->isVisible = false;
}

Menu::~Menu () {

}

bool Menu::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::Menu));
}

Menu *Menu::castWidget (Widget *widget) {
	return (Menu::isWidgetType (widget) ? (Menu *) widget : NULL);
}

void Menu::addItem (const StdString &name, Sprite *sprite, const Widget::EventCallbackContext &callback, int selectionGroup, bool isSelected) {
	std::list<Menu::Item>::iterator i, end;
	Menu::Item item;
	Panel *panel;
	Image *image;
	float w;

	item.isChoice = true;
	panel = new Panel ();
	panel->zLevel = 1;
	addWidget (panel);
	item.panel = panel;

	item.label = (Label *) panel->addWidget (new Label (name, UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));
	if (sprite) {
		item.image = (Image *) panel->addWidget (new Image (sprite, UiConfiguration::BlackButtonFrame));
	}
	item.callback = callback;

	item.selectionGroup = selectionGroup;
	if (item.selectionGroup >= 0) {
		item.isSelected = isSelected;
		if (checkmarkImageMap.count (item.selectionGroup) <= 0) {
			image = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::CheckmarkSprite)));
			image->isVisible = false;
			image->zLevel = panel->zLevel + 1;
			w = image->width + (UiConfiguration::instance->paddingSize / 2.0f);
			if (selectionMarginSize < w) {
				selectionMarginSize = w;
			}
			checkmarkImageMap.insert (std::pair<int, Image *> (item.selectionGroup, image));
		}
	}

	if (item.isSelected) {
		unselectItemGroup (item.selectionGroup);
	}

	itemList.push_back (item);
	refreshLayout ();
}

void Menu::addDivider () {
	Menu::Item item;
	Panel *panel;

	item.isDivider = true;
	panel = new Panel ();
	panel->setFillBg (true, UiConfiguration::instance->primaryTextColor);
	panel->setFixedSize (true, width, UiConfiguration::instance->menuDividerLineWidth);
	addWidget (panel);

	item.panel = panel;

	itemList.push_back (item);
	refreshLayout ();
}

bool Menu::doProcessMouseState (const Widget::MouseState &mouseState) {
	std::list<Menu::Item>::iterator i, end;
	bool shouldrefresh;

	shouldrefresh = false;
	if (! mouseState.isEntered) {
		if (isItemFocused) {
			isItemFocused = false;
			lastFocusPanel = NULL;
			shouldrefresh = true;
		}
	}
	else {
		i = itemList.begin ();
		end = itemList.end ();
		while (i != end) {
			if (i->isChoice) {
				if ((mouseState.enterDeltaY >= i->panel->position.y) && (mouseState.enterDeltaY < (i->panel->position.y + i->panel->height))) {
					if ((! isItemFocused) || (i->panel != lastFocusPanel)) {
						isItemFocused = true;
						lastFocusPanel = i->panel;
						focusBackgroundPanel->position.assign (i->panel->position);
						focusBackgroundPanel->setFixedSize (true, i->panel->width, i->panel->height);
						shouldrefresh = true;
					}

					if (mouseState.isLeftClickReleased && mouseState.isLeftClickEntered) {
						if ((i->selectionGroup >= 0) && (! i->isSelected)) {
							unselectItemGroup (i->selectionGroup);
							i->isSelected = true;
							shouldrefresh = true;
						}
						eventCallback (i->callback);
					}
					break;
				}
			}
			++i;
		}
	}

	if (isClickDestroyEnabled) {
		if (mouseState.isLeftClickReleased) {
			isDestroyed = true;
		}
	}

	if (shouldrefresh && (! isDestroyed)) {
		refreshLayout ();
	}

	return (false);
}

void Menu::refreshLayout () {
	std::list<Menu::Item>::iterator i, end;
	std::map<int, Image *>::iterator mi, mend;
	std::map<int, bool> checkfoundmap;
	Label *label;
	Image *image;
	Panel *panel;
	float x0, x, y, w, h, maxw, maxh, maximagew, padw, padh;

	x0 = UiConfiguration::instance->paddingSize + selectionMarginSize;
	maxw = 0.0f;
	maxh = 0.0f;
	maximagew = 0.0f;
	padw = UiConfiguration::instance->paddingSize;
	padh = UiConfiguration::instance->paddingSize;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->isChoice) {
			image = i->image;
			if (image && (image->width > maximagew)) {
				maximagew = image->width;
			}
		}
		++i;
	}

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->isChoice) {
			label = i->label;
			image = i->image;

			w = label->width;
			if (image) {
				w += UiConfiguration::instance->marginSize + maximagew;
			}
			h = label->height;
			if (image && (image->height > h)) {
				h = image->height;
			}
			if (w > maxw) {
				maxw = w;
			}
			if (h > maxh) {
				maxh = h;
			}
		}

		++i;
	}

	maxw += UiConfiguration::instance->paddingSize + selectionMarginSize;
	maxh += UiConfiguration::instance->paddingSize;
	itemHeight = maxh;

	y = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->isChoice) {
			label = i->label;
			image = i->image;
			panel = i->panel;
			x = x0;
			if (image) {
				image->position.assign (x, (maxh / 2.0f) - (image->height / 2.0f));
				x += maximagew + UiConfiguration::instance->marginSize;
			}
			label->position.assign (x, (maxh / 2.0f) - (label->height / 2.0f));
			panel->setFixedSize (true, maxw, maxh);
			panel->position.assign (0.0f, y);

			if (i->selectionGroup >= 0) {
				if (i->isSelected) {
					mi = checkmarkImageMap.find (i->selectionGroup);
					if (mi != checkmarkImageMap.end ()) {
						checkfoundmap.insert (std::pair<int, bool> (i->selectionGroup, true));
						mi->second->position.assign (UiConfiguration::instance->paddingSize / 2.0f, y + (maxh / 2.0f) - (mi->second->height / 2.0f));
					}
				}
			}

			y += panel->height;
			padh = 0.0f;
		}
		else if (i->isDivider) {
			panel = i->panel;
			y += (UiConfiguration::instance->marginSize / 2.0f);
			panel->position.assign (0.0f, y);
			panel->setFixedSize (true, maxw + (UiConfiguration::instance->paddingSize * 2.0f), UiConfiguration::instance->menuDividerLineWidth);
			y += panel->height + (UiConfiguration::instance->marginSize / 2.0f);
			padw = 0.0f;
			padh = UiConfiguration::instance->paddingSize;
		}
		++i;
	}

	mi = checkmarkImageMap.begin ();
	mend = checkmarkImageMap.end ();
	while (mi != mend) {
		image = mi->second;
		if (checkfoundmap.count (mi->first) <= 0) {
			image->isVisible = false;
		}
		else {
			image->isVisible = true;
		}
		++mi;
	}

	focusBackgroundPanel->isVisible = false;
	resetSize ();

	w = maxWidgetX + padw;
	h = maxWidgetY + padh;
	if (! isItemFocused) {
		focusBackgroundPanel->isVisible = false;
	}
	else {
		focusBackgroundPanel->setFixedSize (true, w, focusBackgroundPanel->height);
		focusBackgroundPanel->isVisible = true;
	}
	setFixedSize (true, w, h);
}

void Menu::unselectItemGroup (int selectionGroup) {
	std::list<Menu::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->selectionGroup == selectionGroup) {
			i->isSelected = false;
		}
		++i;
	}
}
