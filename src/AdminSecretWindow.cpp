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
#include "openssl/evp.h"
#include "Result.h"
#include "Log.h"
#include "ClassId.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Color.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Label.h"
#include "HashMap.h"
#include "Button.h"
#include "TextArea.h"
#include "Image.h"
#include "TextField.h"
#include "ListView.h"
#include "Json.h"
#include "SystemInterface.h"
#include "IconLabelWindow.h"
#include "ActionWindow.h"
#include "AdminSecretWindow.h"

AdminSecretWindow::AdminSecretWindow ()
: Panel ()
, isExpanded (false)
, iconImage (NULL)
, nameLabel (NULL)
, itemListView (NULL)
, addButton (NULL)
, expandToggle (NULL)
, addButtonClickCallback (NULL)
, addButtonClickCallbackData (NULL)
, expandStateChangeCallback (NULL)
, expandStateChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::AdminSecretWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::KeyIconSprite)));
	nameLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::serverPasswords).capitalized (), UiConfiguration::HeadlineFont, uiconfig->primaryTextColor));

	itemListView = (ListView *) addWidget (new ListView (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, 3, UiConfiguration::CaptionFont, StdString (""), uitext->getText (UiTextString::emptyAdminSecretListText)));
	itemListView->setListChangeCallback (AdminSecretWindow::listChanged, this);
	itemListView->setItemDeleteCallback (AdminSecretWindow::listItemDeleted, this);
	itemListView->isVisible = false;

	addButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::AddButtonSprite)));
	addButton->setImageColor (uiconfig->flatButtonTextColor);
	addButton->setMouseClickCallback (AdminSecretWindow::addButtonClicked, this);
	addButton->setMouseHoverTooltip (uitext->getText (UiTextString::addAdminPasswordTooltip));
	addButton->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (AdminSecretWindow::expandToggleStateChanged, this);
	expandToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::expand).capitalized (), uitext->getText (UiTextString::minimize).capitalized ());

	readItems ();
}

AdminSecretWindow::~AdminSecretWindow () {

}

StdString AdminSecretWindow::toStringDetail () {
	return (StdString (" AdminSecretWindow"));
}

bool AdminSecretWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::AdminSecretWindow));
}

AdminSecretWindow *AdminSecretWindow::castWidget (Widget *widget) {
	return (AdminSecretWindow::isWidgetType (widget) ? (AdminSecretWindow *) widget : NULL);
}

void AdminSecretWindow::setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	expandStateChangeCallback = callback;
	expandStateChangeCallbackData = callbackData;
}

void AdminSecretWindow::setAddButtonClickCallback (Widget::EventCallback callback, void *callbackData) {
	addButtonClickCallback = callback;
	addButtonClickCallbackData = callbackData;
	if (addButtonClickCallback && isExpanded) {
		addButton->isVisible = true;
	}
	else {
		addButton->isVisible = false;
	}
	refreshLayout ();
}

void AdminSecretWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	isExpanded = expanded;
	if (isExpanded) {
		expandToggle->setChecked (true, true);
		itemListView->isVisible = true;
		if (addButtonClickCallback) {
			addButton->isVisible = true;
		}
		else {
			addButton->isVisible = false;
		}
	}
	else {
		expandToggle->setChecked (false, true);
		itemListView->isVisible = false;
		addButton->isVisible = false;
	}

	refreshLayout ();
	if ((! shouldSkipStateChangeCallback) && expandStateChangeCallback) {
		expandStateChangeCallback (expandStateChangeCallbackData, this);
	}
}

void AdminSecretWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, y0, x2, y2;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	nameLabel->position.assignY (y0 + ((y2 - y0) / 2.0f) - (nameLabel->height / 2.0f));
	expandToggle->position.assign (x2 + uiconfig->marginSize, y0);

	x = x0;
	x2 = 0.0f;
	y = y2 + uiconfig->marginSize;
	if (itemListView->isVisible) {
		itemListView->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	x2 = 0.0f;
	if (addButton->isVisible) {
		addButton->flowDown (x, &y, &x2, &y2);
	}

	resetSize ();

	x = width - widthPadding;
	expandToggle->flowLeft (&x);

	x = width - widthPadding;
	if (addButton->isVisible) {
		addButton->flowLeft (&x);
	}
}

void AdminSecretWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	AdminSecretWindow *window;
	Toggle *toggle;

	window = (AdminSecretWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback) {
		window->expandStateChangeCallback (window->expandStateChangeCallbackData, window);
	}
}

void AdminSecretWindow::addButtonClicked (void *windowPtr, Widget *widgetPtr) {
	AdminSecretWindow *window;

	window = (AdminSecretWindow *) windowPtr;
	if (window->addButtonClickCallback) {
		window->addButtonClickCallback (window->addButtonClickCallbackData, window);
	}
}

void AdminSecretWindow::listChanged (void *windowPtr, Widget *widgetPtr) {
	AdminSecretWindow *window;

	window = (AdminSecretWindow *) windowPtr;
	window->refreshLayout ();
}

void AdminSecretWindow::listItemDeleted (void *windowPtr, Widget *widgetPtr) {
	ListView *listview;
	StdString name;

	listview = (ListView *) widgetPtr;
	name = listview->getItemText (listview->focusItemIndex);
	if (! name.empty ()) {
		App::instance->agentControl.removeAdminSecret (name);
	}
	listview->removeItem (listview->focusItemIndex);
}

ActionWindow *AdminSecretWindow::createAddActionWindow () {
	ActionWindow *action;
	UiConfiguration *uiconfig;
	UiText *uitext;
	TextField *textfield;
	TextFieldWindow *textfieldwindow;
	StdString name;
	int i, len;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	action = new ActionWindow ();
	action->setInverseColor (true);
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setTitleText (uitext->getText (UiTextString::createAdminPassword).capitalized ());

	i = itemListView->getItemCount () + 1;
	while (true) {
		name.sprintf ("%s %i", uitext->getText (UiTextString::password).capitalized ().c_str (), i);
		if (! itemListView->contains (name)) {
			break;
		}
		++i;
	}

	len = (uiconfig->textFieldShortLineLength + uiconfig->textFieldMediumLineLength) / 2;

	textfield = new TextField (len * uiconfig->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, uitext->getText (UiTextString::adminPasswordNamePrompt));
	textfield->setPromptErrorColor (true);
	textfield->setValue (name);
	action->addOption (uitext->getText (UiTextString::name).capitalized (), textfield);
	action->setOptionNotEmptyString (uitext->getText (UiTextString::name).capitalized ());

	textfieldwindow = new TextFieldWindow (len * uiconfig->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, uitext->getText (UiTextString::adminPasswordPrompt));
	textfieldwindow->setPromptErrorColor (true);
	textfieldwindow->setObscured (true);
	textfieldwindow->setButtonsEnabled (false, false, false, false, true);
	action->addOption (uitext->getText (UiTextString::password).capitalized (), textfieldwindow);
	action->setOptionNotEmptyString (uitext->getText (UiTextString::password).capitalized ());

	return (action);
}

void AdminSecretWindow::addItem (ActionWindow *actionWindow) {
	UiText *uitext;
	StdString name, password;
	EVP_MD_CTX *ctx;
	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int len;

	uitext = &(App::instance->uiText);
	name = actionWindow->getStringValue (uitext->getText (UiTextString::name).capitalized (), StdString (""));
	password = actionWindow->getStringValue (uitext->getText (UiTextString::password).capitalized (), StdString (""));
	if (name.empty () || password.empty ()) {
		return;
	}

	ctx = EVP_MD_CTX_create ();
	if (! ctx) {
		password.wipe ();
		Log::err ("Failed to store admin secret; err=EVP_MD_CTX_create failed");
		return;
	}

	len = 0;
	if (EVP_DigestInit_ex (ctx, EVP_sha256 (), NULL) != 1) {
		Log::err ("Failed to store admin secret; err=EVP_DigestInit_ex failed");
	}
	else {
		if (EVP_DigestUpdate (ctx, password.c_str (), password.length ()) != 1) {
			Log::err ("Failed to store admin secret; err=EVP_DigestUpdate failed");
		}
		else {
			if (EVP_DigestFinal_ex (ctx, digest, &len) != 1) {
				Log::err ("Failed to store admin secret; err=EVP_DigestFinal_ex failed");
			}
		}
	}

	EVP_MD_CTX_destroy (ctx);
	password.wipe ();
	if (len <= 0) {
		return;
	}

	App::instance->agentControl.addAdminSecret (name, StdString::createHex (digest, len));
	itemListView->addItem (name);
	refreshLayout ();
}

void AdminSecretWindow::readItems () {
	StringList items;

	App::instance->agentControl.getAdminSecretNames (&items);
	itemListView->setItems (&items, true);
	refreshLayout ();
}
