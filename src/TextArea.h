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
// Widget that holds a set of text and maintains line flow

#ifndef TEXT_AREA_H
#define TEXT_AREA_H

#include <list>
#include "Label.h"
#include "Font.h"
#include "Color.h"
#include "Panel.h"

class TextArea : public Panel {
public:
	// maxTextLineLength specifies the maximum number of characters per text line, with a value of zero or less indicating that a default line length should be chosen from UiConfiguration. maxTextLineWidth specifies the maximum width in pixels per text line, with a value of zero or less indicating that no such maximum should apply.
	TextArea (int fontType, const Color &textColor = Color (0.0f, 0.0f, 0.0f), int maxTextLineLength = 0, float maxTextLineWidth = 0.0f);
	~TextArea ();

	// Clear the text area's content
	void clear ();

	// Set the maximum draw width that should be used for each line of text
	void setMaxLineWidth (float maxTextLineWidth);

	// Set the color that should be used to draw text
	void setTextColor (const Color &color);

	// Set the text area's content
	void setText (const StdString &text);

	// Set the amount of size padding that should be applied to the text area
	void setPadding (float widthPaddingSize, float heightPaddingSize);

protected:
	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	int textFontType;
	Color textColor;
	int maxLineLength;
	float maxLineWidth;
	std::list<Label *> lineList;
};

#endif
