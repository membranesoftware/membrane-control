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
#include "Resource.h"
#include "SpriteGroup.h"
#include "Util.h"
#include "AgentControl.h"
#include "Button.h"
#include "SystemInterface.h"
#include "HashMap.h"
#include "Ui.h"
#include "UiText.h"
#include "Toolbar.h"
#include "Menu.h"
#include "CardView.h"
#include "IconCardWindow.h"
#include "StreamWindow.h"
#include "ThumbnailWindow.h"
#include "StreamItemUi.h"

const float StreamItemUi::smallImageScale = 0.123f;
const float StreamItemUi::mediumImageScale = 0.240f;
const float StreamItemUi::largeImageScale = 0.480f;

StreamItemUi::StreamItemUi (StreamWindow *card, const StdString &captionText)
: Ui ()
, captionText (captionText)
, segmentCount (0)
, frameWidth (0)
, frameHeight (0)
, streamBitrate (0)
, streamDuration (0)
, streamFrameRate (0.0f)
, cardView (NULL)
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
, detailCard (NULL)
, thumbnailClickCallback (NULL)
, thumbnailClickCallbackData (NULL)
{
	displayAgentMap.sort (HashMap::sortAscending);
	streamId.assign (card->streamId);
	streamName.assign (card->streamName);
	thumbnailUrl.assign (card->thumbnailUrl);
	segmentCount = card->segmentCount;
	frameWidth = card->frameWidth;
	frameHeight = card->frameHeight;
	streamBitrate = card->bitrate;
	streamDuration = card->duration;
	streamFrameRate = card->frameRate;
}

StreamItemUi::~StreamItemUi () {

}

void StreamItemUi::setThumbnailClickCallback (Widget::EventCallback callback, void *callbackData) {
	thumbnailClickCallback = callback;
	thumbnailClickCallbackData = callbackData;
}

StdString StreamItemUi::getSpritePath () {
	return (StdString ("ui/StreamItemUi/sprite"));
}

StdString StreamItemUi::getBreadcrumbTitle () {
	return (streamName);
}

Sprite *StreamItemUi::getBreadcrumbSprite () {
	// TODO: Implement this
	return (NULL);
}

int StreamItemUi::doLoad () {
	App *app;
	SystemInterface *interface;
	UiText *uitext;
	UiConfiguration *uiconfig;
	float t;
	int i, layout;
	Json *params;
	StdString cardid, url, cmdjson, text, rationame, framesizename;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	uitext = &(app->uiText);
	uiconfig = &(app->uiConfig);

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setItemMarginSize (uiconfig->marginSize / 2.0f);
	cardView->setItemClickCallback (StreamItemUi::cardClicked, this);
	cardView->position.assign (0.0f, app->topBarHeight);

	layout = app->prefsMap.find (App::prefsStreamItemImageSize, (int) ThumbnailWindow::MEDIUM_DETAIL);
	switch (layout) {
		case ThumbnailWindow::LOW_DETAIL: {
			StreamItemUi::viewSmallActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::MEDIUM_DETAIL: {
			StreamItemUi::viewMediumActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::HIGH_DETAIL: {
			StreamItemUi::viewLargeActionClicked (this, NULL);
			break;
		}
		default: {
			StreamItemUi::viewMediumActionClicked (this, NULL);
			break;
		}
	}

	text.sprintf ("%s: %ix%i", uitext->frameSize.capitalized ().c_str (), frameWidth, frameHeight);
	rationame = Util::getAspectRatioDisplayString (frameWidth, frameHeight);
	framesizename = Util::getFrameSizeName (frameWidth, frameHeight);
	if ((! rationame.empty ()) || (! framesizename.empty ())) {
		text.append (" (");
		if (! rationame.empty ()) {
			text.append (rationame);
		}
		if (! framesizename.empty ()) {
			if (! rationame.empty ()) {
				text.append (", ");
			}
			text.append (framesizename);
		}
		text.append (")");
	}

	text.appendSprintf ("\n%s: %s (%s)", uitext->duration.capitalized ().c_str (), Util::getDurationDisplayString (streamDuration).c_str (), Util::getDurationString (streamDuration, Util::HOURS).c_str ());
	if (streamFrameRate <= 0.0f) {
		text.appendSprintf ("\n%s: %s", uitext->frameRate.capitalized ().c_str (), uitext->unknown.capitalized ().c_str ());
	}
	else {
		text.appendSprintf ("\n%s: %.2ffps", uitext->frameRate.capitalized ().c_str (), streamFrameRate);
	}

	if (streamBitrate <= 0.0f) {
		text.appendSprintf ("\n%s: %s", uitext->bitrate.capitalized ().c_str (), uitext->unknown.capitalized ().c_str ());
	}
	else {
		text.appendSprintf ("\n%s: %s", uitext->bitrate.capitalized ().c_str (), Util::getBitrateDisplayString (streamBitrate).c_str ());
	}

	detailCard = new IconCardWindow (sprites.getSprite (StreamItemUi::STREAM_ICON), streamName, StdString (""), text);
	cardView->addItem (detailCard, streamId, 0);

	if ((segmentCount > 0) && (! thumbnailUrl.empty ())) {
		t = 0.0f;
		for (i = 0; i < segmentCount; ++i) {
			params = new Json ();
			params->set ("id", streamId);
			params->set ("thumbnailIndex", i);
			cmdjson = app->createCommandJson ("GetThumbnailImage", SystemInterface::Constant_Media, params);
			if (cmdjson.empty ()) {
				continue;
			}

			url = interface->getInvokeUrl (thumbnailUrl, cmdjson);
			cardid.sprintf ("%i", i);
			cardView->addItem (new ThumbnailWindow (t, frameWidth, frameHeight, url, sprites.getSprite (StreamItemUi::LOADING_IMAGE_ICON), cardLayout, cardMaxImageWidth), cardid, 1, true);

			t += (streamDuration / (float) segmentCount);
		}
	}
	cardView->refresh ();

	return (Result::SUCCESS);
}

void StreamItemUi::doUnload () {
	viewMenu.clear ();
	actionWindow.clear ();
	displayAgentMap.clear ();
}

void StreamItemUi::doResetMainToolbar (Toolbar *toolbar) {
	Button *button;

	button = new Button (StdString (""), sprites.getSprite (StreamItemUi::VIEW_BUTTON));
	button->setMouseClickCallback (StreamItemUi::viewButtonClicked, this);
// TODO: Set tooltip text
//  button->tooltipText.assign (app->uiText.mediaUiViewTooltip);
//     button->isMouseHoverEnabled = true;
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void StreamItemUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	UiConfiguration *uiconfig;
	Label *label;
	TimelineBar *bar;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);

	toolbar->setLeftCorner (new Image (sprites.getSprite (StreamItemUi::TIME_ICON)));

	bar = new TimelineBar (app->windowWidth * 0.5f, 0.0f, streamDuration);
	timelineBar.destroyAndClear ();
	timelineBar.assign (bar);
	toolbar->addLeftItem (bar);

	if (! captionText.empty ()) {
		label = new Label (captionText, UiConfiguration::CAPTION, uiconfig->inverseTextColor);
		toolbar->addRightItem (label);
	}

	toolbar->isVisible = true;
}

void StreamItemUi::doClearPopupWidgets () {
	viewMenu.destroyAndClear ();
	actionWindow.destroyAndClear ();
}

void StreamItemUi::doRefresh () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void StreamItemUi::doResume () {
	App *app;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/StreamItem/bg");

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void StreamItemUi::doPause () {
	App *app;

	app = App::getInstance ();
	app->prefsMap.insert (App::prefsStreamItemImageSize, cardLayout);
}

void StreamItemUi::doUpdate (int msElapsed) {
	viewMenu.compact ();
}

void StreamItemUi::doSyncRecordStore (RecordStore *store) {
	store->populateAgentMap (&displayAgentMap, StdString ("displayServerStatus"));
}

void StreamItemUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void StreamItemUi::viewButtonClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;

	ui = (StreamItemUi *) uiPtr;
	ui->handleViewButtonClick (widgetPtr);
}

void StreamItemUi::handleViewButtonClick (Widget *buttonWidget) {
	App *app;
	UiText *uitext;
	Menu *menu;

	app = App::getInstance ();
	uitext = &(app->uiText);

	if (viewMenu.widget) {
		viewMenu.destroyAndClear ();
		return;
	}

	clearPopupWidgets ();
	menu = (Menu *) app->rootPanel->addWidget (new Menu ());
	menu->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->small.capitalized (), sprites.getSprite (StreamItemUi::SMALL_THUMBNAILS_ICON), StreamItemUi::viewSmallActionClicked, this, 0, cardLayout == ThumbnailWindow::LOW_DETAIL);
	menu->addItem (uitext->medium.capitalized (), sprites.getSprite (StreamItemUi::MEDIUM_THUMBNAILS_ICON), StreamItemUi::viewMediumActionClicked, this, 0, cardLayout == ThumbnailWindow::MEDIUM_DETAIL);
	menu->addItem (uitext->large.capitalized (), sprites.getSprite (StreamItemUi::LARGE_THUMBNAILS_ICON), StreamItemUi::viewLargeActionClicked, this, 0, cardLayout == ThumbnailWindow::HIGH_DETAIL);
	menu->position.assign (buttonWidget->position.x + buttonWidget->width - menu->width, buttonWidget->position.y + buttonWidget->height);
	viewMenu.assign (menu);
}

void StreamItemUi::viewSmallActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	float y0, h0;

	ui = (StreamItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::LOW_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::LOW_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * StreamItemUi::smallImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (StreamItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.12f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void StreamItemUi::viewMediumActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	float y0, h0;

	ui = (StreamItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::MEDIUM_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::MEDIUM_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * StreamItemUi::mediumImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (StreamItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.24f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void StreamItemUi::viewLargeActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	float y0, h0;

	ui = (StreamItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::HIGH_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::HIGH_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * StreamItemUi::largeImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (StreamItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.36f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void StreamItemUi::resetCardLayout (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ThumbnailWindow *card;

	ui = (StreamItemUi *) uiPtr;
	card = ThumbnailWindow::castWidget (widgetPtr);
	if (! card) {
		return;
	}

	card->setLayout (ui->cardLayout, ui->cardMaxImageWidth);
}

void StreamItemUi::cardClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ThumbnailWindow *card;

	ui = (StreamItemUi *) uiPtr;
	card = ThumbnailWindow::castWidget (widgetPtr);
	if (! card) {
		return;
	}

	if (ui->thumbnailClickCallback) {
		ui->thumbnailClickCallback (ui->thumbnailClickCallbackData, card);
	}
}
