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
#include "Util.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "UiConfiguration.h"
#include "TimelineBar.h"

const int TimelineBar::segmentCount = 4;

TimelineBar::TimelineBar (float barWidth, float startTime, float endTime)
: Panel ()
, barWidth (barWidth)
, startTime (startTime)
, endTime (endTime)
, minDurationUnitType (0)
{
	UiConfiguration *uiconfig;

	typeName.assign ("TimelineBar");

	uiconfig = &(App::getInstance ()->uiConfig);
	setFillBg (true, uiconfig->lightBackgroundColor);

	if (startTime < 0.0f) {
		startTime = 0.0f;
	}
	if (endTime < 0.0f) {
		endTime = 0.0f;
	}
	if (endTime < startTime) {
		endTime = startTime;
	}
	minDurationUnitType = Util::getDurationMinUnitType (endTime - startTime);

	resetLayout ();
}

TimelineBar::~TimelineBar () {

}

void TimelineBar::resetLayout () {
	UiConfiguration *uiconfig;
	Panel *linepanel;
	Label *label;
	float x, y, dx;

	uiconfig = &(App::getInstance ()->uiConfig);
	destroyAllChildWidgets ();

	x = 0.0f;
	dx = barWidth / (float) TimelineBar::segmentCount;
	if (dx < 1.0f) {
		dx = 1.0f;
	}
	x += dx;
	while (x < barWidth) {
		linepanel = (Panel *) addWidget (new Panel (), x, 0.0f);
		linepanel->setFillBg (true, uiconfig->primaryTextColor);
		// TODO: Use a different size value here (i.e. a UiConfiguration field dedicated for this purpose, instead of marginSize)
		linepanel->setFixedSize (true, 1.0f, (uiconfig->marginSize * 2.0f));
		x += dx;
	}

	x = uiconfig->paddingSize / 2.0f;
	y = uiconfig->paddingSize;
	label = (Label *) addWidget (new Label (Util::getDurationString (startTime, minDurationUnitType), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
	label->position.assign (x, y);

	x = barWidth - (uiconfig->paddingSize / 2.0f);
	label = (Label *) addWidget (new Label (Util::getDurationString (endTime, minDurationUnitType), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
	label->position.assign (x - label->width, y);

	resetSize ();
	setFixedSize (true, barWidth, maxWidgetY + uiconfig->paddingSize);
}
