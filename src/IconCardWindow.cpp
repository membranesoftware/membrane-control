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
#include "TextArea.h"
#include "Image.h"
#include "UiConfiguration.h"
#include "HyperlinkWindow.h"
#include "IconCardWindow.h"

IconCardWindow::IconCardWindow (Sprite *iconSprite, const StdString &cardName, const StdString &cardSubtitle, const StdString &cardDetailText)
: Panel ()
, iconImage (NULL)
, nameLabel (NULL)
, subtitleLabel (NULL)
, detailText (NULL)
, linkWindow (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (iconSprite));
	nameLabel = (Label *) addWidget (new Label (cardName, UiConfiguration::HEADLINE, uiconfig->primaryTextColor));
	if (! cardSubtitle.empty ()) {
		subtitleLabel = (Label *) addWidget (new Label (cardSubtitle, UiConfiguration::CAPTION, uiconfig->primaryTextColor));
	}
	if (! cardDetailText.empty ()) {
		detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->primaryTextColor));
		detailText->setText (cardDetailText);
	}

	refreshLayout ();
}

IconCardWindow::~IconCardWindow () {

}

StdString IconCardWindow::toStringDetail () {
	return (StdString (" IconCardWindow"));
}

void IconCardWindow::setDetailText (const StdString &text) {
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

void IconCardWindow::setLink (const StdString &text, const StdString &url) {
	if (linkWindow) {
		linkWindow->isDestroyed = true;
	}
	linkWindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow (text, url));
	refreshLayout ();
}

void IconCardWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, ymin;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = widthPadding;
	y = heightPadding;
	ymin = 0.0f;

	iconImage->position.assign (x, y);
	x += iconImage->width + uiconfig->marginSize;
	ymin = y + iconImage->height + uiconfig->marginSize;

	if (subtitleLabel) {
		nameLabel->position.assign (x, y);
		y += nameLabel->maxLineHeight + uiconfig->textLineHeightMargin;
		subtitleLabel->position.assign (x, y);
		y += subtitleLabel->maxLineHeight;
	}
	else {
		nameLabel->position.assign (x, y + (iconImage->height / 2.0f) - (nameLabel->maxLineHeight / 2.0f));
		y += nameLabel->maxLineHeight + uiconfig->textLineHeightMargin;
	}

	if (y < ymin) {
		y = ymin;
	}

	x = widthPadding;
	if (detailText) {
		detailText->position.assign (x, y);
		y += detailText->height + uiconfig->marginSize;
	}
	if (linkWindow) {
		linkWindow->position.assign (x, y);
		y += linkWindow->height + uiconfig->marginSize;
	}

	resetSize ();

	x = width - widthPadding;
	if (linkWindow) {
		linkWindow->position.assignX (x - linkWindow->width);
		x -= (linkWindow->width + uiconfig->marginSize);
	}
}
