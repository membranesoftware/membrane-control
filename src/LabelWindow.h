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
// Panel that holds a Label widget and surrounding features

#ifndef LABEL_WINDOW_H
#define LABEL_WINDOW_H

#include "Label.h"
#include "Sprite.h"
#include "Panel.h"

class LabelWindow : public Panel {
public:
	LabelWindow (Label *label);
	virtual ~LabelWindow ();

	// Set a fixed size for the window. If the window's label does not fill the provided width, it is optionally centered inside a larger background space.
	void setWindowWidth (float fixedWidth, bool isLabelCentered = false);

	// Set the amount of size padding that should be applied to the window
	void setPadding (float widthPadding, float heightPadding);

	// Set the label's text color
	void setTextColor (const Color &color);

	// Execute a rotation operation for the label's text color
	void rotateTextColor (const Color &color, int duration);

	// Return the text string shown in the window's label
	StdString getText () const;

	// Set the text value shown in the label
	void setText (const StdString &text);

	// Set the font type used in the label
	void setFont (int fontType);

	// Set the label window's mouseover highlight option. If enabled, the window changes background and text color during mouseover events.
	void setMouseoverHighlight (bool enable, const Color &normalTextColor = Color (), const Color &normalBgColor = Color (), const Color &highlightTextColor = Color (), const Color &highlightBgColor = Color (), int colorRotateDuration = 0);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations appropriate when the widget receives new mouse state
	void doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	Label *label;
	bool isFixedWidth;
	bool isWidthCentered;
	float windowWidth;
	bool isMouseoverHighlightEnabled;
	Color mouseoverNormalTextColor;
	Color mouseoverNormalBgColor;
	Color mouseoverHighlightTextColor;
	Color mouseoverHighlightBgColor;
	int mouseoverColorRotateDuration;
};

#endif
