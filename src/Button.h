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
// Widget that behaves as a clickable button

#ifndef BUTTON_H
#define BUTTON_H

#include "SDL2/SDL.h"
#include "Label.h"
#include "Image.h"
#include "Sprite.h"
#include "Color.h"
#include "Panel.h"

class Button : public Panel {
public:
	Button (Sprite *sprite = NULL, const StdString &labelText = StdString (""));
	virtual ~Button ();

	static const float FocusTextOffset;

	// Read-write data members
	SDL_Keycode shortcutKey;
	bool isFocusDropShadowDisabled;

	// Read-only data members
	float maxImageWidth, maxImageHeight;
	Color raiseNormalBgColor;
	Color normalTextColor;
	bool isFocused;
	bool isPressed;
	bool isDisabled;
	bool isRaised;
	bool isInverseColor;
	bool isImageColorEnabled;

	// Set the amount of size padding that should be applied to the button
	void setPadding (float widthPaddingSize, float heightPaddingSize);

	// Set the button's text
	void setText (const StdString &text);

	// Set the button's text color
	void setTextColor (const Color &textColor);

	// Set the button's image color
	void setImageColor (const Color &imageColor);

	// Set the button's raised state. If enabled, the button is drawn with a raised appearance.
	void setRaised (bool raised, const Color &normalBgColor = Color ());

	// Set the button's inverse color state. If enabled, the button renders using an inverse color scheme.
	void setInverseColor (bool inverse);

	// Set the button's pressed state, appropriate for use when the button has been activated
	void setPressed (bool pressed);

	// Set the button's disabled state, appropriate for use when the button becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the button's focused state, appropriate for use when the button has input focus
	void setFocused (bool focused);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static Button *castWidget (Widget *widget);

	// Callback functions
	static void mouseEntered (void *buttonPtr, Widget *widgetPtr);
	static void mouseExited (void *buttonPtr, Widget *widgetPtr);
	static void mousePressed (void *buttonPtr, Widget *widgetPtr);
	static void mouseReleased (void *buttonPtr, Widget *widgetPtr);

protected:
	// Execute subclass-specific operations to update object state as appropriate for an elapsed millisecond time period
	virtual void doUpdate (int msElapsed);

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	Label *label;
	Image *image;
	int pressClock;
};

#endif
