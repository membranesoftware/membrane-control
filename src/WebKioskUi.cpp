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
#include "SDL2/SDL.h"
#include "App.h"
#include "Log.h"
#include "StdString.h"
#include "Resource.h"
#include "SpriteGroup.h"
#include "OsUtil.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "SystemInterface.h"
#include "Agent.h"
#include "StringList.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "Json.h"
#include "Toggle.h"
#include "Button.h"
#include "Menu.h"
#include "Chip.h"
#include "TextFieldWindow.h"
#include "ActionWindow.h"
#include "HelpWindow.h"
#include "CardView.h"
#include "IconLabelWindow.h"
#include "MonitorWindow.h"
#include "WebPlaylistWindow.h"
#include "WebKioskUi.h"

const char *WebKioskUi::SelectedAgentsKey = "WebKiosk_SelectedAgents";
const char *WebKioskUi::ExpandedAgentsKey = "WebKiosk_ExpandedAgents";
const char *WebKioskUi::PlaylistsKey = "WebKiosk_Playlists";

WebKioskUi::WebKioskUi ()
: Ui ()
, agentCount (0)
, addressField (NULL)
, browseUrlButton (NULL)
, showUrlButton (NULL)
, writePlaylistButton (NULL)
, clearDisplayButton (NULL)
{

}

WebKioskUi::~WebKioskUi () {

}

StdString WebKioskUi::getSpritePath () {
	return (StdString ("ui/WebKioskUi/sprite"));
}

Widget *WebKioskUi::createBreadcrumbWidget () {
	return (new Chip (UiText::instance->getText (UiTextString::WebKiosk).capitalized (), sprites.getSprite (WebKioskUi::BreadcrumbIconSprite)));
}

void WebKioskUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextString::WebKioskUiHelpTitle), UiText::instance->getText (UiTextString::WebKioskUiHelpText));
	if (agentCount <= 0) {
		helpWindow->addAction (UiText::instance->getText (UiTextString::WebKioskUiHelpAction1Text));
	}
	else {
		helpWindow->addAction (UiText::instance->getText (UiTextString::WebKioskUiHelpAction2Text));
	}
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::MembraneSoftwareOverviewHelpTitle), App::getHelpUrl ("membrane-software-overview"));
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

WebPlaylistWindow *WebKioskUi::createWebPlaylistWindow () {
	WebPlaylistWindow *playlist;

	playlist = new WebPlaylistWindow (&sprites);
	playlist->selectStateChangeCallback = Widget::EventCallbackContext (WebKioskUi::playlistSelectStateChanged, this);
	playlist->expandStateChangeCallback = Widget::EventCallbackContext (WebKioskUi::playlistExpandStateChanged, this);
	playlist->nameClickCallback = Widget::EventCallbackContext (WebKioskUi::playlistNameClicked, this);
	playlist->urlListChangeCallback = Widget::EventCallbackContext (WebKioskUi::playlistUrlListChanged, this);
	playlist->removeClickCallback = Widget::EventCallbackContext (WebKioskUi::playlistRemoveActionClicked, this);
	playlist->addItemClickCallback = Widget::EventCallbackContext (WebKioskUi::playlistAddItemActionClicked, this);
	playlist->addItemMouseEnterCallback = Widget::EventCallbackContext (WebKioskUi::playlistAddItemMouseEntered, this);
	playlist->addItemMouseExitCallback = Widget::EventCallbackContext (WebKioskUi::playlistAddItemMouseExited, this);
	return (playlist);
}

OsUtil::Result WebKioskUi::doLoad () {
	Panel *panel;
	Toggle *toggle;
	Button *button;

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (WebKioskUi::expandAgentsToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::MinimizeAll).capitalized (), UiText::instance->getText (UiTextString::ExpandAll).capitalized ());
	expandAgentsToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (UiText::instance->getText (UiTextString::Monitors).capitalized (), panel), StdString (""), WebKioskUi::AgentToggleRow);

	cardView->setRowReverseSorted (WebKioskUi::ExpandedAgentRow, true);
	cardView->setRowReverseSorted (WebKioskUi::ExpandedPlaylistRow, true);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (WebKioskUi::expandPlaylistsToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::MinimizeAll).capitalized (), UiText::instance->getText (UiTextString::ExpandAll).capitalized ());
	expandPlaylistsToggle.assign (toggle);

	button = (Button *) panel->addWidget (new Button (sprites.getSprite (WebKioskUi::CreatePlaylistButtonSprite)));
	button->mouseClickCallback = Widget::EventCallbackContext (WebKioskUi::createPlaylistButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::WebKioskUiAddPlaylistTooltip));

	panel->setLayout (Panel::HorizontalLayout);
	cardView->addItem (createRowHeaderPanel (UiText::instance->getText (UiTextString::Playlists).capitalized (), panel), StdString (""), WebKioskUi::PlaylistToggleRow);

	return (OsUtil::Success);
}

void WebKioskUi::doUnload () {
	selectedAgentMap.clear ();
	selectedPlaylistId.assign ("");
	commandPopup.destroyAndClear ();
	commandPopupSource.clear ();
	expandAgentsToggle.clear ();
	expandPlaylistsToggle.clear ();
}

void WebKioskUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (WebKioskUi::reloadButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ReloadTooltip));
	toolbar->addRightItem (button);
}

void WebKioskUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	addressField = new TextFieldWindow (App::instance->windowWidth * 0.4f, UiText::instance->getText (UiTextString::EnterUrlPrompt), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite));
	addressField->setPadding (UiConfiguration::instance->paddingSize, 0.0f);
	addressField->setButtonsEnabled (false, false, true, true);
	toolbar->addLeftItem (addressField);

	browseUrlButton = new Button (sprites.getSprite (WebKioskUi::BrowseUrlButtonSprite));
	browseUrlButton->mouseClickCallback = Widget::EventCallbackContext (WebKioskUi::browseUrlButtonClicked, this);
	browseUrlButton->mouseEnterCallback = Widget::EventCallbackContext (WebKioskUi::commandButtonMouseEntered, this);
	browseUrlButton->mouseExitCallback = Widget::EventCallbackContext (WebKioskUi::commandButtonMouseExited, this);
	browseUrlButton->setInverseColor (true);
	browseUrlButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::WebKioskUiBrowseUrlTooltip));
	toolbar->addLeftItem (browseUrlButton);

	showUrlButton = new Button (sprites.getSprite (WebKioskUi::ShowUrlButtonSprite));
	showUrlButton->mouseClickCallback = Widget::EventCallbackContext (WebKioskUi::showUrlButtonClicked, this);
	showUrlButton->mouseEnterCallback = Widget::EventCallbackContext (WebKioskUi::commandButtonMouseEntered, this);
	showUrlButton->mouseExitCallback = Widget::EventCallbackContext (WebKioskUi::commandButtonMouseExited, this);
	showUrlButton->setInverseColor (true);
	showUrlButton->shortcutKey = SDLK_F3;
	showUrlButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::WebKioskUiShowUrlTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (showUrlButton);

	writePlaylistButton = new Button (sprites.getSprite (WebKioskUi::WritePlaylistButtonSprite));
	writePlaylistButton->mouseClickCallback = Widget::EventCallbackContext (WebKioskUi::writePlaylistButtonClicked, this);
	writePlaylistButton->mouseEnterCallback = Widget::EventCallbackContext (WebKioskUi::commandButtonMouseEntered, this);
	writePlaylistButton->mouseExitCallback = Widget::EventCallbackContext (WebKioskUi::commandButtonMouseExited, this);
	writePlaylistButton->setInverseColor (true);
	writePlaylistButton->shortcutKey = SDLK_F2;
	writePlaylistButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::WebKioskUiWritePlaylistTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (writePlaylistButton);

	clearDisplayButton = new Button (sprites.getSprite (WebKioskUi::ClearDisplayButtonSprite));
	clearDisplayButton->mouseClickCallback = Widget::EventCallbackContext (WebKioskUi::clearDisplayButtonClicked, this);
	clearDisplayButton->mouseEnterCallback = Widget::EventCallbackContext (WebKioskUi::commandButtonMouseEntered, this);
	clearDisplayButton->mouseExitCallback = Widget::EventCallbackContext (WebKioskUi::commandButtonMouseExited, this);
	clearDisplayButton->setInverseColor (true);
	clearDisplayButton->shortcutKey = SDLK_F1;
	clearDisplayButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::WebKioskUiClearDisplayTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (clearDisplayButton);
}

void WebKioskUi::doClearPopupWidgets () {
	commandPopup.destroyAndClear ();
}

void WebKioskUi::doResume () {
	HashMap *prefs;
	WebPlaylistWindow *playlist;
	JsonList playlists;
	JsonList::iterator i, end;
	StdString id, name;
	int count;
	int64_t now;

	App::instance->setNextBackgroundTexturePath ("ui/WebKioskUi/bg");

	if (! isFirstResumeComplete) {
		prefs = App::instance->lockPrefs ();
		prefs->find (WebKioskUi::PlaylistsKey, &playlists);
		// TODO: Remove this operation (when transition from the legacy key is no longer needed)
		App::transferJsonListPrefs (prefs, "WebKiosk_Playlists", &playlists);
		App::instance->unlockPrefs ();

		now = OsUtil::getTime ();
		count = 0;
		i = playlists.begin ();
		end = playlists.end ();
		while (i != end) {
			playlist = createWebPlaylistWindow ();
			playlist->readState (*i);

			id = cardView->getAvailableItemId ();
			if (selectedPlaylistId.empty ()) {
				if (playlist->isSelected) {
					selectedPlaylistId.assign (id);
				}
			}
			else {
				if (playlist->isSelected) {
					playlist->setSelected (false, true);
				}
			}
			playlist->itemId.assign (id);
			if (playlist->isExpanded) {
				playlist->sortKey.sprintf ("%016llx%s", (long long int) now, playlist->playlistName.lowercased ().c_str ());
				cardView->addItem (playlist, id, WebKioskUi::ExpandedPlaylistRow, true);
			}
			else {
				playlist->sortKey.assign (playlist->playlistName.lowercased ());
				cardView->addItem (playlist, id, WebKioskUi::UnexpandedPlaylistRow, true);
			}
			playlist->animateNewCard ();
			++count;
			++i;
		}

		if (count <= 0) {
			name = getAvailablePlaylistName ();
			id = cardView->getAvailableItemId ();
			playlist = createWebPlaylistWindow ();
			playlist->itemId.assign (id);
			playlist->setPlaylistName (name);
			playlist->setExpanded (true);
			cardView->addItem (playlist, id, WebKioskUi::ExpandedPlaylistRow);
			playlist->setSelected (true);
			playlist->animateNewCard ();
		}
	}

	cardView->refresh ();
	resetExpandToggles ();
}

static void doPause_appendPlaylistState (void *jsonListPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;

	window = WebPlaylistWindow::castWidget (widgetPtr);
	if (window) {
		((JsonList *) jsonListPtr)->push_back (window->createState ());
	}
}
static void doPause_appendSelectedAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = MonitorWindow::castWidget (widgetPtr);
	if (window && window->isSelected) {
		((StringList *) stringListPtr)->push_back (window->agentId);
	}
}
static void doPause_appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = MonitorWindow::castWidget (widgetPtr);
	if (window && window->isExpanded) {
		((StringList *) stringListPtr)->push_back (window->agentId);
	}
}
void WebKioskUi::doPause () {
	HashMap *prefs;
	JsonList playlists;
	StringList ids;

	cardView->processItems (doPause_appendPlaylistState, &playlists);
	prefs = App::instance->lockPrefs ();
	prefs->insert (WebKioskUi::PlaylistsKey, &playlists);
	App::instance->unlockPrefs ();

	ids.clear ();
	cardView->processItems (doPause_appendSelectedAgentId, &ids);
	prefs = App::instance->lockPrefs ();
	prefs->insert (WebKioskUi::SelectedAgentsKey, ids);
	App::instance->unlockPrefs ();

	ids.clear ();
	cardView->processItems (doPause_appendExpandedAgentId, &ids);
	prefs = App::instance->lockPrefs ();
	prefs->insert (WebKioskUi::ExpandedAgentsKey, ids);
	App::instance->unlockPrefs ();
}

void WebKioskUi::doUpdate (int msElapsed) {
	commandPopup.compact ();
	commandPopupSource.compact ();
	expandAgentsToggle.compact ();
	expandPlaylistsToggle.compact ();
}

void WebKioskUi::doResize () {
	addressField->setWindowWidth (App::instance->windowWidth * 0.5f);
}

void WebKioskUi::handleLinkClientConnect (const StdString &agentId) {
	AgentControl::instance->writeLinkCommand (App::instance->createCommand (SystemInterface::Command_WatchStatus), agentId);
}

void WebKioskUi::doSyncRecordStore () {
	agentCount = 0;
	RecordStore::instance->processAgentRecords ("monitorServerStatus", WebKioskUi::doSyncRecordStore_processAgentStatus, this);
	cardView->syncRecordStore ();
	cardView->refresh ();
	resetExpandToggles ();
}

void WebKioskUi::doSyncRecordStore_processAgentStatus (void *uiPtr, Json *record, const StdString &recordId) {
	WebKioskUi *ui;
	HashMap *prefs;
	MonitorWindow *monitor;
	StringList items;
	int row, pos;

	ui = (WebKioskUi *) uiPtr;
	++(ui->agentCount);
	if (! ui->cardView->contains (recordId)) {
		monitor = new MonitorWindow (recordId);
		monitor->setScreenshotDisplayEnabled (true);
		monitor->setSelectEnabled (true);
		monitor->selectStateChangeCallback = Widget::EventCallbackContext (WebKioskUi::agentSelectStateChanged, ui);
		monitor->expandStateChangeCallback = Widget::EventCallbackContext (WebKioskUi::agentExpandStateChanged, ui);
		monitor->screenshotLoadCallback = Widget::EventCallbackContext (WebKioskUi::agentScreenshotLoaded, ui);

		prefs = App::instance->lockPrefs ();
		prefs->find (WebKioskUi::SelectedAgentsKey, &items);
		if (items.contains (recordId)) {
			monitor->setSelected (true, true);
			ui->selectedAgentMap.insert (recordId, Agent::getCommandAgentName (record));
		}
		prefs->find (WebKioskUi::ExpandedAgentsKey, &items);
		App::instance->unlockPrefs ();

		pos = items.indexOf (recordId);
		if (pos < 0) {
			row = WebKioskUi::UnexpandedAgentRow;
			monitor->sortKey.assign (Agent::getCommandAgentName (record).lowercased ());
		}
		else {
			row = WebKioskUi::ExpandedAgentRow;
			monitor->setExpanded (true, true);
			monitor->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
		}

		ui->cardView->addItem (monitor, recordId, row);
		monitor->animateNewCard ();
		ui->addLinkAgent (recordId);
	}
}

static void reloadButtonClicked_processItem (void *uiPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		AgentControl::instance->refreshAgentStatus (monitor->agentId);
	}
}
void WebKioskUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->cardView->processItems (reloadButtonClicked_processItem, ui);
}

void WebKioskUi::agentSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	MonitorWindow *monitor;

	ui = (WebKioskUi *) uiPtr;
	monitor = (MonitorWindow *) widgetPtr;
	if (monitor->isSelected) {
		ui->selectedAgentMap.insert (monitor->agentId, monitor->agentName);
	}
	else {
		ui->selectedAgentMap.remove (monitor->agentId);
	}
	ui->clearPopupWidgets ();
}

void WebKioskUi::playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist, *item;

	ui = (WebKioskUi *) uiPtr;
	playlist = (WebPlaylistWindow *) widgetPtr;
	if (playlist->isSelected) {
		if (! ui->selectedPlaylistId.equals (playlist->itemId)) {
			if (! ui->selectedPlaylistId.empty ()) {
				item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
				if (item) {
					item->setSelected (false, true);
				}
			}
			ui->selectedPlaylistId.assign (playlist->itemId);
		}
	}
	else {
		ui->selectedPlaylistId.assign ("");
	}
	ui->clearPopupWidgets ();
}

void WebKioskUi::playlistExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;

	ui = (WebKioskUi *) uiPtr;
	playlist = (WebPlaylistWindow *) widgetPtr;

	if (playlist->isExpanded) {
		playlist->sortKey.sprintf ("%016llx%s", (long long int) OsUtil::getTime (), playlist->playlistName.lowercased ().c_str ());
		ui->cardView->setItemRow (playlist->itemId, WebKioskUi::ExpandedPlaylistRow, true);
	}
	else {
		playlist->sortKey.assign (playlist->playlistName.lowercased ());
		ui->cardView->setItemRow (playlist->itemId, WebKioskUi::UnexpandedPlaylistRow, true);
	}
	playlist->resetInputState ();
	playlist->animateNewCard ();
	ui->resetExpandToggles ();
	ui->clearPopupWidgets ();
	ui->cardView->refresh ();
}

static void expandAgentsToggleStateChanged_appendAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
	}
}
void WebKioskUi::expandAgentsToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	Toggle *toggle;
	MonitorWindow *monitor;
	StringList idlist;
	StringList::iterator i, end;
	int64_t now;

	ui = (WebKioskUi *) uiPtr;
	toggle = (Toggle *) ui->expandAgentsToggle.widget;
	if (! toggle) {
		return;
	}
	now = OsUtil::getTime ();
	ui->cardView->processItems (expandAgentsToggleStateChanged_appendAgentId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		monitor = MonitorWindow::castWidget (ui->cardView->getItem (*i));
		if (monitor) {
			if (toggle->isChecked) {
				monitor->setExpanded (false, true);
				monitor->sortKey.assign (monitor->agentName.lowercased ());
				ui->cardView->setItemRow (monitor->agentId, WebKioskUi::UnexpandedAgentRow, true);
			}
			else {
				monitor->setExpanded (true, true);
				monitor->sortKey.sprintf ("%016llx%s", (long long int) now, monitor->agentName.lowercased ().c_str ());
				ui->cardView->setItemRow (monitor->agentId, WebKioskUi::ExpandedAgentRow, true);
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

static void expandPlaylistsToggleStateChanged_appendPlaylistId (void *stringListPtr, Widget *widgetPtr) {
	WebPlaylistWindow *playlist;

	playlist = WebPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		((StringList *) stringListPtr)->push_back (playlist->itemId);
	}
}
void WebKioskUi::expandPlaylistsToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	Toggle *toggle;
	WebPlaylistWindow *playlist;
	StringList idlist;
	StringList::iterator i, end;
	int64_t now;

	ui = (WebKioskUi *) uiPtr;
	toggle = (Toggle *) ui->expandPlaylistsToggle.widget;
	if (! toggle) {
		return;
	}
	now = OsUtil::getTime ();
	ui->cardView->processItems (expandPlaylistsToggleStateChanged_appendPlaylistId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		playlist = WebPlaylistWindow::castWidget (ui->cardView->getItem (*i));
		if (playlist) {
			playlist->setExpanded (! toggle->isChecked, true);
			if (playlist->isExpanded) {
				playlist->sortKey.sprintf ("%016llx%s", (long long int) now, playlist->playlistName.lowercased ().c_str ());
				ui->cardView->setItemRow (playlist->itemId, WebKioskUi::ExpandedPlaylistRow, true);
			}
			else {
				playlist->sortKey.assign (playlist->playlistName.lowercased ());
				ui->cardView->setItemRow (playlist->itemId, WebKioskUi::UnexpandedPlaylistRow, true);
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

void WebKioskUi::browseUrlButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	StdString url;
	int result;

	ui = (WebKioskUi *) uiPtr;
	url = ui->addressField->getValue ();
	if (url.empty ()) {
		return;
	}
	ui->clearPopupWidgets ();
	result = OsUtil::openUrl (OsUtil::getProtocolString (url));
	if (result != OsUtil::Success) {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::LaunchWebBrowserError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::LaunchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void WebKioskUi::showUrlButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	StdString url;
	StringList idlist;
	Json *params;

	ui = (WebKioskUi *) uiPtr;
	url = ui->addressField->getValue ();
	ui->selectedAgentMap.getKeys (&idlist);
	if (url.empty () || idlist.empty ()) {
		return;
	}
	url.assign (OsUtil::getProtocolString (url));

	ui->clearPopupWidgets ();
	params = new Json ();
	params->set ("url", url);
	ui->invokeCommand (CommandHistory::instance->showWebUrl (idlist, url), idlist, App::instance->createCommand (SystemInterface::Command_ShowWebUrl, params), WebKioskUi::invokeCommandComplete);
}

void WebKioskUi::invokeCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess) {
	if (isResponseCommandSuccess) {
		AgentControl::instance->refreshAgentStatus (agentId);
	}
}

void WebKioskUi::createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;
	StdString name;

	ui = (WebKioskUi *) uiPtr;
	name = ui->getAvailablePlaylistName ();
	playlist = ui->createWebPlaylistWindow ();
	playlist->itemId.assign (ui->cardView->getAvailableItemId ());
	playlist->setPlaylistName (name);
	playlist->setExpanded (true);
	ui->cardView->addItem (playlist, playlist->itemId, WebKioskUi::ExpandedPlaylistRow);

	playlist->setSelected (true);
	playlist->animateNewCard ();
	ui->cardView->scrollToItem (playlist->itemId);
	ui->cardView->refresh ();
	ui->clearPopupWidgets ();
	ui->resetExpandToggles ();
	App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextString::CreatedPlaylist).capitalized ().c_str (), name.c_str ()));
}

void WebKioskUi::writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;
	StringList idlist;

	ui = (WebKioskUi *) uiPtr;
	ui->selectedAgentMap.getKeys (&idlist);
	if (ui->selectedPlaylistId.empty () || idlist.empty ()) {
		return;
	}
	playlist = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
	if ((! playlist) || (playlist->getItemCount () <= 0)) {
		return;
	}
	ui->clearPopupWidgets ();
	ui->invokeCommand (CommandHistory::instance->createWebDisplayIntent (idlist, playlist->playlistName), idlist, playlist->createCommand (), WebKioskUi::invokeCommandComplete);
}

void WebKioskUi::playlistNameClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;
	TextFieldWindow *textfield;

	ui = (WebKioskUi *) uiPtr;
	playlist = (WebPlaylistWindow *) widgetPtr;

	ui->clearPopupWidgets ();
	textfield = new TextFieldWindow (playlist->windowWidth, UiText::instance->getText (UiTextString::EnterPlaylistNamePrompt));
	textfield->setValue (playlist->playlistName);
	textfield->valueEditCallback = Widget::EventCallbackContext (WebKioskUi::playlistNameEdited, ui);
	textfield->enterButtonClickCallback = Widget::EventCallbackContext (WebKioskUi::playlistNameEditEnterButtonClicked, ui);
	textfield->setFillBg (true, UiConfiguration::instance->lightPrimaryColor);
	textfield->setButtonsEnabled (true, true, true, true);
	textfield->shouldSkipTextClearCallbacks = true;
	textfield->assignKeyFocus ();

	ui->showActionPopup (textfield, playlist, WebKioskUi::playlistNameClicked, playlist->getScreenRect (), Ui::LeftEdgeAlignment, Ui::TopEdgeAlignment);
}

void WebKioskUi::playlistNameEdited (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;
	TextFieldWindow *textfield;

	ui = (WebKioskUi *) uiPtr;
	textfield = TextFieldWindow::castWidget (ui->actionWidget.widget);
	playlist = WebPlaylistWindow::castWidget (ui->actionTarget.widget);
	if ((! textfield) || (! playlist)) {
		return;
	}
	playlist->setPlaylistName (ui->getAvailablePlaylistName (textfield->getValue ()));
	playlist->sortKey.assign (playlist->playlistName.lowercased ());
	ui->clearPopupWidgets ();
}

void WebKioskUi::playlistNameEditEnterButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->clearPopupWidgets ();
}

static void getAvailablePlaylistName_matchName (void *stringPtr, Widget *widgetPtr) {
	WebPlaylistWindow *playlist;
	StdString *name;

	playlist = WebPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		name = (StdString *) stringPtr;
		if (name->lowercased ().equals (playlist->playlistName.lowercased ())) {
			name->assign ("");
		}
	}
}
StdString WebKioskUi::getAvailablePlaylistName (const StdString &baseName) {
	StdString base, name;
	int i;

	if (baseName.empty ()) {
		base.assign (UiText::instance->getText (UiTextString::WebsiteList).capitalized ());
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

void WebKioskUi::playlistUrlListChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->refresh ();
}

void WebKioskUi::playlistRemoveActionClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;
	ActionWindow *action;

	ui = (WebKioskUi *) uiPtr;
	playlist = (WebPlaylistWindow *) widgetPtr;
	if (ui->clearActionPopup (widgetPtr, WebKioskUi::playlistRemoveActionClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiConfiguration::instance->fonts[UiConfiguration::TitleFont]->truncatedText (playlist->playlistName, playlist->width * 0.34f, Font::DotTruncateSuffix));
	action->setDescriptionText (UiText::instance->getText (UiTextString::RemovePlaylistDescription));
	action->closeCallback = Widget::EventCallbackContext (WebKioskUi::playlistRemoveActionClosed, ui);

	ui->showActionPopup (action, widgetPtr, WebKioskUi::playlistRemoveActionClicked, playlist->getRemoveButtonScreenRect (), Ui::LeftEdgeAlignment, Ui::TopOfAlignment);
}

void WebKioskUi::playlistRemoveActionClosed (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	ActionWindow *action;
	WebPlaylistWindow *playlist;

	ui = (WebKioskUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}
	playlist = WebPlaylistWindow::castWidget (ui->actionTarget.widget);
	if (playlist) {
		ui->cardView->removeItem (playlist->itemId);
		ui->resetExpandToggles ();
	}
}

void WebKioskUi::playlistAddItemActionClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;
	StdString url;

	ui = (WebKioskUi *) uiPtr;
	url = ui->addressField->getValue ();
	if (url.empty ()) {
		return;
	}
	playlist = WebPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		playlist->addUrl (OsUtil::getProtocolString (url));
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextString::WebsiteAddedMessage).c_str (), playlist->playlistName.c_str ()));
	}
}

void WebKioskUi::playlistAddItemMouseEntered (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	StdString text;
	Color color;

	ui = (WebKioskUi *) uiPtr;
	playlist = (WebPlaylistWindow *) widgetPtr;

	ui->commandPopup.destroyAndClear ();
	ui->commandPopupSource.assign (widgetPtr);
	panel = (Panel *) App::instance->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	panel->setBorder (true, Color (UiConfiguration::instance->darkBackgroundColor.r, UiConfiguration::instance->darkBackgroundColor.g, UiConfiguration::instance->darkBackgroundColor.b, UiConfiguration::instance->overlayWindowAlpha));

	label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::AddWebsite).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));
	label->setPadding (0.0f, 0.0f);

	text.assign (playlist->playlistName);
	UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->rootPanel->width * 0.20f, Font::DotTruncateSuffix);

	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setRightAligned (true);

	text.assign (ui->addressField->getValue ());
	if (text.empty ()) {
		text.assign (UiText::instance->getText (UiTextString::WebKioskUiNoAddressEnteredPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->rootPanel->width * 0.20f, Font::DotTruncateSuffix);
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite), text, UiConfiguration::CaptionFont, color));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setRightAligned (true);

	panel->setLayout (Panel::VerticalRightJustifiedLayout);
	ui->assignPopupPosition (panel, playlist->getAddItemButtonScreenRect (), Ui::RightOfAlignment, Ui::YCenteredAlignment);
}

void WebKioskUi::playlistAddItemMouseExited (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	if (ui->commandPopupSource.widget == widgetPtr) {
		ui->commandPopup.destroyAndClear ();
		ui->commandPopupSource.clear ();
	}
}

void WebKioskUi::clearDisplayButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	StringList idlist;

	ui = (WebKioskUi *) uiPtr;
	ui->selectedAgentMap.getKeys (&idlist);
	if (idlist.empty ()) {
		return;
	}
	ui->clearPopupWidgets ();
	ui->invokeCommand (CommandHistory::instance->clearDisplay (idlist), idlist, App::instance->createCommand (SystemInterface::Command_ClearDisplay), WebKioskUi::invokeCommandComplete);
}

void WebKioskUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	Button *button;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	WebPlaylistWindow *playlist;
	StdString text;
	Color color;

	ui = (WebKioskUi *) uiPtr;
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
	if (button == ui->writePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::RunPlaylist).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			color.assign (UiConfiguration::instance->errorTextColor);
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);

		playlist = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
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
			UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->windowWidth * 0.4f, Font::DotTruncateSuffix);
			color.assign (UiConfiguration::instance->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearDisplayButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::Clear).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->showUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::ShowWebsite).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::WebKioskUiNoAddressEnteredPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		else {
			UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->rootPanel->width * 0.20f, Font::DotTruncateSuffix);
			color.assign (UiConfiguration::instance->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->browseUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::BrowseWebsite).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::WebKioskUiNoAddressEnteredPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		else {
			UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, App::instance->rootPanel->width * 0.20f, Font::DotTruncateSuffix);
			color.assign (UiConfiguration::instance->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite), text, UiConfiguration::CaptionFont, color));
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

void WebKioskUi::commandButtonMouseExited (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	Button *button;

	ui = (WebKioskUi *) uiPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);

	if (ui->commandPopupSource.widget == button) {
		ui->commandPopup.destroyAndClear ();
		ui->commandPopupSource.clear ();
	}
}

StdString WebKioskUi::getSelectedAgentNames (float maxWidth) {
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
	UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, maxWidth, StdString::createSprintf ("... (%i)", count));

	return (text);
}

void WebKioskUi::agentExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	MonitorWindow *monitor;

	ui = (WebKioskUi *) uiPtr;
	monitor = (MonitorWindow *) widgetPtr;

	if (monitor->isExpanded) {
		monitor->sortKey.sprintf ("%016llx", (long long int) OsUtil::getTime ());
		ui->cardView->setItemRow (monitor->agentId, WebKioskUi::ExpandedAgentRow);
	}
	else {
		monitor->sortKey.assign (monitor->agentName.lowercased ());
		ui->cardView->setItemRow (monitor->agentId, WebKioskUi::UnexpandedAgentRow);
	}
	monitor->resetInputState ();
	monitor->animateNewCard ();
	ui->resetExpandToggles ();
	ui->clearPopupWidgets ();
	ui->cardView->refresh ();
}

void WebKioskUi::agentScreenshotLoaded (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->cardView->refresh ();
}

static void resetExpandToggles_countExpandedAgents (void *intPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;
	int *count;

	monitor = MonitorWindow::castWidget (widgetPtr);
	count = (int *) intPtr;
	if (monitor && count && monitor->isExpanded) {
		++(*count);
	}
}
static void resetExpandToggles_countExpandedPlaylists (void *intPtr, Widget *widgetPtr) {
	WebPlaylistWindow *playlist;
	int *count;

	playlist = WebPlaylistWindow::castWidget (widgetPtr);
	count = (int *) intPtr;
	if (playlist && count && playlist->isExpanded) {
		++(*count);
	}
}
void WebKioskUi::resetExpandToggles () {
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
