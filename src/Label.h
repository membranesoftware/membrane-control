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
// Widget that shows a text label

#ifndef LABEL_H
#define LABEL_H

#include <list>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "UiConfiguration.h"
#include "Font.h"
#include "Color.h"
#include "Widget.h"

class Label : public Widget {
public:
	Label (const StdString &text, UiConfiguration::FontType fontType = UiConfiguration::BodyFont, const Color &color = Color (0.0f, 0.0f, 0.0f));
	~Label ();

	static const char ObscureCharacter;

	// Read-write data members
	Color textColor;

	// Read-only data members
	StdString text;
	UiConfiguration::FontType textFontType;
	Font *textFont;
	StdString textFontName;
	int textFontSize;
	float spaceWidth;
	float maxGlyphWidth;
	float maxLineHeight;
	float maxCharacterHeight;
	float descenderHeight;
	bool isUnderlined;
	bool isObscured;

	// Set the label's text, changing its active font if a type is provided
	void setText (const StdString &textContent, UiConfiguration::FontType fontType = UiConfiguration::NoFont, bool forceFontReload = false);

	// Set the label's font
	void setFont (UiConfiguration::FontType fontType);

	// Set the label's underline state
	void setUnderlined (bool enable);

	// Set the label's obscured state. If enabled, the label renders using spacer characters to conceal its value.
	void setObscured (bool enable);

	// Return the provided y position value, adjusted as appropriate for the label's line height
	float getLinePosition (float targetY);

	// Return the width of the label's text up to the specified character position
	float getCharacterPosition (int position);

	// Assign the widget's position to the provided x/y values, then reset positionX as appropriate for a rightward flow. If rightExtent and bottomExtent are provided, update them with the widget's right (x plus width) and bottom (y plus height) extents if greater.
	virtual void flowRight (float *positionX, float positionY, float *rightExtent = NULL, float *bottomExtent = NULL);

	// Assign the widget's position to the provided x/y values, then reset positionY as appropriate for a downward flow. If rightExtent and bottomExtent are provided, update them with the widget's right (x plus width) and bottom (y plus height) extents if greater.
	virtual void flowDown (float positionX, float *positionY, float *rightExtent = NULL, float *bottomExtent = NULL);

	// Assign the widget's y position to a centered value within the provided vertical extents
	virtual void centerVertical (float topExtent, float bottomExtent);

protected:
	// Execute operations to update object state as appropriate for an elapsed millisecond time period
	virtual void doUpdate (int msElapsed);

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

	// Add subclass-specific draw commands for execution by the App. If targetTexture is non-NULL, it has been set as the render target and draw commands should adjust coordinates as appropriate.
	virtual void doDraw (SDL_Texture *targetTexture, float originX, float originY);

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
