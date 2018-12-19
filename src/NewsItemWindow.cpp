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
#include "Panel.h"
#include "Label.h"
#include "LabelWindow.h"
#include "TextArea.h"
#include "Button.h"
#include "Image.h"
#include "UiConfiguration.h"
#include "NewsItemWindow.h"

NewsItemWindow::NewsItemWindow (float windowWidth, Sprite *iconSprite, const StdString &itemTitle, const StdString &itemText, bool shouldCopyIconSprite)
: Panel ()
, createTime (0)
, windowWidth (windowWidth)
, iconImage (NULL)
, titleLabel (NULL)
, timestampLabel (NULL)
, detailText (NULL)
, deleteButton (NULL)
, deleteCallback (NULL)
, deleteCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	createTime = Util::getTime ();
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->darkBackgroundColor);

	if (shouldCopyIconSprite) {
		iconImage = (Image *) addWidget (new Image (iconSprite->copy (), 0, true));
	}
	else {
		iconImage = (Image *) addWidget (new Image (iconSprite));
	}
	titleLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (itemTitle, UiConfiguration::BODY, uiconfig->primaryTextColor)));
	titleLabel->setPadding (0.0f, uiconfig->textLineHeightMargin * 2.0f);

	timestampLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (Util::getTimestampDisplayString (createTime), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor)));
	timestampLabel->setPadding (0.0f, uiconfig->textLineHeightMargin * 2.0f);

	detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor, uiconfig->textAreaMediumLineLength, windowWidth - iconImage->width - (uiconfig->paddingSize * 2.0f)));
	detailText->setText (itemText);

	deleteButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON)));
	deleteButton->setMouseClickCallback (NewsItemWindow::deleteButtonClicked, this);
	deleteButton->setImageColor (uiconfig->flatButtonTextColor);

	refreshLayout ();
}

NewsItemWindow::~NewsItemWindow () {

}

StdString NewsItemWindow::toStringDetail () {
	return (StdString (" NewsItemWindow"));
}

void NewsItemWindow::setWindowWidth (float windowWidthValue) {
	UiConfiguration *uiconfig;

	if (FLOAT_EQUALS (windowWidthValue, windowWidth)) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	windowWidth = windowWidthValue;
	detailText->setMaxLineWidth (windowWidth - iconImage->width - (uiconfig->paddingSize * 2.0f));
	refreshLayout ();
}

void NewsItemWindow::setDeleteCallback (Widget::EventCallback callback, void *callbackData) {
	deleteCallback = callback;
	deleteCallbackData = callbackData;
}

void NewsItemWindow::setDetailText (const StdString &text) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	if (text.empty ()) {
		if (detailText) {
			detailText->isDestroyed = true;
			detailText = NULL;
			refreshLayout ();
		}
	}
	else {
		if (! detailText) {
			detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->primaryTextColor));
		}
		detailText->setText (text);
		refreshLayout ();
	}
}

void NewsItemWindow::deleteButtonClicked (void *windowPtr, Widget *widgetPtr) {
	NewsItemWindow *window;

	window = (NewsItemWindow *) windowPtr;
	if (window->deleteCallback) {
		window->deleteCallback (window->deleteCallbackData, window);
	}
}

void NewsItemWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, w, h;

	uiconfig = &(App::getInstance ()->uiConfig);
	y = uiconfig->paddingSize;

	x = windowWidth - uiconfig->paddingSize;
	x -= iconImage->width;
	iconImage->position.assign (x, y);

	x = uiconfig->paddingSize;
	w = windowWidth - x - iconImage->width - (uiconfig->paddingSize * 2.0f);
	timestampLabel->position.assign (x, y);
	timestampLabel->setFixedSize (true, w, timestampLabel->height);
	y += timestampLabel->height + uiconfig->textLineHeightMargin;

	w = windowWidth - x - iconImage->width - (uiconfig->paddingSize * 2.0f);
	titleLabel->position.assign (x, y);
	titleLabel->setFixedSize (true, w, titleLabel->height);
	y += titleLabel->height + uiconfig->textLineHeightMargin;

	x = uiconfig->paddingSize;
	if (detailText) {
		detailText->position.assign (x, y);
	}
	y += detailText->height;

	x = windowWidth - uiconfig->paddingSize;
	x -= deleteButton->width;
	deleteButton->position.assign (x, y);

	resetSize ();
	h = maxWidgetY + uiconfig->paddingSize;
	setFixedSize (true, windowWidth, h);
}
