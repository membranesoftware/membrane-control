/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "ClassId.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "OsUtil.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "ProgressBar.h"
#include "AgentTaskWindow.h"

const float AgentTaskWindow::textTruncateScale = 0.15f;

AgentTaskWindow::AgentTaskWindow (const StdString &agentId)
: Panel ()
, agentId (agentId)
, isTaskRunning (false)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, progressBar (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::AgentTaskWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::TaskInProgressIconSprite)));
	iconImage->isInputSuspended = true;

	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor));
	nameLabel->isInputSuspended = true;

	descriptionLabel  = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	descriptionLabel->isInputSuspended = true;

	progressBar = (ProgressBar *) addWidget (new ProgressBar (((float) App::instance->windowWidth) * AgentTaskWindow::textTruncateScale, uiconfig->progressBarHeight));
	progressBar->isInputSuspended = true;

	setMouseHoverTooltip (uitext->getText (UiTextString::taskInProgress).capitalized ());
	refreshLayout ();
}

AgentTaskWindow::~AgentTaskWindow () {

}

StdString AgentTaskWindow::toStringDetail () {
	return (StdString::createSprintf (" AgentTaskWindow agentId=\"%s\"", agentId.c_str ()));
}

bool AgentTaskWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::AgentTaskWindow));
}

AgentTaskWindow *AgentTaskWindow::castWidget (Widget *widget) {
	return (AgentTaskWindow::isWidgetType (widget) ? (AgentTaskWindow *) widget : NULL);
}

void AgentTaskWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	UiText *uitext;
	Json *record;
	float pct;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	uitext = &(App::instance->uiText);
	record = store->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}

	nameLabel->setText (interface->getCommandStringParam (record, "runTaskName", ""));
	descriptionLabel->setText (Label::getTruncatedText (interface->getCommandStringParam (record, "runTaskSubtitle", ""), UiConfiguration::CaptionFont, ((float) App::instance->windowWidth) * AgentTaskWindow::textTruncateScale, StdString ("...")));
	pct = interface->getCommandNumberParam (record, "runTaskPercentComplete", (float) 0.0f);
	progressBar->setProgress (pct, 100.0f);
	setMouseHoverTooltip (StdString::createSprintf ("%s: %i%% %s", uitext->getText (UiTextString::taskInProgress).capitalized ().c_str (), (int) pct, uitext->getText (UiTextString::complete).c_str ()));

	if (nameLabel->text.empty ()) {
		isTaskRunning = false;
	}
	else {
		isTaskRunning = true;
	}
}

void AgentTaskWindow::refreshLayout () {
	float x, y, y0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;
	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowRight (&x, y, &x2, &y2);
	descriptionLabel->flowDown (x, &y, &x2, &y2);
	progressBar->flowDown (x, &y, &x2, &y2);
	iconImage->centerVertical (y0, y2);
	nameLabel->centerVertical (y0, y2);

	resetSize ();
}
