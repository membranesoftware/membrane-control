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
// Widget that holds a text flow and provides vertical scrolling

#ifndef TEXT_AREA_H
#define TEXT_AREA_H

#include "UiConfiguration.h"
#include "Label.h"
#include "Font.h"
#include "Color.h"
#include "Panel.h"
#include "TextFlow.h"
#include "ScrollBar.h"
#include "ScrollView.h"

class TextArea : public ScrollView {
public:
	TextArea (float viewWidth, int viewLineCount = 8, UiConfiguration::FontType fontType = UiConfiguration::BodyFont, bool isScrollEnabled = false);
	~TextArea ();

	// Set the text area's content, changing its active font if a type is provided
	void setText (const StdString &textContent, UiConfiguration::FontType fontType = UiConfiguration::NoFont, bool forceFontReload = false);

	// Append new lines to the text area's content, optionally moving the view origin to the lowest extent if scrolling is enabled
	void appendText (const StdString &textContent, bool scrollToBottom = false);

	// Set the text area's font
	void setFont (UiConfiguration::FontType fontType);

	// Set the text area's text color
	void setTextColor (const Color &color);

	// Set the text area's inverse color state. If enabled, the text area renders using an inverse color scheme.
	void setInverseColor (bool isInverseColor);

	// Set the maximum number of lines that should be held in the text area. Appending text lines beyond this limit causes old lines to be discarded.
	void setMaxLineCount (int max);

protected:
	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	// Callback functions
	static void scrollBarPositionChanged (void *textAreaPtr, Widget *widgetPtr);

	TextFlow *textFlow;
	ScrollBar *scrollBar;
};

#endif
