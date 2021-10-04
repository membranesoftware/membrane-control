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
#include "Config.h"
#include <stdlib.h>
#include <math.h>
#include "App.h"
#include "Log.h"
#include "StdString.h"
#include "Ui.h"
#include "Input.h"
#include "Sprite.h"
#include "Widget.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Panel.h"
#include "ScrollView.h"
#include "ScrollBar.h"
#include "UiConfiguration.h"
#include "ComboBox.h"

ComboBox::ComboBox ()
: Panel ()
, isDisabled (false)
, isInverseColor (false)
, hasItemData (false)
, isExpanded (false)
, expandScreenX (0.0f)
, expandScreenY (0.0f)
, isFocused (false)
, selectedItemLabel (NULL)
, maxTextWidth (0.0f)
{
	normalBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
	normalBorderColor.assign (UiConfiguration::instance->darkBackgroundColor);
	focusBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
	focusBorderColor.assign (UiConfiguration::instance->lightBackgroundColor);
	disabledBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
	disabledBorderColor.assign (UiConfiguration::instance->mediumBackgroundColor);
	normalItemTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
	focusItemTextColor.assign (UiConfiguration::instance->primaryTextColor);
	disabledTextColor.assign (UiConfiguration::instance->lightPrimaryTextColor);

	setFillBg (true, normalBgColor);
	setBorder (true, normalBorderColor);
}

ComboBox::~ComboBox () {
	expandView.destroyAndClear ();
}

void ComboBox::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}

	isDisabled = disabled;
	if (isDisabled) {
		setFocused (false);
		unexpand ();
	}
	refreshLayout ();
}

void ComboBox::setInverseColor (bool inverse) {
	std::list<ComboBox::Item>::iterator i, end;

	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	if (isInverseColor) {
		normalBgColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		normalBorderColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		focusBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		focusBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		disabledBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		disabledBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		normalItemTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		focusItemTextColor.assign (UiConfiguration::instance->inverseTextColor);
		disabledTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
	}
	else {
		normalBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
		normalBorderColor.assign (UiConfiguration::instance->darkBackgroundColor);
		focusBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
		focusBorderColor.assign (UiConfiguration::instance->lightBackgroundColor);
		disabledBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
		disabledBorderColor.assign (UiConfiguration::instance->mediumBackgroundColor);
		normalItemTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
		focusItemTextColor.assign (UiConfiguration::instance->primaryTextColor);
		disabledTextColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
	}

	bgColor.assign (normalBgColor);
	borderColor.assign (normalBorderColor);
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		i->label->setTextColor (normalItemTextColor);
		i->label->setFillBg (true, normalBgColor);
		i->label->setBorder (true, normalBorderColor);
		++i;
	}

	refreshLayout ();
}

void ComboBox::setValue (const StdString &value, bool shouldSkipChangeCallback) {
	std::list<ComboBox::Item>::iterator i, end;
	bool found;

	found = false;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->value.equals (value)) {
			selectedItemLabel = i->label;
			selectedItemValue.assign (i->value);
			selectedItemData.assign (i->itemData);
			found = true;
			break;
		}
		++i;
	}

	if (found) {
		refreshLayout ();
		if (! shouldSkipChangeCallback) {
			eventCallback (valueChangeCallback);
		}
	}
}

void ComboBox::setValueByItemData (const StdString &itemData, bool shouldSkipChangeCallback) {
	std::list<ComboBox::Item>::iterator i, end;
	bool found;

	if (! hasItemData) {
		return;
	}

	found = false;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->itemData.equals (itemData)) {
			selectedItemLabel = i->label;
			selectedItemValue.assign (i->value);
			selectedItemData.assign (i->itemData);
			found = true;
			break;
		}
		++i;
	}

	if (found) {
		refreshLayout ();
		if (! shouldSkipChangeCallback) {
			eventCallback (valueChangeCallback);
		}
	}
}

StdString ComboBox::getValue () {
	if (hasItemData) {
		return (selectedItemData);
	}
	return (selectedItemValue);
}

void ComboBox::addItem (const StdString &itemValue) {
	ComboBox::Item item;
	LabelWindow *label;

	label = (LabelWindow *) addWidget (new LabelWindow (new Label (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (itemValue, UiConfiguration::instance->comboBoxLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, Font::DotTruncateSuffix), UiConfiguration::CaptionFont, normalItemTextColor)));
	label->setFillBg (true, normalBgColor);
	label->setBorder (true, normalBorderColor);
	if (label->width > maxTextWidth) {
		maxTextWidth = label->width;
	}
	if (! selectedItemLabel) {
		selectedItemLabel = label;
		selectedItemValue.assign (itemValue);
		selectedItemData.assign ("");
	}

	item.value.assign (itemValue);
	item.itemData.assign ("");
	item.label = label;
	itemList.push_back (item);
	refreshLayout ();
}

void ComboBox::addItem (const StdString &itemValue, const StdString &itemData) {
	ComboBox::Item item;
	LabelWindow *label;

	label = (LabelWindow *) addWidget (new LabelWindow (new Label (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (itemValue, UiConfiguration::instance->comboBoxLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, Font::DotTruncateSuffix), UiConfiguration::CaptionFont, normalItemTextColor)));
	label->setFillBg (true, normalBgColor);
	label->setBorder (true, normalBorderColor);
	if (label->width > maxTextWidth) {
		maxTextWidth = label->width;
	}
	if (! selectedItemLabel) {
		selectedItemLabel = label;
		selectedItemValue.assign (itemValue);
		selectedItemData.assign (itemData);
	}

	item.value.assign (itemValue);
	item.itemData.assign (itemData);
	item.label = label;
	itemList.push_back (item);
	hasItemData = true;
	refreshLayout ();
}

void ComboBox::addItems (StringList *nameList) {
	StringList::iterator i, end;

	i = nameList->begin ();
	end = nameList->end ();
	while (i != end) {
		addItem (*i);
		++i;
	}
}

void ComboBox::addItems (HashMap *itemMap) {
	HashMap::Iterator i;
	StdString key, value;

	i = itemMap->begin ();
	while (itemMap->hasNext (&i)) {
		key = itemMap->next (&i);
		value = itemMap->find (key, StdString (""));
		addItem (key, value);
	}
}

void ComboBox::refreshLayout () {
	std::list<ComboBox::Item>::iterator i, end;
	LabelWindow *label;
	float x, y;

	if (isDisabled) {
		bgColor.translate (disabledBgColor, UiConfiguration::instance->shortColorTranslateDuration);
		borderColor.translate (disabledBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else if (isFocused) {
		bgColor.translate (focusBgColor, UiConfiguration::instance->shortColorTranslateDuration);
		borderColor.translate (focusBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else {
		bgColor.translate (normalBgColor, UiConfiguration::instance->shortColorTranslateDuration);
		borderColor.translate (normalBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
	}

	x = 0.0f;
	y = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		label = i->label;
		if (label != selectedItemLabel) {
			label->isVisible = false;
		}
		else {
			label->setWindowWidth (maxTextWidth + (UiConfiguration::instance->paddingSize * 2.0f), true);
			label->position.assign (x, y);
			if (isDisabled) {
				label->bgColor.translate (disabledBgColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->borderColor.translate (disabledBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->translateTextColor (disabledTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			}
			else if (isFocused) {
				label->bgColor.translate (focusBgColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->borderColor.translate (focusBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->translateTextColor (focusItemTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			}
			else {
				label->bgColor.translate (normalBgColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->borderColor.translate (normalBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->translateTextColor (normalItemTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			}
			y += label->height;
			label->isVisible = true;
		}
		++i;
	}

	resetSize ();
}

void ComboBox::setFocused (bool focused) {
	if (isFocused == focused) {
		return;
	}
	isFocused = focused;
	refreshLayout ();
}

bool ComboBox::doProcessMouseState (const Widget::MouseState &mouseState) {
	ScrollView *scrollview;
	bool shouldunexpand;
	int x, y, x1, x2, y1, y2;

	if (isDisabled) {
		return (false);
	}
	if (isExpanded) {
		setFocused (false);
		if (mouseState.isLeftClicked) {
			shouldunexpand = false;

			if (mouseState.isEntered) {
				shouldunexpand = true;
			}

			scrollview = (ScrollView *) expandView.widget;
			if ((! shouldunexpand) && scrollview) {
				x = Input::instance->mouseX;
				y = Input::instance->mouseY;
				x1 = (int) scrollview->screenX;
				y1 = (int) scrollview->screenY;
				x2 = x1 + (int) scrollview->width;
				y2 = y1 + (int) scrollview->height;
				if ((x < x1) || (x > x2) || (y < y1) || (y > y2)) {
					shouldunexpand = true;
				}
			}

			if (shouldunexpand) {
				unexpand ();
			}
		}
		return (false);
	}

	setFocused (mouseState.isEntered);
	if (mouseState.isEntered && mouseState.isLeftClicked) {
		expand ();
	}
	return (false);
}

void ComboBox::expand () {
	std::list<ComboBox::Item>::iterator i, end;
	ScrollView *scrollview;
	ScrollBar *scrollbar;
	LabelWindow *label;
	float x, y, w, h;
	int count;

	if (isExpanded || isDisabled) {
		return;
	}

	if (itemList.size () < 2) {
		return;
	}

	scrollview = new ScrollView ();
	scrollview->setFillBg (true, normalBgColor);
	scrollview->setBorder (true, normalBorderColor);
	scrollview->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);

	count = 0;
	x = 0.0f;
	y = 0.0f;
	w = maxTextWidth + (UiConfiguration::instance->paddingSize * 2.0f);
	h = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->label != selectedItemLabel) {
			label = (LabelWindow *) scrollview->addWidget (new LabelWindow (new Label (i->label->getText (), UiConfiguration::CaptionFont, normalItemTextColor)), x, y);
			label->mouseClickCallback = Widget::EventCallbackContext (ComboBox::expandItemClicked, this);
			label->setFillBg (true, normalBgColor);
			label->setWindowWidth (w);
			label->setMouseoverHighlight (true, normalItemTextColor, normalBgColor, focusItemTextColor, focusBgColor, UiConfiguration::instance->shortColorTranslateDuration);
			y += label->height;
			if (count < UiConfiguration::instance->comboBoxExpandViewItems) {
				h = y;
			}
			++count;
		}

		++i;
	}

	scrollview->setViewSize (w, h);
	scrollview->setVerticalScrollBounds (0.0f, y - h);
	if (count > UiConfiguration::instance->comboBoxExpandViewItems) {
		scrollview->isMouseWheelScrollEnabled = true;
		scrollbar = (ScrollBar *) scrollview->addWidget (new ScrollBar (h));
		scrollbar->positionChangeCallback = Widget::EventCallbackContext (ComboBox::scrollBarPositionChanged, this);
		scrollbar->updateCallback = Widget::UpdateCallbackContext (ComboBox::scrollBarUpdated, this);
		scrollbar->zLevel = 1;
		scrollbar->setScrollBounds (h, y);
		scrollbar->position.assign (w - scrollbar->width, 0.0f);
	}
	scrollview->refresh ();
	expandView.assign (scrollview);
	expandScreenX = screenX;
	expandScreenY = screenY;

	x = screenX;
	y = screenY + height;
	if ((y + scrollview->height) >= App::instance->windowHeight) {
		y = screenY - scrollview->height;
	}
	App::instance->rootPanel->addWidget (scrollview, x, y, App::instance->rootPanel->maxWidgetZLevel + 1);
	isExpanded = true;
}

void ComboBox::scrollBarPositionChanged (void *comboBoxPtr, Widget *widgetPtr) {
	ComboBox *combobox;
	ScrollBar *scrollbar;
	ScrollView *scrollview;

	combobox = (ComboBox *) comboBoxPtr;
	scrollbar = (ScrollBar *) widgetPtr;
	scrollview = (ScrollView *) combobox->expandView.widget;
	if (! scrollview) {
		return;
	}

	scrollview->setViewOrigin (0.0f, scrollbar->scrollPosition);
	scrollbar->position.assignY (scrollview->viewOriginY);
}

void ComboBox::scrollBarUpdated (void *comboBoxPtr, int msElapsed, Widget *widgetPtr) {
	ComboBox *combobox;
	ScrollBar *scrollbar;
	ScrollView *scrollview;

	combobox = (ComboBox *) comboBoxPtr;
	scrollbar = (ScrollBar *) widgetPtr;
	scrollview = (ScrollView *) combobox->expandView.widget;
	if (! scrollview) {
		return;
	}

	scrollbar->setPosition (scrollview->viewOriginY, true);
	scrollbar->position.assignY (scrollview->viewOriginY);
}

void ComboBox::unexpand () {
	isExpanded = false;
	expandView.destroyAndClear ();
}

void ComboBox::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (isExpanded) {
		expandView.compact ();
		if ((! expandView.widget) || (! FLOAT_EQUALS (screenX, expandScreenX)) || (! FLOAT_EQUALS (screenY, expandScreenY))) {
			unexpand ();
		}
	}
}

void ComboBox::expandItemClicked (void *comboBoxPtr, Widget *widgetPtr) {
	LabelWindow *label;
	ComboBox *combobox;

	combobox = (ComboBox *) comboBoxPtr;
	label = (LabelWindow *) widgetPtr;

	combobox->setValue (label);
	combobox->unexpand ();
}

void ComboBox::setValue (LabelWindow *choiceLabel, bool shouldSkipChangeCallback) {
	StdString choicetext;
	std::list<ComboBox::Item>::iterator i, end;
	bool found;

	// TODO: Fix incorrect item choice in cases where multiple item labels hold the same text (i.e. similar item values truncated to identical choice label text)

	choicetext = choiceLabel->getText ();
	found = false;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (choicetext.equals (i->label->getText ())) {
			selectedItemLabel = i->label;
			selectedItemValue.assign (i->value);
			selectedItemData.assign (i->itemData);
			found = true;
			break;
		}
		++i;
	}
	if (found) {
		refreshLayout ();
		if (! shouldSkipChangeCallback) {
			eventCallback (valueChangeCallback);
		}
	}
}
