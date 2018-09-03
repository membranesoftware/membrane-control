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
#include <list>
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
#include "LinkUi.h"
#include "HyperlinkWindow.h"
#include "AgentStatusWindow.h"

AgentStatusWindow::AgentStatusWindow (const StdString &agentId, SpriteGroup *linkUiSpriteGroup)
: Panel ()
, agentId (agentId)
, spriteGroup (linkUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionText (NULL)
, helpLinkWindow (NULL)
, statsWindow (NULL)
, connectButton (NULL)
, connectCallback (NULL)
, connectCallbackData (NULL)
, disconnectButton (NULL)
, disconnectCallback (NULL)
, disconnectCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	populate ();
	resetLayout ();
}

AgentStatusWindow::~AgentStatusWindow () {

}

StdString AgentStatusWindow::toStringDetail () {
  return (StdString (" AgentStatusWindow"));
}

bool AgentStatusWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" AgentStatusWindow"));
}

AgentStatusWindow *AgentStatusWindow::castWidget (Widget *widget) {
	return (AgentStatusWindow::isWidgetType (widget) ? (AgentStatusWindow *) widget : NULL);
}

void AgentStatusWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	if (agentId.empty ()) {
		iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::ERROR_ICON)));
		nameLabel = (Label *) addWidget (new Label (uitext->linkUiEmptyAgentStatusTitle, UiConfiguration::HEADLINE, uiconfig->primaryTextColor));
	}
	else {
		iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (LinkUi::SERVER_ICON)));
		nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::HEADLINE, uiconfig->primaryTextColor));
	}

	descriptionText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	descriptionText->setPadding (uiconfig->paddingSize, 0.0f);
	if (agentId.empty ()) {
		descriptionText->setText (uitext->linkUiEmptyAgentStatusText);
	}
	else {
		descriptionText->isVisible = false;
	}

	helpLinkWindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow ());
	if (agentId.empty ()) {
		helpLinkWindow->setLink (uitext->learnMore.capitalized (), Util::getHelpUrl ("servers"));
	}
	else {
		helpLinkWindow->isVisible = false;
	}

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
	statsWindow->setItem (uitext->address.capitalized (), "");
	statsWindow->setItem (uitext->version.capitalized (), "");
	statsWindow->setItem (uitext->uptime.capitalized (), "");
	statsWindow->isVisible = false;

	connectButton = (Button *) addWidget (new Button (uitext->connect.uppercased ()));
	connectButton->setMouseClickCallback (AgentStatusWindow::connectButtonClicked, this);
	connectButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	connectButton->setMouseHoverTooltip (uitext->connectTooltip);
	connectButton->isVisible = false;

	disconnectButton = (Button *) addWidget (new Button (uitext->disconnect.uppercased ()));
	disconnectButton->setMouseClickCallback (AgentStatusWindow::disconnectButtonClicked, this);
	disconnectButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	disconnectButton->setMouseHoverTooltip (uitext->disconnectTooltip);
	disconnectButton->isVisible = false;
}

void AgentStatusWindow::setConnectCallback (Widget::EventCallback callback, void *callbackData) {
	connectCallback = callback;
	connectCallbackData = callbackData;
}

void AgentStatusWindow::setDisconnectCallback (Widget::EventCallback callback, void *callbackData) {
	disconnectCallback = callback;
	disconnectCallbackData = callbackData;
}

void AgentStatusWindow::setConnecting () {
	UiText *uitext;
	std::list<AgentStatusWindow::ServerItem>::iterator i;

	i = findServerItem ("linkServerStatus");
	if (i == serverList.end ()) {
		return;
	}

	uitext = &(App::getInstance ()->uiText);
	connectButton->setWaiting (true);
	connectButton->isVisible = true;
	disconnectButton->isVisible = false;
	i->statsWindow->setItem (uitext->uplinkStatus.capitalized (), uitext->connecting.capitalized ());
	resetLayout ();
}

void AgentStatusWindow::setConnected () {
	UiText *uitext;
	std::list<AgentStatusWindow::ServerItem>::iterator i;

	i = findServerItem ("linkServerStatus");
	if (i == serverList.end ()) {
		return;
	}

	uitext = &(App::getInstance ()->uiText);
	connectButton->setWaiting (false);
	connectButton->isVisible = false;
	disconnectButton->isVisible = true;
	i->statsWindow->setItem (uitext->uplinkStatus.capitalized (), uitext->connected.capitalized ());
	resetLayout ();
}

void AgentStatusWindow::setDisconnected () {
	UiText *uitext;
	std::list<AgentStatusWindow::ServerItem>::iterator i;

	i = findServerItem ("linkServerStatus");
	if (i == serverList.end ()) {
		return;
	}

	uitext = &(App::getInstance ()->uiText);
	connectButton->setWaiting (false);
	connectButton->isVisible = true;
	disconnectButton->isVisible = false;
	i->statsWindow->setItem (uitext->uplinkStatus.capitalized (), uitext->notConnected.capitalized ());
	resetLayout ();
}

void AgentStatusWindow::syncRecordStore (RecordStore *store) {
	SystemInterface *interface;
	UiText *uitext;
	Json *record;

	interface = &(App::getInstance ()->systemInterface);
	record = store->findRecord (agentId, SystemInterface::Command_AgentStatus);
	if (! record) {
		return;
	}

	uitext = &(App::getInstance ()->uiText);
	nameLabel->setText (interface->getCommandAgentName (record));
	descriptionText->setText (interface->getCommandStringParam (record, "applicationName", ""));
	descriptionText->isVisible = true;

	statsWindow->setItem (uitext->address.capitalized (), Util::getAddressDisplayString (interface->getCommandAgentAddress (record), SystemInterface::Constant_DefaultTcpPort));
	statsWindow->setItem (uitext->version.capitalized (), interface->getCommandStringParam (record, "version", ""));
	statsWindow->setItem (uitext->uptime.capitalized (), interface->getCommandStringParam (record, "uptime", ""));
	statsWindow->isVisible = true;

	syncMediaServerStatus (record);
	syncStreamServerStatus (record);
	syncDisplayServerStatus (record);
	syncMasterServerStatus (record);
	syncMonitorServerStatus (record);
	syncLinkServerStatus (record);

	resetLayout ();
	Panel::syncRecordStore (store);
}

void AgentStatusWindow::syncMediaServerStatus (Json *record) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StatsWindow *stats;
	Json serverstatus;
	std::list<AgentStatusWindow::ServerItem>::iterator i;
	AgentStatusWindow::ServerItem item;

	interface = &(App::getInstance ()->systemInterface);
	if (! interface->getCommandObjectParam (record, "mediaServerStatus", &serverstatus)) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	stats = NULL;
	i = findServerItem ("mediaServerStatus");
	if (i == serverList.end ()) {
		item.itemId.assign ("mediaServerStatus");
		item.iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (LinkUi::MEDIA_ICON)));

		item.statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
		item.statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
		stats = item.statsWindow;

		serverList.push_back (item);
	}
	else {
		stats = i->statsWindow;
	}

	if (stats) {
		stats->setItem (uitext->mediaItems.capitalized (), StdString::createSprintf ("%i", serverstatus.getNumber ("mediaCount", (int) 0)));
	}
}

void AgentStatusWindow::syncStreamServerStatus (Json *record) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Json serverstatus;
	std::list<AgentStatusWindow::ServerItem>::iterator i;
	AgentStatusWindow::ServerItem item;
	StatsWindow *stats;
	int64_t totalspace, freespace;
	float pct;

	interface = &(App::getInstance ()->systemInterface);
	if (! interface->getCommandObjectParam (record, "streamServerStatus", &serverstatus)) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	totalspace = serverstatus.getNumber ("totalSpace", (int64_t) 0);
	freespace = serverstatus.getNumber ("freeSpace", (int64_t) 0);
	pct = 0.0f;
	if (totalspace > 0) {
		pct = (float) freespace;
		pct /= (float) totalspace;
		pct *= 100.0f;
	}

	stats = NULL;
	i = findServerItem ("streamServerStatus");
	if (i == serverList.end ()) {
		item.itemId.assign ("streamServerStatus");
		item.iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (LinkUi::STREAM_SERVER_ICON)));

		item.statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
		item.statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
		stats = item.statsWindow;

		serverList.push_back (item);
	}
	else {
		stats = i->statsWindow;
	}

	if (stats) {
		stats->setItem (uitext->videoStreams.capitalized (), StdString::createSprintf ("%i", serverstatus.getNumber ("streamCount", (int) 0)));
		stats->setItem (uitext->totalStorage.capitalized (), Util::getFileSizeDisplayString (totalspace));
		stats->setItem (uitext->freeStorage.capitalized (), StdString::createSprintf ("%s (%i%%)", Util::getFileSizeDisplayString (freespace).c_str (), (int) pct));
	}
}

void AgentStatusWindow::syncDisplayServerStatus (Json *record) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StatsWindow *stats;
	Json serverstatus;
	std::list<AgentStatusWindow::ServerItem>::iterator i;
	AgentStatusWindow::ServerItem item;

	interface = &(App::getInstance ()->systemInterface);
	if (! interface->getCommandObjectParam (record, "displayServerStatus", &serverstatus)) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	stats = NULL;
	i = findServerItem ("displayServerStatus");
	if (i == serverList.end ()) {
		item.itemId.assign ("displayServerStatus");
		item.iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (LinkUi::DISPLAY_ICON)));

		item.statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
		item.statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
		stats = item.statsWindow;

		serverList.push_back (item);
	}
	else {
		stats = i->statsWindow;
	}

	if (stats) {
		stats->setItem (uitext->displayStatus.capitalized (), uitext->getMonitorStatusText (&serverstatus));
	}
}

void AgentStatusWindow::syncMasterServerStatus (Json *record) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StatsWindow *stats;
	Json serverstatus;
	std::list<AgentStatusWindow::ServerItem>::iterator i;
	AgentStatusWindow::ServerItem item;

	interface = &(App::getInstance ()->systemInterface);
	if (! interface->getCommandObjectParam (record, "masterServerStatus", &serverstatus)) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	stats = NULL;
	i = findServerItem ("masterServerStatus");
	if (i == serverList.end ()) {
		item.itemId.assign ("masterServerStatus");
		item.iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (LinkUi::MASTER_ICON)));

		item.statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
		item.statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
		stats = item.statsWindow;

		serverList.push_back (item);
	}
	else {
		stats = i->statsWindow;
	}

	if (stats) {
		stats->setItem (uitext->jobs.capitalized (), StdString::createSprintf ("%i", serverstatus.getNumber ("intentCount", (int) 0)));
	}
}

void AgentStatusWindow::syncMonitorServerStatus (Json *record) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StatsWindow *stats;
	Json serverstatus;
	std::list<AgentStatusWindow::ServerItem>::iterator i;
	AgentStatusWindow::ServerItem item;

	interface = &(App::getInstance ()->systemInterface);
	if (! interface->getCommandObjectParam (record, "monitorServerStatus", &serverstatus)) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	stats = NULL;
	i = findServerItem ("monitorServerStatus");
	if (i == serverList.end ()) {
		item.itemId.assign ("monitorServerStatus");
		item.iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (LinkUi::DISPLAY_ICON)));

		item.statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
		item.statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
		stats = item.statsWindow;

		serverList.push_back (item);
	}
	else {
		stats = i->statsWindow;
	}

	if (stats) {
		stats->setItem (uitext->program.capitalized (), serverstatus.getString ("intentName", ""));
		stats->setItem (uitext->displayStatus.capitalized (), uitext->getMonitorStatusText (&serverstatus));
	}
}

void AgentStatusWindow::syncLinkServerStatus (Json *record) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	Json serverstatus;
	std::list<AgentStatusWindow::ServerItem>::iterator i;
	AgentStatusWindow::ServerItem item;
	bool isnew;

	interface = &(App::getInstance ()->systemInterface);
	if (! interface->getCommandObjectParam (record, "linkServerStatus", &serverstatus)) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	isnew = false;
	i = findServerItem ("linkServerStatus");
	if (i == serverList.end ()) {
		item.itemId.assign ("linkServerStatus");
		item.iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (LinkUi::LINK_ICON)));

		item.statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
		item.statsWindow->setPadding (uiconfig->paddingSize, 0.0f);

		serverList.push_back (item);
		isnew = true;
	}

	if (isnew) {
		if (App::getInstance ()->agentControl.isLinkClientConnected (agentId)) {
			setConnected ();
		}
		else {
			setDisconnected ();
		}
	}
}

std::list<AgentStatusWindow::ServerItem>::iterator AgentStatusWindow::findServerItem (const StdString &itemId) {
	std::list<AgentStatusWindow::ServerItem>::iterator i, end, result;

	i = serverList.begin ();
	end = serverList.end ();
	result = end;
	while (i != end) {
		if (i->itemId.equals (itemId)) {
			result = i;
			break;
		}
		++i;
	}

	return (result);
}

void AgentStatusWindow::resetLayout () {
	UiConfiguration *uiconfig;
	std::list<AgentStatusWindow::ServerItem>::iterator i, end;
	float x, y, x0, h;

	uiconfig = &(App::getInstance ()->uiConfig);
	x0 = widthPadding;
	x = x0;
	y = heightPadding;

	iconImage->position.assign (x, y);
	x += iconImage->width + uiconfig->marginSize;
	nameLabel->position.assign (x, y + (iconImage->height / 2.0f) - (nameLabel->height / 2.0f));

	x = x0;
	y += iconImage->height;
	if (descriptionText->isVisible) {
		if (agentId.empty ()) {
			y += uiconfig->marginSize;
		}
		descriptionText->position.assign (x, y);
		y += descriptionText->height + uiconfig->marginSize;
	}
	if (statsWindow->isVisible) {
		statsWindow->position.assign (x, y);
		y += statsWindow->height + uiconfig->marginSize;
	}

	i = serverList.begin ();
	end = serverList.end ();
	while (i != end) {
		x = x0;

		h = i->iconImage->height;
		i->iconImage->position.assign (x, y);
		x += i->iconImage->width + uiconfig->marginSize;

		i->statsWindow->position.assign (x, y);
		if (i->statsWindow->height > h) {
			h = i->statsWindow->height;
		}

		y += h + uiconfig->marginSize;
		++i;
	}

	x = x0;
	if (helpLinkWindow->isVisible) {
		helpLinkWindow->position.assign (x, helpLinkWindow->getLinePosition (y));
		y += helpLinkWindow->height + uiconfig->marginSize;
	}
	if (connectButton->isVisible) {
		connectButton->position.assign (x, y);
		y += connectButton->height + uiconfig->marginSize;
	}
	if (disconnectButton->isVisible) {
		disconnectButton->position.assign (x, y);
		y += disconnectButton->height + uiconfig->marginSize;
	}

	resetSize ();

	x = width - widthPadding;
	if (helpLinkWindow->isVisible) {
		x -= helpLinkWindow->width;
		helpLinkWindow->position.assignX (x);
		x -= uiconfig->marginSize;
	}

	x = width - widthPadding;
	if (connectButton->isVisible) {
		x -= connectButton->width;
		connectButton->position.assignX (x);
		x -= uiconfig->marginSize;
	}
	if (disconnectButton->isVisible) {
		x -= disconnectButton->width;
		disconnectButton->position.assignX (x);
		x -= uiconfig->marginSize;
	}
}

void AgentStatusWindow::connectButtonClicked (void *windowPtr, Widget *widgetPtr) {
	AgentStatusWindow *window;

	window = (AgentStatusWindow *) windowPtr;
	if (window->connectCallback) {
		window->connectCallback (window->connectCallbackData, window);
	}
}

void AgentStatusWindow::disconnectButtonClicked (void *windowPtr, Widget *widgetPtr) {
	AgentStatusWindow *window;

	window = (AgentStatusWindow *) windowPtr;
	if (window->disconnectCallback) {
		window->disconnectCallback (window->disconnectCallbackData, window);
	}
}
