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
#include "Menu.h"
#include "CardView.h"
#include "MediaWindow.h"
#include "ActionWindow.h"
#include "IconCardWindow.h"
#include "SystemInterface.h"
#include "Ui.h"
#include "MediaItemUi.h"
#include "MediaUi.h"

// TODO: Remove references to this macro (when Membrane Media Server is available and the media interface should be enabled)
#ifndef ENABLE_INTERFACE
#define ENABLE_INTERFACE 0
#endif

const int MediaUi::readEventsPeriod = 60 * 1000; // ms
const float MediaUi::smallImageScale = 0.123f;
const float MediaUi::mediumImageScale = 0.240f;
const float MediaUi::largeImageScale = 0.480f;

MediaUi::MediaUi ()
: Ui ()
, cardView (NULL)
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
, lastReadEventsTime (0)
{
	streamServerAgentMap.sort (HashMap::sortAscending);
}

MediaUi::~MediaUi () {

}

StdString MediaUi::getSpritePath () {
	return (StdString ("ui/MediaUi/sprite"));
}

StdString MediaUi::getBreadcrumbTitle () {
	return (App::getInstance ()->uiText.media.capitalized ());
}

Sprite *MediaUi::getBreadcrumbSprite () {
	return (sprites.getSprite (MediaUi::BREADCRUMB_ICON)->copy ());
}

int MediaUi::doLoad () {
	App *app;
	int layout;

	app = App::getInstance ();

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setItemMarginSize (0.0f);
	cardView->sort (MediaUi::sortMediaCards);
	cardView->setItemClickCallback (MediaUi::mediaCardClicked, this);
	cardView->position.assign (0.0f, app->topBarHeight);

	layout = app->prefsMap.find (App::prefsMediaImageSize, (int) MediaWindow::LOW_DETAIL);
	switch (layout) {
		case MediaWindow::LOW_DETAIL: {
			MediaUi::viewSmallActionClicked (this, NULL);
			break;
		}
		case MediaWindow::MEDIUM_DETAIL: {
			MediaUi::viewMediumActionClicked (this, NULL);
			break;
		}
		case MediaWindow::HIGH_DETAIL: {
			MediaUi::viewLargeActionClicked (this, NULL);
			break;
		}
		default: {
			MediaUi::viewSmallActionClicked (this, NULL);
			break;
		}
	}

#if (! ENABLE_INTERFACE)
	IconCardWindow *emptycard;
	UiText *uitext;

	uitext = &(app->uiText);
	emptycard = new IconCardWindow (sprites.getSprite (MediaUi::EMPTY_INTERFACE_ICON), uitext->emptyInterfaceTitle, StdString (""), uitext->mediaUiEmptyInterfaceText);
	emptycard->setLink (uitext->learnMore.capitalized (), Util::getFeatureUrl ("build-a-movie-wall"));
	cardView->addItem (emptycard, StdString ("EmptyCard"), 0);
#endif
	return (Result::SUCCESS);
}

void MediaUi::doUnload () {
	viewMenu.clear ();
	actionWindow.clear ();
	streamServerAgentMap.clear ();
}

void MediaUi::doResetMainToolbar (Toolbar *toolbar) {
	App *app;
	Button *button;

	app = App::getInstance ();
	button = new Button (StdString (""), sprites.getSprite (MediaUi::VIEW_BUTTON));
	button->setMouseClickCallback (MediaUi::viewButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.mediaUiViewTooltip);
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void MediaUi::doClearPopupWidgets () {
	viewMenu.destroyAndClear ();
	actionWindow.destroyAndClear ();
}

void MediaUi::doResume () {
	App *app;
	Json *params;
	int64_t now;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/MediaUi/bg");

	now = Util::getTime ();
	if ((now - lastReadEventsTime) >= MediaUi::readEventsPeriod) {
		params = new Json ();
		params->set ("commandId", SystemInterface::Command_MediaItem);
		app->agentControl.writeLinkCommand (app->createCommandJson ("ReadEvents", SystemInterface::Constant_Link, params));

		params = new Json ();
		params->set ("commandId", SystemInterface::Command_StreamItem);
		app->agentControl.writeLinkCommand (app->createCommandJson ("ReadEvents", SystemInterface::Constant_Link, params));

		lastReadEventsTime = now;
	}

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MediaUi::doPause () {
	App *app;

	app = App::getInstance ();
	app->prefsMap.insert (App::prefsMediaImageSize, cardLayout);
}

void MediaUi::doRefresh () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MediaUi::doUpdate (int msElapsed) {
	viewMenu.compact ();
	actionWindow.compact ();
}

void MediaUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MediaUi::doSyncRecordStore (RecordStore *store) {
#if ENABLE_INTERFACE
	std::list<Json *> records;
	std::list<Json *>::iterator i, end;
	SystemInterface *interface;
	StdString mediaid;
	MediaWindow *mediacard;
	int commandid;
	Json *record;
	bool shouldreset;

	interface = &(App::getInstance ()->systemInterface);
	commandid = SystemInterface::Command_MediaItem;
	shouldreset = false;
	store->findRecords (RecordStore::matchCommandId, &commandid, &records, true);
	i = records.begin ();
	end = records.end ();
	while (i != end) {
		record = *i;
		++i;

		mediaid = interface->getCommandStringParam (record, "id", "");
		if (mediaid.empty ()) {
			continue;
		}

		if (! cardView->contains (mediaid)) {
			shouldreset = true;
			mediacard = (MediaWindow *) cardView->addItem (new MediaWindow (record, sprites.getSprite (MediaUi::LOADING_IMAGE_ICON), cardLayout, cardMaxImageWidth), mediaid);
			mediacard->syncRecordStore (store);
			mediacard->setActionCallbacks (this, MediaUi::viewThumbnailsActionClicked, MediaUi::createStreamActionClicked, MediaUi::removeStreamActionClicked);
		}
	}
	if (shouldreset) {
		cardView->refresh ();
	}

	store->populateAgentMap (&streamServerAgentMap, StdString ("streamServerStatus"));
#endif
}

void MediaUi::viewButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->handleViewButtonClick (widgetPtr);
}

void MediaUi::handleViewButtonClick (Widget *buttonWidget) {
	App *app;
	UiText *uitext;
	Menu *menu;

	app = App::getInstance ();
	uitext = &(app->uiText);

	suspendMouseHover ();
	if (viewMenu.widget) {
		viewMenu.widget->isDestroyed = true;
		viewMenu.clear ();
		return;
	}

	clearPopupWidgets ();
	menu = (Menu *) app->rootPanel->addWidget (new Menu ());
	menu->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->small.capitalized (), sprites.getSprite (MediaUi::SMALL_THUMBNAILS_ICON), MediaUi::viewSmallActionClicked, this, 0, cardLayout == MediaWindow::LOW_DETAIL);
	menu->addItem (uitext->medium.capitalized (), sprites.getSprite (MediaUi::MEDIUM_THUMBNAILS_ICON), MediaUi::viewMediumActionClicked, this, 0, cardLayout == MediaWindow::MEDIUM_DETAIL);
	menu->addItem (uitext->large.capitalized (), sprites.getSprite (MediaUi::LARGE_THUMBNAILS_ICON), MediaUi::viewLargeActionClicked, this, 0, cardLayout == MediaWindow::HIGH_DETAIL);
	menu->position.assign (buttonWidget->position.x + buttonWidget->width - menu->width, buttonWidget->position.y + buttonWidget->height);
	viewMenu.assign (menu);
}

void MediaUi::viewSmallActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	float y0, h0;

	ui = (MediaUi *) uiPtr;
	if (ui->cardLayout == MediaWindow::LOW_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = MediaWindow::LOW_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * MediaUi::smallImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaUi::resetMediaCardLayout, ui);
	ui->cardView->setItemMarginSize (0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.12f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MediaUi::viewMediumActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	float y0, h0;

	ui = (MediaUi *) uiPtr;
	if (ui->cardLayout == MediaWindow::MEDIUM_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = MediaWindow::MEDIUM_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * MediaUi::mediumImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaUi::resetMediaCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.24f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MediaUi::viewLargeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	float y0, h0;

	ui = (MediaUi *) uiPtr;
	if (ui->cardLayout == MediaWindow::HIGH_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = MediaWindow::HIGH_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * MediaUi::largeImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaUi::resetMediaCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.36f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MediaUi::resetMediaCardLayout (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *window;

	ui = (MediaUi *) uiPtr;
	window = MediaWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	window->setLayout (ui->cardLayout, ui->cardMaxImageWidth);
}

bool MediaUi::sortMediaCards (Widget *a, Widget *b) {
	MediaWindow *wa, *wb;

	wa = MediaWindow::castWidget (a);
	wb = MediaWindow::castWidget (b);
	if (!(wa && wb)) {
		return (false);
	}

	if (wa->mediaName.lowercased ().compare (wb->mediaName.lowercased ()) < 0) {
		return (true);
	}

	return (false);
}

void MediaUi::mediaCardClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaItemUi *itemui;
	MediaWindow *card;

	ui = (MediaUi *) uiPtr;
	card = MediaWindow::castWidget (widgetPtr);
	if ((! card) || card->thumbnailUrl.empty () || (card->thumbnailCount <= 0)) {
		return;
	}

	if (ui->cardLayout == MediaWindow::HIGH_DETAIL) {
		return;
	}

	itemui = new MediaItemUi (card);
	App::getInstance ()->pushUi (itemui);
}

void MediaUi::viewThumbnailsActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *itemui;
	MediaWindow *card;

	card = MediaWindow::castWidget (widgetPtr);
	if ((! card) || card->thumbnailUrl.empty () || (card->thumbnailCount <= 0)) {
		return;
	}

	itemui = new MediaItemUi (card);
	App::getInstance ()->pushUi (itemui);
}

void MediaUi::createStreamActionClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	MediaUi *ui;
	MediaWindow *card;
	ActionWindow *action;
	UiConfiguration *uiconfig;
	UiText *uitext;

	ui = (MediaUi *) uiPtr;
	card = MediaWindow::castWidget (widgetPtr);
	if (! card) {
		return;
	}
	if (ui->streamServerAgentMap.empty ()) {
		return;
	}

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);
	ui->clearPopupWidgets ();
	action = (ActionWindow *) app->rootPanel->addWidget (new ActionWindow (StdString::createSprintf ("%s: %s", uitext->createStream.capitalized ().c_str (), card->mediaName.c_str ()), uitext->createStream.uppercased ()));
	ui->actionWindow.assign (action);
	action->sourceWidget.assign (card);
	action->addComboBoxOption (uitext->streamServer.capitalized (), &(ui->streamServerAgentMap));
	action->setCloseCallback (MediaUi::createStreamActionClosed, ui);
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->position.assignBounded (card->drawX + card->width - action->width, card->drawY + card->height - action->height, uiconfig->paddingSize, uiconfig->paddingSize, app->rootPanel->width - uiconfig->paddingSize - action->width, app->rootPanel->height - uiconfig->paddingSize - action->height);
}

void MediaUi::createStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (action->isConfirmed) {
		ui->invokeCreateMediaStream ();
	}
	ui->actionWindow.clear ();
}

void MediaUi::invokeCreateMediaStream () {
	App *app;
	UiText *uitext;
	ActionWindow *action;
	MediaWindow *card;
	StdString agentid;
	Json *cmd, *params;
	int64_t jobid;
	int result;

	app = App::getInstance ();
	uitext = &(app->uiText);
	action = (ActionWindow *) actionWindow.widget;
	if (! action) {
		return;
	}

	agentid = action->getOptionValue (uitext->streamServer, "");
	if (agentid.empty ()) {
		return;
	}

	card = MediaWindow::castWidget (action->sourceWidget.widget);
	if (! card) {
		return;
	}

	params = new Json ();
	params->set ("name", card->mediaName);
	params->set ("mediaId", card->mediaId);
	params->set ("mediaUrl", card->mediaUrl);
	cmd = app->createCommand ("CreateMediaStream", SystemInterface::Constant_Stream, params);
	if (! cmd) {
		return;
	}

	result = app->agentControl.invokeCommand (agentid, cmd, MediaUi::createMediaStreamComplete, NULL, &jobid);
	if (result != Result::SUCCESS) {
		// TODO: Show an error message here
		Log::write (Log::ERR, "Failed to execute CreateMediaStream command; err=%i agentId=\"%s\"", result, agentid.c_str ());
		return;
	}
}

void MediaUi::createMediaStreamComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	App *app;
	Json *params;
	StdString recordid;


	app = App::getInstance ();
	if (responseCommand) {
		recordid = app->systemInterface.getCommandStringParam (responseCommand, "taskId", "");
		if (! recordid.empty ()) {
			params = new Json ();
			params->set ("recordId", recordid);
			// TODO: Possibly restrict this command to the stream server's link agent
			app->agentControl.writeLinkCommand (app->createCommandJson ("WatchEvents", SystemInterface::Constant_Link, params));
		}
	}
}

void MediaUi::removeStreamActionClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	MediaUi *ui;
	MediaWindow *card;
	ActionWindow *action;
	UiConfiguration *uiconfig;
	UiText *uitext;

	ui = (MediaUi *) uiPtr;
	card = MediaWindow::castWidget (widgetPtr);
	if (! card) {
		return;
	}
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);
	ui->clearPopupWidgets ();
	action = (ActionWindow *) app->rootPanel->addWidget (new ActionWindow (StdString::createSprintf ("%s: %s", uitext->removeStream.capitalized ().c_str (), card->mediaName.c_str ()), uitext->removeStream.uppercased ()));
	ui->actionWindow.assign (action);
	action->sourceWidget.assign (card);
	action->setCloseCallback (MediaUi::removeStreamActionClosed, ui);
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->position.assignBounded (card->drawX + card->width - action->width, card->drawY + card->height - action->height, uiconfig->paddingSize, uiconfig->paddingSize, app->rootPanel->width - uiconfig->paddingSize - action->width, app->rootPanel->height - uiconfig->paddingSize - action->height);
}

void MediaUi::removeStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (action->isConfirmed) {
		ui->invokeRemoveMediaStream ();
	}
	ui->actionWindow.clear ();
}

void MediaUi::invokeRemoveMediaStream () {
	App *app;
	ActionWindow *action;
	MediaWindow *card;
	Json *cmd, *params;
	int64_t jobid;
	int result;

	action = (ActionWindow *) actionWindow.widget;
	if (! action) {
		return;
	}

	card = MediaWindow::castWidget (action->sourceWidget.widget);
	if ((! card) || card->streamId.empty () || card->streamAgentId.empty ()) {
		return;
	}

	app = App::getInstance ();
	params = new Json ();
	params->set ("id", card->streamId);
	cmd = app->createCommand ("RemoveStream", SystemInterface::Constant_Stream, params);
	if (! cmd) {
		return;
	}

	params =  new Json ();
	params->set ("recordId", card->streamId);
	// TODO: Possibly restrict this command to the stream server's link agent
	app->agentControl.writeLinkCommand (app->createCommandJson ("WatchEvents", SystemInterface::Constant_Link, params));

	retain ();
	result = app->agentControl.invokeCommand (card->streamAgentId, cmd, MediaUi::removeMediaStreamComplete, this, &jobid);
	if (result != Result::SUCCESS) {
		// TODO: Show an error message here
		Log::write (Log::ERR, "Failed to execute RemoveStream command; err=%i agentId=\"%s\"", result, card->streamAgentId.c_str ());
		release ();
		return;
	}
}

void MediaUi::removeMediaStreamComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	// TODO: Implement this

	ui->release ();
}
