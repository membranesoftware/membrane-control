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
#include "ClassId.h"
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
#include "TextFlow.h"
#include "Toggle.h"
#include "SliderWindow.h"
#include "UiConfiguration.h"
#include "ActionWindow.h"

ActionWindow::ActionWindow ()
: Panel ()
, isOptionDataValid (true)
, isConfirmed (false)
, isInverseColor (false)
, titleLabel (NULL)
, headerDescriptionText (NULL)
, footerPanel (NULL)
, confirmButton (NULL)
, cancelButton (NULL)
{
	classId = ClassId::ActionWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setFillBg (true, UiConfiguration::instance->lightBackgroundColor);

	titleLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	titleLabel->isInputSuspended = true;
	titleLabel->isVisible = false;

	headerDescriptionText = (TextFlow *) addWidget (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
	headerDescriptionText->setTextColor (UiConfiguration::instance->primaryTextColor);
	headerDescriptionText->isInputSuspended = true;
	headerDescriptionText->isVisible = false;

	confirmButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::OkButtonSprite)));
	confirmButton->mouseClickCallback = Widget::EventCallbackContext (ActionWindow::confirmButtonClicked, this);
	confirmButton->setTextColor (UiConfiguration::instance->raisedButtonTextColor);
	confirmButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	confirmButtonTooltipText.assign (UiText::instance->getText (UiTextString::Confirm).capitalized ());
	confirmButton->setMouseHoverTooltip (confirmButtonTooltipText);

	cancelButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::CancelButtonSprite)));
	cancelButton->mouseClickCallback = Widget::EventCallbackContext (ActionWindow::cancelButtonClicked, this);
	cancelButton->setTextColor (UiConfiguration::instance->raisedButtonTextColor);
	cancelButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	cancelButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Cancel).capitalized ());

	refreshLayout ();
}

ActionWindow::~ActionWindow () {

}

StdString ActionWindow::toStringDetail () {
	return (StdString (" ActionWindow"));
}

bool ActionWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::ActionWindow));
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

void ActionWindow::setDescriptionText (const StdString &text) {
	headerDescriptionText->setText (text);
	if (text.empty ()) {
		headerDescriptionText->isVisible = false;
	}
	else {
		headerDescriptionText->isVisible = true;
	}
	refreshLayout ();
}

void ActionWindow::setConfirmTooltipText (const StdString &text) {
	confirmButtonTooltipText.assign (text);
	if (isOptionDataValid) {
		confirmButton->setMouseHoverTooltip (confirmButtonTooltipText);
	}
	else {
		confirmButton->setMouseHoverTooltip (StdString::createSprintf ("%s %s", confirmButtonTooltipText.c_str (), UiText::instance->getText (UiTextString::ActionWindowInvalidDataTooltip).c_str ()));
	}
}

void ActionWindow::setInverseColor (bool inverse) {
	std::list<ActionWindow::Item>::iterator i, end;

	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	if (isInverseColor) {
		if (isFilledBg) {
			setFillBg (true, UiConfiguration::instance->lightPrimaryColor);
		}

		titleLabel->textColor.assign (UiConfiguration::instance->inverseTextColor);
		headerDescriptionText->setTextColor (UiConfiguration::instance->inverseTextColor);

		confirmButton->setInverseColor (true);
		confirmButton->setTextColor (UiConfiguration::instance->raisedButtonInverseTextColor);
		confirmButton->setRaised (true, UiConfiguration::instance->darkInverseBackgroundColor);

		cancelButton->setInverseColor (true);
		cancelButton->setTextColor (UiConfiguration::instance->raisedButtonInverseTextColor);
		cancelButton->setRaised (true, UiConfiguration::instance->darkInverseBackgroundColor);
	}
	else {
		if (isFilledBg) {
			setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		}

		titleLabel->textColor.assign (UiConfiguration::instance->primaryTextColor);
		headerDescriptionText->setTextColor (UiConfiguration::instance->primaryTextColor);

		confirmButton->setInverseColor (false);
		confirmButton->setTextColor (UiConfiguration::instance->raisedButtonTextColor);
		confirmButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);

		cancelButton->setInverseColor (false);
		cancelButton->setTextColor (UiConfiguration::instance->raisedButtonTextColor);
		cancelButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	}

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		i->nameLabel->textColor.assign (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
		switch (i->type) {
			case ActionWindow::ComboBoxItem: {
				((ComboBox *) i->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
			case ActionWindow::TextFieldItem: {
				((TextField *) i->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
			case ActionWindow::TextFieldWindowItem: {
				((TextFieldWindow *) i->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
			case ActionWindow::ToggleItem: {
				((Toggle *) i->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
		}
		if (i->descriptionText) {
			i->descriptionText->setTextColor (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
		}
		++i;
	}
}

void ActionWindow::setFooterPanel (Panel *panel) {
	if (footerPanel) {
		footerPanel->isDestroyed = true;
	}
	footerPanel = panel;
	addWidget (footerPanel);
	refreshLayout ();
}

void ActionWindow::addOption (const StdString &optionName, ComboBox *comboBox, const StdString &descriptionText) {
	comboBox->valueChangeCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	comboBox->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::ComboBoxItem, optionName, comboBox, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, TextField *textField, const StdString &descriptionText) {
	textField->valueChangeCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	textField->valueEditCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	textField->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::TextFieldItem, optionName, textField, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, TextFieldWindow *textFieldWindow, const StdString &descriptionText) {
	textFieldWindow->valueChangeCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	textFieldWindow->valueEditCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	textFieldWindow->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::TextFieldWindowItem, optionName, textFieldWindow, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, Toggle *toggle, const StdString &descriptionText) {
	toggle->stateChangeCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	toggle->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::ToggleItem, optionName, toggle, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, SliderWindow *slider, const StdString &descriptionText) {
	slider->valueChangeCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	slider->setInverseColor (isInverseColor);
	doAddOption (ActionWindow::SliderItem, optionName, slider, descriptionText);
}

void ActionWindow::doAddOption (int itemType, const StdString &optionName, Widget *optionWidget, const StdString &descriptionText) {
	std::list<ActionWindow::Item>::iterator item;

	addWidget (optionWidget);
	item = findItem (optionName, true);
	if (item != itemList.end ()) {
		item->type = itemType;
		if (! item->nameLabel) {
			item->nameLabel = (Label *) addWidget (new Label (item->name, UiConfiguration::CaptionFont, isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor));
		}
		else {
			item->nameLabel->setText (item->name);
		}

		if (item->descriptionText) {
			item->descriptionText->isDestroyed = true;
			item->descriptionText = NULL;
		}
		if (! descriptionText.empty ()) {
			item->descriptionText = (TextFlow *) addWidget (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
			item->descriptionText->setTextColor (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
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

void ActionWindow::setOptionValue (const StdString &optionName, const char *optionValue, bool shouldSkipChangeCallback) {
	setOptionValue (optionName, StdString (optionValue), shouldSkipChangeCallback);
}

void ActionWindow::setOptionValue (const StdString &optionName, const StdString &optionValue, bool shouldSkipChangeCallback) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	switch (item->type) {
		case ActionWindow::ComboBoxItem: {
			((ComboBox *) item->optionWidget)->setValue (optionValue, shouldSkipChangeCallback);
			break;
		}
		case ActionWindow::TextFieldItem: {
			((TextField *) item->optionWidget)->setValue (optionValue, shouldSkipChangeCallback, true);
			break;
		}
		case ActionWindow::TextFieldWindowItem: {
			((TextFieldWindow *) item->optionWidget)->setValue (optionValue, shouldSkipChangeCallback, true);
			break;
		}
		case ActionWindow::ToggleItem: {
			((Toggle *) item->optionWidget)->setChecked (optionValue.equals ("true"), shouldSkipChangeCallback);
			break;
		}
	}

	if (shouldSkipChangeCallback) {
		verifyOptions ();
	}
	refreshLayout ();
}

void ActionWindow::setOptionValue (const StdString &optionName, int optionValue, bool shouldSkipChangeCallback) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	switch (item->type) {
		case ActionWindow::ComboBoxItem: {
			((ComboBox *) item->optionWidget)->setValue (StdString::createSprintf ("%i", optionValue), shouldSkipChangeCallback);
			break;
		}
		case ActionWindow::TextFieldItem: {
			((TextField *) item->optionWidget)->setValue (StdString::createSprintf ("%i", optionValue), shouldSkipChangeCallback, true);
			break;
		}
		case ActionWindow::TextFieldWindowItem: {
			((TextFieldWindow *) item->optionWidget)->setValue (StdString::createSprintf ("%i", optionValue), shouldSkipChangeCallback, true);
			break;
		}
		case ActionWindow::SliderItem: {
			((SliderWindow *) item->optionWidget)->setValue ((float) optionValue, shouldSkipChangeCallback);
			break;
		}
	}

	if (shouldSkipChangeCallback) {
		verifyOptions ();
	}
	refreshLayout ();
}

void ActionWindow::setOptionValue (const StdString &optionName, float optionValue, bool shouldSkipChangeCallback) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	switch (item->type) {
		case ActionWindow::ComboBoxItem: {
			((ComboBox *) item->optionWidget)->setValue (StdString::createSprintf ("%f", optionValue), shouldSkipChangeCallback);
			break;
		}
		case ActionWindow::TextFieldItem: {
			((TextField *) item->optionWidget)->setValue (StdString::createSprintf ("%f", optionValue), shouldSkipChangeCallback, true);
			break;
		}
		case ActionWindow::TextFieldWindowItem: {
			((TextFieldWindow *) item->optionWidget)->setValue (StdString::createSprintf ("%f", optionValue), shouldSkipChangeCallback, true);
			break;
		}
		case ActionWindow::SliderItem: {
			((SliderWindow *) item->optionWidget)->setValue (optionValue, shouldSkipChangeCallback);
			break;
		}
	}

	if (shouldSkipChangeCallback) {
		verifyOptions ();
	}
	refreshLayout ();
}

void ActionWindow::setOptionValue (const StdString &optionName, bool optionValue, bool shouldSkipChangeCallback) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	switch (item->type) {
		case ActionWindow::ToggleItem: {
			((Toggle *) item->optionWidget)->setChecked (optionValue, shouldSkipChangeCallback);
			break;
		}
	}

	if (shouldSkipChangeCallback) {
		verifyOptions ();
	}
	refreshLayout ();
}

StdString ActionWindow::getStringValue (const StdString &optionName, const StdString &defaultValue) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return (defaultValue);
	}
	switch (item->type) {
		case ActionWindow::ComboBoxItem: {
			return (((ComboBox *) item->optionWidget)->getValue ());
		}
		case ActionWindow::TextFieldItem: {
			return (((TextField *) item->optionWidget)->getValue ());
		}
		case ActionWindow::TextFieldWindowItem: {
			return (((TextFieldWindow *) item->optionWidget)->getValue ());
		}
		case ActionWindow::ToggleItem: {
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
		case ActionWindow::ComboBoxItem: {
			s = ((ComboBox *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return ((int) val);
			}
			break;
		}
		case ActionWindow::TextFieldItem: {
			s = ((TextField *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return ((int) val);
			}
			break;
		}
		case ActionWindow::TextFieldWindowItem: {
			s = ((TextFieldWindow *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return ((int) val);
			}
			break;
		}
		case ActionWindow::SliderItem: {
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
		case ActionWindow::ComboBoxItem: {
			s = ((ComboBox *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return (val);
			}
			break;
		}
		case ActionWindow::TextFieldItem: {
			s = ((TextField *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return (val);
			}
			break;
		}
		case ActionWindow::TextFieldWindowItem: {
			s = ((TextFieldWindow *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return (val);
			}
			break;
		}
		case ActionWindow::SliderItem: {
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
		case ActionWindow::ToggleItem: {
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
	std::list<ActionWindow::Item>::iterator i, end;
	float x, y, x0, x2, y2, w, h;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	x2 = 0.0f;
	y2 = 0.0f;

	if (titleLabel->isVisible) {
		titleLabel->flowDown (x, &y, &x2, &y2);
		y = y2 + UiConfiguration::instance->marginSize;
	}
	if (headerDescriptionText->isVisible) {
		headerDescriptionText->flowDown (x, &y, &x2, &y2);
		y = y2 + UiConfiguration::instance->marginSize;
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
		i->optionWidget->position.assign (x + w + UiConfiguration::instance->marginSize, y);

		h = i->nameLabel->height;
		if (i->optionWidget->height > h) {
			h = i->optionWidget->height;
			i->nameLabel->position.assignY (y + (h / 2.0f) - (i->nameLabel->height / 2.0f));
		}

		y += h + UiConfiguration::instance->marginSize;
		if (i->descriptionText) {
			i->descriptionText->position.assign (x + w + UiConfiguration::instance->marginSize, y);
			y += i->descriptionText->height + UiConfiguration::instance->marginSize;
		}

		++i;
	}

	x = x0;
	x2 = 0.0f;
	if (footerPanel) {
		footerPanel->flowRight (&x, y, &x2, &y2);
	}
	if (cancelButton->isVisible) {
		cancelButton->flowRight (&x, y, &x2, &y2);
	}
	if (confirmButton->isVisible) {
		confirmButton->flowRight (&x, y, &x2, &y2);
	}
	if (footerPanel) {
		footerPanel->centerVertical (y, y2);
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
	window->eventCallback (window->closeCallback);
	window->isDestroyed = true;
}

void ActionWindow::cancelButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ActionWindow *window;

	window = (ActionWindow *) windowPtr;
	window->isConfirmed = false;
	window->eventCallback (window->closeCallback);
	window->isDestroyed = true;
}

void ActionWindow::optionValueChanged (void *windowPtr, Widget *widgetPtr) {
	ActionWindow *window;

	window = (ActionWindow *) windowPtr;
	window->verifyOptions ();
	window->eventCallback (window->optionChangeCallback);
}

void ActionWindow::setOptionNameText (const StdString &optionName, const StdString &nameText) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	if (item->nameLabel) {
		item->nameLabel->setText (nameText);
	}
	refreshLayout ();
}

void ActionWindow::setOptionDescriptionText (const StdString &optionName, const StdString &descriptionText) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	if (item->descriptionText) {
		item->descriptionText->isDestroyed = true;
		item->descriptionText = NULL;
	}
	if (! descriptionText.empty ()) {
		item->descriptionText = (TextFlow *) addWidget (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
		item->descriptionText->setTextColor (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
		item->descriptionText->setText (descriptionText);
	}
	refreshLayout ();
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

void ActionWindow::setOptionDisabled (const StdString &optionName, bool disable) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	item->isDisabled = disable;
	switch (item->type) {
		case ActionWindow::ComboBoxItem: {
			((ComboBox *) item->optionWidget)->setDisabled (item->isDisabled);
			break;
		}
		case ActionWindow::TextFieldItem: {
			((TextField *) item->optionWidget)->setDisabled (item->isDisabled);
			break;
		}
		case ActionWindow::TextFieldWindowItem: {
			((TextFieldWindow *) item->optionWidget)->setDisabled (item->isDisabled);
			break;
		}
		case ActionWindow::ToggleItem: {
			((Toggle *) item->optionWidget)->setDisabled (item->isDisabled);
			break;
		}
		case ActionWindow::SliderItem: {
			((SliderWindow *) item->optionWidget)->setDisabled (item->isDisabled);
			break;
		}
	}
}

void ActionWindow::verifyOptions () {
	std::list<ActionWindow::Item>::iterator i, end;
	StdString s;
	bool windowvalid, optionvalid;

	windowvalid = true;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		optionvalid = true;
		switch (i->type) {
			case ActionWindow::ComboBoxItem: {
				s = ((ComboBox *) i->optionWidget)->getValue ();
				if (i->isNotEmptyString) {
					if (s.empty ()) {
						optionvalid = false;
						break;
					}
				}
				break;
			}
			case ActionWindow::TextFieldItem: {
				s = ((TextField *) i->optionWidget)->getValue ();
				if (i->isNotEmptyString) {
					if (s.empty ()) {
						optionvalid = false;
						break;
					}
				}
				break;
			}
			case ActionWindow::TextFieldWindowItem: {
				s = ((TextFieldWindow *) i->optionWidget)->getValue ();
				if (i->isNotEmptyString) {
					if (s.empty ()) {
						optionvalid = false;
						break;
					}
				}
				break;
			}
			case ActionWindow::ToggleItem: {
				break;
			}
		}

		if (optionvalid) {
			i->nameLabel->textColor.translate (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		}
		else {
			windowvalid = false;
			i->nameLabel->textColor.translate (UiConfiguration::instance->errorTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		}
		++i;
	}

	isOptionDataValid = windowvalid;
	if (isOptionDataValid) {
		confirmButton->setDisabled (false);
		confirmButton->setMouseHoverTooltip (confirmButtonTooltipText);
	}
	else {
		confirmButton->setDisabled (true);
		confirmButton->setMouseHoverTooltip (StdString::createSprintf ("%s %s", confirmButtonTooltipText.c_str (), UiText::instance->getText (UiTextString::ActionWindowInvalidDataTooltip).c_str ()));
	}
}
