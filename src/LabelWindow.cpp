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
#include "Config.h"
#include <stdlib.h>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "LabelWindow.h"

LabelWindow::LabelWindow (Label *label)
: Panel ()
, label (label)
, isFixedWidth (false)
, isWidthCentered (false)
, windowWidth (0.0f)
, isMouseoverHighlightEnabled (false)
, mouseoverColorRotateDuration (0)
{
	UiConfiguration *uiconfig;

	addWidget (label);
	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
}

LabelWindow::~LabelWindow () {

}

StdString LabelWindow::toStringDetail () {
	return (StdString (" LabelWindow"));
}

void LabelWindow::setPadding (float widthPadding, float heightPadding) {
	Panel::setPadding (widthPadding, heightPadding);
	refreshLayout ();
}

void LabelWindow::setWindowWidth (float fixedWidth, bool isLabelCentered) {
	isFixedWidth = true;
	isWidthCentered = isLabelCentered;
	windowWidth = fixedWidth;
	refreshLayout ();
}

void LabelWindow::setTextColor (const Color &color) {
	label->textColor.assign (color);
}

void LabelWindow::rotateTextColor (const Color &color, int duration) {
	label->textColor.rotate (color, duration);
}

StdString LabelWindow::getText () const {
	return (label->text);
}

void LabelWindow::setText (const StdString &text) {
	label->setText (text);
	refreshLayout ();
}

void LabelWindow::setFont (int fontType) {
	label->setFont (fontType);
	refreshLayout ();
}

void LabelWindow::setMouseoverHighlight (bool enable, const Color &normalTextColor, const Color &normalBgColor, const Color &highlightTextColor, const Color &highlightBgColor, int colorRotateDuration) {
	isMouseoverHighlightEnabled = enable;
	if (isMouseoverHighlightEnabled) {
		mouseoverNormalTextColor = normalTextColor;
		mouseoverNormalBgColor = normalBgColor;
		mouseoverHighlightTextColor = highlightTextColor;
		mouseoverHighlightBgColor = highlightBgColor;
		mouseoverColorRotateDuration = colorRotateDuration;
		if (mouseoverColorRotateDuration < 1) {
			mouseoverColorRotateDuration = 1;
		}

		setFillBg (true, mouseoverNormalBgColor);
	}
}

void LabelWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (isMouseoverHighlightEnabled) {
		if (mouseState.isEntered) {
			label->textColor.rotate (mouseoverHighlightTextColor, mouseoverColorRotateDuration);
			bgColor.rotate (mouseoverHighlightBgColor, mouseoverColorRotateDuration);
		}
		else {
			label->textColor.rotate (mouseoverNormalTextColor, mouseoverColorRotateDuration);
			bgColor.rotate (mouseoverNormalBgColor, mouseoverColorRotateDuration);
		}
	}
}

void LabelWindow::refreshLayout () {
	float x, w, h;

	x = widthPadding;
	h = label->maxLineHeight;
	h += (heightPadding * 2.0f);

	label->position.assign (x, (h / 2.0f) - (label->height / 2.0f));

	if (isFixedWidth) {
		w = windowWidth;
		if (isWidthCentered) {
			label->position.assignX ((w / 2.0f) - (label->width / 2.0f));
		}
	}
	else {
		w = label->width;
		w += (widthPadding * 2.0f);
	}

	setFixedSize (true, w, h);
}
