/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
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
#include "SliderWindow.h"
#include "UiConfiguration.h"
#include "ActionWindow.h"

ActionWindow::ActionWindow ()
: Panel ()
, isOptionDataValid (false)
, isConfirmed (false)
, isInverseColor (false)
, closeCallback (NULL)
, closeCallbackData (NULL)
, optionChangeCallback (NULL)
, optionChangeCallbackData (NULL)
, titleLabel (NULL)
, confirmButton (NULL)
, cancelButton (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->lightBackgroundColor);

	titleLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::TITLE, uiconfig->primaryTextColor));
	titleLabel->isVisible = false;

	confirmButton = (Button *) addWidget (new Button (uitext->getText (UiTextString::ok).uppercased ()));
	confirmButton->setMouseClickCallback (ActionWindow::confirmButtonClicked, this);
	confirmButton->setTextColor (uiconfig->raisedButtonTextColor);
	confirmButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);

	cancelButton = (Button *) addWidget (new Button (uitext->getText (UiTextString::cancel).uppercased ()));
	cancelButton->setMouseClickCallback (ActionWindow::cancelButtonClicked, this);
	cancelButton->setTextColor (uiconfig->raisedButtonTextColor);
	cancelButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);

	refreshLayout ();
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

void ActionWindow::setButtonsVisible (bool visible) {
	cancelButton->isVisible = visible;
	confirmButton->isVisible = visible;
	refreshLayout ();
}

void ActionWindow::setTitleText (const StdString &text) {
	titleLabel->setText (text);
	if (text.empty ()) {
		titleLabel->isVisible = false;
	}
	else {
		titleLabel->isVisible = true;
	}
	refreshLayout ();
}

void ActionWindow::setConfirmButtonText (const StdString &text) {
	confirmButton->setText (text);
	refreshLayout ();
}

void ActionWindow::setInverseColor (bool inverse) {
	UiConfiguration *uiconfig;
	std::list<ActionWindow::Item>::iterator i, end;

	if (isInverseColor == inverse) {
		return;
	}
	uiconfig = &(App::instance->uiConfig);
	isInverseColor = inverse;
	if (isInverseColor) {
		if (isFilledBg) {
			setFillBg (true, uiconfig->lightPrimaryColor);
		}

		titleLabel->textColor.assign (uiconfig->inverseTextColor);

		confirmButton->setInverseColor (true);
		confirmButton->setTextColor (uiconfig->raisedButtonInverseTextColor);
		confirmButton->setRaised (true, uiconfig->darkInverseBackgroundColor);

		cancelButton->setInverseColor (true);
		cancelButton->setTextColor (uiconfig->raisedButtonInverseTextColor);
		cancelButton->setRaised (true, uiconfig->darkInverseBackgroundColor);
	}
	else {
		if (isFilledBg) {
			setFillBg (true, uiconfig->lightBackgroundColor);
		}

		titleLabel->textColor.assign (uiconfig->primaryTextColor);

		confirmButton->setInverseColor (false);
		confirmButton->setTextColor (uiconfig->raisedButtonTextColor);
		confirmButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);

		cancelButton->setInverseColor (false);
		cancelButton->setTextColor (uiconfig->raisedButtonTextColor);
		cancelButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	}

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		i->nameLabel->textColor.assign (isInverseColor ? uiconfig->inverseTextColor : uiconfig->primaryTextColor);
		switch (i->type) {
			case ActionWindow::COMBO_BOX: {
				((ComboBox *) i->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
			case ActionWindow::TEXT_FIELD: {
				((TextField *) i->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
			case ActionWindow::TEXT_FIELD_WINDOW: {
				((TextFieldWindow *) i->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
			case ActionWindow::TOGGLE: {
				((Toggle *) i->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
		}
		++i;
	}
}

void ActionWindow::setOptionChangeCallback (Widget::EventCallback callback, void *callbackData) {
	optionChangeCallback = callback;
	optionChangeCallbackData = callbackData;
}

void ActionWindow::setCloseCallback (Widget::EventCallback callback, void *callbackData) {
	closeCallback = callback;
	closeCallbackData = callbackData;
}

void ActionWindow::addOption (const StdString &optionName, ComboBox *comboBox, const StdString &descriptionText) {
	comboBox->setValueChangeCallback (ActionWindow::optionValueChanged, this);
	comboBox->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::COMBO_BOX, optionName, comboBox, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, TextField *textField, const StdString &descriptionText) {
	textField->setValueChangeCallback (ActionWindow::optionValueChanged, this);
	textField->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::TEXT_FIELD, optionName, textField, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, TextFieldWindow *textFieldWindow, const StdString &descriptionText) {
	textFieldWindow->setEditCallback (ActionWindow::optionValueChanged, this);
	textFieldWindow->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::TEXT_FIELD_WINDOW, optionName, textFieldWindow, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, Toggle *toggle, const StdString &descriptionText) {
	toggle->setStateChangeCallback (ActionWindow::optionValueChanged, this);
	toggle->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::TOGGLE, optionName, toggle, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, SliderWindow *slider, const StdString &descriptionText) {
	slider->setValueChangeCallback (ActionWindow::optionValueChanged, this);
	slider->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::SLIDER, optionName, slider, descriptionText);
}

void ActionWindow::doAddOption (int itemType, const StdString &optionName, Widget *optionWidget, const StdString &descriptionText) {
	UiConfiguration *uiconfig;
	std::list<ActionWindow::Item>::iterator item;

	uiconfig = &(App::instance->uiConfig);
	addWidget (optionWidget);
	item = findItem (optionName, true);
	if (item != itemList.end ()) {
		item->type = itemType;
		if (! item->nameLabel) {
			item->nameLabel = (Label *) addWidget (new Label (item->name, UiConfiguration::CAPTION, isInverseColor ? uiconfig->inverseTextColor : uiconfig->primaryTextColor));
		}
		else {
			item->nameLabel->setText (item->name);
		}

		if (item->descriptionText) {
			item->descriptionText->isDestroyed = true;
			item->descriptionText = NULL;
		}
		if (! descriptionText.empty ()) {
			item->descriptionText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, isInverseColor ? uiconfig->darkInverseTextColor : uiconfig->lightPrimaryTextColor, 0, uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CAPTION]->maxGlyphWidth));
			item->descriptionText->setText (descriptionText);
		}

		if (item->optionWidget) {
			item->optionWidget->isDestroyed = true;
		}
		item->optionWidget = optionWidget;
	}
	verifyOptions ();
	refreshLayout ();
}

StdString ActionWindow::getStringValue (const StdString &optionName, const StdString &defaultValue) {
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
			return (((TextField *) item->optionWidget)->getValue ());
		}
		case ActionWindow::TEXT_FIELD_WINDOW: {
			return (((TextFieldWindow *) item->optionWidget)->getValue ());
		}
		case ActionWindow::TOGGLE: {
			return (((Toggle *) item->optionWidget)->isChecked ? "true" : "false");
		}
	}

	return (defaultValue);
}

StdString ActionWindow::getStringValue (const StdString &optionName, const char *defaultValue) {
	return (getStringValue (optionName, StdString (defaultValue)));
}

int ActionWindow::getNumberValue (const StdString &optionName, int defaultValue) {
	std::list<ActionWindow::Item>::iterator item;
	StdString s;
	float val;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return (defaultValue);
	}

	switch (item->type) {
		case ActionWindow::COMBO_BOX: {
			s = ((ComboBox *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return ((int) val);
			}
			break;
		}
		case ActionWindow::TEXT_FIELD: {
			s = ((TextField *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return ((int) val);
			}
			break;
		}
		case ActionWindow::TEXT_FIELD_WINDOW: {
			s = ((TextFieldWindow *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return ((int) val);
			}
			break;
		}
		case ActionWindow::SLIDER: {
			return ((int) ((SliderWindow *) item->optionWidget)->value);
		}
	}

	return (defaultValue);
}

float ActionWindow::getNumberValue (const StdString &optionName, float defaultValue) {
	std::list<ActionWindow::Item>::iterator item;
	StdString s;
	float val;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return (defaultValue);
	}

	switch (item->type) {
		case ActionWindow::COMBO_BOX: {
			s = ((ComboBox *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return (val);
			}
			break;
		}
		case ActionWindow::TEXT_FIELD: {
			s = ((TextField *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return (val);
			}
			break;
		}
		case ActionWindow::TEXT_FIELD_WINDOW: {
			s = ((TextFieldWindow *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return (val);
			}
			break;
		}
		case ActionWindow::SLIDER: {
			return (((SliderWindow *) item->optionWidget)->value);
		}
	}

	return (defaultValue);
}

bool ActionWindow::getBooleanValue (const StdString &optionName, bool defaultValue) {
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

std::list<ActionWindow::Item>::iterator ActionWindow::findItem (const StdString &optionName, bool createNewItem) {
	std::list<ActionWindow::Item>::iterator i, end;
	StdString option;
	ActionWindow::Item item;

	option = optionName.lowercased ();
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (option.equals (i->name.lowercased ())) {
			return (i);
		}
		++i;
	}

	if (createNewItem) {
		item.name.assign (optionName);
		itemList.push_back (item);

		i = itemList.begin ();
		end = itemList.end ();
		while (i != end) {
			if (option.equals (i->name.lowercased ())) {
				return (i);
			}
			++i;
		}
	}

	return (end);
}

void ActionWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	std::list<ActionWindow::Item>::iterator i, end;
	float x, y, x0, x2, y2, w, h;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	x2 = 0.0f;
	y2 = 0.0f;

	if (titleLabel->isVisible) {
		titleLabel->flowDown (x, &y, &x2, &y2);
		y = y2 + uiconfig->marginSize;
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
		if (i->descriptionText) {
			i->descriptionText->position.assign (x + w + uiconfig->marginSize, y);
			y += i->descriptionText->height + uiconfig->marginSize;
		}

		++i;
	}

	x = x0;
	x2 = 0.0f;
	if (cancelButton->isVisible) {
		cancelButton->flowRight (&x, y, &x2, &y2);
	}
	if (confirmButton->isVisible) {
		confirmButton->flowRight (&x, y, &x2, &y2);
	}

	resetSize ();

	x = width - widthPadding;
	if (confirmButton->isVisible) {
		confirmButton->flowLeft (&x);
	}
	if (cancelButton->isVisible) {
		cancelButton->flowLeft (&x);
	}
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

void ActionWindow::optionValueChanged (void *windowPtr, Widget *widgetPtr) {
	ActionWindow *window;

	window = (ActionWindow *) windowPtr;
	window->verifyOptions ();
	if (window->optionChangeCallback) {
		window->optionChangeCallback (window->optionChangeCallbackData, window);
	}
}

void ActionWindow::setOptionNotEmptyString (const StdString &optionName) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}

	item->isNotEmptyString = true;
	verifyOptions ();
}

void ActionWindow::verifyOptions () {
	UiConfiguration *uiconfig;
	std::list<ActionWindow::Item>::iterator i, end;
	StdString s;
	bool windowvalid, optionvalid;

	uiconfig = &(App::instance->uiConfig);
	windowvalid = true;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		optionvalid = true;
		switch (i->type) {
			case ActionWindow::COMBO_BOX: {
				s = ((ComboBox *) i->optionWidget)->getValue ();
				if (i->isNotEmptyString) {
					if (s.empty ()) {
						optionvalid = false;
						break;
					}
				}

				break;
			}
			case ActionWindow::TEXT_FIELD: {
				s = ((TextField *) i->optionWidget)->getValue ();
				if (i->isNotEmptyString) {
					if (s.empty ()) {
						optionvalid = false;
						break;
					}
				}

				break;
			}
			case ActionWindow::TEXT_FIELD_WINDOW: {
				s = ((TextFieldWindow *) i->optionWidget)->getValue ();
				if (i->isNotEmptyString) {
					if (s.empty ()) {
						optionvalid = false;
						break;
					}
				}

				break;
			}
			case ActionWindow::TOGGLE: {
				break;
			}
		}

		if (optionvalid) {
			i->nameLabel->textColor.translate (isInverseColor ? uiconfig->inverseTextColor : uiconfig->primaryTextColor, uiconfig->shortColorTranslateDuration);
		}
		else {
			windowvalid = false;
			i->nameLabel->textColor.translate (uiconfig->errorTextColor, uiconfig->shortColorTranslateDuration);
		}
		++i;
	}

	isOptionDataValid = windowvalid;
	if (isOptionDataValid) {
		confirmButton->setDisabled (false);
	}
	else {
		confirmButton->setDisabled (true);
	}
}
