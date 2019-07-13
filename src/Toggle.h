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
// Widget that shows one of two buttons to indicate a checked or unchecked state

#ifndef TOGGLE_H
#define TOGGLE_H

#include "SDL2/SDL.h"
#include "StdString.h"
#include "Sprite.h"
#include "Button.h"
#include "Color.h"
#include "Panel.h"

class Toggle : public Panel {
public:
	// If all button sprites are non-NULL, the toggle uses those instead of the default assets from core sprites
	Toggle (Sprite *uncheckedButtonSprite = NULL, Sprite *checkedButtonSprite = NULL);
	virtual ~Toggle ();

	// Read-write data members
	SDL_Keycode shortcutKey;
	bool isFocusDropShadowDisabled;

	// Read-only data members
	bool isChecked;
	bool isDisabled;
	bool isFocused;

	// Set the toggle's checked state and invoke any configured change callback unless shouldSkipChangeCallback is true
	void setChecked (bool checked, bool shouldSkipChangeCallback = false);

	// Set the toggle's disabled state, appropriate for use when the toggle becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the toggle's inverse color state. If enabled, the toggle uses an inverse color scheme.
	void setInverseColor (bool inverse);

	// Set the draw color for the toggle's button images
	void setImageColor (const Color &imageColor);

	// Set a callback that should be invoked when the toggle button's checked state changes
	void setStateChangeCallback (Widget::EventCallback callback, void *callbackData);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static Toggle *castWidget (Widget *widget);

	// Callback functions
	static void mouseEntered (void *togglePtr, Widget *widgetPtr);
	static void mouseExited (void *togglePtr, Widget *widgetPtr);
	static void mousePressed (void *togglePtr, Widget *widgetPtr);
	static void mouseReleased (void *togglePtr, Widget *widgetPtr);
	static void mouseClicked (void *togglePtr, Widget *widgetPtr);

protected:
	// Execute operations appropriate when the widget's input state is reset
	virtual void doResetInputState ();

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	Button *uncheckedButton;
	Button *checkedButton;
	Widget::EventCallback stateChangeCallback;
	void *stateChangeCallbackData;
};

#endif
