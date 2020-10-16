/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "MediaUtil.h"
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

const char *MediaUi::ImageSizeKey = "Media_ImageSize";
const char *MediaUi::SortOrderKey = "Media_SortOrder";
const char *MediaUi::SelectedAgentsKey = "Media_SelectedAgents";
const char *MediaUi::ExpandedAgentsKey = "Media_ExpandedAgents";
const char *MediaUi::PlaylistsKey = "Media_Playlists";
const char *MediaUi::ToolbarModeKey = "Media_ToolbarMode";
const char *MediaUi::VideoQualityKey = "Media_VideoQuality";
const char *MediaUi::ShowMediaWithoutStreamsKey = "Media_ShowWithoutStreams";

const int MediaUi::PageSize = 64;
const float MediaUi::TruncateWidthMultiplier = 0.25f;

MediaUi::MediaUi ()
: Ui ()
, toolbarMode (-1)
, playButton (NULL)
, writePlaylistButton (NULL)
, pauseButton (NULL)
, stopButton (NULL)
, configureStreamButton (NULL)
, cacheStreamButton (NULL)
, deleteStreamButton (NULL)
, selectAllButton (NULL)
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
	return (new Chip (App::instance->uiText.getText (UiTextString::Media).capitalized (), sprites.getSprite (MediaUi::BreadcrumbIconSprite)));
}

void MediaUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::MediaUiHelpTitle), uitext->getText (UiTextString::MediaUiHelpText));
	if (mediaServerCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::MediaUiHelpAction1Text), uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
	}
	else if (mediaItemCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::MediaUiHelpAction4Text), uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("media-streaming"));
	}
	else {
		helpWindow->addAction (uitext->getText (UiTextString::MediaUiHelpAction2Text));
		helpWindow->addAction (uitext->getText (UiTextString::MediaUiHelpAction3Text), uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("media-streaming"));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::MediaStreaming).capitalized (), App::getHelpUrl ("media-streaming"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

int MediaUi::doLoad () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	HashMap *prefs;
	Panel *panel;
	Toggle *toggle;
	Button *button;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	mediaServerCount = 0;
	mediaItemCount = 0;
	mediaStreamCount = 0;
	findMediaComplete = false;
	isLoadingMedia = false;
	emptyStateType = -1;

	prefs = App::instance->lockPrefs ();
	isShowingMediaWithoutStreams = prefs->find (MediaUi::ShowMediaWithoutStreamsKey, true);
	mediaSortOrder = prefs->find (MediaUi::SortOrderKey, (int) SystemInterface::Constant_NameSort);
	cardDetail = prefs->find (MediaUi::ImageSizeKey, (int) CardView::MediumDetail);
	App::instance->unlockPrefs ();

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (MediaUi::expandAgentsToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::MinimizeAll).capitalized (), uitext->getText (UiTextString::ExpandAll).capitalized ());
	expandAgentsToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::MediaUiAgentRowHeaderText), panel), StdString (""), MediaUi::AgentToggleRow);

	cardView->setRowReverseSorted (MediaUi::ExpandedAgentRow, true);
	cardView->setRowReverseSorted (MediaUi::ExpandedPlaylistRow, true);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (MediaUi::expandPlaylistsToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::MinimizeAll).capitalized (), uitext->getText (UiTextString::ExpandAll).capitalized ());
	expandPlaylistsToggle.assign (toggle);

	button = (Button *) panel->addWidget (new Button (sprites.getSprite (MediaUi::CreatePlaylistButtonSprite)));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::createPlaylistButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiCreatePlaylistTooltip));

	panel->setLayout (Panel::HorizontalLayout);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::Playlists).capitalized (), panel), StdString (""), MediaUi::PlaylistToggleRow);

	cardView->setRowHeader (MediaUi::EmptyMediaRow, createRowHeaderPanel (uitext->getText (UiTextString::Media).capitalized ()));
	cardView->setRowHeader (MediaUi::MediaRow, createRowHeaderPanel (uitext->getText (UiTextString::Media).capitalized ()));
	cardView->setRowItemMarginSize (MediaUi::MediaRow, 0.0f);
	cardView->setRowSelectionAnimated (MediaUi::MediaRow, true);
	cardView->setRowDetail (MediaUi::MediaRow, cardDetail);
	cardView->setRowItemMarginSize (MediaUi::MediaLoadingRow, uiconfig->marginSize);

	return (Result::Success);
}

void MediaUi::doUnload () {
	searchField.clear ();
	emptyStateWindow.clear ();
	targetMediaWindow.clear ();
	lastSelectedMediaWindow.clear ();
	selectedPlaylistWindow.clear ();
	selectedMonitorMap.clear ();
	selectedMediaMap.clear ();
	expandAgentsToggle.clear ();
	expandPlaylistsToggle.clear ();
	configureStreamSizeIcon.clear ();

	SDL_LockMutex (findMediaStreamsMapMutex);
	findMediaStreamsMap.clear ();
	SDL_UnlockMutex (findMediaStreamsMapMutex);

	SDL_LockMutex (mediaServerMapMutex);
	mediaServerMap.clear ();
	SDL_UnlockMutex (mediaServerMapMutex);
}

void MediaUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::imageSizeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::ThumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::reloadButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::ReloadTooltip));
	toolbar->addRightItem (button);
}

void MediaUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	UiText *uitext;
	HashMap *prefs;
	TextFieldWindow *textfield;
	Button *button;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	textfield = new TextFieldWindow (App::instance->windowWidth * 0.37f, uitext->getText (UiTextString::EnterSearchKeyPrompt));
	textfield->setPadding (uiconfig->paddingSize, 0.0f);
	textfield->setButtonsEnabled (false, false, true, true);
	textfield->valueEditCallback = Widget::EventCallbackContext (MediaUi::searchFieldEdited, this);
	toolbar->addLeftItem (textfield);
	searchField.assign (textfield);

	button = new Button (sprites.getSprite (MediaUi::SearchButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::searchButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiSearchTooltip));
	toolbar->addLeftItem (button);

	button = new Button (sprites.getSprite (MediaUi::SortButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::sortButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiSortTooltip));
	toolbar->addLeftItem (button);

	prefs = App::instance->lockPrefs ();
	toolbarMode = prefs->find (MediaUi::ToolbarModeKey, (int) -1);
	App::instance->unlockPrefs ();
	if (toolbarMode < 0) {
		toolbarMode = MediaUi::MonitorMode;
	}
	setToolbarMode (toolbarMode, true);
}

void MediaUi::doResume () {
	HashMap *prefs;
	JsonList playlists;
	JsonList::iterator i, end;
	StdString id;
	StreamPlaylistWindow *playlist;
	int64_t now;

	App::instance->setNextBackgroundTexturePath ("ui/MediaUi/bg");

	if (searchField.widget) {
		((TextFieldWindow *) searchField.widget)->setValue (searchKey);
	}

	if (! isFirstResumeComplete) {
		prefs = App::instance->lockPrefs ();
		prefs->find (MediaUi::PlaylistsKey, &playlists);
		// TODO: Remove this operation (when transition from the legacy key is no longer needed)
		App::transferJsonListPrefs (prefs, "Media_Playlists", &playlists);
		App::instance->unlockPrefs ();

		now = OsUtil::getTime ();
		i = playlists.begin ();
		end = playlists.end ();
		while (i != end) {
			playlist = createStreamPlaylistWindow ();
			playlist->readState (*i);

			id = cardView->getAvailableItemId ();
			playlist->itemId.assign (id);
			if (playlist->isExpanded) {
				playlist->sortKey.sprintf ("%016llx%s", (long long int) now, playlist->playlistName.lowercased ().c_str ());
				cardView->addItem (playlist, id, MediaUi::ExpandedPlaylistRow, true);
			}
			else {
				playlist->sortKey.assign (playlist->playlistName.lowercased ());
				cardView->addItem (playlist, id, MediaUi::UnexpandedPlaylistRow, true);
			}
			playlist->animateNewCard ();
			if (playlist->isSelected) {
				selectedPlaylistWindow.assign (playlist);
			}
			++i;
		}
	}

	cardView->refresh ();
	resetExpandToggles ();
}

static void doPause_appendPlaylistState (void *jsonListPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *playlist;

	playlist = StreamPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		((JsonList *) jsonListPtr)->push_back (playlist->createState ());
	}
}
static void doPause_appendSelectedAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor && monitor->isSelected) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
	}
}
static void doPause_appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
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
void MediaUi::doPause () {
	HashMap *prefs;
	JsonList playlists;
	StringList ids;

	cardView->processItems (doPause_appendPlaylistState, &playlists);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaUi::PlaylistsKey, &playlists);
	App::instance->unlockPrefs ();

	ids.clear ();
	cardView->processItems (doPause_appendSelectedAgentId, &ids);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaUi::SelectedAgentsKey, &ids);
	App::instance->unlockPrefs ();

	ids.clear ();
	cardView->processItems (doPause_appendExpandedAgentId, &ids);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaUi::ExpandedAgentsKey, &ids);
	prefs->insert (MediaUi::ToolbarModeKey, toolbarMode);
	prefs->insert (MediaUi::ShowMediaWithoutStreamsKey, isShowingMediaWithoutStreams);
	prefs->insert (MediaUi::SortOrderKey, mediaSortOrder);
	App::instance->unlockPrefs ();
}

void MediaUi::doClearPopupWidgets () {
	commandPopup.destroyAndClear ();
	commandPopupSource.clear ();
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
	targetMediaWindow.compact ();
	lastSelectedMediaWindow.compact ();
	selectedPlaylistWindow.compact ();
	expandAgentsToggle.compact ();
	expandPlaylistsToggle.compact ();
	configureStreamSizeIcon.compact ();

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
					params->set ("maxResults", MediaUi::PageSize);
					params->set ("sortOrder", mediaSortOrder);
					App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Media, params), i->first);
					i->second.resultOffset += MediaUi::PageSize;
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
		cardView->setRowHeader (MediaUi::MediaRow, createRowHeaderPanel (uitext->getText (UiTextString::Media).capitalized (), createLoadingIconWindow ()));
		cardView->addItem (createLoadingIconWindow (), StdString (""), MediaUi::MediaLoadingRow);
	}
	else if (isLoadingMedia && (! found)) {
		cardView->setRowHeader (MediaUi::MediaRow, createRowHeaderPanel (uitext->getText (UiTextString::Media).capitalized ()));
		cardView->removeRowItems (MediaUi::MediaLoadingRow);
	}
	isLoadingMedia = found;
}

void MediaUi::doResize () {
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
		unselectAllMedia ();
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
	store->processAgentRecords ("mediaServerStatus", MediaUi::doSyncRecordStore_processMediaServerAgent, this);
	store->processAgentRecords ("monitorServerStatus", MediaUi::doSyncRecordStore_processMonitorAgent, this);

	mediaItemCount = 0;
	mediaStreamCount = 0;
	store->processCommandRecords (SystemInterface::CommandId_MediaItem, MediaUi::doSyncRecordStore_processMediaItem, this);

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
				window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite), uitext->getText (UiTextString::MediaUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::MediaUiEmptyAgentStatusText));
				window->setLink (uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
				window->itemId.assign (cardView->getAvailableItemId ());
				cardView->addItem (window, window->itemId, MediaUi::UnexpandedAgentRow);
				emptyStateWindow.assign (window);
				break;
			}
			case MediaUi::EmptyMediaState: {
				window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite), uitext->getText (UiTextString::MediaUiEmptyMediaStatusTitle), StdString (""), uitext->getText (UiTextString::MediaUiEmptyMediaStatusText));
				window->setLink (uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
				window->itemId.assign (cardView->getAvailableItemId ());
				cardView->addItem (window, window->itemId, MediaUi::EmptyMediaRow);
				emptyStateWindow.assign (window);
				break;
			}
			case MediaUi::EmptyMediaStreamState: {
				window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeStreamIconSprite), uitext->getText (UiTextString::MediaUiEmptyMediaStreamStatusTitle), StdString (""), uitext->getText (UiTextString::MediaUiEmptyMediaStreamStatusText));
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

void MediaUi::doSyncRecordStore_processMediaServerAgent (void *uiPtr, Json *record, const StdString &recordId) {
	MediaUi *ui;
	SystemInterface *interface;
	HashMap *prefs;
	MediaLibraryWindow *medialibrary;
	StringList items;
	int row, pos;

	ui = (MediaUi *) uiPtr;
	interface = &(App::instance->systemInterface);

	++(ui->mediaServerCount);
	if (! ui->cardView->contains (recordId)) {
		medialibrary = new MediaLibraryWindow (recordId);
		medialibrary->menuClickCallback = Widget::EventCallbackContext (MediaUi::mediaLibraryMenuClicked, ui);
		medialibrary->expandStateChangeCallback = Widget::EventCallbackContext (MediaUi::cardExpandStateChanged, ui);

		prefs = App::instance->lockPrefs ();
		prefs->find (MediaUi::ExpandedAgentsKey, &items);
		App::instance->unlockPrefs ();
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

void MediaUi::doSyncRecordStore_processMonitorAgent (void *uiPtr, Json *record, const StdString &recordId) {
	MediaUi *ui;
	SystemInterface *interface;
	HashMap *prefs;
	MonitorWindow *monitor;
	StringList items;
	int row, pos;

	ui = (MediaUi *) uiPtr;
	interface = &(App::instance->systemInterface);

	if (! ui->cardView->contains (recordId)) {
		monitor = new MonitorWindow (recordId);
		monitor->setScreenshotDisplayEnabled (true);
		monitor->setStorageDisplayEnabled (true);
		monitor->setSelectEnabled (true);
		monitor->selectStateChangeCallback = Widget::EventCallbackContext (MediaUi::monitorSelectStateChanged, ui);
		monitor->expandStateChangeCallback = Widget::EventCallbackContext (MediaUi::cardExpandStateChanged, ui);
		monitor->screenshotLoadCallback = Widget::EventCallbackContext (MediaUi::monitorScreenshotLoaded, ui);
		monitor->addActionButton (ui->sprites.getSprite (MediaUi::CacheButtonSprite), App::instance->uiText.getText (UiTextString::ViewMonitorCacheTooltip), Widget::EventCallbackContext (MediaUi::monitorCacheButtonClicked, ui));

		prefs = App::instance->lockPrefs ();
		prefs->find (MediaUi::SelectedAgentsKey, &items);
		if (items.contains (recordId)) {
			monitor->setSelected (true, true);
			ui->selectedMonitorMap.insert (recordId, App::instance->systemInterface.getCommandAgentName (record));
		}
		prefs->find (MediaUi::ExpandedAgentsKey, &items);
		App::instance->unlockPrefs ();

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

void MediaUi::doSyncRecordStore_processMediaItem (void *uiPtr, Json *record, const StdString &recordId) {
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
				media = new MediaWindow (record, &(ui->sprites));
				media->mediaImageClickCallback = Widget::EventCallbackContext (MediaUi::mediaWindowImageClicked, ui);
				media->viewButtonClickCallback = Widget::EventCallbackContext (MediaUi::mediaWindowViewButtonClicked, ui);
				media->browserPlayButtonClickCallback = Widget::EventCallbackContext (MediaUi::mediaWindowBrowserPlayButtonClicked, ui);
				media->selectStateChangeCallback = Widget::EventCallbackContext (MediaUi::mediaWindowSelectStateChanged, ui);
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
			info->second.resultOffset = MediaUi::PageSize;
			params = new Json ();
			params->set ("searchKey", searchKey);
			params->set ("maxResults", MediaUi::PageSize);
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

	ui = (MediaUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::sortButtonClicked)) {
		return;
	}

	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::MediaUiShowMediaWithoutStreamsAction), uiconfig->coreSprites.getSprite (UiConfiguration::VisibilityOnButtonSprite), MediaUi::showMediaWithoutStreamsActionClicked, ui, 0, (! ui->isShowingMediaWithoutStreams));
	menu->addItem (uitext->getText (UiTextString::SortByName).capitalized (), ui->sprites.getSprite (MediaUi::SortButtonSprite), MediaUi::sortByNameActionClicked, ui, 2, ui->mediaSortOrder == SystemInterface::Constant_NameSort);
	menu->addItem (uitext->getText (UiTextString::SortByNewest).capitalized (), ui->sprites.getSprite (MediaUi::SortButtonSprite), MediaUi::sortByNewestActionClicked, ui, 2, ui->mediaSortOrder == SystemInterface::Constant_NewestSort);

	ui->showActionPopup (menu, widgetPtr, MediaUi::sortButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

static void showMediaWithoutStreamsActionClicked_appendMediaId (void *stringListPtr, Widget *widgetPtr) {
	StringList *idlist;
	MediaWindow *media;

	idlist = (StringList *) stringListPtr;
	media = MediaWindow::castWidget (widgetPtr);
	if (media && media->streamId.empty ()) {
		idlist->push_back (media->mediaId);
	}
}
void MediaUi::showMediaWithoutStreamsActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StringList idlist;
	StringList::iterator i, end;

	ui = (MediaUi *) uiPtr;
	ui->isShowingMediaWithoutStreams = (! ui->isShowingMediaWithoutStreams);

	if (! ui->isShowingMediaWithoutStreams) {
		ui->cardView->processRowItems (MediaUi::MediaRow, showMediaWithoutStreamsActionClicked_appendMediaId, &idlist);
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
		i->second.resultOffset = MediaUi::PageSize;
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
	params->set ("maxResults", MediaUi::PageSize);
	params->set ("sortOrder", mediaSortOrder);
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Media, params));
}

void MediaUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Menu *menu;

	ui = (MediaUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::imageSizeButtonClicked)) {
		return;
	}

	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::Small).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), MediaUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (uitext->getText (UiTextString::Medium).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), MediaUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (uitext->getText (UiTextString::Large).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), MediaUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);

	ui->showActionPopup (menu, widgetPtr, MediaUi::imageSizeButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::BottomOfAlignment);
}

void MediaUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	HashMap *prefs;
	int detail;

	ui = (MediaUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaUi::MediaRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void MediaUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	HashMap *prefs;
	int detail;

	ui = (MediaUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaUi::MediaRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void MediaUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	HashMap *prefs;
	int detail;

	ui = (MediaUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaUi::MediaRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

static void reloadButtonClicked_processItem (void *uiPtr, Widget *widgetPtr) {
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
void MediaUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	SDL_LockMutex (ui->findMediaStreamsMapMutex);
	ui->findMediaStreamsMap.clear ();
	SDL_UnlockMutex (ui->findMediaStreamsMapMutex);

	ui->cardView->processItems (reloadButtonClicked_processItem, ui);
	ui->loadSearchResults ();
}

StreamPlaylistWindow *MediaUi::createStreamPlaylistWindow () {
	StreamPlaylistWindow *playlist;

	playlist = new StreamPlaylistWindow (&sprites);
	playlist->selectStateChangeCallback = Widget::EventCallbackContext (MediaUi::playlistSelectStateChanged, this);
	playlist->expandStateChangeCallback = Widget::EventCallbackContext (MediaUi::cardExpandStateChanged, this);
	playlist->renameClickCallback = Widget::EventCallbackContext (MediaUi::playlistRenameActionClicked, this);
	playlist->listChangeCallback = Widget::EventCallbackContext (MediaUi::playlistItemsChanged, this);
	playlist->removeClickCallback = Widget::EventCallbackContext (MediaUi::playlistRemoveActionClicked, this);
	playlist->addItemClickCallback = Widget::EventCallbackContext (MediaUi::playlistAddItemActionClicked, this);
	playlist->addItemMouseEnterCallback = Widget::EventCallbackContext (MediaUi::playlistAddItemMouseEntered, this);
	playlist->addItemMouseExitCallback = Widget::EventCallbackContext (MediaUi::playlistAddItemMouseExited, this);

	return (playlist);
}

static void getAvailablePlaylistName_matchName (void *stringPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *playlist;
	StdString *name;

	playlist = StreamPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		name = (StdString *) stringPtr;
		if (name->lowercased ().equals (playlist->playlistName.lowercased ())) {
			name->assign ("");
		}
	}
}
StdString MediaUi::getAvailablePlaylistName (const StdString &baseName) {
	StdString base, name;
	int i;

	if (baseName.empty ()) {
		base.assign (App::instance->uiText.getText (UiTextString::Playlist).capitalized ());
	}
	else {
		base.assign (baseName);
	}
	name.assign (base);
	cardView->processItems (getAvailablePlaylistName_matchName, &name);
	if (name.empty ()) {
		i = 2;
		while (true) {
			name.sprintf ("%s %i", base.c_str (), i);
			cardView->processItems (getAvailablePlaylistName_matchName, &name);
			if (! name.empty ()) {
				break;
			}
			++i;
		}
	}

	return (name);
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
		ui->targetMediaWindow.assign (target);
		streamitemui = new StreamItemUi (target->streamId, target->mediaName);
		streamitemui->thumbnailClickCallback = Widget::EventCallbackContext (MediaUi::itemUiThumbnailClicked, ui);
		App::instance->uiStack.pushUi (streamitemui);
	}
	else {
		if (target->hasThumbnails ()) {
			ui->targetMediaWindow.assign (target);
			mediaitemui = new MediaItemUi (target->mediaId, target->mediaName);
			mediaitemui->removeMediaCallback = Widget::EventCallbackContext (MediaUi::mediaItemUiMediaRemoved, ui);
			App::instance->uiStack.pushUi (mediaitemui);
		}
	}
}

void MediaUi::mediaWindowBrowserPlayButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaWindow *media;
	int result;

	media = (MediaWindow *) widgetPtr;
	if (media->streamAgentId.empty () || media->streamId.empty () || media->htmlPlayerPath.empty ()) {
		return;
	}
	result = OsUtil::openUrl (StdString::createSprintf ("%s?%s", App::instance->agentControl.getAgentSecondaryUrl (media->streamAgentId, NULL, media->htmlPlayerPath).c_str (), media->streamId.urlEncoded ().c_str ()));
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::LaunchWebBrowserError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::LaunchedWebBrowser).capitalized ());
	}
}

void MediaUi::itemUiThumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaThumbnailWindow *thumbnail;
	MediaWindow *target;

	ui = (MediaUi *) uiPtr;
	thumbnail = MediaThumbnailWindow::castWidget (widgetPtr);
	target = MediaWindow::castWidget (ui->targetMediaWindow.widget);
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
	target = MediaWindow::castWidget (ui->targetMediaWindow.widget);
	if (! target) {
		return;
	}

	ui->cardView->removeItem (target->mediaId);
	ui->targetMediaWindow.clear ();
}

void MediaUi::mediaWindowSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *media;
	StdString mediaid;
	HashMap::Iterator i;

	ui = (MediaUi *) uiPtr;
	media = (MediaWindow *) widgetPtr;
	if (media->isSelected) {
		if (ui->toolbarMode == MediaUi::MonitorMode) {
			ui->unselectAllMedia ();
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

	ui->clearPopupWidgets ();
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
		ui->clearPopupWidgets ();
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
		ui->clearPopupWidgets ();
		ui->cardView->refresh ();
		return;
	}

	playlist = StreamPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		if (playlist->isExpanded) {
			playlist->sortKey.sprintf ("%016llx%s", (long long int) OsUtil::getTime (), playlist->playlistName.lowercased ().c_str ());
			ui->cardView->setItemRow (playlist->itemId, MediaUi::ExpandedPlaylistRow, true);
		}
		else {
			playlist->sortKey.assign (playlist->playlistName.lowercased ());
			ui->cardView->setItemRow (playlist->itemId, MediaUi::UnexpandedPlaylistRow, true);
		}
		playlist->resetInputState ();
		playlist->animateNewCard ();
		ui->resetExpandToggles ();
		ui->clearPopupWidgets ();
		ui->cardView->refresh ();
		return;
	}
}

static void expandAgentsToggleStateChanged_appendAgentId (void *stringListPtr, Widget *widgetPtr) {
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
	ui->cardView->processItems (expandAgentsToggleStateChanged_appendAgentId, &idlist);
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

static void expandPlaylistsToggleStateChanged_appendPlaylistId (void *stringListPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *playlist;

	playlist = StreamPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		((StringList *) stringListPtr)->push_back (playlist->itemId);
	}
}
void MediaUi::expandPlaylistsToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	Toggle *toggle;
	StreamPlaylistWindow *playlist;
	StringList idlist;
	StringList::iterator i, end;
	int64_t now;

	ui = (MediaUi *) uiPtr;
	toggle = (Toggle *) ui->expandPlaylistsToggle.widget;
	if (! toggle) {
		return;
	}

	now = OsUtil::getTime ();
	ui->cardView->processItems (expandPlaylistsToggleStateChanged_appendPlaylistId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		playlist = StreamPlaylistWindow::castWidget (ui->cardView->getItem (*i));
		if (playlist) {
			playlist->setExpanded (! toggle->isChecked, true);
			if (playlist->isExpanded) {
				playlist->sortKey.sprintf ("%016llx%s", (long long int) now, playlist->playlistName.lowercased ().c_str ());
				ui->cardView->setItemRow (playlist->itemId, MediaUi::ExpandedPlaylistRow, true);
			}
			else {
				playlist->sortKey.assign (playlist->playlistName.lowercased ());
				ui->cardView->setItemRow (playlist->itemId, MediaUi::UnexpandedPlaylistRow, true);
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

void MediaUi::mediaLibraryMenuClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	MediaLibraryWindow *medialibrary;
	Menu *menu;

	ui = (MediaUi *) uiPtr;
	medialibrary = (MediaLibraryWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	if (ui->clearActionPopup (widgetPtr, MediaUi::mediaLibraryMenuClicked)) {
		return;
	}

	menu = new Menu ();
	menu->addItem (uitext->getText (UiTextString::ScanForMedia).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::UpdateButtonSprite), MediaUi::mediaLibraryScanActionClicked, ui);
	menu->isClickDestroyEnabled = true;

	ui->showActionPopup (menu, widgetPtr, MediaUi::mediaLibraryMenuClicked, medialibrary->getMenuButtonScreenRect (), Ui::LeftEdgeAlignment, Ui::BottomOfAlignment);
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
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::ScanForMediaStartedMessage));
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
	ui->clearPopupWidgets ();
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
	textfield = new TextFieldWindow (target->windowWidth, uitext->getText (UiTextString::EnterPlaylistNamePrompt));
	textfield->setValue (target->playlistName);
	textfield->valueEditCallback = Widget::EventCallbackContext (MediaUi::playlistNameEdited, ui);
	textfield->enterButtonClickCallback = Widget::EventCallbackContext (MediaUi::playlistNameEditEnterButtonClicked, ui);
	textfield->setFillBg (true, uiconfig->lightPrimaryColor);
	textfield->setButtonsEnabled (true, true, true, true);
	textfield->shouldSkipTextClearCallbacks = true;
	textfield->assignKeyFocus ();

	ui->showActionPopup (textfield, widgetPtr, MediaUi::playlistRenameActionClicked, widgetPtr->getScreenRect (), Ui::LeftEdgeAlignment, Ui::TopEdgeAlignment);
}

void MediaUi::playlistNameEdited (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;
	TextFieldWindow *textfield;

	ui = (MediaUi *) uiPtr;
	textfield = TextFieldWindow::castWidget (ui->actionWidget.widget);
	playlist = StreamPlaylistWindow::castWidget (ui->actionTarget.widget);
	if ((! textfield) || (! playlist)) {
		return;
	}

	playlist->setPlaylistName (ui->getAvailablePlaylistName (textfield->getValue ()));
	playlist->sortKey.assign (playlist->playlistName.lowercased ());
	ui->clearPopupWidgets ();
}

void MediaUi::playlistNameEditEnterButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->clearPopupWidgets ();
}

void MediaUi::playlistRemoveActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;

	ui = (MediaUi *) uiPtr;
	playlist = (StreamPlaylistWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	if (ui->clearActionPopup (playlist, MediaUi::playlistRemoveActionClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (Label::getTruncatedText (playlist->playlistName, UiConfiguration::TitleFont, playlist->width * 0.34f, Label::DotTruncateSuffix));
	action->setDescriptionText (uitext->getText (UiTextString::RemovePlaylistDescription));
	action->closeCallback = Widget::EventCallbackContext (MediaUi::removePlaylistActionClosed, ui);

	ui->showActionPopup (action, playlist, MediaUi::playlistRemoveActionClicked, playlist->getRemoveButtonScreenRect (), Ui::LeftEdgeAlignment, Ui::TopOfAlignment);
}

void MediaUi::removePlaylistActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	StreamPlaylistWindow *playlist;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}
	playlist = StreamPlaylistWindow::castWidget (ui->actionTarget.widget);
	if (! playlist) {
		return;
	}

	ui->cardView->removeItem (playlist->itemId);
	ui->resetExpandToggles ();
}

void MediaUi::playlistAddItemActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *media;
	StreamPlaylistWindow *playlist;
	StdString streamurl, thumbnailurl;
	HashMap::Iterator i;
	StdString mediaid;
	int count;
	float startpos;

	ui = (MediaUi *) uiPtr;
	playlist = (StreamPlaylistWindow *) widgetPtr;
	if (ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (true).empty ()) {
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

	ui->clearPopupWidgets ();
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else if (count == 1) {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", playlist->playlistName.c_str (), App::instance->uiText.getText (UiTextString::AddedPlaylistItem).c_str ()));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s (%i)", playlist->playlistName.c_str (), App::instance->uiText.getText (UiTextString::AddedPlaylistItems).c_str (), count));
	}
}

void MediaUi::playlistAddItemMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	StdString text;
	Color color;

	ui = (MediaUi *) uiPtr;
	playlist = (StreamPlaylistWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	ui->commandPopup.destroyAndClear ();
	ui->commandPopupSource.assign (widgetPtr);
	panel = (Panel *) App::instance->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha));
	panel->setBorder (true, Color (uiconfig->darkBackgroundColor.r, uiconfig->darkBackgroundColor.g, uiconfig->darkBackgroundColor.b, uiconfig->overlayWindowAlpha));

	label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::AddPlaylistItems).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));
	label->setPadding (0.0f, 0.0f);

	text.assign (playlist->playlistName);
	Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, Label::DotTruncateSuffix);

	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setRightAligned (true);

	text.assign (ui->getSelectedMediaNames (true));
	color.assign (uiconfig->primaryTextColor);
	if (text.empty ()) {
		text.assign (uitext->getText (UiTextString::MediaUiNoStreamableMediaSelectedPrompt));
		color.assign (uiconfig->errorTextColor);
	}
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setRightAligned (true);

	panel->setLayout (Panel::VerticalRightJustifiedLayout);
	ui->assignPopupPosition (panel, playlist->getAddItemButtonScreenRect (), Ui::RightOfAlignment, Ui::YCenteredAlignment);
}

void MediaUi::playlistAddItemMouseExited (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	if (ui->commandPopupSource.widget == widgetPtr) {
		ui->commandPopup.destroyAndClear ();
		ui->commandPopupSource.clear ();
	}
}

void MediaUi::modeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiText *uitext;
	Menu *menu;

	ui = (MediaUi *) uiPtr;
	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::modeButtonClicked)) {
		return;
	}

	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::PlayStreams).capitalized (), ui->sprites.getSprite (MediaUi::PlayButtonSprite), MediaUi::monitorModeActionClicked, ui, 0, ui->toolbarMode == MediaUi::MonitorMode);
	menu->addItem (uitext->getText (UiTextString::ManageStreams).capitalized (), ui->sprites.getSprite (MediaUi::ConfigureStreamButtonSprite), MediaUi::streamModeActionClicked, ui, 0, ui->toolbarMode == MediaUi::StreamMode);

	ui->showActionPopup (menu, widgetPtr, MediaUi::modeButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void MediaUi::monitorModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	if (ui->toolbarMode != MediaUi::MonitorMode) {
		ui->unselectAllMedia ();
		ui->setToolbarMode (MediaUi::MonitorMode);
	}
}

void MediaUi::streamModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	if (ui->toolbarMode != MediaUi::StreamMode) {
		ui->unselectAllMedia ();
		ui->setToolbarMode (MediaUi::StreamMode);
	}
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
	configureStreamButton = NULL;
	cacheStreamButton = NULL;
	deleteStreamButton = NULL;
	selectAllButton = NULL;

	button = new Button (uiconfig->coreSprites.getSprite (UiConfiguration::ToolsButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::modeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::ToolbarModeButtonTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (button);

	toolbarMode = mode;
	switch (toolbarMode) {
		case MediaUi::MonitorMode: {
			playButton = new Button (sprites.getSprite (MediaUi::PlayButtonSprite));
			playButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::playButtonClicked, this);
			playButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			playButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			playButton->setInverseColor (true);
			playButton->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiPlayTooltip).capitalized (), Widget::LeftAlignment);
			playButton->shortcutKey = SDLK_F4;
			toolbar->addRightItem (playButton);

			writePlaylistButton = new Button (sprites.getSprite (MediaUi::WritePlaylistButtonSprite));
			writePlaylistButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::writePlaylistButtonClicked, this);
			writePlaylistButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			writePlaylistButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			writePlaylistButton->setInverseColor (true);
			writePlaylistButton->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiWritePlaylistTooltip), Widget::LeftAlignment);
			writePlaylistButton->shortcutKey = SDLK_F3;
			toolbar->addRightItem (writePlaylistButton);

			pauseButton = new Button (sprites.getSprite (MediaUi::PauseButtonSprite));
			pauseButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::pauseButtonClicked, this);
			pauseButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			pauseButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			pauseButton->setInverseColor (true);
			pauseButton->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiPauseTooltip), Widget::LeftAlignment);
			pauseButton->shortcutKey = SDLK_F2;
			toolbar->addRightItem (pauseButton);

			stopButton = new Button (sprites.getSprite (MediaUi::StopButtonSprite));
			stopButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::stopButtonClicked, this);
			stopButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			stopButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			stopButton->setInverseColor (true);
			stopButton->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiStopTooltip), Widget::LeftAlignment);
			stopButton->shortcutKey = SDLK_F1;
			toolbar->addRightItem (stopButton);
			break;
		}
		case MediaUi::StreamMode: {
			selectAllButton = new Button (uiconfig->coreSprites.getSprite (UiConfiguration::StarHalfButtonSprite));
			selectAllButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::selectAllButtonClicked, this);
			selectAllButton->setInverseColor (true);
			selectAllButton->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiSelectAllTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (selectAllButton);

			configureStreamButton = new Button (sprites.getSprite (MediaUi::ConfigureStreamButtonSprite));
			configureStreamButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::configureStreamButtonClicked, this);
			configureStreamButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			configureStreamButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			configureStreamButton->setInverseColor (true);
			configureStreamButton->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiConfigureStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (configureStreamButton);

			cacheStreamButton = new Button (sprites.getSprite (MediaUi::CacheButtonSprite));
			cacheStreamButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::cacheStreamButtonClicked, this);
			cacheStreamButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			cacheStreamButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			cacheStreamButton->setInverseColor (true);
			cacheStreamButton->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiAddCacheStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (cacheStreamButton);

			deleteStreamButton = new Button (uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite));
			deleteStreamButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::deleteStreamButtonClicked, this);
			deleteStreamButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			deleteStreamButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			deleteStreamButton->setInverseColor (true);
			deleteStreamButton->setMouseHoverTooltip (uitext->getText (UiTextString::MediaUiDeleteStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (deleteStreamButton);
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
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::ConfigureStream).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMediaNames (false, true));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::MediaUiNoMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallMediaIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->deleteStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::DeleteStream).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMediaNames (true));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::MediaUiNoStreamableMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->cacheStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::CacheStream).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->getSelectedMediaNames (true));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::MediaUiNoStreamableMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->playButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::Play).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		media = MediaWindow::castWidget (ui->lastSelectedMediaWindow.widget);
		if ((! media) || media->hlsStreamPath.empty ()) {
			text.assign (uitext->getText (UiTextString::MediaUiNoStreamableMediaSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (media->mediaName);
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, Label::DotTruncateSuffix);
			text.appendSprintf (" %s", OsUtil::getDurationString ((media->displayTimestamp > 0.0f) ? (int64_t) media->displayTimestamp : 0, OsUtil::HoursUnit).c_str ());
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->writePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::RunPlaylist).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		playlist = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
		if (! playlist) {
			text.assign (uitext->getText (UiTextString::NoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else if (playlist->getItemCount () <= 0) {
			text.assign (uitext->getText (UiTextString::EmptyPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (playlist->playlistName);
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, Label::DotTruncateSuffix);
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->stopButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::Stop).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->pauseButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (StdString::createSprintf ("%s / %s", uitext->getText (UiTextString::Pause).capitalized ().c_str (), uitext->getText (UiTextString::Resume).c_str ()), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
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
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokePlayMediaMessage));
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

	count = App::instance->agentControl.invokeCommand (&idlist, playlist->createCommand ());
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokeCreateStreamPlaylistMessage));
	}
}

void MediaUi::pauseButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StringList idlist;
	int count;

	ui = (MediaUi *) uiPtr;
	ui->selectedMonitorMap.getKeys (&idlist, true);
	if (idlist.empty ()) {
		return;
	}

	count = App::instance->agentControl.invokeCommand (&idlist, App::instance->createCommand (SystemInterface::Command_PauseMedia, SystemInterface::Constant_Monitor));
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokePauseMediaMessage));
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
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokeClearDisplayMessage));
	}
}

void MediaUi::configureStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	HashMap *prefs;
	ActionWindow *action;
	ComboBox *combobox;
	Panel *panel;
	IconLabelWindow *icon;
	int count, profile;

	ui = (MediaUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	if (ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (false, true).empty ()) {
		return;
	}

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::configureStreamButtonClicked)) {
		return;
	}

	prefs = App::instance->lockPrefs ();
	profile = prefs->find (MediaUi::VideoQualityKey, SystemInterface::Constant_DefaultStreamProfile);
	App::instance->unlockPrefs ();

	action = new ActionWindow ();
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (uitext->getText (UiTextString::ConfigureStream).capitalized ());
	action->setConfirmTooltipText (uitext->getText (UiTextString::Apply).capitalized ());
	action->optionChangeCallback = Widget::EventCallbackContext (MediaUi::configureStreamOptionChanged, ui);
	action->closeCallback = Widget::EventCallbackContext (MediaUi::configureStreamActionClosed, ui);

	combobox = new ComboBox ();
	combobox->addItem (MediaUtil::getStreamProfileDescription (SystemInterface::Constant_DefaultStreamProfile));
	combobox->addItem (MediaUtil::getStreamProfileDescription (SystemInterface::Constant_CompressedStreamProfile));
	combobox->addItem (MediaUtil::getStreamProfileDescription (SystemInterface::Constant_LowQualityStreamProfile));
	combobox->addItem (MediaUtil::getStreamProfileDescription (SystemInterface::Constant_LowestQualityStreamProfile));
	combobox->setValue (MediaUtil::getStreamProfileDescription (profile));
	action->addOption (uitext->getText (UiTextString::VideoQuality).capitalized (), combobox, uitext->getText (UiTextString::VideoQualityDescription));

	panel = new Panel ();
	count = ui->getSelectedCreateStreamCount ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::ConfigureStreamItemCountTooltip, UiTextString::ConfigureStreamItemsCountTooltip));

	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::StorageIconSprite), OsUtil::getByteCountDisplayString (ui->getSelectedCreateStreamSize (profile)), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setMouseHoverTooltip (uitext->getText (UiTextString::ConfigureStreamByteCountTooltip));
	ui->configureStreamSizeIcon.assign (icon);
	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, MediaUi::configureStreamButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void MediaUi::configureStreamOptionChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	UiText *uitext;
	IconLabelWindow *icon;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	uitext = &(App::instance->uiText);
	icon = (IconLabelWindow *) ui->configureStreamSizeIcon.widget;
	if (! icon) {
		return;
	}
	icon->setText (OsUtil::getByteCountDisplayString (ui->getSelectedCreateStreamSize (MediaUtil::getStreamProfile (action->getStringValue (uitext->getText (UiTextString::VideoQuality).capitalized (), "")))));
	action->refresh ();
}

void MediaUi::configureStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiText *uitext;
	HashMap *prefs;
	ActionWindow *action;
	MediaWindow *media;
	HashMap::Iterator i;
	StdString mediaid;
	Json *params;
	int result, profile, invokecount, errcount;

	ui = (MediaUi *) uiPtr;
	uitext = &(App::instance->uiText);
	action = (ActionWindow *) widgetPtr;
	invokecount = 0;
	errcount = 0;
	if (action->isConfirmed) {
		profile = MediaUtil::getStreamProfile (action->getStringValue (uitext->getText (UiTextString::VideoQuality).capitalized (), ""));
		prefs = App::instance->lockPrefs ();
		if (profile != SystemInterface::Constant_DefaultStreamProfile) {
			prefs->insert (MediaUi::VideoQualityKey, profile);
		}
		else {
			prefs->remove (MediaUi::VideoQualityKey);
		}
		App::instance->unlockPrefs ();

		i = ui->selectedMediaMap.begin ();
		while (ui->selectedMediaMap.hasNext (&i)) {
			mediaid = ui->selectedMediaMap.next (&i);
			media = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
			if (media && media->isCreateStreamAvailable) {
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

	if (invokecount <= 0) {
		if (errcount > 0) {
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
		}
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokeConfigureMediaStreamMessage));
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
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;
	IconLabelWindow *icon;
	Panel *panel;
	int count;

	ui = (MediaUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	if (ui->selectedMonitorMap.empty () || ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (true).empty ()) {
		return;
	}

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::cacheStreamButtonClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (uitext->getText (UiTextString::CacheStreams).capitalized ());
	action->setDescriptionText (uitext->getText (UiTextString::CacheStreamsDescription));
	action->closeCallback = Widget::EventCallbackContext (MediaUi::cacheStreamActionClosed, ui);

	panel = new Panel ();
	count = (int) ui->selectedMonitorMap.size ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::MonitorSelected, UiTextString::MonitorsSelected));

	count = ui->getSelectedStreamCount ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::CacheStreamItemCountTooltip, UiTextString::CacheStreamItemsCountTooltip));

	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::StorageIconSprite), OsUtil::getByteCountDisplayString (ui->getSelectedStreamSize ()), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setMouseHoverTooltip (uitext->getText (UiTextString::CacheStreamByteCountTooltip));
	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, MediaUi::cacheStreamButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void MediaUi::cacheStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	MediaWindow *media;
	RecordStore *store;
	SystemInterface *interface;
	HashMap::Iterator i, j;
	StdString mediaid, agentid;
	Json *params, *streamitem, *agentstatus, serverstatus;
	int result, invokecount, errcount;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if ((! action->isConfirmed) || ui->selectedMonitorMap.empty () || ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (true).empty ()) {
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
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
		}
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokeCreateCacheStreamMessage));
	}
}

void MediaUi::deleteStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;
	IconLabelWindow *icon;
	Panel *panel;
	int count;

	ui = (MediaUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	if (ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (true).empty ()) {
		return;
	}

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::deleteStreamButtonClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (uitext->getText (UiTextString::DeleteStreamTitle));
	action->setDescriptionText (uitext->getText (UiTextString::DeleteStreamDescription));
	action->closeCallback = Widget::EventCallbackContext (MediaUi::deleteStreamActionClosed, ui);

	panel = new Panel ();
	count = ui->getSelectedStreamCount ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::DeleteStreamItemCountTooltip, UiTextString::DeleteStreamItemsCountTooltip));

	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::StorageIconSprite), OsUtil::getByteCountDisplayString (ui->getSelectedStreamSize ()), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setMouseHoverTooltip (uitext->getText (UiTextString::DeleteStreamByteCountTooltip));

	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, MediaUi::deleteStreamButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void MediaUi::deleteStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	MediaWindow *media;
	HashMap::Iterator i;
	StdString mediaid;
	Json *params;
	int result, errcount;
	StringList removelist;
	StringList::iterator ri, rend;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if ((! action->isConfirmed) || ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (true).empty ()) {
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
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
		}
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokeRemoveStreamMessage));

		ri = removelist.begin ();
		rend = removelist.end ();
		while (ri != rend) {
			App::instance->agentControl.recordStore.removeRecord (*ri);
			++ri;
		}
		App::instance->shouldSyncRecordStore = true;
	}
}

void MediaUi::selectAllButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	if (ui->selectedMediaMap.empty ()) {
		ui->selectAllMedia ();
	}
	else {
		ui->unselectAllMedia ();
	}
}

void MediaUi::createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;

	ui = (MediaUi *) uiPtr;
	playlist = ui->createStreamPlaylistWindow ();
	playlist->itemId.assign (ui->cardView->getAvailableItemId ());
	playlist->setPlaylistName (ui->getAvailablePlaylistName ());
	playlist->setExpanded (true);
	ui->cardView->addItem (playlist, playlist->itemId, MediaUi::ExpandedPlaylistRow);

	playlist->setSelected (true);
	playlist->animateNewCard ();
	ui->cardView->scrollToItem (playlist->itemId);
	ui->cardView->refresh ();
	ui->resetExpandToggles ();
	App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", App::instance->uiText.getText (UiTextString::CreatedPlaylist).capitalized ().c_str (), playlist->playlistName.c_str ()));
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

StdString MediaUi::getSelectedMonitorNames () {
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
	Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * MediaUi::TruncateWidthMultiplier, StdString::createSprintf ("... (%i)", count));

	return (text);
}

StdString MediaUi::getSelectedMediaNames (bool isStreamRequired, bool isCreateStreamRequired) {
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
			if (isCreateStreamRequired && (! media->isCreateStreamAvailable)) {
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
	Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * MediaUi::TruncateWidthMultiplier, StdString::createSprintf ("... (%i)", (int) names.size ()));

	return (text);
}

void MediaUi::selectAllMedia () {
	selectedMediaMap.clear ();
	lastSelectedMediaWindow.clear ();
	cardView->processRowItems (MediaUi::MediaRow, MediaUi::selectMediaWindow, this);
}

void MediaUi::selectMediaWindow (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *media;

	ui = (MediaUi *) uiPtr;
	media = MediaWindow::castWidget (widgetPtr);
	if (media) {
		media->setSelected (true, true);
		ui->selectedMediaMap.insert (media->mediaId, media->mediaName);
	}
}

void MediaUi::unselectAllMedia () {
	StringList keys;
	StringList::iterator ki, kend;
	MediaWindow *media;
	StdString id;

	selectedMediaMap.getKeys (&keys);
	ki = keys.begin ();
	kend = keys.end ();
	while (ki != kend) {
		id.assign (*ki);
		media = (MediaWindow *) cardView->findItem (MediaUi::matchMediaItem, &id);
		if (media) {
			media->setSelected (false, true);
		}
		++ki;
	}
	selectedMediaMap.clear ();
	lastSelectedMediaWindow.clear ();
}

static void resetExpandToggles_countExpandedAgents (void *intPtr, Widget *widgetPtr) {
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
static void resetExpandToggles_countExpandedPlaylists (void *intPtr, Widget *widgetPtr) {
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
void MediaUi::resetExpandToggles () {
	Toggle *toggle;
	int count;

	toggle = (Toggle *) expandAgentsToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (resetExpandToggles_countExpandedAgents, &count);
		toggle->setChecked ((count <= 0), true);
	}

	toggle = (Toggle *) expandPlaylistsToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (resetExpandToggles_countExpandedPlaylists, &count);
		toggle->setChecked ((count <= 0), true);
	}
}

int64_t MediaUi::getSelectedStreamSize () {
	HashMap::Iterator i;
	MediaWindow *media;
	int64_t bytes;

	bytes = 0;
	i = selectedMediaMap.begin ();
	while (selectedMediaMap.hasNext (&i)) {
		media = MediaWindow::castWidget (cardView->getItem (selectedMediaMap.next (&i)));
		if (media && (! media->streamId.empty ())) {
			bytes += media->streamSize;
		}
	}

	return (bytes);
}

int MediaUi::getSelectedStreamCount () {
	HashMap::Iterator i;
	MediaWindow *media;
	int count;

	count = 0;
	i = selectedMediaMap.begin ();
	while (selectedMediaMap.hasNext (&i)) {
		media = MediaWindow::castWidget (cardView->getItem (selectedMediaMap.next (&i)));
		if (media && (! media->streamId.empty ())) {
			++count;
		}
	}

	return (count);
}

int MediaUi::getSelectedCreateStreamCount () {
	HashMap::Iterator i;
	MediaWindow *media;
	int count;

	count = 0;
	i = selectedMediaMap.begin ();
	while (selectedMediaMap.hasNext (&i)) {
		media = MediaWindow::castWidget (cardView->getItem (selectedMediaMap.next (&i)));
		if (media && media->isCreateStreamAvailable) {
			++count;
		}
	}

	return (count);
}

int64_t MediaUi::getSelectedCreateStreamSize (int profile) {
	HashMap::Iterator i;
	MediaWindow *media;
	int64_t bytes;

	bytes = 0;
	i = selectedMediaMap.begin ();
	while (selectedMediaMap.hasNext (&i)) {
		media = MediaWindow::castWidget (cardView->getItem (selectedMediaMap.next (&i)));
		if (media && media->isCreateStreamAvailable) {
			bytes += MediaUtil::getStreamSize (media->mediaSize, profile);
		}
	}

	return (bytes);
}
