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
#include "Ui.h"
#include "SystemInterface.h"
#include "StringList.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "Json.h"
#include "Toggle.h"
#include "Button.h"
#include "Menu.h"
#include "ScrollView.h"
#include "CardView.h"
#include "TextFieldWindow.h"
#include "ActionWindow.h"
#include "HelpWindow.h"
#include "IconCardWindow.h"
#include "MonitorWindow.h"
#include "MediaLibraryWindow.h"
#include "StreamWindow.h"
#include "ThumbnailWindow.h"
#include "IconLabelWindow.h"
#include "StreamPlaylistWindow.h"
#include "StreamItemUi.h"
#include "MonitorUi.h"

const float MonitorUi::smallImageScale = 0.123f;
const float MonitorUi::mediumImageScale = 0.240f;
const float MonitorUi::largeImageScale = 0.480f;
const int MonitorUi::pageSize = 64;

MonitorUi::MonitorUi ()
: Ui ()
, cardView (NULL)
, searchField (NULL)
, searchButton (NULL)
, stopButton (NULL)
, playButton (NULL)
, createPlaylistButton (NULL)
, writePlaylistButton (NULL)
, addPlaylistItemButton (NULL)
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
, recordReceiveCount (0)
, nextRecordSyncTime (0)
, monitorCount (0)
, streamServerCount (0)
, streamCount (0)
, findStreamsComplete (false)
{

}

MonitorUi::~MonitorUi () {

}

StdString MonitorUi::getSpritePath () {
	return (StdString ("ui/MonitorUi/sprite"));
}

StdString MonitorUi::getBreadcrumbTitle () {
	return (App::getInstance ()->uiText.getText (UiTextString::monitorUiTitle));
}

Sprite *MonitorUi::getBreadcrumbSprite () {
	return (sprites.getSprite (MonitorUi::BREADCRUMB_ICON)->copy ());
}

void MonitorUi::setHelpWindowContent (Widget *helpWindowPtr) {
	App *app;
	HelpWindow *help;
	UiText *uitext;

	help = (HelpWindow *) helpWindowPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);

	help->setHelpText (uitext->getText (UiTextString::monitorUiHelpTitle), uitext->getText (UiTextString::monitorUiHelpText));
	if (monitorCount <= 0) {
		help->addAction (uitext->getText (UiTextString::monitorUiHelpAction1Text), uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("servers"));
	}
	if (streamServerCount <= 0) {
		help->addAction (uitext->getText (UiTextString::monitorUiHelpAction2Text), uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("servers"));
	}

	if ((monitorCount > 0) && (streamServerCount > 0)) {
		if (streamCount <= 0) {
			help->addAction (uitext->getText (UiTextString::monitorUiHelpAction3Text), uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("media-streaming"));
		}
		else {
			help->addAction (uitext->getText (UiTextString::monitorUiHelpAction4Text));
			help->addAction (uitext->getText (UiTextString::monitorUiHelpAction5Text));
		}
	}

	help->addTopicLink (uitext->getText (UiTextString::searchForHelp), Util::getHelpUrl (""));
}

int MonitorUi::doLoad () {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	int layout;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	monitorCount = 0;
	streamServerCount = 0;
	streamCount = 0;
	findStreamsComplete = false;
	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (1, uitext->getText (UiTextString::playlists).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->setRowHeader (2, uitext->getText (UiTextString::videoStreams).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, app->topBarHeight);

	layout = app->prefsMap.find (App::prefsMonitorImageSize, (int) StreamWindow::LOW_DETAIL);
	switch (layout) {
		case StreamWindow::LOW_DETAIL: {
			MonitorUi::smallThumbnailActionClicked (this, NULL);
			break;
		}
		case StreamWindow::MEDIUM_DETAIL: {
			MonitorUi::mediumThumbnailActionClicked (this, NULL);
			break;
		}
		case StreamWindow::HIGH_DETAIL: {
			MonitorUi::largeThumbnailActionClicked (this, NULL);
			break;
		}
		default: {
			MonitorUi::smallThumbnailActionClicked (this, NULL);
			break;
		}
	}

	return (Result::SUCCESS);
}

void MonitorUi::doUnload () {
	selectedAgentMap.clear ();
	actionTarget.clear ();
	actionWidget.destroyAndClear ();
	emptyStateWindow.destroyAndClear ();
	selectedStreamWindow.clear ();
	selectedPlaylistWindow.clear ();
	commandPopup.destroyAndClear ();
	commandButton.clear ();
	streamServerResultOffsetMap.clear ();
	streamServerSetSizeMap.clear ();
	streamServerRecordCountMap.clear ();
}

void MonitorUi::doResetMainToolbar (Toolbar *toolbar) {
	App *app;
	Button *button;

	app = App::getInstance ();

	button = new Button (StdString (""), sprites.getSprite (MonitorUi::THUMBNAIL_SIZE_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (MonitorUi::thumbnailSizeButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.getText (UiTextString::mediaUiThumbnailSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (StdString (""), app->uiConfig.coreSprites.getSprite (UiConfiguration::RELOAD_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (MonitorUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void MonitorUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	searchField = new TextFieldWindow (app->windowWidth * 0.5f, uitext->getText (UiTextString::enterSearchKeyPrompt));
	searchField->setPadding (uiconfig->paddingSize, 0.0f);
	searchField->setButtonsEnabled (false, false, true, true);
	searchField->setEditCallback (MonitorUi::searchFieldEdited, this);
	toolbar->addLeftItem (searchField);

	searchButton = new Button (StdString (""), sprites.getSprite (MonitorUi::SEARCH_BUTTON));
	searchButton->setInverseColor (true);
	searchButton->setMouseClickCallback (MonitorUi::searchButtonClicked, this);
	searchButton->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiSearchTooltip));
	toolbar->addLeftItem (searchButton);

	playButton = new Button (StdString (""), sprites.getSprite (MonitorUi::PLAY_BUTTON));
	playButton->setInverseColor (true);
	playButton->setMouseClickCallback (MonitorUi::playButtonClicked, this);
	playButton->setMouseEnterCallback (MonitorUi::commandButtonMouseEntered, this);
	playButton->setMouseExitCallback (MonitorUi::commandButtonMouseExited, this);
	playButton->shortcutKey = SDLK_F5;
	playButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorUiPlayTooltip), Widget::LEFT);
	toolbar->addRightItem (playButton);

	writePlaylistButton = new Button (StdString (""), sprites.getSprite (MonitorUi::WRITE_PLAYLIST_BUTTON));
	writePlaylistButton->setInverseColor (true);
	writePlaylistButton->setMouseClickCallback (MonitorUi::writePlaylistButtonClicked, this);
	writePlaylistButton->setMouseEnterCallback (MonitorUi::commandButtonMouseEntered, this);
	writePlaylistButton->setMouseExitCallback (MonitorUi::commandButtonMouseExited, this);
	writePlaylistButton->shortcutKey = SDLK_F4;
	writePlaylistButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorUiWritePlaylistTooltip), Widget::LEFT);
	toolbar->addRightItem (writePlaylistButton);

	stopButton = new Button (StdString (""), sprites.getSprite (MonitorUi::STOP_BUTTON));
	stopButton->setInverseColor (true);
	stopButton->setMouseClickCallback (MonitorUi::stopButtonClicked, this);
	stopButton->setMouseEnterCallback (MonitorUi::commandButtonMouseEntered, this);
	stopButton->setMouseExitCallback (MonitorUi::commandButtonMouseExited, this);
	stopButton->shortcutKey = SDLK_F3;
	stopButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorUiStopTooltip), Widget::LEFT);
	toolbar->addRightItem (stopButton);

	toolbar->addRightSpacer ();

	addPlaylistItemButton = new Button (StdString (""), sprites.getSprite (MonitorUi::ADD_PLAYLIST_ITEM_BUTTON));
	addPlaylistItemButton->setInverseColor (true);
	addPlaylistItemButton->setMouseClickCallback (MonitorUi::addPlaylistItemButtonClicked, this);
	addPlaylistItemButton->setMouseEnterCallback (MonitorUi::commandButtonMouseEntered, this);
	addPlaylistItemButton->setMouseExitCallback (MonitorUi::commandButtonMouseExited, this);
	addPlaylistItemButton->shortcutKey = SDLK_F2;
	addPlaylistItemButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorUiAddPlaylistItemTooltip), Widget::LEFT);
	toolbar->addRightItem (addPlaylistItemButton);

	createPlaylistButton = new Button (StdString (""), sprites.getSprite (MonitorUi::CREATE_PLAYLIST_BUTTON));
	createPlaylistButton->setInverseColor (true);
	createPlaylistButton->setMouseClickCallback (MonitorUi::createPlaylistButtonClicked, this);
	createPlaylistButton->setMouseEnterCallback (MonitorUi::commandButtonMouseEntered, this);
	createPlaylistButton->setMouseExitCallback (MonitorUi::commandButtonMouseExited, this);
	createPlaylistButton->shortcutKey = SDLK_F1;
	createPlaylistButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorUiCreatePlaylistTooltip), Widget::LEFT);
	toolbar->addRightItem (createPlaylistButton);

	toolbar->isVisible = true;
}

void MonitorUi::doClearPopupWidgets () {
	actionWidget.destroyAndClear ();
	commandPopup.destroyAndClear ();
	commandButton.clear ();
}

void MonitorUi::doResume () {
	App *app;
	StringList items;
	StringList::iterator i, end;
	StdString id;
	StreamPlaylistWindow *window;
	Json *obj;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/MonitorUi/bg");
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
	actionTarget.clear ();

	searchField->setValue (searchKey);

	if (! isFirstResumeComplete) {
		app->prefsMap.find (App::prefsMonitorUiPlaylists, &items);
		i = items.begin ();
		end = items.end ();
		while (i != end) {
			obj = new Json ();
			if (obj->parse (*i) == Result::SUCCESS) {
				window = createStreamPlaylistWindow ();
				window->setState (obj);

				id = cardView->getAvailableItemId ();
				window->itemId.assign (id);
				window->sortKey.assign (window->playlistName.lowercased ());
				cardView->addItem (window, id, 1);
				if (window->isSelected) {
					selectedPlaylistWindow.assign (window);
				}
			}
			delete (obj);
			++i;
		}
	}
}

void MonitorUi::doRefresh () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MonitorUi::doPause () {
	App *app;
	StringList items;

	app = App::getInstance ();
	app->prefsMap.insert (App::prefsMonitorImageSize, cardLayout);

	cardView->processItems (MonitorUi::appendPlaylistJson, &items);
	if (items.empty ()) {
		app->prefsMap.remove (App::prefsMonitorUiPlaylists);
	}
	else {
		app->prefsMap.insert (App::prefsMonitorUiPlaylists, &items);
	}
}

void MonitorUi::appendPlaylistJson (void *stringListPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	Json *json;

	window = StreamPlaylistWindow::castWidget (widgetPtr);
	if (window) {
		json = window->getState ();
		((StringList *) stringListPtr)->push_back (json->toString ());
		delete (json);
	}
}

void MonitorUi::doUpdate (int msElapsed) {
	App *app;
	StringList idlist;
	StringList::iterator i, end;
	Json *params;
	int offset, setsize, recordcount;
	int64_t now;

	app = App::getInstance ();
	actionWidget.compact ();
	actionTarget.compact ();
	emptyStateWindow.compact ();
	selectedStreamWindow.compact ();
	selectedPlaylistWindow.compact ();
	commandPopup.compact ();

	if (recordReceiveCount > 0) {
		now = Util::getTime ();
		if ((recordReceiveCount > 16) || (now >= nextRecordSyncTime)) {
			app->shouldSyncRecordStore = true;
			recordReceiveCount = 0;
		}
	}
	else {
		if (cardView->isScrolledToBottom ()) {
			streamServerResultOffsetMap.getKeys (&idlist, true);
			i = idlist.begin ();
			end = idlist.end ();
			while (i != end) {
				offset = streamServerResultOffsetMap.find (*i, 0);
				setsize = streamServerSetSizeMap.find (*i, 0);
				recordcount = streamServerRecordCountMap.find (*i, 0);
				if ((recordcount >= offset) && (recordcount < setsize)) {
					params = new Json ();
					params->set ("searchKey", streamSearchKey);
					params->set ("resultOffset", offset);
					params->set ("maxResults", MonitorUi::pageSize);
					app->agentControl.writeLinkCommand (app->createCommandJson ("FindItems", SystemInterface::Constant_Stream, params), *i);
					offset += MonitorUi::pageSize;
					streamServerResultOffsetMap.insert (*i, offset);
				}
				++i;
			}
		}
	}
}

void MonitorUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void MonitorUi::doSyncRecordStore (RecordStore *store) {
	IconCardWindow *window;
	UiConfiguration *uiconfig;
	UiText *uitext;

	monitorCount = 0;
	store->processAgentRecords ("monitorServerStatus", MonitorUi::processMonitorAgentStatus, this);

	streamServerCount = 0;
	store->processAgentRecords ("streamServerStatus", MonitorUi::processStreamServerAgentStatus, this);

	streamCount = 0;
	store->processCommandRecords (SystemInterface::Command_StreamItem, MonitorUi::processStreamItem, this);

	window = (IconCardWindow *) emptyStateWindow.widget;
	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	if ((monitorCount + streamServerCount) <= 0) {
		if (! window) {
			window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SERVER_ICON), uitext->getText (UiTextString::monitorUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::monitorUiEmptyAgentStatusText));
			window->setLink (uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("servers"));
			emptyStateWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, 0);
		}
	}
	else if ((streamServerCount <= 0) || ((streamCount <= 0) && findStreamsComplete)) {
		if (! window) {
			window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::STREAM_ICON), uitext->getText (UiTextString::monitorUiEmptyStreamStatusTitle), StdString (""), uitext->getText (UiTextString::monitorUiEmptyStreamStatusText));
			window->setLink (uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("media-streaming"));
			emptyStateWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, 2);
		}
	}
	else {
		if (window) {
			cardView->removeItem (window->itemId);
			emptyStateWindow.clear ();
		}
	}

	cardView->syncRecordStore (store);
	cardView->refresh ();
}

StreamPlaylistWindow *MonitorUi::createStreamPlaylistWindow () {
	StreamPlaylistWindow *window;

	window = new StreamPlaylistWindow ();
	window->setSelectStateChangeCallback (MonitorUi::playlistSelectStateChanged, this);
	window->setExpandStateChangeCallback (MonitorUi::playlistExpandStateChanged, this);
	window->setListChangeCallback (MonitorUi::playlistItemsChanged, this);
	window->setRenameClickCallback (MonitorUi::renamePlaylistActionClicked, this);
	window->setMenuClickCallback (MonitorUi::playlistMenuClicked, this);

	return (window);
}

StdString MonitorUi::getSelectedAgentNames (float maxWidth) {
	StdString text, id;
	HashMap::Iterator i;
	StringList names;
	int count;

	count = selectedAgentMap.size ();
	if (count <= 0) {
		return (StdString (""));
	}

	i = selectedAgentMap.begin ();
	while (selectedAgentMap.hasNext (&i)) {
		id = selectedAgentMap.next (&i);
		names.push_back (selectedAgentMap.find (id, ""));
	}

	names.sort (StringList::compareCaseInsensitiveAscending);
	text.assign (names.join (", "));
	Label::truncateText (&text, UiConfiguration::CAPTION, maxWidth, StdString::createSprintf ("... (%i)", count));

	return (text);
}

void MonitorUi::processMonitorAgentStatus (void *uiPtr, Json *record, const StdString &recordId) {
	MonitorUi *ui;
	SystemInterface *interface;
	MonitorWindow *window;

	ui = (MonitorUi *) uiPtr;
	interface = &(App::getInstance ()->systemInterface);
	++(ui->monitorCount);
	if (! ui->cardView->contains (recordId)) {
		window = new MonitorWindow (recordId);
		window->setSelectStateChangeCallback (MonitorUi::monitorAgentSelectStateChanged, ui);
		window->sortKey.assign (interface->getCommandAgentName (record).lowercased ());
		ui->cardView->addItem (window, recordId, 0);
	}
}

void MonitorUi::processStreamServerAgentStatus (void *uiPtr, Json *record, const StdString &recordId) {
	MonitorUi *ui;
	SystemInterface *interface;
	MediaLibraryWindow *window;

	ui = (MonitorUi *) uiPtr;
	interface = &(App::getInstance ()->systemInterface);
	++(ui->streamServerCount);
	if (! ui->cardView->contains (recordId)) {
		window = new MediaLibraryWindow (recordId);
		window->sortKey.assign (interface->getCommandAgentName (record).lowercased ());
		ui->cardView->addItem (window, recordId, 0);
		ui->addLinkAgent (recordId);
	}
}

void MonitorUi::processStreamItem (void *uiPtr, Json *record, const StdString &recordId) {
	MonitorUi *ui;
	StreamWindow *window;
	SystemInterface *interface;
	StdString agentid;

	ui = (MonitorUi *) uiPtr;
	++(ui->streamCount);
	if (! ui->cardView->contains (recordId)) {
		interface = &(App::getInstance ()->systemInterface);
		agentid = interface->getCommandAgentId (record);
		if (ui->streamServerResultOffsetMap.exists (agentid)) {
			window = new StreamWindow (record, &(ui->sprites), ui->cardLayout, ui->cardMaxImageWidth);
			window->setStreamImageClickCallback (MonitorUi::streamWindowImageClicked, ui);
			window->sortKey.assign (window->streamName);
			ui->cardView->addItem (window, recordId, 2);
		}
	}
}

void MonitorUi::monitorAgentSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	MonitorWindow *window;

	ui = (MonitorUi *) uiPtr;
	window = (MonitorWindow *) widgetPtr;
	if (window->isSelected) {
		ui->selectedAgentMap.insert (window->agentId, window->agentName);
	}
	else {
		ui->selectedAgentMap.remove (window->agentId);
	}
}

void MonitorUi::handleLinkClientConnect (const StdString &agentId) {
	App *app;
	Json *params;

	app = App::getInstance ();
	if (! streamServerResultOffsetMap.exists (agentId)) {
		streamServerResultOffsetMap.insert (agentId, MonitorUi::pageSize);
		streamServerSetSizeMap.insert (agentId, (int) 0);
		streamServerRecordCountMap.insert (agentId, (int) 0);
		params = new Json ();
		params->set ("searchKey", streamSearchKey);
		params->set ("maxResults", MonitorUi::pageSize);
		app->agentControl.writeLinkCommand (app->createCommandJson ("FindItems", SystemInterface::Constant_Stream, params), agentId);
	}
}

void MonitorUi::handleLinkClientCommand (const StdString &agentId, int commandId, Json *command) {
	App *app;
	SystemInterface *interface;
	UiConfiguration *uiconfig;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	interface = &(app->systemInterface);

	switch (commandId) {
		case SystemInterface::Command_FindStreamsResult: {
			streamServerSetSizeMap.insert (agentId, interface->getCommandNumberParam (command, "setSize", (int) 0));
			findStreamsComplete = true;
			break;
		}
		case SystemInterface::Command_StreamItem: {
			app->agentControl.recordStore.addRecord (command);
			streamServerRecordCountMap.insert (agentId, streamServerRecordCountMap.find (agentId, 0) + 1);
			++recordReceiveCount;
			nextRecordSyncTime = Util::getTime () + uiconfig->recordSyncDelayDuration;
			break;
		}
	}
}

void MonitorUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;

	ui = (MonitorUi *) uiPtr;
	ui->cardView->processRowItems (0, MonitorUi::reloadAgent, ui);
}

void MonitorUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	App *app;
	MonitorWindow *monitorwindow;
	MediaLibraryWindow *medialibrarywindow;
	StdString id;

	app = App::getInstance ();
	monitorwindow = MonitorWindow::castWidget (widgetPtr);
	if (monitorwindow) {
		id.assign (monitorwindow->agentId);
	}

	if (id.empty ()) {
		medialibrarywindow = MediaLibraryWindow::castWidget (widgetPtr);
		if (medialibrarywindow) {
			id.assign (medialibrarywindow->agentId);
		}
	}

	if (! id.empty ()) {
		app->agentControl.refreshAgentStatus (id);
	}
}

void MonitorUi::thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	UiText *uitext;
	Menu *action;
	bool show;

	ui = (MonitorUi *) uiPtr;
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
	action->addItem (uitext->getText (UiTextString::small).capitalized (), ui->sprites.getSprite (MonitorUi::SMALL_THUMBNAIL_BUTTON), MonitorUi::smallThumbnailActionClicked, ui, 0, ui->cardLayout == StreamWindow::LOW_DETAIL);
	action->addItem (uitext->getText (UiTextString::medium).capitalized (), ui->sprites.getSprite (MonitorUi::MEDIUM_THUMBNAIL_BUTTON), MonitorUi::mediumThumbnailActionClicked, ui, 0, ui->cardLayout == StreamWindow::MEDIUM_DETAIL);
	action->addItem (uitext->getText (UiTextString::large).capitalized (), ui->sprites.getSprite (MonitorUi::LARGE_THUMBNAIL_BUTTON), MonitorUi::largeThumbnailActionClicked, ui, 0, ui->cardLayout == StreamWindow::HIGH_DETAIL);
	action->position.assign (widgetPtr->position.x + widgetPtr->width - action->width, widgetPtr->position.y + widgetPtr->height);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (widgetPtr);
}

void MonitorUi::smallThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	float y0, h0;

	ui = (MonitorUi *) uiPtr;
	if (ui->cardLayout == StreamWindow::LOW_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = StreamWindow::LOW_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * MonitorUi::smallImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MonitorUi::resetStreamWindowLayout, ui);
	ui->cardView->setRowItemMarginSize (2, 0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.1f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MonitorUi::mediumThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	float y0, h0;

	ui = (MonitorUi *) uiPtr;
	if (ui->cardLayout == StreamWindow::MEDIUM_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = StreamWindow::MEDIUM_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * MonitorUi::mediumImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MonitorUi::resetStreamWindowLayout, ui);
	ui->cardView->setRowItemMarginSize (2, App::getInstance ()->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.2f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MonitorUi::largeThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	float y0, h0;

	ui = (MonitorUi *) uiPtr;
	if (ui->cardLayout == StreamWindow::HIGH_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = StreamWindow::HIGH_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * MonitorUi::largeImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MonitorUi::resetStreamWindowLayout, ui);
	ui->cardView->setRowItemMarginSize (2, App::getInstance ()->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.3f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MonitorUi::resetStreamWindowLayout (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	StreamWindow *window;

	ui = (MonitorUi *) uiPtr;
	window = StreamWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	window->setLayout (ui->cardLayout, ui->cardMaxImageWidth);
}

void MonitorUi::streamWindowImageClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	MonitorUi *ui;
	StreamWindow *target;
	StreamItemUi *itemui;

	target = StreamWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	ui = (MonitorUi *) uiPtr;
	app = App::getInstance ();

	ui->actionTarget.assign (target);
	itemui = new StreamItemUi (target, app->uiText.getText (UiTextString::selectPlayPosition).capitalized ());
	itemui->setThumbnailClickCallback (MonitorUi::streamUiThumbnailClicked, ui);
	app->pushUi (itemui);
}

void MonitorUi::streamUiThumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	ThumbnailWindow *thumbnail;
	StreamWindow *target;

	ui = (MonitorUi *) uiPtr;
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

void MonitorUi::setSelectedStream (StreamWindow *streamWindow, float timestamp) {
	selectedStreamWindow.assign (streamWindow);
	cardView->processRowItems (2, MonitorUi::unselectStreamWindow, this);
	streamWindow->setSelected (true, timestamp);
}

void MonitorUi::unselectStreamWindow (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	StreamWindow *window;

	ui = (MonitorUi *) uiPtr;
	window = StreamWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	if (window != ui->selectedStreamWindow.widget) {
		window->setSelected (false);
	}
}

void MonitorUi::searchFieldEdited (void *uiPtr, Widget *widgetPtr) {
	MonitorUi::searchButtonClicked (uiPtr, NULL);
}

void MonitorUi::searchButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	StringList idlist;
	StringList::iterator i, end;
	Json *params;

	ui = (MonitorUi *) uiPtr;
	app = App::getInstance ();

	if (ui->searchKey.equals (ui->searchField->getValue ())) {
		return;
	}

	ui->searchKey.assign (ui->searchField->getValue ());
	ui->streamServerResultOffsetMap.getKeys (&idlist, true);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		ui->streamServerResultOffsetMap.insert (*i, MonitorUi::pageSize);
		ui->streamServerSetSizeMap.insert (*i, (int) 0);
		ui->streamServerRecordCountMap.insert (*i, (int) 0);
		++i;
	}
	app->agentControl.recordStore.removeRecords (SystemInterface::Command_StreamItem);
	ui->cardView->removeRowItems (2);

	params = new Json ();
	params->set ("searchKey", ui->searchKey);
	params->set ("maxResults", MonitorUi::pageSize);
	app->agentControl.writeLinkCommand (app->createCommandJson ("FindItems", SystemInterface::Constant_Stream, params));
}

void MonitorUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	Button *button;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	StreamWindow *streamwindow;
	StreamPlaylistWindow *playlistwindow;
	StdString text;
	Color color;

	ui = (MonitorUi *) uiPtr;
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

		text.assign (ui->getSelectedAgentNames (app->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::monitorUiNoAgentsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (MonitorUi::SMALL_MONITOR_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->playButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::play).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (app->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::monitorUiNoAgentsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (MonitorUi::SMALL_MONITOR_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

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
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (MonitorUi::SMALL_STREAM_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->createPlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::createPlaylist).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));
	}
	else if (button == ui->writePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::runPlaylist).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (app->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::monitorUiNoAgentsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (MonitorUi::SMALL_MONITOR_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		playlistwindow = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
		if (! playlistwindow) {
			text.assign (uitext->getText (UiTextString::monitorUiNoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (playlistwindow->playlistName);
			Label::truncateText (&text, UiConfiguration::CAPTION, app->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (MonitorUi::SMALL_PROGRAM_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->addPlaylistItemButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::addPlaylistItem).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		playlistwindow = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
		if (! playlistwindow) {
			text.assign (uitext->getText (UiTextString::monitorUiNoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (playlistwindow->playlistName);
			Label::truncateText (&text, UiConfiguration::CAPTION, app->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (MonitorUi::SMALL_PROGRAM_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

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
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (MonitorUi::SMALL_STREAM_ICON), text, UiConfiguration::CAPTION, color));
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

void MonitorUi::commandButtonMouseExited (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	Button *button;

	ui = (MonitorUi *) uiPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);

	if (ui->commandButton.widget == button) {
		ui->commandPopup.destroyAndClear ();
		ui->commandButton.clear ();
	}
}

void MonitorUi::stopButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	HashMap::Iterator i;
	StdString id;
	int result, count;

	ui = (MonitorUi *) uiPtr;
	app = App::getInstance ();
	if (ui->selectedAgentMap.empty ()) {
		return;
	}

	count = 0;
	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);
		result = app->agentControl.invokeCommand (id, app->createCommand ("ClearDisplay", SystemInterface::Constant_Monitor), NULL, NULL, NULL, id);
		if (result != Result::SUCCESS) {
			Log::write (Log::DEBUG, "Failed to invoke ClearDisplay command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
		else {
			++count;
			app->agentControl.refreshAgentStatus (id, id);
		}
	}

	if (count <= 0) {
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
	}
	else {
		app->showSnackbar (app->uiText.getText (UiTextString::invokeClearDisplayMessage));
	}
}

void MonitorUi::playButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	HashMap::Iterator i;
	StdString id, streamurl;
	StreamWindow *streamwindow;
	Json *params;
	int result, count;

	ui = (MonitorUi *) uiPtr;
	app = App::getInstance ();
	streamwindow = StreamWindow::castWidget (ui->selectedStreamWindow.widget);
	if ((! streamwindow) || ui->selectedAgentMap.empty ()) {
		return;
	}

	params = new Json ();
	params->set ("streamId", streamwindow->streamId);
	params->set ("startPosition", streamwindow->selectedTimestamp / 1000.0f);
	streamurl = app->agentControl.getAgentSecondaryUrl (streamwindow->agentId, app->createCommand ("GetHlsManifest", SystemInterface::Constant_Stream, params), streamwindow->hlsStreamPath);

	count = 0;
	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);

		params = new Json ();
		params->set ("mediaName", streamwindow->streamName);
		params->set ("streamUrl", streamurl);
		result = app->agentControl.invokeCommand (id, app->createCommand ("PlayMedia", SystemInterface::Constant_Monitor, params), NULL, NULL, NULL, id);
		if (result != Result::SUCCESS) {
			Log::write (Log::DEBUG, "Failed to invoke PlayMedia command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
		else {
			++count;
			app->agentControl.refreshAgentStatus (id, id);
		}
	}

	if (count <= 0) {
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
	}
	else {
		app->showSnackbar (app->uiText.getText (UiTextString::invokePlayMediaMessage));
	}
}

void MonitorUi::createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	StreamPlaylistWindow *window;
	StdString id;

	ui = (MonitorUi *) uiPtr;
	app = App::getInstance ();

	window = ui->createStreamPlaylistWindow ();
	id = ui->cardView->getAvailableItemId ();
	window->itemId.assign (id);
	window->setPlaylistName (ui->getAvailablePlaylistName ());
	window->setExpanded (true);
	ui->cardView->addItem (window, id, 1);
	window->setSelected (true);
	ui->cardView->scrollToItem (id);
	app->showSnackbar (StdString::createSprintf ("%s: %s", app->uiText.getText (UiTextString::streamPlaylistCreatedMessage).c_str (), window->playlistName.c_str ()));
}

void MonitorUi::writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	StreamPlaylistWindow *playlistwindow;
	HashMap::Iterator i;
	StdString id;
	int result, count;

	ui = (MonitorUi *) uiPtr;
	app = App::getInstance ();
	playlistwindow = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
	if (! playlistwindow) {
		return;
	}

	count = 0;
	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);
		result = app->agentControl.invokeCommand (id, playlistwindow->getCreateCommand (), NULL, NULL, NULL, id);
		if (result != Result::SUCCESS) {
			Log::write (Log::DEBUG, "Failed to invoke StreamPlaylistWindow create command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
		else {
			++count;
			app->agentControl.refreshAgentStatus (id, id);
		}
	}

	if (count <= 0) {
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
	}
	else {
		app->showSnackbar (app->uiText.getText (UiTextString::invokeCreateStreamPlaylistMessage));
	}
}

void MonitorUi::addPlaylistItemButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	StreamWindow *streamwindow;
	StreamPlaylistWindow *playlistwindow;
	StdString streamurl;

	ui = (MonitorUi *) uiPtr;
	app = App::getInstance ();
	streamwindow = StreamWindow::castWidget (ui->selectedStreamWindow.widget);
	playlistwindow = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
	if ((! streamwindow) || (! playlistwindow)) {
		return;
	}

	streamurl = app->agentControl.getAgentSecondaryUrl (streamwindow->agentId, NULL, streamwindow->hlsStreamPath);
	playlistwindow->addItem (streamurl, streamwindow->streamId, streamwindow->streamName, streamwindow->selectedTimestamp);
	ui->cardView->refresh ();
	app->showSnackbar (StdString::createSprintf ("%s: %s", app->uiText.getText (UiTextString::streamItemAddedMessage).c_str (), playlistwindow->playlistName.c_str ()));
}

StdString MonitorUi::getAvailablePlaylistName () {
	StdString basename, name;
	int i;

	basename.assign (App::getInstance ()->uiText.getText (UiTextString::playlist).capitalized ());
	name.assign (basename);
	cardView->processRowItems (1, MonitorUi::matchPlaylistName, &name);
	if (name.empty ()) {
		i = 2;
		while (true) {
			name.sprintf ("%s %i", basename.c_str (), i);
			cardView->processRowItems (1, MonitorUi::matchPlaylistName, &name);
			if (! name.empty ()) {
				break;
			}
			++i;
		}
	}

	return (name);
}

// Clear the provided string if the widget is of the correct type and matches its content by name
void MonitorUi::matchPlaylistName (void *stringPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	StdString *name;

	window = StreamPlaylistWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	name = (StdString *) stringPtr;
	if (name->lowercased ().equals (window->playlistName.lowercased ())) {
		name->assign ("");
	}
}

void MonitorUi::playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	StreamPlaylistWindow *window, *item;

	ui = (MonitorUi *) uiPtr;
	window = (StreamPlaylistWindow *) widgetPtr;
	if (window->isSelected) {
		if (ui->selectedPlaylistWindow.widget != window) {
			if (ui->selectedPlaylistWindow.widget) {
				item = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
				if (item) {
					item->setSelected (false, true);
				}
			}
			ui->selectedPlaylistWindow.assign (window);
		}
	}
	else {
		ui->selectedPlaylistWindow.clear ();
	}
}

void MonitorUi::playlistExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;

	ui = (MonitorUi *) uiPtr;
	ui->cardView->refresh ();
}

void MonitorUi::playlistItemsChanged (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;

	ui = (MonitorUi *) uiPtr;
	ui->cardView->refresh ();
}

void MonitorUi::playlistMenuClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StreamPlaylistWindow *window;
	Menu *action;
	bool show;

	ui = (MonitorUi *) uiPtr;
	window = (StreamPlaylistWindow *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == window)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = (Menu *) app->rootPanel->addWidget (new Menu ());
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (window);

	action->addItem (uitext->getText (UiTextString::rename).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::RENAME_BUTTON), MonitorUi::renamePlaylistActionClicked, ui);
	action->addItem (uitext->getText (UiTextString::remove).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON), MonitorUi::removePlaylistActionClicked, ui);

	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->position.assign (window->drawX + window->menuPositionX, window->drawY + window->menuPositionY);
}

void MonitorUi::renamePlaylistActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StreamPlaylistWindow *target;
	TextFieldWindow *action;

	ui = (MonitorUi *) uiPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);
	target = StreamPlaylistWindow::castWidget (widgetPtr);
	if (! target) {
		target = StreamPlaylistWindow::castWidget (ui->actionTarget.widget);
	}
	if (! target) {
		return;
	}

	ui->clearPopupWidgets ();
	action = (TextFieldWindow *) app->rootPanel->addWidget (new TextFieldWindow (target->width - (uiconfig->marginSize * 2.0f), uitext->getText (UiTextString::enterPlaylistNamePrompt)));
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (target);
	action->setValue (target->playlistName);
	action->setEditCallback (MonitorUi::playlistNameEdited, ui);
	action->setFillBg (true, uiconfig->lightPrimaryColor);
	action->setButtonsEnabled (true, true, true, true);
	action->setEditing (true);
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (target->drawX + uiconfig->marginSize, target->drawY);
}

void MonitorUi::playlistNameEdited (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	StreamPlaylistWindow *target;
	TextFieldWindow *action;
	StdString name;

	ui = (MonitorUi *) uiPtr;
	action = (TextFieldWindow *) ui->actionWidget.widget;
	target = (StreamPlaylistWindow *) ui->actionTarget.widget;
	if ((! action) || (! target)) {
		return;
	}

	name = action->getValue ();
	if (name.empty ()) {
		name.assign (ui->getAvailablePlaylistName ());
	}
	target->setPlaylistName (name);
	ui->clearPopupWidgets ();
}

void MonitorUi::removePlaylistActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorUi *ui;
	StreamPlaylistWindow *target;

	ui = (MonitorUi *) uiPtr;
	target = StreamPlaylistWindow::castWidget (ui->actionTarget.widget);
	if (target) {
		ui->cardView->removeItem (target->itemId);
	}
	ui->clearPopupWidgets ();
}
