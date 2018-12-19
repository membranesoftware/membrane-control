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
// Panel that shows a list of news items

#ifndef NEWS_WINDOW_H
#define NEWS_WINDOW_H

#include <list>
#include <map>
#include "StdString.h"
#include "RecordStore.h"
#include "Json.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Panel.h"
#include "NewsItemWindow.h"
#include "ScrollView.h"

class NewsWindow : public ScrollView {
public:
	NewsWindow (float viewWidth, float viewHeight);
	virtual ~NewsWindow ();

	// Set the window's view size
	void setViewSize (float viewWidth, float viewHeight);

	// Add a news item to the window using the provided values, optionally creating a copy of the icon sprite (i.e. if the sprite is not expected to persist as UI changes occur)
	void addNewsItem (Sprite *iconSprite, const StdString &itemTitle, const StdString &itemText, bool shouldCopyIconSprite = false);

	// Add a task item to the window using information from the provided TaskItem record
	void addTaskItem (const StdString &taskId, Sprite *iconSprite, const StdString &itemTitle, const StdString &itemText);
	void addTaskItem (Json *taskItem);

	// Remove all news items from the window
	void clearNewsItemList ();

	// Remove all task items from the window
	void clearTaskItemList ();

	// Execute operations appropriate to sync widget state with records present in the provided RecordStore object, which has been locked prior to invocation
	void syncRecordStore (RecordStore *store);

	// Callback functions
	static void processTaskItemRecord (void *windowPtr, Json *record, const StdString &recordId);
	static void taskItemDeleteClicked (void *windowPtr, Widget *widgetPtr);
	static void newsItemDeleteClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Destroy any task window associated with the specified ID
	void removeTaskItem (const StdString &taskId);

	std::list<NewsItemWindow *> newsItemList;
//	std::list<TaskItemWindow *> taskItemList;
//	std::map<StdString, TaskItemWindow *> taskItemMap;
	LabelWindow *emptyLabel;
};

#endif
