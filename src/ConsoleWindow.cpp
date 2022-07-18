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
#include "Config.h"
#include <stdlib.h>
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "LuaScript.h"
#include "TaskGroup.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Button.h"
#include "IconLabelWindow.h"
#include "TextArea.h"
#include "ConsoleWindow.h"

const float ConsoleWindow::WindowWidthMultiplier = 0.64f;
const int ConsoleWindow::MaxTextLines = 1024;

ConsoleWindow::ConsoleWindow ()
: Panel ()
, windowWidth (0.0f)
, icon (NULL)
, closeButton (NULL)
, textArea (NULL)
, textField (NULL)
{
	StringList lines;

	classId = ClassId::ConsoleWindow;
	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	windowWidth = App::instance->windowWidth * ConsoleWindow::WindowWidthMultiplier;

	icon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ConsoleIconSprite), UiText::instance->getText (UiTextString::Console).capitalized ()));
	icon->setPadding (0.0f, 0.0f);

	closeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExitButtonSprite)));
	closeButton->mouseClickCallback = Widget::EventCallbackContext (ConsoleWindow::closeButtonClicked, this);
	closeButton->zLevel = 1;
	closeButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);

	textArea = (TextArea *) addWidget (new TextArea (windowWidth, UiConfiguration::instance->textAreaLargeLineCount, UiConfiguration::ConsoleFont, true));
	textArea->setMaxLineCount (ConsoleWindow::MaxTextLines);
	textArea->setInverseColor (true);
	lines.push_back (StdString (""));
	lines.push_back (UiText::instance->getText (UiTextString::ConsoleStartText2));
	lines.push_back (UiText::instance->getText (UiTextString::ConsoleStartText3));
	lines.push_back (UiText::instance->getText (UiTextString::ConsoleStartText4));
	textArea->setText (lines.join ("\n"));

	textField = (TextFieldWindow *) addWidget (new TextFieldWindow (windowWidth, UiText::instance->getText (UiTextString::ConsoleWindowPrompt)));
	textField->setPadding (0.0f, 0.0f);
	textField->setButtonsEnabled (false, false, true, true, false);
	textField->setRetainFocusOnReturnKey (true);
	textField->valueEditCallback = Widget::EventCallbackContext (ConsoleWindow::textFieldWindowValueEdited, this);

	refreshLayout ();
}

ConsoleWindow::~ConsoleWindow () {

}

void ConsoleWindow::assignKeyFocus () {
	textField->assignKeyFocus ();
}

void ConsoleWindow::doRefresh () {
	windowWidth = App::instance->windowWidth * ConsoleWindow::WindowWidthMultiplier;
	textField->setWindowWidth (windowWidth);
	Panel::doRefresh ();
}

void ConsoleWindow::refreshLayout () {
	float x, y, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x2 = 0.0f;
	y2 = 0.0f;
	icon->flowRight (&x, y, &x2, &y2);
	closeButton->flowRight (&x, y, &x2, &y2);
	icon->centerVertical (y, y2);
	closeButton->centerVertical (y, y2);

	x = widthPadding;
	y = y2 + UiConfiguration::instance->marginSize;
	textArea->flowDown (x, &y, &x2, &y2);
	textField->flowDown (x, &y, &x2, &y2);

	resetSize ();

	x = width - widthPadding;
	closeButton->flowLeft (&x);
}

void ConsoleWindow::appendText (const StdString &text) {
	textArea->appendText (text, true);
}

void ConsoleWindow::closeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ConsoleWindow *window;

	window = (ConsoleWindow *) windowPtr;
	window->isDestroyed = true;
}

void ConsoleWindow::textFieldWindowValueEdited (void *windowPtr, Widget *widgetPtr) {
	ConsoleWindow *window;
	TextFieldWindow *textfield;
	StdString script;
	LuaScript *lua;

	window = (ConsoleWindow *) windowPtr;
	textfield = (TextFieldWindow *) widgetPtr;
	script = textfield->getValue ();

	lua = new LuaScript (script);
	if (! TaskGroup::instance->run (TaskGroup::RunContext (LuaScript::run, lua))) {
		Log::debug ("Failed to execute task LuaScript::run");
		delete (lua);
	}
	else {
		window->textArea->appendText (StdString::createSprintf ("\n> %s", script.c_str ()), true);
		textfield->setValue (StdString (""), true, true);
	}
}
