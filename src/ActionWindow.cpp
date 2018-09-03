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
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Util.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "ComboBox.h"
#include "TextField.h"
#include "TextFieldWindow.h"
#include "Toggle.h"
#include "UiConfiguration.h"
#include "ActionWindow.h"

ActionWindow::ActionWindow (const StdString &titleText, const StdString &confirmButtonText)
: Panel ()
, isConfirmed (false)
, closeCallback (NULL)
, closeCallbackData (NULL)
, titleLabel (NULL)
, confirmButton (NULL)
, cancelButton (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->lightPrimaryColor);

	uitext = &(App::getInstance ()->uiText);
	if (! titleText.empty ()) {
		titleLabel = (Label *) addWidget (new Label (titleText, UiConfiguration::TITLE, uiconfig->inverseTextColor));
	}

	confirmButton = (Button *) addWidget (new Button ((! confirmButtonText.empty ()) ? confirmButtonText : uitext->ok.uppercased ()));
	confirmButton->setMouseClickCallback (ActionWindow::confirmButtonClicked, this);
	confirmButton->setRaised (true, uiconfig->darkInverseBackgroundColor);
	confirmButton->setTextColor (uiconfig->raisedButtonInverseTextColor);
	confirmButton->setInverseColor (true);
	cancelButton = (Button *) addWidget (new Button (uitext->cancel.uppercased ()));
	cancelButton->setMouseClickCallback (ActionWindow::cancelButtonClicked, this);
	cancelButton->setRaised (true, uiconfig->darkInverseBackgroundColor);
	cancelButton->setTextColor (uiconfig->raisedButtonInverseTextColor);
	cancelButton->setInverseColor (true);

	resetLayout ();
}

ActionWindow::~ActionWindow () {

}

StdString ActionWindow::toStringDetail () {
	return (StdString (" ActionWindow"));
}

bool ActionWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" ActionWindow"));
}

ActionWindow *ActionWindow::castWidget (Widget *widget) {
	return (ActionWindow::isWidgetType (widget) ? (ActionWindow *) widget : NULL);
}

void ActionWindow::setCloseCallback (Widget::EventCallback callback, void *callbackData) {
	closeCallback = callback;
	closeCallbackData = callbackData;
}

void ActionWindow::addComboBoxOption (const StdString &optionName, StringList *optionItemList, const StdString &optionValue) {
	UiConfiguration *uiconfig;
	ComboBox *combobox;
	ActionWindow::Item item;
	std::list<ActionWindow::Item>::iterator curitem;

	uiconfig = &(App::getInstance ()->uiConfig);

	combobox = (ComboBox *) addWidget (new ComboBox ());
	combobox->addItems (optionItemList);
	if (! optionValue.empty ()) {
		combobox->setValue (optionValue);
	}

	curitem = findItem (optionName);
	if (curitem != itemList.end ()) {
		curitem->name.assign (optionName);
		curitem->type = ActionWindow::COMBO_BOX;
		curitem->nameLabel->setText (curitem->name);
		curitem->optionWidget->isDestroyed = true;
		curitem->optionWidget = combobox;
	}
	else {
		item.name.assign (optionName);
		item.type = ActionWindow::COMBO_BOX;
		item.nameLabel = (Label *) addWidget (new Label (item.name, UiConfiguration::CAPTION, uiconfig->inverseTextColor));
		item.optionWidget = combobox;
		itemList.push_back (item);
	}
	resetLayout ();
}

void ActionWindow::addComboBoxOption (const StdString &optionName, HashMap *optionItemMap, const StdString &optionValue) {
	UiConfiguration *uiconfig;
	ComboBox *combobox;
	ActionWindow::Item item;
	std::list<ActionWindow::Item>::iterator curitem;

	uiconfig = &(App::getInstance ()->uiConfig);

	combobox = (ComboBox *) addWidget (new ComboBox ());
	combobox->addItems (optionItemMap);
	if (! optionValue.empty ()) {
		combobox->setValue (optionValue);
	}

	curitem = findItem (optionName);
	if (curitem != itemList.end ()) {
		curitem->name.assign (optionName);
		curitem->type = ActionWindow::COMBO_BOX;
		curitem->nameLabel->setText (curitem->name);
		curitem->optionWidget->isDestroyed = true;
		curitem->optionWidget = combobox;
	}
	else {
		item.name.assign (optionName);
		item.type = ActionWindow::COMBO_BOX;
		item.nameLabel = (Label *) addWidget (new Label (item.name, UiConfiguration::CAPTION, uiconfig->inverseTextColor));
		item.optionWidget = combobox;
		itemList.push_back (item);
	}
	resetLayout ();
}

void ActionWindow::addTextFieldOption (const StdString &optionName, const StdString &promptText, const StdString &optionValue) {
	UiConfiguration *uiconfig;
	TextFieldWindow *textfield;
	ActionWindow::Item item;
	std::list<ActionWindow::Item>::iterator curitem;

	uiconfig = &(App::getInstance ()->uiConfig);

	textfield = (TextFieldWindow *) addWidget (new TextFieldWindow (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CAPTION]->maxGlyphWidth, promptText));
	textfield->setPadding (0.0f, 0.0f);
	if (! optionValue.empty ()) {
		textfield->setValue (optionValue);
	}

	curitem = findItem (optionName);
	if (curitem != itemList.end ()) {
		curitem->name.assign (optionName);
		curitem->type = ActionWindow::TEXT_FIELD;
		curitem->nameLabel->setText (curitem->name);
		curitem->optionWidget->isDestroyed = true;
		curitem->optionWidget = textfield;
	}
	else {
		item.name.assign (optionName);
		item.type = ActionWindow::TEXT_FIELD;
		item.nameLabel = (Label *) addWidget (new Label (item.name, UiConfiguration::CAPTION, uiconfig->inverseTextColor));
		item.optionWidget = textfield;
		itemList.push_back (item);
	}
	resetLayout ();
}

void ActionWindow::addToggleOption (const StdString &optionName, bool optionValue) {
	UiConfiguration *uiconfig;
	Toggle *toggle;
	ActionWindow::Item item;
	std::list<ActionWindow::Item>::iterator curitem;

	uiconfig = &(App::getInstance ()->uiConfig);

	toggle = (Toggle *) addWidget (new Toggle ());
	toggle->setChecked (optionValue);

	curitem = findItem (optionName);
	if (curitem != itemList.end ()) {
		curitem->name.assign (optionName);
		curitem->type = ActionWindow::TOGGLE;
		curitem->nameLabel->setText (curitem->name);
		curitem->optionWidget->isDestroyed = true;
		curitem->optionWidget = toggle;
	}
	else {
		item.name.assign (optionName);
		item.type = ActionWindow::TOGGLE;
		item.nameLabel = (Label *) addWidget (new Label (item.name, UiConfiguration::CAPTION, uiconfig->inverseTextColor));
		item.optionWidget = toggle;
		itemList.push_back (item);
	}
	resetLayout ();
}

StdString ActionWindow::getOptionValue (const StdString &optionName, const StdString &defaultValue) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return (defaultValue);
	}

	switch (item->type) {
		case ActionWindow::COMBO_BOX: {
			return (((ComboBox *) item->optionWidget)->getValue ());
		}
		case ActionWindow::TEXT_FIELD: {
			return (((TextFieldWindow *) item->optionWidget)->getValue ());
		}
		case ActionWindow::TOGGLE: {
			return (((Toggle *) item->optionWidget)->isChecked ? "true" : "false");
		}
	}

	return (defaultValue);
}

bool ActionWindow::getOptionValue (const StdString &optionName, bool defaultValue) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return (defaultValue);
	}

	switch (item->type) {
		case ActionWindow::TOGGLE: {
			return (((Toggle *) item->optionWidget)->isChecked);
		}
	}

	return (defaultValue);
}

StdString ActionWindow::getOptionValue (const StdString &optionName, const char *defaultValue) {
	return (getOptionValue (optionName, StdString (defaultValue)));
}

std::list<ActionWindow::Item>::iterator ActionWindow::findItem (const StdString &optionName) {
	std::list<ActionWindow::Item>::iterator i, end;
	StdString option;

	option = optionName.lowercased ();
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (option.equals (i->name.lowercased ())) {
			return (i);
		}
		++i;
	}

	return (end);
}

void ActionWindow::resetLayout () {
	UiConfiguration *uiconfig;
	std::list<ActionWindow::Item>::iterator i, end;
	float x, y, w, h;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = widthPadding;
	y = heightPadding;
	if (titleLabel) {
		titleLabel->position.assign (x, y);
		y += titleLabel->height + uiconfig->marginSize;
	}

	w = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->nameLabel->width > w) {
			w = i->nameLabel->width;
		}
		++i;
	}

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		i->nameLabel->position.assign (x + w - i->nameLabel->width, i->nameLabel->getLinePosition (y));
		i->optionWidget->position.assign (x + w + uiconfig->marginSize, y);

		h = i->nameLabel->height;
		if (i->optionWidget->height > h) {
			h = i->optionWidget->height;
			i->nameLabel->position.assignY (y + (h / 2.0f) - (i->nameLabel->height / 2.0f));
		}

		y += h + uiconfig->marginSize;
		++i;
	}

	cancelButton->position.assign (x, y);
	x += cancelButton->width + uiconfig->marginSize;
	confirmButton->position.assign (x, y);

	resetSize ();

	x = width - uiconfig->paddingSize;
	x -= confirmButton->width;
	confirmButton->position.assignX (x);
	x -= uiconfig->marginSize;

	x -= cancelButton->width;
	cancelButton->position.assignX (x);
}

void ActionWindow::confirmButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ActionWindow *window;

	window = (ActionWindow *) windowPtr;
	window->isConfirmed = true;
	if (window->closeCallback) {
		window->closeCallback (window->closeCallbackData, window);
	}
	window->isDestroyed = true;
}

void ActionWindow::cancelButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ActionWindow *window;

	window = (ActionWindow *) windowPtr;
	window->isConfirmed = false;
	if (window->closeCallback) {
		window->closeCallback (window->closeCallbackData, window);
	}
	window->isDestroyed = true;
}
