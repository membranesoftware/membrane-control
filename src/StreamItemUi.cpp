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
#include "HelpWindow.h"
#include "StreamDetailWindow.h"
#include "StreamWindow.h"
#include "ThumbnailWindow.h"
#include "StreamItemUi.h"

StreamItemUi::StreamItemUi (StreamWindow *streamWindow, const StdString &captionText)
: Ui ()
, captionText (captionText)
, cardView (NULL)
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
, lastTimelineHoverPosition (-1)
, thumbnailClickCallback (NULL)
, thumbnailClickCallbackData (NULL)
{
	sourceStreamWindow.assign (streamWindow);
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
	if (! sourceStreamWindow.widget) {
		return (StdString (""));
	}

	return (((StreamWindow *) sourceStreamWindow.widget)->streamName);
}

void StreamItemUi::setHelpWindowContent (Widget *helpWindowPtr) {
	App *app;
	HelpWindow *help;
	UiText *uitext;

	help = (HelpWindow *) helpWindowPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);

	help->setHelpText (uitext->getText (UiTextString::streamItemUiHelpTitle), uitext->getText (UiTextString::streamItemUiHelpText));
	help->addAction (uitext->getText (UiTextString::streamItemUiHelpAction1Text));

	help->addTopicLink (uitext->getText (UiTextString::mediaStreaming).capitalized (), Util::getHelpUrl ("media-streaming"));
	help->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), Util::getHelpUrl (""));
}

int StreamItemUi::doLoad () {
	App *app;
	AgentControl *agentcontrol;
	UiConfiguration *uiconfig;
	float t;
	int i, segmentcount, layout;
	Json *params;
	StreamDetailWindow *detailwindow;
	StreamWindow *streamwindow;
	ThumbnailWindow *window;
	StdString cardid, url;

	app = App::getInstance ();
	agentcontrol = &(app->agentControl);
	uiconfig = &(app->uiConfig);
	streamwindow = (StreamWindow *) sourceStreamWindow.widget;

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setItemMarginSize (uiconfig->marginSize / 2.0f);
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

	detailwindow = new StreamDetailWindow (streamwindow->streamId);
	cardView->addItem (detailwindow, streamwindow->streamId, 0);

	segmentcount = streamwindow->segmentCount;
	if ((segmentcount > 0) && (! streamwindow->thumbnailPath.empty ())) {
		for (i = 0; i < segmentcount; ++i) {
			params = new Json ();
			params->set ("id", streamwindow->streamId);
			params->set ("thumbnailIndex", i);
			url = agentcontrol->getAgentSecondaryUrl (streamwindow->agentId, app->createCommand ("GetThumbnailImage", SystemInterface::Constant_Stream, params), streamwindow->thumbnailPath);

			cardid.sprintf ("%i", i);
			if (i < (int) streamwindow->segmentPositions.size ()) {
				t = ((float) streamwindow->segmentPositions.at (i)) * 1000.0f;
			}
			else {
				t = i * (streamwindow->duration / (float) segmentcount);
			}
			window = new ThumbnailWindow (i, t, streamwindow->frameWidth, streamwindow->frameHeight, url, cardLayout, cardMaxImageWidth);
			window->setMouseClickCallback (StreamItemUi::thumbnailClicked, this);
			window->setMouseEnterCallback (StreamItemUi::thumbnailMouseEntered, this);
			window->setMouseExitCallback (StreamItemUi::thumbnailMouseExited, this);
			cardView->addItem (window, cardid, 1, true);
		}
	}
	cardView->refresh ();
	lastTimelineHoverPosition = -1;

	return (Result::SUCCESS);
}

void StreamItemUi::doUnload () {
	thumbnailSizeMenu.clear ();
	actionWindow.clear ();
}

void StreamItemUi::doResetMainToolbar (Toolbar *toolbar) {
	App *app;
	UiText *uitext;
	Button *button;

	app = App::getInstance ();
	uitext = &(app->uiText);

	button = new Button (StdString (""), sprites.getSprite (StreamItemUi::THUMBNAIL_SIZE_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (StreamItemUi::thumbnailSizeButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void StreamItemUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	UiConfiguration *uiconfig;
	Label *label;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);

	toolbar->setLeftCorner (new Image (sprites.getSprite (StreamItemUi::TIME_ICON)));

	timelineBar = new TimelineBar (app->windowWidth * 0.5f, ((StreamWindow *) sourceStreamWindow.widget)->streamId);
	timelineBar->setPositionHoverCallback (StreamItemUi::timelineBarPositionHovered, this);
	timelineBar->setPositionClickCallback (StreamItemUi::timelineBarPositionClicked, this);
	toolbar->addLeftItem (timelineBar);

	if (! captionText.empty ()) {
		label = new Label (captionText, UiConfiguration::CAPTION, uiconfig->mediumSecondaryColor);
		toolbar->addRightItem (label);
	}

	toolbar->isVisible = true;
}

void StreamItemUi::doClearPopupWidgets () {
	thumbnailSizeMenu.destroyAndClear ();
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
	app->setNextBackgroundTexturePath ("ui/StreamItemUi/bg");

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void StreamItemUi::doPause () {
	App *app;

	app = App::getInstance ();
	app->prefsMap.insert (App::prefsStreamItemImageSize, cardLayout);
}

void StreamItemUi::doUpdate (int msElapsed) {
	thumbnailSizeMenu.compact ();
	actionWindow.compact ();
}

void StreamItemUi::doSyncRecordStore (RecordStore *store) {
	timelineBar->syncRecordStore (store);
	cardView->syncRecordStore (store);
}

void StreamItemUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void StreamItemUi::thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;

	ui = (StreamItemUi *) uiPtr;
	ui->handleThumbnailSizeButtonClick (widgetPtr);
}

void StreamItemUi::handleThumbnailSizeButtonClick (Widget *buttonWidget) {
	App *app;
	UiText *uitext;
	Menu *menu;

	app = App::getInstance ();
	uitext = &(app->uiText);

	if (thumbnailSizeMenu.widget) {
		thumbnailSizeMenu.destroyAndClear ();
		return;
	}

	clearPopupWidgets ();
	menu = (Menu *) app->rootPanel->addWidget (new Menu ());
	menu->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), sprites.getSprite (StreamItemUi::SMALL_THUMBNAILS_ICON), StreamItemUi::viewSmallActionClicked, this, 0, cardLayout == ThumbnailWindow::LOW_DETAIL);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), sprites.getSprite (StreamItemUi::MEDIUM_THUMBNAILS_ICON), StreamItemUi::viewMediumActionClicked, this, 0, cardLayout == ThumbnailWindow::MEDIUM_DETAIL);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), sprites.getSprite (StreamItemUi::LARGE_THUMBNAILS_ICON), StreamItemUi::viewLargeActionClicked, this, 0, cardLayout == ThumbnailWindow::HIGH_DETAIL);
	menu->position.assign (buttonWidget->position.x + buttonWidget->width - menu->width, buttonWidget->position.y + buttonWidget->height);
	thumbnailSizeMenu.assign (menu);
}

void StreamItemUi::viewSmallActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (StreamItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::LOW_DETAIL) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::LOW_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->smallThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (StreamItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.1f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void StreamItemUi::viewMediumActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (StreamItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::MEDIUM_DETAIL) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::MEDIUM_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->mediumThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (StreamItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.2f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void StreamItemUi::viewLargeActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (StreamItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::HIGH_DETAIL) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::HIGH_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->largeThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (StreamItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.3f);
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

void StreamItemUi::thumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ThumbnailWindow *target;

	ui = (StreamItemUi *) uiPtr;
	target = ThumbnailWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	if (ui->thumbnailClickCallback) {
		ui->thumbnailClickCallback (ui->thumbnailClickCallbackData, target);
	}
}

void StreamItemUi::thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ThumbnailWindow *window;

	ui = (StreamItemUi *) uiPtr;
	window = (ThumbnailWindow *) widgetPtr;
	ui->timelineBar->setHighlightedMarker (window->thumbnailIndex);
	window->setHighlighted (true);
}

void StreamItemUi::thumbnailMouseExited (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ThumbnailWindow *window;

	ui = (StreamItemUi *) uiPtr;
	window = (ThumbnailWindow *) widgetPtr;
	if (ui->timelineBar->highlightedMarkerIndex == window->thumbnailIndex) {
		ui->timelineBar->setHighlightedMarker (-1);
	}
	window->setHighlighted (false);
}

void StreamItemUi::timelineBarPositionHovered (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	TimelineBar *bar;
	StreamWindow *streamwindow;
	ThumbnailWindow *target;
	float pos;
	int index;

	ui = (StreamItemUi *) uiPtr;
	bar = (TimelineBar *) widgetPtr;
	streamwindow = (StreamWindow *) ui->sourceStreamWindow.widget;

	pos = bar->hoverPosition;
	if (pos < 0.0f) {
		index = -1;
	}
	else {
		index = (int) (pos / bar->width * (float) streamwindow->segmentCount);
	}

	if (ui->lastTimelineHoverPosition != index) {
		bar->setHighlightedMarker (index);

		if (ui->lastTimelineHoverPosition >= 0) {
			target = (ThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &(ui->lastTimelineHoverPosition));
			if (target) {
				target->setHighlighted (false);
			}
		}

		ui->lastTimelineHoverPosition = index;
		if (index >= 0) {
			target = (ThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &index);
			if (target) {
				target->setHighlighted (true);
			}
		}
	}
}

void StreamItemUi::timelineBarPositionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	TimelineBar *bar;
	StreamWindow *streamwindow;
	ThumbnailWindow *target;
	int index;

	ui = (StreamItemUi *) uiPtr;
	bar = (TimelineBar *) widgetPtr;
	streamwindow = (StreamWindow *) ui->sourceStreamWindow.widget;

	if (ui->thumbnailClickCallback) {
		index = (int) (bar->clickPosition / bar->width * (float) streamwindow->segmentCount);
		target = (ThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &index);
		if (target) {
			ui->thumbnailClickCallback (ui->thumbnailClickCallbackData, target);
		}
	}
}

bool StreamItemUi::matchThumbnailIndex (void *intPtr, Widget *widgetPtr) {
	ThumbnailWindow *item;
	int *index;

	item = ThumbnailWindow::castWidget (widgetPtr);
	if (! item) {
		return (false);
	}

	index = (int *) intPtr;
	return (*index == item->thumbnailIndex);
}
