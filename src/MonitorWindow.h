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
// Panel that contains elements representing a Membrane Monitor server on a card view

#ifndef MONITOR_WINDOW_H
#define MONITOR_WINDOW_H

#include "StdString.h"
#include "Sprite.h"
#include "Json.h"
#include "Image.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "IconLabelWindow.h"
#include "AgentTaskWindow.h"
#include "Panel.h"

class MonitorWindow : public Panel {
public:
	static const float ScreenshotImageScale; // portion of total window width, from 0.0f to 1.0f

	MonitorWindow (const StdString &agentId);
	virtual ~MonitorWindow ();

	// Read-write data members
	Widget::EventCallbackContext selectStateChangeCallback;
	Widget::EventCallbackContext expandStateChangeCallback;
	Widget::EventCallbackContext screenshotLoadCallback;
	Widget::EventCallbackContext actionClickCallback;

	// Read-only data members
	bool isSelected;
	bool isExpanded;
	bool isSelectEnabled;
	bool isScreenshotDisplayEnabled;
	bool isStorageDisplayEnabled;
	StdString agentId;
	StdString agentName;
	int agentTaskCount;
	int64_t screenshotTime;

	// Set the enable state of the window's select toggle
	void setSelectEnabled (bool enable);

	// Set the enable state of the window's screenshot image
	void setScreenshotDisplayEnabled (bool enable);

	// Set the enable state of the window's storage display elements
	void setStorageDisplayEnabled (bool enable);

	// Add an action button to the window
	void addActionButton (Sprite *sprite, const StdString &tooltipText = StdString (""), Widget::EventCallbackContext clickCallback = Widget::EventCallbackContext ());

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static MonitorWindow *castWidget (Widget *widget);

	// Set the window's select state, then execute any select state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setSelected (bool selected, bool shouldSkipStateChangeCallback = false);

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void screenshotImageLoaded (void *windowPtr, Widget *widgetPtr);
	static void screenshotImageLongPressed (void *windowPtr, Widget *widgetPtr);
	static void selectToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void expandToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void actionButtonClicked (void *windowPtr, Widget *widgetPtr);

	Image *iconImage;
	Label *nameLabel;
	Label *descriptionLabel;
	Panel *dividerPanel;
	ImageWindow *screenshotImage;
	IconLabelWindow *statusIcon;
	IconLabelWindow *taskCountIcon;
	IconLabelWindow *storageIcon;
	IconLabelWindow *streamCountIcon;
	Toggle *selectToggle;
	Toggle *expandToggle;
	Button *actionButton;
	AgentTaskWindow *agentTaskWindow;
};

#endif
