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
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "ProgressBar.h"
#include "UiConfiguration.h"
#include "TaskWindow.h"

TaskWindow::TaskWindow (const StdString &taskId)
: Panel ()
, taskId (taskId)
, isTaskComplete (false)
, isDeleted (false)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, progressBar (NULL)
, deleteButton (NULL)
{
	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setFillBg (true, UiConfiguration::instance->darkBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::TaskInProgressIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));
	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	progressBar = (ProgressBar *) addWidget (new ProgressBar (((float) App::instance->windowWidth) * 0.16f, UiConfiguration::instance->progressBarHeight));

	deleteButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	deleteButton->mouseClickCallback = Widget::EventCallbackContext (TaskWindow::deleteButtonClicked, this);
	deleteButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	deleteButton->isVisible = false;

	refreshLayout ();
}

TaskWindow::~TaskWindow () {

}

StdString TaskWindow::toStringDetail () {
	return (StdString (" TaskWindow"));
}

void TaskWindow::syncRecordStore () {
	Json *record;
	float pct;

	record = RecordStore::instance->findRecord (taskId, SystemInterface::CommandId_TaskItem);
	if (! record) {
		return;
	}
	nameLabel->setText (SystemInterface::instance->getCommandStringParam (record, "name", ""));
	descriptionLabel->setText (SystemInterface::instance->getCommandStringParam (record, "subtitle", ""));

	// TODO: Show the delete button if the task can be cancelled

	if (! isTaskComplete) {
		pct = SystemInterface::instance->getCommandNumberParam (record, "percentComplete", (float) 0.0f);
		progressBar->setProgress (pct, 100.0f);
		if (pct >= 100.0f) {
			isTaskComplete = true;
			iconImage->isDestroyed = true;
			iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::TaskCompleteIconSprite)));
		}
	}
	refreshLayout ();
}

void TaskWindow::refreshLayout () {
	float x, y, x0, y0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	nameLabel->flowDown (x, &y, &x2, &y2);
	descriptionLabel->flowDown (x, &y, &x2, &y2);

	x = x2 + UiConfiguration::instance->marginSize;
	y = y0;
	iconImage->flowDown (x, &y, &x2, &y2);

	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
	y2 = 0.0f;
	progressBar->flowDown (x, &y, NULL, &y2);

	y = y2 + UiConfiguration::instance->marginSize;
	y2 = 0.0f;
	if (deleteButton->isVisible) {
		deleteButton->flowDown (x, &y, NULL, &y2);
	}

	resetSize ();
}

void TaskWindow::deleteButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TaskWindow *window;

	window = (TaskWindow *) windowPtr;
	window->isDeleted = true;
	window->eventCallback (window->deleteCallback);
}
