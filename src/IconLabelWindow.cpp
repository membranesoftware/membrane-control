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
#include "Util.h"
#include "Sprite.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "UiConfiguration.h"
#include "IconLabelWindow.h"

IconLabelWindow::IconLabelWindow (Sprite *iconSprite, const StdString &iconText, int iconFontType, const Color &iconTextColor, bool isTextWordWrapped)
: Panel ()
, label (NULL)
, image (NULL)
, isWordWrapped (false)
, isRightAligned (false)
, textArea (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, 0.0f);

	label = (Label *) addWidget (new Label (iconText, iconFontType, iconTextColor));
	image = (Image *) addWidget (new Image (iconSprite));
	textArea = (TextArea *) addWidget (new TextArea (iconFontType, iconTextColor));
	textArea->isVisible = false;

	setWordWrapped (isTextWordWrapped);
	resetLayout ();
}

IconLabelWindow::~IconLabelWindow () {

}

StdString IconLabelWindow::toStringDetail () {
	return (StdString (" IconLabelWindow"));
}

void IconLabelWindow::setText (const StdString &text) {
	label->setText (text);
	resetLayout ();
}

void IconLabelWindow::setWordWrapped (bool enable, int maxTextLineLength, int maxTextLineWidth) {
	if (enable == isWordWrapped) {
		return;
	}
	isWordWrapped = enable;
	if (isWordWrapped) {
		// TODO: Apply the provided maxTextLineLength and maxTextLineWidth values (currently ignored)
		textArea->setText (label->text);
	}
	else {
		// TODO: Copy textArea content to label (an operation currently not supported by TextArea)
	}
	resetLayout ();
}

void IconLabelWindow::setRightAligned (bool enable) {
	if (enable == isRightAligned) {
		return;
	}
	isRightAligned = enable;
	resetLayout ();
}

void IconLabelWindow::resetLayout () {
	UiConfiguration *uiconfig;
	float x, y, h;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = widthPadding;
	y = heightPadding;

	if (isWordWrapped) {
		label->isVisible = false;

		if (isRightAligned) {
			textArea->position.assign (x, y);
			x += textArea->width + uiconfig->marginSize;
			image->position.assign (x, y);
		}
		else {
			image->position.assign (x, y);
			x += image->width + uiconfig->marginSize;
			textArea->position.assign (x, y);
		}
		textArea->isVisible = true;
	}
	else {
		textArea->isVisible = false;

		h = label->height;
		if (image->height > h) {
			h = image->height;
		}
		if (isRightAligned) {
			label->position.assign (x, y + (h / 2.0f) - (label->height / 2.0f) + label->descenderHeight);
			x += label->width + uiconfig->marginSize;
			image->position.assign (x, y + (h / 2.0f) - (image->height / 2.0f));
		}
		else {
			image->position.assign (x, y + (h / 2.0f) - (image->height / 2.0f));
			x += image->width + uiconfig->marginSize;
			label->position.assign (x, y + (h / 2.0f) - (label->height / 2.0f) + label->descenderHeight);
		}
		label->isVisible = true;
	}

	resetSize ();
}
