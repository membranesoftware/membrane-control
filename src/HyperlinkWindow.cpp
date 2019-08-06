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
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "OsUtil.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "UiConfiguration.h"
#include "HyperlinkWindow.h"

HyperlinkWindow::HyperlinkWindow (const StdString &linkText, const StdString &linkUrl, int fontType)
: Panel ()
, maxLineHeight (0.0f)
, label (NULL)
, url (linkUrl)
, labelWidth (0.0f)
, labelHeight (0.0f)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	label = (Label *) addWidget (new Label (linkText, fontType, uiconfig->linkTextColor));
	label->isInputSuspended = true;
	label->setUnderlined (true);
	maxLineHeight = label->maxLineHeight + uiconfig->textUnderlineMargin;
	labelWidth = label->width;
	labelHeight = label->height;
	setMouseClickCallback (HyperlinkWindow::windowClicked, this);
	setMouseEnterCallback (HyperlinkWindow::windowMouseEntered, this);
	setMouseExitCallback (HyperlinkWindow::windowMouseExited, this);
	setMouseHoverTooltip (uitext->getText (UiTextString::hyperlinkTooltip));

	refreshLayout ();
}

HyperlinkWindow::~HyperlinkWindow () {

}

StdString HyperlinkWindow::toStringDetail () {
	return (StdString (" HyperlinkWindow"));
}

void HyperlinkWindow::setLink (const StdString &linkText, const StdString &linkUrl) {
	label->setText (linkText);
	url.assign (linkUrl);
	labelWidth = label->width;
	labelHeight = label->height;
	refreshLayout ();
}

void HyperlinkWindow::setPadding (float widthPadding, float heightPadding) {
	Panel::setPadding (widthPadding, heightPadding);
	refreshLayout ();
}

float HyperlinkWindow::getLinePosition (float targetY) {
	return (label->getLinePosition (targetY));
}

void HyperlinkWindow::refreshLayout () {
	float x, y;

	x = widthPadding;
	y = heightPadding;
	label->position.assign (x, y);

	setFixedSize (true, labelWidth + (widthPadding * 2.0f), labelHeight + (heightPadding * 2.0f));
}

void HyperlinkWindow::windowClicked (void *windowPtr, Widget *widgetPtr) {
	HyperlinkWindow *window;
	int result;

	window = (HyperlinkWindow *) windowPtr;
	if (window->url.empty ()) {
		return;
	}

	result = OsUtil::openUrl (window->url);
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openHelpUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), window->url.c_str ()));
	}
}

void HyperlinkWindow::windowMouseEntered (void *windowPtr, Widget *widgetPtr) {
	HyperlinkWindow *window;
	UiConfiguration *uiconfig;

	window = (HyperlinkWindow *) windowPtr;
	uiconfig = &(App::instance->uiConfig);
	window->label->setUnderlined (false);
	window->label->textColor.translate (uiconfig->lightPrimaryTextColor, uiconfig->shortColorTranslateDuration);
}

void HyperlinkWindow::windowMouseExited (void *windowPtr, Widget *widgetPtr) {
	HyperlinkWindow *window;
	UiConfiguration *uiconfig;

	window = (HyperlinkWindow *) windowPtr;
	uiconfig = &(App::instance->uiConfig);
	window->label->setUnderlined (true);
	window->label->textColor.translate (uiconfig->linkTextColor, uiconfig->shortColorTranslateDuration);
}
