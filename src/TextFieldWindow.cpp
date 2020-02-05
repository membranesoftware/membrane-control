/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Panel.h"
#include "Button.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Toggle.h"
#include "UiConfiguration.h"
#include "TextField.h"
#include "TextFieldWindow.h"

const int TextFieldWindow::randomizeStringLength = 16;

TextFieldWindow::TextFieldWindow (float windowWidth, const StdString &promptText, Sprite *iconSprite)
: Panel ()
, shouldSkipTextClearCallbacks (false)
, isDisabled (false)
, isInverseColor (false)
, isObscured (false)
, isFixedHeight (false)
, windowWidth (windowWidth)
, windowHeight (0.0f)
, textField (NULL)
, enterButton (NULL)
, cancelButton (NULL)
, pasteButton (NULL)
, clearButton (NULL)
, randomizeButton (NULL)
, iconImage (NULL)
, visibilityToggle (NULL)
, isCancelled (false)
{
	UiConfiguration *uiconfig;
	UiText *uitext;
	Image *image;

	classId = ClassId::TextFieldWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize / 2.0f);

	textField = (TextField *) addWidget (new TextField (windowWidth - uiconfig->marginSize - uiconfig->paddingSize, promptText));
	textField->valueChangeCallback = Widget::EventCallbackContext (TextFieldWindow::textFieldValueChanged, this);
	textField->valueEditCallback = Widget::EventCallbackContext (TextFieldWindow::textFieldValueEdited, this);
	if (iconSprite) {
		image = new Image (iconSprite);
		iconImage = (ImageWindow *) addWidget (new ImageWindow (image));
		iconImage->setFillBg (true, uiconfig->darkBackgroundColor);
		iconImage->setWindowSize (image->width + uiconfig->paddingSize, textField->height);
	}

	enterButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::EnterTextButtonSprite)));
	enterButton->zLevel = 1;
	enterButton->setInverseColor (true);
	enterButton->setMouseClickCallback (TextFieldWindow::enterButtonClicked, this);
	enterButton->setMouseHoverTooltip (uitext->getText (UiTextString::textFieldEnterTooltip));
	enterButton->isVisible = false;

	cancelButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::CancelButtonSprite)));
	cancelButton->zLevel = 1;
	cancelButton->setInverseColor (true);
	cancelButton->setMouseClickCallback (TextFieldWindow::cancelButtonClicked, this);
	cancelButton->setMouseHoverTooltip (uitext->getText (UiTextString::cancel).capitalized ());
	cancelButton->isVisible = false;

	pasteButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::PasteButtonSprite)));
	pasteButton->zLevel = 1;
	pasteButton->setMouseClickCallback (TextFieldWindow::pasteButtonClicked, this);
	pasteButton->setMouseHoverTooltip (uitext->getText (UiTextString::textFieldPasteTooltip));
	pasteButton->isFocusDropShadowDisabled = true;
	pasteButton->isVisible = false;

	clearButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::ClearButtonSprite)));
	clearButton->zLevel = 1;
	clearButton->setMouseClickCallback (TextFieldWindow::clearButtonClicked, this);
	clearButton->setMouseHoverTooltip (uitext->getText (UiTextString::textFieldClearTooltip));
	clearButton->isFocusDropShadowDisabled = true;
	clearButton->isVisible = false;

	randomizeButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::RandomizeButtonSprite)));
	randomizeButton->zLevel = 1;
	randomizeButton->setMouseClickCallback (TextFieldWindow::randomizeButtonClicked, this);
	randomizeButton->setMouseHoverTooltip (uitext->getText (UiTextString::textFieldRandomizeTooltip));
	randomizeButton->isFocusDropShadowDisabled = true;
	randomizeButton->isVisible = false;

	refreshLayout ();
}

TextFieldWindow::~TextFieldWindow () {

}

StdString TextFieldWindow::toStringDetail () {
	return (StdString (" TextFieldWindow"));
}

bool TextFieldWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::TextFieldWindow));
}

TextFieldWindow *TextFieldWindow::castWidget (Widget *widget) {
	return (TextFieldWindow::isWidgetType (widget) ? (TextFieldWindow *) widget : NULL);
}

void TextFieldWindow::setDisabled (bool disabled) {
	if (isDisabled == disabled) {
		return;
	}

	isDisabled = disabled;
	textField->setDisabled (isDisabled);
	enterButton->setDisabled (isDisabled);
	cancelButton->setDisabled (isDisabled);
	pasteButton->setDisabled (isDisabled);
	clearButton->setDisabled (isDisabled);
	randomizeButton->setDisabled (isDisabled);
	if (visibilityToggle) {
		visibilityToggle->setDisabled (isDisabled);
	}
	refreshLayout ();
}

void TextFieldWindow::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}

	isInverseColor = inverse;
	textField->setInverseColor (isInverseColor);
	enterButton->setInverseColor (isInverseColor);
	cancelButton->setInverseColor (isInverseColor);
	pasteButton->setInverseColor (isInverseColor);
	clearButton->setInverseColor (isInverseColor);
	randomizeButton->setInverseColor (isInverseColor);
	if (visibilityToggle) {
		visibilityToggle->setInverseColor (isInverseColor);
	}
	refreshLayout ();
}

void TextFieldWindow::setPromptErrorColor (bool enable) {
	textField->setPromptErrorColor (enable);
}

void TextFieldWindow::setObscured (bool enable) {
	UiConfiguration *uiconfig;
	UiText *uitext;

	if (isObscured == enable) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	if (visibilityToggle) {
		visibilityToggle->isDestroyed = true;
		visibilityToggle = NULL;
	}
	isObscured = enable;
	if (isObscured) {
		textField->setObscured (true);
		visibilityToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::VisibilityOffButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::VisibilityOnButtonSprite)));
		visibilityToggle->setInverseColor (isInverseColor);
		visibilityToggle->setStateChangeCallback (TextFieldWindow::visibilityToggleStateChanged, this);
		visibilityToggle->setMouseHoverTooltip (uitext->getText (UiTextString::textFieldVisibilityToggleTooltip));
		visibilityToggle->zLevel = 1;
	}
	else {
		textField->setObscured (false);
	}
	refreshLayout ();
}

StdString TextFieldWindow::getValue () {
	if (isCancelled) {
		return (cancelValue);
	}
	return (textField->getValue ());
}

void TextFieldWindow::setValue (const StdString &valueText) {
	textField->setValue (valueText);
	cancelValue.assign (valueText);
}

void TextFieldWindow::assignKeyFocus () {
	App::instance->uiStack.setKeyFocusTarget (textField);
}

void TextFieldWindow::setWindowWidth (float fixedWidth) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	windowWidth = fixedWidth;
	textField->setFieldWidth (windowWidth - uiconfig->marginSize - uiconfig->paddingSize);
	refreshLayout ();
}

void TextFieldWindow::setWindowHeight (float fixedHeight) {
	isFixedHeight = true;
	windowHeight = fixedHeight;
	refreshLayout ();
}

void TextFieldWindow::setButtonsEnabled (bool enableEnterButton, bool enableCancelButton, bool enablePasteButton, bool enableClearButton, bool enableRandomizeButton) {
	enterButton->isVisible = enableEnterButton;
	cancelButton->isVisible = enableCancelButton;
	pasteButton->isVisible = enablePasteButton;
	clearButton->isVisible = enableClearButton;
	randomizeButton->isVisible = enableRandomizeButton;
	refreshLayout ();
}

void TextFieldWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, w, h;
	bool found;

	uiconfig = &(App::instance->uiConfig);
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
	if (visibilityToggle) {
		x -= visibilityToggle->width;
		visibilityToggle->position.assign (x, y - (visibilityToggle->height / 2.0f));
	}
	if (clearButton->isVisible) {
		x -= clearButton->width;
		clearButton->position.assign (x, y - (clearButton->height / 2.0f));
	}
	if (pasteButton->isVisible) {
		x -= pasteButton->width;
		pasteButton->position.assign (x, y - (pasteButton->height / 2.0f));
	}
	if (randomizeButton->isVisible) {
		x -= randomizeButton->width;
		randomizeButton->position.assign (x, y - (randomizeButton->height / 2.0f));
	}
}

void TextFieldWindow::textFieldValueChanged (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	if (window->valueChangeCallback.callback) {
		window->valueChangeCallback.callback (window->valueChangeCallback.callbackData, window);
	}
}

void TextFieldWindow::textFieldValueEdited (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	if (window->valueEditCallback.callback) {
		window->valueEditCallback.callback (window->valueEditCallback.callbackData, window);
	}
}

void TextFieldWindow::enterButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	window->textField->setKeyFocus (false);
}

void TextFieldWindow::cancelButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	window->isCancelled = true;
	window->isDestroyed = true;
}

void TextFieldWindow::pasteButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	App::instance->uiStack.setKeyFocusTarget (window->textField);
	window->textField->appendClipboardText ();
}

void TextFieldWindow::clearButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	App::instance->uiStack.setKeyFocusTarget (window->textField);
	window->textField->setValue (StdString (""), window->shouldSkipTextClearCallbacks, window->shouldSkipTextClearCallbacks);
}

void TextFieldWindow::randomizeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	window->textField->setValue (App::instance->getRandomString (TextFieldWindow::randomizeStringLength));
}

void TextFieldWindow::visibilityToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	TextFieldWindow *window;

	window = (TextFieldWindow *) windowPtr;
	if (window->visibilityToggle->isChecked) {
		window->textField->setObscured (false);
	}
	else {
		window->textField->setObscured (true);
	}
}
