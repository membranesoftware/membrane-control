/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "App.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "SystemInterface.h"
#include "Agent.h"
#include "AgentControl.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Sprite.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "StatsWindow.h"
#include "ServerAttachWindow.h"

const float ServerAttachWindow::NameTruncateScale = 0.21f;

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
{
	classId = ClassId::ServerAttachWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));
	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));

	attachButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::AttachServerButtonSprite)));
	attachButton->mouseClickCallback = Widget::EventCallbackContext (ServerAttachWindow::attachButtonClicked, this);
	attachButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	attachButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::AttachServerTooltip));

	removeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (ServerAttachWindow::removeButtonClicked, this);
	removeButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::RemoveServer).capitalized ());

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (UiConfiguration::instance->paddingSize, 0.0f);
	statsWindow->setItem (UiText::instance->getText (UiTextString::Address).capitalized (), StdString (""));
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

Widget::Rectangle ServerAttachWindow::getRemoveButtonScreenRect () {
	return (removeButton->getScreenRect ());
}

void ServerAttachWindow::refreshAgentData () {
	int type;

	agentDisplayName = AgentControl::instance->getAgentDisplayName (agentId);
	nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::BodyFont]->truncatedText (agentDisplayName, (float) App::instance->windowWidth * ServerAttachWindow::NameTruncateScale, Font::DotTruncateSuffix));
	descriptionLabel->setText (AgentControl::instance->getAgentApplicationName (agentId));
	statsWindow->setItem (UiText::instance->getText (UiTextString::Address).capitalized (), AgentControl::instance->getAgentHostAddress (agentId));

	type = AgentControl::instance->getAgentServerType (agentId);
	if (type != serverType) {
		serverType = type;
		iconImage->isDestroyed = true;
		switch (serverType) {
			case Agent::Monitor: {
				iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeDisplayIconSprite)));
				break;
			}
			case Agent::Media: {
				iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite)));
				break;
			}
			case Agent::Camera: {
				iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeCameraIconSprite)));
				break;
			}
			default: {
				iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite)));
				break;
			}
		}
	}

	refreshLayout ();
}

void ServerAttachWindow::refreshLayout () {
	float x, y, x0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	descriptionLabel->flowDown (x, &y, &x2, &y2);

	x = x0 + UiConfiguration::instance->marginSize;
	y = y2 + UiConfiguration::instance->marginSize;
	x2 = 0.0f;
	y2 = 0.0f;
	statsWindow->flowDown (x, &y, &x2, &y2);

	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
	attachButton->flowRight (&x, y);
	removeButton->flowRight (&x, y);

	resetSize ();

	x = width - widthPadding;
	removeButton->flowLeft (&x);
	attachButton->flowLeft (&x);
}

void ServerAttachWindow::attachButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((ServerAttachWindow *) windowPtr)->eventCallback (((ServerAttachWindow *) windowPtr)->attachClickCallback);
}

void ServerAttachWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((ServerAttachWindow *) windowPtr)->eventCallback (((ServerAttachWindow *) windowPtr)->removeClickCallback);
}
