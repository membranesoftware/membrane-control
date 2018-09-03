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
// Widget that allows selection of a bounded numeric value

#ifndef SLIDER_H
#define SLIDER_H

#include <list>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "Color.h"
#include "Sprite.h"

class Slider : public Widget {
public:
	Slider (float minValue = 0.0f, float maxValue = 1.0f);
	virtual ~Slider ();

	// Read-only data members
	float value;
	float hoverValue;
	float minValue;
	float maxValue;
	bool isHovering;
	bool isDragging;

	// Set the slider's value
	void setValue (float sliderValue);

	// Set a callback function that should be invoked when the slider's value changes
	void setValueChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback function that should be invoked when a slider hover value becomes available
	void setValueHoverCallback (Widget::EventCallback callback, void *callbackData);

	// Add the specified value as a snap position on the slider. If at least one snap position is present, changes to the slider value are rounded down to the nearest snap value.
	void addSnapValue (float snapValue);

	// Callback functions
	static void createThumbTextureComplete (void *sliderPtr, const StdString &path, SDL_Surface *surface, SDL_Texture *texture);

protected:
	// Execute subclass-specific operations to update object state as appropriate for an elapsed millisecond time period and origin position
	virtual void doUpdate (int msElapsed, float originX, float originY);

	// Add subclass-specific draw commands for execution by the App
	virtual void doDraw ();

	// Execute operations appropriate when the widget receives new mouse state
	virtual void doProcessMouseState (const Widget::MouseState &mouseState);

	// Execute operations appropriate when the widget's input state is reset
	virtual void doResetInputState ();

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

private:
	// Reset layout as appropriate for current state
	void resetLayout ();

	// Return the value indicated by the specified target after applying any configured snap positions
	float getSnappedValue (float targetValue);

	// Execute operations to populate thumbSprite with a texture for use in rendering the slider's thumb element
	void loadThumbSprite ();

	float thumbSize;
	Color thumbColor;
	bool isThumbSpriteLoaded;
	bool isThumbSpriteLoading;
	Sprite *thumbSprite;
	Uint32 *thumbSpritePixels;
	float trackWidth;
	float trackHeight;
	Color trackColor;
	float hoverSize;
	Color hoverColor;
	std::list<float> snapValueList;
	Widget::EventCallback valueChangeCallback;
	void *valueChangeCallbackData;
	Widget::EventCallback valueHoverCallback;
	void *valueHoverCallbackData;
};

#endif
