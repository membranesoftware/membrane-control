/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
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
// Panel that contains controls describing a UI type and allowing it to be launched

#ifndef UI_LAUNCH_WINDOW_H
#define UI_LAUNCH_WINDOW_H

#include <vector>
#include "StdString.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Image.h"
#include "Label.h"
#include "Button.h"
#include "TextArea.h"
#include "IconLabelWindow.h"
#include "Panel.h"

class UiLaunchWindow : public Panel {
public:
	// Constants to use for UI types
	enum {
		SERVER_UI = 0,
		MEDIA_UI = 1,
		MONITOR_UI = 2,
		WEB_KIOSK_UI = 3
	};

	UiLaunchWindow (int uiType, SpriteGroup *mainUiSpriteGroup);
	virtual ~UiLaunchWindow ();

	// Read-write data members
	StdString itemId;

	// Read-only data members
	int uiType;

	// Set a function that should be invoked when the window's open action is clicked
	void setOpenCallback (Widget::EventCallback callback, void *callbackData);

	// Execute operations appropriate to sync widget state with records present in the provided RecordStore object, which has been locked prior to invocation
	void syncRecordStore (RecordStore *store);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static UiLaunchWindow *castWidget (Widget *widget);

	// Return a boolean value indicating if a window of the specified type would be ready if synced from the provided RecordStore
	static bool isReadyState (int uiType, RecordStore *store);

	// Callback functions
	static void openButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void addMediaCount (void *sumPtr, Json *record, const StdString &recordId);
	static void addStreamCount (void *sumPtr, Json *record, const StdString &recordId);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Populate widgets as appropriate for the configured intent type
	void populate ();

	// Return the total of all mediaCount fields on MediaServer agents
	int countMediaItems (RecordStore *store);

	// Return the total of all streamCount fields on StreamServer agents
	int countStreamItems (RecordStore *store);

	SpriteGroup *spriteGroup;
	Image *iconImage;
	Label *nameLabel;
	TextArea *descriptionText;
	std::vector<IconLabelWindow *> noteIcons;
	Button *openButton;
	Widget::EventCallback openCallback;
	void *openCallbackData;
};

#endif
