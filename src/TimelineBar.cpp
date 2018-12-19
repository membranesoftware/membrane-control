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

const int TimelineBar::guideSegmentCount = 4;

TimelineBar::TimelineBar (float barWidth, const StdString &recordId)
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

	typeName.assign ("TimelineBar");

	uiconfig = &(App::getInstance ()->uiConfig);
	setFillBg (true, uiconfig->lightBackgroundColor);

	startTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->darkBackgroundColor)));
	startTimeLabel->setPadding (uiconfig->paddingSize / 2.0f, uiconfig->textLineHeightMargin);
	startTimeLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	startTimeLabel->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	startTimeLabel->isInputSuspended = true;
	startTimeLabel->zLevel = 2;

	endTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->darkBackgroundColor)));
	endTimeLabel->setPadding (uiconfig->paddingSize / 2.0, uiconfig->textLineHeightMargin);
	endTimeLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	endTimeLabel->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	endTimeLabel->isInputSuspended = true;
	endTimeLabel->zLevel = 2;

	highlightTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->mediumSecondaryColor)));
	highlightTimeLabel->setPadding (uiconfig->paddingSize / 2.0, uiconfig->textLineHeightMargin);
	highlightTimeLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	highlightTimeLabel->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	highlightTimeLabel->isInputSuspended = true;
	highlightTimeLabel->zLevel = 3;
	highlightTimeLabel->isVisible = false;

	barHeight = startTimeLabel->height * 2.0f;

	refreshLayout ();
}

TimelineBar::~TimelineBar () {

}

void TimelineBar::setPositionHoverCallback (Widget::EventCallback callback, void *callbackData) {
	positionHoverCallback = callback;
	positionHoverCallbackData = callbackData;
}

void TimelineBar::setPositionClickCallback (Widget::EventCallback callback, void *callbackData) {
	positionClickCallback = callback;
	positionClickCallbackData = callbackData;
}

void TimelineBar::syncRecordStore (RecordStore *store) {
	App *app;
	SystemInterface *interface;
	Json *record, *agentstatus, serverstatus;

	app = App::getInstance ();
	interface = &(app->systemInterface);

	record = store->findRecord (recordId, SystemInterface::Command_MediaItem);
	if (record) {
		recordType = SystemInterface::Command_MediaItem;
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
		record = store->findRecord (recordId, SystemInterface::Command_StreamItem);
		if (record) {
			recordType = SystemInterface::Command_StreamItem;
			agentId = interface->getCommandAgentId (record);
			duration = interface->getCommandNumberParam (record, "duration", (float) 0.0f);
			thumbnailCount = interface->getCommandNumberParam (record, "segmentCount", (int) 0);
			interface->getCommandNumberArrayParam (record, "segmentPositions", &segmentPositionList, true);
		}
	}

	if (! record) {
		return;
	}
	minDurationUnitType = Util::getDurationMinUnitType (duration);
	startTimeLabel->setText (Util::getDurationString (0.0f, minDurationUnitType));
	endTimeLabel->setText (Util::getDurationString (duration, minDurationUnitType));
	populateMarkers ();
	refreshLayout ();
}

void TimelineBar::populateMarkers () {
	if ((! markerList.empty ()) || (recordType < 0)) {
		return;
	}

	switch (recordType) {
		case SystemInterface::Command_MediaItem: {
			UiConfiguration *uiconfig;
			Panel *panel;
			float x, dx;
			int i;

			if (thumbnailCount > 0) {
				uiconfig = &(App::getInstance ()->uiConfig);
				dx = barWidth / (float) thumbnailCount;
				if (dx < 1.0f) {
					dx = 1.0f;
				}
				x = 0.0f;
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
		case SystemInterface::Command_StreamItem: {
			UiConfiguration *uiconfig;
			Panel *panel;
			float x, dx;

			uiconfig = &(App::getInstance ()->uiConfig);
			x = 0.0f;
			dx = barWidth / (float) TimelineBar::guideSegmentCount;
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

void TimelineBar::setHighlightedMarker (int markerIndex) {
	std::list<Panel *>::iterator i, end;
	Panel *panel;
	UiConfiguration *uiconfig;
	float x, t;
	int curindex;
	bool found;

	uiconfig = &(App::getInstance ()->uiConfig);
	switch (recordType) {
		case SystemInterface::Command_MediaItem: {
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
					panel->bgColor.rotate (uiconfig->lightPrimaryColor, uiconfig->shortColorRotateDuration);
					if ((thumbnailCount > 0) && (duration > 0.0f)) {
						found = true;
						t = (float) markerIndex * duration / (float) thumbnailCount;
						highlightTimeLabel->setText (Util::getDurationString (t, minDurationUnitType));
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
					panel->bgColor.rotate (uiconfig->darkInverseBackgroundColor, uiconfig->shortColorRotateDuration);
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
		case SystemInterface::Command_StreamItem: {
			found = false;
			if (streamHighlightMarker) {
				streamHighlightMarker->bgColor.rotate (uiconfig->lightBackgroundColor, uiconfig->shortColorRotateDuration);
				streamHighlightMarker->setDestroyDelay (uiconfig->shortColorRotateDuration);
				streamHighlightMarker = NULL;
			}

			if ((markerIndex >= 0) && (thumbnailCount > 0)) {
				x = ((float) markerIndex) * barWidth / (float) thumbnailCount;
				if (x > barWidth) {
					x = barWidth;
				}
				streamHighlightMarker = (Panel *) addWidget (new Panel (), x - (uiconfig->timelineMarkerWidth / 2.0f), 1.0f);
				streamHighlightMarker->setFillBg (true, uiconfig->lightBackgroundColor);
				streamHighlightMarker->bgColor.rotate (uiconfig->darkPrimaryColor, uiconfig->shortColorRotateDuration);
				streamHighlightMarker->setFixedSize (true, uiconfig->timelineMarkerWidth, barHeight - 2.0f);
				streamHighlightMarker->isInputSuspended = true;
				streamHighlightMarker->zLevel = 1;

				if (markerIndex < (int) segmentPositionList.size ()) {
					found = true;
					t = ((float) segmentPositionList.at (markerIndex)) * 1000.0f;
					highlightTimeLabel->setText (Util::getDurationString (t, minDurationUnitType));
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

void TimelineBar::refreshLayout () {
	setFixedSize (true, barWidth, barHeight);
	startTimeLabel->position.assign (0.0f, barHeight - startTimeLabel->height);
	endTimeLabel->position.assign (barWidth - endTimeLabel->width, barHeight - endTimeLabel->height);
}

void TimelineBar::doProcessMouseState (const Widget::MouseState &mouseState) {
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
