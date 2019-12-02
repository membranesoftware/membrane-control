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
#include <map>
#include "SDL2/SDL.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "OsUtil.h"
#include "Button.h"
#include "Toggle.h"
#include "Menu.h"
#include "Chip.h"
#include "SystemInterface.h"
#include "CardView.h"
#include "ComboBox.h"
#include "MediaWindow.h"
#include "ActionWindow.h"
#include "HelpWindow.h"
#include "IconCardWindow.h"
#include "IconLabelWindow.h"
#include "TextFieldWindow.h"
#include "MediaLibraryWindow.h"
#include "MonitorWindow.h"
#include "MediaThumbnailWindow.h"
#include "StreamPlaylistWindow.h"
#include "Ui.h"
#include "MediaItemUi.h"
#include "StreamItemUi.h"
#include "MonitorCacheUi.h"
#include "MediaUi.h"

const int MediaUi::pageSize = 64;

MediaUi::MediaUi ()
: Ui ()
, toolbarMode (-1)
, cardView (NULL)
, playButton (NULL)
, writePlaylistButton (NULL)
, stopButton (NULL)
, browserPlayButton (NULL)
, configureStreamButton (NULL)
, cacheStreamButton (NULL)
, deleteStreamButton (NULL)
, addPlaylistItemButton (NULL)
, createPlaylistButton (NULL)
, deletePlaylistButton (NULL)
, cardDetail (-1)
, emptyStateType (-1)
, isShowingMediaWithoutStreams (true)
, mediaSortOrder (-1)
, mediaServerCount (0)
, mediaItemCount (0)
, mediaStreamCount (0)
, findMediaComplete (false)
, isLoadingMedia (false)
, recordReceiveCount (0)
, nextRecordSyncTime (0)
, mediaServerMapMutex (NULL)
, findMediaStreamsMapMutex (NULL)
{
	mediaServerMapMutex = SDL_CreateMutex ();
	findMediaStreamsMapMutex = SDL_CreateMutex ();
}

MediaUi::~MediaUi () {
	if (mediaServerMapMutex) {
		SDL_DestroyMutex (mediaServerMapMutex);
		mediaServerMapMutex = NULL;
	}

	if (findMediaStreamsMapMutex) {
		SDL_DestroyMutex (findMediaStreamsMapMutex);
		findMediaStreamsMapMutex = NULL;
	}
}

StdString MediaUi::getSpritePath () {
	return (StdString ("ui/MediaUi/sprite"));
}

Widget *MediaUi::createBreadcrumbWidget () {
	return (new Chip (App::instance->uiText.getText (UiTextString::media).capitalized (), sprites.getSprite (MediaUi::BreadcrumbIconSprite)));
}

void MediaUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::mediaUiHelpTitle), uitext->getText (UiTextString::mediaUiHelpText));
	if (mediaServerCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::mediaUiHelpAction1Text), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
	}
	else if (mediaItemCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::mediaUiHelpAction4Text), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("media-streaming"));
	}
	else {
		helpWindow->addAction (uitext->getText (UiTextString::mediaUiHelpAction2Text));
		helpWindow->addAction (uitext->getText (UiTextString::mediaUiHelpAction3Text), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("media-streaming"));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::mediaStreaming).capitalized (), App::getHelpUrl ("media-streaming"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

int MediaUi::doLoad () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	mediaServerCount = 0;
	mediaItemCount = 0;
	mediaStreamCount = 0;
	findMediaComplete = false;
	isLoadingMedia = false;
	isShowingMediaWithoutStreams = App::instance->prefsMap.find (App::MediaUiShowMediaWithoutStreamsKey, true);
	mediaSortOrder = App::instance->prefsMap.find (App::MediaUiSortOrderKey, (int) SystemInterface::Constant_NameSort);
	cardDetail = App::instance->prefsMap.find (App::MediaUiImageSizeKey, (int) CardView::MediumDetail);
	emptyStateType = -1;

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;

	panel = new Panel ();
	panel->setFillBg (true, uiconfig->mediumBackgroundColor);
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->setImageColor (uiconfig->flatButtonTextColor);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::minimizeAll).capitalized (), uitext->getText (UiTextString::expandAll).capitalized ());
	toggle->setStateChangeCallback (MediaUi::expandAgentsToggleStateChanged, this);
	expandAgentsToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::servers).capitalized (), panel), StdString (""), MediaUi::AgentToggleRow);

	cardView->setRowReverseSorted (MediaUi::ExpandedAgentRow, true);

	panel = new Panel ();
	panel->setFillBg (true, uiconfig->mediumBackgroundColor);
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->setImageColor (uiconfig->flatButtonTextColor);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::minimizeAll).capitalized (), uitext->getText (UiTextString::expandAll).capitalized ());
	toggle->setStateChangeCallback (MediaUi::expandPlaylistsToggleStateChanged, this);
	expandPlaylistsToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::playlists).capitalized (), panel), StdString (""), MediaUi::PlaylistToggleRow);

	cardView->setRowHeader (MediaUi::EmptyMediaRow, createRowHeaderPanel (uitext->getText (UiTextString::media).capitalized ()));
	cardView->setRowHeader (MediaUi::MediaRow, createRowHeaderPanel (uitext->getText (UiTextString::media).capitalized ()));
	cardView->setRowItemMarginSize (MediaUi::MediaRow, 0.0f);
	cardView->setRowSelectionAnimated (MediaUi::MediaRow, true);
	cardView->setRowDetail (MediaUi::MediaRow, cardDetail);
	cardView->setRowItemMarginSize (MediaUi::MediaLoadingRow, uiconfig->marginSize);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	return (Result::Success);
}

void MediaUi::doUnload () {
	searchField.clear ();
	emptyStateWindow.clear ();
	lastSelectedMediaWindow.clear ();
	selectedPlaylistWindow.clear ();
	selectedMonitorMap.clear ();
	selectedMediaMap.clear ();
	expandAgentsToggle.clear ();
	expandPlaylistsToggle.clear ();

	SDL_LockMutex (findMediaStreamsMapMutex);
	findMediaStreamsMap.clear ();
	SDL_UnlockMutex (findMediaStreamsMapMutex);

	SDL_LockMutex (mediaServerMapMutex);
	mediaServerMap.clear ();
	SDL_UnlockMutex (mediaServerMapMutex);
}

void MediaUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaUi::imageSizeButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);
}

void MediaUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	UiText *uitext;
	TextFieldWindow *textfield;
	Button *button;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	textfield = new TextFieldWindow (App::instance->windowWidth * 0.37f, uitext->getText (UiTextString::enterSearchKeyPrompt));
	textfield->setPadding (uiconfig->paddingSize, 0.0f);
	textfield->setButtonsEnabled (false, false, true, true);
	textfield->setEditCallback (MediaUi::searchFieldEdited, this);
	toolbar->addLeftItem (textfield);
	searchField.assign (textfield);

	button = new Button (StdString (""), sprites.getSprite (MediaUi::SearchButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaUi::searchButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiSearchTooltip));
	toolbar->addLeftItem (button);

	button = new Button (StdString (""), sprites.getSprite (MediaUi::SortButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaUi::sortButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiSortTooltip));
	toolbar->addLeftItem (button);

	toolbarMode = App::instance->prefsMap.find (App::MediaUiToolbarModeKey, (int) -1);
	if (toolbarMode < 0) {
		toolbarMode = MediaUi::MonitorMode;
	}
	setToolbarMode (toolbarMode, true);
}

void MediaUi::doResume () {
	StringList items;
	StringList::iterator i, end;
	StdString id;
	StreamPlaylistWindow *playlist;
	Json *obj;

	App::instance->setNextBackgroundTexturePath ("ui/MediaUi/bg");

	if (searchField.widget) {
		((TextFieldWindow *) searchField.widget)->setValue (searchKey);
	}
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	if (! isFirstResumeComplete) {
		App::instance->prefsMap.find (App::MediaUiPlaylistsKey, &items);
		i = items.begin ();
		end = items.end ();
		while (i != end) {
			obj = new Json ();
			if (obj->parse (*i)) {
				playlist = createStreamPlaylistWindow ();
				playlist->setState (obj);

				id = cardView->getAvailableItemId ();
				playlist->itemId.assign (id);
				playlist->sortKey.assign (playlist->playlistName.lowercased ());
				cardView->addItem (playlist, id, playlist->isExpanded ? MediaUi::ExpandedPlaylistRow : MediaUi::UnexpandedPlaylistRow);
				playlist->animateNewCard ();
				if (playlist->isSelected) {
					selectedPlaylistWindow.assign (playlist);
				}
			}
			delete (obj);
			++i;
		}
	}

	resetExpandToggles ();
}

void MediaUi::doPause () {
	StringList items;

	App::instance->prefsMap.insert (App::MediaUiToolbarModeKey, toolbarMode);

	items.clear ();
	cardView->processItems (MediaUi::appendPlaylistJson, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::MediaUiPlaylistsKey);
	}
	else {
		App::instance->prefsMap.insert (App::MediaUiPlaylistsKey, &items);
	}

	items.clear ();
	cardView->processItems (MediaUi::appendSelectedAgentId, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::MediaUiSelectedAgentsKey);
	}
	else {
		App::instance->prefsMap.insert (App::MediaUiSelectedAgentsKey, &items);
	}

	items.clear ();
	cardView->processItems (MediaUi::appendExpandedAgentId, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::MediaUiExpandedAgentsKey);
	}
	else {
		App::instance->prefsMap.insert (App::MediaUiExpandedAgentsKey, &items);
	}

	App::instance->prefsMap.insert (App::MediaUiShowMediaWithoutStreamsKey, isShowingMediaWithoutStreams);
	App::instance->prefsMap.insert (App::MediaUiSortOrderKey, mediaSortOrder);
}

void MediaUi::appendSelectedAgentId (void *stringListPtr, Widget *widgetPtr) {
	MediaLibraryWindow *medialibrary;
	MonitorWindow *monitor;

	medialibrary = MediaLibraryWindow::castWidget (widgetPtr);
	if (medialibrary && medialibrary->isSelected) {
		((StringList *) stringListPtr)->push_back (medialibrary->agentId);
		return;
	}

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor && monitor->isSelected) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
		return;
	}
}

void MediaUi::appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
	MediaLibraryWindow *medialibrary;
	MonitorWindow *monitor;

	medialibrary = MediaLibraryWindow::castWidget (widgetPtr);
	if (medialibrary && medialibrary->isExpanded) {
		((StringList *) stringListPtr)->push_back (medialibrary->agentId);
		return;
	}

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor && monitor->isExpanded) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
		return;
	}
}

void MediaUi::appendPlaylistJson (void *stringListPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *playlist;
	Json *json;

	playlist = StreamPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		json = playlist->getState ();
		((StringList *) stringListPtr)->push_back (json->toString ());
		delete (json);
	}
}

void MediaUi::doClearPopupWidgets () {
	commandPopup.destroyAndClear ();
	commandPopupSource.clear ();
}

void MediaUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MediaUi::doUpdate (int msElapsed) {
	UiText *uitext;
	std::map<StdString, MediaUi::MediaServerInfo>::iterator i, end;
	Json *params;
	int64_t now;
	int count;
	bool found;

	uitext = &(App::instance->uiText);
	emptyStateWindow.compact ();
	commandPopup.compact ();
	lastSelectedMediaWindow.compact ();
	selectedPlaylistWindow.compact ();
	expandAgentsToggle.compact ();
	expandPlaylistsToggle.compact ();

	if (recordReceiveCount > 0) {
		now = OsUtil::getTime ();
		if ((recordReceiveCount > 16) || (now >= nextRecordSyncTime)) {
			App::instance->shouldSyncRecordStore = true;
			recordReceiveCount = 0;
		}
	}
	else {
		if (cardView->isScrolledToBottom ()) {
			SDL_LockMutex (mediaServerMapMutex);
			i = mediaServerMap.begin ();
			end = mediaServerMap.end ();
			while (i != end) {
				if ((i->second.recordCount >= i->second.resultOffset) && (i->second.recordCount < i->second.setSize)) {
					params = new Json ();
					params->set ("searchKey", searchKey);
					params->set ("resultOffset", i->second.resultOffset);
					params->set ("maxResults", MediaUi::pageSize);
					params->set ("sortOrder", mediaSortOrder);
					App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Media, params), i->first);
					i->second.resultOffset += MediaUi::pageSize;
				}
				++i;
			}
			SDL_UnlockMutex (mediaServerMapMutex);
		}
	}

	found = false;
	SDL_LockMutex (mediaServerMapMutex);
	i = mediaServerMap.begin ();
	end = mediaServerMap.end ();
	while (i != end) {
		if (i->second.setSize < i->second.resultOffset) {
			count = i->second.setSize;
		}
		else {
			count = i->second.resultOffset;
		}
		if (i->second.recordCount < count) {
			found = true;
			break;
		}
		++i;
	}
	SDL_UnlockMutex (mediaServerMapMutex);

	if ((! isLoadingMedia) && found) {
		cardView->setRowHeader (MediaUi::MediaRow, createRowHeaderPanel (uitext->getText (UiTextString::media).capitalized (), createLoadingIconWindow ()));
		cardView->addItem (createLoadingIconWindow (), StdString (""), MediaUi::MediaLoadingRow);
	}
	else if (isLoadingMedia && (! found)) {
		cardView->setRowHeader (MediaUi::MediaRow, createRowHeaderPanel (uitext->getText (UiTextString::media).capitalized ()));
		cardView->removeRowItems (MediaUi::MediaLoadingRow);
	}
	isLoadingMedia = found;
}

void MediaUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	if (searchField.widget) {
		((TextFieldWindow *) searchField.widget)->setWindowWidth (App::instance->windowWidth * 0.37f);
	}
}

bool MediaUi::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	int mode;

	if (keycode == SDLK_TAB) {
		mode = toolbarMode + 1;
		if (mode >= MediaUi::ModeCount) {
			mode = 0;
		}
		setToolbarMode (mode);
		return (true);
	}

	return (false);
}

void MediaUi::doSyncRecordStore () {
	RecordStore *store;
	UiConfiguration *uiconfig;
	UiText *uitext;
	IconCardWindow *window;
	int type;

	store = &(App::instance->agentControl.recordStore);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	mediaServerCount = 0;
	store->processAgentRecords ("mediaServerStatus", MediaUi::processMediaServerAgent, this);
	store->processAgentRecords ("monitorServerStatus", MediaUi::processMonitorAgent, this);

	mediaItemCount = 0;
	mediaStreamCount = 0;
	store->processCommandRecords (SystemInterface::CommandId_MediaItem, MediaUi::processMediaItem, this);

	type = -1;
	if (cardView->getRowItemCount (MediaUi::MediaRow) <= 0) {
		if (mediaServerCount <= 0) {
			type = MediaUi::EmptyAgentState;
		}
		else if ((mediaItemCount <= 0) && findMediaComplete) {
			type = MediaUi::EmptyMediaState;
		}
		else if ((mediaItemCount > 0) && (mediaStreamCount <= 0) && (! isShowingMediaWithoutStreams)) {
			type = MediaUi::EmptyMediaStreamState;
		}
	}

	if (type != emptyStateType) {
		window = (IconCardWindow *) emptyStateWindow.widget;
		if (window) {
			cardView->removeItem (window->itemId);
			emptyStateWindow.clear ();
		}

		window = NULL;
		switch (type) {
			case MediaUi::EmptyAgentState: {
				window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite), uitext->getText (UiTextString::mediaUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::mediaUiEmptyAgentStatusText));
				window->setLink (uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
				window->itemId.assign (cardView->getAvailableItemId ());
				cardView->addItem (window, window->itemId, MediaUi::UnexpandedAgentRow);
				emptyStateWindow.assign (window);
				break;
			}
			case MediaUi::EmptyMediaState: {
				window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite), uitext->getText (UiTextString::mediaUiEmptyMediaStatusTitle), StdString (""), uitext->getText (UiTextString::mediaUiEmptyMediaStatusText));
				window->setLink (uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
				window->itemId.assign (cardView->getAvailableItemId ());
				cardView->addItem (window, window->itemId, MediaUi::EmptyMediaRow);
				emptyStateWindow.assign (window);
				break;
			}
			case MediaUi::EmptyMediaStreamState: {
				window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeStreamIconSprite), uitext->getText (UiTextString::mediaUiEmptyMediaStreamStatusTitle), StdString (""), uitext->getText (UiTextString::mediaUiEmptyMediaStreamStatusText));
				window->itemId.assign (cardView->getAvailableItemId ());
				cardView->addItem (window, window->itemId, MediaUi::EmptyMediaRow);
				emptyStateWindow.assign (window);
				break;
			}
		}
	}
	emptyStateType = type;

	cardView->syncRecordStore ();
	cardView->refresh ();
	resetExpandToggles ();
}

void MediaUi::processMediaServerAgent (void *uiPtr, Json *record, const StdString &recordId) {
	MediaUi *ui;
	SystemInterface *interface;
	MediaLibraryWindow *medialibrary;
	StringList items;
	int row, pos;

	ui = (MediaUi *) uiPtr;
	interface = &(App::instance->systemInterface);

	++(ui->mediaServerCount);
	if (! ui->cardView->contains (recordId)) {
		medialibrary = new MediaLibraryWindow (recordId);
		medialibrary->setMenuClickCallback (MediaUi::mediaLibraryMenuClicked, ui);
		medialibrary->setExpandStateChangeCallback (MediaUi::cardExpandStateChanged, ui);

		App::instance->prefsMap.find (App::MediaUiExpandedAgentsKey, &items);
		pos = items.indexOf (recordId);
		if (pos < 0) {
			row = MediaUi::UnexpandedAgentRow;
			medialibrary->sortKey.sprintf ("a%s", interface->getCommandAgentName (record).lowercased ().c_str ());
		}
		else {
			row = MediaUi::ExpandedAgentRow;
			medialibrary->setExpanded (true, true);
			medialibrary->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
		}

		ui->cardView->addItem (medialibrary, recordId, row);
		medialibrary->animateNewCard ();
		App::instance->agentControl.refreshAgentStatus (recordId);
		ui->addLinkAgent (recordId);
	}
}

void MediaUi::processMonitorAgent (void *uiPtr, Json *record, const StdString &recordId) {
	MediaUi *ui;
	SystemInterface *interface;
	MonitorWindow *monitor;
	StringList items;
	int row, pos;

	ui = (MediaUi *) uiPtr;
	interface = &(App::instance->systemInterface);

	if (! ui->cardView->contains (recordId)) {
		monitor = new MonitorWindow (recordId);
		monitor->setScreenshotDisplayEnabled (true);
		monitor->setStorageDisplayEnabled (true);
		monitor->setSelectStateChangeCallback (MediaUi::monitorSelectStateChanged, ui);
		monitor->setExpandStateChangeCallback (MediaUi::cardExpandStateChanged, ui);
		monitor->setScreenshotLoadCallback (MediaUi::monitorScreenshotLoaded, ui);
		monitor->addCacheButton (ui->sprites.getSprite (MediaUi::CacheButtonSprite), MediaUi::monitorCacheButtonClicked, NULL, NULL, ui);

		App::instance->prefsMap.find (App::MediaUiSelectedAgentsKey, &items);
		if (items.contains (recordId)) {
			monitor->setSelected (true, true);
			ui->selectedMonitorMap.insert (recordId, App::instance->systemInterface.getCommandAgentName (record));
		}

		App::instance->prefsMap.find (App::MediaUiExpandedAgentsKey, &items);
		pos = items.indexOf (recordId);
		if (pos < 0) {
			row = MediaUi::UnexpandedAgentRow;
			monitor->sortKey.sprintf ("b%s", interface->getCommandAgentName (record).lowercased ().c_str ());
		}
		else {
			row = MediaUi::ExpandedAgentRow;
			monitor->setExpanded (true, true);
			monitor->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
		}

		ui->cardView->addItem (monitor, recordId, row);
		monitor->animateNewCard ();
		App::instance->agentControl.refreshAgentStatus (recordId);
		ui->addLinkAgent (recordId);
	}
}

void MediaUi::processMediaItem (void *uiPtr, Json *record, const StdString &recordId) {
	MediaUi *ui;
	MediaWindow *media;
	SystemInterface *interface;
	StdString mediaid, agentid;
	Json *params, *streamrecord;
	StringList sourceids;
	int findstate;
	bool show;

	ui = (MediaUi *) uiPtr;
	++(ui->mediaItemCount);

	interface = &(App::instance->systemInterface);
	agentid = interface->getCommandAgentId (record);
	mediaid.assign (recordId);
	streamrecord = App::instance->agentControl.recordStore.findRecord (MediaWindow::matchStreamSourceId, &mediaid);
	if (streamrecord) {
		++(ui->mediaStreamCount);
	}

	SDL_LockMutex (ui->findMediaStreamsMapMutex);
	findstate = ui->findMediaStreamsMap.find (recordId, (int) -1);
	SDL_UnlockMutex (ui->findMediaStreamsMapMutex);

	if (findstate < 0) {
		if (App::instance->agentControl.isLinkClientConnected (agentid)) {
			sourceids.push_back (recordId);
			params = new Json ();
			params->set ("sourceIds", &sourceids);
			App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindMediaStreams, SystemInterface::Constant_Stream, params), agentid);

			SDL_LockMutex (ui->findMediaStreamsMapMutex);
			ui->findMediaStreamsMap.insert (recordId, MediaUi::StreamsRequestedState);
			SDL_UnlockMutex (ui->findMediaStreamsMapMutex);
		}
	}
	else if (findstate == MediaUi::StreamsReceivedState) {
		if (! ui->cardView->contains (recordId)) {
			show = true;
			if ((!(ui->isShowingMediaWithoutStreams)) && (! streamrecord)) {
				show = false;
			}
			else {
				SDL_LockMutex (ui->mediaServerMapMutex);
				if (ui->mediaServerMap.count (agentid) <= 0) {
					show = false;
				}
				SDL_UnlockMutex (ui->mediaServerMapMutex);
			}

			if (show) {
				media = new MediaWindow (record);
				media->setMediaImageClickCallback (MediaUi::mediaWindowImageClicked, ui);
				media->setViewButtonClickCallback (MediaUi::mediaWindowViewButtonClicked, ui);
				media->setSelectStateChangeCallback (MediaUi::mediaWindowSelectStateChanged, ui);
				if (ui->mediaSortOrder == SystemInterface::Constant_NewestSort) {
					media->sortKey.sprintf ("%016llx", (long long int) (0x7FFFFFFFFFFFFFFFLL - interface->getCommandNumberParam (record, "mtime", (int64_t) 0)));
				}
				else {
					media->sortKey.assign (media->mediaName);
				}
				if (ui->selectedMediaMap.exists (recordId)) {
					media->setSelected (true, true);
				}
				ui->cardView->addItem (media, recordId, MediaUi::MediaRow);
			}
		}
	}
}

bool MediaUi::matchMediaItem (void *idPtr, Widget *widget) {
	StdString *id;
	MediaWindow *media;

	media = MediaWindow::castWidget (widget);
	if (! media) {
		return (false);
	}

	id = (StdString *) idPtr;
	return (id->equals (media->mediaId));
}

void MediaUi::handleLinkClientConnect (const StdString &agentId) {
	RecordStore *store;
	SystemInterface *interface;
	Json *record, *params;
	bool ismedia, ismonitor;
	std::map<StdString, MediaUi::MediaServerInfo>::iterator info;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	ismedia = false;
	ismonitor = false;
	store->lock ();
	record = store->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (record) {
		if (interface->getCommandObjectParam (record, "mediaServerStatus", NULL)) {
			ismedia = true;
		}
		if (interface->getCommandObjectParam (record, "monitorServerStatus", NULL)) {
			ismonitor = true;
		}
	}
	store->unlock ();

	if (ismedia) {
		SDL_LockMutex (mediaServerMapMutex);
		if (mediaServerMap.count (agentId) <= 0) {
			info = getMediaServerInfo (agentId);
			info->second.resultOffset = MediaUi::pageSize;
			params = new Json ();
			params->set ("searchKey", searchKey);
			params->set ("maxResults", MediaUi::pageSize);
			params->set ("sortOrder", mediaSortOrder);
			App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Media, params), agentId);
		}
		SDL_UnlockMutex (mediaServerMapMutex);
	}

	if (ismedia || ismonitor) {
		App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_WatchStatus, SystemInterface::Constant_Admin), agentId);
	}
}

void MediaUi::handleLinkClientCommand (const StdString &agentId, int commandId, Json *command) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	std::map<StdString, MediaUi::MediaServerInfo>::iterator info;
	Json item;
	StdString id;
	int i, len;

	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);

	switch (commandId) {
		case SystemInterface::CommandId_FindMediaResult: {
			SDL_LockMutex (mediaServerMapMutex);
			info = getMediaServerInfo (agentId);
			info->second.setSize = interface->getCommandNumberParam (command, "setSize", (int) 0);
			if (info->second.setSize <= 0) {
				findMediaComplete = true;
			}
			SDL_UnlockMutex (mediaServerMapMutex);
			App::instance->shouldSyncRecordStore = true;
			break;
		}
		case SystemInterface::CommandId_MediaItem: {
			App::instance->agentControl.recordStore.addRecord (command);
			SDL_LockMutex (mediaServerMapMutex);
			info = getMediaServerInfo (agentId);
			++(info->second.recordCount);
			if ((info->second.recordCount >= info->second.setSize) || (info->second.recordCount >= info->second.resultOffset)) {
				App::instance->shouldSyncRecordStore = true;
			}
			SDL_UnlockMutex (mediaServerMapMutex);
			findMediaComplete = true;
			++recordReceiveCount;
			nextRecordSyncTime = OsUtil::getTime () + uiconfig->recordSyncDelayDuration;
			break;
		}
		case SystemInterface::CommandId_FindStreamsResult: {
			break;
		}
		case SystemInterface::CommandId_FindMediaStreamsResult: {
			nextRecordSyncTime = OsUtil::getTime () + uiconfig->recordSyncDelayDuration;
			id = interface->getCommandStringParam (command, "mediaId", "");
			if (! id.empty ()) {
				SDL_LockMutex (findMediaStreamsMapMutex);
				if (findMediaStreamsMap.exists (id)) {
					findMediaStreamsMap.insert (id, MediaUi::StreamsReceivedState);
				}
				SDL_UnlockMutex (findMediaStreamsMapMutex);
			}

			len = interface->getCommandArrayLength (command, "streams");
			if (len <= 0) {
				break;
			}

			for (i = 0; i < len; ++i) {
				if (interface->getCommandObjectArrayItem (command, "streams", i, &item)) {
					if (interface->getCommandId (&item) == SystemInterface::CommandId_StreamItem) {
						App::instance->agentControl.recordStore.addRecord (&item);
						App::instance->shouldSyncRecordStore = true;
					}
				}
			}
			break;
		}
		case SystemInterface::CommandId_StreamItem: {
			App::instance->agentControl.recordStore.addRecord (command);
			++recordReceiveCount;
			nextRecordSyncTime = OsUtil::getTime () + uiconfig->recordSyncDelayDuration;
			break;
		}
	}
}

void MediaUi::searchFieldEdited (void *uiPtr, Widget *widgetPtr) {
	MediaUi::searchButtonClicked (uiPtr, NULL);
}

void MediaUi::searchButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	TextFieldWindow *textfield;

	ui = (MediaUi *) uiPtr;
	textfield = (TextFieldWindow *) ui->searchField.widget;
	if (! textfield) {
		return;
	}

	if (ui->searchKey.equals (textfield->getValue ())) {
		return;
	}
	ui->searchKey.assign (textfield->getValue ());
	ui->loadSearchResults ();
}

void MediaUi::sortButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Menu *menu;
	bool show;

	ui = (MediaUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == widgetPtr)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	menu = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	menu->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::mediaUiShowMediaWithoutStreamsAction), uiconfig->coreSprites.getSprite (UiConfiguration::VisibilityOnButtonSprite), MediaUi::showMediaWithoutStreamsActionClicked, ui, 0, (! ui->isShowingMediaWithoutStreams));
	menu->addItem (uitext->getText (UiTextString::sortByName).capitalized (), ui->sprites.getSprite (MediaUi::SortButtonSprite), MediaUi::sortByNameActionClicked, ui, 2, ui->mediaSortOrder == SystemInterface::Constant_NameSort);
	menu->addItem (uitext->getText (UiTextString::sortByNewest).capitalized (), ui->sprites.getSprite (MediaUi::SortButtonSprite), MediaUi::sortByNewestActionClicked, ui, 2, ui->mediaSortOrder == SystemInterface::Constant_NewestSort);
	menu->position.assign (widgetPtr->screenX + widgetPtr->width - menu->width, widgetPtr->screenY - menu->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void MediaUi::showMediaWithoutStreamsActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StringList idlist;
	StringList::iterator i, end;

	ui = (MediaUi *) uiPtr;
	ui->isShowingMediaWithoutStreams = (! ui->isShowingMediaWithoutStreams);

	if (! ui->isShowingMediaWithoutStreams) {
		ui->cardView->processRowItems (MediaUi::MediaRow, MediaUi::appendMediaIdWithoutStream, &idlist);
		i = idlist.begin ();
		end = idlist.end ();
		while (i != end) {
			ui->cardView->removeItem (*i, true);
			++i;
		}
		ui->cardView->refresh ();
	}
	App::instance->shouldSyncRecordStore = true;
}

void MediaUi::appendMediaIdWithoutStream (void *stringListPtr, Widget *widgetPtr) {
	StringList *idlist;
	MediaWindow *media;

	idlist = (StringList *) stringListPtr;
	media = MediaWindow::castWidget (widgetPtr);
	if (media && media->streamId.empty ()) {
		idlist->push_back (media->mediaId);
	}
}

void MediaUi::sortByNameActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	if (ui->mediaSortOrder == SystemInterface::Constant_NameSort) {
		return;
	}
	ui->mediaSortOrder = SystemInterface::Constant_NameSort;
	ui->loadSearchResults ();
}

void MediaUi::sortByNewestActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	if (ui->mediaSortOrder == SystemInterface::Constant_NewestSort) {
		return;
	}
	ui->mediaSortOrder = SystemInterface::Constant_NewestSort;
	ui->loadSearchResults ();
}

void MediaUi::loadSearchResults () {
	std::map<StdString, MediaUi::MediaServerInfo>::iterator i, end;
	Json *params;

	SDL_LockMutex (mediaServerMapMutex);
	i = mediaServerMap.begin ();
	end = mediaServerMap.end ();
	while (i != end) {
		i->second.resultOffset = MediaUi::pageSize;
		i->second.setSize = 0;
		i->second.recordCount = 0;
		++i;
	}
	SDL_UnlockMutex (mediaServerMapMutex);

	SDL_LockMutex (findMediaStreamsMapMutex);
	findMediaStreamsMap.clear ();
	SDL_UnlockMutex (findMediaStreamsMapMutex);

	App::instance->agentControl.recordStore.removeRecords (SystemInterface::CommandId_MediaItem);
	cardView->removeRowItems (MediaUi::MediaRow);

	findMediaComplete = false;
	params = new Json ();
	params->set ("searchKey", searchKey);
	params->set ("maxResults", MediaUi::pageSize);
	params->set ("sortOrder", mediaSortOrder);
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Media, params));
}

void MediaUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Menu *menu;
	bool show;

	ui = (MediaUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == widgetPtr)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	menu = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	menu->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), MediaUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), MediaUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), MediaUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);
	menu->position.assign (widgetPtr->screenX + widgetPtr->width - menu->width, widgetPtr->screenY + widgetPtr->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void MediaUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	int detail;

	ui = (MediaUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaUi::MediaRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::MediaUiImageSizeKey, ui->cardDetail);
}

void MediaUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	int detail;

	ui = (MediaUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaUi::MediaRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::MediaUiImageSizeKey, ui->cardDetail);
}

void MediaUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	int detail;

	ui = (MediaUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaUi::MediaRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::MediaUiImageSizeKey, ui->cardDetail);
}

void MediaUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	SDL_LockMutex (ui->findMediaStreamsMapMutex);
	ui->findMediaStreamsMap.clear ();
	SDL_UnlockMutex (ui->findMediaStreamsMapMutex);

	ui->cardView->processItems (MediaUi::reloadAgent, ui);
	ui->loadSearchResults ();
}

void MediaUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	MediaLibraryWindow *medialibrary;
	MonitorWindow *monitor;

	medialibrary = MediaLibraryWindow::castWidget (widgetPtr);
	if (medialibrary) {
		App::instance->agentControl.refreshAgentStatus (medialibrary->agentId);
		return;
	}

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		App::instance->agentControl.refreshAgentStatus (monitor->agentId);
		return;
	}
}

StreamPlaylistWindow *MediaUi::createStreamPlaylistWindow () {
	StreamPlaylistWindow *playlist;

	playlist = new StreamPlaylistWindow ();
	playlist->setSelectStateChangeCallback (MediaUi::playlistSelectStateChanged, this);
	playlist->setExpandStateChangeCallback (MediaUi::cardExpandStateChanged, this);
	playlist->setListChangeCallback (MediaUi::playlistItemsChanged, this);
	playlist->setRenameClickCallback (MediaUi::playlistRenameActionClicked, this);

	return (playlist);
}

StdString MediaUi::getAvailablePlaylistName () {
	StdString basename, name;
	int i;

	basename.assign (App::instance->uiText.getText (UiTextString::playlist).capitalized ());
	name.assign (basename);
	cardView->processItems (MediaUi::matchPlaylistName, &name);
	if (name.empty ()) {
		i = 2;
		while (true) {
			name.sprintf ("%s %i", basename.c_str (), i);
			cardView->processItems (MediaUi::matchPlaylistName, &name);
			if (! name.empty ()) {
				break;
			}
			++i;
		}
	}

	return (name);
}

void MediaUi::matchPlaylistName (void *stringPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *playlist;
	StdString *name;

	playlist = StreamPlaylistWindow::castWidget (widgetPtr);
	if (! playlist) {
		return;
	}

	name = (StdString *) stringPtr;
	if (name->lowercased ().equals (playlist->playlistName.lowercased ())) {
		name->assign ("");
	}
}

void MediaUi::mediaWindowImageClicked (void *uiPtr, Widget *widgetPtr) {
	MediaWindow *target;

	target = MediaWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	target->setSelected (! target->isSelected);
}

void MediaUi::mediaWindowViewButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *target;
	MediaItemUi *mediaitemui;
	StreamItemUi *streamitemui;

	ui = (MediaUi *) uiPtr;
	target = MediaWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	if (! target->streamId.empty ()) {
		ui->actionTarget.assign (target);
		streamitemui = new StreamItemUi (target->streamId, target->mediaName, App::instance->uiText.getText (UiTextString::selectPlayPosition).capitalized ());
		streamitemui->setThumbnailClickCallback (MediaUi::itemUiThumbnailClicked, ui);
		App::instance->uiStack.pushUi (streamitemui);
	}
	else {
		if (target->hasThumbnails ()) {
			ui->actionTarget.assign (target);
			mediaitemui = new MediaItemUi (target->mediaId, target->mediaName);
			mediaitemui->setRemoveMediaCallback (MediaUi::mediaItemUiMediaRemoved, ui);
			App::instance->uiStack.pushUi (mediaitemui);
		}
	}
}

void MediaUi::itemUiThumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaThumbnailWindow *thumbnail;
	MediaWindow *target;

	ui = (MediaUi *) uiPtr;
	thumbnail = MediaThumbnailWindow::castWidget (widgetPtr);
	target = MediaWindow::castWidget (ui->actionTarget.widget);
	if ((! thumbnail) || (! target)) {
		return;
	}

	target->setDisplayTimestamp (thumbnail->thumbnailTimestamp);
	target->setThumbnail (thumbnail->sourceUrl, thumbnail->thumbnailIndex);
	target->setSelected (true);

	App::instance->uiStack.popUi ();
}

void MediaUi::mediaItemUiMediaRemoved (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *target;

	ui = (MediaUi *) uiPtr;
	target = MediaWindow::castWidget (ui->actionTarget.widget);
	if (! target) {
		return;
	}

	ui->cardView->removeItem (target->mediaId);
	ui->actionTarget.clear ();
}

void MediaUi::mediaWindowSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *media, *item;
	StdString mediaid;
	HashMap::Iterator i;
	StringList keys;
	StringList::iterator ki, kend;

	ui = (MediaUi *) uiPtr;
	media = (MediaWindow *) widgetPtr;
	if (media->isSelected) {
		if (ui->toolbarMode == MediaUi::MonitorMode) {
			ui->selectedMediaMap.getKeys (&keys);
			ki = keys.begin ();
			kend = keys.end ();
			while (ki != kend) {
				mediaid.assign (*ki);
				item = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
				if (item) {
					item->setSelected (false, true);
				}
				++ki;
			}
			ui->selectedMediaMap.clear ();
		}
		ui->selectedMediaMap.insert (media->mediaId, media->mediaName);
		ui->lastSelectedMediaWindow.assign (media);
	}
	else {
		ui->selectedMediaMap.remove (media->mediaId);
		if (ui->selectedMediaMap.empty ()) {
			ui->lastSelectedMediaWindow.clear ();
		}
		else {
			if (media == ui->lastSelectedMediaWindow.widget) {
				i = ui->selectedMediaMap.begin ();
				mediaid = ui->selectedMediaMap.next (&i);
				ui->lastSelectedMediaWindow.assign (ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid));
			}
		}
	}
}

void MediaUi::cardExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MonitorWindow *monitor;
	MediaLibraryWindow *medialibrary;
	StreamPlaylistWindow *playlist;

	ui = (MediaUi *) uiPtr;

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		if (monitor->isExpanded) {
			monitor->sortKey.sprintf ("%016llx", (long long int) OsUtil::getTime ());
			ui->cardView->setItemRow (monitor->agentId, MediaUi::ExpandedAgentRow);
		}
		else {
			monitor->sortKey.sprintf ("b%s", monitor->agentName.lowercased ().c_str ());
			ui->cardView->setItemRow (monitor->agentId, MediaUi::UnexpandedAgentRow);
		}
		monitor->resetInputState ();
		monitor->animateNewCard ();
		ui->resetExpandToggles ();
		ui->cardView->refresh ();
		return;
	}

	medialibrary = MediaLibraryWindow::castWidget (widgetPtr);
	if (medialibrary) {
		if (medialibrary->isExpanded) {
			medialibrary->sortKey.sprintf ("%016llx", (long long int) OsUtil::getTime ());
			ui->cardView->setItemRow (medialibrary->agentId, MediaUi::ExpandedAgentRow);
		}
		else {
			medialibrary->sortKey.sprintf ("a%s", medialibrary->agentName.lowercased ().c_str ());
			ui->cardView->setItemRow (medialibrary->agentId, MediaUi::UnexpandedAgentRow);
		}
		medialibrary->resetInputState ();
		medialibrary->animateNewCard ();
		ui->resetExpandToggles ();
		ui->cardView->refresh ();
		return;
	}

	playlist = StreamPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		if (playlist->isExpanded) {
			ui->cardView->setItemRow (playlist->itemId, MediaUi::ExpandedPlaylistRow);
		}
		else {
			playlist->sortKey.assign (playlist->playlistName.lowercased ());
			ui->cardView->setItemRow (playlist->itemId, MediaUi::UnexpandedPlaylistRow);
		}
		playlist->resetInputState ();
		playlist->animateNewCard ();
		ui->resetExpandToggles ();
		ui->cardView->refresh ();
		return;
	}
}

void MediaUi::expandAgentsToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	Toggle *toggle;
	Widget *item;
	MonitorWindow *monitor;
	MediaLibraryWindow *medialibrary;
	StringList idlist;
	StringList::iterator i, end;
	int64_t now;

	ui = (MediaUi *) uiPtr;
	toggle = (Toggle *) ui->expandAgentsToggle.widget;
	if (! toggle) {
		return;
	}

	now = OsUtil::getTime ();
	ui->cardView->processItems (MediaUi::appendAgentId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		item = ui->cardView->getItem (*i);
		monitor = MonitorWindow::castWidget (item);
		if (monitor) {
			if (toggle->isChecked) {
				monitor->setExpanded (false, true);
				monitor->sortKey.sprintf ("b%s", monitor->agentName.lowercased ().c_str ());
				ui->cardView->setItemRow (monitor->agentId, MediaUi::UnexpandedAgentRow, true);
			}
			else {
				monitor->setExpanded (true, true);
				monitor->sortKey.sprintf ("%016llx%s", (long long int) now, monitor->agentName.lowercased ().c_str ());
				ui->cardView->setItemRow (monitor->agentId, MediaUi::ExpandedAgentRow, true);
			}
		}
		else {
			medialibrary = MediaLibraryWindow::castWidget (item);
			if (medialibrary) {
				if (toggle->isChecked) {
					medialibrary->setExpanded (false, true);
					medialibrary->sortKey.sprintf ("a%s", medialibrary->agentName.lowercased ().c_str ());
					ui->cardView->setItemRow (medialibrary->agentId, MediaUi::UnexpandedAgentRow, true);
				}
				else {
					medialibrary->setExpanded (true, true);
					medialibrary->sortKey.sprintf ("%016llx%s", (long long int) now, medialibrary->agentName.lowercased ().c_str ());
					ui->cardView->setItemRow (medialibrary->agentId, MediaUi::ExpandedAgentRow, true);
				}
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

void MediaUi::appendAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;
	MediaLibraryWindow *medialibrary;

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
		return;
	}

	medialibrary = MediaLibraryWindow::castWidget (widgetPtr);
	if (medialibrary) {
		((StringList *) stringListPtr)->push_back (medialibrary->agentId);
		return;
	}
}

void MediaUi::expandPlaylistsToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	Toggle *toggle;
	StreamPlaylistWindow *playlist;
	StringList idlist;
	StringList::iterator i, end;

	ui = (MediaUi *) uiPtr;
	toggle = (Toggle *) ui->expandPlaylistsToggle.widget;
	if (! toggle) {
		return;
	}

	ui->cardView->processItems (MediaUi::appendPlaylistId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		playlist = StreamPlaylistWindow::castWidget (ui->cardView->getItem (*i));
		if (playlist) {
			if (toggle->isChecked) {
				playlist->setExpanded (false, true);
				ui->cardView->setItemRow (playlist->itemId, MediaUi::UnexpandedPlaylistRow, true);
			}
			else {
				playlist->setExpanded (true, true);
				ui->cardView->setItemRow (playlist->itemId, MediaUi::ExpandedPlaylistRow, true);
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

void MediaUi::appendPlaylistId (void *stringListPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *playlist;

	playlist = StreamPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		((StringList *) stringListPtr)->push_back (playlist->itemId);
		return;
	}
}

void MediaUi::mediaLibraryMenuClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	MediaLibraryWindow *target;
	Menu *action;
	bool show;

	ui = (MediaUi *) uiPtr;
	target = (MediaLibraryWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == target)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (target);

	action->addItem (uitext->getText (UiTextString::scanForMedia).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::UpdateButtonSprite), MediaUi::mediaLibraryScanActionClicked, ui);
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->position.assign (target->screenX + target->menuPositionX, target->screenY + target->menuPositionY);
}

void MediaUi::mediaLibraryScanActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaLibraryWindow *target;
	int result;

	ui = (MediaUi *) uiPtr;
	target = MediaLibraryWindow::castWidget (ui->actionTarget.widget);
	if (! target) {
		return;
	}

	result = App::instance->agentControl.invokeCommand (target->agentId, App::instance->createCommand (SystemInterface::Command_ScanMediaItems, SystemInterface::Constant_Media));
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::scanForMediaStartedMessage));
	}
}

void MediaUi::monitorSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MonitorWindow *monitor;

	ui = (MediaUi *) uiPtr;
	monitor = (MonitorWindow *) widgetPtr;
	if (monitor->isSelected) {
		ui->selectedMonitorMap.insert (monitor->agentId, monitor->agentName);
	}
	else {
		ui->selectedMonitorMap.remove (monitor->agentId);
	}
}

void MediaUi::monitorScreenshotLoaded (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->cardView->refresh ();
}

void MediaUi::monitorCacheButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *cacheui;
	MonitorWindow *target;

	target = MonitorWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	cacheui = new MonitorCacheUi (target->agentId, target->agentName);
	App::instance->uiStack.pushUi (cacheui);
}

void MediaUi::playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist, *item;

	ui = (MediaUi *) uiPtr;
	playlist = (StreamPlaylistWindow *) widgetPtr;
	if (playlist->isSelected) {
		if (ui->selectedPlaylistWindow.widget != playlist) {
			if (ui->selectedPlaylistWindow.widget) {
				item = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
				if (item) {
					item->setSelected (false, true);
				}
			}
			ui->selectedPlaylistWindow.assign (playlist);
		}
	}
	else {
		ui->selectedPlaylistWindow.clear ();
	}
}

void MediaUi::playlistItemsChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->cardView->refresh ();
}

void MediaUi::playlistRenameActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StreamPlaylistWindow *target;
	TextFieldWindow *textfield;

	ui = (MediaUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	target = StreamPlaylistWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	ui->clearPopupWidgets ();
	textfield = (TextFieldWindow *) App::instance->rootPanel->addWidget (new TextFieldWindow (target->width - (uiconfig->marginSize * 2.0f), uitext->getText (UiTextString::enterPlaylistNamePrompt)));
	ui->actionWidget.assign (textfield);
	ui->actionTarget.assign (target);
	textfield->setValue (target->playlistName);
	textfield->setEditCallback (MediaUi::playlistNameEdited, ui);
	textfield->setFillBg (true, uiconfig->lightPrimaryColor);
	textfield->setButtonsEnabled (true, true, true, true);
	textfield->assignKeyFocus ();
	textfield->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	textfield->position.assign (target->screenX + uiconfig->marginSize, target->screenY);
}

void MediaUi::playlistNameEdited (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;
	TextFieldWindow *textfield;
	StdString name;

	ui = (MediaUi *) uiPtr;
	textfield = TextFieldWindow::castWidget (ui->actionWidget.widget);
	playlist = StreamPlaylistWindow::castWidget (ui->actionTarget.widget);
	if ((! textfield) || (! playlist)) {
		return;
	}

	name = textfield->getValue ();
	if (name.empty ()) {
		name.assign (ui->getAvailablePlaylistName ());
	}
	playlist->setPlaylistName (name);
	playlist->sortKey.assign (playlist->playlistName.lowercased ());
	ui->clearPopupWidgets ();
}

void MediaUi::modeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiText *uitext;
	Menu *menu;
	bool show;

	ui = (MediaUi *) uiPtr;
	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == widgetPtr)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	menu = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	menu->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::playStreams).capitalized (), ui->sprites.getSprite (MediaUi::PlayButtonSprite), MediaUi::monitorModeActionClicked, ui, 0, ui->toolbarMode == MediaUi::MonitorMode);
	menu->addItem (uitext->getText (UiTextString::manageStreams).capitalized (), ui->sprites.getSprite (MediaUi::ConfigureStreamButtonSprite), MediaUi::streamModeActionClicked, ui, 0, ui->toolbarMode == MediaUi::StreamMode);
	menu->addItem (uitext->getText (UiTextString::managePlaylists).capitalized (), ui->sprites.getSprite (MediaUi::AddPlaylistItemButtonSprite), MediaUi::playlistModeActionClicked, ui, 0, ui->toolbarMode == MediaUi::PlaylistMode);
	menu->position.assign (widgetPtr->screenX + widgetPtr->width - menu->width, widgetPtr->screenY - menu->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void MediaUi::monitorModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->setToolbarMode (MediaUi::MonitorMode);
}

void MediaUi::streamModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->setToolbarMode (MediaUi::StreamMode);
}

void MediaUi::playlistModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->setToolbarMode (MediaUi::PlaylistMode);
}

void MediaUi::setToolbarMode (int mode, bool forceReset) {
	UiConfiguration *uiconfig;
	UiText *uitext;
	Toolbar *toolbar;
	Button *button;

	if ((toolbarMode == mode) && (! forceReset)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	toolbar = App::instance->uiStack.secondaryToolbar;
	toolbar->clearRightItems ();
	playButton = NULL;
	writePlaylistButton = NULL;
	stopButton = NULL;
	browserPlayButton = NULL;
	configureStreamButton = NULL;
	cacheStreamButton = NULL;
	deleteStreamButton = NULL;
	addPlaylistItemButton = NULL;
	createPlaylistButton = NULL;
	deletePlaylistButton = NULL;

	button = new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::ToolsButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaUi::modeButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::toolbarModeButtonTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (button);

	toolbarMode = mode;
	switch (toolbarMode) {
		case MediaUi::MonitorMode: {
			button = new Button (StdString (""), sprites.getSprite (MediaUi::PlayButtonSprite));
			button->setInverseColor (true);
			button->setMouseClickCallback (MediaUi::playButtonClicked, this);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiPlayTooltip).capitalized (), Widget::LeftAlignment);
			button->shortcutKey = SDLK_F4;
			toolbar->addRightItem (button);
			playButton = button;

			button = new Button (StdString (""), sprites.getSprite (MediaUi::WritePlaylistButtonSprite));
			button->setInverseColor (true);
			button->setMouseClickCallback (MediaUi::writePlaylistButtonClicked, this);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiWritePlaylistTooltip), Widget::LeftAlignment);
			button->shortcutKey = SDLK_F3;
			toolbar->addRightItem (button);
			writePlaylistButton = button;

			button = new Button (StdString (""), sprites.getSprite (MediaUi::StopButtonSprite));
			button->setInverseColor (true);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseClickCallback (MediaUi::stopButtonClicked, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiStopTooltip), Widget::LeftAlignment);
			button->shortcutKey = SDLK_F2;
			toolbar->addRightItem (button);
			stopButton = button;

			button = new Button (StdString (""), sprites.getSprite (MediaUi::BrowserPlayButtonSprite));
			button->setInverseColor (true);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseClickCallback (MediaUi::browserPlayButtonClicked, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiBrowserPlayTooltip), Widget::LeftAlignment);
			button->shortcutKey = SDLK_F1;
			toolbar->addRightItem (button);
			browserPlayButton = button;
			break;
		}
		case MediaUi::StreamMode: {
			button = new Button (StdString (""), sprites.getSprite (MediaUi::ConfigureStreamButtonSprite));
			button->setInverseColor (true);
			button->setMouseClickCallback (MediaUi::configureStreamButtonClicked, this);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiConfigureStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (button);
			configureStreamButton = button;

			button = new Button (StdString (""), sprites.getSprite (MediaUi::CacheButtonSprite));
			button->setInverseColor (true);
			button->setMouseClickCallback (MediaUi::cacheStreamButtonClicked, this);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiAddCacheStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (button);
			cacheStreamButton = button;

			button = new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite));
			button->setInverseColor (true);
			button->setMouseClickCallback (MediaUi::deleteStreamButtonClicked, this);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiDeleteStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (button);
			deleteStreamButton = button;
			break;
		}
		case MediaUi::PlaylistMode: {
			button = new Button (StdString (""), sprites.getSprite (MediaUi::AddPlaylistItemButtonSprite));
			button->setInverseColor (true);
			button->setMouseClickCallback (MediaUi::addPlaylistItemButtonClicked, this);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiAddPlaylistItemTooltip), Widget::LeftAlignment);
			button->shortcutKey = SDLK_F3;
			toolbar->addRightItem (button);
			addPlaylistItemButton = button;

			button = new Button (StdString (""), sprites.getSprite (MediaUi::CreatePlaylistButtonSprite));
			button->setInverseColor (true);
			button->setMouseClickCallback (MediaUi::createPlaylistButtonClicked, this);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiCreatePlaylistTooltip), Widget::LeftAlignment);
			button->shortcutKey = SDLK_F2;
			toolbar->addRightItem (button);
			createPlaylistButton = button;

			button = new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite));
			button->setInverseColor (true);
			button->setMouseClickCallback (MediaUi::deletePlaylistButtonClicked, this);
			button->setMouseEnterCallback (MediaUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (MediaUi::commandButtonMouseExited, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiDeletePlaylistTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (button);
			deletePlaylistButton = button;
			break;
		}
	}
}

void MediaUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	Button *button;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	MediaWindow *media;
	StreamPlaylistWindow *playlist;
	StdString text;
	Color color;

	ui = (MediaUi *) uiPtr;
	button = (Button *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	Button::mouseEntered (button, button);

	ui->commandPopup.destroyAndClear ();
	ui->commandPopupSource.assign (button);
	panel = (Panel *) App::instance->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha));
	panel->setBorder (true, Color (uiconfig->darkBackgroundColor.r, uiconfig->darkBackgroundColor.g, uiconfig->darkBackgroundColor.b, uiconfig->overlayWindowAlpha));

	label = NULL;
	if (button == ui->configureStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::configureStream).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMediaNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallMediaIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->deleteStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::deleteStream).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMediaNames (App::instance->rootPanel->width * 0.25f, true));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoStreamableMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->cacheStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::cacheStream).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->getSelectedMediaNames (App::instance->rootPanel->width * 0.25f, true));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoStreamableMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->playButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::play).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		media = MediaWindow::castWidget (ui->lastSelectedMediaWindow.widget);
		if ((! media) || media->hlsStreamPath.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoStreamableMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (media->mediaName);
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			text.appendSprintf (" %s", OsUtil::getDurationString ((media->displayTimestamp > 0.0f) ? (int64_t) media->displayTimestamp : 0, OsUtil::HoursUnit).c_str ());
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->writePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::runPlaylist).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		playlist = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
		if (! playlist) {
			text.assign (uitext->getText (UiTextString::mediaUiNoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else if (playlist->getItemCount () <= 0) {
			text.assign (uitext->getText (UiTextString::mediaUiEmptyPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (playlist->playlistName);
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->stopButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::stop).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->browserPlayButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::playInBrowser).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		media = MediaWindow::castWidget (ui->lastSelectedMediaWindow.widget);
		if ((! media) || media->streamId.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoStreamableMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (media->mediaName);
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			text.appendSprintf (" %s", OsUtil::getDurationString ((media->displayTimestamp > 0.0f) ? (int64_t) media->displayTimestamp : 0, OsUtil::HoursUnit).c_str ());
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->createPlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::createPlaylist).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));
	}
	else if (button == ui->addPlaylistItemButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::addPlaylistItem).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		playlist = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
		if (! playlist) {
			text.assign (uitext->getText (UiTextString::mediaUiNoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (playlist->playlistName);
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->getSelectedMediaNames (App::instance->rootPanel->width * 0.25f, true));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::mediaUiNoStreamableMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->deletePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::deletePlaylist).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		playlist = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
		if (! playlist) {
			text.assign (uitext->getText (UiTextString::mediaUiNoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (playlist->playlistName);
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}

	if (label) {
		label->setPadding (0.0f, 0.0f);
	}
	panel->setLayout (Panel::VerticalRightJustifiedLayout);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	panel->position.assign (App::instance->windowWidth - panel->width - uiconfig->paddingSize, App::instance->windowHeight - App::instance->uiStack.bottomBarHeight - panel->height - uiconfig->marginSize);
}

void MediaUi::commandButtonMouseExited (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	Button *button;

	ui = (MediaUi *) uiPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);

	if (ui->commandPopupSource.widget == button) {
		ui->commandPopup.destroyAndClear ();
		ui->commandPopupSource.clear ();
	}
}

void MediaUi::playButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *media;
	StringList idlist;
	StdString streamurl;
	Json *params;
	float startpos;
	int count;

	ui = (MediaUi *) uiPtr;
	media = MediaWindow::castWidget (ui->lastSelectedMediaWindow.widget);
	ui->selectedMonitorMap.getKeys (&idlist, true);
	if ((! media) || media->hlsStreamPath.empty () || idlist.empty ()) {
		return;
	}

	startpos = (media->displayTimestamp > 0.0f) ? (media->displayTimestamp / 1000.0f) : 0.0f;
	params = new Json ();
	params->set ("streamId", media->streamId);
	params->set ("startPosition", startpos);
	streamurl = App::instance->agentControl.getAgentSecondaryUrl (media->streamAgentId, App::instance->createCommand (SystemInterface::Command_GetHlsManifest, SystemInterface::Constant_Stream, params), media->hlsStreamPath);

	params = new Json ();
	params->set ("mediaName", media->mediaName);
	params->set ("streamUrl", streamurl);
	params->set ("streamId", media->streamId);
	params->set ("startPosition", startpos);
	if (! media->playThumbnailUrl.empty ()) {
		params->set ("thumbnailUrl", media->playThumbnailUrl);
	}
	count = App::instance->agentControl.invokeCommand (&idlist, App::instance->createCommand (SystemInterface::Command_PlayMedia, SystemInterface::Constant_Monitor, params));
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokePlayMediaMessage));
	}
}

void MediaUi::writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;
	StringList idlist;
	int count;

	ui = (MediaUi *) uiPtr;
	playlist = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
	ui->selectedMonitorMap.getKeys (&idlist, true);
	if ((! playlist) || (playlist->getItemCount () <= 0) || idlist.empty ()) {
		return;
	}

	count = App::instance->agentControl.invokeCommand (&idlist, playlist->getCreateCommand ());
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeCreateStreamPlaylistMessage));
	}
}

void MediaUi::stopButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StringList idlist;
	int count;

	ui = (MediaUi *) uiPtr;
	ui->selectedMonitorMap.getKeys (&idlist, true);
	if (idlist.empty ()) {
		return;
	}

	count = App::instance->agentControl.invokeCommand (&idlist, App::instance->createCommand (SystemInterface::Command_ClearDisplay, SystemInterface::Constant_Monitor));
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeClearDisplayMessage));
	}
}

void MediaUi::browserPlayButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *media;

	ui = (MediaUi *) uiPtr;
	media = MediaWindow::castWidget (ui->lastSelectedMediaWindow.widget);
	if ((! media) || media->streamAgentId.empty () || media->streamId.empty () || media->htmlPlayerPath.empty ()) {
		return;
	}

	OsUtil::openUrl (StdString::createSprintf ("%s?%s", App::instance->agentControl.getAgentSecondaryUrl (media->streamAgentId, NULL, media->htmlPlayerPath).c_str (), media->streamId.c_str ()));
}

void MediaUi::configureStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;
	ComboBox *combobox;
	bool show;

	ui = (MediaUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	if (ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (App::instance->rootPanel->width * 0.25f).empty ()) {
		return;
	}

	App::instance->uiStack.suspendMouseHover ();
	show = true;
	if (ActionWindow::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == widgetPtr)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = (ActionWindow *) App::instance->rootPanel->addWidget (new ActionWindow ());
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (uitext->getText (UiTextString::configureStream).capitalized ());
	action->setConfirmButtonText (uitext->getText (UiTextString::apply).uppercased ());
	action->setCloseCallback (MediaUi::configureStreamActionClosed, ui);

	combobox = new ComboBox ();
	combobox->addItem (uitext->getText (UiTextString::normalVideoQualityDescription));
	combobox->addItem (uitext->getText (UiTextString::highVideoQualityDescription));
	combobox->addItem (uitext->getText (UiTextString::lowVideoQualityDescription));
	combobox->addItem (uitext->getText (UiTextString::lowestVideoQualityDescription));
	combobox->setValue (App::instance->prefsMap.find (App::MediaUiVideoQualityKey, uitext->getText (UiTextString::normalVideoQualityDescription)));
	action->addOption (uitext->getText (UiTextString::videoQuality).capitalized (), combobox, uitext->getText (UiTextString::videoQualityDescription));

	action->position.assign (widgetPtr->screenX + widgetPtr->width - action->width, widgetPtr->screenY - action->height);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (widgetPtr);
}

void MediaUi::configureStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiText *uitext;
	ActionWindow *action;
	MediaWindow *media;
	HashMap::Iterator i;
	StdString mediaid, quality;
	Json *params;
	int result, profile, invokecount, errcount;

	ui = (MediaUi *) uiPtr;
	uitext = &(App::instance->uiText);
	action = (ActionWindow *) widgetPtr;
	invokecount = 0;
	errcount = 0;
	if (action->isConfirmed) {
		quality = action->getStringValue (uitext->getText (UiTextString::videoQuality).capitalized (), "");
		if (! quality.empty ()) {
			App::instance->prefsMap.insert (App::MediaUiVideoQualityKey, quality);
		}
		profile = SystemInterface::Constant_DefaultStreamProfile;
		if (quality.equals (uitext->getText (UiTextString::highVideoQualityDescription))) {
			profile = SystemInterface::Constant_CompressedStreamProfile;
		}
		else if (quality.equals (uitext->getText (UiTextString::lowVideoQualityDescription))) {
			profile = SystemInterface::Constant_LowQualityStreamProfile;
		}
		else if (quality.equals (uitext->getText (UiTextString::lowestVideoQualityDescription))) {
			profile = SystemInterface::Constant_LowestQualityStreamProfile;
		}

		i = ui->selectedMediaMap.begin ();
		while (ui->selectedMediaMap.hasNext (&i)) {
			mediaid = ui->selectedMediaMap.next (&i);
			media = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
			if (media) {
				params = new Json ();
				params->set ("mediaId", media->mediaId);
				params->set ("mediaServerAgentId", media->agentId);
				params->set ("streamName", media->mediaName);
				params->set ("mediaWidth", media->mediaWidth);
				params->set ("mediaHeight", media->mediaHeight);
				params->set ("profile", profile);

				// TODO: Populate the mediaUrl field (to allow streams to be populated on a stream server other than the source agent)
				//  params->set ("mediaUrl", url);

				ui->retain ();
				result = App::instance->agentControl.invokeCommand (media->agentId, App::instance->createCommand (SystemInterface::Command_ConfigureMediaStream, SystemInterface::Constant_Stream, params), MediaUi::configureMediaStreamComplete, ui);
				if (result != Result::Success) {
					ui->release ();
					++errcount;
					Log::debug ("Failed to invoke ConfigureMediaStream command; err=%i agentId=%s", result, media->agentId.c_str ());
				}
				else {
					++invokecount;
				}
			}
		}
	}
	ui->actionWidget.clear ();

	if (invokecount <= 0) {
		if (errcount > 0) {
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		}
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeConfigureMediaStreamMessage));
	}
}

void MediaUi::configureMediaStreamComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	MediaUi *ui;
	RecordStore *store;
	SystemInterface *interface;
	StdString taskid, mediaid, streamid;
	Json *streamitem;

	ui = (MediaUi *) uiPtr;
	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);

	taskid = interface->getCommandStringParam (responseCommand, "taskId", "");
	mediaid = interface->getCommandStringParam (invokeCommand, "mediaId", "");
	if ((! taskid.empty ()) && (! mediaid.empty ())) {
		store->lock ();
		streamitem = store->findRecord (MediaWindow::matchStreamSourceId, &mediaid);
		if (streamitem) {
			streamid = interface->getCommandRecordId (streamitem);
			if (! streamid.empty ()) {
				store->removeRecord (streamid);
			}
		}
		store->unlock ();
	}
	if (! streamid.empty ()) {
		App::instance->agentControl.refreshAgentStatus (agentId);
	}

	ui->release ();
}

void MediaUi::cacheStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *media;
	RecordStore *store;
	SystemInterface *interface;
	HashMap::Iterator i, j;
	StdString mediaid, agentid;
	Json *params, *streamitem, *agentstatus, serverstatus;
	int result, invokecount, errcount;

	ui = (MediaUi *) uiPtr;
	if (ui->selectedMonitorMap.empty () || ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (App::instance->rootPanel->width * 0.25f, true).empty ()) {
		return;
	}

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	invokecount = 0;
	errcount = 0;
	i = ui->selectedMediaMap.begin ();
	while (ui->selectedMediaMap.hasNext (&i)) {
		mediaid = ui->selectedMediaMap.next (&i);
		media = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
		if (media && (! media->streamId.empty ())) {
			params = NULL;
			store->lock ();
			streamitem = store->findRecord (media->streamId, SystemInterface::CommandId_StreamItem);
			agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &(media->streamAgentId));
			if (streamitem && agentstatus && interface->getCommandObjectParam (agentstatus, "streamServerStatus", &serverstatus)) {
				params = new Json ();
				params->set ("streamUrl", App::instance->agentControl.getAgentSecondaryUrl (media->streamAgentId, NULL, media->hlsStreamPath));
				params->set ("thumbnailUrl", App::instance->agentControl.getAgentSecondaryUrl (media->streamAgentId, NULL, serverstatus.getString ("thumbnailPath", "")));
				params->set ("streamId", media->streamId);
				params->set ("streamName", interface->getCommandStringParam (streamitem, "name", ""));
				params->set ("duration", interface->getCommandNumberParam (streamitem, "duration", (float) 0.0f));
				params->set ("width", interface->getCommandNumberParam (streamitem, "width", (int) 0));
				params->set ("height", interface->getCommandNumberParam (streamitem, "height", (int) 0));
				params->set ("bitrate", interface->getCommandNumberParam (streamitem, "bitrate", (int64_t) 0));
				params->set ("frameRate", interface->getCommandNumberParam (streamitem, "frameRate", (float) 0.0f));
			}
			store->unlock ();

			if (! params) {
				++errcount;
			}
			else {
				j = ui->selectedMonitorMap.begin ();
				while (ui->selectedMonitorMap.hasNext (&j)) {
					agentid = ui->selectedMonitorMap.next (&j);
					result = App::instance->agentControl.invokeCommand (agentid, App::instance->createCommand (SystemInterface::Command_CreateCacheStream, SystemInterface::Constant_Monitor, params->copy ()));
					if (result != Result::Success) {
						++errcount;
					}
					else {
						++invokecount;
					}
				}

				delete (params);
			}
		}
	}

	if (invokecount <= 0) {
		if (errcount > 0) {
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		}
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeCreateCacheStreamMessage));
	}
}

void MediaUi::deleteStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *media;
	HashMap::Iterator i;
	StdString mediaid;
	Json *params;
	int result, errcount;
	StringList removelist;
	StringList::iterator ri, rend;

	ui = (MediaUi *) uiPtr;
	if (ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (App::instance->rootPanel->width * 0.25f, true).empty ()) {
		return;
	}

	errcount = 0;
	i = ui->selectedMediaMap.begin ();
	while (ui->selectedMediaMap.hasNext (&i)) {
		mediaid = ui->selectedMediaMap.next (&i);
		media = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
		if (media && (! media->streamId.empty ())) {
			params = new Json ();
			params->set ("id", media->streamId);
			result = App::instance->agentControl.invokeCommand (media->streamAgentId, App::instance->createCommand (SystemInterface::Command_RemoveStream, SystemInterface::Constant_Stream, params));
			if (result != Result::Success) {
				++errcount;
				Log::debug ("Failed to invoke RemoveStream command; err=%i agentId=\"%s\"", result, media->streamAgentId.c_str ());
			}
			else {
				removelist.push_back (media->streamId);
			}
		}
	}

	if (removelist.size () <= 0) {
		if (errcount > 0) {
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		}
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeRemoveStreamMessage));

		ri = removelist.begin ();
		rend = removelist.end ();
		while (ri != rend) {
			App::instance->agentControl.recordStore.removeRecord (*ri);
			++ri;
		}
		App::instance->shouldSyncRecordStore = true;
	}
}

void MediaUi::addPlaylistItemButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *media;
	StreamPlaylistWindow *playlist;
	StdString streamurl, thumbnailurl;
	HashMap::Iterator i;
	StdString mediaid;
	int count;
	float startpos;

	ui = (MediaUi *) uiPtr;
	playlist = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
	if ((! playlist) || ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (App::instance->rootPanel->width * 0.25f, true).empty ()) {
		return;
	}

	count = 0;
	i = ui->selectedMediaMap.begin ();
	while (ui->selectedMediaMap.hasNext (&i)) {
		mediaid = ui->selectedMediaMap.next (&i);
		media = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
		if (media && (! media->streamId.empty ())) {
			++count;
			streamurl = App::instance->agentControl.getAgentSecondaryUrl (media->streamAgentId, NULL, media->hlsStreamPath);
			startpos = (float) media->displayTimestamp;
			if (startpos < 0.0f) {
				startpos = 0.0f;
			}
			startpos /= 1000.0f;

			thumbnailurl.assign ("");
			if (! media->streamThumbnailPath.empty ()) {
				thumbnailurl = App::instance->agentControl.getAgentSecondaryUrl (media->streamAgentId, NULL, media->streamThumbnailPath);
			}
			playlist->addItem (streamurl, media->streamId, media->mediaName, startpos, thumbnailurl, media->playThumbnailIndex);
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", App::instance->uiText.getText (UiTextString::streamItemAddedMessage).c_str (), playlist->playlistName.c_str ()));
	}
}

void MediaUi::createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;
	StdString id;

	ui = (MediaUi *) uiPtr;

	playlist = ui->createStreamPlaylistWindow ();
	id = ui->cardView->getAvailableItemId ();
	playlist->itemId.assign (id);
	playlist->setPlaylistName (ui->getAvailablePlaylistName ());
	playlist->setExpanded (true);
	ui->cardView->addItem (playlist, id, MediaUi::ExpandedPlaylistRow);
	playlist->animateNewCard ();
	playlist->setSelected (true);
	ui->cardView->scrollToItem (id);
	App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", App::instance->uiText.getText (UiTextString::streamPlaylistCreatedMessage).c_str (), playlist->playlistName.c_str ()));
}

void MediaUi::deletePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;

	ui = (MediaUi *) uiPtr;
	playlist = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
	if (playlist) {
		ui->cardView->removeItem (playlist->itemId);
		ui->commandPopup.destroyAndClear ();
		ui->commandPopupSource.clear ();
	}
}

std::map<StdString, MediaUi::MediaServerInfo>::iterator MediaUi::getMediaServerInfo (const StdString &agentId) {
	std::map<StdString, MediaUi::MediaServerInfo>::iterator i;

	i = mediaServerMap.find (agentId);
	if (i == mediaServerMap.end ()) {
		mediaServerMap.insert (std::pair<StdString, MediaUi::MediaServerInfo> (agentId, MediaUi::MediaServerInfo ()));
		i = mediaServerMap.find (agentId);
	}

	return (i);
}

StdString MediaUi::getSelectedMonitorNames (float maxWidth) {
	StdString text, id;
	HashMap::Iterator i;
	StringList names;
	int count;

	count = selectedMonitorMap.size ();
	if (count <= 0) {
		return (StdString (""));
	}

	i = selectedMonitorMap.begin ();
	while (selectedMonitorMap.hasNext (&i)) {
		id = selectedMonitorMap.next (&i);
		names.push_back (selectedMonitorMap.find (id, ""));
	}

	names.sort (StringList::compareCaseInsensitiveAscending);
	text.assign (names.join (", "));
	Label::truncateText (&text, UiConfiguration::CaptionFont, maxWidth, StdString::createSprintf ("... (%i)", count));

	return (text);
}

StdString MediaUi::getSelectedMediaNames (float maxWidth, bool isStreamRequired) {
	StdString text, id;
	HashMap::Iterator i;
	MediaWindow *media;
	StringList names;

	i = selectedMediaMap.begin ();
	while (selectedMediaMap.hasNext (&i)) {
		id = selectedMediaMap.next (&i);
		media = MediaWindow::castWidget (cardView->getItem (id));
		if (media) {
			if (isStreamRequired && media->streamId.empty ()) {
				continue;
			}
			names.push_back (selectedMediaMap.find (id, ""));
		}
	}
	if (names.empty ()) {
		return (StdString (""));
	}

	names.sort (StringList::compareCaseInsensitiveAscending);
	text.assign (names.join (", "));
	Label::truncateText (&text, UiConfiguration::CaptionFont, maxWidth, StdString::createSprintf ("... (%i)", (int) names.size ()));

	return (text);
}

void MediaUi::resetExpandToggles () {
	Toggle *toggle;
	int count;

	toggle = (Toggle *) expandAgentsToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (MediaUi::countExpandedAgents, &count);
		toggle->setChecked ((count <= 0), true);
	}

	toggle = (Toggle *) expandPlaylistsToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (MediaUi::countExpandedPlaylists, &count);
		toggle->setChecked ((count <= 0), true);
	}
}

void MediaUi::countExpandedAgents (void *intPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;
	MediaLibraryWindow *medialibrary;
	int *count;

	count = (int *) intPtr;
	if (! count) {
		return;
	}

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor && monitor->isExpanded) {
		++(*count);
		return;
	}

	medialibrary = MediaLibraryWindow::castWidget (widgetPtr);
	if (medialibrary && medialibrary->isExpanded) {
		++(*count);
		return;
	}
}

void MediaUi::countExpandedPlaylists (void *intPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *playlist;
	int *count;

	count = (int *) intPtr;
	if (! count) {
		return;
	}

	playlist = StreamPlaylistWindow::castWidget (widgetPtr);
	if (playlist && playlist->isExpanded) {
		++(*count);
		return;
	}
}
