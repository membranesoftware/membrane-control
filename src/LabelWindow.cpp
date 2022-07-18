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
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "LabelWindow.h"

LabelWindow::LabelWindow (Label *label)
: Panel ()
, isCrawlEnabled (false)
, label (label)
, isFixedWidth (false)
, isWidthCentered (false)
, windowWidth (0.0f)
, isMouseoverHighlightEnabled (false)
, mouseoverColorTranslateDuration (0)
, crawlStage (0)
, crawlClock (0)
{
	addWidget (label);
	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
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

void LabelWindow::setFitWidth () {
	isFixedWidth = false;
	isWidthCentered = false;
	refreshLayout ();
}

void LabelWindow::setTextColor (const Color &color) {
	label->textColor.assign (color);
}

void LabelWindow::translateTextColor (const Color &targetColor, int durationMs) {
	label->textColor.translate (targetColor, durationMs);
}

void LabelWindow::translateTextColor (const Color &startColor, const Color &targetColor, int durationMs) {
	label->textColor.translate (startColor, targetColor, durationMs);
}

StdString LabelWindow::getText () const {
	return (label->text);
}

void LabelWindow::setText (const StdString &text) {
	label->setText (text);
	refreshLayout ();
}

void LabelWindow::setFont (UiConfiguration::FontType fontType) {
	label->setFont (fontType);
	refreshLayout ();
}

void LabelWindow::setMouseoverHighlight (bool enable, const Color &normalTextColor, const Color &normalBgColor, const Color &highlightTextColor, const Color &highlightBgColor, int colorTranslateDuration) {
	isMouseoverHighlightEnabled = enable;
	if (isMouseoverHighlightEnabled) {
		mouseoverNormalTextColor = normalTextColor;
		mouseoverNormalBgColor = normalBgColor;
		mouseoverHighlightTextColor = highlightTextColor;
		mouseoverHighlightBgColor = highlightBgColor;
		mouseoverColorTranslateDuration = colorTranslateDuration;
		if (mouseoverColorTranslateDuration < 1) {
			mouseoverColorTranslateDuration = 1;
		}

		setFillBg (true, mouseoverNormalBgColor);
	}
}

void LabelWindow::setCrawl (bool enable) {
	if (isCrawlEnabled == enable) {
		return;
	}
	isCrawlEnabled = enable;
	label->position.assignX (widthPadding);
	if (isCrawlEnabled) {
		crawlStage = 0;
	}
}

bool LabelWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (isMouseoverHighlightEnabled) {
		if (mouseState.isEntered) {
			label->textColor.translate (mouseoverHighlightTextColor, mouseoverColorTranslateDuration);
			bgColor.translate (mouseoverHighlightBgColor, mouseoverColorTranslateDuration);
		}
		else {
			label->textColor.translate (mouseoverNormalTextColor, mouseoverColorTranslateDuration);
			bgColor.translate (mouseoverNormalBgColor, mouseoverColorTranslateDuration);
		}
	}

	return (false);
}

void LabelWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (isCrawlEnabled) {
		switch (crawlStage) {
			case 0: {
				crawlClock = 7000;
				crawlStage = 1;
				break;
			}
			case 1: {
				crawlClock -= msElapsed;
				if (crawlClock <= 0) {
					crawlStage = 2;
					label->position.translateX (-(label->width), 100 * (int) label->text.length ());
				}
				break;
			}
			case 2: {
				if (! label->position.isTranslating) {
					crawlStage = 0;
					label->position.assignX (widthPadding);
				}
				break;
			}
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
