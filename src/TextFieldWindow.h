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
// Panel that holds a TextField widget and control buttons

#ifndef TEXT_FIELD_WINDOW_H
#define TEXT_FIELD_WINDOW_H

#include "TextField.h"
#include "Button.h"
#include "Sprite.h"
#include "Toggle.h"
#include "ImageWindow.h"
#include "Panel.h"

class TextFieldWindow : public Panel {
public:
	TextFieldWindow (float windowWidth, const StdString &promptText = StdString (""), Sprite *iconSprite = NULL);
	virtual ~TextFieldWindow ();

	// Read-only data members
	bool isDisabled;
	bool isInverseColor;
	bool isObscured;

	// Set the window's inverse color option
	void setInverseColor (bool inverse);

	// Set the text field's prompt error color state. If enabled, the text field shows its prompt text in the UiConfiguration error color.
	void setPromptErrorColor (bool enable);

	// Set the text field's disabled state, appropriate for use when the field becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the window's text field obscure option
	void setObscured (bool enable);

	// Set the window's width
	void setWindowWidth (float fixedWidth);

	// Set the window to use a fixed height of the specified value, replacing default window behavior that computes height as needed to contain its elements
	void setWindowHeight (float fixedHeight);

	// Set a callback that should be invoked when the window's text field completes an edit
	void setEditCallback (Widget::EventCallback callback, void *callbackData);

	// Set the enable state for the window's utility buttons
	void setButtonsEnabled (bool enableEnterButton = false, bool enableCancelButton = false, bool enablePasteButton = false, bool enableClearButton = false, bool enableRandomizeButton = false);

	// Return the text field's value
	StdString getValue ();

	// Set the text field's value
	void setValue (const StdString &valueText);

	// Assign keypress edit focus to the text field
	void assignKeyFocus ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static TextFieldWindow *castWidget (Widget *widget);

	// Callback functions
	static void textFieldValueChanged (void *windowPtr, Widget *widgetPtr);
	static void enterButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void cancelButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void pasteButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void clearButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void randomizeButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void visibilityToggleStateChanged (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	bool isFixedHeight;
	float windowWidth;
	float windowHeight;
	Widget::EventCallback editCallback;
	void *editCallbackData;
	TextField *textField;
	Button *enterButton;
	Button *cancelButton;
	Button *pasteButton;
	Button *clearButton;
	Button *randomizeButton;
	ImageWindow *iconImage;
	Toggle *visibilityToggle;
	StdString cancelValue;
	bool isCancelled;
};

#endif
