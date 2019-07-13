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
#include <vector>
#include <list>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "OsUtil.h"
#include "Widget.h"
#include "Color.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Panel.h"
#include "UiConfiguration.h"
#include "CameraTimelineUi.h"
#include "CameraTimelineWindow.h"

CameraTimelineWindow::CameraTimelineWindow (float barWidth)
: Panel ()
, isDisabled (false)
, startTime (0)
, endTime (0)
, highlightTime (-1)
, selectTime (-1)
, hoverPosition (0.0f)
, clickPosition (0.0f)
, barWidth (barWidth)
, barHeight (0.0f)
, startTimeLabel (NULL)
, endTimeLabel (NULL)
, highlightTimeLabel (NULL)
, highlightMarker (NULL)
, selectTimeLabel (NULL)
, selectMarker (NULL)
, positionHoverCallback (NULL)
, positionHoverCallbackData (NULL)
, positionClickCallback (NULL)
, positionClickCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	setFillBg (true, uiconfig->lightBackgroundColor);

	startTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->darkBackgroundColor)));
	startTimeLabel->setPadding (uiconfig->paddingSize / 2.0f, uiconfig->textLineHeightMargin);
	startTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	startTimeLabel->isInputSuspended = true;
	startTimeLabel->zLevel = 3;
	startTimeLabel->isVisible = false;

	endTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->darkBackgroundColor)));
	endTimeLabel->setPadding (uiconfig->paddingSize / 2.0, uiconfig->textLineHeightMargin);
	endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	endTimeLabel->isInputSuspended = true;
	endTimeLabel->zLevel = 3;
	endTimeLabel->isVisible = false;

	highlightTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->mediumSecondaryColor)));
	highlightTimeLabel->setPadding (uiconfig->paddingSize / 2.0, uiconfig->textLineHeightMargin);
	highlightTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	highlightTimeLabel->isInputSuspended = true;
	highlightTimeLabel->zLevel = 3;
	highlightTimeLabel->isVisible = false;

	selectTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->darkBackgroundColor)));
	selectTimeLabel->setPadding (uiconfig->paddingSize / 2.0, uiconfig->textLineHeightMargin);
	selectTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	selectTimeLabel->isInputSuspended = true;
	selectTimeLabel->zLevel = 3;
	selectTimeLabel->isVisible = false;

	selectMarker = (Panel *) addWidget (new Panel ());
	selectMarker->setFillBg (true, uiconfig->darkSecondaryColor);
	selectMarker->isInputSuspended = true;
	selectMarker->zLevel = 1;
	selectMarker->isVisible = false;

	refreshLayout ();
}

CameraTimelineWindow::~CameraTimelineWindow () {

}

void CameraTimelineWindow::setPositionHoverCallback (Widget::EventCallback callback, void *callbackData) {
	positionHoverCallback = callback;
	positionHoverCallbackData = callbackData;
}

void CameraTimelineWindow::setPositionClickCallback (Widget::EventCallback callback, void *callbackData) {
	positionClickCallback = callback;
	positionClickCallbackData = callbackData;
}

void CameraTimelineWindow::setDisabled (bool disabled) {
	UiConfiguration *uiconfig;

	if (disabled == isDisabled) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isDisabled = disabled;
	if (isDisabled) {
		setFillBg (true, uiconfig->lightBackgroundColor.copy (uiconfig->buttonDisabledShadeAlpha));
		startTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f));
		endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f));
		selectTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f));
		selectMarker->setFillBg (true, uiconfig->darkSecondaryColor.copy (uiconfig->buttonDisabledShadeAlpha));
	}
	else {
		setFillBg (true, uiconfig->lightBackgroundColor);
		startTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
		endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
		selectTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
		selectMarker->setFillBg (true, uiconfig->darkSecondaryColor);
	}
	refreshLayout ();
}

void CameraTimelineWindow::refreshLayout () {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	barHeight = startTimeLabel->height * 2.0f;
	setFixedSize (true, barWidth, barHeight);

	if ((selectTime >= 0) && (highlightTime < 0)) {
		selectTimeLabel->isVisible = true;
	}
	else {
		selectTimeLabel->isVisible = false;
	}

	if (selectMarker->isVisible) {
		selectMarker->setFixedSize (true, uiconfig->timelineMarkerWidth, barHeight - 2.0f);
		selectMarker->position.assignY (1.0f);
	}
	startTimeLabel->position.assign (0.0f, barHeight - startTimeLabel->height);
	endTimeLabel->position.assign (barWidth - endTimeLabel->width, barHeight - endTimeLabel->height);
}

void CameraTimelineWindow::setTimespan (int64_t spanStartTime, int64_t spanEndTime) {
	if ((spanStartTime <= 0) || (spanEndTime <= 0)) {
		return;
	}
	startTime = spanStartTime;
	endTime = spanEndTime;
	startTimeLabel->setText (OsUtil::getTimestampDisplayString (startTime));
	endTimeLabel->setText (OsUtil::getTimestampDisplayString (endTime));
	startTimeLabel->isVisible = true;
	endTimeLabel->isVisible = true;
	refreshLayout ();
}

void CameraTimelineWindow::setHighlightedTime (int64_t highlightTimeValue) {
	UiConfiguration *uiconfig;
	float x;

	if ((width <= 0.0f) || (startTime >= endTime)) {
		return;
	}
	uiconfig = &(App::instance->uiConfig);
	highlightTime = highlightTimeValue;
	if (highlightMarker) {
		highlightMarker->bgColor.translate (uiconfig->lightBackgroundColor, uiconfig->shortColorTranslateDuration);
		highlightMarker->setDestroyDelay (uiconfig->shortColorTranslateDuration);
		highlightMarker = NULL;
	}
	if ((highlightTime < startTime) || (highlightTime > endTime)) {
		highlightTime = -1;
		highlightTimeLabel->isVisible = false;
	}
	else {
		x = (float) (highlightTime - startTime);
		x *= width;
		x /= (float) (endTime - startTime);
		highlightMarker = (Panel *) addWidget (new Panel (), x - (uiconfig->timelineMarkerWidth / 2.0f), 1.0f);
		highlightMarker->setFillBg (true, uiconfig->lightBackgroundColor);
		highlightMarker->bgColor.translate (uiconfig->darkPrimaryColor, uiconfig->shortColorTranslateDuration);
		highlightMarker->setFixedSize (true, uiconfig->timelineMarkerWidth, barHeight - 2.0f);
		highlightMarker->isInputSuspended = true;
		highlightMarker->zLevel = 2;

		highlightTimeLabel->setText (OsUtil::getTimestampDisplayString (highlightTime));
		highlightTimeLabel->position.assignBounded (x - (highlightTimeLabel->width / 2.0f), 0.0f, 0.0f, 0.0f, width - highlightTimeLabel->width, 0.0f);
		highlightTimeLabel->isVisible = true;
	}

	refreshLayout ();
}

void CameraTimelineWindow::setSelectedTime (int64_t selectTimeValue, bool isSpanDescending) {
	float x;

	if ((width <= 0.0f) || (startTime >= endTime)) {
		return;
	}

	selectTime = selectTimeValue;
	if ((selectTime < startTime) || (selectTime > endTime)) {
		selectTime = -1;
		selectMarker->isVisible = false;
	}
	else {
		x = (float) (selectTime - startTime);
		x *= width;
		x /= (float) (endTime - startTime);
		selectMarker->position.assignX (x - (selectMarker->width / 2.0f));
		selectMarker->isVisible = true;

		if (isSpanDescending) {
			selectTimeLabel->setText (StdString::createSprintf ("< %s", OsUtil::getTimestampDisplayString (selectTime).c_str ()));
		}
		else {
			selectTimeLabel->setText (StdString::createSprintf ("%s >", OsUtil::getTimestampDisplayString (selectTime).c_str ()));
		}
		selectTimeLabel->position.assignBounded (x - (selectTimeLabel->width / 2.0f), 0.0f, 0.0f, 0.0f, width - selectTimeLabel->width, 0.0f);
	}

	refreshLayout ();
}

void CameraTimelineWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);

	if (isDisabled) {
		return;
	}

	if (mouseState.isEntered) {
		if (! FLOAT_EQUALS (hoverPosition, mouseState.enterDeltaX)) {
			hoverPosition = mouseState.enterDeltaX;
			if (positionHoverCallback) {
				positionHoverCallback (positionHoverCallbackData, this);
			}
		}

		if (mouseState.isLeftClicked) {
			clickPosition = mouseState.enterDeltaX;
			if (positionClickCallback) {
				positionClickCallback (positionClickCallbackData, this);
			}
		}
	}
	else {
		if (hoverPosition >= 0.0f) {
			hoverPosition = -1.0f;
			if (positionHoverCallback) {
				positionHoverCallback (positionHoverCallbackData, this);
			}
		}
	}
}
