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
// Panel that shows a thumbnail image from a media item

#ifndef THUMBNAIL_WINDOW_H
#define THUMBNAIL_WINDOW_H

#include "StdString.h"
#include "Sprite.h"
#include "ImageWindow.h"
#include "LabelWindow.h"
#include "Panel.h"

class ThumbnailWindow : public Panel {
public:
	// Constants to use for layout types
	enum {
		LOW_DETAIL = 0,
		MEDIUM_DETAIL = 1,
		HIGH_DETAIL = 2
	};

	ThumbnailWindow (float thumbnailTimestamp, int sourceWidth, int sourceHeight, const StdString &sourceUrl, Sprite *loadingThumbnailSprite, int cardLayout = ThumbnailWindow::LOW_DETAIL, float maxMediaImageWidth = 64.0f);
	virtual ~ThumbnailWindow ();

	// Read-only data members
	float thumbnailTimestamp;
	int sourceWidth;
	int sourceHeight;
	StdString sourceUrl;

	// Set the card's layout type and reset widgets to show the specified content
	void setLayout (int cardLayout, float maxImageWidth);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static ThumbnailWindow *castWidget (Widget *widget);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations appropriate when the widget receives new mouse state
	void doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void resetLayout ();

private:
	Sprite *loadingSprite;
	ImageWindow *mediaImage;
	LabelWindow *timestampLabel;
};

#endif
