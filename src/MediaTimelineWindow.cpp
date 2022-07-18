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
#include <math.h>
#include <vector>
#include <list>
#include "App.h"
#include "StdString.h"
#include "OsUtil.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "UiConfiguration.h"
#include "MediaTimelineWindow.h"

const int MediaTimelineWindow::GuideSegmentCount = 4;

MediaTimelineWindow::MediaTimelineWindow (float barWidth, const StdString &recordId)
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
, minDurationUnitType (OsUtil::MillisecondsUnit)
, thumbnailCount (0)
{
	setFillBg (true, UiConfiguration::instance->lightBackgroundColor);

	startTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->darkBackgroundColor)));
	startTimeLabel->setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->textLineHeightMargin);
	startTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	startTimeLabel->isInputSuspended = true;
	startTimeLabel->zLevel = 2;

	endTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->darkBackgroundColor)));
	endTimeLabel->setPadding (UiConfiguration::instance->paddingSize / 2.0, UiConfiguration::instance->textLineHeightMargin);
	endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	endTimeLabel->isInputSuspended = true;
	endTimeLabel->zLevel = 2;

	highlightTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->darkBackgroundColor)));
	highlightTimeLabel->setPadding (UiConfiguration::instance->paddingSize / 2.0, UiConfiguration::instance->textLineHeightMargin);
	highlightTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	highlightTimeLabel->isInputSuspended = true;
	highlightTimeLabel->zLevel = 3;
	highlightTimeLabel->isVisible = false;

	barHeight = startTimeLabel->height * 2.0f;

	refreshLayout ();
}

MediaTimelineWindow::~MediaTimelineWindow () {

}

StdString MediaTimelineWindow::toStringDetail () {
	return (StdString (" MediaTimelineWindow"));
}

void MediaTimelineWindow::syncRecordStore () {
	Json *record, *agentstatus, serverstatus;

	record = RecordStore::instance->findRecord (recordId, SystemInterface::CommandId_MediaItem);
	if (record) {
		recordType = SystemInterface::CommandId_MediaItem;
		agentId = SystemInterface::instance->getCommandAgentId (record);
		duration = SystemInterface::instance->getCommandNumberParam (record, "duration", (float) 0.0f);
		agentstatus = RecordStore::instance->findRecord (RecordStore::matchAgentStatusSource, &agentId);
		if (agentstatus) {
			if (SystemInterface::instance->getCommandObjectParam (agentstatus, "mediaServerStatus", &serverstatus)) {
				thumbnailCount = serverstatus.getNumber ("thumbnailCount", (int) 0);
			}
		}
	}

	if (! record) {
		record = RecordStore::instance->findRecord (recordId, SystemInterface::CommandId_StreamItem);
		if (record) {
			recordType = SystemInterface::CommandId_StreamItem;
			agentId = SystemInterface::instance->getCommandAgentId (record);
			duration = SystemInterface::instance->getCommandNumberParam (record, "duration", (float) 0.0f);
			thumbnailCount = SystemInterface::instance->getCommandNumberParam (record, "segmentCount", (int) 0);
			SystemInterface::instance->getCommandNumberArrayParam (record, "segmentPositions", &segmentPositionList, true);
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

void MediaTimelineWindow::populateMarkers () {
	if ((! markerList.empty ()) || (recordType < 0)) {
		return;
	}

	switch (recordType) {
		case SystemInterface::CommandId_MediaItem: {
			Panel *panel;
			float x, dx;
			int i;

			if (thumbnailCount > 0) {
				dx = barWidth / (float) thumbnailCount;
				if (dx < 1.0f) {
					dx = 1.0f;
				}
				x = (dx / 2.0f);
				for (i = 0; i < thumbnailCount; ++i) {
					panel = (Panel *) addWidget (new Panel (), x - (UiConfiguration::instance->timelineMarkerWidth / 2.0f), 1.0f);
					panel->setFillBg (true, UiConfiguration::instance->darkInverseBackgroundColor);
					panel->setFixedSize (true, UiConfiguration::instance->timelineMarkerWidth, barHeight - 2.0f);
					panel->isInputSuspended = true;
					markerList.push_back (panel);
					x += dx;
				}
			}
			break;
		}
		case SystemInterface::CommandId_StreamItem: {
			Panel *panel;
			float x, dx;

			x = 0.0f;
			dx = barWidth / (float) MediaTimelineWindow::GuideSegmentCount;
			if (dx < 1.0f) {
				dx = 1.0f;
			}
			x += dx;
			while (x < barWidth) {
				panel = (Panel *) addWidget (new Panel (), x, 1.0f);
				panel->setFillBg (true, UiConfiguration::instance->primaryTextColor);
				panel->setFixedSize (true, 1.0f, barHeight - 2.0f);
				panel->isInputSuspended = true;
				markerList.push_back (panel);
				x += dx;
			}
			break;
		}
	}
}

void MediaTimelineWindow::setHighlightColor (const Color &color) {
	highlightTimeLabel->setTextColor (color);
}

void MediaTimelineWindow::setHighlightedMarker (int markerIndex) {
	std::list<Panel *>::iterator i, end;
	Panel *panel;
	float x, t, dt;
	int curindex;
	bool found;

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
					panel->bgColor.translate (UiConfiguration::instance->lightPrimaryColor, UiConfiguration::instance->shortColorTranslateDuration);
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
					panel->bgColor.translate (UiConfiguration::instance->darkInverseBackgroundColor, UiConfiguration::instance->shortColorTranslateDuration);
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
				streamHighlightMarker->bgColor.translate (UiConfiguration::instance->lightBackgroundColor, UiConfiguration::instance->shortColorTranslateDuration);
				streamHighlightMarker->setDestroyDelay (UiConfiguration::instance->shortColorTranslateDuration);
				streamHighlightMarker = NULL;
			}

			if ((markerIndex >= 0) && (thumbnailCount > 0)) {
				x = ((float) markerIndex) * barWidth / (float) thumbnailCount;
				if (x > barWidth) {
					x = barWidth;
				}
				streamHighlightMarker = (Panel *) addWidget (new Panel (), x - (UiConfiguration::instance->timelineMarkerWidth / 2.0f), 1.0f);
				streamHighlightMarker->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
				streamHighlightMarker->bgColor.translate (UiConfiguration::instance->darkPrimaryColor, UiConfiguration::instance->shortColorTranslateDuration);
				streamHighlightMarker->setFixedSize (true, UiConfiguration::instance->timelineMarkerWidth, barHeight - 2.0f);
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

void MediaTimelineWindow::refreshLayout () {
	setFixedSize (true, barWidth, barHeight);
	startTimeLabel->position.assign (0.0f, barHeight - startTimeLabel->height);
	endTimeLabel->position.assign (barWidth - endTimeLabel->width, barHeight - endTimeLabel->height);
}

bool MediaTimelineWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (mouseState.isEntered) {
		if (! FLOAT_EQUALS (hoverPosition, mouseState.enterDeltaX)) {
			hoverPosition = mouseState.enterDeltaX;
			eventCallback (positionHoverCallback);
		}

		if (mouseState.isLeftClicked) {
			clickPosition = mouseState.enterDeltaX;
			eventCallback (positionClickCallback);
		}
	}
	else {
		if (hoverPosition >= 0.0f) {
			hoverPosition = -1.0f;
			eventCallback (positionHoverCallback);
		}
	}

	return (false);
}
