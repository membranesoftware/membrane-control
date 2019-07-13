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
#include "App.h"
#include "ClassId.h"
#include "StdString.h"
#include "Label.h"
#include "Panel.h"
#include "BannerWindow.h"

BannerWindow::BannerWindow (float windowWidth)
: Panel ()
, windowWidth (windowWidth)
, messageLabel (NULL)
, iconImage (NULL)
, actionButton (NULL)
{
	UiConfiguration *uiconfig;

	classId = ClassId::BannerWindow;
	uiconfig = &(App::instance->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize / 2.0f);
	setFillBg (true, uiconfig->darkPrimaryColor);

	messageLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::ButtonFont, uiconfig->inverseTextColor)));
	messageLabel->setPadding (0.0f, 0.0f);

	actionButton = (Button *) addWidget (new Button ());
	actionButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	actionButton->setTextColor (uiconfig->raisedButtonTextColor);
	actionButton->zLevel = 2;
	actionButton->isVisible = false;

	refreshLayout ();
}

BannerWindow::~BannerWindow () {

}

StdString BannerWindow::toStringDetail () {
	return (StdString (" BannerWindow"));
}

bool BannerWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::BannerWindow));
}

BannerWindow *BannerWindow::castWidget (Widget *widget) {
	return (BannerWindow::isWidgetType (widget) ? (BannerWindow *) widget : NULL);
}

void BannerWindow::setWindowWidth (float fixedWidth) {
	windowWidth = fixedWidth;
	refreshLayout ();
}

void BannerWindow::setBanner (const StdString &messageText, Sprite *iconSprite, const StdString &actionText, Widget::EventCallback actionClickCallback, void *actionClickCallbackData) {
	UiConfiguration *uiconfig;
	float w;

	uiconfig = &(App::instance->uiConfig);
	messageLabel->setFitWidth ();
	messageLabel->setCrawl (false);
	messageLabel->setText (messageText);
	messageLabel->translateTextColor (bgColor, uiconfig->inverseTextColor, uiconfig->longColorTranslateDuration);
	w = windowWidth - (widthPadding * 2.0f);

	if (iconImage) {
		iconImage->isDestroyed = true;
		iconImage = NULL;
	}
	if (iconSprite) {
		iconImage = (Image *) addWidget (new Image (iconSprite));
		iconImage->setDrawColor (true, uiconfig->mediumSecondaryColor);
		iconImage->translateAlpha (0.0f, 1.0f, uiconfig->longColorTranslateDuration);
		w -= (iconImage->width + uiconfig->marginSize);
	}
	if ((! actionClickCallback) || actionText.empty ()) {
		actionButton->isVisible = false;
	}
	else {
		actionButton->setText (actionText.uppercased ());
		actionButton->setMouseClickCallback (actionClickCallback, actionClickCallbackData);
		actionButton->isVisible = true;
		w -= (actionButton->width + uiconfig->marginSize);
	}

	if (messageLabel->width > w) {
		messageLabel->setWindowWidth (w);
		messageLabel->setCrawl (true);
	}
	refreshLayout ();
}

void BannerWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;

	if (iconImage) {
		iconImage->flowRight (&x, y);
	}
	messageLabel->flowRight (&x, y);
	if (actionButton->isVisible) {
		actionButton->flowRight (&x, y);
	}
	setFixedSize (true, windowWidth, messageLabel->height + (uiconfig->paddingSize * 2.0f));
	messageLabel->position.assignY ((height / 2.0f) - (messageLabel->height / 2.0f));
	if (iconImage) {
		iconImage->position.assignY ((height / 2.0f) - (iconImage->height / 2.0f));
	}
	if (actionButton->isVisible) {
		actionButton->position.assignY ((height / 2.0f) - (actionButton->height / 2.0f));
	}
}
