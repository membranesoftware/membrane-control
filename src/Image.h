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
// Widget that draws image data from a Sprite resource

#ifndef IMAGE_H
#define IMAGE_H

#include "SDL2/SDL_surface.h"
#include "StdString.h"
#include "Color.h"
#include "Sprite.h"
#include "SpriteHandle.h"
#include "Widget.h"

class Image : public Widget {
public:
	Image (Sprite *sprite, int spriteFrame = 0, bool shouldDestroySprite = false);
	~Image ();

	// Read-write data members
	float maxSpriteWidth;
	float maxSpriteHeight;
	float drawAlpha;

	// Set the sprite frame for use in drawing the image
	void setFrame (int frame);

	// Set the scale factor for use in drawing the image
	void setScale (float scale);

	// Set the image's mouse highlight scale option. If enabled, the image uses the specified scale factor during mouseover events.
	void setMouseHighlightScale (bool enable, float highlightScale);

	// Set the image's sprite destroy option. If enabled, the image unloads and destroys its source sprite when itself destroyed.
	void setSpriteDestroy (bool enable);

	// Set the image's draw color option. If enabled, the image renders its texture with a filter of the specified color.
	void setDrawColor (bool enable, const Color &color = Color ());

	// Return a boolean value indicating if the image uses the specified Sprite object
	bool hasSprite (Sprite *sprite) const;

protected:
	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	virtual void doUpdate (int msElapsed, float originX, float originY);

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

	// Add subclass-specific draw commands for execution by the App
	virtual void doDraw ();

	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the widget's width and height values
	void resetSize ();

	bool isDrawColorEnabled;
	Color drawColor;
	SpriteHandle spriteHandle;
	bool shouldDestroySprite;
	float drawScale;
	bool isMouseHighlightScaled;
	float mouseHighlightScale;
};

#endif
