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
#include <math.h>
#include "Result.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiConfiguration.h"
#include "OsUtil.h"
#include "Json.h"
#include "Widget.h"
#include "Sprite.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "TextArea.h"
#include "CardView.h"
#include "MediaThumbnailWindow.h"

MediaThumbnailWindow::MediaThumbnailWindow (int thumbnailIndex, float thumbnailTimestamp, int sourceWidth, int sourceHeight, const StdString &sourceUrl)
: Panel ()
, thumbnailIndex (thumbnailIndex)
, thumbnailTimestamp (thumbnailTimestamp)
, sourceWidth (sourceWidth)
, sourceHeight (sourceHeight)
, sourceUrl (sourceUrl)
, isHighlighted (false)
, thumbnailImage (NULL)
, timestampLabel (NULL)
{
	UiConfiguration *uiconfig;

	classId = ClassId::MediaThumbnailWindow;
	uiconfig = &(App::instance->uiConfig);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	thumbnailImage = (ImageWindow *) addWidget (new ImageWindow (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	thumbnailImage->setLoadSprite (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));
	thumbnailImage->setLoadUrl (sourceUrl);

	timestampLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->inverseTextColor)));
	timestampLabel->zLevel = 1;
	timestampLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	timestampLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	timestampLabel->isVisible = false;
	if (thumbnailTimestamp >= 0.0f) {
		timestampLabel->setText (OsUtil::getDurationString ((int64_t) thumbnailTimestamp, OsUtil::HoursUnit));
	}
}

MediaThumbnailWindow::~MediaThumbnailWindow () {

}

StdString MediaThumbnailWindow::toStringDetail () {
	return (StdString::createSprintf (" MediaThumbnailWindow url=\"%s\"", sourceUrl.c_str ()));
}

bool MediaThumbnailWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::MediaThumbnailWindow));
}

MediaThumbnailWindow *MediaThumbnailWindow::castWidget (Widget *widget) {
	return (MediaThumbnailWindow::isWidgetType (widget) ? (MediaThumbnailWindow *) widget : NULL);
}

void MediaThumbnailWindow::setLayout (int layoutType, float maxPanelWidth) {
	float w, h;

	if (layoutType == layout) {
		return;
	}
	layout = layoutType;
	w = maxPanelWidth;
	h = sourceHeight;
	h *= maxPanelWidth;
	h /= sourceWidth;
	w = floorf (w);
	h = floorf (h);
	thumbnailImage->setWindowSize (w, h);
	thumbnailImage->reload ();

	refreshLayout ();
}

void MediaThumbnailWindow::refreshLayout () {
	float x, y;

	x = 0.0f;
	y = 0.0f;
	thumbnailImage->position.assign (x, y);
	timestampLabel->position.assign (thumbnailImage->position.x, thumbnailImage->position.y + thumbnailImage->height - timestampLabel->height);

	switch (layout) {
		case CardView::LowDetail: {
			timestampLabel->isVisible = false;
			break;
		}
		case CardView::MediumDetail: {
			timestampLabel->isVisible = true;
			break;
		}
		case CardView::HighDetail: {
			timestampLabel->isVisible = true;
			break;
		}
	}

	resetSize ();
}

void MediaThumbnailWindow::setHighlighted (bool highlighted) {
	UiConfiguration *uiconfig;

	if (isHighlighted == highlighted) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isHighlighted = highlighted;
	if (isHighlighted) {
		timestampLabel->translateTextColor (uiconfig->mediumSecondaryColor, uiconfig->shortColorTranslateDuration);
	}
	else {
		timestampLabel->translateTextColor (uiconfig->inverseTextColor, uiconfig->shortColorTranslateDuration);
	}
	refreshLayout ();
}

void MediaThumbnailWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (layout == CardView::LowDetail) {
		if (mouseState.isEntered) {
			timestampLabel->isVisible = true;
		}
		else {
			timestampLabel->isVisible = false;
		}
	}
}
