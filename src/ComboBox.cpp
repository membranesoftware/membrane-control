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
#include "App.h"
#include "Result.h"
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
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);

	normalBgColor.assign (uiconfig->lightBackgroundColor);
	normalBorderColor.assign (uiconfig->darkBackgroundColor);
	focusBgColor.assign (uiconfig->darkBackgroundColor);
	focusBorderColor.assign (uiconfig->lightBackgroundColor);
	disabledBgColor.assign (uiconfig->darkBackgroundColor);
	disabledBorderColor.assign (uiconfig->mediumBackgroundColor);
	normalItemTextColor.assign (uiconfig->lightPrimaryColor);
	focusItemTextColor.assign (uiconfig->primaryTextColor);
	disabledTextColor.assign (uiconfig->lightPrimaryTextColor);

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
	UiConfiguration *uiconfig;

	if (isInverseColor == inverse) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isInverseColor = inverse;
	if (isInverseColor) {
		normalBgColor.assign (uiconfig->darkInverseBackgroundColor);
		normalBorderColor.assign (uiconfig->lightInverseBackgroundColor);
		focusBgColor.assign (uiconfig->lightInverseBackgroundColor);
		focusBorderColor.assign (uiconfig->darkInverseBackgroundColor);
		disabledBgColor.assign (uiconfig->lightInverseBackgroundColor);
		disabledBorderColor.assign (uiconfig->darkInverseBackgroundColor);
		normalItemTextColor.assign (uiconfig->darkInverseTextColor);
		focusItemTextColor.assign (uiconfig->inverseTextColor);
		disabledTextColor.assign (uiconfig->darkInverseTextColor);
	}
	else {
		normalBgColor.assign (uiconfig->lightBackgroundColor);
		normalBorderColor.assign (uiconfig->darkBackgroundColor);
		focusBgColor.assign (uiconfig->darkBackgroundColor);
		focusBorderColor.assign (uiconfig->lightBackgroundColor);
		disabledBgColor.assign (uiconfig->darkBackgroundColor);
		disabledBorderColor.assign (uiconfig->mediumBackgroundColor);
		normalItemTextColor.assign (uiconfig->lightPrimaryColor);
		focusItemTextColor.assign (uiconfig->primaryTextColor);
		disabledTextColor.assign (uiconfig->lightPrimaryTextColor);
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
		if (valueChangeCallback.callback && (! shouldSkipChangeCallback)) {
			valueChangeCallback.callback (valueChangeCallback.callbackData, this);
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
		if (valueChangeCallback.callback && (! shouldSkipChangeCallback)) {
			valueChangeCallback.callback (valueChangeCallback.callbackData, this);
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
	UiConfiguration *uiconfig;
	ComboBox::Item item;
	LabelWindow *label;

	uiconfig = &(App::instance->uiConfig);
	label = (LabelWindow *) addWidget (new LabelWindow (new Label (Label::getTruncatedText (itemValue, UiConfiguration::CaptionFont, uiconfig->comboBoxLineLength * uiconfig->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, Label::DotTruncateSuffix), UiConfiguration::CaptionFont, normalItemTextColor)));
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
	UiConfiguration *uiconfig;
	ComboBox::Item item;
	LabelWindow *label;

	uiconfig = &(App::instance->uiConfig);
	label = (LabelWindow *) addWidget (new LabelWindow (new Label (Label::getTruncatedText (itemValue, UiConfiguration::CaptionFont, uiconfig->comboBoxLineLength * uiconfig->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, Label::DotTruncateSuffix), UiConfiguration::CaptionFont, normalItemTextColor)));
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
	UiConfiguration *uiconfig;
	std::list<ComboBox::Item>::iterator i, end;
	LabelWindow *label;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	if (isDisabled) {
		bgColor.translate (disabledBgColor, uiconfig->shortColorTranslateDuration);
		borderColor.translate (disabledBorderColor, uiconfig->shortColorTranslateDuration);
	}
	else if (isFocused) {
		bgColor.translate (focusBgColor, uiconfig->shortColorTranslateDuration);
		borderColor.translate (focusBorderColor, uiconfig->shortColorTranslateDuration);
	}
	else {
		bgColor.translate (normalBgColor, uiconfig->shortColorTranslateDuration);
		borderColor.translate (normalBorderColor, uiconfig->shortColorTranslateDuration);
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
			label->setWindowWidth (maxTextWidth + (uiconfig->paddingSize * 2.0f), true);
			label->position.assign (x, y);
			if (isDisabled) {
				label->bgColor.translate (disabledBgColor, uiconfig->shortColorTranslateDuration);
				label->borderColor.translate (disabledBorderColor, uiconfig->shortColorTranslateDuration);
				label->translateTextColor (disabledTextColor, uiconfig->shortColorTranslateDuration);
			}
			else if (isFocused) {
				label->bgColor.translate (focusBgColor, uiconfig->shortColorTranslateDuration);
				label->borderColor.translate (focusBorderColor, uiconfig->shortColorTranslateDuration);
				label->translateTextColor (focusItemTextColor, uiconfig->shortColorTranslateDuration);
			}
			else {
				label->bgColor.translate (normalBgColor, uiconfig->shortColorTranslateDuration);
				label->borderColor.translate (normalBorderColor, uiconfig->shortColorTranslateDuration);
				label->translateTextColor (normalItemTextColor, uiconfig->shortColorTranslateDuration);
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
				x = App::instance->input.mouseX;
				y = App::instance->input.mouseY;
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
	UiConfiguration *uiconfig;
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

	uiconfig = &(App::instance->uiConfig);
	scrollview = new ScrollView ();
	scrollview->setFillBg (true, normalBgColor);
	scrollview->setBorder (true, normalBorderColor);
	scrollview->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);

	count = 0;
	x = 0.0f;
	y = 0.0f;
	w = maxTextWidth + (uiconfig->paddingSize * 2.0f);
	h = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->label != selectedItemLabel) {
			label = (LabelWindow *) scrollview->addWidget (new LabelWindow (new Label (i->label->getText (), UiConfiguration::CaptionFont, normalItemTextColor)), x, y);
			label->mouseClickCallback = Widget::EventCallbackContext (ComboBox::expandItemClicked, this);
			label->setFillBg (true, normalBgColor);
			label->setWindowWidth (w);
			label->setMouseoverHighlight (true, normalItemTextColor, normalBgColor, focusItemTextColor, focusBgColor, uiconfig->shortColorTranslateDuration);
			y += label->height;
			if (count < uiconfig->comboBoxExpandViewItems) {
				h = y;
			}
			++count;
		}

		++i;
	}

	scrollview->setViewSize (w, h);
	scrollview->setVerticalScrollBounds (0.0f, y - h);
	if (count > uiconfig->comboBoxExpandViewItems) {
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
		if (valueChangeCallback.callback && (! shouldSkipChangeCallback)) {
			valueChangeCallback.callback (valueChangeCallback.callbackData, this);
		}
	}
}
