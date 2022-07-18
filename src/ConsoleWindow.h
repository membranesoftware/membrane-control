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
// Panel that shows console output and accepts script commands

#ifndef CONSOLE_WINDOW_H
#define CONSOLE_WINDOW_H

#include <list>
#include "StdString.h"
#include "IconLabelWindow.h"
#include "Button.h"
#include "TextArea.h"
#include "TextFieldWindow.h"
#include "Panel.h"

class ConsoleWindow : public Panel {
public:
	ConsoleWindow ();
	virtual ~ConsoleWindow ();

	static const float WindowWidthMultiplier;
	static const int MaxTextLines;

	// Read-only data members
	float windowWidth;

	// Assign keypress edit focus to the console's text field
	void assignKeyFocus ();

	// Append text as console window output
	void appendText (const StdString &text);

protected:
	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void closeButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void textFieldWindowValueEdited (void *windowPtr, Widget *widgetPtr);

	IconLabelWindow *icon;
	Button *closeButton;
	TextArea *textArea;
	TextFieldWindow *textField;
};

#endif
