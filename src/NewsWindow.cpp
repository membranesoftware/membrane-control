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
#include <list>
#include "Result.h"
#include "Log.h"
#include "App.h"
#include "StdString.h"
#include "Util.h"
#include "Json.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "ScrollView.h"
#include "NewsItemWindow.h"
#include "TaskItemWindow.h"
#include "NewsWindow.h"

NewsWindow::NewsWindow (float viewWidth, float viewHeight)
: ScrollView (viewWidth, viewHeight)
, emptyLabel (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setFillBg (true, 0.0f, 0.0f, 0.0f);
	setAlphaBlend (true, uiconfig->overlayWindowAlpha);

	emptyLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (App::getInstance ()->uiText.emptyNotificationList, UiConfiguration::CAPTION, uiconfig->primaryTextColor)));
	emptyLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	emptyLabel->setFillBg (true, uiconfig->lightBackgroundColor);
	resetLayout ();
}

NewsWindow::~NewsWindow () {

}

StdString NewsWindow::toStringDetail () {
	return (StdString (" NewsWindow"));
}

void NewsWindow::clearNewsItemList () {
	std::list<NewsItemWindow *>::iterator i, end;

	i = newsItemList.begin ();
	end = newsItemList.end ();
	while (i != end) {
		(*i)->isDestroyed = true;
		++i;
	}

	newsItemList.clear ();
	resetLayout ();
}

void NewsWindow::clearTaskItemList () {
	std::list<TaskItemWindow *>::iterator i, end;

	i = taskItemList.begin ();
	end = taskItemList.end ();
	while (i != end) {
		(*i)->isDestroyed = true;
		++i;
	}
	taskItemList.clear ();
	taskItemMap.clear ();
	resetLayout ();
}

void NewsWindow::setViewSize (float viewWidth, float viewHeight) {
	setFixedSize (true, viewWidth, viewHeight);
	resetLayout ();
}

void NewsWindow::resetLayout () {
	UiConfiguration *uiconfig;
	std::list<TaskItemWindow *>::iterator ti, tend;
	std::list<NewsItemWindow *>::iterator ni, nend;
	TaskItemWindow *taskitem;
	NewsItemWindow *newsitem;
	float x, y;
	bool isempty;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = uiconfig->paddingSize;
	y = uiconfig->paddingSize;
	isempty = true;

	if (! taskItemList.empty ()) {
		ti = taskItemList.begin ();
		tend = taskItemList.end ();
		while (ti != tend) {
			taskitem = *ti;
			if (! taskitem->isDeleted) {
				taskitem->position.assign (x, y);
				y += taskitem->height + uiconfig->marginSize;
				isempty = false;
			}
			++ti;
		}
	}

	if (! newsItemList.empty ()) {
		ni = newsItemList.begin ();
		nend = newsItemList.end ();
		while (ni != nend) {
			newsitem = *ni;
			newsitem->position.assign (x, y);
			y += newsitem->height + uiconfig->marginSize;

			isempty = false;
			++ni;
		}
	}

	if (! isempty) {
		emptyLabel->isVisible = false;
	}
	else {
		emptyLabel->position.assign (x, y);
		emptyLabel->isVisible = true;
	}

	y -= height;
	if (y < 0.0f) {
		y = 0.0f;
	}
	setVerticalScrollBounds (0.0f, y);
}

void NewsWindow::doRefresh () {
	UiConfiguration *uiconfig;
	std::list<NewsItemWindow *>::iterator i, end;

	uiconfig = &(App::getInstance ()->uiConfig);
	i = newsItemList.begin ();
	end = newsItemList.end ();
	while (i != end) {
		(*i)->setWindowWidth (width - (uiconfig->paddingSize * 2.0f));
		++i;
	}

	ScrollView::doRefresh ();
}

void NewsWindow::syncRecordStore (RecordStore *store) {
	store->processRecords (SystemInterface::Command_TaskItem, NewsWindow::processTaskItemRecord, this);
	ScrollView::syncRecordStore (store);
}

void NewsWindow::processTaskItemRecord (void *windowPtr, Json *record, const StdString &recordId) {
	UiConfiguration *uiconfig;
	SystemInterface *interface;
	NewsWindow *window;
	std::map<StdString, TaskItemWindow *>::iterator pos;
	TaskItemWindow *taskitemwindow;
	StdString title, text;

	window = (NewsWindow *) windowPtr;
	uiconfig = &(App::getInstance ()->uiConfig);
	interface = &(App::getInstance ()->systemInterface);

	pos = window->taskItemMap.find (recordId);
	if (interface->isRecordClosed (record)) {
		if (pos != window->taskItemMap.end ()) {
			taskitemwindow = pos->second;
			if (taskitemwindow->isDeleted) {
				window->removeTaskItem (recordId);
			}
		}
	}
	else {
		if (pos == window->taskItemMap.end ()) {
			title = interface->getCommandStringParam (record, "name", "");
			text = interface->getCommandStringParam (record, "subtitle", "");
			taskitemwindow = (TaskItemWindow *) window->addWidget (new TaskItemWindow ((window->width - (uiconfig->paddingSize * 2.0f)), recordId, title, text));
			taskitemwindow->setDeleteCallback (NewsWindow::taskItemDeleteClicked, window);
			window->taskItemList.push_back (taskitemwindow);
			window->taskItemMap.insert (std::pair<StdString, TaskItemWindow *> (recordId, taskitemwindow));
			window->resetLayout ();
		}
	}
}

void NewsWindow::taskItemDeleteClicked (void *windowPtr, Widget *widgetPtr) {
	NewsWindow *window;
	TaskItemWindow *taskitemwindow;

	window = (NewsWindow *) windowPtr;
	taskitemwindow = (TaskItemWindow *) widgetPtr;
	if (taskitemwindow->isTaskComplete) {
		window->removeTaskItem (taskitemwindow->taskId);
	}
	else {
		taskitemwindow->isVisible = false;
	}
	window->resetLayout ();
}

void NewsWindow::removeTaskItem (const StdString &taskId) {
	std::map<StdString, TaskItemWindow *>::iterator pos;
	std::list<TaskItemWindow *>::iterator i, end;
	TaskItemWindow *taskitemwindow;

	pos = taskItemMap.find (taskId);
	if (pos == taskItemMap.end ()) {
		return;
	}

	taskitemwindow = pos->second;
	taskItemMap.erase (pos);

	i = taskItemList.begin ();
	end = taskItemList.end ();
	while (i != end) {
		if ((*i) == taskitemwindow) {
			taskItemList.erase (i);
			break;
		}
		++i;
	}
	taskitemwindow->isDestroyed = true;
	resetLayout ();
}

void NewsWindow::addNewsItem (Sprite *iconSprite, const StdString &itemTitle, const StdString &itemText, bool shouldCopyIconSprite) {
	UiConfiguration *uiconfig;
	NewsItemWindow *window;

	uiconfig = &(App::getInstance ()->uiConfig);
	window = (NewsItemWindow *) addWidget (new NewsItemWindow ((width - (uiconfig->paddingSize * 2.0f)), iconSprite, itemTitle, itemText, shouldCopyIconSprite));
	window->setDeleteCallback (NewsWindow::newsItemDeleteClicked, this);
	newsItemList.push_back (window);
}

void NewsWindow::newsItemDeleteClicked (void *windowPtr, Widget *widgetPtr) {
	NewsWindow *window;
	NewsItemWindow *newsitem;
	std::list<NewsItemWindow *>::iterator i, end;

	window = (NewsWindow *) windowPtr;
	newsitem = (NewsItemWindow *) widgetPtr;
	i = window->newsItemList.begin ();
	end = window->newsItemList.end ();
	while (i != end) {
		if (newsitem == *i) {
			window->newsItemList.erase (i);
			break;
		}
		++i;
	}

	newsitem->isDestroyed = true;
	window->resetLayout ();
}

void NewsWindow::addTaskItem (const StdString &taskId, Sprite *iconSprite, const StdString &itemTitle, const StdString &itemText) {
	// TODO: Implement this
}

void NewsWindow::addTaskItem (Json *taskItem) {
// TODO: Fix this
/*
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	NewsWindow::NewsItem item;
	Panel *panel;
	Label *label;
	ProgressBar *bar;
	StdString text;
	float x, y;

	interface = &(App::getInstance ()->systemInterface);
	uiconfig = &(App::getInstance ()->uiConfig);
	item.taskId = interface->getCommandStringParam (taskItem, "id", "");

	panel = (Panel *) addWidget (new Panel ());
	panel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	x = uiconfig->paddingSize;
	y = uiconfig->paddingSize;
	label = (Label *) panel->addWidget (new Label (interface->getCommandStringParam (taskItem, "name", "-"), UiConfiguration::BODY, uiconfig->primaryTextColor));
	label->position.assign (x, y);
	y += label->maxLineHeight + uiconfig->textLineHeightMargin;

	text = interface->getCommandStringParam (taskItem, "subtitle", "");
	if (! text.empty ()) {
		label = (Label *) panel->addWidget (new Label (text, UiConfiguration::CAPTION, uiconfig->primaryTextColor));
		label->position.assign (x, y);
		y += label->maxLineHeight + uiconfig->textLineHeightMargin;
	}

	bar = (ProgressBar *) panel->addWidget (new ProgressBar (1.0f, uiconfig->progressBarHeight));
	bar->position.assign (x, y);
	bar->setProgress ((float) interface->getCommandNumberParam (taskItem, "percentComplete", 0), 100.0f);
	item.progressBar = bar;

	panel->resetSize ();
	item.panel = panel;
	newsItemList.push_back (item);
	resetLayout ();
*/
}
