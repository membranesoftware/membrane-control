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
#include "App.h"
#include "StdString.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "OsUtil.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "LogoWindow.h"

LogoWindow::LogoWindow ()
: Panel ()
, logoImage (NULL)
, networkActivityImage (NULL)
, networkActivityImageHideTime (0)
, dateLabel (NULL)
, timeLabel (NULL)
, lastDisplayTime (0)
{
	Image *image;

	logoImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::AppLogoSprite)));
	logoImage->setDrawColor (true, UiConfiguration::instance->mediumSecondaryColor);

	image = new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::NetworkActivityIconSprite));
	image->setDrawColor (true, UiConfiguration::instance->mediumSecondaryColor);
	image->drawColor.animate (UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->darkBackgroundColor, UiConfiguration::instance->shortColorAnimateDuration);
	networkActivityImage = (ImageWindow *) addWidget (new ImageWindow (image));
	networkActivityImage->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	networkActivityImage->setBorder (true, UiConfiguration::instance->lightInverseBackgroundColor);
	networkActivityImage->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	networkActivityImage->setWindowSize (true, logoImage->width, logoImage->height);
	networkActivityImage->borderColor.animate (UiConfiguration::instance->lightInverseBackgroundColor, UiConfiguration::instance->darkBackgroundColor, UiConfiguration::instance->shortColorAnimateDuration);

	dateLabel = (Label *) addWidget (new Label (OsUtil::getDateString (), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor));
	timeLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor));
	timeLabel->setText (OsUtil::getTimeString ());
	if (! App::instance->isMainToolbarClockEnabled) {
		dateLabel->isVisible = false;
		timeLabel->isVisible = false;
	}

	refreshLayout ();
}

LogoWindow::~LogoWindow () {

}

StdString LogoWindow::toStringDetail () {
	return (StdString (" LogoWindow"));
}

void LogoWindow::doRefresh () {
	if (App::instance->isMainToolbarClockEnabled) {
		dateLabel->setText (OsUtil::getDateString ());
		timeLabel->setText (OsUtil::getTimeString ());
		dateLabel->isVisible = true;
		timeLabel->isVisible = true;
	}
	else {
		dateLabel->isVisible = false;
		timeLabel->isVisible = false;
	}

	Panel::doRefresh ();
}

void LogoWindow::refreshLayout () {
	float x, y;

	x = 0.0f;
	y = 0.0f;
	logoImage->position.assign (x, y);
	x += logoImage->width + UiConfiguration::instance->marginSize;

	if (networkActivityImage->isVisible) {
		networkActivityImage->position.assign (logoImage->position);
	}

	if (dateLabel->isVisible) {
		y += (UiConfiguration::instance->textLineHeightMargin * 2.0f);
		dateLabel->position.assign (x, dateLabel->getLinePosition (y));
		y += dateLabel->maxLineHeight + UiConfiguration::instance->textLineHeightMargin;
	}
	if (timeLabel->isVisible) {
		timeLabel->position.assign (x, timeLabel->getLinePosition (y));
	}

	resetSize ();
}

void LogoWindow::doUpdate (int msElapsed) {
	int64_t now, t;

	Panel::doUpdate (msElapsed);
	now = OsUtil::getTime ();

	if (App::instance->isNetworkActive) {
		networkActivityImage->isVisible = true;
		networkActivityImageHideTime = now + UiConfiguration::instance->activityIconLingerDuration;
	}
	else {
		if (networkActivityImage->isVisible && (now >= networkActivityImageHideTime)) {
			networkActivityImage->isVisible = false;
		}
	}

	if (dateLabel->isVisible || timeLabel->isVisible) {
		t = now / 1000;
		if (lastDisplayTime != t) {
			lastDisplayTime = t;
			dateLabel->setText (OsUtil::getDateString (t * 1000));
			timeLabel->setText (OsUtil::getTimeString (t * 1000));
			refreshLayout ();
		}
	}
}
