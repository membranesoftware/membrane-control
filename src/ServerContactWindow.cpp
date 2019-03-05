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
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Color.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Image.h"
#include "TextArea.h"
#include "Button.h"
#include "Label.h"
#include "ServerContactWindow.h"

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
, stateChangeCallback (NULL)
, stateChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::SERVER_ICON)));
	nameLabel = (Label *) addWidget (new Label (agentDisplayName, UiConfiguration::HEADLINE, uiconfig->primaryTextColor));
	statusLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::serverUiContactingAgentDescription), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));

	detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->primaryTextColor, uiconfig->textAreaShortLineLength));
	detailText->isVisible = false;

	progressBar = (ProgressBar *) addWidget (new ProgressBar (1.0f, uiconfig->progressBarHeight));
	progressBar->zLevel = 1;
	progressBar->setIndeterminate (true);
	progressBar->isVisible = false;

	deleteButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON)));
	deleteButton->setMouseClickCallback (ServerContactWindow::deleteButtonClicked, this);
	deleteButton->setImageColor (uiconfig->flatButtonTextColor);
	deleteButton->isVisible = false;

	layout = ServerContactWindow::CONTACTING;
	refreshLayout ();
}

ServerContactWindow::~ServerContactWindow () {

}

StdString ServerContactWindow::toStringDetail () {
	return (StdString (" ServerContactWindow"));
}

bool ServerContactWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" ServerContactWindow"));
}

ServerContactWindow *ServerContactWindow::castWidget (Widget *widget) {
	return (ServerContactWindow::isWidgetType (widget) ? (ServerContactWindow *) widget : NULL);
}

void ServerContactWindow::setStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	stateChangeCallback = callback;
	stateChangeCallbackData = callbackData;
}

void ServerContactWindow::doUpdate (int msElapsed, float originX, float originY) {
	UiText *uitext;
	int curlayout;

	Panel::doUpdate (msElapsed, originX, originY);

	uitext = &(App::instance->uiText);
	curlayout = layout;
	if (App::instance->agentControl.isContacted (agentHostname, agentPort)) {
		curlayout = ServerContactWindow::IDLE;
		isDeleted = true;
	}
	else if (App::instance->agentControl.isContacting (agentHostname, agentPort)) {
		curlayout = ServerContactWindow::CONTACTING;
	}
	else {
		if (curlayout == ServerContactWindow::CONTACTING) {
			if (App::instance->agentControl.isUnauthorized (agentHostname, agentPort)) {
				curlayout = ServerContactWindow::UNAUTHORIZED;
			}
			else {
				curlayout = ServerContactWindow::FAILED;
			}
		}
	}

	if (curlayout != layout) {
		layout = curlayout;
		switch (layout) {
			case ServerContactWindow::IDLE: {
				progressBar->isVisible = false;
				statusLabel->isVisible = false;
				detailText->isVisible = false;
				deleteButton->isVisible = false;
				break;
			}
			case ServerContactWindow::CONTACTING: {
				progressBar->isVisible = true;
				statusLabel->setText (uitext->getText (UiTextString::serverUiContactingAgentDescription));
				statusLabel->isVisible = true;
				detailText->isVisible = false;
				deleteButton->isVisible = false;
				break;
			}
			case ServerContactWindow::UNAUTHORIZED: {
				progressBar->isVisible = false;
				statusLabel->setText (uitext->getText (UiTextString::serverUiUnauthorizedErrorTitle));
				statusLabel->isVisible = true;
				detailText->setText (uitext->getText (UiTextString::serverUiUnauthorizedErrorText));
				detailText->isVisible = true;
				deleteButton->isVisible = true;
				break;
			}
			case ServerContactWindow::FAILED: {
				progressBar->isVisible = false;
				statusLabel->setText (uitext->getText (UiTextString::serverUiFailedContactErrorTitle));
				statusLabel->isVisible = true;
				detailText->setText (uitext->getText (UiTextString::serverUiFailedContactErrorText));
				detailText->isVisible = true;
				deleteButton->isVisible = true;
				break;
			}
		}
		refreshLayout ();
		if (stateChangeCallback) {
			stateChangeCallback (stateChangeCallbackData, this);
		}
	}
}

void ServerContactWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, y0, x2, y2;

	uiconfig = &(App::instance->uiConfig);
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
		deleteButton->position.assign (x2 + uiconfig->marginSize, y0);
	}
	if (detailText->isVisible) {
		detailText->flowDown (x, &y, &x2, &y2);
	}
	if (progressBar->isVisible) {
		progressBar->position.assign (0.0f, y);
		progressBar->setSize (x2 + (widthPadding * 2.0f), progressBar->height);
	}

	resetSize ();
	x = width;
	if (deleteButton->isVisible) {
		deleteButton->flowLeft (&x);
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
