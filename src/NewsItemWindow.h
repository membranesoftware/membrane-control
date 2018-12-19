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
// Panel that shows notification content, for use as an item in a news window

#ifndef NEWS_ITEM_WINDOW_H
#define NEWS_ITEM_WINDOW_H

#include "StdString.h"
#include "Sprite.h"
#include "TextArea.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Image.h"
#include "Button.h"
#include "Panel.h"

class NewsItemWindow : public Panel {
public:
	NewsItemWindow (float windowWidth, Sprite *iconSprite, const StdString &itemTitle = StdString (""), const StdString &itemText = StdString (""), bool shouldCopyIconSprite = false);
	virtual ~NewsItemWindow ();

	// Read-only data members
	int64_t createTime;

	// Set the window's width
	void setWindowWidth (float windowWidthValue);

	// Set a function that should be invoked when the window's delete action is clicked
	void setDeleteCallback (Widget::EventCallback callback, void *callbackData);

	// Set the content of the window's detail text area
	void setDetailText (const StdString &text);

	// Callback functions
	static void deleteButtonClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	float windowWidth;
	Image *iconImage;
	LabelWindow *titleLabel;
	LabelWindow *timestampLabel;
	TextArea *detailText;
	Button *deleteButton;
	Widget::EventCallback deleteCallback;
	void *deleteCallbackData;
};

#endif
