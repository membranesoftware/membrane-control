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
// Widget that holds text as a set of labels and maintains line flow

#ifndef TEXT_FLOW_H
#define TEXT_FLOW_H

#include <list>
#include "UiConfiguration.h"
#include "Label.h"
#include "Color.h"
#include "Panel.h"

class TextFlow : public Panel {
public:
	TextFlow (float viewWidth, UiConfiguration::FontType fontType = UiConfiguration::BodyFont);
	~TextFlow ();

	// Read-write data members
	int maxLineCount;

	// Read-only data members
	float viewWidth;
	int lineCount;
	StdString text;
	UiConfiguration::FontType textFontType;
	Font *textFont;
	StdString textFontName;
	int textFontSize;
	Color textColor;

	// Set the text flow's width
	void setViewWidth (float targetWidth);

	// Set the text flow's font
	void setFont (UiConfiguration::FontType fontType);

	// Set the text flow's text color
	void setTextColor (const Color &color);

	// Set the text flow's content, changing its active font if fontType is provided
	void setText (const StdString &textContent, UiConfiguration::FontType fontType = UiConfiguration::NoFont, bool forceFontReload = false);

	// Append new lines to the text flow's content
	void appendText (const StdString &textContent);

protected:
	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	// Create labels from linesText and add them to lineList
	void addLines (const StdString &linesText);

	std::list<Label *> lineList;
};

#endif
