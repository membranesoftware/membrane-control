/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "TextFlow.h"
#include "Image.h"
#include "UiConfiguration.h"
#include "HyperlinkWindow.h"
#include "IconCardWindow.h"

IconCardWindow::IconCardWindow (Sprite *iconSprite)
: Panel ()
, iconImage (NULL)
, nameLabel (NULL)
, subtitleLabel (NULL)
, detailText (NULL)
, linkWindow (NULL)
{
	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (iconSprite));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::HeadlineFont, UiConfiguration::instance->primaryTextColor));
	subtitleLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));

	refreshLayout ();
}

IconCardWindow::~IconCardWindow () {

}

StdString IconCardWindow::toStringDetail () {
	return (StdString (" IconCardWindow"));
}

void IconCardWindow::setName (const StdString &text, UiConfiguration::FontType fontType) {
	nameLabel->setText (text);
	if (fontType != UiConfiguration::NoFont) {
		nameLabel->setFont (fontType);
	}
	refreshLayout ();
}

void IconCardWindow::setSubtitle (const StdString &text, UiConfiguration::FontType fontType) {
	subtitleLabel->setText (text);
	if (fontType != UiConfiguration::NoFont) {
		subtitleLabel->setFont (fontType);
	}
	subtitleLabel->isVisible = (! subtitleLabel->text.empty ());
	refreshLayout ();
}

void IconCardWindow::setSubtitleColor (const Color &color) {
	subtitleLabel->textColor.assign (color);
}

void IconCardWindow::setDetailText (const StdString &text, UiConfiguration::FontType fontType) {
	if (text.empty ()) {
		if (detailText) {
			detailText->isDestroyed = true;
			detailText = NULL;
			refreshLayout ();
		}
	}
	else {
		if (! detailText) {
			detailText = (TextFlow *) addWidget (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
		}
		detailText->setText (text);
		if (fontType != UiConfiguration::NoFont) {
			detailText->setFont (fontType);
		}
		refreshLayout ();
	}
}

void IconCardWindow::setLink (const StdString &text, const StdString &url) {
	if (linkWindow) {
		linkWindow->isDestroyed = true;
	}
	linkWindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow (text, url));
	linkWindow->linkOpenCallback = Widget::EventCallbackContext (App::hyperlinkOpened, this);
	refreshLayout ();
}

void IconCardWindow::refreshLayout () {
	float x, y, ymin;

	x = widthPadding;
	y = heightPadding;
	ymin = 0.0f;

	iconImage->position.assign (x, y);
	x += iconImage->width + UiConfiguration::instance->marginSize;
	ymin = y + iconImage->height + UiConfiguration::instance->marginSize;

	if (subtitleLabel->isVisible) {
		nameLabel->position.assign (x, y);
		y += nameLabel->maxLineHeight + UiConfiguration::instance->textLineHeightMargin;
		subtitleLabel->position.assign (x, y);
		y += subtitleLabel->maxLineHeight;
	}
	else {
		nameLabel->position.assign (x, y + (iconImage->height / 2.0f) - (nameLabel->maxLineHeight / 2.0f));
		y += nameLabel->maxLineHeight + UiConfiguration::instance->textLineHeightMargin;
	}

	if (y < ymin) {
		y = ymin;
	}

	x = widthPadding;
	if (detailText) {
		detailText->position.assign (x, y);
		y += detailText->height + UiConfiguration::instance->marginSize;
	}
	if (linkWindow) {
		linkWindow->position.assign (x, y);
		y += linkWindow->height + UiConfiguration::instance->marginSize;
	}

	resetSize ();

	x = width - widthPadding;
	if (linkWindow) {
		linkWindow->position.assignX (x - linkWindow->width);
		x -= (linkWindow->width + UiConfiguration::instance->marginSize);
	}
}
