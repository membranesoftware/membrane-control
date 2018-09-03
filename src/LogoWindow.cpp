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
#include "Color.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "UiConfiguration.h"
#include "LogoWindow.h"

LogoWindow::LogoWindow ()
: Panel ()
, logoImage (NULL)
, dateLabel (NULL)
, timeLabel (NULL)
, timeLabelMaxWidth (0.0f)
{
	App *app;
	UiConfiguration *uiconfig;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	logoImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::APP_LOGO)));
	logoImage->setDrawColor (true, uiconfig->mediumSecondaryColor);

	dateLabel = (Label *) addWidget (new Label (Util::getDateString (), UiConfiguration::CAPTION, uiconfig->inverseTextColor));
	timeLabel = (Label *) addWidget (new Label (StdString ("99:99:99"), UiConfiguration::CAPTION, uiconfig->inverseTextColor));
	timeLabelMaxWidth = timeLabel->width;
	timeLabel->setText (Util::getTimeString ());
	if (! app->prefsMap.find (App::prefsShowClock, false)) {
		dateLabel->isVisible = false;
		timeLabel->isVisible = false;
	}

	resetLayout ();
}

LogoWindow::~LogoWindow () {

}

StdString LogoWindow::toStringDetail () {
	return (StdString (" LogoWindow"));
}

void LogoWindow::doRefresh () {
	App *app;

	app = App::getInstance ();
	if (app->prefsMap.find (App::prefsShowClock, false)) {
		dateLabel->setText (Util::getDateString ());
		timeLabel->setText (Util::getTimeString ());
		dateLabel->isVisible = true;
		timeLabel->isVisible = true;
	}
	else {
		dateLabel->isVisible = false;
		timeLabel->isVisible = false;
	}

	Panel::doRefresh ();
}

void LogoWindow::resetLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = 0.0f;
	y = 0.0f;
	logoImage->position.assign (x, y);
	x += logoImage->width + uiconfig->marginSize;

	if (dateLabel->isVisible) {
		y += (uiconfig->textLineHeightMargin * 2.0f);
		dateLabel->position.assign (x, dateLabel->getLinePosition (y));
		y += dateLabel->maxLineHeight + uiconfig->textLineHeightMargin;
		x += dateLabel->width;
	}
	if (timeLabel->isVisible) {
		timeLabel->position.assign (x - timeLabelMaxWidth, timeLabel->getLinePosition (y));
	}

	resetSize ();
}

void LogoWindow::doUpdate (int msElapsed, float originX, float originY) {
	if (dateLabel->isVisible) {
		dateLabel->setText (Util::getDateString ());
	}
	if (timeLabel->isVisible) {
		timeLabel->setText (Util::getTimeString ());
	}
	Panel::doUpdate (msElapsed, originX, originY);

	resetLayout ();
}
