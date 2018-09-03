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
#include "WebKioskUi.h"
#include "WebKioskWindow.h"

WebKioskWindow::WebKioskWindow (const StdString &agentId, SpriteGroup *webKioskUiSpriteGroup)
: Panel ()
, isSelected (false)
, agentId (agentId)
, spriteGroup (webKioskUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionText (NULL)
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
	resetLayout ();
}

WebKioskWindow::~WebKioskWindow () {

}

StdString WebKioskWindow::toStringDetail () {
	return (StdString::createSprintf (" WebKioskWindow agentId=\"%s\"", agentId.c_str ()));
}

bool WebKioskWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" WebKioskWindow agentId="));
}

WebKioskWindow *WebKioskWindow::castWidget (Widget *widget) {
	return (WebKioskWindow::isWidgetType (widget) ? (WebKioskWindow *) widget : NULL);
}

void WebKioskWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (WebKioskUi::DISPLAY_ICON)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::HEADLINE, uiconfig->primaryTextColor));

	descriptionText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	descriptionText->isVisible = false;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::STAR_OUTLINE_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::STAR_BUTTON)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (WebKioskWindow::selectToggleStateChanged, this);
	selectToggle->setMouseHoverTooltip (uitext->selectToggleTooltip);
}

void WebKioskWindow::syncRecordStore (RecordStore *store) {
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

	descriptionText->setText (interface->getCommandStringParam (record, "applicationName", ""));
	descriptionText->isVisible = true;

	statsWindow->setItem (uitext->address.capitalized (), Util::getAddressDisplayString (interface->getCommandAgentAddress (record), SystemInterface::Constant_DefaultTcpPort));
	statsWindow->setItem (uitext->version.capitalized (), interface->getCommandStringParam (record, "version", ""));
	statsWindow->setItem (uitext->uptime.capitalized (), interface->getCommandStringParam (record, "uptime", ""));
	statsWindow->setItem (uitext->program.capitalized (), serverstatus.getString ("intentName", ""));
	statsWindow->setItem (uitext->displayStatus.capitalized (), uitext->getMonitorStatusText (&serverstatus));

	resetLayout ();
	Panel::syncRecordStore (store);
}

void WebKioskWindow::setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	selectStateChangeCallback = callback;
	selectStateChangeCallbackData = callbackData;
}

void WebKioskWindow::resetLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, y0;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;

	iconImage->position.assign (x, y);
	x += iconImage->width + uiconfig->marginSize;

	nameLabel->position.assign (x, nameLabel->getLinePosition (y));
	x += nameLabel->width + uiconfig->marginSize;

	selectToggle->position.assign (x, y);

	y += nameLabel->maxLineHeight + uiconfig->textLineHeightMargin;

	if (y < (y0 + iconImage->height)) {
		y = y0 + iconImage->height;
	}

	x = x0;
	descriptionText->position.assign (x + uiconfig->paddingSize, y);
	y += descriptionText->height + uiconfig->marginSize;

	x = x0;
	statsWindow->position.assign (x, y);
	y += statsWindow->height + uiconfig->marginSize;

	resetSize ();

	x = width - uiconfig->marginSize;
	x -= selectToggle->width;
	selectToggle->position.assignX (x);
	x -= uiconfig->marginSize;
}

void WebKioskWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	WebKioskWindow *window;
	Toggle *toggle;

	window = (WebKioskWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->isSelected = toggle->isChecked;
	if (window->selectStateChangeCallback) {
		window->selectStateChangeCallback (window->selectStateChangeCallbackData, window);
	}
}
