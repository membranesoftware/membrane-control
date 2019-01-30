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
#include "UiText.h"
#include "Util.h"
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
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Image.h"
#include "Json.h"
#include "SystemInterface.h"
#include "IconLabelWindow.h"
#include "ActionWindow.h"
#include "HyperlinkWindow.h"
#include "ServerWindow.h"

ServerWindow::ServerWindow (const StdString &agentId)
: Panel ()
, agentId (agentId)
, isRecordLoaded (false)
, menuPositionY (0.0f)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, statsWindow (NULL)
, menuButton (NULL)
, menuClickCallback (NULL)
, menuClickCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::SERVER_ICON)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::HEADLINE, uiconfig->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
	statsWindow->setItem (uitext->getText (UiTextString::status).capitalized (), "");
	statsWindow->setItem (uitext->getText (UiTextString::uptime).capitalized (), "");
	statsWindow->setItem (uitext->getText (UiTextString::address).capitalized (), "");
	statsWindow->setItem (uitext->getText (UiTextString::version).capitalized (), "");
	statsWindow->setItem (uitext->getText (UiTextString::platform).capitalized (), "");
	statsWindow->isVisible = false;

	menuButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::MAIN_MENU_BUTTON)));
	menuButton->setMouseClickCallback (ServerWindow::menuButtonClicked, this);
	menuButton->setImageColor (uiconfig->flatButtonTextColor);
	menuButton->setMouseHoverTooltip (uitext->getText (UiTextString::moreActionsTooltip));
	menuButton->isVisible = false;

	refreshLayout ();
}

ServerWindow::~ServerWindow () {

}

StdString ServerWindow::toStringDetail () {
	return (StdString (" ServerWindow"));
}

bool ServerWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" ServerWindow"));
}

ServerWindow *ServerWindow::castWidget (Widget *widget) {
	return (ServerWindow::isWidgetType (widget) ? (ServerWindow *) widget : NULL);
}

void ServerWindow::setMenuClickCallback (Widget::EventCallback callback, void *callbackData) {
	menuClickCallback = callback;
	menuClickCallbackData = callbackData;
}

void ServerWindow::syncRecordStore (RecordStore *store) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Json *record, serverstatus;
	StdString s, version, platform;
	Color color;
	int icontype;

	interface = &(App::getInstance ()->systemInterface);
	record = store->findRecord (agentId, SystemInterface::Command_AgentStatus);
	if (! record) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	version = interface->getCommandStringParam (record, "version", "");
	platform = interface->getCommandStringParam (record, "platform", "");
	if (! version.empty ()) {
		applicationId.assign (version);
		if (! platform.empty ()) {
			applicationId.appendSprintf ("_%s", platform.c_str ());
		}
	}

	nameLabel->setText (interface->getCommandAgentName (record));
	agentDisplayName.assign (nameLabel->text);
	descriptionLabel->setText (interface->getCommandStringParam (record, "applicationName", ""));
	descriptionLabel->isVisible = true;

	s.assign (uitext->getText (UiTextString::ready).capitalized ());
	color.assign (uiconfig->lightPrimaryTextColor);
	if (! interface->getCommandBooleanParam (record, "isEnabled", false)) {
		s.assign (uitext->getText (UiTextString::disabled).capitalized ());
		color.assign (uiconfig->errorTextColor);
	}
	statsWindow->setItem (uitext->getText (UiTextString::status).capitalized (), s);
	statsWindow->setItemTextColor (uitext->getText (UiTextString::status).capitalized (), color);

	statsWindow->setItem (uitext->getText (UiTextString::address).capitalized (), Util::getAddressDisplayString (interface->getCommandAgentAddress (record), SystemInterface::Constant_DefaultTcpPort1));
	statsWindow->setItem (uitext->getText (UiTextString::version).capitalized (), interface->getCommandStringParam (record, "version", ""));
	statsWindow->setItem (uitext->getText (UiTextString::platform).capitalized (), interface->getCommandStringParam (record, "platform", ""));
	statsWindow->setItem (uitext->getText (UiTextString::uptime).capitalized (), interface->getCommandStringParam (record, "uptime", ""));
	statsWindow->isVisible = true;
	if (menuClickCallback) {
		menuButton->isVisible = true;
	}

	icontype = UiConfiguration::SERVER_ICON;
	if (interface->getCommandObjectParam (record, "streamServerStatus", &serverstatus)) {
		statsWindow->setItem (uitext->getText (UiTextString::storage).capitalized (), Util::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));
	}
	if (interface->getCommandObjectParam (record, "mediaServerStatus", &serverstatus)) {
		icontype = UiConfiguration::LARGE_MEDIA_ICON;
		statsWindow->setItem (uitext->getText (UiTextString::mediaItems).capitalized (), StdString::createSprintf ("%i", serverstatus.getNumber ("mediaCount", (int) 0)));
	}
	if (interface->getCommandObjectParam (record, "streamServerStatus", &serverstatus)) {
		statsWindow->setItem (uitext->getText (UiTextString::videoStreams).capitalized (), StdString::createSprintf ("%i", serverstatus.getNumber ("streamCount", (int) 0)));
	}
	if (interface->getCommandObjectParam (record, "monitorServerStatus", &serverstatus)) {
		icontype = UiConfiguration::DISPLAY_ICON;
		statsWindow->setItem (uitext->getText (UiTextString::storage).capitalized (), Util::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));
		statsWindow->setItem (uitext->getText (UiTextString::cachedStreams).capitalized (), StdString::createSprintf ("%i", serverstatus.getNumber ("streamCount", (int) 0)));
		statsWindow->setItem (uitext->getText (UiTextString::program).capitalized (), serverstatus.getString ("intentName", ""));
		statsWindow->setItem (uitext->getText (UiTextString::displayStatus).capitalized (), uitext->getMonitorStatusText (&serverstatus));
	}

	if (! isRecordLoaded) {
		iconImage->isDestroyed = true;
		iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (icontype)));
	}
	isRecordLoaded = true;

	refreshLayout ();
	Panel::syncRecordStore (store);
}

void ServerWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, y0, x2, y2;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	descriptionLabel->flowRight (&x, y, &x2, &y2);

	x = x2;
	y = y0;
	if (menuButton->isVisible) {
		menuButton->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	if (statsWindow->isVisible) {
		statsWindow->flowDown (x, &y);
	}

	resetSize ();

	x = width - uiconfig->paddingSize;
	if (menuButton->isVisible) {
		menuButton->flowLeft (&x);
		menuPositionY = menuButton->position.y + menuButton->height;
	}
}

void ServerWindow::menuButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ServerWindow *window;

	window = (ServerWindow *) windowPtr;
	if (window->menuClickCallback) {
		window->menuClickCallback (window->menuClickCallbackData, window);
	}
}
