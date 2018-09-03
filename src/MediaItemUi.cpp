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
#include "SDL2/SDL.h"
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
#include "Menu.h"
#include "Ui.h"
#include "UiText.h"
#include "HashMap.h"
#include "Toolbar.h"
#include "TimelineBar.h"
#include "CardView.h"
#include "IconCardWindow.h"
#include "MediaWindow.h"
#include "ThumbnailWindow.h"
#include "ActionWindow.h"
#include "MediaItemUi.h"

const float MediaItemUi::smallImageScale = 0.123f;
const float MediaItemUi::mediumImageScale = 0.240f;
const float MediaItemUi::largeImageScale = 0.480f;

MediaItemUi::MediaItemUi (MediaWindow *card)
: Ui ()
, thumbnailCount (0)
, mediaWidth (0)
, mediaHeight (0)
, mediaDuration (0.0f)
, mediaFrameRate (0.0f)
, mediaSize (0)
, mediaBitrate (0)
, cardView (NULL)
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
, detailCard (NULL)
{
	streamServerAgentMap.sort (HashMap::sortAscending);
	mediaId.assign (card->mediaId);
	mediaName.assign (card->mediaName);
	mediaUrl.assign (card->mediaUrl);
	thumbnailUrl.assign (card->thumbnailUrl);
	thumbnailCount = card->thumbnailCount;
	mediaWidth = card->mediaWidth;
	mediaHeight = card->mediaHeight;
	mediaDuration = card->mediaDuration;
	mediaFrameRate = card->mediaFrameRate;
	mediaSize = card->mediaSize;
	mediaBitrate = card->mediaBitrate;
}

MediaItemUi::~MediaItemUi () {

}

StdString MediaItemUi::getBreadcrumbTitle () {
	return (mediaName);
}

Sprite *MediaItemUi::getBreadcrumbSprite () {
	// TODO: Implement this (possibly returning an icon scaled from the media card's thumbnail image)

	return (NULL);
}

StdString MediaItemUi::getSpritePath () {
	return (StdString ("ui/MediaItemUi/sprite"));
}

int MediaItemUi::doLoad () {
	App *app;
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	float t;
	int i, layout;
	Json *params;
	StdString cardid, url, cmdjson;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	uiconfig = &(app->uiConfig);

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setItemMarginSize (uiconfig->marginSize / 2.0f);
	cardView->position.assign (0.0f, app->topBarHeight);

	layout = app->prefsMap.find (App::prefsMediaItemImageSize, (int) ThumbnailWindow::MEDIUM_DETAIL);
	switch (layout) {
		case ThumbnailWindow::LOW_DETAIL: {
			MediaItemUi::viewSmallActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::MEDIUM_DETAIL: {
			MediaItemUi::viewMediumActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::HIGH_DETAIL: {
			MediaItemUi::viewLargeActionClicked (this, NULL);
			break;
		}
		default: {
			MediaItemUi::viewMediumActionClicked (this, NULL);
			break;
		}
	}

	detailCard = new IconCardWindow (sprites.getSprite (MediaItemUi::MEDIA_ICON), mediaName);
	cardView->addItem (detailCard, mediaId, 0);

	if ((thumbnailCount > 0) && (! thumbnailUrl.empty ())) {
		t = 0.0f;
		for (i = 0; i < thumbnailCount; ++i) {
			params = new Json ();
			params->set ("id", mediaId);
			params->set ("thumbnailIndex", i);
			cmdjson = app->createCommandJson ("GetThumbnailImage", SystemInterface::Constant_Media, params);
			if (cmdjson.empty ()) {
				continue;
			}

			url = interface->getInvokeUrl (thumbnailUrl, cmdjson);

			cardid.sprintf ("%i", i);
			cardView->addItem (new ThumbnailWindow (t, mediaWidth, mediaHeight, url, sprites.getSprite (MediaItemUi::LOADING_IMAGE_ICON), cardLayout, cardMaxImageWidth), cardid, 1);

			t += (mediaDuration / (float) thumbnailCount);
		}
	}

	cardView->refresh ();
	return (Result::SUCCESS);
}

void MediaItemUi::doUnload () {
	viewMenu.clear ();
	actionWindow.clear ();
	streamServerAgentMap.clear ();
}

void MediaItemUi::doResetMainToolbar (Toolbar *toolbar) {
	Button *button;

	button = new Button (StdString (""), sprites.getSprite (MediaItemUi::VIEW_BUTTON));
	button->setMouseClickCallback (MediaItemUi::viewButtonClicked, this);
// TODO: Set tooltip text
//     button->tooltipText.assign (app->uiText.mediaUiViewTooltip);
//     button->isMouseHoverEnabled = true;
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void MediaItemUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	TimelineBar *bar;

	app = App::getInstance ();
	toolbar->setLeftCorner (new Image (sprites.getSprite (MediaItemUi::TIME_ICON)));

	bar = new TimelineBar (app->windowWidth * 0.5f, 0.0f, mediaDuration);
	timelineBar.destroyAndClear ();
	timelineBar.assign (bar);
	toolbar->addLeftItem (bar);

	toolbar->isVisible = true;
}

void MediaItemUi::doClearPopupWidgets () {
	viewMenu.destroyAndClear ();
	actionWindow.destroyAndClear ();
}

void MediaItemUi::doRefresh () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MediaItemUi::doResume () {
	App *app;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/MediaItemUi/bg");

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MediaItemUi::doPause () {
	App *app;

	app = App::getInstance ();
	app->prefsMap.insert (App::prefsMediaItemImageSize, cardLayout);
}

void MediaItemUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MediaItemUi::doUpdate (int msElapsed) {
	viewMenu.compact ();
	actionWindow.compact ();
}

void MediaItemUi::doSyncRecordStore (RecordStore *store) {
	App *app;
	Toolbar *toolbar;
	SystemInterface *interface;
	UiText *uitext;
	UiConfiguration *uiconfig;
	Json *streamitem, *agentstatus;
	Button *button;
	StdString recordid, agentid, agentname, text, rationame, framesizename;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	uitext = &(app->uiText);
	uiconfig = &(app->uiConfig);

	streamitem = store->findRecord (MediaWindow::matchStreamSourceId, &mediaId);
	if (! streamitem) {
		streamAgentId.assign ("");
		streamId.assign ("");
		streamAgentName.assign ("");
	}
	else {
		recordid = interface->getCommandRecordId (streamitem);
		if (! recordid.empty ()) {
			agentid = interface->getCommandAgentId (streamitem);
		}
		if (! agentid.empty ()) {
			agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentid);
			if (agentstatus) {
				agentname = interface->getCommandAgentName (agentstatus);
			}
		}

		if ((! recordid.empty ()) && (! agentid.empty ()) && (! agentname.empty ())) {
			streamId.assign (recordid);
			streamAgentId.assign (agentid);
			streamAgentName.assign (agentname);
		}
	}

	toolbar = app->secondaryToolbar;
	toolbar->clearRightItems ();
	if (streamId.empty ()) {
		button = new Button (uitext->createStream.uppercased ());
		button->setRaised (true, uiconfig->darkInverseBackgroundColor);
		button->setTextColor (uiconfig->raisedButtonInverseTextColor);
		button->setInverseColor (true);
		button->setMouseClickCallback (MediaItemUi::createStreamButtonClicked, this);
		toolbar->addRightItem (button);
	}
	else {
		button = new Button (uitext->removeStream.uppercased ());
		button->setRaised (true, uiconfig->darkInverseBackgroundColor);
		button->setTextColor (uiconfig->raisedButtonInverseTextColor);
		button->setInverseColor (true);
		button->setMouseClickCallback (MediaItemUi::removeStreamButtonClicked, this);
		toolbar->addRightItem (button);
	}

	text.sprintf ("%s: %ix%i", uitext->frameSize.capitalized ().c_str (), mediaWidth, mediaHeight);
	rationame = Util::getAspectRatioDisplayString (mediaWidth, mediaHeight);
	framesizename = Util::getFrameSizeName (mediaWidth, mediaHeight);
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

	if (mediaFrameRate <= 0.0f) {
		text.appendSprintf ("\n%s: %s", uitext->frameRate.capitalized ().c_str (), uitext->unknown.capitalized ().c_str ());
	}
	else {
		text.appendSprintf ("\n%s: %.2ffps", uitext->frameRate.capitalized ().c_str (), mediaFrameRate);
	}

	if (mediaBitrate <= 0.0f) {
		text.appendSprintf ("\n%s: %s", uitext->bitrate.capitalized ().c_str (), uitext->unknown.capitalized ().c_str ());
	}
	else {
		text.appendSprintf ("\n%s: %s", uitext->bitrate.capitalized ().c_str (), Util::getBitrateDisplayString (mediaBitrate).c_str ());
	}

	text.appendSprintf ("\n%s: %s (%s)", uitext->fileSize.capitalized ().c_str (), Util::getFileSizeDisplayString (mediaSize).c_str (), uitext->getCountText (UiText::BYTES, mediaSize).c_str ());
	text.appendSprintf ("\n%s: %s (%s)", uitext->duration.capitalized ().c_str (), Util::getDurationDisplayString (mediaDuration).c_str (), Util::getDurationString (mediaDuration, Util::HOURS).c_str ());
	if (! streamAgentName.empty ()) {
		text.appendSprintf ("\n%s: %s", uitext->streamServer.capitalized ().c_str (), streamAgentName.c_str ());
	}
	detailCard->setDetailText (text);
	cardView->refresh ();

	store->populateAgentMap (&streamServerAgentMap, StdString ("streamServerStatus"));
}

void MediaItemUi::viewButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;

	ui = (MediaItemUi *) uiPtr;
	ui->handleViewButtonClick (widgetPtr);
}

void MediaItemUi::handleViewButtonClick (Widget *buttonWidget) {
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
	menu->addItem (uitext->small.capitalized (), sprites.getSprite (MediaItemUi::SMALL_THUMBNAILS_ICON), MediaItemUi::viewSmallActionClicked, this, 0, cardLayout == ThumbnailWindow::LOW_DETAIL);
	menu->addItem (uitext->medium.capitalized (), sprites.getSprite (MediaItemUi::MEDIUM_THUMBNAILS_ICON), MediaItemUi::viewMediumActionClicked, this, 0, cardLayout == ThumbnailWindow::MEDIUM_DETAIL);
	menu->addItem (uitext->large.capitalized (), sprites.getSprite (MediaItemUi::LARGE_THUMBNAILS_ICON), MediaItemUi::viewLargeActionClicked, this, 0, cardLayout == ThumbnailWindow::HIGH_DETAIL);
	menu->position.assign (buttonWidget->position.x + buttonWidget->width - menu->width, buttonWidget->position.y + buttonWidget->height);
	viewMenu.assign (menu);
}

void MediaItemUi::viewSmallActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	float y0, h0;

	ui = (MediaItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::LOW_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::LOW_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * MediaItemUi::smallImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.12f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void MediaItemUi::viewMediumActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	float y0, h0;

	ui = (MediaItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::MEDIUM_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::MEDIUM_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * MediaItemUi::mediumImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.24f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void MediaItemUi::viewLargeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	float y0, h0;

	ui = (MediaItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::HIGH_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::HIGH_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * MediaItemUi::largeImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.36f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void MediaItemUi::resetCardLayout (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ThumbnailWindow *window;

	ui = (MediaItemUi *) uiPtr;
	window = ThumbnailWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	window->setLayout (ui->cardLayout, ui->cardMaxImageWidth);
}

void MediaItemUi::createStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	MediaItemUi *ui;
	ActionWindow *action;
	UiText *uitext;

	ui = (MediaItemUi *) uiPtr;
	if (ui->streamServerAgentMap.empty ()) {
		// TODO: Show an error message here
		return;
	}

	app = App::getInstance ();
	uitext = &(app->uiText);
	ui->clearPopupWidgets ();
	action = (ActionWindow *) app->rootPanel->addWidget (new ActionWindow (StdString::createSprintf ("%s: %s", uitext->createStream.capitalized ().c_str (), ui->mediaName.c_str ()), uitext->createStream.uppercased ()));
	ui->actionWindow.assign (action);
	action->addComboBoxOption (uitext->streamServer.capitalized (), &(ui->streamServerAgentMap));
	action->setCloseCallback (MediaItemUi::createStreamActionClosed, ui);
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (widgetPtr->drawX + widgetPtr->width - action->width, widgetPtr->drawY + widgetPtr->height - action->height);
}

void MediaItemUi::createStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ActionWindow *action;

	ui = (MediaItemUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (action->isConfirmed) {
		ui->invokeCreateMediaStream ();
	}
	ui->actionWindow.clear ();
}

void MediaItemUi::invokeCreateMediaStream () {
	App *app;
	UiText *uitext;
	ActionWindow *action;
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

	params = new Json ();
	params->set ("name", mediaName);
	params->set ("mediaId", mediaId);
	params->set ("mediaUrl", mediaUrl);
	cmd = app->createCommand ("CreateMediaStream", SystemInterface::Constant_Stream, params);
	if (! cmd) {
		return;
	}

	retain ();
	result = app->agentControl.invokeCommand (agentid, cmd, MediaItemUi::createMediaStreamComplete, this, &jobid);
	if (result != Result::SUCCESS) {
		// TODO: Show an error message here
		Log::write (Log::ERR, "Failed to execute CreateMediaStream command; err=%i agentId=\"%s\"", result, agentid.c_str ());
		release ();
		return;
	}
}

void MediaItemUi::createMediaStreamComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	MediaItemUi *ui;
	App *app;
	UiText *uitext;
	SystemInterface *interface;
	Json *params;
	StdString recordid, agentname, text;


	ui = (MediaItemUi *) uiPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);
	interface = &(app->systemInterface);
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_CommandResult)) {
		recordid = interface->getCommandStringParam (responseCommand, "taskId", "");
		if (! recordid.empty ()) {
			params = new Json ();
			params->set ("recordId", recordid);
			// TODO: Possibly restrict this command to the stream server's link agent
			app->agentControl.writeLinkCommand (app->createCommandJson ("WatchEvents", SystemInterface::Constant_Link, params));
		}

		agentname = app->agentControl.getAgentDisplayName (agentId);
		if (! agentname.empty ()) {
			text.appendSprintf ("%s: ", agentname.c_str ());
		}
		text.appendSprintf ("%s, %s", uitext->startedCreatingStream.capitalized ().c_str (), ui->mediaName.c_str ());
		app->showSnackbar (text);
	}

	ui->release ();
}

void MediaItemUi::removeStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	MediaItemUi *ui;
	ActionWindow *action;
	UiText *uitext;

	ui = (MediaItemUi *) uiPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);
	ui->clearPopupWidgets ();
	action = (ActionWindow *) app->rootPanel->addWidget (new ActionWindow (StdString::createSprintf ("%s: %s", uitext->removeStream.capitalized ().c_str (), ui->mediaName.c_str ()), uitext->removeStream.uppercased ()));
	ui->actionWindow.assign (action);
	action->setCloseCallback (MediaItemUi::removeStreamActionClosed, ui);
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (widgetPtr->drawX + widgetPtr->width - action->width, widgetPtr->drawY + widgetPtr->height - action->height);
}

void MediaItemUi::removeStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ActionWindow *action;

	ui = (MediaItemUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (action->isConfirmed) {
		ui->invokeRemoveMediaStream ();
	}
	ui->actionWindow.clear ();
}

void MediaItemUi::invokeRemoveMediaStream () {
	App *app;
	ActionWindow *action;
	Json *cmd, *params;
	int64_t jobid;
	int result;

	action = (ActionWindow *) actionWindow.widget;
	if (! action) {
		return;
	}

	if (streamId.empty () || streamAgentId.empty ()) {
		return;
	}

	app = App::getInstance ();
	params = new Json ();
	params->set ("id", streamId);
	cmd = app->createCommand ("RemoveStream", SystemInterface::Constant_Stream, params);
	if (! cmd) {
		return;
	}

	params =  new Json ();
	params->set ("recordId", streamId);
	// TODO: Possibly restrict this command to the stream server's link agent
	app->agentControl.writeLinkCommand (app->createCommandJson ("WatchEvents", SystemInterface::Constant_Link, params));

	retain ();
	result = app->agentControl.invokeCommand (streamAgentId, cmd, MediaItemUi::removeMediaStreamComplete, this, &jobid);
	if (result != Result::SUCCESS) {
		// TODO: Show an error message here
		Log::write (Log::ERR, "Failed to execute RemoveStream command; err=%i agentId=\"%s\"", result, streamAgentId.c_str ());
		release ();
		return;
	}
}

void MediaItemUi::removeMediaStreamComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	MediaItemUi *ui;
	App *app;
	SystemInterface *interface;
	UiText *uitext;
	StdString agentname, text;

	ui = (MediaItemUi *) uiPtr;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	uitext = &(app->uiText);

	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		agentname = app->agentControl.getAgentDisplayName (agentId);
		if (! agentname.empty ()) {
			text.appendSprintf ("%s: ", agentname.c_str ());
		}
		text.append (uitext->removedStream.capitalized ());
		app->showSnackbar (text);
	}
	else {
		// TODO: Show an error message here
	}
	ui->release ();
}
