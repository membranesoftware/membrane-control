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
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Image.h"
#include "ImageWindow.h"
#include "UiConfiguration.h"
#include "HelpActionWindow.h"
#include "HyperlinkWindow.h"
#include "HelpWindow.h"

HelpWindow::HelpWindow (float windowWidth, float windowHeight)
: Panel ()
, headerImage (NULL)
, isHeaderImageLoaded (false)
, titleLabel (NULL)
, closeButton (NULL)
, helpTitleLabel (NULL)
, helpText (NULL)
, linkTitleLabel (NULL)
, linkIconImage (NULL)
, versionLabel (NULL)
{
	UiText *uitext;
	UiConfiguration *uiconfig;

	uitext = &(App::instance->uiText);
	uiconfig = &(App::instance->uiConfig);

	setFixedSize (true, windowWidth, windowHeight);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	headerImage = (ImageWindow *) addWidget (new ImageWindow ());
	headerImage->setLoadResourcePath (StdString::createSprintf ("bg/help/%i.png", App::instance->imageScale));

	titleLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::help).capitalized (), UiConfiguration::TitleFont, uiconfig->primaryTextColor)));
	titleLabel->setPadding (0.0f, 0.0f);

	closeButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::ExitButtonSprite)));
	closeButton->zLevel = 1;
	closeButton->setMouseClickCallback (HelpWindow::closeButtonClicked, this);
	closeButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);

	helpTitleLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::TitleFont, uiconfig->primaryTextColor));
	helpText = (TextArea *) addWidget (new TextArea (UiConfiguration::CaptionFont, uiconfig->primaryTextColor, uiconfig->textAreaLongLineLength, windowWidth - (uiconfig->paddingSize * 2.0f)));
	linkTitleLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::moreHelpTopics).capitalized (), UiConfiguration::TitleFont, uiconfig->primaryTextColor));
	linkIconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite)));

	versionLabel = (Label *) addWidget (new Label (StdString::createSprintf ("%s %s", BUILD_ID, BUILD_DATE), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));

	refreshLayout ();
}

HelpWindow::~HelpWindow () {

}

StdString HelpWindow::toStringDetail () {
	return (StdString (" HelpWindow"));
}

void HelpWindow::setWindowSize (float windowWidth, float windowHeight) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	helpText->setMaxLineWidth (windowWidth - (uiconfig->paddingSize * 2.0f));
	setFixedSize (true, windowWidth, windowHeight);
	refreshLayout ();
}

void HelpWindow::setHelpText (const StdString &title, const StdString &text) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	helpTitleLabel->setText (title);
	helpText->setMaxLineWidth (width - (uiconfig->paddingSize * 2.0f));
	helpText->setText (text);
	refreshLayout ();
}

void HelpWindow::addAction (const StdString &actionText, const StdString &linkText, const StdString &linkUrl) {
	UiConfiguration *uiconfig;
	HelpActionWindow *action;

	uiconfig = &(App::instance->uiConfig);
	action = new HelpActionWindow (width - (uiconfig->paddingSize * 2.0f), actionText, linkText, linkUrl);

	addWidget (action);
	actionList.push_back (action);
	refreshLayout ();
}

void HelpWindow::addTopicLink (const StdString &linkText, const StdString &linkUrl) {
	HyperlinkWindow *linkwindow;

	linkwindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow (linkText, linkUrl));
	linkList.push_back (linkwindow);
	refreshLayout ();
}

void HelpWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (! isHeaderImageLoaded) {
		if (headerImage->isLoaded ()) {
			isHeaderImageLoaded = true;
			refreshLayout ();
		}
	}
}

void HelpWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	std::list<HelpActionWindow *>::iterator hi, hend;
	std::list<HyperlinkWindow *>::iterator li, lend;
	HelpActionWindow *action;
	HyperlinkWindow *linkwindow;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	x = 0.0f;
	y = 0.0f;

	if (! headerImage->isLoaded ()) {
		x += uiconfig->paddingSize;
		y += uiconfig->paddingSize;
		headerImage->isVisible = false;

		titleLabel->setFillBg (false);
		titleLabel->setTextColor (uiconfig->primaryTextColor);
		titleLabel->position.assign (x, y);

		closeButton->position.assign (width - closeButton->width - uiconfig->paddingSize, y);
		y += closeButton->height + uiconfig->marginSize;
	}
	else {
		headerImage->position.assign (x, y);
		headerImage->isVisible = true;

		x += uiconfig->paddingSize;
		titleLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
		titleLabel->setTextColor (uiconfig->inverseTextColor);
		titleLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
		titleLabel->position.assign (x, y + headerImage->height - titleLabel->height - uiconfig->paddingSize);
		closeButton->position.assign (width - closeButton->width - uiconfig->paddingSize, y + headerImage->height - closeButton->height - uiconfig->paddingSize);
		y += headerImage->height + uiconfig->marginSize;
	}

	x = uiconfig->paddingSize;
	helpTitleLabel->position.assign (x, helpTitleLabel->getLinePosition (y));
	y += helpTitleLabel->maxLineHeight + uiconfig->marginSize;
	helpText->position.assign (x, y);
	y += helpText->height + uiconfig->marginSize;

	hi = actionList.begin ();
	hend = actionList.end ();
	while (hi != hend) {
		action = *hi;
		action->position.assign (x, y);
		y += action->height + uiconfig->marginSize;
		++hi;
	}

	if (linkList.empty ()) {
		linkTitleLabel->isVisible = false;
		linkIconImage->isVisible = false;
	}
	else {
		y += (uiconfig->marginSize * 2.0f);
		linkTitleLabel->position.assign (x, y);
		linkTitleLabel->isVisible = true;
		y += linkTitleLabel->height + uiconfig->marginSize;
		linkIconImage->position.assign (x, y);
		linkIconImage->isVisible = true;

		x += linkIconImage->width + uiconfig->marginSize;
		li = linkList.begin ();
		lend = linkList.end ();
		while (li != lend) {
			linkwindow = *li;
			linkwindow->position.assign (x, linkwindow->getLinePosition (y));
			y += linkwindow->maxLineHeight + (uiconfig->textLineHeightMargin * 2.0f);
			++li;
		}
	}

	versionLabel->position.assign (width - uiconfig->paddingSize - versionLabel->width, height - uiconfig->paddingSize - versionLabel->height);
}

void HelpWindow::closeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	HelpWindow *window;

	window = (HelpWindow *) windowPtr;
	window->isDestroyed = true;
}
