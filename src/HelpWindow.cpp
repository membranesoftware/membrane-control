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
#include "App.h"
#include "StdString.h"
#include "UiText.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "TextFlow.h"
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
	setFixedSize (true, windowWidth, windowHeight);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	headerImage = (ImageWindow *) addWidget (new ImageWindow ());
	headerImage->setImageFilePath (StdString::createSprintf ("bg/help/%i.png", App::instance->imageScale));

	titleLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::Help).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor)));
	titleLabel->setPadding (0.0f, 0.0f);

	closeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExitButtonSprite)));
	closeButton->mouseClickCallback = Widget::EventCallbackContext (HelpWindow::closeButtonClicked, this);
	closeButton->zLevel = 1;
	closeButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);

	helpTitleLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	helpText = (TextFlow *) addWidget (new TextFlow (windowWidth - (UiConfiguration::instance->paddingSize * 2.0f), UiConfiguration::CaptionFont));
	helpText->setTextColor (UiConfiguration::instance->primaryTextColor);
	linkTitleLabel = (Label *) addWidget (new Label (UiText::instance->getText (UiTextString::MoreHelpTopics).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	linkIconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite)));

	versionLabel = (Label *) addWidget (new Label (StdString::createSprintf ("%s %s", BUILD_ID, BUILD_DATE), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));

	refreshLayout ();
}

HelpWindow::~HelpWindow () {

}

StdString HelpWindow::toStringDetail () {
	return (StdString (" HelpWindow"));
}

void HelpWindow::setWindowSize (float windowWidth, float windowHeight) {
	helpText->setViewWidth (windowWidth - (UiConfiguration::instance->paddingSize * 2.0f));
	setFixedSize (true, windowWidth, windowHeight);
	refreshLayout ();
}

void HelpWindow::setHelpText (const StdString &title, const StdString &text) {
	helpTitleLabel->setText (title);
	helpText->setViewWidth (width - (UiConfiguration::instance->paddingSize * 2.0f));
	helpText->setText (text);
	refreshLayout ();
}

void HelpWindow::addAction (const StdString &actionText, const StdString &linkText, const StdString &linkUrl) {
	HelpActionWindow *action;

	action = new HelpActionWindow (width - (UiConfiguration::instance->paddingSize * 2.0f), actionText, linkText, linkUrl);

	addWidget (action);
	actionList.push_back (action);
	refreshLayout ();
}

void HelpWindow::addTopicLink (const StdString &linkText, const StdString &linkUrl) {
	HyperlinkWindow *linkwindow;

	linkwindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow (linkText, linkUrl));
	linkwindow->linkOpenCallback = Widget::EventCallbackContext (App::hyperlinkOpened, this);
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
	std::list<HelpActionWindow *>::iterator hi, hend;
	std::list<HyperlinkWindow *>::iterator li, lend;
	HelpActionWindow *action;
	HyperlinkWindow *linkwindow;
	float x, y;

	x = 0.0f;
	y = 0.0f;

	if (! headerImage->isLoaded ()) {
		x += UiConfiguration::instance->paddingSize;
		y += UiConfiguration::instance->paddingSize;
		headerImage->isVisible = false;

		titleLabel->setFillBg (false);
		titleLabel->setTextColor (UiConfiguration::instance->primaryTextColor);
		titleLabel->position.assign (x, y);

		closeButton->position.assign (width - closeButton->width - UiConfiguration::instance->paddingSize, y);
		y += closeButton->height + UiConfiguration::instance->marginSize;
	}
	else {
		headerImage->position.assign (x, y);
		headerImage->isVisible = true;

		x += UiConfiguration::instance->paddingSize;
		titleLabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
		titleLabel->setTextColor (UiConfiguration::instance->inverseTextColor);
		titleLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
		titleLabel->position.assign (x, y + headerImage->height - titleLabel->height - UiConfiguration::instance->paddingSize);
		closeButton->position.assign (width - closeButton->width - UiConfiguration::instance->paddingSize, y + headerImage->height - closeButton->height - UiConfiguration::instance->paddingSize);
		y += headerImage->height + UiConfiguration::instance->marginSize;
	}

	x = UiConfiguration::instance->paddingSize;
	helpTitleLabel->position.assign (x, helpTitleLabel->getLinePosition (y));
	y += helpTitleLabel->maxLineHeight + UiConfiguration::instance->marginSize;
	helpText->position.assign (x, y);
	y += helpText->height + UiConfiguration::instance->marginSize;

	hi = actionList.begin ();
	hend = actionList.end ();
	while (hi != hend) {
		action = *hi;
		action->position.assign (x, y);
		y += action->height + UiConfiguration::instance->marginSize;
		++hi;
	}

	if (linkList.empty ()) {
		linkTitleLabel->isVisible = false;
		linkIconImage->isVisible = false;
	}
	else {
		y += (UiConfiguration::instance->marginSize * 2.0f);
		linkTitleLabel->position.assign (x, y);
		linkTitleLabel->isVisible = true;
		y += linkTitleLabel->height + UiConfiguration::instance->marginSize;
		linkIconImage->position.assign (x, y);
		linkIconImage->isVisible = true;

		x += linkIconImage->width + UiConfiguration::instance->marginSize;
		li = linkList.begin ();
		lend = linkList.end ();
		while (li != lend) {
			linkwindow = *li;
			linkwindow->position.assign (x, linkwindow->getLinePosition (y));
			y += linkwindow->maxLineHeight + (UiConfiguration::instance->textLineHeightMargin * 2.0f);
			++li;
		}
	}

	versionLabel->position.assign (width - UiConfiguration::instance->paddingSize - versionLabel->width, height - UiConfiguration::instance->paddingSize - versionLabel->height);
}

void HelpWindow::closeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	HelpWindow *window;

	window = (HelpWindow *) windowPtr;
	window->isDestroyed = true;
}
