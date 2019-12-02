/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "App.h"
#include "Log.h"
#include "StdString.h"
#include "UiConfiguration.h"
#include "Sprite.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "ProgressBar.h"
#include "IconLabelWindow.h"

IconLabelWindow::IconLabelWindow (Sprite *iconSprite, const StdString &iconText, int iconFontType, const Color &iconTextColor)
: Panel ()
, label (NULL)
, image (NULL)
, isRightAligned (false)
, isTextChangeHighlightEnabled (false)
, progressBar (NULL)
, textClickCallback (NULL)
, textClickCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	setPadding (uiconfig->paddingSize, 0.0f);

	normalTextColor.assign (iconTextColor);
	label = (Label *) addWidget (new Label (iconText, iconFontType, normalTextColor));
	label->setMouseClickCallback (IconLabelWindow::labelClicked, this);

	image = (Image *) addWidget (new Image (iconSprite));

	refreshLayout ();
}

IconLabelWindow::~IconLabelWindow () {

}

StdString IconLabelWindow::toStringDetail () {
	return (StdString (" IconLabelWindow"));
}

void IconLabelWindow::setText (const StdString &text) {
	if (isTextChangeHighlightEnabled) {
		if ((! label->text.empty ()) && (! text.equals (label->text))) {
			label->textColor.assign (highlightTextColor);
			label->textColor.translate (normalTextColor, App::instance->uiConfig.longColorTranslateDuration);
		}
	}
	label->setText (text);
	refreshLayout ();
}

void IconLabelWindow::setTextClickCallback (Widget::EventCallback callback, void *callbackData) {
	textClickCallback = callback;
	textClickCallbackData = callbackData;
}

void IconLabelWindow::labelClicked (void *windowPtr, Widget *widgetPtr) {
	IconLabelWindow *window;

	window = (IconLabelWindow *) windowPtr;
	if (window->textClickCallback) {
		window->textClickCallback (window->textClickCallbackData, window);
	}
}

void IconLabelWindow::setTextColor (const Color &textColor) {
	label->textColor.assign (textColor);
}

void IconLabelWindow::setTextUnderlined (bool enable) {
	label->setUnderlined (enable);
	refreshLayout ();
}

void IconLabelWindow::setTextFont (int fontType) {
	label->setFont (fontType);
	refreshLayout ();
}

void IconLabelWindow::setTextChangeHighlight (bool enable, const Color &highlightColor) {
	isTextChangeHighlightEnabled = enable;
	if (isTextChangeHighlightEnabled) {
		highlightTextColor.assign (highlightColor);
	}
}

void IconLabelWindow::setRightAligned (bool enable) {
	if (enable == isRightAligned) {
		return;
	}
	isRightAligned = enable;
	refreshLayout ();
}

void IconLabelWindow::setIconSprite (Sprite *iconSprite) {
	if (image) {
		image->isDestroyed = true;
	}
	image = (Image *) addWidget (new Image (iconSprite));
	refreshLayout ();
}

void IconLabelWindow::setIconImageColor (const Color &imageColor) {
	image->setDrawColor (true, imageColor);
}

void IconLabelWindow::setIconImageFrame (int frame) {
	image->setFrame (frame);
	refreshLayout ();
}

void IconLabelWindow::setIconImageScale (float scale) {
	image->setScale (scale);
	refreshLayout ();
}

void IconLabelWindow::setProgressBar (bool enable, float progressValue, float targetProgressValue) {
	if (! enable) {
		if (progressBar) {
			progressBar->isDestroyed = true;
			progressBar = NULL;
		}
	}
	else {
		if (! progressBar) {
			progressBar = (ProgressBar *) addWidget (new ProgressBar ());
			progressBar->zLevel = 1;
		}
		if ((progressValue < 0.0f) && (targetProgressValue < 0.0f)) {
			progressBar->setIndeterminate (true);
		}
		else {
			progressBar->setIndeterminate (false);
			if (targetProgressValue >= 0.0f) {
				progressBar->setProgress (progressValue, targetProgressValue);
			}
			else {
				progressBar->setProgress (progressValue);
			}
		}
	}

	refreshLayout ();
}

void IconLabelWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x2, y2;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x2 = 0.0f;
	y2 = 0.0f;

	if (isRightAligned) {
		label->flowRight (&x, y, &x2, &y2);
		image->flowRight (&x, y, &x2, &y2);
	}
	else {
		image->flowRight (&x, y, &x2, &y2);
		label->flowRight (&x, y, &x2, &y2);
	}

	image->position.assignY (y + ((y2 - y) / 2.0f) - (image->height / 2.0f));
	label->position.assignY (y + ((y2 - y) / 2.0f) - (label->height / 2.0f));
	label->isVisible = true;

	x2 += widthPadding;
	if (progressBar) {
		progressBar->setSize (x2, uiconfig->progressBarHeight);
		progressBar->position.assign (0.0f, y2);
		y2 += progressBar->height;
	}
	else {
		y2 += heightPadding;
	}
	setFixedSize (true, x2, y2);
}
