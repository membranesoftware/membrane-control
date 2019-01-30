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
#include "ComboBox.h"
#include "HelpWindow.h"
#include "IconCardWindow.h"
#include "MediaWindow.h"
#include "ThumbnailWindow.h"
#include "ActionWindow.h"
#include "MediaDetailWindow.h"
#include "StreamDetailWindow.h"
#include "MediaItemUi.h"

MediaItemUi::MediaItemUi (MediaWindow *mediaWindow)
: Ui ()
, cardView (NULL)
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
, timelineBar (NULL)
, streamCount (0)
{
	sourceMediaWindow.assign (mediaWindow);
	streamServerAgentMap.sort (HashMap::sortAscending);
}

MediaItemUi::~MediaItemUi () {

}

StdString MediaItemUi::getBreadcrumbTitle () {
	if (! sourceMediaWindow.widget) {
		return (StdString (""));
	}
	return (((MediaWindow *) sourceMediaWindow.widget)->mediaName);
}

StdString MediaItemUi::getSpritePath () {
	return (StdString ("ui/MediaItemUi/sprite"));
}

void MediaItemUi::setHelpWindowContent (Widget *helpWindowPtr) {
	App *app;
	HelpWindow *help;
	UiText *uitext;

	help = (HelpWindow *) helpWindowPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);

	help->setHelpText (uitext->getText (UiTextString::mediaItemUiHelpTitle), uitext->getText (UiTextString::mediaItemUiHelpText));
	if (streamCount <= 0) {
		help->addAction (uitext->getText (UiTextString::mediaItemUiHelpAction1Text), uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("media-streaming"));
	}

	help->addTopicLink (uitext->getText (UiTextString::mediaStreaming).capitalized (), Util::getHelpUrl ("media-streaming"));
	help->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), Util::getHelpUrl (""));
}

int MediaItemUi::doLoad () {
	App *app;
	UiConfiguration *uiconfig;
	MediaWindow *mediawindow;
	MediaDetailWindow *detailwindow;
	ThumbnailWindow *thumbnailwindow;
	int layout, i, thumbnailcount;
	float t;
	Json *params;
	StdString cardid, url, cmdjson;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	mediawindow = (MediaWindow *) sourceMediaWindow.widget;

	streamCount = 0;
	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowItemMarginSize (1, uiconfig->marginSize / 2.0f);
	cardView->position.assign (0.0f, app->topBarHeight);

	layout = app->prefsMap.find (App::prefsMediaItemImageSize, (int) ThumbnailWindow::MEDIUM_DETAIL);
	switch (layout) {
		case ThumbnailWindow::LOW_DETAIL: {
			MediaItemUi::smallThumbnailActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::MEDIUM_DETAIL: {
			MediaItemUi::mediumThumbnailActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::HIGH_DETAIL: {
			MediaItemUi::largeThumbnailActionClicked (this, NULL);
			break;
		}
		default: {
			MediaItemUi::mediumThumbnailActionClicked (this, NULL);
			break;
		}
	}

	detailwindow = new MediaDetailWindow (mediawindow->mediaId);
	cardView->addItem (detailwindow, mediawindow->mediaId, 0);

	thumbnailcount = mediawindow->thumbnailCount;
	if ((thumbnailcount > 0) && (! mediawindow->thumbnailPath.empty ())) {
		t = 0.0f;
		for (i = 0; i < thumbnailcount; ++i) {
			params = new Json ();
			params->set ("id", mediawindow->mediaId);
			params->set ("thumbnailIndex", i);
			url = app->agentControl.getAgentSecondaryUrl (mediawindow->agentId, app->createCommand ("GetThumbnailImage", SystemInterface::Constant_Media, params), mediawindow->thumbnailPath);

			cardid.sprintf ("%i", i);
			thumbnailwindow = new ThumbnailWindow (i, t, mediawindow->mediaWidth, mediawindow->mediaHeight, url, cardLayout, cardMaxImageWidth);
			thumbnailwindow->setMouseEnterCallback (MediaItemUi::thumbnailMouseEntered, this);
			thumbnailwindow->setMouseExitCallback (MediaItemUi::thumbnailMouseExited, this);
			cardView->addItem (thumbnailwindow, cardid, 1);

			t += (mediawindow->mediaDuration / (float) thumbnailcount);
		}
	}

	cardView->refresh ();
	return (Result::SUCCESS);
}

void MediaItemUi::doUnload () {
	thumbnailSizeMenu.clear ();
	streamServerAgentMap.clear ();
}

void MediaItemUi::doResetMainToolbar (Toolbar *toolbar) {
	App *app;
	UiText *uitext;
	Button *button;

	app = App::getInstance ();
	uitext = &(app->uiText);

	button = new Button (StdString (""), sprites.getSprite (MediaItemUi::THUMBNAIL_SIZE_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaItemUi::thumbnailSizeButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void MediaItemUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	UiText *uitext;
	Button *button;

	app = App::getInstance ();
	uitext = &(app->uiText);
	toolbar->setLeftCorner (new Image (sprites.getSprite (MediaItemUi::TIME_ICON)));

	timelineBar = new TimelineBar (app->windowWidth * 0.5f, ((MediaWindow *) sourceMediaWindow.widget)->mediaId);
	toolbar->addLeftItem (timelineBar);

	button = new Button (StdString (""), sprites.getSprite (MediaItemUi::CREATE_STREAM_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaItemUi::createStreamButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::createStreamTooltip));
	toolbar->addRightItem (button);

	toolbar->isVisible = true;
}

void MediaItemUi::doClearPopupWidgets () {
	thumbnailSizeMenu.destroyAndClear ();
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
	thumbnailSizeMenu.compact ();
}

void MediaItemUi::doSyncRecordStore (RecordStore *store) {
	store->populateAgentMap (&streamServerAgentMap, StdString ("streamServerStatus"));

	streamCount = 0;
	store->processRecords (MediaWindow::matchStreamSourceId, &(((MediaWindow *) sourceMediaWindow.widget)->mediaId), MediaItemUi::processStreamItem, this);

	timelineBar->syncRecordStore (store);
	cardView->syncRecordStore (store);
	cardView->refresh ();
}

void MediaItemUi::processStreamItem (void *uiPtr, Json *record, const StdString &recordId) {
	MediaItemUi *ui;
	StreamDetailWindow *window;

	ui = (MediaItemUi *) uiPtr;
	++(ui->streamCount);
	if (! ui->cardView->contains (recordId)) {
		window = new StreamDetailWindow (recordId);
		window->setMenuClickCallback (MediaItemUi::streamDetailMenuClicked, ui);
		ui->cardView->addItem (window, recordId, 0);
	}
}

void MediaItemUi::thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;

	ui = (MediaItemUi *) uiPtr;
	ui->handleThumbnailSizeButtonClick (widgetPtr);
}

void MediaItemUi::handleThumbnailSizeButtonClick (Widget *buttonWidget) {
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
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), sprites.getSprite (MediaItemUi::SMALL_THUMBNAILS_ICON), MediaItemUi::smallThumbnailActionClicked, this, 0, cardLayout == ThumbnailWindow::LOW_DETAIL);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), sprites.getSprite (MediaItemUi::MEDIUM_THUMBNAILS_ICON), MediaItemUi::mediumThumbnailActionClicked, this, 0, cardLayout == ThumbnailWindow::MEDIUM_DETAIL);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), sprites.getSprite (MediaItemUi::LARGE_THUMBNAILS_ICON), MediaItemUi::largeThumbnailActionClicked, this, 0, cardLayout == ThumbnailWindow::HIGH_DETAIL);
	menu->position.assign (buttonWidget->position.x + buttonWidget->width - menu->width, buttonWidget->position.y + buttonWidget->height);
	thumbnailSizeMenu.assign (menu);
}

void MediaItemUi::smallThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MediaItemUi *) uiPtr;
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
	ui->cardView->processItems (MediaItemUi::resetCardLayout, ui);
	ui->cardView->setRowItemMarginSize (1, 0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.1f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void MediaItemUi::mediumThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MediaItemUi *) uiPtr;
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
	ui->cardView->processItems (MediaItemUi::resetCardLayout, ui);
	ui->cardView->setRowItemMarginSize (1, App::getInstance ()->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.2f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void MediaItemUi::largeThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MediaItemUi *) uiPtr;
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
	ui->cardView->processItems (MediaItemUi::resetCardLayout, ui);
	ui->cardView->setRowItemMarginSize (1, App::getInstance ()->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.3f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void MediaItemUi::thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ThumbnailWindow *window;

	ui = (MediaItemUi *) uiPtr;
	window = (ThumbnailWindow *) widgetPtr;
	ui->timelineBar->setHighlightedMarker (window->thumbnailIndex);
}

void MediaItemUi::thumbnailMouseExited (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ThumbnailWindow *window;

	ui = (MediaItemUi *) uiPtr;
	window = (ThumbnailWindow *) widgetPtr;
	if (ui->timelineBar->highlightedMarkerIndex == window->thumbnailIndex) {
		ui->timelineBar->setHighlightedMarker (-1);
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

void MediaItemUi::streamDetailMenuClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StreamDetailWindow *target;
	Menu *action;
	bool show;

	ui = (MediaItemUi *) uiPtr;
	target = (StreamDetailWindow *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == target)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = (Menu *) app->rootPanel->addWidget (new Menu ());
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (target);

	action->addItem (uitext->getText (UiTextString::remove).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON), MediaItemUi::removeActionClicked, ui);

	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->position.assign (target->drawX + target->width - action->width - uiconfig->marginSize, target->drawY + target->menuPositionY);
}

void MediaItemUi::createStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	MediaItemUi *ui;
	UiText *uitext;
	ActionWindow *action;
	Widget *target;
	ComboBox *combobox;

	ui = (MediaItemUi *) uiPtr;
	action = ActionWindow::castWidget (ui->actionWidget.widget);
	target = ui->actionTarget.widget;
	if (action && (target == widgetPtr)) {
		ui->clearPopupWidgets ();
		ui->actionTarget.clear ();
		return;
	}

	if (ui->streamServerAgentMap.empty ()) {
		// TODO: Show an error message here
		return;
	}

	app = App::getInstance ();
	uitext = &(app->uiText);
	ui->clearPopupWidgets ();
	ui->actionTarget.assign (widgetPtr);
	action = (ActionWindow *) app->rootPanel->addWidget (new ActionWindow ());
	ui->actionWidget.assign (action);
	action->setInverseColor (true);
	action->setTitleText (uitext->getText (UiTextString::createStream).capitalized ());
	action->setConfirmButtonText (uitext->getText (UiTextString::createStream).uppercased ());

// TODO: Allow selection of a stream server (currently restricted to the server holding the source media item)
//	action->addComboBoxOption (uitext->getText (UiTextString::streamServer).capitalized (), &(ui->streamServerAgentMap));

	combobox = new ComboBox ();
	combobox->addItem (uitext->getText (UiTextString::normalVideoQualityDescription));
	combobox->addItem (uitext->getText (UiTextString::highVideoQualityDescription));
	combobox->addItem (uitext->getText (UiTextString::lowVideoQualityDescription));
	combobox->addItem (uitext->getText (UiTextString::lowestVideoQualityDescription));
	action->addOption (uitext->getText (UiTextString::videoQuality).capitalized (), combobox, uitext->getText (UiTextString::videoQualityDescription));

	action->setCloseCallback (MediaItemUi::createStreamActionClosed, ui);
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (widgetPtr->drawX + widgetPtr->width - action->width, widgetPtr->drawY - action->height);
}

void MediaItemUi::createStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ActionWindow *action;

	ui = (MediaItemUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (action->isConfirmed) {
		ui->invokeCreateMediaStream ();
	}
	ui->actionWidget.clear ();
}

void MediaItemUi::invokeCreateMediaStream () {
	App *app;
	UiText *uitext;
	MediaWindow *mediawindow;
	ActionWindow *action;
	StdString agentid, quality;
	Json *params;
	int result, w, h;

	app = App::getInstance ();
	uitext = &(app->uiText);
	mediawindow = (MediaWindow *) sourceMediaWindow.widget;
	action = ActionWindow::castWidget (actionWidget.widget);
	if (! action) {
		return;
	}

	agentid.assign (mediawindow->agentId);
// TODO: Allow selection of a stream server (currently restricted to the server holding the source media item)
//	agentid = action->getStringValue (uitext->getText (UiTextString::streamServer), "");
	if (agentid.empty ()) {
		return;
	}

	params = new Json ();
	params->set ("name", mediawindow->mediaName);
	params->set ("mediaServerAgentId", mediawindow->agentId);
	params->set ("mediaId", mediawindow->mediaId);

	quality = action->getStringValue (uitext->getText (UiTextString::videoQuality).capitalized (), "");
	if (quality.equals (uitext->getText (UiTextString::normalVideoQualityDescription))) {
		params->set ("h264Preset", "faster");
	}
	else if (quality.equals (uitext->getText (UiTextString::highVideoQualityDescription))) {
		params->set ("h264Preset", "slower");
	}
	else if (quality.equals (uitext->getText (UiTextString::lowVideoQualityDescription))) {
		params->set ("h264Preset", "medium");
		w = mediawindow->mediaWidth;
		h = mediawindow->mediaHeight;
		w /= 2;
		h /= 2;
		w -= (w % 16);
		h -= (h % 16);
		if ((w > 0) && (h > 0)) {
			params->set ("width", w);
			params->set ("height", h);
		}
	}
	else if (quality.equals (uitext->getText (UiTextString::lowestVideoQualityDescription))) {
		params->set ("h264Preset", "medium");
		w = mediawindow->mediaWidth;
		h = mediawindow->mediaHeight;
		w /= 4;
		h /= 4;
		w -= (w % 16);
		h -= (h % 16);
		if ((w > 0) && (h > 0)) {
			params->set ("width", w);
			params->set ("height", h);
		}
	}

	// TODO: Populate the mediaUrl field (to allow streams to be populated on a stream server other than the source agent)
	//	params->set ("mediaUrl", url);

	retain ();
	result = app->agentControl.invokeCommand (agentid, app->createCommand ("CreateMediaStream", SystemInterface::Constant_Stream, params), MediaItemUi::createMediaStreamComplete, this);
	if (result != Result::SUCCESS) {
		release ();
		Log::write (Log::DEBUG, "Failed to invoke CreateMediaStream command; err=%i agentId=\"%s\"", result, agentid.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
		return;
	}
}

void MediaItemUi::createMediaStreamComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	MediaItemUi *ui;
	App *app;
	MediaWindow *mediawindow;
	UiText *uitext;
	SystemInterface *interface;
	StdString recordid, agentname, text;

	ui = (MediaItemUi *) uiPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);
	interface = &(app->systemInterface);
	mediawindow = (MediaWindow *) ui->sourceMediaWindow.widget;
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_CommandResult)) {
		recordid = interface->getCommandStringParam (responseCommand, "taskId", "");
		// TODO: Watch the task for events
		/*
		if (! recordid.empty ()) {
			params = new Json ();
			params->set ("recordId", recordid);
			app->agentControl.writeLinkCommand (app->createCommandJson ("WatchEvents", SystemInterface::Constant_Link, params));
		}
		*/

		agentname = app->agentControl.getAgentDisplayName (agentId);
		if (! agentname.empty ()) {
			text.appendSprintf ("%s: ", agentname.c_str ());
		}
		text.appendSprintf ("%s, %s", uitext->getText (UiTextString::startedCreatingStream).capitalized ().c_str (), mediawindow->mediaName.c_str ());
		app->showSnackbar (text);
	}

	ui->release ();
}

void MediaItemUi::removeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;

	ui = (MediaItemUi *) uiPtr;
	ui->clearPopupWidgets ();
	ui->invokeRemoveMediaStream ();
}

void MediaItemUi::invokeRemoveMediaStream () {
	App *app;
	StreamDetailWindow *target;
	Json *params;
	int result;

	app = App::getInstance ();
	target = StreamDetailWindow::castWidget (actionTarget.widget);
	if (! target) {
		return;
	}

	params = new Json ();
	params->set ("id", target->recordId);

	retain ();
	result = app->agentControl.invokeCommand (target->agentId, app->createCommand ("RemoveStream", SystemInterface::Constant_Stream, params), MediaItemUi::removeStreamComplete, this);
	if (result != Result::SUCCESS) {
		release ();
		Log::write (Log::DEBUG, "Failed to invoke RemoveStream command; err=%i agentId=\"%s\"", result, target->agentId.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
		return;
	}
}

void MediaItemUi::removeStreamComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	MediaItemUi *ui;
	App *app;
	SystemInterface *interface;
	UiText *uitext;
	StdString agentname, text, id;

	ui = (MediaItemUi *) uiPtr;
	app = App::getInstance ();
	interface = &(app->systemInterface);
	uitext = &(app->uiText);

	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		id = interface->getCommandStringParam (invokeCommand, "id", "");
		if (! id.empty ()) {
			app->agentControl.recordStore.removeRecord (id);
			ui->cardView->removeItem (id);
		}
		agentname = app->agentControl.getAgentDisplayName (agentId);
		if (! agentname.empty ()) {
			text.appendSprintf ("%s: ", agentname.c_str ());
		}
		text.append (uitext->getText (UiTextString::removedStream).capitalized ());
		app->showSnackbar (text);
	}
	else {
		// TODO: Show an error message here
	}
	ui->release ();
}
