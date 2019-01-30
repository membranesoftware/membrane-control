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
#include "Ui.h"
#include "SystemInterface.h"
#include "StringList.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "Json.h"
#include "Menu.h"
#include "MonitorWindow.h"
#include "IconCardWindow.h"
#include "IconLabelWindow.h"
#include "StreamWindow.h"
#include "ThumbnailWindow.h"
#include "HelpWindow.h"
#include "StreamItemUi.h"
#include "MonitorCacheUi.h"

MonitorCacheUi::MonitorCacheUi (const StdString &agentId, const StdString &agentName, int streamWindowLayout)
: Ui ()
, agentId (agentId)
, agentName (agentName)
, streamWindowLayout (streamWindowLayout)
, cardView (NULL)
, stopButton (NULL)
, playButton (NULL)
, writePlaylistButton (NULL)
, deleteButton (NULL)
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
, streamCount (0)
, streamSetSize (-1)
, recordReceiveCount (0)
, nextRecordSyncTime (0)
{
}

MonitorCacheUi::~MonitorCacheUi () {

}

StdString MonitorCacheUi::getSpritePath () {
	return (StdString ("ui/MonitorCacheUi/sprite"));
}

StdString MonitorCacheUi::getBreadcrumbTitle () {
	return (agentName);
}

Sprite *MonitorCacheUi::getBreadcrumbSprite () {
	return (sprites.getSprite (MonitorCacheUi::BREADCRUMB_ICON)->copy ());
}

void MonitorCacheUi::setHelpWindowContent (Widget *helpWindowPtr) {
	App *app;
	HelpWindow *help;
	UiText *uitext;

	help = (HelpWindow *) helpWindowPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);

	help->setHelpText (uitext->getText (UiTextString::monitorCacheUiHelpTitle), uitext->getText (UiTextString::monitorCacheUiHelpText));
	if ((streamCount <= 0) && (streamSetSize == 0)) {
		help->addAction (uitext->getText (UiTextString::monitorCacheUiHelpAction1Text));
	}
	else {
		help->addAction (uitext->getText (UiTextString::monitorCacheUiHelpAction2Text));
	}

	help->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), Util::getHelpUrl (""));
}

int MonitorCacheUi::doLoad () {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	streamSetSize = -1;
	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (1, uitext->getText (UiTextString::videoStreams).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, app->topBarHeight);

	switch (streamWindowLayout) {
		case StreamWindow::LOW_DETAIL: {
			MonitorCacheUi::smallThumbnailActionClicked (this, NULL);
			break;
		}
		case StreamWindow::MEDIUM_DETAIL: {
			MonitorCacheUi::mediumThumbnailActionClicked (this, NULL);
			break;
		}
		case StreamWindow::HIGH_DETAIL: {
			MonitorCacheUi::largeThumbnailActionClicked (this, NULL);
			break;
		}
		default: {
			MonitorCacheUi::smallThumbnailActionClicked (this, NULL);
			break;
		}
	}

	return (Result::SUCCESS);
}

void MonitorCacheUi::doUnload () {
	emptyStreamWindow.destroyAndClear ();
	selectedStreamWindow.clear ();
	commandPopup.destroyAndClear ();
	commandButton.clear ();
}

void MonitorCacheUi::doResetMainToolbar (Toolbar *toolbar) {
	App *app;
	Button *button;

	app = App::getInstance ();

	button = new Button (StdString (""), sprites.getSprite (MonitorCacheUi::THUMBNAIL_SIZE_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (MonitorCacheUi::thumbnailSizeButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.getText (UiTextString::mediaUiThumbnailSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (StdString (""), app->uiConfig.coreSprites.getSprite (UiConfiguration::RELOAD_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (MonitorCacheUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void MonitorCacheUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	deleteButton = new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON));
	deleteButton->setInverseColor (true);
	deleteButton->setMouseClickCallback (MonitorCacheUi::deleteButtonClicked, this);
	deleteButton->setMouseEnterCallback (MonitorCacheUi::commandButtonMouseEntered, this);
	deleteButton->setMouseExitCallback (MonitorCacheUi::commandButtonMouseExited, this);
	deleteButton->setMouseHoverTooltip (app->uiText.getText (UiTextString::monitorCacheUiDeleteTooltip));
	toolbar->addLeftItem (deleteButton);

	playButton = new Button (StdString (""), sprites.getSprite (MonitorCacheUi::PLAY_BUTTON));
	playButton->setInverseColor (true);
	playButton->setMouseClickCallback (MonitorCacheUi::playButtonClicked, this);
	playButton->setMouseEnterCallback (MonitorCacheUi::commandButtonMouseEntered, this);
	playButton->setMouseExitCallback (MonitorCacheUi::commandButtonMouseExited, this);
	playButton->shortcutKey = SDLK_F3;
	playButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorCacheUiPlayTooltip), Widget::LEFT);
	toolbar->addRightItem (playButton);

	writePlaylistButton = new Button (StdString (""), sprites.getSprite (MonitorCacheUi::WRITE_PLAYLIST_BUTTON));
	writePlaylistButton->setInverseColor (true);
	writePlaylistButton->setMouseClickCallback (MonitorCacheUi::writePlaylistButtonClicked, this);
	writePlaylistButton->setMouseEnterCallback (MonitorCacheUi::commandButtonMouseEntered, this);
	writePlaylistButton->setMouseExitCallback (MonitorCacheUi::commandButtonMouseExited, this);
	writePlaylistButton->shortcutKey = SDLK_F2;
	writePlaylistButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorCacheUiWritePlaylistTooltip), Widget::LEFT);
	toolbar->addRightItem (writePlaylistButton);

	stopButton = new Button (StdString (""), sprites.getSprite (MonitorCacheUi::STOP_BUTTON));
	stopButton->setInverseColor (true);
	stopButton->setMouseClickCallback (MonitorCacheUi::stopButtonClicked, this);
	stopButton->setMouseEnterCallback (MonitorCacheUi::commandButtonMouseEntered, this);
	stopButton->setMouseExitCallback (MonitorCacheUi::commandButtonMouseExited, this);
	stopButton->shortcutKey = SDLK_F1;
	stopButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorCacheUiStopTooltip), Widget::LEFT);
	toolbar->addRightItem (stopButton);

	toolbar->isVisible = true;
}

void MonitorCacheUi::doClearPopupWidgets () {
	commandPopup.destroyAndClear ();
	commandButton.clear ();
}

void MonitorCacheUi::doResume () {
	App *app;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/MonitorCacheUi/bg");
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MonitorCacheUi::doRefresh () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MonitorCacheUi::doPause () {

}

void MonitorCacheUi::doUpdate (int msElapsed) {
	App *app;
	StreamWindow *window;
	int64_t now;

	app = App::getInstance ();
	emptyStreamWindow.compact ();
	commandPopup.compact ();

	selectedStreamWindow.compact ();
	window = StreamWindow::castWidget (selectedStreamWindow.widget);
	if (window && (! window->isSelected)) {
		selectedStreamWindow.clear ();
	}

	if (recordReceiveCount > 0) {
		now = Util::getTime ();
		if ((recordReceiveCount > 16) || (now >= nextRecordSyncTime)) {
			app->shouldSyncRecordStore = true;
			recordReceiveCount = 0;
		}
	}
}

void MonitorCacheUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MonitorCacheUi::doSyncRecordStore (RecordStore *store) {
	MonitorWindow *monitorwindow;
	IconCardWindow *iconwindow;
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	if (! cardView->contains (agentId)) {
		if (store->findRecord (agentId, SystemInterface::Command_AgentStatus)) {
			monitorwindow = new MonitorWindow (agentId);
			monitorwindow->setStorageDisplayEnabled (true);
			monitorwindow->setExpanded (true);
			cardView->addItem (monitorwindow, agentId, 0);
			addLinkAgent (agentId);
		}
	}

	streamCount = 0;
	store->processCommandRecords (SystemInterface::Command_StreamItem, MonitorCacheUi::processStreamItem, this);

	iconwindow = (IconCardWindow *) emptyStreamWindow.widget;
	if ((streamCount <= 0) && (streamSetSize == 0)) {
		if (! iconwindow) {
			iconwindow = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LARGE_STREAM_ICON), uitext->getText (UiTextString::monitorCacheUiEmptyStreamStatusTitle), StdString (""), uitext->getText (UiTextString::monitorCacheUiEmptyStreamStatusText));
			iconwindow->setLink (uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("media-streaming"));
			emptyStreamWindow.assign (iconwindow);

			iconwindow->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (iconwindow, iconwindow->itemId, 1);
		}
	}
	else {
		if (iconwindow) {
			cardView->removeItem (iconwindow->itemId);
			emptyStreamWindow.clear ();
		}
	}

	cardView->syncRecordStore (store);
	cardView->refresh ();
}

void MonitorCacheUi::handleLinkClientConnect (const StdString &linkAgentId) {
	App *app;
	Json *params;

	app = App::getInstance ();
	if (! agentId.equals (linkAgentId)) {
		return;
	}

	params = new Json ();
	params->set ("maxResults", 0);
	app->agentControl.writeLinkCommand (app->createCommand ("FindItems", SystemInterface::Constant_Monitor, params), agentId);
}

void MonitorCacheUi::handleLinkClientCommand (const StdString &linkAgentId, int commandId, Json *command) {
	App *app;
	SystemInterface *interface;
	UiConfiguration *uiconfig;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	uiconfig = &(app->uiConfig);
	if (! agentId.equals (linkAgentId)) {
		return;
	}

	switch (commandId) {
		case SystemInterface::Command_FindStreamsResult: {
			streamSetSize = interface->getCommandNumberParam (command, "setSize", (int) 0);
			app->shouldSyncRecordStore = true;
			break;
		}
		case SystemInterface::Command_StreamItem: {
			app->agentControl.recordStore.addRecord (command);
			++recordReceiveCount;
			nextRecordSyncTime = Util::getTime () + uiconfig->recordSyncDelayDuration;
			break;
		}
	}
}

void MonitorCacheUi::thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	App *app;
	UiText *uitext;
	Menu *action;
	bool show;

	ui = (MonitorCacheUi *) uiPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == widgetPtr)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = (Menu *) app->rootPanel->addWidget (new Menu ());
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->addItem (uitext->getText (UiTextString::small).capitalized (), ui->sprites.getSprite (MonitorCacheUi::SMALL_THUMBNAIL_BUTTON), MonitorCacheUi::smallThumbnailActionClicked, ui, 0, ui->cardLayout == StreamWindow::LOW_DETAIL);
	action->addItem (uitext->getText (UiTextString::medium).capitalized (), ui->sprites.getSprite (MonitorCacheUi::MEDIUM_THUMBNAIL_BUTTON), MonitorCacheUi::mediumThumbnailActionClicked, ui, 0, ui->cardLayout == StreamWindow::MEDIUM_DETAIL);
	action->addItem (uitext->getText (UiTextString::large).capitalized (), ui->sprites.getSprite (MonitorCacheUi::LARGE_THUMBNAIL_BUTTON), MonitorCacheUi::largeThumbnailActionClicked, ui, 0, ui->cardLayout == StreamWindow::HIGH_DETAIL);
	action->position.assign (widgetPtr->position.x + widgetPtr->width - action->width, widgetPtr->position.y + widgetPtr->height);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (widgetPtr);
}

void MonitorCacheUi::smallThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MonitorCacheUi *) uiPtr;
	if (ui->cardLayout == StreamWindow::LOW_DETAIL) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = StreamWindow::LOW_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->smallThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MonitorCacheUi::resetStreamWindowLayout, ui);
	ui->cardView->setRowItemMarginSize (1, 0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.1f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MonitorCacheUi::mediumThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MonitorCacheUi *) uiPtr;
	if (ui->cardLayout == StreamWindow::MEDIUM_DETAIL) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = StreamWindow::MEDIUM_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->mediumThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MonitorCacheUi::resetStreamWindowLayout, ui);
	ui->cardView->setRowItemMarginSize (1, App::getInstance ()->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.2f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MonitorCacheUi::largeThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MonitorCacheUi *) uiPtr;
	if (ui->cardLayout == StreamWindow::HIGH_DETAIL) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = StreamWindow::HIGH_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->largeThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MonitorCacheUi::resetStreamWindowLayout, ui);
	ui->cardView->setRowItemMarginSize (1, App::getInstance ()->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.3f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MonitorCacheUi::resetStreamWindowLayout (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	StreamWindow *window;

	ui = (MonitorCacheUi *) uiPtr;
	window = StreamWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	window->setLayout (ui->cardLayout, ui->cardMaxImageWidth);
}

void MonitorCacheUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	App *app;
	Json *params;

	ui = (MonitorCacheUi *) uiPtr;
	app = App::getInstance ();
	app->agentControl.refreshAgentStatus (ui->agentId);

	params = new Json ();
	params->set ("maxResults", 0);
	app->agentControl.writeLinkCommand (app->createCommand ("FindItems", SystemInterface::Constant_Monitor, params), ui->agentId);
}

void MonitorCacheUi::processStreamItem (void *uiPtr, Json *record, const StdString &recordId) {
	MonitorCacheUi *ui;
	App *app;
	SystemInterface *interface;
	StreamWindow *window;

	ui = (MonitorCacheUi *) uiPtr;
	app = App::getInstance ();
	interface = &(app->systemInterface);
	if (! ui->agentId.equals (interface->getCommandAgentId (record))) {
		return;
	}

	++(ui->streamCount);
	if (! ui->cardView->contains (recordId)) {
		window = new StreamWindow (record, ui->cardLayout, ui->cardMaxImageWidth);
		window->setStreamImageClickCallback (MonitorCacheUi::streamWindowImageClicked, ui);
		window->setViewButtonClickCallback (MonitorCacheUi::streamWindowViewButtonClicked, ui);
		window->sortKey.assign (window->streamName);
		ui->cardView->addItem (window, recordId, 1);
	}
}

void MonitorCacheUi::streamWindowImageClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	StreamWindow *target;

	ui = (MonitorCacheUi *) uiPtr;
	target = StreamWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	if (! target->isSelected) {
		ui->setSelectedStream (target, target->selectedTimestamp);
	}
}

void MonitorCacheUi::streamWindowViewButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	MonitorCacheUi *ui;
	StreamWindow *target;
	StreamItemUi *itemui;

	target = StreamWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	ui = (MonitorCacheUi *) uiPtr;
	app = App::getInstance ();

	ui->actionTarget.assign (target);
	itemui = new StreamItemUi (target, app->uiText.getText (UiTextString::selectPlayPosition).capitalized ());
	itemui->setThumbnailClickCallback (MonitorCacheUi::streamItemUiThumbnailClicked, ui);
	app->pushUi (itemui);
}

void MonitorCacheUi::streamItemUiThumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	App *app;
	ThumbnailWindow *thumbnail;
	StreamWindow *target;

	ui = (MonitorCacheUi *) uiPtr;
	app = App::getInstance ();
	thumbnail = ThumbnailWindow::castWidget (widgetPtr);
	target = StreamWindow::castWidget (ui->actionTarget.widget);
	if ((! thumbnail) || (! target)) {
		return;
	}

	target->setThumbnailIndex (thumbnail->thumbnailIndex);
	ui->setSelectedStream (target, thumbnail->thumbnailTimestamp);
	app->popUi ();
}

void MonitorCacheUi::setSelectedStream (StreamWindow *streamWindow, float timestamp) {
	selectedStreamWindow.assign (streamWindow);
	cardView->processRowItems (1, MonitorCacheUi::unselectStreamWindow, this);
	streamWindow->setSelected (true, timestamp);
}

void MonitorCacheUi::unselectStreamWindow (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	StreamWindow *window;

	ui = (MonitorCacheUi *) uiPtr;
	window = StreamWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	if (window != ui->selectedStreamWindow.widget) {
		window->setSelected (false);
	}
}

void MonitorCacheUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	Button *button;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	StreamWindow *streamwindow;
	StdString text;
	Color color;

	ui = (MonitorCacheUi *) uiPtr;
	button = (Button *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);
	Button::mouseEntered (button, button);

	ui->commandPopup.destroyAndClear ();
	ui->commandButton.assign (button);
	panel = (Panel *) app->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	panel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	panel->setBorder (true, uiconfig->darkBackgroundColor);
	panel->setAlphaBlend (true, uiconfig->overlayWindowAlpha);

	label = NULL;
	if (button == ui->stopButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::stop).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));
	}
	else if (button == ui->playButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::play).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		streamwindow = StreamWindow::castWidget (ui->selectedStreamWindow.widget);
		if (! streamwindow) {
			text.assign (uitext->getText (UiTextString::monitorUiNoStreamSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (streamwindow->streamName);
			Label::truncateText (&text, UiConfiguration::CAPTION, app->rootPanel->width * 0.20f, StdString ("..."));
			text.appendSprintf (" %s", Util::getDurationString (streamwindow->selectedTimestamp, Util::HOURS).c_str ());
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (MonitorCacheUi::SMALL_STREAM_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->writePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::playAll).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));
	}
	else if (button == ui->deleteButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::removeStream).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		streamwindow = StreamWindow::castWidget (ui->selectedStreamWindow.widget);
		if (! streamwindow) {
			text.assign (uitext->getText (UiTextString::monitorUiNoStreamSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (streamwindow->streamName);
			Label::truncateText (&text, UiConfiguration::CAPTION, app->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (MonitorCacheUi::SMALL_STREAM_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}

	if (label) {
		label->setPadding (0.0f, 0.0f);
	}
	panel->setLayout (Panel::VERTICAL_RIGHT_JUSTIFIED);
	panel->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	panel->position.assign (app->windowWidth - panel->width - uiconfig->paddingSize, app->windowHeight - app->bottomBarHeight - panel->height - uiconfig->marginSize);
}

void MonitorCacheUi::commandButtonMouseExited (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	Button *button;

	ui = (MonitorCacheUi *) uiPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);

	if (ui->commandButton.widget == button) {
		ui->commandPopup.destroyAndClear ();
		ui->commandButton.clear ();
	}
}

void MonitorCacheUi::stopButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	App *app;
	int result;

	ui = (MonitorCacheUi *) uiPtr;
	app = App::getInstance ();

	result = app->agentControl.invokeCommand (ui->agentId, app->createCommand ("ClearDisplay", SystemInterface::Constant_Monitor));
	if (result != Result::SUCCESS) {
		Log::write (Log::DEBUG, "Failed to invoke ClearDisplay command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
	}
	else {
		app->showSnackbar (app->uiText.getText (UiTextString::invokeClearDisplayMessage));
	}

	app->agentControl.refreshAgentStatus (ui->agentId);
}

void MonitorCacheUi::playButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	App *app;
	StreamWindow *streamwindow;
	Json *params;
	int result;

	ui = (MonitorCacheUi *) uiPtr;
	app = App::getInstance ();
	streamwindow = StreamWindow::castWidget (ui->selectedStreamWindow.widget);
	if (! streamwindow) {
		return;
	}

	params = new Json ();
	params->set ("streamId", streamwindow->streamId);
	params->set ("startPosition", streamwindow->selectedTimestamp / 1000.0f);
	result = app->agentControl.invokeCommand (ui->agentId, app->createCommand ("PlayCacheStream", SystemInterface::Constant_Monitor, params));
	if (result != Result::SUCCESS) {
		Log::write (Log::DEBUG, "Failed to invoke PlayCacheStream command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
	}
	else {
		app->showSnackbar (app->uiText.getText (UiTextString::invokePlayMediaMessage));
	}

	app->agentControl.refreshAgentStatus (ui->agentId);
}

void MonitorCacheUi::writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	App *app;
	std::list<Json *> items;
	Json *params;
	int result;

	ui = (MonitorCacheUi *) uiPtr;
	app = App::getInstance ();

	if (ui->streamCount <= 0) {
		return;
	}

	ui->cardView->processItems (MonitorCacheUi::appendPlaylistItem, &items);
	if (items.empty ()) {
		return;
	}

	params = new Json ();
	params->set ("items", &items);
	params->set ("displayName", "Loop");
	params->set ("isShuffle", true);
	params->set ("minStartPositionDelta", 0);
	params->set ("maxStartPositionDelta", 85);
	params->set ("minItemDisplayDuration", 30);
	params->set ("maxItemDisplayDuration", 360);

	result = app->agentControl.invokeCommand (ui->agentId, app->createCommand ("CreateMediaDisplayIntent", SystemInterface::Constant_Monitor, params));
	if (result != Result::SUCCESS) {
		Log::write (Log::DEBUG, "Failed to invoke CreateMediaDisplayIntent command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
	}
	else {
		app->showSnackbar (app->uiText.getText (UiTextString::monitorCacheUiWritePlaylistMessage));
	}

	app->agentControl.refreshAgentStatus (ui->agentId);
}

void MonitorCacheUi::appendPlaylistItem (void *jsonListPtr, Widget *widgetPtr) {
	StreamWindow *window;
	std::list<Json *> *items;
	Json *obj;

	items = (std::list<Json *> *) jsonListPtr;
	window = StreamWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	obj = new Json ();
	obj->set ("mediaName", window->streamName);
	obj->set ("streamId", window->streamId);

	items->push_back (obj);
}

void MonitorCacheUi::deleteButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;

	ui = (MonitorCacheUi *) uiPtr;
	ui->clearPopupWidgets ();
	ui->invokeRemoveMediaStream ();
}

void MonitorCacheUi::invokeRemoveMediaStream () {
	App *app;
	StreamWindow *target;
	Json *params;
	int result;

	app = App::getInstance ();
	target = StreamWindow::castWidget (selectedStreamWindow.widget);
	if (! target) {
		return;
	}

	params = new Json ();
	params->set ("id", target->streamId);

	retain ();
	result = app->agentControl.invokeCommand (target->agentId, app->createCommand ("RemoveStream", SystemInterface::Constant_Monitor, params), MonitorCacheUi::removeStreamComplete, this);
	if (result != Result::SUCCESS) {
		release ();
		Log::write (Log::DEBUG, "Failed to invoke RemoveStream command; err=%i agentId=\"%s\"", result, target->agentId.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
		return;
	}
}

void MonitorCacheUi::removeStreamComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	MonitorCacheUi *ui;
	App *app;
	SystemInterface *interface;
	UiText *uitext;
	StdString agentname, text, id;

	ui = (MonitorCacheUi *) uiPtr;

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
		app->agentControl.refreshAgentStatus (agentId);
	}
	else {
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
	}
	ui->release ();
}
