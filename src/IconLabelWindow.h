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
// Panel that contains an image and a text label

#ifndef ICON_LABEL_WINDOW_H
#define ICON_LABEL_WINDOW_H

#include <list>
#include "StdString.h"
#include "Sprite.h"
#include "Color.h"
#include "Image.h"
#include "Label.h"
#include "TextArea.h"
#include "Panel.h"

class IconLabelWindow : public Panel {
public:
	IconLabelWindow (Sprite *iconSprite, const StdString &iconText = StdString (""), int iconFontType = UiConfiguration::BodyFont, const Color &iconTextColor = Color (0.0f, 0.0f, 0.0f), bool isTextWordWrapped = false);
	~IconLabelWindow ();

	// Set the window's text
	void setText (const StdString &labelText);

	// Set the window's text change highlight option. If enabled, the window highlights text changes with a rotation from the specified color.
	void setTextChangeHighlight (bool enable, const Color &highlightColor = Color (0.0f, 0.0f, 0.0f));

	// Set the window's word wrap option. If enabled, the window applies specified maximums to word wrap its text content. maxTextLineLength specifies the maximum number of characters per text line, with a value of zero or less indicating that a default line length should be chosen from UiConfiguration. maxTextLineWidth specifies the maximum width in pixels per text line, with a value of zero or less indicating that no such maximum should apply.
	void setWordWrapped (bool enable, int maxTextLineLength = 0, int maxTextLineWidth = 0.0f);

	// Set the window's right-aligned option. If enabled, the window places the icon on the right side instead of the left side.
	void setRightAligned (bool enable);

	// Set the sprite frame that should be drawn by the window's icon image
	void setIconImageFrame (int frame);

	// Set the scale value that should be applied to the window's icon image
	void setIconImageScale (float scale);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	Label *label;
	Image *image;
	bool isWordWrapped;
	bool isRightAligned;
	bool isTextChangeHighlightEnabled;
	Color normalTextColor;
	Color highlightTextColor;
	TextArea *textArea;
};

#endif
