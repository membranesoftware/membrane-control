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
#include "UiText.h"
#include "Util.h"
#include "Widget.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Button.h"
#include "Toggle.h"
#include "StatsWindow.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "MonitorWindow.h"

MonitorWindow::MonitorWindow (const StdString &agentId)
: Panel ()
, isSelected (false)
, agentId (agentId)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, statsWindow (NULL)
, selectToggle (NULL)
, selectStateChangeCallback (NULL)
, selectStateChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	populate ();
	refreshLayout ();
}

MonitorWindow::~MonitorWindow () {

}

StdString MonitorWindow::toStringDetail () {
	return (StdString::createSprintf (" MonitorWindow agentId=\"%s\"", agentId.c_str ()));
}

bool MonitorWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" MonitorWindow agentId="));
}

MonitorWindow *MonitorWindow::castWidget (Widget *widget) {
	return (MonitorWindow::isWidgetType (widget) ? (MonitorWindow *) widget : NULL);
}

void MonitorWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::DISPLAY_ICON)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::HEADLINE, uiconfig->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::STAR_OUTLINE_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::STAR_BUTTON)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (MonitorWindow::selectToggleStateChanged, this);
	selectToggle->setMouseHoverTooltip (uitext->getText (UiTextString::selectToggleTooltip));
	selectToggle->isVisible = false;
}

void MonitorWindow::syncRecordStore (RecordStore *store) {
	SystemInterface *interface;
	UiText *uitext;
	Json *record, serverstatus;

	interface = &(App::getInstance ()->systemInterface);
	record = store->findRecord (agentId, SystemInterface::Command_AgentStatus);
	if (! record) {
		return;
	}
	if (! interface->getCommandObjectParam (record, "monitorServerStatus", &serverstatus)) {
		return;
	}

	uitext = &(App::getInstance ()->uiText);
	agentName.assign (interface->getCommandAgentName (record));
	nameLabel->setText (agentName);

	descriptionLabel->setText (interface->getCommandStringParam (record, "applicationName", ""));
	descriptionLabel->isVisible = true;

	statsWindow->setItem (uitext->getText (UiTextString::address).capitalized (), Util::getAddressDisplayString (interface->getCommandAgentAddress (record), SystemInterface::Constant_DefaultTcpPort1));
	statsWindow->setItem (uitext->getText (UiTextString::version).capitalized (), interface->getCommandStringParam (record, "version", ""));
	statsWindow->setItem (uitext->getText (UiTextString::uptime).capitalized (), interface->getCommandStringParam (record, "uptime", ""));
	statsWindow->setItem (uitext->getText (UiTextString::program).capitalized (), serverstatus.getString ("intentName", ""));
	statsWindow->setItem (uitext->getText (UiTextString::displayStatus).capitalized (), uitext->getMonitorStatusText (&serverstatus));

	refreshLayout ();
	Panel::syncRecordStore (store);
}

void MonitorWindow::setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	selectStateChangeCallback = callback;
	selectStateChangeCallbackData = callbackData;
	selectToggle->isVisible = selectStateChangeCallback ? true : false;
}

void MonitorWindow::refreshLayout () {
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
	if (selectToggle->isVisible) {
		selectToggle->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	statsWindow->flowDown (x, &y);

	resetSize ();

	x = width - uiconfig->paddingSize;
	if (selectToggle->isVisible) {
		selectToggle->flowLeft (&x);
	}
}

void MonitorWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	Toggle *toggle;

	window = (MonitorWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->isSelected = toggle->isChecked;
	if (window->selectStateChangeCallback) {
		window->selectStateChangeCallback (window->selectStateChangeCallbackData, window);
	}
}
