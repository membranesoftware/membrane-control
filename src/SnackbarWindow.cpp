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
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Widget.h"
#include "Color.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "Button.h"
#include "UiConfiguration.h"
#include "SnackbarWindow.h"

SnackbarWindow::SnackbarWindow (float maxWidth)
: Panel ()
, backgroundPanel (NULL)
, messageLabel (NULL)
, actionButton (NULL)
, maxWidth (maxWidth)
, timeoutClock (0)
, isTimeoutSuspended (false)
, isTimeoutDismissed (false)
, scrollDuration (0)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);

	backgroundPanel = (Panel *) addWidget (new Panel ());
	backgroundPanel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	backgroundPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha));
	backgroundPanel->setMouseClickCallback (SnackbarWindow::backgroundPanelClicked, this);

	messageLabel = (Label *) backgroundPanel->addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->inverseTextColor));
	messageLabel->zLevel = 1;

	actionButton = (Button *) backgroundPanel->addWidget (new Button ());
	actionButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	actionButton->setTextColor (uiconfig->raisedButtonTextColor);
	actionButton->zLevel = 2;
	actionButton->isVisible = false;

	refreshLayout ();
}

SnackbarWindow::~SnackbarWindow () {

}

StdString SnackbarWindow::toStringDetail () {
	return (StdString (" SnackbarWindow"));
}

void SnackbarWindow::setMaxWidth (float maxWidthValue) {
	maxWidth = maxWidthValue;
	refreshLayout ();
}

void SnackbarWindow::setMessageText (const StdString &messageText) {
	messageLabel->setText (messageText);
	refreshLayout ();
}

void SnackbarWindow::setActionButtonEnabled (bool enable, const StdString &buttonText, Widget::EventCallback buttonClickCallback, void *buttonClickCallbackData) {
	if (enable && (! buttonText.empty ()) && buttonClickCallback) {
		actionButton->setText (buttonText);
		actionButton->setMouseClickCallback (buttonClickCallback, buttonClickCallbackData);
		actionButton->isVisible = true;
	}
	else {
		actionButton->isVisible = false;
	}
	refreshLayout ();
}

void SnackbarWindow::startScroll (int duration) {
	refreshLayout ();

	scrollDuration = duration;
	if (scrollDuration < 1) {
		scrollDuration = 1;
	}
	backgroundPanel->position.assign (0.0f, -(backgroundPanel->height));
	backgroundPanel->position.translate (0.0f, 0.0f, scrollDuration);
}

void SnackbarWindow::startTimeout (int timeout) {
	if (timeout < 1) {
		timeout = 1;
	}
	isTimeoutDismissed = false;
	isInputSuspended = false;
	timeoutClock = timeout;
}

void SnackbarWindow::backgroundPanelClicked (void *windowPtr, Widget *widgetPtr) {
	SnackbarWindow *window;

	window = (SnackbarWindow *) windowPtr;
	window->isTimeoutDismissed = true;
}

void SnackbarWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (isVisible) {
		if (timeoutClock > 0) {
			if (! isTimeoutSuspended) {
				timeoutClock -= msElapsed;
			}
			if (isTimeoutDismissed) {
				timeoutClock = 0;
				isTimeoutDismissed = false;
			}

			if (timeoutClock <= 0) {
				backgroundPanel->position.translate (0.0f, -(backgroundPanel->height), scrollDuration);
				isInputSuspended = true;
			}
		}
		else {
			if (! backgroundPanel->position.isTranslating) {
				isVisible = false;
				isInputSuspended = false;
			}
		}
	}
}

void SnackbarWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, w;

	uiconfig = &(App::instance->uiConfig);

	x = backgroundPanel->widthPadding;
	y = backgroundPanel->heightPadding;
	messageLabel->position.assign (x, y);
	x += messageLabel->width;
	if (actionButton->isVisible) {
		x += uiconfig->marginSize;
		actionButton->position.assign (x, y);
		x += actionButton->width;
	}
	backgroundPanel->refresh ();

	messageLabel->position.assignY ((backgroundPanel->height / 2.0f) - (messageLabel->height / 2.0f));
	if (actionButton->isVisible) {
		actionButton->position.assignY ((backgroundPanel->height / 2.0f) - (actionButton->height / 2.0f));
	}

	w = backgroundPanel->width;
	if (w > maxWidth) {
		w = maxWidth;
	}
	setFixedSize (true, w, backgroundPanel->height);
}

void SnackbarWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);

	if (mouseState.isEntered) {
		isTimeoutSuspended = true;
	}
	else {
		isTimeoutSuspended = false;
	}
}
