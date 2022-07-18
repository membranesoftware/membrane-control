/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Panel containing an icon and text, suitable for use as a generic item in a card view

#ifndef ICON_CARD_WINDOW_H
#define ICON_CARD_WINDOW_H

#include "StdString.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Color.h"
#include "Sprite.h"
#include "TextFlow.h"
#include "Label.h"
#include "Image.h"
#include "HyperlinkWindow.h"

class IconCardWindow : public Panel {
public:
	IconCardWindow (Sprite *iconSprite);
	virtual ~IconCardWindow ();

	// Read-write data members
	StdString itemId;

	// Set the content of the card's name label
	void setName (const StdString &text, UiConfiguration::FontType fontType = UiConfiguration::NoFont);

	// Set the content of the card's subtitle label
	void setSubtitle (const StdString &text, UiConfiguration::FontType fontType = UiConfiguration::NoFont);

	// Set the text color of the card's subtitle label
	void setSubtitleColor (const Color &color);

	// Set the content of the card's detail text area
	void setDetailText (const StdString &text, UiConfiguration::FontType fontType = UiConfiguration::NoFont);

	// Set the content of the card's link element
	void setLink (const StdString &text, const StdString &url);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	Image *iconImage;
	Label *nameLabel;
	Label *subtitleLabel;
	TextFlow *detailText;
	HyperlinkWindow *linkWindow;
};

#endif
