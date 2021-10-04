/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "ClassId.h"
#include "StdString.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Agent.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "Widget.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "CameraTimelineUi.h"
#include "CameraDetailWindow.h"

const float CameraDetailWindow::NameTruncateScale = 0.24f;

CameraDetailWindow::CameraDetailWindow (const StdString &agentId, SpriteGroup *cameraTimelineUiSpriteGroup)
: Panel ()
, agentId (agentId)
, sprites (cameraTimelineUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, captureTimespanIcon (NULL)
, selectedTimespanIcon (NULL)
{
	classId = ClassId::CameraDetailWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeCameraIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));

	captureTimespanIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (CameraTimelineUi::TimeIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	captureTimespanIcon->setPadding (0.0f, 0.0f);
	captureTimespanIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CaptureTimespanTooltip));
	captureTimespanIcon->isVisible = false;

	selectedTimespanIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (CameraTimelineUi::SelectedTimespanIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	selectedTimespanIcon->setPadding (0.0f, 0.0f);
	selectedTimespanIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::SelectedCaptureTimespanTooltip));
	selectedTimespanIcon->isVisible = false;

	refreshLayout ();
}

CameraDetailWindow::~CameraDetailWindow () {

}

StdString CameraDetailWindow::toStringDetail () {
	return (StdString::createSprintf (" CameraDetailWindow agentId=\"%s\"", agentId.c_str ()));
}

bool CameraDetailWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::CameraDetailWindow));
}

CameraDetailWindow *CameraDetailWindow::castWidget (Widget *widget) {
	return (CameraDetailWindow::isWidgetType (widget) ? (CameraDetailWindow *) widget : NULL);
}

void CameraDetailWindow::syncRecordStore () {
	Json *record, serverstatus;
	int64_t t1, t2;

	record = RecordStore::instance->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}
	if (! SystemInterface::instance->getCommandObjectParam (record, "cameraServerStatus", &serverstatus)) {
		return;
	}

	nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::TitleFont]->truncatedText (Agent::getCommandAgentName (record), (float) App::instance->windowWidth * CameraDetailWindow::NameTruncateScale, Font::DotTruncateSuffix));
	t1 = serverstatus.getNumber ("minCaptureTime", (int64_t) 0);
	t2 = serverstatus.getNumber ("lastCaptureTime", (int64_t) 0);
	if ((t1 <= 0) || (t2 <= 0)) {
		captureTimespanIcon->isVisible = false;
		selectedTimespanIcon->isVisible = false;
	}
	else {
		captureTimespanIcon->setText (StdString::createSprintf ("%s - %s (%s)", OsUtil::getTimestampDisplayString (t1).c_str (), OsUtil::getTimestampDisplayString (t2).c_str (), OsUtil::getDurationDisplayString (t2 - t1).c_str ()));
		captureTimespanIcon->isVisible = true;
		selectedTimespanIcon->isVisible = true;
	}

	refreshLayout ();
	Panel::syncRecordStore ();
}

void CameraDetailWindow::refreshLayout () {
	float x, y, x0, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, NULL, &y2);
	nameLabel->flowRight (&x, y, NULL, &y2);

	y = y2 + UiConfiguration::instance->marginSize;
	x = x0;
	captureTimespanIcon->flowDown (x, &y, NULL, &y2);
	selectedTimespanIcon->flowDown (x, &y, NULL, &y2);

	resetSize ();
}

void CameraDetailWindow::setSelectedTimespan (int64_t selectedTime, bool isDescending) {
	if (selectedTime <= 0) {
		selectedTimespanIcon->setText (StdString (""));
	}
	else {
		selectedTimespanIcon->setText (StdString::createSprintf ("%s: %s", UiText::instance->getText (isDescending ? UiTextString::Before : UiTextString::After).capitalized ().c_str (), OsUtil::getTimestampDisplayString (selectedTime).c_str ()));
	}
	refreshLayout ();
}
