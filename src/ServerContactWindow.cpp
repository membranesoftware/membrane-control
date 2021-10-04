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
#include "UiText.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Color.h"
#include "UiConfiguration.h"
#include "AgentControl.h"
#include "Panel.h"
#include "Image.h"
#include "TextFlow.h"
#include "Button.h"
#include "Label.h"
#include "ServerContactWindow.h"

const float ServerContactWindow::NameTruncateScale = 0.21f;

ServerContactWindow::ServerContactWindow (const StdString &displayName, const StdString &hostname, int port)
: Panel ()
, agentDisplayName (displayName)
, agentHostname (hostname)
, agentPort (port)
, isDeleted (false)
, iconImage (NULL)
, nameLabel (NULL)
, statusLabel (NULL)
, detailText (NULL)
, progressBar (NULL)
, deleteButton (NULL)
{
	classId = ClassId::ServerContactWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite)));
	nameLabel = (Label *) addWidget (new Label (UiConfiguration::instance->fonts[UiConfiguration::HeadlineFont]->truncatedText (agentDisplayName, (float) App::instance->windowWidth * ServerContactWindow::NameTruncateScale, Font::DotTruncateSuffix), UiConfiguration::HeadlineFont, UiConfiguration::instance->primaryTextColor));
	statusLabel = (Label *) addWidget (new Label (UiText::instance->getText (UiTextString::ServerUiContactingAgentDescription), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));

	detailText = (TextFlow *) addWidget (new TextFlow (UiConfiguration::instance->textFieldShortLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
	detailText->setTextColor (UiConfiguration::instance->primaryTextColor);
	detailText->isVisible = false;

	progressBar = (ProgressBar *) addWidget (new ProgressBar (1.0f, UiConfiguration::instance->progressBarHeight));
	progressBar->zLevel = 1;
	progressBar->setIndeterminate (true);

	deleteButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	deleteButton->mouseClickCallback = Widget::EventCallbackContext (ServerContactWindow::deleteButtonClicked, this);
	deleteButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Remove).capitalized ());
	deleteButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	deleteButton->isVisible = false;

	layout = ServerContactWindow::ContactingLayout;
	refreshLayout ();
}

ServerContactWindow::~ServerContactWindow () {

}

StdString ServerContactWindow::toStringDetail () {
	return (StdString (" ServerContactWindow"));
}

bool ServerContactWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::ServerContactWindow));
}

ServerContactWindow *ServerContactWindow::castWidget (Widget *widget) {
	return (ServerContactWindow::isWidgetType (widget) ? (ServerContactWindow *) widget : NULL);
}

void ServerContactWindow::doUpdate (int msElapsed) {
	int curlayout;

	Panel::doUpdate (msElapsed);
	curlayout = layout;
	if (AgentControl::instance->isHostContacted (agentHostname, agentPort)) {
		curlayout = ServerContactWindow::IdleLayout;
		isDeleted = true;
	}
	else if (AgentControl::instance->isHostContacting (agentHostname, agentPort)) {
		curlayout = ServerContactWindow::ContactingLayout;
	}
	else {
		if (curlayout == ServerContactWindow::ContactingLayout) {
			if (AgentControl::instance->isHostUnauthorized (agentHostname, agentPort)) {
				curlayout = ServerContactWindow::UnauthorizedLayout;
			}
			else {
				curlayout = ServerContactWindow::FailedLayout;
			}
		}
	}

	if (curlayout != layout) {
		layout = curlayout;
		switch (layout) {
			case ServerContactWindow::IdleLayout: {
				progressBar->isVisible = false;
				statusLabel->isVisible = false;
				detailText->isVisible = false;
				deleteButton->isVisible = false;
				break;
			}
			case ServerContactWindow::ContactingLayout: {
				progressBar->isVisible = true;
				statusLabel->setText (UiText::instance->getText (UiTextString::ServerUiContactingAgentDescription));
				statusLabel->isVisible = true;
				detailText->isVisible = false;
				deleteButton->isVisible = false;
				break;
			}
			case ServerContactWindow::UnauthorizedLayout: {
				progressBar->isVisible = false;
				statusLabel->setText (UiText::instance->getText (UiTextString::ServerUiUnauthorizedErrorTitle));
				statusLabel->isVisible = true;
				detailText->setText (UiText::instance->getText (UiTextString::ServerUiUnauthorizedErrorText));
				detailText->isVisible = true;
				deleteButton->isVisible = true;
				break;
			}
			case ServerContactWindow::FailedLayout: {
				progressBar->isVisible = false;
				statusLabel->setText (UiText::instance->getText (UiTextString::ServerUiFailedContactErrorTitle));
				statusLabel->isVisible = true;
				detailText->setText (UiText::instance->getText (UiTextString::ServerUiFailedContactErrorText));
				detailText->isVisible = true;
				deleteButton->isVisible = true;
				break;
			}
		}
		refreshLayout ();
		eventCallback (stateChangeCallback);
	}
}

void ServerContactWindow::refreshLayout () {
	float x, y, y0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	if (statusLabel->isVisible) {
		statusLabel->flowDown (x, &y, &x2, &y2);
	}
	if (deleteButton->isVisible) {
		deleteButton->position.assign (x2 + UiConfiguration::instance->marginSize, y0);
	}
	if (detailText->isVisible) {
		detailText->flowDown (x, &y, &x2, &y2);
	}
	if (progressBar->isVisible) {
		progressBar->position.assign (0.0f, y);
		progressBar->setSize (x2 + (widthPadding * 2.0f), progressBar->height);
	}

	resetSize ();
	if (deleteButton->isVisible) {
		deleteButton->position.assignX (width - widthPadding - deleteButton->width);
	}
	if (progressBar->isVisible) {
		progressBar->position.assign (0.0f, height - progressBar->height);
		progressBar->setSize (width, progressBar->height);
	}
}

void ServerContactWindow::deleteButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ServerContactWindow *window;

	window = (ServerContactWindow *) windowPtr;
	window->isDeleted = true;
}
