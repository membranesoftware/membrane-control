/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "SystemInterface.h"
#include "AgentControl.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Sprite.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "StatsWindow.h"
#include "IconLabelWindow.h"
#include "ServerAttachWindow.h"

ServerAttachWindow::ServerAttachWindow (const StdString &agentId)
: Panel ()
, agentId (agentId)
, serverType (-1)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, statsWindow (NULL)
, attachButton (NULL)
, removeButton (NULL)
, attachClickCallback (NULL)
, attachClickCallbackData (NULL)
, removeClickCallback (NULL)
, removeClickCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::ServerAttachWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setCornerRadius (uiconfig->cornerRadius);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::ServerIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor));
	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));

	attachButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::AttachServerButtonSprite)));
	attachButton->setMouseClickCallback (ServerAttachWindow::attachButtonClicked, this);
	attachButton->setImageColor (uiconfig->flatButtonTextColor);
	attachButton->setMouseHoverTooltip (uitext->getText (UiTextString::attachServerTooltip));
	attachButton->isVisible = false;

	removeButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->setMouseClickCallback (ServerAttachWindow::removeButtonClicked, this);
	removeButton->setImageColor (uiconfig->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (uitext->getText (UiTextString::removeServer).capitalized ());
	removeButton->isVisible = false;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
	statsWindow->setItem (uitext->getText (UiTextString::address).capitalized (), StdString (""));
}

ServerAttachWindow::~ServerAttachWindow () {

}

StdString ServerAttachWindow::toStringDetail () {
	return (StdString::createSprintf (" ServerAttachWindow agentId=%s", agentId.c_str ()));
}

bool ServerAttachWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::ServerAttachWindow));
}

ServerAttachWindow *ServerAttachWindow::castWidget (Widget *widget) {
	return (ServerAttachWindow::isWidgetType (widget) ? (ServerAttachWindow *) widget : NULL);
}

void ServerAttachWindow::setAttachClickCallback (Widget::EventCallback callback, void *callbackData) {
	attachClickCallback = callback;
	attachClickCallbackData = callbackData;
	attachButton->isVisible = attachClickCallback ? true : false;
	refreshLayout ();
}

void ServerAttachWindow::setRemoveClickCallback (Widget::EventCallback callback, void *callbackData) {
	removeClickCallback = callback;
	removeClickCallbackData = callbackData;
	removeButton->isVisible = removeClickCallback ? true : false;
	refreshLayout ();
}

void ServerAttachWindow::refreshAgentData () {
	AgentControl *agentcontrol;
	UiConfiguration *uiconfig;
	UiText *uitext;
	int type;

	agentcontrol = &(App::instance->agentControl);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	agentDisplayName = agentcontrol->getAgentDisplayName (agentId);
	nameLabel->setText (agentDisplayName);
	descriptionLabel->setText (agentcontrol->getAgentApplicationName (agentId));
	statsWindow->setItem (uitext->getText (UiTextString::address).capitalized (), agentcontrol->getAgentHostAddress (agentId));

	type = agentcontrol->getAgentServerType (agentId);
	if (type != serverType) {
		serverType = type;
		iconImage->isDestroyed = true;
		switch (serverType) {
			case SystemInterface::Constant_Monitor: {
				iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeDisplayIconSprite)));
				break;
			}
			case SystemInterface::Constant_Media: {
				iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite)));
				break;
			}
			case SystemInterface::Constant_Camera: {
				iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeCameraIconSprite)));
				break;
			}
			default: {
				iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::ServerIconSprite)));
				break;
			}
		}
	}

	refreshLayout ();
}

void ServerAttachWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, x2, y2;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	descriptionLabel->flowDown (x, &y, &x2, &y2);

	x = x0 + uiconfig->marginSize;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	y2 = 0.0f;
	statsWindow->flowDown (x, &y, &x2, &y2);

	x = x0;
	y = y2 + uiconfig->marginSize;
	if (attachButton->isVisible) {
		attachButton->flowRight (&x, y);
	}
	if (removeButton->isVisible) {
		removeButton->flowRight (&x, y);
	}

	resetSize ();

	x = width - widthPadding;
	if (removeButton->isVisible) {
		removeButton->flowLeft (&x);
	}
	if (attachButton->isVisible) {
		attachButton->flowLeft (&x);
	}
}

void ServerAttachWindow::attachButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ServerAttachWindow *window;

	window = (ServerAttachWindow *) windowPtr;
	if (window->attachClickCallback) {
		window->attachClickCallback (window->attachClickCallbackData, window);
	}
}

void ServerAttachWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ServerAttachWindow *window;

	window = (ServerAttachWindow *) windowPtr;
	if (window->removeClickCallback) {
		window->removeClickCallback (window->removeClickCallbackData, window);
	}
}
