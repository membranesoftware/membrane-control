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
#include "openssl/evp.h"
#include "App.h"
#include "Log.h"
#include "ClassId.h"
#include "StdString.h"
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
#include "Image.h"
#include "TextField.h"
#include "ListView.h"
#include "Json.h"
#include "SystemInterface.h"
#include "AgentControl.h"
#include "IconLabelWindow.h"
#include "ActionWindow.h"
#include "AdminSecretWindow.h"

AdminSecretWindow::AdminSecretWindow ()
: Panel ()
, isExpanded (false)
, iconImage (NULL)
, iconLabel (NULL)
, dividerPanel (NULL)
, itemListView (NULL)
, addButton (NULL)
, expandToggle (NULL)
{
	classId = ClassId::AdminSecretWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeKeyIconSprite)));
	iconImage->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ServerPasswords).capitalized ());
	iconLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), StdString ("0"), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	itemListView = (ListView *) addWidget (new ListView (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, 3, 3, UiConfiguration::CaptionFont, UiText::instance->getText (UiTextString::EmptyAdminSecretListText)));
	itemListView->listChangeCallback = Widget::EventCallbackContext (AdminSecretWindow::listChanged, this);
	itemListView->itemDeleteCallback = Widget::EventCallbackContext (AdminSecretWindow::listItemDeleted, this);
	itemListView->isVisible = false;

	addButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::AddButtonSprite)));
	addButton->mouseClickCallback = Widget::EventCallbackContext (AdminSecretWindow::addButtonClicked, this);
	addButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	addButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::AddAdminPasswordTooltip));
	addButton->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (AdminSecretWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());

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

void AdminSecretWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	isExpanded = expanded;
	if (isExpanded) {
		expandToggle->setChecked (true, true);
		dividerPanel->isVisible = true;
		itemListView->isVisible = true;
		addButton->isVisible = true;
	}
	else {
		expandToggle->setChecked (false, true);
		dividerPanel->isVisible = false;
		itemListView->isVisible = false;
		addButton->isVisible = false;
	}

	refreshLayout ();
	if (! shouldSkipStateChangeCallback) {
		eventCallback (expandStateChangeCallback);
	}
}

void AdminSecretWindow::refreshLayout () {
	float x, y, x0, y0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	iconLabel->flowDown (x, &y, &x2, &y2);
	iconLabel->position.assignY (y0 + ((y2 - y0) / 2.0f) - (iconLabel->height / 2.0f));
	expandToggle->position.assign (x2 + UiConfiguration::instance->marginSize, y0);

	x = x0;
	x2 = 0.0f;
	y = y2 + UiConfiguration::instance->marginSize;
	if (dividerPanel->isVisible) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
	}
	if (itemListView->isVisible) {
		itemListView->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	x2 = 0.0f;
	if (addButton->isVisible) {
		addButton->flowDown (x, &y, &x2, &y2);
	}

	resetSize ();

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	}

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
	window->eventCallback (window->expandStateChangeCallback);
}

void AdminSecretWindow::addButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((AdminSecretWindow *) windowPtr)->eventCallback (((AdminSecretWindow *) windowPtr)->addButtonClickCallback);
}

void AdminSecretWindow::listChanged (void *windowPtr, Widget *widgetPtr) {
	AdminSecretWindow *window;
	ListView *listview;
	int count;

	window = (AdminSecretWindow *) windowPtr;
	listview = (ListView *) widgetPtr;

	count = listview->getItemCount ();
	window->iconLabel->setText (StdString::createSprintf ("%i", count));
	window->iconLabel->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::PasswordStored, UiTextString::PasswordsStored));
	window->refreshLayout ();
}

void AdminSecretWindow::listItemDeleted (void *windowPtr, Widget *widgetPtr) {
	AdminSecretWindow *window;
	ListView *listview;
	int count;

	window = (AdminSecretWindow *) windowPtr;
	listview = (ListView *) widgetPtr;
	if (listview->focusItemIndex < 0) {
		return;
	}

	AgentControl::instance->removeAdminSecret (listview->focusItemIndex);
	listview->removeItem (listview->focusItemIndex);
	count = listview->getItemCount ();
	window->iconLabel->setText (StdString::createSprintf ("%i", count));
	window->iconLabel->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::PasswordStored, UiTextString::PasswordsStored));
	window->refreshLayout ();
}

ActionWindow *AdminSecretWindow::createAddActionWindow () {
	ActionWindow *action;
	TextField *textfield;
	TextFieldWindow *textfieldwindow;
	StdString name;
	int i, len;

	action = new ActionWindow ();
	action->setInverseColor (true);
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setTitleText (UiText::instance->getText (UiTextString::CreateAdminPassword).capitalized ());

	i = itemListView->getItemCount () + 1;
	while (true) {
		name.sprintf ("%s %i", UiText::instance->getText (UiTextString::Password).capitalized ().c_str (), i);
		if (! itemListView->contains (name)) {
			break;
		}
		++i;
	}

	len = (UiConfiguration::instance->textFieldShortLineLength + UiConfiguration::instance->textFieldMediumLineLength) / 2;

	textfield = new TextField (len * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiText::instance->getText (UiTextString::AdminPasswordNamePrompt));
	textfield->setPromptErrorColor (true);
	textfield->setValue (name);
	action->addOption (UiText::instance->getText (UiTextString::Name).capitalized (), textfield);
	action->setOptionNotEmptyString (UiText::instance->getText (UiTextString::Name).capitalized ());

	textfieldwindow = new TextFieldWindow (len * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiText::instance->getText (UiTextString::AdminPasswordPrompt));
	textfieldwindow->setPromptErrorColor (true);
	textfieldwindow->setObscured (true);
	textfieldwindow->setButtonsEnabled (false, false, false, false, true);
	action->addOption (UiText::instance->getText (UiTextString::Password).capitalized (), textfieldwindow);
	action->setOptionNotEmptyString (UiText::instance->getText (UiTextString::Password).capitalized ());

	return (action);
}

void AdminSecretWindow::addItem (ActionWindow *actionWindow) {
	StdString name, password;
	EVP_MD_CTX *ctx;
	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int len;

	name = actionWindow->getStringValue (UiText::instance->getText (UiTextString::Name).capitalized (), StdString (""));
	password = actionWindow->getStringValue (UiText::instance->getText (UiTextString::Password).capitalized (), StdString (""));
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

	AgentControl::instance->addAdminSecret (name, StdString::createHex (digest, len));
	itemListView->addItem (name);
	itemListView->scrollToBottom ();
	refreshLayout ();
}

void AdminSecretWindow::readItems () {
	StringList items;

	AgentControl::instance->getAdminSecretNames (&items);
	iconLabel->setText (StdString::createSprintf ("%i", (int) items.size ()));
	iconLabel->setMouseHoverTooltip (UiText::instance->getCountText ((int) items.size (), UiTextString::PasswordStored, UiTextString::PasswordsStored));
	itemListView->setItems (&items, true);
	refreshLayout ();
}
