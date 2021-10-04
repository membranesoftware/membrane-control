/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Panel containing a label that can be clicked to open a URL in a browser

#ifndef HYPERLINK_WINDOW_H
#define HYPERLINK_WINDOW_H

#include "StdString.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "UiConfiguration.h"
#include "HyperlinkWindow.h"

class HyperlinkWindow : public Panel {
public:
	HyperlinkWindow (const StdString &linkText = StdString (""), const StdString &linkUrl = StdString (""), UiConfiguration::FontType fontType = UiConfiguration::CaptionFont);
	virtual ~HyperlinkWindow ();

	// Read-write data members
	Widget::EventCallbackContext linkOpenCallback;

	// Read-only data members
	StdString url;
	int linkOpenResult;
	float maxLineHeight;

	// Set the link's text and target URL
	void setLink (const StdString &linkText, const StdString &linkUrl);

	// Set the amount of size padding that should be applied to the window
	void setPadding (float widthPadding, float heightPadding);

	// Return the provided y position value, adjusted as appropriate for the link's line height
	float getLinePosition (float targetY);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void windowClicked (void *windowPtr, Widget *widgetPtr);
	static void windowMouseEntered (void *windowPtr, Widget *widgetPtr);
	static void windowMouseExited (void *windowPtr, Widget *widgetPtr);

	Label *label;
	float labelWidth;
	float labelHeight;
};

#endif
