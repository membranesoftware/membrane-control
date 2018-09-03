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
#include "Config.h"
#include <stdlib.h>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Panel.h"
#include "Button.h"
#include "Image.h"
#include "ImageWindow.h"
#include "UiConfiguration.h"
#include "TextField.h"
#include "TextFieldWindow.h"

TextFieldWindow::TextFieldWindow (float windowWidth, const StdString &promptText, Sprite *iconSprite)
: Panel ()
, isFixedHeight (false)
, windowWidth (windowWidth)
, windowHeight (0.0f)
, editCallback (NULL)
, editCallbackData (NULL)
, textField (NULL)
, enterButton (NULL)
, cancelButton (NULL)
, pasteButton (NULL)
, clearButton (NULL)
, iconImage (NULL)
, shouldResetEditing (false)
{
	UiConfiguration *uiconfig;
	UiText *uitext;
	Image *image;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize / 2.0f);

	textField = (TextField *) addWidget (new TextField (windowWidth - uiconfig->marginSize - uiconfig->paddingSize, promptText));
	textField->setBorder (false);
	textField->setEnterCallback (TextFieldWindow::textFieldEntered, this);
	if (iconSprite) {
		image = new Image (iconSprite);
		iconImage = (ImageWindow *) addWidget (new ImageWindow (image));
		iconImage->setFillBg (true, uiconfig->darkBackgroundColor);
		iconImage->setWindowSize (image->width + uiconfig->paddingSize, textField->height);
	}

	enterButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::ENTER_TEXT_BUTTON)));
	enterButton->setMouseClickCallback (TextFieldWindow::enterButtonClicked, this);
	enterButton->setMouseHoverTooltip (uitext->confirm.capitalized ());
	enterButton->isVisible = false;

	cancelButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::CANCEL_BUTTON)));
	cancelButton->setMouseClickCallback (TextFieldWindow::cancelButtonClicked, this);
	cancelButton->setMouseHoverTooltip (uitext->cancel.capitalized ());
	cancelButton->isVisible = false;

	pasteButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::PASTE_BUTTON)));
	pasteButton->setMouseClickCallback (TextFieldWindow::pasteButtonClicked, this);
	pasteButton->setInverseColor (true);
	pasteButton->setRaised (true, uiconfig->darkBackgroundColor);
	pasteButton->setMouseHoverTooltip (uitext->textFieldPasteTooltip);
	pasteButton->isVisible = false;

	clearButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::CLEAR_BUTTON)));
	clearButton->setMouseClickCallback (TextFieldWindow::clearButtonClicked, this);
	clearButton->setInverseColor (true);
	clearButton->setRaised (true, uiconfig->darkBackgroundColor);
	clearButton->setMouseHoverTooltip (uitext->textFieldClearTooltip);
	clearButton->isVisible = false;

	resetLayout ();
}

TextFieldWindow::~TextFieldWindow () {

}

StdString TextFieldWindow::toStringDetail () {
	return (StdString (" TextFieldWindow"));
}

StdString TextFieldWindow::getValue () {
	return (textField->getValue ());
}

void TextFieldWindow::setValue (const StdString &valueText) {
	textField->setValue (valueText);
}

void TextFieldWindow::setWindowHeight (float fixedHeight) {
	isFixedHeight = true;
	windowHeight = fixedHeight;
	resetLayout ();
}

void TextFieldWindow::setEditing (bool enable) {
	textField->setEditing (enable);
}

void TextFieldWindow::setEditCallback (Widget::EventCallback callback, void *callbackData) {
	editCallback = callback;
	editCallbackData = callbackData;
}

void TextFieldWindow::setButtonsEnabled (bool enableEnterButton, bool enableCancelButton, bool enablePasteButton, bool enableClearButton) {
	enterButton->isVisible = enableEnterButton;
	cancelButton->isVisible = enableCancelButton;
	pasteButton->isVisible = enablePasteButton;
	clearButton->isVisible = enableClearButton;
	resetLayout ();
}

void TextFieldWindow::resetLayout () {
	UiConfiguration *uiconfig;
	float x, y, w, h;
	bool found;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = 0.0f;
	y = heightPadding;
	w = windowWidth;
	found = false;
	if (iconImage) {
		w -= iconImage->width;
		found = true;
	}
	if (enterButton->isVisible) {
		w -= enterButton->width + uiconfig->marginSize;
		found = true;
	}
	if (cancelButton->isVisible) {
		w -= cancelButton->width + uiconfig->marginSize;
		found = true;
	}
	if (found) {
		w -= widthPadding;
		x += widthPadding;
	}
	textField->setFieldWidth (w);

	h = 0.0f;
	if (iconImage) {
		iconImage->position.assign (x, y);
		x += iconImage->width;
		if (! isFixedHeight) {
			if (iconImage->height > h) {
				h = iconImage->height;
			}
		}
	}

	if (isFixedHeight) {
		y = (windowHeight / 2.0f) - (textField->height / 2.0f);
	}
	else {
		if (textField->height > h) {
			h = textField->height;
		}
	}
	textField->position.assign (x, y);
	x += textField->width + uiconfig->marginSize;

	if (cancelButton->isVisible) {
		if (isFixedHeight) {
			y = (windowHeight / 2.0f) - (cancelButton->height / 2.0f);
		}
		else {
			if (cancelButton->height > h) {
				h = cancelButton->height;
			}
		}
		cancelButton->position.assign (x, y);
		x += cancelButton->width + uiconfig->marginSize;
	}

	if (enterButton->isVisible) {
		if (isFixedHeight) {
			y = (windowHeight / 2.0f) - (enterButton->height / 2.0f);
		}
		else {
			if (enterButton->height > h) {
				h = enterButton->height;
			}
		}
		enterButton->position.assign (x, y);
		x += enterButton->width + uiconfig->marginSize;
	}

	if (iconImage) {
		iconImage->position.assignY (textField->position.y + (textField->height / 2.0f) - (iconImage->height / 2.0f));
	}

	if (isFixedHeight) {
		h = windowHeight;
	}
	else {
		h += (heightPadding * 2.0f);
	}
	setFixedSize (true, windowWidth, h);

	textField->position.assignY ((h / 2.0f) - (textField->height / 2.0f));
	x = textField->position.x + textField->width;
	y = textField->position.y + (textField->height / 2.0f);
	if (clearButton->isVisible) {
		x -= clearButton->width;
		clearButton->position.assign (x, y - (clearButton->height / 2.0f));
	}
	if (pasteButton->isVisible) {
		x -= pasteButton->width;
		pasteButton->position.assign (x, y - (pasteButton->height / 2.0f));
	}
}

void TextFieldWindow::doUpdate (int msElapsed, float originX, float originY) {
	Panel::doUpdate (msElapsed, originX, originY);

	if (shouldResetEditing) {
		setEditing (true);
		shouldResetEditing = false;
	}
}

void TextFieldWindow::textFieldEntered (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	if (window->editCallback) {
		window->editCallback (window->editCallbackData, window);
	}
}

void TextFieldWindow::enterButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	window->textField->setEditing (false);
	if (window->editCallback) {
		window->editCallback (window->editCallbackData, window);
	}
}

void TextFieldWindow::cancelButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	window->isDestroyed = true;
}

void TextFieldWindow::pasteButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;
	bool editing;

	window = (TextFieldWindow *) windowPtr;
	editing = window->textField->isEditing;
	window->textField->appendClipboardText ();
	if (editing) {
		window->shouldResetEditing = true;
	}
}

void TextFieldWindow::clearButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;
	bool editing;

	window = (TextFieldWindow *) windowPtr;
	editing = window->textField->isEditing;
	window->textField->clearValue ();
	if (editing) {
		window->shouldResetEditing = true;
	}
}
