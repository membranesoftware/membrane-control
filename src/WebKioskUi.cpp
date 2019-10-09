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
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Resource.h"
#include "SpriteGroup.h"
#include "OsUtil.h"
#include "Ui.h"
#include "SystemInterface.h"
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

WebKioskUi::WebKioskUi ()
: Ui ()
, toolbarMode (-1)
, agentCount (0)
, cardView (NULL)
, addressField (NULL)
, addUrlButton (NULL)
, browseUrlButton (NULL)
, showUrlButton (NULL)
, addPlaylistButton (NULL)
, writePlaylistButton (NULL)
, clearDisplayButton (NULL)
, deletePlaylistButton (NULL)
{

}

WebKioskUi::~WebKioskUi () {

}

StdString WebKioskUi::getSpritePath () {
	return (StdString ("ui/WebKioskUi/sprite"));
}

Widget *WebKioskUi::createBreadcrumbWidget () {
	return (new Chip (App::instance->uiText.getText (UiTextString::webKiosk).capitalized (), sprites.getSprite (WebKioskUi::BreadcrumbIconSprite)));
}

void WebKioskUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);

	helpWindow->setHelpText (uitext->getText (UiTextString::webKioskUiHelpTitle), uitext->getText (UiTextString::webKioskUiHelpText));
	if (agentCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::webKioskUiHelpAction1Text));
	}
	else {
		helpWindow->addAction (uitext->getText (UiTextString::webKioskUiHelpAction2Text));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::membraneSoftwareOverviewHelpTitle), App::getHelpUrl ("membrane-software-overview"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

WebPlaylistWindow *WebKioskUi::createWebPlaylistWindow () {
	WebPlaylistWindow *playlist;

	playlist = new WebPlaylistWindow ();
	playlist->setSelectStateChangeCallback (WebKioskUi::playlistSelectStateChanged, this);
	playlist->setExpandStateChangeCallback (WebKioskUi::playlistExpandStateChanged, this);
	playlist->setNameClickCallback (WebKioskUi::playlistNameClicked, this);
	playlist->setUrlListChangeCallback (WebKioskUi::playlistUrlListChanged, this);

	return (playlist);
}

int WebKioskUi::doLoad () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	panel = new Panel ();
	panel->setFillBg (true, uiconfig->mediumBackgroundColor);
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->setImageColor (uiconfig->flatButtonTextColor);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::minimizeAll).capitalized (), uitext->getText (UiTextString::expandAll).capitalized ());
	toggle->setStateChangeCallback (WebKioskUi::expandAgentsToggleStateChanged, this);
	expandAgentsToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::monitors).capitalized (), panel), StdString (""), WebKioskUi::AgentToggleRow);

	cardView->setRowReverseSorted (WebKioskUi::ExpandedAgentRow, true);

	panel = new Panel ();
	panel->setFillBg (true, uiconfig->mediumBackgroundColor);
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->setImageColor (uiconfig->flatButtonTextColor);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::minimizeAll).capitalized (), uitext->getText (UiTextString::expandAll).capitalized ());
	toggle->setStateChangeCallback (WebKioskUi::expandPlaylistsToggleStateChanged, this);
	expandPlaylistsToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::playlists).capitalized (), panel), StdString (""), WebKioskUi::PlaylistToggleRow);

	return (Result::Success);
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

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (WebKioskUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);
}

void WebKioskUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	addressField = new TextFieldWindow (App::instance->windowWidth * 0.4f, uitext->getText (UiTextString::enterUrlPrompt), uiconfig->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite));
	addressField->setPadding (uiconfig->paddingSize, 0.0f);
	addressField->setButtonsEnabled (false, false, true, true);
	toolbar->addLeftItem (addressField);

	browseUrlButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::BrowseUrlButtonSprite));
	browseUrlButton->setInverseColor (true);
	browseUrlButton->setMouseClickCallback (WebKioskUi::browseUrlButtonClicked, this);
	browseUrlButton->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiBrowseUrlTooltip));
	browseUrlButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	browseUrlButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addLeftItem (browseUrlButton);

	toolbarMode = App::instance->prefsMap.find (App::WebKioskUiToolbarModeKey, (int) -1);
	if (toolbarMode < 0) {
		toolbarMode = WebKioskUi::MonitorMode;
	}
	setToolbarMode (toolbarMode, true);
}

void WebKioskUi::doClearPopupWidgets () {
	commandPopup.destroyAndClear ();
}

void WebKioskUi::doResume () {
	WebPlaylistWindow *playlist;
	Json *obj;
	StringList items;
	StringList::iterator i, end;
	StdString id, name;
	int count;

	App::instance->setNextBackgroundTexturePath ("ui/WebKioskUi/bg");

	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	if (! isFirstResumeComplete) {
		App::instance->prefsMap.find (App::WebKioskUiPlaylistsKey, &items);
		count = 0;
		i = items.begin ();
		end = items.end ();
		while (i != end) {
			obj = new Json ();
			if (obj->parse (*i)) {
				playlist = createWebPlaylistWindow ();
				playlist->setState (obj);

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
				playlist->sortKey.assign (playlist->playlistName.lowercased ());
				cardView->addItem (playlist, id, playlist->isExpanded ? WebKioskUi::ExpandedPlaylistRow : WebKioskUi::UnexpandedPlaylistRow);
				playlist->animateNewCard ();
			}
			delete (obj);
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

	resetExpandToggles ();
}

void WebKioskUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	cardView->refresh ();
}

void WebKioskUi::doPause () {
	StringList items;

	App::instance->prefsMap.insert (App::WebKioskUiToolbarModeKey, toolbarMode);

	cardView->processItems (WebKioskUi::appendPlaylistJson, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::WebKioskUiPlaylistsKey);
	}
	else {
		App::instance->prefsMap.insert (App::WebKioskUiPlaylistsKey, &items);
	}

	items.clear ();
	cardView->processItems (WebKioskUi::appendSelectedAgentId, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::WebKioskUiSelectedAgentsKey);
	}
	else {
		App::instance->prefsMap.insert (App::WebKioskUiSelectedAgentsKey, &items);
	}

	items.clear ();
	cardView->processItems (WebKioskUi::appendExpandedAgentId, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::WebKioskUiExpandedAgentsKey);
	}
	else {
		App::instance->prefsMap.insert (App::WebKioskUiExpandedAgentsKey, &items);
	}
}

void WebKioskUi::appendPlaylistJson (void *stringListPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Json *json;

	window = WebPlaylistWindow::castWidget (widgetPtr);
	if (window) {
		json = window->getState ();
		((StringList *) stringListPtr)->push_back (json->toString ());
		delete (json);
	}
}

void WebKioskUi::appendSelectedAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = MonitorWindow::castWidget (widgetPtr);
	if (window && window->isSelected) {
		((StringList *) stringListPtr)->push_back (window->agentId);
	}
}

void WebKioskUi::appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = MonitorWindow::castWidget (widgetPtr);
	if (window && window->isExpanded) {
		((StringList *) stringListPtr)->push_back (window->agentId);
	}
}

void WebKioskUi::doUpdate (int msElapsed) {
	commandPopup.compact ();
	commandPopupSource.compact ();
	expandAgentsToggle.compact ();
	expandPlaylistsToggle.compact ();
}

void WebKioskUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	addressField->setWindowWidth (App::instance->windowWidth * 0.5f);
}

bool WebKioskUi::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	int mode;

	if (keycode == SDLK_TAB) {
		mode = toolbarMode + 1;
		if (mode >= WebKioskUi::ModeCount) {
			mode = 0;
		}
		setToolbarMode (mode);
		return (true);
	}

	return (false);
}

void WebKioskUi::handleLinkClientConnect (const StdString &agentId) {
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_WatchStatus, SystemInterface::Constant_Admin), agentId);
}

void WebKioskUi::doSyncRecordStore () {
	RecordStore *store;

	store = &(App::instance->agentControl.recordStore);

	agentCount = 0;
	store->processAgentRecords ("monitorServerStatus", WebKioskUi::processAgentStatus, this);
	cardView->syncRecordStore ();
	cardView->refresh ();
	resetExpandToggles ();
}

void WebKioskUi::processAgentStatus (void *uiPtr, Json *record, const StdString &recordId) {
	WebKioskUi *ui;
	SystemInterface *interface;
	MonitorWindow *monitor;
	StringList items;
	int row, pos;

	ui = (WebKioskUi *) uiPtr;
	interface = &(App::instance->systemInterface);

	++(ui->agentCount);
	if (! ui->cardView->contains (recordId)) {
		monitor = new MonitorWindow (recordId);
		monitor->setScreenshotDisplayEnabled (true);
		monitor->setSelectStateChangeCallback (WebKioskUi::agentSelectStateChanged, ui);
		monitor->setExpandStateChangeCallback (WebKioskUi::agentExpandStateChanged, ui);
		monitor->setScreenshotLoadCallback (WebKioskUi::agentScreenshotLoaded, ui);

		App::instance->prefsMap.find (App::WebKioskUiSelectedAgentsKey, &items);
		if (items.contains (recordId)) {
			monitor->setSelected (true, true);
			ui->selectedAgentMap.insert (recordId, interface->getCommandAgentName (record));
		}

		App::instance->prefsMap.find (App::WebKioskUiExpandedAgentsKey, &items);
		pos = items.indexOf (recordId);
		if (pos < 0) {
			row = WebKioskUi::UnexpandedAgentRow;
			monitor->sortKey.assign (interface->getCommandAgentName (record).lowercased ());
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

void WebKioskUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->cardView->processItems (WebKioskUi::reloadAgent, ui);
}

void WebKioskUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (! monitor) {
		return;
	}

	App::instance->agentControl.refreshAgentStatus (monitor->agentId);
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
}

void WebKioskUi::playlistExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;

	ui = (WebKioskUi *) uiPtr;
	playlist = (WebPlaylistWindow *) widgetPtr;

	if (playlist->isExpanded) {
		ui->cardView->setItemRow (playlist->itemId, WebKioskUi::ExpandedPlaylistRow);
	}
	else {
		playlist->sortKey.assign (playlist->playlistName.lowercased ());
		ui->cardView->setItemRow (playlist->itemId, WebKioskUi::UnexpandedPlaylistRow);
	}
	playlist->resetInputState ();
	playlist->animateNewCard ();
	ui->resetExpandToggles ();
	ui->cardView->refresh ();
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
	ui->cardView->processItems (WebKioskUi::appendAgentId, &idlist);
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

void WebKioskUi::appendAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
	}
}

void WebKioskUi::expandPlaylistsToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	Toggle *toggle;
	WebPlaylistWindow *playlist;
	StringList idlist;
	StringList::iterator i, end;

	ui = (WebKioskUi *) uiPtr;
	toggle = (Toggle *) ui->expandPlaylistsToggle.widget;
	if (! toggle) {
		return;
	}

	ui->cardView->processItems (WebKioskUi::appendPlaylistId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		playlist = WebPlaylistWindow::castWidget (ui->cardView->getItem (*i));
		if (playlist) {
			if (toggle->isChecked) {
				playlist->setExpanded (false, true);
				ui->cardView->setItemRow (playlist->itemId, WebKioskUi::UnexpandedPlaylistRow, true);
			}
			else {
				playlist->setExpanded (true, true);
				ui->cardView->setItemRow (playlist->itemId, WebKioskUi::ExpandedPlaylistRow, true);
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

void WebKioskUi::appendPlaylistId (void *stringListPtr, Widget *widgetPtr) {
	WebPlaylistWindow *playlist;

	playlist = WebPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		((StringList *) stringListPtr)->push_back (playlist->itemId);
	}
}

void WebKioskUi::addUrlButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	StdString url;

	ui = (WebKioskUi *) uiPtr;
	url = ui->addressField->getValue ();
	if (url.empty ()) {
		return;
	}

	url = OsUtil::getProtocolString (url);
	ui->cardView->processItems (WebKioskUi::addPlaylistUrl, &url);
}

void WebKioskUi::addPlaylistUrl (void *urlStringPtr, Widget *widgetPtr) {
	WebPlaylistWindow *playlist;
	StdString url;

	playlist = WebPlaylistWindow::castWidget (widgetPtr);
	if (playlist && playlist->isSelected) {
		url.assign (((StdString *) urlStringPtr)->c_str ());
		playlist->addUrl (url);
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", App::instance->uiText.getText (UiTextString::websiteAddedMessage).c_str (), playlist->playlistName.c_str ()));
	}
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

	result = OsUtil::openUrl (OsUtil::getProtocolString (url));
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::launchWebBrowserError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void WebKioskUi::showUrlButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	StdString url;
	StringList idlist;
	Json *params;
	int count;

	ui = (WebKioskUi *) uiPtr;
	url = ui->addressField->getValue ();
	ui->selectedAgentMap.getKeys (&idlist);
	if (url.empty () || idlist.empty ()) {
		return;
	}

	params = new Json ();
	params->set ("url", OsUtil::getProtocolString (url));
	count = App::instance->agentControl.invokeCommand (&idlist, App::instance->createCommand (SystemInterface::Command_ShowWebUrl, SystemInterface::Constant_Monitor, params));
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeShowWebUrlMessage));
	}
}

void WebKioskUi::addPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;
	StdString name, id;

	ui = (WebKioskUi *) uiPtr;

	name = ui->getAvailablePlaylistName ();
	id = ui->cardView->getAvailableItemId ();
	playlist = ui->createWebPlaylistWindow ();
	playlist->itemId.assign (id);
	playlist->setPlaylistName (name);
	playlist->setExpanded (true);
	ui->cardView->addItem (playlist, id, WebKioskUi::ExpandedPlaylistRow);
	playlist->setSelected (true);
	playlist->animateNewCard ();
	ui->cardView->scrollToItem (id);
	App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", App::instance->uiText.getText (UiTextString::webPlaylistCreatedMessage).c_str (), name.c_str ()));
}

void WebKioskUi::deletePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *item;

	ui = (WebKioskUi *) uiPtr;
	item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
	if (item) {
		ui->cardView->removeItem (item->itemId);
		ui->commandPopup.destroyAndClear ();
		ui->commandPopupSource.clear ();
	}
}

void WebKioskUi::writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *item;
	StringList idlist;
	int count;

	ui = (WebKioskUi *) uiPtr;
	ui->selectedAgentMap.getKeys (&idlist);
	if (ui->selectedPlaylistId.empty () || idlist.empty ()) {
		return;
	}
	item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
	if ((! item) || (item->getItemCount () <= 0)) {
		return;
	}

	count = App::instance->agentControl.invokeCommand (&idlist, item->getCreateCommand ());
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeCreateWebPlaylistMessage));
	}
}

void WebKioskUi::playlistNameClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	WebPlaylistWindow *playlist;
	TextFieldWindow *action;

	ui = (WebKioskUi *) uiPtr;
	playlist = (WebPlaylistWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	ui->clearPopupWidgets ();
	action = (TextFieldWindow *) App::instance->rootPanel->addWidget (new TextFieldWindow (playlist->width - (uiconfig->marginSize * 2.0f), uitext->getText (UiTextString::enterWebPlaylistNamePrompt)));
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (playlist);
	action->setValue (playlist->playlistName);
	action->setEditCallback (WebKioskUi::playlistNameEdited, ui);
	action->setFillBg (true, uiconfig->lightPrimaryColor);
	action->setButtonsEnabled (true, true, true, true);
	action->assignKeyFocus ();
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (playlist->screenX + uiconfig->marginSize, playlist->screenY);
}

void WebKioskUi::playlistNameEdited (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *playlist;
	TextFieldWindow *action;
	StdString name, id;

	ui = (WebKioskUi *) uiPtr;
	action = (TextFieldWindow *) ui->actionWidget.widget;
	playlist = (WebPlaylistWindow *) ui->actionTarget.widget;
	if ((! action) || (! playlist)) {
		return;
	}

	name = action->getValue ();
	if (name.empty ()) {
		name.assign (ui->getAvailablePlaylistName ());
	}
	playlist->setPlaylistName (name);
	playlist->sortKey.assign (playlist->playlistName.lowercased ());
	ui->clearPopupWidgets ();
}

StdString WebKioskUi::getAvailablePlaylistName () {
	StdString basename, name, id;
	int i;

	basename.assign (App::instance->uiText.getText (UiTextString::websiteList).capitalized ());
	name.assign (basename);
	cardView->processItems (WebKioskUi::matchPlaylistName, &name);
	if (name.empty ()) {
		i = 2;
		while (true) {
			name.sprintf ("%s %i", basename.c_str (), i);
			cardView->processItems (WebKioskUi::matchPlaylistName, &name);
			if (! name.empty ()) {
				break;
			}
			++i;
		}
	}

	return (name);
}

void WebKioskUi::matchPlaylistName (void *stringPtr, Widget *widgetPtr) {
	WebPlaylistWindow *playlist;
	StdString *name;

	playlist = WebPlaylistWindow::castWidget (widgetPtr);
	if (! playlist) {
		return;
	}

	name = (StdString *) stringPtr;
	if (name->lowercased ().equals (playlist->playlistName.lowercased ())) {
		name->assign ("");
	}
}

void WebKioskUi::playlistUrlListChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->refresh ();
}

void WebKioskUi::setToolbarMode (int mode, bool forceReset) {
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
	addUrlButton = NULL;
	showUrlButton = NULL;
	addPlaylistButton = NULL;
	writePlaylistButton = NULL;
	clearDisplayButton = NULL;
	deletePlaylistButton = NULL;

	button = new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::ToolsButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (WebKioskUi::modeButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::toolbarModeButtonTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (button);

	toolbarMode = mode;
	switch (toolbarMode) {
		case WebKioskUi::MonitorMode: {
			button = new Button (StdString (""), sprites.getSprite (WebKioskUi::ShowUrlButtonSprite));
			button->setInverseColor (true);
			button->shortcutKey = SDLK_F3;
			button->setMouseClickCallback (WebKioskUi::showUrlButtonClicked, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiShowUrlTooltip), Widget::LeftAlignment);
			button->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
			toolbar->addRightItem (button);
			showUrlButton = button;

			button = new Button (StdString (""), sprites.getSprite (WebKioskUi::WritePlaylistButtonSprite));
			button->setInverseColor (true);
			button->shortcutKey = SDLK_F2;
			button->setMouseClickCallback (WebKioskUi::writePlaylistButtonClicked, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiWritePlaylistTooltip), Widget::LeftAlignment);
			button->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
			toolbar->addRightItem (button);
			writePlaylistButton = button;

			button = new Button (StdString (""), sprites.getSprite (WebKioskUi::ClearDisplayButtonSprite));
			button->setInverseColor (true);
			button->shortcutKey = SDLK_F1;
			button->setMouseClickCallback (WebKioskUi::clearDisplayButtonClicked, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiClearDisplayTooltip), Widget::LeftAlignment);
			button->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
			toolbar->addRightItem (button);
			clearDisplayButton = button;
			break;
		}
		case WebKioskUi::PlaylistMode: {
			button = new Button (StdString (""), sprites.getSprite (WebKioskUi::AddUrlButtonSprite));
			button->setInverseColor (true);
			button->shortcutKey = SDLK_F3;
			button->setMouseClickCallback (WebKioskUi::addUrlButtonClicked, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiAddUrlTooltip), Widget::LeftAlignment);
			button->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
			toolbar->addRightItem (button);
			addUrlButton = button;

			button = new Button (StdString (""), sprites.getSprite (WebKioskUi::AddPlaylistButtonSprite));
			button->setInverseColor (true);
			button->shortcutKey = SDLK_F2;
			button->setMouseClickCallback (WebKioskUi::addPlaylistButtonClicked, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiAddPlaylistTooltip), Widget::LeftAlignment);
			button->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
			toolbar->addRightItem (button);
			addPlaylistButton = button;

			button = new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite));
			button->setInverseColor (true);
			button->setMouseClickCallback (WebKioskUi::deletePlaylistButtonClicked, this);
			button->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiDeletePlaylistTooltip), Widget::LeftAlignment);
			button->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
			button->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
			toolbar->addRightItem (button);
			deletePlaylistButton = button;
			break;
		}
	}
}

void WebKioskUi::modeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	UiText *uitext;
	Menu *menu;
	bool show;

	ui = (WebKioskUi *) uiPtr;
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
	menu->addItem (uitext->getText (UiTextString::viewWebsites).capitalized (), ui->sprites.getSprite (WebKioskUi::ShowUrlButtonSprite), WebKioskUi::monitorModeActionClicked, ui, 0, ui->toolbarMode == WebKioskUi::MonitorMode);
	menu->addItem (uitext->getText (UiTextString::managePlaylists).capitalized (), ui->sprites.getSprite (WebKioskUi::AddPlaylistButtonSprite), WebKioskUi::playlistModeActionClicked, ui, 0, ui->toolbarMode == WebKioskUi::PlaylistMode);
	menu->position.assign (widgetPtr->screenX + widgetPtr->width - menu->width, widgetPtr->screenY - menu->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void WebKioskUi::monitorModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->setToolbarMode (WebKioskUi::MonitorMode);
}

void WebKioskUi::playlistModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->setToolbarMode (WebKioskUi::PlaylistMode);
}

void WebKioskUi::clearDisplayButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	StringList idlist;
	int count;

	ui = (WebKioskUi *) uiPtr;
	ui->selectedAgentMap.getKeys (&idlist);
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

void WebKioskUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	Button *button;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	WebPlaylistWindow *item;
	StdString text;
	Color color;

	ui = (WebKioskUi *) uiPtr;
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
	if (button == ui->writePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::runPlaylist).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			color.assign (uiconfig->errorTextColor);
			text.assign (uitext->getText (UiTextString::webKioskUiNoAgentSelectedPrompt));
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
		if (! item) {
			text.assign (uitext->getText (UiTextString::webKioskUiNoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else if (item->getItemCount () <= 0) {
			text.assign (uitext->getText (UiTextString::webKioskUiEmptyPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (item->playlistName);
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearDisplayButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::clear).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskUiNoAgentSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->addUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::addWebsite).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
		if (! item) {
			text.assign (uitext->getText (UiTextString::webKioskUiNoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (item->playlistName);
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskUiNoAddressEnteredPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->showUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::showWebsite).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskUiNoAgentSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskUiNoAddressEnteredPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->browseUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::browseWebsite).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskUiNoAddressEnteredPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->addPlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::createPlaylist).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));
	}
	else if (button == ui->deletePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::deletePlaylist).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
		if (! item) {
			text.assign (uitext->getText (UiTextString::webKioskUiNoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (item->playlistName);
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
	Label::truncateText (&text, UiConfiguration::CaptionFont, maxWidth, StdString::createSprintf ("... (%i)", count));

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
	ui->cardView->refresh ();
}

void WebKioskUi::agentScreenshotLoaded (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->cardView->refresh ();
}

void WebKioskUi::resetExpandToggles () {
	Toggle *toggle;
	int count;

	toggle = (Toggle *) expandAgentsToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (WebKioskUi::countExpandedAgents, &count);
		toggle->setChecked ((count <= 0), true);
	}

	toggle = (Toggle *) expandPlaylistsToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (WebKioskUi::countExpandedPlaylists, &count);
		toggle->setChecked ((count <= 0), true);
	}
}

void WebKioskUi::countExpandedAgents (void *intPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;
	int *count;

	monitor = MonitorWindow::castWidget (widgetPtr);
	count = (int *) intPtr;
	if (monitor && count && monitor->isExpanded) {
		++(*count);
	}
}

void WebKioskUi::countExpandedPlaylists (void *intPtr, Widget *widgetPtr) {
	WebPlaylistWindow *playlist;
	int *count;

	playlist = WebPlaylistWindow::castWidget (widgetPtr);
	count = (int *) intPtr;
	if (playlist && count && playlist->isExpanded) {
		++(*count);
	}
}
