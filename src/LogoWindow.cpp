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
#include "StdString.h"
#include "App.h"
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
, dateLabel (NULL)
, timeLabel (NULL)
, lastDisplayTime (0)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	logoImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::APP_LOGO)));
	logoImage->setDrawColor (true, uiconfig->mediumSecondaryColor);

	dateLabel = (Label *) addWidget (new Label (OsUtil::getDateString (), UiConfiguration::CAPTION, uiconfig->inverseTextColor));
	timeLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->inverseTextColor));
	timeLabel->setText (OsUtil::getTimeString ());
	if (! App::instance->prefsMap.find (App::prefsShowClock, false)) {
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
	if (App::instance->prefsMap.find (App::prefsShowClock, false)) {
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
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	x = 0.0f;
	y = 0.0f;
	logoImage->position.assign (x, y);
	x += logoImage->width + uiconfig->marginSize;

	if (dateLabel->isVisible) {
		y += (uiconfig->textLineHeightMargin * 2.0f);
		dateLabel->position.assign (x, dateLabel->getLinePosition (y));
		y += dateLabel->maxLineHeight + uiconfig->textLineHeightMargin;
	}
	if (timeLabel->isVisible) {
		timeLabel->position.assign (x, timeLabel->getLinePosition (y));
	}

	resetSize ();
}

void LogoWindow::doUpdate (int msElapsed, float originX, float originY) {
	int64_t t;

	Panel::doUpdate (msElapsed, originX, originY);

	if (dateLabel->isVisible || timeLabel->isVisible) {
		t = OsUtil::getTime ();
		t /= 1000;
		if (lastDisplayTime != t) {
			lastDisplayTime = t;
			dateLabel->setText (OsUtil::getDateString (t * 1000));
			timeLabel->setText (OsUtil::getTimeString (t * 1000));
			refreshLayout ();
		}
	}
}
