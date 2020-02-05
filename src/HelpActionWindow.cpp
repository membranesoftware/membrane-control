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
#include <math.h>
#include "Result.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "LabelWindow.h"
#include "TextArea.h"
#include "Image.h"
#include "UiConfiguration.h"
#include "HyperlinkWindow.h"
#include "HelpActionWindow.h"

HelpActionWindow::HelpActionWindow (float windowWidth, const StdString &helpActionText, const StdString &helpLinkText, const StdString &helpLinkUrl)
: Panel ()
, windowWidth (windowWidth)
, iconImage (NULL)
, actionText (NULL)
, linkWindow (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::InfoIconSprite)));

	actionText = (TextArea *) addWidget (new TextArea (UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor, uiconfig->textAreaMediumLineLength, windowWidth - iconImage->width - (uiconfig->paddingSize * 2.0f)));
	actionText->setText (helpActionText);

	if ((! helpLinkText.empty ()) && (! helpLinkUrl.empty ())) {
		linkWindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow (helpLinkText, helpLinkUrl));
	}

	refreshLayout ();
}

HelpActionWindow::~HelpActionWindow () {

}

StdString HelpActionWindow::toStringDetail () {
	return (StdString (" HelpActionWindow"));
}

void HelpActionWindow::setWindowWidth (float windowWidthValue) {
	UiConfiguration *uiconfig;

	if (FLOAT_EQUALS (windowWidthValue, windowWidth)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	windowWidth = windowWidthValue;
	actionText->setMaxLineWidth (windowWidth - iconImage->width - (uiconfig->paddingSize * 2.0f));
	refreshLayout ();
}

void HelpActionWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, h;

	uiconfig = &(App::instance->uiConfig);
	x = 0.0f;
	y = 0.0f;
	iconImage->position.assign (x, y);
	x += iconImage->width + uiconfig->marginSize;

	actionText->position.assign (x, y);
	y += actionText->height + uiconfig->marginSize;

	if (linkWindow) {
		linkWindow->position.assign (windowWidth - uiconfig->paddingSize - linkWindow->width, y);
	}

	resetSize ();
	h = maxWidgetY + uiconfig->paddingSize;
	setFixedSize (true, windowWidth, h);
}
