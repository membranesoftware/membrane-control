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
// Panel that shows a list of recent agent commands

#ifndef HISTORY_WINDOW_H
#define HISTORY_WINDOW_H

#include <list>
#include "CommandHistory.h"
#include "StdString.h"
#include "ImageWindow.h"
#include "Label.h"
#include "LabelWindow.h"
#include "IconLabelWindow.h"
#include "Button.h"
#include "Toggle.h"
#include "ScrollView.h"
#include "ScrollBar.h"
#include "Panel.h"

class HistoryItemWindow : public Panel {
public:
	HistoryItemWindow (float windowWidth, float rightMargin, const CommandHistory::Record &record);
	virtual ~HistoryItemWindow ();

	// Read-write data members
	Widget::EventCallbackContext executeClickCallback;
	Widget::EventCallbackContext expandStateChangeCallback;

	// Read-only data members
	float windowWidth;
	float rightMargin;
	bool isExpanded;
	StdString recordId;
	int64_t commandTime;
	bool isExecutable;
	bool isSaved;

	// Reset window content from a CommandHistory::Record struct
	void readRecord (const CommandHistory::Record &record);

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

protected:
	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void saveToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void expandToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void executeButtonClicked (void *windowPtr, Widget *widgetPtr);

	// Set the image and text of an IconLabelWindow as appropriate for a CommandHistory::CommandStatus value
	void setStatusIcon (IconLabelWindow *icon, CommandHistory::CommandStatus status);

	Panel *dividerPanel;
	Label *nameLabel;
	Label *detailLabel;
	IconLabelWindow *agentNameIcon;
	IconLabelWindow *timeIcon;
	IconLabelWindow *commandStatusIcon;
	Panel *agentStatusPanel;
	Toggle *saveToggle;
	Button *executeButton;
	Toggle *expandToggle;
};

class HistoryWindow : public Panel {
public:
	HistoryWindow (float windowWidth, float windowHeight);
	virtual ~HistoryWindow ();

	// Read-write data members
	Widget::EventCallbackContext executeClickCallback;

	// Read-only data members
	StdString executeCommandId;

	// Populate window content from records in the application's CommandHistory
	void readHistory ();

	// Set the window's size
	void setWindowSize (float windowWidth, float windowHeight);

protected:
	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	void doUpdate (int msElapsed);

	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	bool doProcessMouseState (const Widget::MouseState &mouseState);

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void closeButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void clearButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void scrollBarPositionChanged (void *windowPtr, Widget *widgetPtr);
	static void readHistory_processRecord (void *windowPtr, CommandHistory::Record *record);
	static void itemExecuteClicked (void *windowPtr, Widget *widgetPtr);
	static void itemExpandStateChanged (void *windowPtr, Widget *widgetPtr);

	ImageWindow *headerImage;
	bool isHeaderImageLoaded;
	LabelWindow *titleLabel;
	Button *closeButton;
	Button *clearButton;
	ScrollView *itemView;
	std::list<HistoryItemWindow *> itemList;
	ScrollBar *scrollBar;
};

#endif
