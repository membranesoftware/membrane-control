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
#include "UiConfiguration.h"
#include "ComboBox.h"

ComboBox::ComboBox ()
: Panel ()
, isDisabled (false)
, isInverseColor (false)
, hasItemData (false)
, valueChangeCallback (NULL)
, valueChangeCallbackData (NULL)
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
	normalBorderColor.assign (uiconfig->mediumBackgroundColor);
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
	expandPanel.destroyAndClear ();
}

void ComboBox::setValueChangeCallback (Widget::EventCallback callback, void *callbackData) {
	valueChangeCallback = callback;
	valueChangeCallbackData = callbackData;
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
		normalBorderColor.assign (uiconfig->mediumInverseBackgroundColor);
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
		normalBorderColor.assign (uiconfig->mediumBackgroundColor);
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
		if (valueChangeCallback && (! shouldSkipChangeCallback)) {
			valueChangeCallback (valueChangeCallbackData, this);
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
		if (valueChangeCallback && (! shouldSkipChangeCallback)) {
			valueChangeCallback (valueChangeCallbackData, this);
		}
	}
}

StdString ComboBox::getValue () {
	if (hasItemData) {
		return (selectedItemData);
	}

	return (selectedItemValue);
}

void ComboBox::addItem (const StdString &itemValue, const StdString &itemData) {
	ComboBox::Item item;
	LabelWindow *labelwindow;
	Label *label;

	label = new Label (itemValue, UiConfiguration::CaptionFont, normalItemTextColor);
	if (label->width > maxTextWidth) {
		maxTextWidth = label->width;
	}

	labelwindow = (LabelWindow *) addWidget (new LabelWindow (label));
	labelwindow->setFillBg (true, normalBgColor);
	labelwindow->setBorder (true, normalBorderColor);
	if (! selectedItemLabel) {
		selectedItemLabel = labelwindow;
		selectedItemValue.assign (itemValue);
		selectedItemData.assign (itemData);
	}

	item.value.assign (itemValue);
	item.itemData.assign (itemData);
	item.label = labelwindow;
	itemList.push_back (item);

	if (! itemData.empty ()) {
		hasItemData = true;
	}
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
	bool show;

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
		label->setWindowWidth (maxTextWidth + (uiconfig->paddingSize * 2.0f), true);

		show = false;
		if (label == selectedItemLabel) {
			show = true;
		}

		if (show) {
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
		else {
			label->isVisible = false;
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

void ComboBox::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel *panel;
	bool shouldunexpand;
	int x, y, x1, x2, y1, y2;

	if (isDisabled) {
		return;
	}

	if (isExpanded) {
		setFocused (false);
		if (mouseState.isLeftClicked) {
			shouldunexpand = false;

			if (mouseState.isEntered) {
				shouldunexpand = true;
			}

			panel = (Panel *) expandPanel.widget;
			if ((! shouldunexpand) && panel) {
				x = App::instance->input.mouseX;
				y = App::instance->input.mouseY;
				x1 = (int) panel->screenX;
				y1 = (int) panel->screenY;
				x2 = x1 + (int) panel->width;
				y2 = y1 + (int) panel->height;
				if ((x < x1) || (x > x2) || (y < y1) || (y > y2)) {
					shouldunexpand = true;
				}
			}

			if (shouldunexpand) {
				unexpand ();
			}
		}
		return;
	}

	setFocused (mouseState.isEntered);
	if (mouseState.isEntered && mouseState.isLeftClicked) {
		expand ();
	}
}

void ComboBox::expand () {
	UiConfiguration *uiconfig;
	Panel *panel;
	std::list<ComboBox::Item>::iterator i, end;
	LabelWindow *label;
	float x, y;

	if (isExpanded || isDisabled) {
		return;
	}

	if (itemList.size () < 2) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	panel = new Panel ();
	panel->setFillBg (true, normalBgColor);
	panel->setBorder (true, normalBorderColor);

	// TODO: Add scrolling for cases when there are many options to choose from

	x = 0.0f;
	y = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->label != selectedItemLabel) {
			label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (i->value, UiConfiguration::CaptionFont, normalItemTextColor)), x, y);
			label->setFillBg (true, normalBgColor);
			label->setWindowWidth (maxTextWidth + (uiconfig->paddingSize * 2.0f), true);
			label->setMouseoverHighlight (true, normalItemTextColor, normalBgColor, focusItemTextColor, focusBgColor, uiconfig->shortColorTranslateDuration);
			label->setMouseClickCallback (ComboBox::expandItemClicked, this);
			y += label->height;
		}

		++i;
	}
	panel->refresh ();
	expandPanel.assign (panel);
	expandScreenX = screenX;
	expandScreenY = screenY;

	y = expandScreenY + height;
	if ((y + panel->height) >= App::instance->windowHeight) {
		y = expandScreenY - panel->height;
	}
	App::instance->rootPanel->addWidget (panel, expandScreenX, y, App::instance->rootPanel->maxWidgetZLevel + 1);

	isExpanded = true;
}

void ComboBox::unexpand (const StdString &value) {
	if (! value.empty ()) {
		setValue (value);
	}
	isExpanded = false;
	expandPanel.destroyAndClear ();
}

void ComboBox::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (isExpanded) {
		expandPanel.compact ();
		if ((! expandPanel.widget) || (! FLOAT_EQUALS (screenX, expandScreenX)) || (! FLOAT_EQUALS (screenY, expandScreenY))) {
			unexpand ();
		}
	}
}

void ComboBox::expandItemClicked (void *comboBoxPtr, Widget *labelWindowPtr) {
	LabelWindow *label;
	ComboBox *combobox;

	combobox = (ComboBox *) comboBoxPtr;
	label = (LabelWindow *) labelWindowPtr;

	combobox->unexpand (label->getText ());
}
