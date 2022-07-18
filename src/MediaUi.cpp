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
#include <map>
#include "SDL2/SDL.h"
#include "App.h"
#include "StdString.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "Button.h"
#include "Toggle.h"
#include "Menu.h"
#include "Chip.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "CommandListener.h"
#include "Agent.h"
#include "CardView.h"
#include "ComboBox.h"
#include "TextField.h"
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
#include "UiConfiguration.h"
#include "UiText.h"
#include "AgentControl.h"
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
const float MediaUi::TextTruncateWidthScale = 0.25f;
const float MediaUi::SearchFieldWidthScale = 0.27f;
const float MediaUi::BottomPaddingHeightScale = 0.5f;

MediaUi::MediaUi ()
: Ui ()
, toolbarMode (-1)
, playButton (NULL)
, writePlaylistButton (NULL)
, pauseButton (NULL)
, stopButton (NULL)
, configureStreamButton (NULL)
, cacheStreamButton (NULL)
, removeStreamButton (NULL)
, selectAllButton (NULL)
, addTagButton (NULL)
, removeTagButton (NULL)
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
	return (new Chip (UiText::instance->getText (UiTextString::Media).capitalized (), sprites.getSprite (MediaUi::BreadcrumbIconSprite)));
}

void MediaUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextString::MediaUiHelpTitle), UiText::instance->getText (UiTextString::MediaUiHelpText));
	if (mediaServerCount <= 0) {
		helpWindow->addAction (UiText::instance->getText (UiTextString::MediaUiHelpAction1Text), UiText::instance->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
	}
	else if (mediaItemCount <= 0) {
		helpWindow->addAction (UiText::instance->getText (UiTextString::MediaUiHelpAction4Text), UiText::instance->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("media-streaming"));
	}
	else {
		helpWindow->addAction (UiText::instance->getText (UiTextString::MediaUiHelpAction2Text));
		helpWindow->addAction (UiText::instance->getText (UiTextString::MediaUiHelpAction3Text), UiText::instance->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("media-streaming"));
	}
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::MediaStreaming).capitalized (), App::getHelpUrl ("media-streaming"));
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

static bool findItem_matchMediaName (void *data, Widget *widget) {
	MediaWindow *media;

	media = MediaWindow::castWidget (widget);
	return (media && media->mediaName.lowercased ().equals ((char *) data));
}
static bool findItem_matchMonitorName (void *data, Widget *widget) {
	MonitorWindow *monitor;

	monitor = MonitorWindow::castWidget (widget);
	return (monitor && monitor->agentName.lowercased ().equals ((char *) data));
}
bool MediaUi::openWidget (const StdString &targetName) {
	MediaWindow *media;
	MonitorWindow *monitor;
	StdString name;

	name.assign (targetName.lowercased ());
	media = (MediaWindow *) cardView->findItem (findItem_matchMediaName, (char *) name.c_str ());
	if (media) {
		media->eventCallback (media->viewButtonClickCallback);
		return (true);
	}
	monitor = (MonitorWindow *) cardView->findItem (findItem_matchMonitorName, (char *) name.c_str ());
	if (monitor) {
		monitor->eventCallback (monitor->actionClickCallback);
		return (true);
	}
	return (false);
}

bool MediaUi::selectWidget (const StdString &targetName) {
	MediaWindow *media;
	StdString name;

	name.assign (targetName.lowercased ());
	media = (MediaWindow *) cardView->findItem (findItem_matchMediaName, (char *) name.c_str ());
	if (media) {
		media->setSelected (true);
		return (true);
	}
	return (false);
}

bool MediaUi::unselectWidget (const StdString &targetName) {
	MediaWindow *media;
	StdString name;

	name.assign (targetName.lowercased ());
	media = (MediaWindow *) cardView->findItem (findItem_matchMediaName, (char *) name.c_str ());
	if (media) {
		media->setSelected (false);
		return (true);
	}
	return (false);
}

OsUtil::Result MediaUi::doLoad () {
	HashMap *prefs;
	Panel *panel;
	Toggle *toggle;
	Button *button;

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
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (MediaUi::expandAgentsToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::MinimizeAll).capitalized (), UiText::instance->getText (UiTextString::ExpandAll).capitalized ());
	expandAgentsToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (UiText::instance->getText (UiTextString::MediaUiAgentRowHeaderText), panel), StdString (""), MediaUi::AgentToggleRow);

	cardView->setRowReverseSorted (MediaUi::ExpandedAgentRow, true);
	cardView->setRowReverseSorted (MediaUi::ExpandedPlaylistRow, true);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (MediaUi::expandPlaylistsToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::MinimizeAll).capitalized (), UiText::instance->getText (UiTextString::ExpandAll).capitalized ());
	expandPlaylistsToggle.assign (toggle);

	button = (Button *) panel->addWidget (new Button (sprites.getSprite (MediaUi::CreatePlaylistButtonSprite)));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::createPlaylistButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiCreatePlaylistTooltip));

	panel->setLayout (Panel::HorizontalLayout);
	cardView->addItem (createRowHeaderPanel (UiText::instance->getText (UiTextString::Playlists).capitalized (), panel), StdString (""), MediaUi::PlaylistToggleRow);

	cardView->setRowHeader (MediaUi::EmptyMediaRow, createRowHeaderPanel (UiText::instance->getText (UiTextString::Media).capitalized ()));
	cardView->setRowHeader (MediaUi::MediaRow, createRowHeaderPanel (UiText::instance->getText (UiTextString::Media).capitalized ()));
	cardView->setRowItemMarginSize (MediaUi::MediaRow, 0.0f);
	cardView->setRowSelectionAnimated (MediaUi::MediaRow, true);
	cardView->setRowDetail (MediaUi::MediaRow, cardDetail);
	cardView->setRowItemMarginSize (MediaUi::MediaLoadingRow, UiConfiguration::instance->marginSize);
	cardView->setBottomPadding (((float) App::instance->windowHeight) * MediaUi::BottomPaddingHeightScale);

	return (OsUtil::Success);
}

void MediaUi::doUnload () {
	searchField.clear ();
	searchStatusIcon.clear ();
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

	RecordStore::instance->removeRecords (SystemInterface::CommandId_MediaItem);
	RecordStore::instance->removeRecords (SystemInterface::CommandId_StreamItem);
}

void MediaUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::imageSizeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ThumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::reloadButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ReloadTooltip));
	toolbar->addRightItem (button);
}

void MediaUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	HashMap *prefs;
	TextFieldWindow *textfield;
	Button *button;
	IconLabelWindow *icon;

	button = new Button (sprites.getSprite (MediaUi::SortButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::sortButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiSortTooltip));
	toolbar->addLeftItem (button);

	textfield = new TextFieldWindow (App::instance->windowWidth * MediaUi::SearchFieldWidthScale, UiText::instance->getText (UiTextString::EnterSearchKeyPrompt));
	textfield->widgetName.assign ("searchtext");
	textfield->setPadding (UiConfiguration::instance->paddingSize, 0.0f);
	textfield->setButtonsEnabled (false, false, true, true);
	textfield->valueEditCallback = Widget::EventCallbackContext (MediaUi::searchFieldEdited, this);
	toolbar->addLeftItem (textfield);
	searchField.assign (textfield);

	button = new Button (sprites.getSprite (MediaUi::SearchButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::searchButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiSearchTooltip));
	toolbar->addLeftItem (button);

	icon = new IconLabelWindow (sprites.getSprite (MediaUi::SearchStatusIconSprite), StdString ("0"), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor);
	icon->setPadding (0.0f, 0.0f);
	icon->setIconImageColor (UiConfiguration::instance->inverseTextColor);
	icon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::SearchEmptyResultTooltip));
	toolbar->addLeftItem (icon);
	searchStatusIcon.assign (icon);

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

	CommandListener::instance->subscribe (SystemInterface::CommandId_FindMediaItemsResult, CommandListener::CommandCallbackContext (MediaUi::receiveFindMediaItemsResult, this));
	CommandListener::instance->subscribe (SystemInterface::CommandId_MediaItem, CommandListener::CommandCallbackContext (MediaUi::receiveMediaItem, this));
	CommandListener::instance->subscribe (SystemInterface::CommandId_FindMediaStreamsResult, CommandListener::CommandCallbackContext (MediaUi::receiveFindMediaStreamsResult, this));
	CommandListener::instance->subscribe (SystemInterface::CommandId_StreamItem, CommandListener::CommandCallbackContext (MediaUi::receiveStreamItem, this));

	if (searchField.widget) {
		((TextFieldWindow *) searchField.widget)->setValue (searchKey, true, true);
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
	prefs->insert (MediaUi::SelectedAgentsKey, ids);
	App::instance->unlockPrefs ();

	ids.clear ();
	cardView->processItems (doPause_appendExpandedAgentId, &ids);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaUi::ExpandedAgentsKey, ids);
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
	std::map<StdString, MediaUi::MediaServerInfo>::iterator i, iend;
	std::map<StdString, Json *> cmdmap;
	std::map<StdString, Json *>::iterator j, jend;
	Json *params, *cmd;
	int64_t now;
	int count;
	bool found;

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
		if ((recordReceiveCount > (MediaUi::PageSize / 2)) || ((nextRecordSyncTime > 0) && (now >= nextRecordSyncTime))) {
			App::instance->shouldSyncRecordStore = true;
			recordReceiveCount = 0;
			nextRecordSyncTime = 0;
		}
	}
	else {
		if (cardView->isScrolledToBottom (((float) App::instance->windowHeight) * MediaUi::BottomPaddingHeightScale)) {
			SDL_LockMutex (mediaServerMapMutex);
			i = mediaServerMap.begin ();
			iend = mediaServerMap.end ();
			while (i != iend) {
				if ((i->second.recordCount >= i->second.resultOffset) && (i->second.recordCount < i->second.setSize)) {
					params = new Json ();
					params->set ("searchKey", searchKey);
					params->set ("resultOffset", i->second.resultOffset);
					params->set ("maxResults", MediaUi::PageSize);
					params->set ("sortOrder", mediaSortOrder);
					cmd = App::instance->createCommand (SystemInterface::Command_FindMediaItems, params);
					if (cmd) {
						j = cmdmap.find (i->first);
						if (j != cmdmap.end ()) {
							if (j->second) {
								delete (j->second);
							}
							j->second = cmd;
						}
						else {
							cmdmap.insert (std::pair<StdString, Json *> (i->first, cmd));
						}
					}

					i->second.resultOffset += MediaUi::PageSize;
				}
				++i;
			}
			SDL_UnlockMutex (mediaServerMapMutex);

			j = cmdmap.begin ();
			jend = cmdmap.end ();
			while (j != jend) {
				cmd = j->second;
				if (cmd) {
					AgentControl::instance->writeLinkCommand (cmd, j->first);
				}
				++j;
			}
			cmdmap.clear ();
		}
	}

	found = false;
	SDL_LockMutex (mediaServerMapMutex);
	i = mediaServerMap.begin ();
	iend = mediaServerMap.end ();
	while (i != iend) {
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
		cardView->setRowHeader (MediaUi::MediaRow, createRowHeaderPanel (UiText::instance->getText (UiTextString::Media).capitalized (), createLoadingIconWindow ()));
		cardView->addItem (createLoadingIconWindow (), StdString (""), MediaUi::MediaLoadingRow);
	}
	else if (isLoadingMedia && (! found)) {
		cardView->setRowHeader (MediaUi::MediaRow, createRowHeaderPanel (UiText::instance->getText (UiTextString::Media).capitalized ()));
		cardView->removeRowItems (MediaUi::MediaLoadingRow);
		App::instance->shouldSyncRecordStore = true;
	}
	isLoadingMedia = found;
	isLinkCommandActive = isLoadingMedia;
}

void MediaUi::doResize () {
	if (searchField.widget) {
		((TextFieldWindow *) searchField.widget)->setWindowWidth (App::instance->windowWidth * MediaUi::SearchFieldWidthScale);
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
		clearPopupWidgets ();
		setToolbarMode (mode);
		return (true);
	}
	return (false);
}

void MediaUi::doSyncRecordStore () {
	IconCardWindow *window;
	int type;

	mediaServerCount = 0;
	RecordStore::instance->processAgentRecords ("mediaServerStatus", MediaUi::doSyncRecordStore_processMediaServerAgent, this);
	RecordStore::instance->processAgentRecords ("monitorServerStatus", MediaUi::doSyncRecordStore_processMonitorAgent, this);

	mediaItemCount = 0;
	mediaStreamCount = 0;
	RecordStore::instance->processCommandRecords (SystemInterface::CommandId_MediaItem, MediaUi::doSyncRecordStore_processMediaItem, this);

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
				window = new IconCardWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite));
				window->setName (UiText::instance->getText (UiTextString::MediaUiEmptyAgentStatusTitle));
				window->setDetailText (UiText::instance->getText (UiTextString::MediaUiEmptyAgentStatusText));
				window->setLink (UiText::instance->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
				window->itemId.assign (cardView->getAvailableItemId ());
				cardView->addItem (window, window->itemId, MediaUi::UnexpandedAgentRow);
				emptyStateWindow.assign (window);
				break;
			}
			case MediaUi::EmptyMediaState: {
				window = new IconCardWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite));
				window->setName (UiText::instance->getText (UiTextString::MediaUiEmptyMediaStatusTitle));
				window->setDetailText (UiText::instance->getText (UiTextString::MediaUiEmptyMediaStatusText));
				window->setLink (UiText::instance->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
				window->itemId.assign (cardView->getAvailableItemId ());
				cardView->addItem (window, window->itemId, MediaUi::EmptyMediaRow);
				emptyStateWindow.assign (window);
				break;
			}
			case MediaUi::EmptyMediaStreamState: {
				window = new IconCardWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeStreamIconSprite));
				window->setName (UiText::instance->getText (UiTextString::MediaUiEmptyMediaStreamStatusTitle));
				window->setDetailText (UiText::instance->getText (UiTextString::MediaUiEmptyMediaStreamStatusText));
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
	HashMap *prefs;
	MediaLibraryWindow *medialibrary;
	StringList items;
	int row, pos;

	ui = (MediaUi *) uiPtr;
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
			medialibrary->sortKey.sprintf ("a%s", Agent::getCommandAgentName (record).lowercased ().c_str ());
		}
		else {
			row = MediaUi::ExpandedAgentRow;
			medialibrary->setExpanded (true, true);
			medialibrary->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
		}

		ui->cardView->addItem (medialibrary, recordId, row);
		medialibrary->animateNewCard ();
		AgentControl::instance->refreshAgentStatus (recordId);
		ui->addLinkAgent (recordId);
	}
}

void MediaUi::doSyncRecordStore_processMonitorAgent (void *uiPtr, Json *record, const StdString &recordId) {
	MediaUi *ui;
	HashMap *prefs;
	MonitorWindow *monitor;
	StringList items;
	int row, pos;

	ui = (MediaUi *) uiPtr;
	if (! ui->cardView->contains (recordId)) {
		monitor = new MonitorWindow (recordId);
		monitor->setScreenshotDisplayEnabled (true);
		monitor->setStorageDisplayEnabled (true);
		monitor->setSelectEnabled (true);
		monitor->selectStateChangeCallback = Widget::EventCallbackContext (MediaUi::monitorSelectStateChanged, ui);
		monitor->expandStateChangeCallback = Widget::EventCallbackContext (MediaUi::cardExpandStateChanged, ui);
		monitor->screenshotLoadCallback = Widget::EventCallbackContext (MediaUi::monitorScreenshotLoaded, ui);
		monitor->addActionButton (ui->sprites.getSprite (MediaUi::CacheButtonSprite), UiText::instance->getText (UiTextString::ViewMonitorCacheTooltip), Widget::EventCallbackContext (MediaUi::monitorCacheButtonClicked, ui));

		prefs = App::instance->lockPrefs ();
		prefs->find (MediaUi::SelectedAgentsKey, &items);
		if (items.contains (recordId)) {
			monitor->setSelected (true, true);
			ui->selectedMonitorMap.insert (recordId, Agent::getCommandAgentName (record));
		}
		prefs->find (MediaUi::ExpandedAgentsKey, &items);
		App::instance->unlockPrefs ();

		pos = items.indexOf (recordId);
		if (pos < 0) {
			row = MediaUi::UnexpandedAgentRow;
			monitor->sortKey.sprintf ("b%s", Agent::getCommandAgentName (record).lowercased ().c_str ());
		}
		else {
			row = MediaUi::ExpandedAgentRow;
			monitor->setExpanded (true, true);
			monitor->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
		}

		ui->cardView->addItem (monitor, recordId, row);
		monitor->animateNewCard ();
		AgentControl::instance->refreshAgentStatus (recordId);
		ui->addLinkAgent (recordId);
	}
}

void MediaUi::doSyncRecordStore_processMediaItem (void *uiPtr, Json *record, const StdString &recordId) {
	MediaUi *ui;
	MediaWindow *media;
	StdString mediaid, agentid;
	Json *params, *streamrecord;
	int findstate;
	bool show;

	ui = (MediaUi *) uiPtr;
	++(ui->mediaItemCount);

	agentid = SystemInterface::instance->getCommandAgentId (record);
	mediaid.assign (recordId);
	streamrecord = RecordStore::instance->findRecord (MediaWindow::matchStreamSourceId, &mediaid);
	if (streamrecord) {
		++(ui->mediaStreamCount);
	}

	SDL_LockMutex (ui->findMediaStreamsMapMutex);
	findstate = ui->findMediaStreamsMap.find (recordId, (int) -1);
	SDL_UnlockMutex (ui->findMediaStreamsMapMutex);

	if (findstate < 0) {
		if (AgentControl::instance->isLinkClientConnected (agentid)) {
			params = new Json ();
			params->set ("sourceIds", StringList (recordId));
			AgentControl::instance->writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindMediaStreams, params), agentid);

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
					media->sortKey.sprintf ("%016llx", (long long int) (0x7FFFFFFFFFFFFFFFLL - SystemInterface::instance->getCommandNumberParam (record, "mtime", (int64_t) 0)));
				}
				else {
					if (! media->mediaSortKey.empty ()) {
						media->sortKey.assign (media->mediaSortKey);
					}
					else {
						media->sortKey.assign (media->mediaName);
					}
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
	Json *record, *params;
	bool ismedia, ismonitor;
	std::map<StdString, MediaUi::MediaServerInfo>::iterator info;

	ismedia = false;
	ismonitor = false;
	RecordStore::instance->lock ();
	record = RecordStore::instance->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (record) {
		if (SystemInterface::instance->getCommandObjectParam (record, "mediaServerStatus", NULL)) {
			ismedia = true;
		}
		if (SystemInterface::instance->getCommandObjectParam (record, "monitorServerStatus", NULL)) {
			ismonitor = true;
		}
	}
	RecordStore::instance->unlock ();

	if (ismedia) {
		SDL_LockMutex (mediaServerMapMutex);
		info = getMediaServerInfo (agentId);
		info->second.resultOffset = MediaUi::PageSize;
		SDL_UnlockMutex (mediaServerMapMutex);
		params = new Json ();
		params->set ("searchKey", searchKey);
		params->set ("maxResults", MediaUi::PageSize);
		params->set ("sortOrder", mediaSortOrder);
		AgentControl::instance->writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindMediaItems, params), agentId);
	}

	if (ismedia || ismonitor) {
		AgentControl::instance->writeLinkCommand (App::instance->createCommand (SystemInterface::Command_WatchStatus), agentId);
	}
}

void MediaUi::receiveFindMediaItemsResult (void *uiPtr, const StdString &agentId, Json *command) {
	MediaUi *ui;
	std::map<StdString, MediaUi::MediaServerInfo>::iterator info;

	ui = (MediaUi *) uiPtr;
	SDL_LockMutex (ui->mediaServerMapMutex);
	info = ui->getMediaServerInfo (agentId);
	info->second.setSize = SystemInterface::instance->getCommandNumberParam (command, "setSize", (int) 0);
	if (info->second.setSize <= 0) {
		ui->findMediaComplete = true;
	}
	SDL_UnlockMutex (ui->mediaServerMapMutex);

	ui->resetSearchStatus ();
	App::instance->shouldSyncRecordStore = true;
}

void MediaUi::receiveMediaItem (void *uiPtr, const StdString &agentId, Json *command) {
	MediaUi *ui;
	std::map<StdString, MediaUi::MediaServerInfo>::iterator info;

	ui = (MediaUi *) uiPtr;
	RecordStore::instance->addRecord (command);
	SDL_LockMutex (ui->mediaServerMapMutex);
	info = ui->getMediaServerInfo (agentId);
	++(info->second.recordCount);
	if ((info->second.recordCount >= info->second.setSize) || (info->second.recordCount >= info->second.resultOffset)) {
		App::instance->shouldSyncRecordStore = true;
	}
	SDL_UnlockMutex (ui->mediaServerMapMutex);
	ui->findMediaComplete = true;
	++(ui->recordReceiveCount);
	ui->resetNextRecordSyncTime ();
	ui->resetSearchStatus ();
}

void MediaUi::receiveFindMediaStreamsResult (void *uiPtr, const StdString &agentId, Json *command) {
	MediaUi *ui;
	StdString id;
	Json item;
	int i, len;

	ui = (MediaUi *) uiPtr;
	ui->resetNextRecordSyncTime ();
	id = SystemInterface::instance->getCommandStringParam (command, "mediaId", "");
	if (! id.empty ()) {
		SDL_LockMutex (ui->findMediaStreamsMapMutex);
		if (ui->findMediaStreamsMap.exists (id)) {
			ui->findMediaStreamsMap.insert (id, MediaUi::StreamsReceivedState);
		}
		SDL_UnlockMutex (ui->findMediaStreamsMapMutex);
	}

	len = SystemInterface::instance->getCommandArrayLength (command, "streams");
	for (i = 0; i < len; ++i) {
		if (SystemInterface::instance->getCommandObjectArrayItem (command, "streams", i, &item)) {
			if (SystemInterface::instance->getCommandId (&item) == SystemInterface::CommandId_StreamItem) {
				RecordStore::instance->addRecord (&item);
				App::instance->shouldSyncRecordStore = true;
			}
		}
	}
}

void MediaUi::receiveStreamItem (void *uiPtr, const StdString &agentId, Json *command) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	RecordStore::instance->addRecord (command);
	++(ui->recordReceiveCount);
	ui->resetNextRecordSyncTime ();
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
	ui->searchKey.assign (textfield->getValue ());
	ui->loadSearchResults ();
}

void MediaUi::sortButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	Menu *menu;

	ui = (MediaUi *) uiPtr;
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::sortButtonClicked)) {
		return;
	}
	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextString::MediaUiShowMediaWithoutStreamsAction), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::VisibilityOnButtonSprite), Widget::EventCallbackContext (MediaUi::showMediaWithoutStreamsActionClicked, ui), 0, (! ui->isShowingMediaWithoutStreams));
	menu->addItem (UiText::instance->getText (UiTextString::SortByName).capitalized (), ui->sprites.getSprite (MediaUi::SortButtonSprite), Widget::EventCallbackContext (MediaUi::sortByNameActionClicked, ui), 2, ui->mediaSortOrder == SystemInterface::Constant_NameSort);
	menu->addItem (UiText::instance->getText (UiTextString::SortByNewest).capitalized (), ui->sprites.getSprite (MediaUi::SortButtonSprite), Widget::EventCallbackContext (MediaUi::sortByNewestActionClicked, ui), 2, ui->mediaSortOrder == SystemInterface::Constant_NewestSort);

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
	StringList agentids;
	Json *params;

	RecordStore::instance->removeRecords (SystemInterface::CommandId_MediaItem);

	SDL_LockMutex (mediaServerMapMutex);
	i = mediaServerMap.begin ();
	end = mediaServerMap.end ();
	while (i != end) {
		agentids.push_back (i->first);
		i->second.resultOffset = MediaUi::PageSize;
		i->second.setSize = 0;
		i->second.recordCount = 0;
		++i;
	}
	SDL_UnlockMutex (mediaServerMapMutex);

	SDL_LockMutex (findMediaStreamsMapMutex);
	findMediaStreamsMap.clear ();
	SDL_UnlockMutex (findMediaStreamsMapMutex);

	cardView->removeRowItems (MediaUi::MediaRow);
	cardView->scrollToRow (MediaUi::MediaRow, ((float) App::instance->windowHeight) * MediaUi::BottomPaddingHeightScale);

	findMediaComplete = false;
	params = new Json ();
	params->set ("searchKey", searchKey);
	params->set ("maxResults", MediaUi::PageSize);
	params->set ("sortOrder", mediaSortOrder);
	AgentControl::instance->writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindMediaItems, params), agentids);
}

void MediaUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	Menu *menu;

	ui = (MediaUi *) uiPtr;
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::imageSizeButtonClicked)) {
		return;
	}
	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextString::Small).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), Widget::EventCallbackContext (MediaUi::smallImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Medium).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), Widget::EventCallbackContext (MediaUi::mediumImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Large).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), Widget::EventCallbackContext (MediaUi::largeImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::HighDetail);

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
		AgentControl::instance->refreshAgentStatus (medialibrary->agentId);
		return;
	}
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		AgentControl::instance->refreshAgentStatus (monitor->agentId);
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
		base.assign (UiText::instance->getText (UiTextString::Playlist).capitalized ());
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
	result = OsUtil::openUrl (StdString::createSprintf ("%s?%s", AgentControl::instance->getAgentSecondaryUrl (media->streamAgentId, media->htmlPlayerPath).c_str (), media->streamId.urlEncoded ().c_str ()));
	if (result != OsUtil::Success) {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::LaunchWebBrowserError));
	}
	else {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::LaunchedWebBrowser).capitalized ());
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
	MediaLibraryWindow *medialibrary;
	Menu *menu;

	ui = (MediaUi *) uiPtr;
	medialibrary = (MediaLibraryWindow *) widgetPtr;
	if (ui->clearActionPopup (widgetPtr, MediaUi::mediaLibraryMenuClicked)) {
		return;
	}
	menu = new Menu ();
	menu->addItem (UiText::instance->getText (UiTextString::ScanForMedia).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::UpdateButtonSprite), Widget::EventCallbackContext (MediaUi::mediaLibraryScanActionClicked, ui));
	menu->isClickDestroyEnabled = true;

	ui->showActionPopup (menu, widgetPtr, MediaUi::mediaLibraryMenuClicked, medialibrary->getMenuButtonScreenRect (), Ui::LeftEdgeAlignment, Ui::BottomOfAlignment);
}

void MediaUi::mediaLibraryScanActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaLibraryWindow *target;

	ui = (MediaUi *) uiPtr;
	target = MediaLibraryWindow::castWidget (ui->actionTarget.widget);
	if (! target) {
		return;
	}
	ui->invokeCommand (CommandHistory::instance->scanMediaItems (StringList (target->agentId)), target->agentId, App::instance->createCommand (SystemInterface::Command_ScanMediaItems));
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
	StreamPlaylistWindow *target;
	TextFieldWindow *textfield;

	ui = (MediaUi *) uiPtr;
	target = StreamPlaylistWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}
	ui->clearPopupWidgets ();
	textfield = new TextFieldWindow (target->windowWidth, UiText::instance->getText (UiTextString::EnterPlaylistNamePrompt));
	textfield->setValue (target->playlistName);
	textfield->valueEditCallback = Widget::EventCallbackContext (MediaUi::playlistNameEdited, ui);
	textfield->enterButtonClickCallback = Widget::EventCallbackContext (MediaUi::playlistNameEditEnterButtonClicked, ui);
	textfield->setFillBg (true, UiConfiguration::instance->lightPrimaryColor);
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
	ActionWindow *action;

	ui = (MediaUi *) uiPtr;
	playlist = (StreamPlaylistWindow *) widgetPtr;
	if (ui->clearActionPopup (playlist, MediaUi::playlistRemoveActionClicked)) {
		return;
	}
	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiConfiguration::instance->fonts[UiConfiguration::TitleFont]->truncatedText (playlist->playlistName, playlist->width * 0.34f, Font::DotTruncateSuffix));
	action->setDescriptionText (UiText::instance->getText (UiTextString::RemovePlaylistDescription));
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
			streamurl = AgentControl::instance->getAgentSecondaryUrl (media->streamAgentId, media->hlsStreamPath);
			startpos = (float) media->displayTimestamp;
			if (startpos < 0.0f) {
				startpos = 0.0f;
			}
			startpos /= 1000.0f;

			thumbnailurl.assign ("");
			if (! media->streamThumbnailPath.empty ()) {
				thumbnailurl = AgentControl::instance->getAgentSecondaryUrl (media->streamAgentId, media->streamThumbnailPath);
			}
			playlist->addItem (streamurl, media->streamId, media->mediaName, startpos, thumbnailurl, media->playThumbnailIndex);
		}
	}

	ui->clearPopupWidgets ();
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::InternalError));
	}
	else if (count == 1) {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", playlist->playlistName.c_str (), UiText::instance->getText (UiTextString::AddedPlaylistItem).c_str ()));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s (%i)", playlist->playlistName.c_str (), UiText::instance->getText (UiTextString::AddedPlaylistItems).c_str (), count));
	}
}

void MediaUi::playlistAddItemMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	StdString text;
	Color color;

	ui = (MediaUi *) uiPtr;
	playlist = (StreamPlaylistWindow *) widgetPtr;
	ui->commandPopup.destroyAndClear ();
	ui->commandPopupSource.assign (widgetPtr);
	panel = (Panel *) App::instance->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	panel->setBorder (true, Color (UiConfiguration::instance->darkBackgroundColor.r, UiConfiguration::instance->darkBackgroundColor.g, UiConfiguration::instance->darkBackgroundColor.b, UiConfiguration::instance->overlayWindowAlpha));

	label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::AddPlaylistItems).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));
	label->setPadding (0.0f, 0.0f);

	text.assign (playlist->playlistName);
	UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->rootPanel->width * 0.20f, Font::DotTruncateSuffix);

	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setRightAligned (true);

	text.assign (ui->getSelectedMediaNames (true));
	color.assign (UiConfiguration::instance->primaryTextColor);
	if (text.empty ()) {
		text.assign (UiText::instance->getText (UiTextString::MediaUiNoStreamableMediaSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
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
	Menu *menu;

	ui = (MediaUi *) uiPtr;
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::modeButtonClicked)) {
		return;
	}
	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextString::PlayStreams).capitalized (), ui->sprites.getSprite (MediaUi::PlayButtonSprite), Widget::EventCallbackContext (MediaUi::monitorModeActionClicked, ui), 0, ui->toolbarMode == MediaUi::MonitorMode);
	menu->addItem (UiText::instance->getText (UiTextString::ManageStreams).capitalized (), ui->sprites.getSprite (MediaUi::ConfigureStreamButtonSprite), Widget::EventCallbackContext (MediaUi::streamModeActionClicked, ui), 0, ui->toolbarMode == MediaUi::StreamMode);
	menu->addItem (UiText::instance->getText (UiTextString::ManageSearchKeys).capitalized (), ui->sprites.getSprite (MediaUi::AddTagButtonSprite), Widget::EventCallbackContext (MediaUi::tagModeActionClicked, ui), 0, ui->toolbarMode == MediaUi::TagMode);

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

void MediaUi::tagModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	if (ui->toolbarMode != MediaUi::TagMode) {
		ui->unselectAllMedia ();
		ui->setToolbarMode (MediaUi::TagMode);
	}
}

void MediaUi::setToolbarMode (int mode, bool forceReset) {
	Toolbar *toolbar;
	Button *button;

	if ((toolbarMode == mode) && (! forceReset)) {
		return;
	}
	toolbar = App::instance->uiStack.secondaryToolbar;
	toolbar->clearRightItems ();
	playButton = NULL;
	writePlaylistButton = NULL;
	stopButton = NULL;
	configureStreamButton = NULL;
	cacheStreamButton = NULL;
	removeStreamButton = NULL;
	selectAllButton = NULL;
	addTagButton = NULL;
	removeTagButton = NULL;

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ToolsButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaUi::modeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ToolbarModeButtonTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (button);

	toolbarMode = mode;
	switch (toolbarMode) {
		case MediaUi::MonitorMode: {
			playButton = new Button (sprites.getSprite (MediaUi::PlayButtonSprite));
			playButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::playButtonClicked, this);
			playButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			playButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			playButton->setInverseColor (true);
			playButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiPlayTooltip).capitalized (), Widget::LeftAlignment);
			playButton->shortcutKey = SDLK_F4;
			toolbar->addRightItem (playButton);

			writePlaylistButton = new Button (sprites.getSprite (MediaUi::WritePlaylistButtonSprite));
			writePlaylistButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::writePlaylistButtonClicked, this);
			writePlaylistButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			writePlaylistButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			writePlaylistButton->setInverseColor (true);
			writePlaylistButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiWritePlaylistTooltip), Widget::LeftAlignment);
			writePlaylistButton->shortcutKey = SDLK_F3;
			toolbar->addRightItem (writePlaylistButton);

			pauseButton = new Button (sprites.getSprite (MediaUi::PauseButtonSprite));
			pauseButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::pauseButtonClicked, this);
			pauseButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			pauseButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			pauseButton->setInverseColor (true);
			pauseButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiPauseTooltip), Widget::LeftAlignment);
			pauseButton->shortcutKey = SDLK_F2;
			toolbar->addRightItem (pauseButton);

			stopButton = new Button (sprites.getSprite (MediaUi::StopButtonSprite));
			stopButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::stopButtonClicked, this);
			stopButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			stopButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			stopButton->setInverseColor (true);
			stopButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiStopTooltip), Widget::LeftAlignment);
			stopButton->shortcutKey = SDLK_F1;
			toolbar->addRightItem (stopButton);
			break;
		}
		case MediaUi::StreamMode: {
			selectAllButton = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarHalfButtonSprite));
			selectAllButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::selectAllButtonClicked, this);
			selectAllButton->setInverseColor (true);
			selectAllButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiSelectAllTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (selectAllButton);

			configureStreamButton = new Button (sprites.getSprite (MediaUi::ConfigureStreamButtonSprite));
			configureStreamButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::configureStreamButtonClicked, this);
			configureStreamButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			configureStreamButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			configureStreamButton->setInverseColor (true);
			configureStreamButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiConfigureStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (configureStreamButton);

			cacheStreamButton = new Button (sprites.getSprite (MediaUi::CacheButtonSprite));
			cacheStreamButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::cacheStreamButtonClicked, this);
			cacheStreamButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			cacheStreamButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			cacheStreamButton->setInverseColor (true);
			cacheStreamButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiAddCacheStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (cacheStreamButton);

			removeStreamButton = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite));
			removeStreamButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::removeStreamButtonClicked, this);
			removeStreamButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			removeStreamButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			removeStreamButton->setInverseColor (true);
			removeStreamButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiRemoveStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (removeStreamButton);
			break;
		}
		case MediaUi::TagMode: {
			selectAllButton = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarHalfButtonSprite));
			selectAllButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::selectAllButtonClicked, this);
			selectAllButton->setInverseColor (true);
			selectAllButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiSelectAllTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (selectAllButton);

			addTagButton = new Button (sprites.getSprite (MediaUi::AddTagButtonSprite));
			addTagButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::addTagButtonClicked, this);
			addTagButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			addTagButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			addTagButton->setInverseColor (true);
			addTagButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiAddTagTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (addTagButton);

			removeTagButton = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite));
			removeTagButton->mouseClickCallback = Widget::EventCallbackContext (MediaUi::removeTagButtonClicked, this);
			removeTagButton->mouseEnterCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseEntered, this);
			removeTagButton->mouseExitCallback = Widget::EventCallbackContext (MediaUi::commandButtonMouseExited, this);
			removeTagButton->setInverseColor (true);
			removeTagButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiRemoveTagTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (removeTagButton);
			break;
		}
	}
}

void MediaUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	Button *button;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	MediaWindow *media;
	StreamPlaylistWindow *playlist;
	StdString text;
	Color color;

	ui = (MediaUi *) uiPtr;
	button = (Button *) widgetPtr;
	Button::mouseEntered (button, button);

	ui->commandPopup.destroyAndClear ();
	ui->commandPopupSource.assign (button);
	panel = (Panel *) App::instance->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	panel->setBorder (true, Color (UiConfiguration::instance->darkBackgroundColor.r, UiConfiguration::instance->darkBackgroundColor.g, UiConfiguration::instance->darkBackgroundColor.b, UiConfiguration::instance->overlayWindowAlpha));

	label = NULL;
	if (button == ui->configureStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::ConfigureStream).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMediaNames (false, true));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::MediaUiNoMediaSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallMediaIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->removeStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::DeleteStream).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMediaNames (true));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::MediaUiNoStreamableMediaSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->cacheStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::CacheStream).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->getSelectedMediaNames (true));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::MediaUiNoStreamableMediaSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->playButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::Play).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);

		media = MediaWindow::castWidget (ui->lastSelectedMediaWindow.widget);
		if ((! media) || media->hlsStreamPath.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::MediaUiNoStreamableMediaSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		else {
			text.assign (media->mediaName);
			UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->rootPanel->width * 0.20f, Font::DotTruncateSuffix);
			text.appendSprintf (" %s", OsUtil::getDurationString ((media->displayTimestamp > 0.0f) ? (int64_t) media->displayTimestamp : 0, OsUtil::HoursUnit).c_str ());
			color.assign (UiConfiguration::instance->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->writePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::RunPlaylist).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);

		playlist = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
		if (! playlist) {
			text.assign (UiText::instance->getText (UiTextString::NoPlaylistSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		else if (playlist->getItemCount () <= 0) {
			text.assign (UiText::instance->getText (UiTextString::EmptyPlaylistSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		else {
			text.assign (playlist->playlistName);
			UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->rootPanel->width * 0.20f, Font::DotTruncateSuffix);
			color.assign (UiConfiguration::instance->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->stopButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::Stop).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->pauseButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (StdString::createSprintf ("%s / %s", UiText::instance->getText (UiTextString::Pause).capitalized ().c_str (), UiText::instance->getText (UiTextString::Resume).c_str ()), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames ());
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->addTagButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::AddMediaTagCommandName), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMediaNames ());
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::MediaUiNoMediaSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallMediaIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->removeTagButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::RemoveMediaTagCommandName), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMediaNames ());
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::MediaUiNoMediaSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallMediaIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}

	if (label) {
		label->setPadding (0.0f, 0.0f);
	}
	panel->setLayout (Panel::VerticalRightJustifiedLayout);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	panel->position.assign (App::instance->windowWidth - panel->width - UiConfiguration::instance->paddingSize, App::instance->windowHeight - App::instance->uiStack.bottomBarHeight - panel->height - UiConfiguration::instance->marginSize);
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
	streamurl = AgentControl::instance->getAgentSecondaryUrl (media->streamAgentId, media->hlsStreamPath, App::instance->createCommand (SystemInterface::Command_GetHlsManifest, params));

	params = new Json ();
	params->set ("mediaName", media->mediaName);
	params->set ("streamUrl", streamurl);
	params->set ("streamId", media->streamId);
	params->set ("startPosition", startpos);
	if (! media->playThumbnailUrl.empty ()) {
		params->set ("thumbnailUrl", media->playThumbnailUrl);
	}

	ui->invokeCommand (CommandHistory::instance->playMedia (idlist, media->mediaName, streamurl, media->streamId, startpos, media->playThumbnailUrl), idlist, App::instance->createCommand (SystemInterface::Command_PlayMedia, params), MediaUi::invokeMonitorCommandComplete);
}

void MediaUi::invokeMonitorCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess) {
	if (isResponseCommandSuccess) {
		AgentControl::instance->refreshAgentStatus (agentId);
	}
}

void MediaUi::writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StreamPlaylistWindow *playlist;
	StringList idlist;

	ui = (MediaUi *) uiPtr;
	playlist = StreamPlaylistWindow::castWidget (ui->selectedPlaylistWindow.widget);
	ui->selectedMonitorMap.getKeys (&idlist, true);
	if ((! playlist) || (playlist->getItemCount () <= 0) || idlist.empty ()) {
		return;
	}
	ui->invokeCommand (CommandHistory::instance->createMediaDisplayIntent (idlist, playlist->playlistName), idlist, playlist->createCommand (), MediaUi::invokeMonitorCommandComplete);
}

void MediaUi::pauseButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StringList idlist;

	ui = (MediaUi *) uiPtr;
	ui->selectedMonitorMap.getKeys (&idlist, true);
	if (idlist.empty ()) {
		return;
	}
	ui->invokeCommand (CommandHistory::instance->pauseMedia (idlist), idlist, App::instance->createCommand (SystemInterface::Command_PauseMedia), MediaUi::invokeMonitorCommandComplete);
}

void MediaUi::stopButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	StringList idlist;

	ui = (MediaUi *) uiPtr;
	ui->selectedMonitorMap.getKeys (&idlist, true);
	if (idlist.empty ()) {
		return;
	}
	ui->invokeCommand (CommandHistory::instance->clearDisplay (idlist), idlist, App::instance->createCommand (SystemInterface::Command_ClearDisplay), MediaUi::invokeMonitorCommandComplete);
}

void MediaUi::configureStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	HashMap *prefs;
	ActionWindow *action;
	ComboBox *combobox;
	Panel *panel;
	IconLabelWindow *icon;
	int count, profile;

	ui = (MediaUi *) uiPtr;
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
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextString::ConfigureStream).capitalized ());
	action->setConfirmTooltipText (UiText::instance->getText (UiTextString::Apply).capitalized ());
	action->optionChangeCallback = Widget::EventCallbackContext (MediaUi::configureStreamOptionChanged, ui);
	action->closeCallback = Widget::EventCallbackContext (MediaUi::configureStreamActionClosed, ui);

	combobox = new ComboBox ();
	combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_SourceMatchStreamProfile));
	combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_HighBitrateStreamProfile));
	combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_MediumBitrateStreamProfile));
	combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_LowBitrateStreamProfile));
	combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_LowestBitrateStreamProfile));
	combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_PreviewStreamProfile));
	combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_FastPreviewStreamProfile));
	combobox->setValue (MediaUtil::getStreamProfileName (profile));
	action->addOption (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), combobox, MediaUtil::getStreamProfileDescription (profile));

	panel = new Panel ();
	count = ui->getSelectedCreateStreamCount ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::ConfigureStreamItemCountTooltip, UiTextString::ConfigureStreamItemsCountTooltip));

	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), OsUtil::getByteCountDisplayString (ui->getSelectedCreateStreamSize (profile)), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ConfigureStreamByteCountTooltip));
	ui->configureStreamSizeIcon.assign (icon);
	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, MediaUi::configureStreamButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void MediaUi::configureStreamOptionChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	IconLabelWindow *icon;
	StdString text;
	int profile;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	icon = (IconLabelWindow *) ui->configureStreamSizeIcon.widget;
	if (icon) {
		icon->setText (OsUtil::getByteCountDisplayString (ui->getSelectedCreateStreamSize (MediaUtil::getStreamProfile (action->getStringValue (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), "")))));
	}

	profile = MediaUtil::getStreamProfile (action->getStringValue (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), ""));
	text = MediaUtil::getStreamProfileDescription (profile);
	if (! text.empty ()) {
		action->setOptionDescriptionText (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), text);
	}
	action->refresh ();
}

void MediaUi::configureStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	HashMap *prefs;
	ActionWindow *action;
	MediaWindow *media;
	HashMap::Iterator i;
	StdString mediaid, cmdtext;
	Json *params, *cmd;
	JsonList commands;
	StringList agentids;
	int profile;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}
	profile = MediaUtil::getStreamProfile (action->getStringValue (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), ""));
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

			cmd = App::instance->createCommand (SystemInterface::Command_ConfigureMediaStream, params);
			if (cmd) {
				agentids.push_back (media->agentId);
				commands.push_back (cmd);
				if (cmdtext.empty ()) {
					cmdtext.assign (media->mediaName);
				}
			}
		}
	}
	if (commands.size () > 1) {
		cmdtext.sprintf ("%i %s", (int) commands.size (), UiText::instance->getText (UiTextString::MediaFiles).c_str ());
	}
	ui->invokeCommand (CommandHistory::instance->configureMediaStream (agentids, (int) commands.size (), cmdtext), agentids, &commands, MediaUi::configureMediaStreamComplete);
}

void MediaUi::configureMediaStreamComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess) {
	StdString mediaid, streamid;
	Json *streamitem;

	if (isResponseCommandSuccess) {
		mediaid = SystemInterface::instance->getCommandStringParam (invokeCommand, "mediaId", "");
		if (! mediaid.empty ()) {
			RecordStore::instance->lock ();
			streamitem = RecordStore::instance->findRecord (MediaWindow::matchStreamSourceId, &mediaid);
			if (streamitem) {
				streamid = SystemInterface::instance->getCommandRecordId (streamitem);
				if (! streamid.empty ()) {
					RecordStore::instance->removeRecord (streamid);
				}
			}
			RecordStore::instance->unlock ();
		}
		if (! streamid.empty ()) {
			AgentControl::instance->refreshAgentStatus (agentId);
			App::instance->shouldSyncRecordStore = true;
		}
	}
}

void MediaUi::cacheStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	IconLabelWindow *icon;
	Panel *panel;
	int count;

	ui = (MediaUi *) uiPtr;
	if (ui->selectedMonitorMap.empty () || ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (true).empty ()) {
		return;
	}
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::cacheStreamButtonClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextString::CacheStreams).capitalized ());
	action->setDescriptionText (UiText::instance->getText (UiTextString::CacheStreamsDescription));
	action->closeCallback = Widget::EventCallbackContext (MediaUi::cacheStreamActionClosed, ui);

	panel = new Panel ();
	count = (int) ui->selectedMonitorMap.size ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::MonitorSelected, UiTextString::MonitorsSelected));

	count = ui->getSelectedStreamCount ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::CacheStreamItemCountTooltip, UiTextString::CacheStreamItemsCountTooltip));

	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), OsUtil::getByteCountDisplayString (ui->getSelectedStreamSize ()), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CacheStreamByteCountTooltip));
	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, MediaUi::cacheStreamButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void MediaUi::cacheStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	MediaWindow *media;
	HashMap::Iterator i, j;
	StdString mediaid, agentid, cmdtext;
	Json *params, *cmd, *streamitem, *agentstatus, serverstatus;
	StringList agentids;
	JsonList commands;
	int mediacount;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if ((! action->isConfirmed) || ui->selectedMonitorMap.empty () || ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (true).empty ()) {
		return;
	}
	mediacount = 0;
	i = ui->selectedMediaMap.begin ();
	while (ui->selectedMediaMap.hasNext (&i)) {
		mediaid = ui->selectedMediaMap.next (&i);
		media = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
		if (media && (! media->streamId.empty ())) {
			params = NULL;
			RecordStore::instance->lock ();
			streamitem = RecordStore::instance->findRecord (media->streamId, SystemInterface::CommandId_StreamItem);
			agentstatus = RecordStore::instance->findRecord (RecordStore::matchAgentStatusSource, &(media->streamAgentId));
			if (streamitem && agentstatus && SystemInterface::instance->getCommandObjectParam (agentstatus, "streamServerStatus", &serverstatus)) {
				params = new Json ();
				params->set ("streamUrl", AgentControl::instance->getAgentSecondaryUrl (media->streamAgentId, media->hlsStreamPath));
				params->set ("thumbnailUrl", AgentControl::instance->getAgentSecondaryUrl (media->streamAgentId, serverstatus.getString ("thumbnailPath", "")));
				params->set ("streamId", media->streamId);
				params->set ("streamName", SystemInterface::instance->getCommandStringParam (streamitem, "name", ""));
				params->set ("duration", SystemInterface::instance->getCommandNumberParam (streamitem, "duration", (float) 0.0f));
				params->set ("width", SystemInterface::instance->getCommandNumberParam (streamitem, "width", (int) 0));
				params->set ("height", SystemInterface::instance->getCommandNumberParam (streamitem, "height", (int) 0));
				params->set ("bitrate", SystemInterface::instance->getCommandNumberParam (streamitem, "bitrate", (int64_t) 0));
				params->set ("frameRate", SystemInterface::instance->getCommandNumberParam (streamitem, "frameRate", (float) 0.0f));
			}
			RecordStore::instance->unlock ();

			if (params) {
				cmd = App::instance->createCommand (SystemInterface::Command_CreateCacheStream, params);
				if (cmd) {
					++mediacount;
					if (cmdtext.empty ()) {
						cmdtext.assign (media->mediaName);
					}
					j = ui->selectedMonitorMap.begin ();
					while (ui->selectedMonitorMap.hasNext (&j)) {
						agentid = ui->selectedMonitorMap.next (&j);
						agentids.push_back (agentid);
						commands.push_back (cmd->copy ());
					}
					delete (cmd);
				}
			}
		}
	}
	if (mediacount > 1) {
		cmdtext.sprintf ("%i %s", mediacount, UiText::instance->getText (UiTextString::VideoStreams).c_str ());
	}
	ui->invokeCommand (CommandHistory::instance->createCacheStream (agentids, (int) commands.size (), cmdtext), agentids, &commands);
}

void MediaUi::removeStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	IconLabelWindow *icon;
	Panel *panel;
	int count;

	ui = (MediaUi *) uiPtr;
	if (ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (true).empty ()) {
		return;
	}
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::removeStreamButtonClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextString::RemoveStreamTitle));
	action->setDescriptionText (UiText::instance->getText (UiTextString::RemoveStreamDescription));
	action->closeCallback = Widget::EventCallbackContext (MediaUi::removeStreamActionClosed, ui);

	panel = new Panel ();
	count = ui->getSelectedStreamCount ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::RemoveStreamItemCountTooltip, UiTextString::RemoveStreamItemsCountTooltip));

	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), OsUtil::getByteCountDisplayString (ui->getSelectedStreamSize ()), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::RemoveStreamByteCountTooltip));

	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, MediaUi::removeStreamButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void MediaUi::removeStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	MediaWindow *media;
	HashMap::Iterator i;
	StdString mediaid, cmdtext;
	Json *params, *cmd;
	StringList agentids;
	StringList::iterator ri, rend;
	JsonList commands;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if ((! action->isConfirmed) || ui->selectedMediaMap.empty () || ui->getSelectedMediaNames (true).empty ()) {
		return;
	}
	i = ui->selectedMediaMap.begin ();
	while (ui->selectedMediaMap.hasNext (&i)) {
		mediaid = ui->selectedMediaMap.next (&i);
		media = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
		if (media && (! media->streamId.empty ())) {
			params = new Json ();
			params->set ("id", media->streamId);

			cmd = App::instance->createCommand (SystemInterface::Command_RemoveStream, params);
			if (cmd) {
				agentids.push_back (media->streamAgentId);
				commands.push_back (cmd);
				if (cmdtext.empty ()) {
					cmdtext.assign (media->mediaName);
				}
			}
		}
	}
	if (commands.size () > 1) {
		cmdtext.sprintf ("%i %s", (int) commands.size (), UiText::instance->getText (UiTextString::VideoStreams).c_str ());
	}
	ui->invokeCommand (CommandHistory::instance->removeStream (agentids, (int) commands.size (), cmdtext), agentids, &commands, MediaUi::removeStreamComplete);
}

void MediaUi::removeStreamComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess) {
	StdString streamid;

	if (isResponseCommandSuccess) {
		streamid = SystemInterface::instance->getCommandStringParam (invokeCommand, "id", "");
		if (! streamid.empty ()) {
			RecordStore::instance->removeRecord (streamid);
			AgentControl::instance->refreshAgentStatus (agentId);
			App::instance->shouldSyncRecordStore = true;
		}
	}
}

void MediaUi::addTagButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	IconLabelWindow *icon;
	Panel *panel;
	TextField *textfield;
	int count;

	ui = (MediaUi *) uiPtr;
	if (ui->selectedMediaMap.empty ()) {
		return;
	}
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::addTagButtonClicked)) {
		return;
	}
	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextString::AddMediaTagCommandName));
	action->setDescriptionText (UiText::instance->getText (UiTextString::MediaUiAddTagPrompt));
	textfield = new TextField (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth);
	action->addOption (UiText::instance->getText (UiTextString::SearchKey), textfield);
	action->setOptionNameText (UiText::instance->getText (UiTextString::SearchKey), StdString (""));
	action->setOptionNotEmptyString (UiText::instance->getText (UiTextString::SearchKey));
	action->closeCallback = Widget::EventCallbackContext (MediaUi::addTagActionClosed, ui);

	panel = new Panel ();
	count = ui->getSelectedMediaCount ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::MediaTagItemCountTooltip, UiTextString::MediaTagItemsCountTooltip));

	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, MediaUi::addTagButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
	App::instance->uiStack.setKeyFocusTarget (textfield);
}

void MediaUi::addTagActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	JsonList commands;
	StringList agentids;
	HashMap::Iterator i;
	MediaWindow *media;
	Json *params, *cmd;
	StdString mediaid, tag, cmdtext;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if ((! action->isConfirmed) || ui->selectedMediaMap.empty ()) {
		return;
	}
	tag = action->getStringValue (UiText::instance->getText (UiTextString::SearchKey), StdString (""));
	i = ui->selectedMediaMap.begin ();
	while (ui->selectedMediaMap.hasNext (&i)) {
		mediaid = ui->selectedMediaMap.next (&i);
		media = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
		if (media) {
			params = new Json ();
			params->set ("mediaId", media->mediaId);
			params->set ("tag", tag);
			cmd = App::instance->createCommand (SystemInterface::Command_AddMediaTag, params);
			if (cmd) {
				agentids.push_back (media->agentId);
				commands.push_back (cmd);
				if (cmdtext.empty ()) {
					cmdtext.assign (media->mediaName);
				}
			}
		}
	}
	if (commands.size () > 1) {
		cmdtext.sprintf ("%i %s", (int) commands.size (), UiText::instance->getText (UiTextString::MediaFiles).c_str ());
	}
	ui->invokeCommand (CommandHistory::instance->addMediaTag (agentids, (int) commands.size (), cmdtext), agentids, &commands, MediaUi::invokeMediaCommandComplete);
}

void MediaUi::invokeMediaCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess) {
	Json item;

	if (isResponseCommandSuccess) {
		if (SystemInterface::instance->getCommandObjectParam (responseCommand, "item", &item)) {
			if (SystemInterface::instance->getCommandId (&item) == SystemInterface::CommandId_MediaItem) {
				RecordStore::instance->addRecord (&item);
			}
		}
	}
}

void MediaUi::removeTagButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	IconLabelWindow *icon;
	Panel *panel;
	TextField *textfield;
	int count;

	ui = (MediaUi *) uiPtr;
	if (ui->selectedMediaMap.empty ()) {
		return;
	}
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaUi::removeTagButtonClicked)) {
		return;
	}
	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextString::RemoveMediaTagCommandName));
	action->setDescriptionText (UiText::instance->getText (UiTextString::MediaUiRemoveTagPrompt));
	textfield = new TextField (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth);
	action->addOption (UiText::instance->getText (UiTextString::SearchKey), textfield);
	action->setOptionNameText (UiText::instance->getText (UiTextString::SearchKey), StdString (""));
	action->setOptionNotEmptyString (UiText::instance->getText (UiTextString::SearchKey));
	action->closeCallback = Widget::EventCallbackContext (MediaUi::removeTagActionClosed, ui);

	panel = new Panel ();
	count = ui->getSelectedMediaCount ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::MediaTagItemCountTooltip, UiTextString::MediaTagItemsCountTooltip));

	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, MediaUi::removeTagButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
	App::instance->uiStack.setKeyFocusTarget (textfield);
}

void MediaUi::removeTagActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	ActionWindow *action;
	JsonList commands;
	StringList agentids;
	HashMap::Iterator i;
	MediaWindow *media;
	Json *params, *cmd;
	StdString mediaid, tag, cmdtext;

	ui = (MediaUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if ((! action->isConfirmed) || ui->selectedMediaMap.empty ()) {
		return;
	}
	tag = action->getStringValue (UiText::instance->getText (UiTextString::SearchKey), StdString (""));
	i = ui->selectedMediaMap.begin ();
	while (ui->selectedMediaMap.hasNext (&i)) {
		mediaid = ui->selectedMediaMap.next (&i);
		media = (MediaWindow *) ui->cardView->findItem (MediaUi::matchMediaItem, &mediaid);
		if (media) {
			params = new Json ();
			params->set ("mediaId", media->mediaId);
			params->set ("tag", tag);
			cmd = App::instance->createCommand (SystemInterface::Command_RemoveMediaTag, params);
			if (cmd) {
				agentids.push_back (media->agentId);
				commands.push_back (cmd);
				if (cmdtext.empty ()) {
					cmdtext.assign (media->mediaName);
				}
			}
		}
	}
	if (commands.size () > 1) {
		cmdtext.sprintf ("%i %s", (int) commands.size (), UiText::instance->getText (UiTextString::MediaFiles).c_str ());
	}
	ui->invokeCommand (CommandHistory::instance->removeMediaTag (agentids, (int) commands.size (), cmdtext), agentids, &commands, MediaUi::invokeMediaCommandComplete);
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
	App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextString::CreatedPlaylist).capitalized ().c_str (), playlist->playlistName.c_str ()));
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
	UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->rootPanel->width * MediaUi::TextTruncateWidthScale, StdString::createSprintf ("... (%i)", count));

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
	UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->rootPanel->width * MediaUi::TextTruncateWidthScale, StdString::createSprintf ("... (%i)", (int) names.size ()));

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

void MediaUi::resetSearchStatus () {
	IconLabelWindow *icon;
	std::map<StdString, MediaUi::MediaServerInfo>::iterator i, end;
	int recordcount, setsize;

	icon = (IconLabelWindow *) searchStatusIcon.widget;
	if (! icon) {
		return;
	}
	recordcount = 0;
	setsize = 0;
	SDL_LockMutex (mediaServerMapMutex);
	i = mediaServerMap.begin ();
	end = mediaServerMap.end ();
	while (i != end) {
		recordcount += i->second.recordCount;
		setsize += i->second.setSize;
		++i;
	}
	SDL_UnlockMutex (mediaServerMapMutex);

	if (setsize <= 0) {
		icon->setText (StdString ("0"));
		icon->setTextColor (UiConfiguration::instance->inverseTextColor);
		icon->setIconImageColor (UiConfiguration::instance->inverseTextColor);
		icon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::SearchEmptyResultTooltip));
	}
	else if (recordcount >= setsize) {
		icon->setText (StdString::createSprintf ("%i", setsize));
		icon->setTextColor (UiConfiguration::instance->inverseTextColor);
		icon->setIconImageColor (UiConfiguration::instance->inverseTextColor);
		icon->setMouseHoverTooltip (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextString::SearchComplete).capitalized ().c_str (), UiText::instance->getCountText (setsize, UiTextString::Result, UiTextString::Results).c_str ()));
	}
	else {
		icon->setText (StdString::createSprintf ("%i / %i", recordcount, setsize));
		icon->setTextColor (UiConfiguration::instance->mediumSecondaryColor);
		icon->setIconImageColor (UiConfiguration::instance->mediumSecondaryColor);
		icon->setMouseHoverTooltip (StdString::createSprintf ("%s: %s %s", UiText::instance->getText (UiTextString::SearchInProgress).capitalized ().c_str (), UiText::instance->getCountText (setsize, UiTextString::Result, UiTextString::Results).c_str (), UiText::instance->getText (UiTextString::SearchInProgressTooltip).c_str ()));
	}
}

void MediaUi::resetNextRecordSyncTime () {
	if (nextRecordSyncTime <= 0) {
		nextRecordSyncTime = OsUtil::getTime () + UiConfiguration::instance->recordSyncDelayDuration;
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

int MediaUi::getSelectedMediaCount () {
	return ((int) selectedMediaMap.size ());
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
			bytes += MediaUtil::getStreamSize (media->mediaSize, media->mediaBitrate, media->mediaDuration, profile);
		}
	}
	return (bytes);
}
