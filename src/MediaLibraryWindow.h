/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Panel that contains elements representing a Membrane Media Library server on a card view

#ifndef MEDIA_LIBRARY_WINDOW_H
#define MEDIA_LIBRARY_WINDOW_H

#include "StdString.h"
#include "Json.h"
#include "Image.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "ProgressBar.h"
#include "IconLabelWindow.h"
#include "Panel.h"

class MediaLibraryWindow : public Panel {
public:
	MediaLibraryWindow (const StdString &agentId);
	virtual ~MediaLibraryWindow ();

	// Read-only data members
	bool isSelected;
	bool isExpanded;
	StdString agentId;
	StdString agentName;
	StdString htmlCatalogPath;
	int agentTaskCount;
	float menuPositionX;
	float menuPositionY;

	// Set a callback function that should be invoked when the window's menu button is pressed
	void setMenuClickCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the select toggle's checked state changes
	void setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback that should be invoked when the expand toggle's checked state changes
	void setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static MediaLibraryWindow *castWidget (Widget *widget);

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Callback functions
	static void menuButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void selectToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void expandToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void catalogLinkClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	Image *iconImage;
	Label *nameLabel;
	Label *descriptionLabel;
	IconLabelWindow *catalogLinkIcon;
	IconLabelWindow *taskCountIcon;
	IconLabelWindow *storageIcon;
	IconLabelWindow *mediaCountIcon;
	IconLabelWindow *streamCountIcon;
	Image *taskImage;
	Label *taskNameLabel;
	Label *taskSubtitleLabel;
	ProgressBar *taskProgressBar;
	Button *menuButton;
	Toggle *selectToggle;
	Toggle *expandToggle;
	Widget::EventCallback menuClickCallback;
	void *menuClickCallbackData;
	Widget::EventCallback selectStateChangeCallback;
	void *selectStateChangeCallbackData;
	Widget::EventCallback expandStateChangeCallback;
	void *expandStateChangeCallbackData;
};

#endif
