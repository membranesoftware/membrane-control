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
// Widget that shows a text label

#ifndef LABEL_H
#define LABEL_H

#include <list>
#include "SDL2/SDL.h"
#include "SDL2/SDL_surface.h"
#include "StdString.h"
#include "Font.h"
#include "Color.h"
#include "Widget.h"

class Label : public Widget {
public:
	Label (const StdString &text, int fontType = UiConfiguration::BODY, const Color &color = Color (0.0f, 0.0f, 0.0f));
	~Label ();

	// Read-write data members
	Color textColor;

	// Read-only data members
	StdString text;
	int textFontType;
	Font *textFont;
	StdString textFontName;
	int textFontSize;
	float spaceWidth;
	float maxGlyphWidth;
	float maxLineHeight;
	float maxCharacterHeight;
	float descenderHeight;
	bool isUnderlined;

	// Remove characters from the end of a string value as needed for a Label to fit in the specified maximum width, including space for an optional truncate suffix
	static void truncateText (StdString *text, int fontType, float maxWidth, const StdString &truncateSuffix = StdString (""));

	// Truncate the provided text using the truncateText method and return the resulting string
	static StdString getTruncatedText (const StdString &text, int fontType, float maxWidth, const StdString &truncateSuffix = StdString (""));

	// Set the label's text, changing its active font if a type is provided
	void setText (const StdString &labelText, int fontType = -1, bool forceFontReload = false);

	// Set the label's font
	void setFont (int fontType);

	// Set the label's underline state
	void setUnderlined (bool enable);

	// Return the provided y position value, adjusted as appropriate for the label's line height
	float getLinePosition (float targetY);

	// Assign the widget's position to the provided x/y values, then reset positionX as appropriate for a rightward flow. If rightExtent and bottomExtent are provided, update them with the widget's right (x plus width) and bottom (y plus height) extents if greater.
	virtual void flowRight (float *positionX, float positionY, float *rightExtent = NULL, float *bottomExtent = NULL);

	// Assign the widget's position to the provided x/y values, then reset positionY as appropriate for a downward flow. If rightExtent and bottomExtent are provided, update them with the widget's right (x plus width) and bottom (y plus height) extents if greater.
	virtual void flowDown (float positionX, float *positionY, float *rightExtent = NULL, float *bottomExtent = NULL);

	// Assign the widget's y position to a centered value within the provided vertical extents
	virtual void centerVertical (float topExtent, float bottomExtent);

protected:
	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	virtual void doUpdate (int msElapsed, float originX, float originY);

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

	// Add subclass-specific draw commands for execution by the App
	virtual void doDraw ();

	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

private:
	std::list<Font::Glyph *> glyphList;
	int maxGlyphTopBearing;
	float underlineMargin;
	std::list<int> kerningList;
	SDL_mutex *textMutex;
};

#endif
