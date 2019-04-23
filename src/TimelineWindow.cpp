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
#include "Panel.h"
#include "Label.h"
#include "UiConfiguration.h"
#include "TimelineWindow.h"

const int TimelineWindow::guideSegmentCount = 4;

TimelineWindow::TimelineWindow (float barWidth, const StdString &recordId)
: Panel ()
, recordId (recordId)
, recordType (-1)
, highlightedMarkerIndex (-1)
, hoverPosition (-1.0f)
, clickPosition (-1.0f)
, streamHighlightMarker (NULL)
, startTimeLabel (NULL)
, endTimeLabel (NULL)
, highlightTimeLabel (NULL)
, barWidth (barWidth)
, barHeight (0.0f)
, duration (0.0f)
, minDurationUnitType (0)
, thumbnailCount (0)
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
	startTimeLabel->zLevel = 2;

	endTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->darkBackgroundColor)));
	endTimeLabel->setPadding (uiconfig->paddingSize / 2.0, uiconfig->textLineHeightMargin);
	endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	endTimeLabel->isInputSuspended = true;
	endTimeLabel->zLevel = 2;

	highlightTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->mediumSecondaryColor)));
	highlightTimeLabel->setPadding (uiconfig->paddingSize / 2.0, uiconfig->textLineHeightMargin);
	highlightTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	highlightTimeLabel->isInputSuspended = true;
	highlightTimeLabel->zLevel = 3;
	highlightTimeLabel->isVisible = false;

	barHeight = startTimeLabel->height * 2.0f;

	refreshLayout ();
}

TimelineWindow::~TimelineWindow () {

}

StdString TimelineWindow::toStringDetail () {
	return (StdString (" TimelineWindow"));
}

void TimelineWindow::setPositionHoverCallback (Widget::EventCallback callback, void *callbackData) {
	positionHoverCallback = callback;
	positionHoverCallbackData = callbackData;
}

void TimelineWindow::setPositionClickCallback (Widget::EventCallback callback, void *callbackData) {
	positionClickCallback = callback;
	positionClickCallbackData = callbackData;
}

void TimelineWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	Json *record, *agentstatus, serverstatus;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);

	record = store->findRecord (recordId, SystemInterface::CommandId_MediaItem);
	if (record) {
		recordType = SystemInterface::CommandId_MediaItem;
		agentId = interface->getCommandAgentId (record);
		duration = interface->getCommandNumberParam (record, "duration", (float) 0.0f);
		agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentId);
		if (agentstatus) {
			if (interface->getCommandObjectParam (agentstatus, "mediaServerStatus", &serverstatus)) {
				thumbnailCount = serverstatus.getNumber ("thumbnailCount", (int) 0);
			}
		}
	}

	if (! record) {
		record = store->findRecord (recordId, SystemInterface::CommandId_StreamItem);
		if (record) {
			recordType = SystemInterface::CommandId_StreamItem;
			agentId = interface->getCommandAgentId (record);
			duration = interface->getCommandNumberParam (record, "duration", (float) 0.0f);
			thumbnailCount = interface->getCommandNumberParam (record, "segmentCount", (int) 0);
			interface->getCommandNumberArrayParam (record, "segmentPositions", &segmentPositionList, true);
		}
	}

	if (! record) {
		return;
	}
	minDurationUnitType = OsUtil::getDurationMinUnitType (duration);
	startTimeLabel->setText (OsUtil::getDurationString (0.0f, minDurationUnitType));
	endTimeLabel->setText (OsUtil::getDurationString (duration, minDurationUnitType));
	populateMarkers ();
	refreshLayout ();
}

void TimelineWindow::populateMarkers () {
	if ((! markerList.empty ()) || (recordType < 0)) {
		return;
	}

	switch (recordType) {
		case SystemInterface::CommandId_MediaItem: {
			UiConfiguration *uiconfig;
			Panel *panel;
			float x, dx;
			int i;

			if (thumbnailCount > 0) {
				uiconfig = &(App::instance->uiConfig);
				dx = barWidth / (float) thumbnailCount;
				if (dx < 1.0f) {
					dx = 1.0f;
				}
				x = (dx / 2.0f);
				for (i = 0; i < thumbnailCount; ++i) {
					panel = (Panel *) addWidget (new Panel (), x - (uiconfig->timelineMarkerWidth / 2.0f), 1.0f);
					panel->setFillBg (true, uiconfig->darkInverseBackgroundColor);
					panel->setFixedSize (true, uiconfig->timelineMarkerWidth, barHeight - 2.0f);
					panel->isInputSuspended = true;
					markerList.push_back (panel);
					x += dx;
				}
			}
			break;
		}
		case SystemInterface::CommandId_StreamItem: {
			UiConfiguration *uiconfig;
			Panel *panel;
			float x, dx;

			uiconfig = &(App::instance->uiConfig);
			x = 0.0f;
			dx = barWidth / (float) TimelineWindow::guideSegmentCount;
			if (dx < 1.0f) {
				dx = 1.0f;
			}
			x += dx;
			while (x < barWidth) {
				panel = (Panel *) addWidget (new Panel (), x, 1.0f);
				panel->setFillBg (true, uiconfig->primaryTextColor);
				panel->setFixedSize (true, 1.0f, barHeight - 2.0f);
				panel->isInputSuspended = true;
				markerList.push_back (panel);
				x += dx;
			}
			break;
		}
	}
}

void TimelineWindow::setHighlightedMarker (int markerIndex) {
	std::list<Panel *>::iterator i, end;
	Panel *panel;
	UiConfiguration *uiconfig;
	float x, t, dt;
	int curindex;
	bool found;

	uiconfig = &(App::instance->uiConfig);
	switch (recordType) {
		case SystemInterface::CommandId_MediaItem: {
			if (markerIndex >= (int) markerList.size ()) {
				markerIndex = -1;
			}
			found = false;
			curindex = 0;
			i = markerList.begin ();
			end = markerList.end ();
			while (i != end) {
				panel = *i;
				if (curindex == markerIndex) {
					panel->bgColor.translate (uiconfig->lightPrimaryColor, uiconfig->shortColorTranslateDuration);
					if ((thumbnailCount > 0) && (duration > 0.0f)) {
						found = true;
						dt = duration / (float) thumbnailCount;
						t = (dt / 2.0f) + ((float) markerIndex * dt);
						highlightTimeLabel->setText (OsUtil::getDurationString (t, minDurationUnitType));
						x = panel->position.x + (panel->width / 2.0f) - (highlightTimeLabel->width / 2.0f);
						if (x < 0.0f) {
							x = 0.0f;
						}
						if (x > (barWidth - highlightTimeLabel->width)) {
							x = (barWidth - highlightTimeLabel->width);
						}
						highlightTimeLabel->position.assignX (x);
						highlightTimeLabel->isVisible = true;
					}
				}
				else {
					panel->bgColor.translate (uiconfig->darkInverseBackgroundColor, uiconfig->shortColorTranslateDuration);
				}
				++curindex;
				++i;
			}
			if (! found) {
				highlightTimeLabel->isVisible = false;
			}
			highlightedMarkerIndex = markerIndex;
			break;
		}
		case SystemInterface::CommandId_StreamItem: {
			found = false;
			if (streamHighlightMarker) {
				streamHighlightMarker->bgColor.translate (uiconfig->lightBackgroundColor, uiconfig->shortColorTranslateDuration);
				streamHighlightMarker->setDestroyDelay (uiconfig->shortColorTranslateDuration);
				streamHighlightMarker = NULL;
			}

			if ((markerIndex >= 0) && (thumbnailCount > 0)) {
				x = ((float) markerIndex) * barWidth / (float) thumbnailCount;
				if (x > barWidth) {
					x = barWidth;
				}
				streamHighlightMarker = (Panel *) addWidget (new Panel (), x - (uiconfig->timelineMarkerWidth / 2.0f), 1.0f);
				streamHighlightMarker->setFillBg (true, uiconfig->lightBackgroundColor);
				streamHighlightMarker->bgColor.translate (uiconfig->darkPrimaryColor, uiconfig->shortColorTranslateDuration);
				streamHighlightMarker->setFixedSize (true, uiconfig->timelineMarkerWidth, barHeight - 2.0f);
				streamHighlightMarker->isInputSuspended = true;
				streamHighlightMarker->zLevel = 1;

				if (markerIndex < (int) segmentPositionList.size ()) {
					found = true;
					t = ((float) segmentPositionList.at (markerIndex)) * 1000.0f;
					highlightTimeLabel->setText (OsUtil::getDurationString (t, minDurationUnitType));
					x = streamHighlightMarker->position.x + (streamHighlightMarker->width / 2.0f) - (highlightTimeLabel->width / 2.0f);
					if (x < 0.0f) {
						x = 0.0f;
					}
					if (x > (barWidth - highlightTimeLabel->width)) {
						x = (barWidth - highlightTimeLabel->width);
					}
					highlightTimeLabel->position.assignX (x);
					highlightTimeLabel->isVisible = true;
				}
			}
			if (! found) {
				highlightTimeLabel->isVisible = false;
			}
			highlightedMarkerIndex = markerIndex;
			break;
		}
	}
}

void TimelineWindow::refreshLayout () {
	setFixedSize (true, barWidth, barHeight);
	startTimeLabel->position.assign (0.0f, barHeight - startTimeLabel->height);
	endTimeLabel->position.assign (barWidth - endTimeLabel->width, barHeight - endTimeLabel->height);
}

void TimelineWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
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
