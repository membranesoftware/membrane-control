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
// Widget that holds an editable text value

#ifndef TEXT_FIELD_H
#define TEXT_FIELD_H

#include "Label.h"
#include "Panel.h"

class TextField : public Panel {
public:
	TextField (float fieldWidth, const StdString &promptText = StdString (""));
	~TextField ();

	// Read-only data members
	float fieldWidth;
	bool isEditing;

	// Return the text field's value
	StdString getValue ();

	// Clear the text field's value
	void clearValue ();

	// Set the text field's value
	void setValue (const StdString &valueText);

	// Read a string from the clipboard and append it to the text field's value
	void appendClipboardText ();

	// Set the field's editing state
	void setEditing (bool enable);

	// Set the text field's width
	void setFieldWidth (float widthValue);

	// Set the text field's width by specifying a line length in characters
	void setLineWidth (int lineLength);

	// Set a callback that should be invoked when the field ends an edit after an enter keypress
	void setEnterCallback (Widget::EventCallback callback, void *callbackData);

protected:
	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	virtual void doUpdate (int msElapsed, float originX, float originY);

	// Execute operations appropriate when the widget receives new mouse state
	virtual void doProcessMouseState (const Widget::MouseState &mouseState);

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void resetLayout ();

private:
	// Set the field's focused state
	void setFocused (bool enable);

	Widget::EventCallback enterCallback;
	void *enterCallbackData;
	Label *promptLabel;
	Label *valueLabel;
	Panel *cursorPanel;
	bool isFocused;
	int cursorClock;
	StdString lastValue;
};

#endif
