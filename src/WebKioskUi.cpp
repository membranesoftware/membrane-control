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

const StdString WebKioskUi::serverApplicationName = StdString ("Membrane Monitor");

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
	WebPlaylistWindow *window;

	window = new WebPlaylistWindow ();
	window->setSelectStateChangeCallback (WebKioskUi::playlistSelectStateChanged, this);
	window->setExpandStateChangeCallback (WebKioskUi::playlistExpandStateChanged, this);
	window->setNameClickCallback (WebKioskUi::playlistNameClicked, this);
	window->setUrlListChangeCallback (WebKioskUi::playlistUrlListChanged, this);

	return (window);
}

int WebKioskUi::doLoad () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (WebKioskUi::AgentRow, uitext->getText (UiTextString::webKiosks).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor);
	cardView->setRowHeader (WebKioskUi::PlaylistRow, uitext->getText (UiTextString::programs).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	return (Result::Success);
}

void WebKioskUi::doUnload () {
	selectedAgentMap.clear ();
	selectedPlaylistId.assign ("");
	commandPopup.destroyAndClear ();
	commandPopupSource.clear ();
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
	Json *obj;
	WebPlaylistWindow *window;
	StringList items;
	StringList::iterator i, end;
	StdString id;
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
				window = createWebPlaylistWindow ();
				window->setState (obj);

				id = cardView->getAvailableItemId ();
				if (selectedPlaylistId.empty ()) {
					if (window->isSelected) {
						selectedPlaylistId.assign (id);
					}
				}
				else {
					if (window->isSelected) {
						window->setSelected (false, true);
					}
				}
				window->itemId.assign (id);
				window->sortKey.assign (window->playlistName.lowercased ());
				cardView->addItem (window, id, WebKioskUi::PlaylistRow);
				window->animateNewCard ();
			}
			delete (obj);
			++count;
			++i;
		}

		if (count <= 0) {
			WebKioskUi::addPlaylistButtonClicked (this, NULL);
		}
	}
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

void WebKioskUi::doSyncRecordStore () {
	RecordStore *store;

	store = &(App::instance->agentControl.recordStore);
	agentCount = store->countRecords (WebKioskUi::matchWebKioskAgentStatus, NULL, App::instance->prefsMap.find (App::ServerTimeoutKey, App::defaultServerTimeout) * 1000);

	store->processCommandRecords (SystemInterface::CommandId_AgentStatus, WebKioskUi::processAgentStatus, this);
	cardView->syncRecordStore ();
	cardView->refresh ();
}

bool WebKioskUi::matchWebKioskAgentStatus (void *ptr, Json *record) {
	SystemInterface *interface;

	interface = &(App::instance->systemInterface);
	if (interface->getCommandId (record) == SystemInterface::CommandId_AgentStatus) {
		if (interface->getCommandStringParam (record, "applicationName", "").equals (WebKioskUi::serverApplicationName)) {
			return (true);
		}
	}

	return (false);
}

void WebKioskUi::processAgentStatus (void *uiPtr, Json *record, const StdString &recordId) {
	WebKioskUi *ui;
	SystemInterface *interface;
	MonitorWindow *window;
	StdString name;
	StringList items;

	ui = (WebKioskUi *) uiPtr;
	interface = &(App::instance->systemInterface);

	if (! ui->cardView->contains (recordId)) {
		name = interface->getCommandStringParam (record, "applicationName", "");
		if (name.equals (WebKioskUi::serverApplicationName)) {
			if (interface->getCommandObjectParam (record, "monitorServerStatus", NULL)) {
				window = new MonitorWindow (recordId);
				window->setSelectStateChangeCallback (WebKioskUi::agentSelectStateChanged, ui);
				window->setExpandStateChangeCallback (WebKioskUi::agentExpandStateChanged, ui);
				window->sortKey.assign (interface->getCommandAgentName (record).lowercased ());

				App::instance->prefsMap.find (App::WebKioskUiSelectedAgentsKey, &items);
				if (items.contains (recordId)) {
					window->setSelected (true, true);
					ui->selectedAgentMap.insert (recordId, interface->getCommandAgentName (record));
				}

				App::instance->prefsMap.find (App::WebKioskUiExpandedAgentsKey, &items);
				if (items.contains (recordId)) {
					window->setExpanded (true, true);
				}

				ui->cardView->addItem (window, recordId, WebKioskUi::AgentRow);
				window->animateNewCard ();
			}
		}
	}
}

void WebKioskUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->cardView->processItems (WebKioskUi::reloadAgent, ui);
}

void WebKioskUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = MonitorWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	App::instance->agentControl.refreshAgentStatus (window->agentId);
}

void WebKioskUi::agentSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	MonitorWindow *window;

	ui = (WebKioskUi *) uiPtr;
	window = (MonitorWindow *) widgetPtr;
	if (window->isSelected) {
		ui->selectedAgentMap.insert (window->agentId, window->agentName);
	}
	else {
		ui->selectedAgentMap.remove (window->agentId);
	}
}

void WebKioskUi::playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *window, *item;

	ui = (WebKioskUi *) uiPtr;
	window = (WebPlaylistWindow *) widgetPtr;
	if (window->isSelected) {
		if (! ui->selectedPlaylistId.equals (window->itemId)) {
			if (! ui->selectedPlaylistId.empty ()) {
				item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
				if (item) {
					item->setSelected (false, true);
				}
			}
			ui->selectedPlaylistId.assign (window->itemId);
		}
	}
	else {
		ui->selectedPlaylistId.assign ("");
	}
}

void WebKioskUi::playlistExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->cardView->refresh ();
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
	WebPlaylistWindow *window;
	StdString url;

	window = WebPlaylistWindow::castWidget (widgetPtr);
	if (window && window->isSelected) {
		url.assign (((StdString *) urlStringPtr)->c_str ());
		window->addUrl (url);
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", App::instance->uiText.getText (UiTextString::websiteAddedMessage).c_str (), window->playlistName.c_str ()));
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
	StdString url, id;
	HashMap::Iterator i;
	Json *params;
	int result, count;

	ui = (WebKioskUi *) uiPtr;
	url = ui->addressField->getValue ();
	if (url.empty ()) {
		return;
	}

	if (ui->selectedAgentMap.empty ()) {
		return;
	}

	count = 0;
	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);
		params = new Json ();
		params->set ("url", OsUtil::getProtocolString (url));
		result = App::instance->agentControl.invokeCommand (id, App::instance->createCommand (SystemInterface::Command_ShowWebUrl, SystemInterface::Constant_Monitor, params));
		if (result != Result::Success) {
			Log::debug ("Failed to invoke ShowWebUrl command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
		else {
			++count;
			App::instance->agentControl.refreshAgentStatus (id);
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeShowWebUrlMessage));
	}
}

void WebKioskUi::addPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *window;
	StdString name, id;

	ui = (WebKioskUi *) uiPtr;

	name = ui->getAvailablePlaylistName ();
	id = ui->cardView->getAvailableItemId ();
	window = ui->createWebPlaylistWindow ();
	window->itemId.assign (id);
	window->setPlaylistName (name);
	window->setExpanded (true);
	ui->cardView->addItem (window, id, WebKioskUi::PlaylistRow);
	window->setSelected (true);
	window->animateNewCard ();
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
	HashMap::Iterator i;
	StdString id;
	int result, count;

	ui = (WebKioskUi *) uiPtr;
	if (ui->selectedAgentMap.empty () || ui->selectedPlaylistId.empty ()) {
		return;
	}

	item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
	if (! item) {
		return;
	}

	count = 0;
	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);
		ui->retain ();
		result = App::instance->agentControl.invokeCommand (id, item->getCreateCommand (), WebKioskUi::writePlaylistComplete, ui);
		if (result != Result::Success) {
			Log::debug ("Failed to invoke WebPlaylistWindow create command; err=%i agentId=\"%s\"", result, id.c_str ());
			ui->release ();
		}
		else {
			++count;
			App::instance->agentControl.refreshAgentStatus (id);
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeCreateWebPlaylistMessage));
	}
}

void WebKioskUi::writePlaylistComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	WebKioskUi *ui;
	SystemInterface *interface;

	ui = (WebKioskUi *) uiPtr;
	interface = &(App::instance->systemInterface);

	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::CommandId_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		App::instance->agentControl.refreshAgentStatus (agentId);
	}
	ui->release ();
}

void WebKioskUi::playlistNameClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	WebPlaylistWindow *window;
	TextFieldWindow *action;

	ui = (WebKioskUi *) uiPtr;
	window = (WebPlaylistWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	ui->clearPopupWidgets ();
	action = (TextFieldWindow *) App::instance->rootPanel->addWidget (new TextFieldWindow (window->width - (uiconfig->marginSize * 2.0f), uitext->getText (UiTextString::enterWebPlaylistNamePrompt)));
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (window);
	action->setValue (window->playlistName);
	action->setEditCallback (WebKioskUi::playlistNameEdited, ui);
	action->setFillBg (true, uiconfig->lightPrimaryColor);
	action->setButtonsEnabled (true, true, true, true);
	action->setEditing (true);
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (window->screenX + uiconfig->marginSize, window->screenY);
}

void WebKioskUi::playlistNameEdited (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *target;
	TextFieldWindow *action;
	StdString name, id;

	ui = (WebKioskUi *) uiPtr;
	action = (TextFieldWindow *) ui->actionWidget.widget;
	target = (WebPlaylistWindow *) ui->actionTarget.widget;
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
	WebPlaylistWindow *window;
	StdString *name;

	window = WebPlaylistWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	name = (StdString *) stringPtr;
	if (name->lowercased ().equals (window->playlistName.lowercased ())) {
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
	menu->addItem (uitext->getText (UiTextString::managePrograms).capitalized (), ui->sprites.getSprite (WebKioskUi::AddPlaylistButtonSprite), WebKioskUi::playlistModeActionClicked, ui, 0, ui->toolbarMode == WebKioskUi::PlaylistMode);
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
	HashMap::Iterator i;
	StdString id;
	int result, count;

	ui = (WebKioskUi *) uiPtr;
	if (ui->selectedAgentMap.empty ()) {
		return;
	}

	count = 0;
	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);
		result = App::instance->agentControl.invokeCommand (id, App::instance->createCommand (SystemInterface::Command_ClearDisplay, SystemInterface::Constant_Monitor, NULL));
		if (result != Result::Success) {
			Log::debug ("Failed to invoke ClearDisplay command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
		else {
			++count;
			App::instance->agentControl.refreshAgentStatus (id);
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
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
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::runProgram).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			color.assign (uiconfig->errorTextColor);
			text.assign (uitext->getText (UiTextString::webKioskNoAgentsSelectedPrompt));
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
		if (! item) {
			text.assign (uitext->getText (UiTextString::webKioskNoPlaylistsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (item->playlistName);
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallProgramIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearDisplayButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::clear).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskNoAgentsSelectedPrompt));
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
			text.assign (uitext->getText (UiTextString::webKioskNoPlaylistsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (item->playlistName);
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallProgramIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskNoAddressEnteredPrompt));
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
			text.assign (uitext->getText (UiTextString::webKioskNoAgentsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskNoAddressEnteredPrompt));
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
			text.assign (uitext->getText (UiTextString::webKioskNoAddressEnteredPrompt));
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
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::createProgram).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));
	}
	else if (button == ui->deletePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::deleteProgram).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
		if (! item) {
			text.assign (uitext->getText (UiTextString::webKioskNoPlaylistsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (item->playlistName);
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallProgramIconSprite), text, UiConfiguration::CaptionFont, color));
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

	ui = (WebKioskUi *) uiPtr;
	ui->cardView->refresh ();
}
