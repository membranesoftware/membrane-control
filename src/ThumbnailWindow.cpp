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
#include <math.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "OsUtil.h"
#include "Widget.h"
#include "Sprite.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "TextArea.h"
#include "Json.h"
#include "UiConfiguration.h"
#include "ThumbnailWindow.h"

ThumbnailWindow::ThumbnailWindow (int thumbnailIndex, float thumbnailTimestamp, int sourceWidth, int sourceHeight, const StdString &sourceUrl, int layoutType, float maxMediaImageWidth)
: Panel ()
, thumbnailIndex (thumbnailIndex)
, thumbnailTimestamp (thumbnailTimestamp)
, sourceWidth (sourceWidth)
, sourceHeight (sourceHeight)
, sourceUrl (sourceUrl)
, isHighlighted (false)
, mediaImage (NULL)
, timestampLabel (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	mediaImage = (ImageWindow *) addWidget (new ImageWindow (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LoadingImageIconSprite))));
	mediaImage->setLoadUrl (sourceUrl, uiconfig->coreSprites.getSprite (UiConfiguration::LoadingImageIconSprite));

	timestampLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->inverseTextColor)));
	timestampLabel->zLevel = 1;
	timestampLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	timestampLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	timestampLabel->isVisible = false;
	if (thumbnailTimestamp >= 0.0f) {
		timestampLabel->setText (OsUtil::getDurationString ((int64_t) thumbnailTimestamp, OsUtil::HoursUnit));
	}

	setLayout (layoutType, maxMediaImageWidth);
}

ThumbnailWindow::~ThumbnailWindow () {

}

StdString ThumbnailWindow::toStringDetail () {
	return (StdString::createSprintf (" ThumbnailWindow url=\"%s\"", sourceUrl.c_str ()));
}

bool ThumbnailWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" ThumbnailWindow url="));
}

ThumbnailWindow *ThumbnailWindow::castWidget (Widget *widget) {
	return (ThumbnailWindow::isWidgetType (widget) ? (ThumbnailWindow *) widget : NULL);
}

void ThumbnailWindow::setLayout (int layoutType, float maxImageWidth) {
	float w, h;

	if (layoutType == layout) {
		return;
	}
	layout = layoutType;
	w = maxImageWidth;
	h = sourceHeight;
	h *= maxImageWidth;
	h /= sourceWidth;
	w = floorf (w);
	h = floorf (h);
	mediaImage->setWindowSize (w, h);
	mediaImage->reload ();

	refreshLayout ();
}

void ThumbnailWindow::refreshLayout () {
	float x, y;

	x = 0.0f;
	y = 0.0f;
	mediaImage->position.assign (x, y);
	timestampLabel->position.assign (mediaImage->position.x, mediaImage->position.y + mediaImage->height - timestampLabel->height);

	switch (layout) {
		case ThumbnailWindow::LowDetailLayout: {
			timestampLabel->isVisible = false;
			break;
		}
		case ThumbnailWindow::MediumDetailLayout: {
			timestampLabel->isVisible = true;
			break;
		}
		case ThumbnailWindow::HighDetailLayout: {
			timestampLabel->isVisible = true;
			break;
		}
	}

	resetSize ();
}

void ThumbnailWindow::setHighlighted (bool highlighted) {
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

void ThumbnailWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (layout == ThumbnailWindow::LowDetailLayout) {
		if (mouseState.isEntered) {
			timestampLabel->isVisible = true;
		}
		else {
			timestampLabel->isVisible = false;
		}
	}
}
