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
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Image.h"
#include "Button.h"
#include "ProgressBar.h"
#include "UiConfiguration.h"
#include "TaskItemWindow.h"

TaskItemWindow::TaskItemWindow (float windowWidth, const StdString &taskId, const StdString &itemTitle, const StdString &itemText)
: Panel ()
, taskId (taskId)
, isTaskComplete (false)
, isDeleted (false)
, windowWidth (windowWidth)
, iconImage (NULL)
, titleLabel (NULL)
, detailText (NULL)
, progressBar (NULL)
, deleteButton (NULL)
, deleteCallback (NULL)
, deleteCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->darkBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::TASK_IN_PROGRESS_ICON)));
	titleLabel = (Label *) addWidget (new Label (itemTitle, UiConfiguration::BODY, uiconfig->primaryTextColor));
	detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	detailText->setText (itemText);
	progressBar = (ProgressBar *) addWidget (new ProgressBar (windowWidth - (uiconfig->paddingSize * 2.0f), uiconfig->progressBarHeight));
	deleteButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON)));
	deleteButton->setMouseClickCallback (TaskItemWindow::deleteButtonClicked, this);
	deleteButton->setImageColor (uiconfig->flatButtonTextColor);

	resetLayout ();
}

TaskItemWindow::~TaskItemWindow () {

}

StdString TaskItemWindow::toStringDetail () {
	return (StdString (" TaskItemWindow"));
}

void TaskItemWindow::setWindowWidth (float windowWidthValue) {
	UiConfiguration *uiconfig;

	if (FLOAT_EQUALS (windowWidthValue, windowWidth)) {
		return;
	}
	uiconfig = &(App::getInstance ()->uiConfig);
	windowWidth = windowWidthValue;
	detailText->setMaxLineWidth (windowWidth - iconImage->width - (uiconfig->paddingSize * 2.0f));
	resetLayout ();
}

void TaskItemWindow::setDeleteCallback (Widget::EventCallback callback, void *callbackData) {
	deleteCallback = callback;
	deleteCallbackData = callbackData;
}

void TaskItemWindow::setDetailText (const StdString &text) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	if (text.empty ()) {
		if (detailText) {
			detailText->isDestroyed = true;
			detailText = NULL;
			resetLayout ();
		}
	}
	else {
		if (! detailText) {
			detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->primaryTextColor));
		}
		detailText->setText (text);
		resetLayout ();
	}
}

void TaskItemWindow::syncRecordStore (RecordStore *store) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	Json *record;
	float pct;

	uiconfig = &(App::getInstance ()->uiConfig);
	interface = &(App::getInstance ()->systemInterface);
	record = store->findRecord (taskId, SystemInterface::Command_TaskItem);
	if (! record) {
		return;
	}

	if (! isTaskComplete) {
		pct = interface->getCommandNumberParam (record, "percentComplete", (float) 0.0f);
		progressBar->setProgress (pct, 100.0f);
		if (pct >= 100.0f) {
			isTaskComplete = true;
			iconImage->isDestroyed = true;
			iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::TASK_COMPLETE_ICON)));
			resetLayout ();
		}
	}
}

void TaskItemWindow::resetLayout () {
	UiConfiguration *uiconfig;
	float x, y, h;

	uiconfig = &(App::getInstance ()->uiConfig);
	y = uiconfig->paddingSize;

	x = windowWidth - uiconfig->paddingSize;
	x -= iconImage->width;
	iconImage->position.assign (x, y);

	x = uiconfig->paddingSize;
	titleLabel->position.assign (x, titleLabel->getLinePosition (y));
	y += titleLabel->maxLineHeight + uiconfig->textLineHeightMargin;

	x = uiconfig->paddingSize;
	if (detailText) {
		detailText->position.assign (x, y);
		y += detailText->height + uiconfig->marginSize;
	}
	progressBar->position.assign (x, y);
	y += progressBar->height + uiconfig->marginSize;

	x = windowWidth - uiconfig->paddingSize;
	x -= deleteButton->width;
	deleteButton->position.assign (x, y);

	resetSize ();
	h = maxWidgetY + uiconfig->paddingSize;
	setFixedSize (true, windowWidth, h);
}

void TaskItemWindow::deleteButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TaskItemWindow *window;

	window = (TaskItemWindow *) windowPtr;
	window->isDeleted = true;
	if (window->deleteCallback) {
		window->deleteCallback (window->deleteCallbackData, window);
	}
}
