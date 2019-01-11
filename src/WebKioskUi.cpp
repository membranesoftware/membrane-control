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
#include "Toggle.h"
#include "Button.h"
#include "Menu.h"
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
, agentCount (0)
, cardView (NULL)
, addressField (NULL)
, addUrlButton (NULL)
, browseUrlButton (NULL)
, showUrlButton (NULL)
, addPlaylistButton (NULL)
, writePlaylistButton (NULL)
, clearDisplayButton (NULL)
{

}

WebKioskUi::~WebKioskUi () {

}

StdString WebKioskUi::getSpritePath () {
	return (StdString ("ui/WebKioskUi/sprite"));
}

StdString WebKioskUi::getBreadcrumbTitle () {
	return (App::getInstance ()->uiText.getText (UiTextString::webKiosk).capitalized ());
}

Sprite *WebKioskUi::getBreadcrumbSprite () {
	return (sprites.getSprite (WebKioskUi::BREADCRUMB_ICON)->copy ());
}

void WebKioskUi::setHelpWindowContent (Widget *helpWindowPtr) {
	App *app;
	HelpWindow *help;
	UiText *uitext;

	help = (HelpWindow *) helpWindowPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);

	help->setHelpText (uitext->getText (UiTextString::webKioskUiHelpTitle), uitext->getText (UiTextString::webKioskUiHelpText));
	if (agentCount <= 0) {
		help->addAction (uitext->getText (UiTextString::webKioskUiHelpAction1Text));
	}
	else {
		help->addAction (uitext->getText (UiTextString::webKioskUiHelpAction2Text));
	}

	help->addTopicLink (uitext->getText (UiTextString::membraneSoftwareOverviewHelpTitle), Util::getHelpUrl ("membrane-software-overview"));
	help->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), Util::getHelpUrl (""));
}

WebPlaylistWindow *WebKioskUi::createWebPlaylistWindow () {
	WebPlaylistWindow *window;

	window = new WebPlaylistWindow ();
	window->setSelectStateChangeCallback (WebKioskUi::playlistSelectStateChanged, this);
	window->setExpandStateChangeCallback (WebKioskUi::playlistExpandStateChanged, this);
	window->setNameClickCallback (WebKioskUi::playlistNameClicked, this);
	window->setMenuClickCallback (WebKioskUi::playlistMenuClicked, this);
	window->setUrlListChangeCallback (WebKioskUi::playlistUrlListChanged, this);

	return (window);
}

int WebKioskUi::doLoad () {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (0, uitext->getText (UiTextString::webKiosks).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->setRowHeader (1, uitext->getText (UiTextString::programs).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, app->topBarHeight);

	return (Result::SUCCESS);
}

void WebKioskUi::doUnload () {
	selectedAgentMap.clear ();
	selectedPlaylistId.assign ("");
	actionWidget.destroyAndClear ();
	actionTarget.clear ();
	commandPopup.destroyAndClear ();
	commandButton.clear ();
}

void WebKioskUi::doResetMainToolbar (Toolbar *toolbar) {
	App *app;
	Button *button;

	app = App::getInstance ();
	button = new Button (StdString (""), app->uiConfig.coreSprites.getSprite (UiConfiguration::RELOAD_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (WebKioskUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void WebKioskUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	addressField = new TextFieldWindow (app->windowWidth * 0.5f, uitext->getText (UiTextString::enterUrlPrompt), uiconfig->coreSprites.getSprite (UiConfiguration::WEB_LINK_ICON));
	addressField->setPadding (uiconfig->paddingSize, 0.0f);
	addressField->setButtonsEnabled (false, false, true, true);
	toolbar->addLeftItem (addressField);

	browseUrlButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::BROWSE_URL_BUTTON));
	browseUrlButton->setInverseColor (true);
	browseUrlButton->shortcutKey = SDLK_F1;
	browseUrlButton->setMouseClickCallback (WebKioskUi::browseUrlButtonClicked, this);
	browseUrlButton->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiBrowseUrlTooltip));
	browseUrlButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	browseUrlButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addLeftItem (browseUrlButton);

	showUrlButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::SHOW_URL_BUTTON));
	showUrlButton->setInverseColor (true);
	showUrlButton->shortcutKey = SDLK_F6;
	showUrlButton->setMouseClickCallback (WebKioskUi::showUrlButtonClicked, this);
	showUrlButton->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiShowUrlTooltip), Widget::LEFT);
	showUrlButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	showUrlButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addRightItem (showUrlButton);

	writePlaylistButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::WRITE_INTENT_BUTTON));
	writePlaylistButton->setInverseColor (true);
	writePlaylistButton->shortcutKey = SDLK_F5;
	writePlaylistButton->setMouseClickCallback (WebKioskUi::writePlaylistButtonClicked, this);
	writePlaylistButton->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiWritePlaylistTooltip), Widget::LEFT);
	writePlaylistButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	writePlaylistButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addRightItem (writePlaylistButton);

	clearDisplayButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::CLEAR_DISPLAY_BUTTON));
	clearDisplayButton->setInverseColor (true);
	clearDisplayButton->shortcutKey = SDLK_F4;
	clearDisplayButton->setMouseClickCallback (WebKioskUi::clearDisplayButtonClicked, this);
	clearDisplayButton->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiClearDisplayTooltip), Widget::LEFT);
	clearDisplayButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	clearDisplayButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addRightItem (clearDisplayButton);

	toolbar->addRightSpacer ();

	addUrlButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::ADD_URL_BUTTON));
	addUrlButton->setInverseColor (true);
	addUrlButton->shortcutKey = SDLK_F3;
	addUrlButton->setMouseClickCallback (WebKioskUi::addUrlButtonClicked, this);
	addUrlButton->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiAddUrlTooltip), Widget::LEFT);
	addUrlButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	addUrlButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addRightItem (addUrlButton);

	addPlaylistButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::ADD_INTENT_BUTTON));
	addPlaylistButton->setInverseColor (true);
	addPlaylistButton->shortcutKey = SDLK_F2;
	addPlaylistButton->setMouseClickCallback (WebKioskUi::addPlaylistButtonClicked, this);
	addPlaylistButton->setMouseHoverTooltip (uitext->getText (UiTextString::webKioskUiAddPlaylistTooltip), Widget::LEFT);
	addPlaylistButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	addPlaylistButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addRightItem (addPlaylistButton);

	toolbar->isVisible = true;
}

void WebKioskUi::doClearPopupWidgets () {
	actionWidget.destroyAndClear ();
	actionTarget.clear ();
	commandPopup.destroyAndClear ();
}

void WebKioskUi::doResume () {
	App *app;
	Json *obj;
	WebPlaylistWindow *window;
	StringList items;
	StringList::iterator i, end;
	StdString id;
	int count;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/WebKioskUi/bg");

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);

	if (! isFirstResumeComplete) {
		app->prefsMap.find (App::prefsWebKioskUiPlaylists, &items);
		count = 0;
		i = items.begin ();
		end = items.end ();
		while (i != end) {
			obj = new Json ();
			if (obj->parse (*i) == Result::SUCCESS) {
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
				cardView->addItem (window, id, 1);
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
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
	cardView->refresh ();
}

void WebKioskUi::doPause () {
	App *app;
	StringList items;

	app = App::getInstance ();
	cardView->processItems (WebKioskUi::appendPlaylistJson, &items);
	if (items.empty ()) {
		app->prefsMap.remove (App::prefsWebKioskUiPlaylists);
	}
	else {
		app->prefsMap.insert (App::prefsWebKioskUiPlaylists, &items);
	}

	items.clear ();
	cardView->processItems (WebKioskUi::appendExpandedAgentId, &items);
	if (items.empty ()) {
		app->prefsMap.remove (App::prefsWebKioskUiExpandedAgents);
	}
	else {
		app->prefsMap.insert (App::prefsWebKioskUiExpandedAgents, &items);
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

void WebKioskUi::appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = MonitorWindow::castWidget (widgetPtr);
	if (window && window->isExpanded) {
		((StringList *) stringListPtr)->push_back (window->agentId);
	}
}

void WebKioskUi::doUpdate (int msElapsed) {
	actionWidget.compact ();
	actionTarget.compact ();
	commandPopup.compact ();
	commandButton.compact ();
}

void WebKioskUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void WebKioskUi::doSyncRecordStore (RecordStore *store) {
	App *app;

	app = App::getInstance ();
	agentCount = store->countRecords (WebKioskUi::matchWebKioskAgentStatus, NULL, app->prefsMap.find (App::prefsServerTimeout, App::defaultServerTimeout) * 1000);

	store->processCommandRecords (SystemInterface::Command_AgentStatus, WebKioskUi::processAgentStatus, this);
	cardView->syncRecordStore (store);
	cardView->refresh ();
}

bool WebKioskUi::matchWebKioskAgentStatus (void *ptr, Json *record) {
	SystemInterface *interface;

	interface = &(App::getInstance ()->systemInterface);
	if (interface->getCommandId (record) == SystemInterface::Command_AgentStatus) {
		if (interface->getCommandStringParam (record, "applicationName", "").equals (WebKioskUi::serverApplicationName)) {
			return (true);
		}
	}

	return (false);
}

void WebKioskUi::processAgentStatus (void *uiPtr, Json *record, const StdString &recordId) {
	WebKioskUi *ui;
	App *app;
	SystemInterface *interface;
	MonitorWindow *window;
	StdString name;
	StringList items;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
	interface = &(app->systemInterface);

	if (! ui->cardView->contains (recordId)) {
		name = interface->getCommandStringParam (record, "applicationName", "");
		if (name.equals (WebKioskUi::serverApplicationName)) {
			if (interface->getCommandObjectParam (record, "monitorServerStatus", NULL)) {
				window = new MonitorWindow (recordId);
				window->setSelectStateChangeCallback (WebKioskUi::agentSelectStateChanged, ui);
				window->setExpandStateChangeCallback (WebKioskUi::agentExpandStateChanged, ui);
				window->sortKey.assign (interface->getCommandAgentName (record).lowercased ());

				app->prefsMap.find (App::prefsWebKioskUiExpandedAgents, &items);
				if (items.contains (recordId)) {
					window->setExpanded (true, true);
				}

				ui->cardView->addItem (window, recordId, 0);
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
	App *app;

	window = MonitorWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	app = App::getInstance ();
	app->agentControl.refreshAgentStatus (window->agentId);
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

	url = Util::getProtocolString (url);
	ui->cardView->processItems (WebKioskUi::addPlaylistUrl, &url);
}

void WebKioskUi::addPlaylistUrl (void *urlStringPtr, Widget *widgetPtr) {
	App *app;
	WebPlaylistWindow *window;
	StdString url;

	app = App::getInstance ();
	window = WebPlaylistWindow::castWidget (widgetPtr);
	if (window && window->isSelected) {
		url.assign (((StdString *) urlStringPtr)->c_str ());
		window->addUrl (url);
		app->showSnackbar (StdString::createSprintf ("%s: %s", app->uiText.getText (UiTextString::websiteAddedMessage).c_str (), window->playlistName.c_str ()));
	}
}

void WebKioskUi::browseUrlButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	StdString url;
	int result;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
	url = ui->addressField->getValue ();
	if (url.empty ()) {
		return;
	}

	result = Util::openUrl (Util::getProtocolString (url));
	if (result != Result::SUCCESS) {
		app->showSnackbar (app->uiText.getText (UiTextString::launchWebBrowserError));
	}
	else {
		app->showSnackbar (StdString::createSprintf ("%s - %s", app->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void WebKioskUi::showUrlButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	StdString url, id;
	HashMap::Iterator i;
	Json *params;
	int result, count;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
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
		params->set ("url", Util::getProtocolString (url));
		result = app->agentControl.invokeCommand (id, app->createCommand ("ShowWebUrl", SystemInterface::Constant_Monitor, params), NULL, NULL, NULL, id);
		if (result != Result::SUCCESS) {
			Log::write (Log::DEBUG, "Failed to invoke ShowWebUrl command; err=%i agentId=\"%s\"", result, id.c_str ());
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
		app->showSnackbar (app->uiText.getText (UiTextString::invokeShowWebUrlMessage));
	}
}

void WebKioskUi::addPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	WebPlaylistWindow *window;
	StdString name, id;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();

	name = ui->getAvailablePlaylistName ();
	id = ui->cardView->getAvailableItemId ();
	window = ui->createWebPlaylistWindow ();
	window->itemId.assign (id);
	window->setPlaylistName (name);
	window->setExpanded (true);
	ui->cardView->addItem (window, id, 1);
	window->setSelected (true);
	ui->cardView->scrollToItem (id);
	app->showSnackbar (StdString::createSprintf ("%s: %s", app->uiText.getText (UiTextString::webPlaylistCreatedMessage).c_str (), name.c_str ()));
}

void WebKioskUi::writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	WebPlaylistWindow *item;
	HashMap::Iterator i;
	StdString id;
	int result, count;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
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
		result = app->agentControl.invokeCommand (id, item->getCreateCommand (), WebKioskUi::writePlaylistComplete, ui, NULL, id);
		if (result != Result::SUCCESS) {
			Log::write (Log::DEBUG, "Failed to invoke WebPlaylistWindow create command; err=%i agentId=\"%s\"", result, id.c_str ());
			ui->release ();
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
		app->showSnackbar (app->uiText.getText (UiTextString::invokeCreateWebPlaylistMessage));
	}
}

void WebKioskUi::writePlaylistComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	WebKioskUi *ui;
	App *app;
	SystemInterface *interface;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
	interface = &(app->systemInterface);

	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		app->agentControl.refreshAgentStatus (agentId);
	}
	ui->release ();
}

void WebKioskUi::playlistNameClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	WebPlaylistWindow *window;
	TextFieldWindow *action;

	ui = (WebKioskUi *) uiPtr;
	window = (WebPlaylistWindow *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	ui->clearPopupWidgets ();
	action = (TextFieldWindow *) app->rootPanel->addWidget (new TextFieldWindow (window->width - (uiconfig->marginSize * 2.0f), uitext->getText (UiTextString::enterWebPlaylistNamePrompt)));
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (window);
	action->setValue (window->playlistName);
	action->setEditCallback (WebKioskUi::playlistNameEdited, ui);
	action->setFillBg (true, uiconfig->lightPrimaryColor);
	action->setButtonsEnabled (true, true, true, true);
	action->setEditing (true);
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (window->drawX + uiconfig->marginSize, window->drawY);
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

	basename.assign (App::getInstance ()->uiText.getText (UiTextString::websiteList).capitalized ());
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

// Clear the provided string if the widget is of the correct type and matches its content by name
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

void WebKioskUi::playlistMenuClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	WebPlaylistWindow *window;
	Menu *action;
	bool show;

	ui = (WebKioskUi *) uiPtr;
	window = (WebPlaylistWindow *) widgetPtr;
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

	action->addItem (uitext->getText (UiTextString::rename).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::RENAME_BUTTON), WebKioskUi::renamePlaylistActionClicked, ui);
	action->addItem (uitext->getText (UiTextString::remove).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON), WebKioskUi::removePlaylistActionClicked, ui);

	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->position.assign (window->drawX + window->menuPositionX, window->drawY + window->menuPositionY);
}

void WebKioskUi::renamePlaylistActionClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *window;

	ui = (WebKioskUi *) uiPtr;
	window = WebPlaylistWindow::castWidget (ui->actionTarget.widget);
	if (! window) {
		return;
	}

	WebKioskUi::playlistNameClicked (ui, window);
}

void WebKioskUi::removePlaylistActionClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebPlaylistWindow *target;

	ui = (WebKioskUi *) uiPtr;
	target = WebPlaylistWindow::castWidget (ui->actionTarget.widget);
	if (target) {
		ui->cardView->removeItem (target->itemId);
	}
	ui->clearPopupWidgets ();
}

void WebKioskUi::clearDisplayButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	HashMap::Iterator i;
	StdString id;
	int result, count;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
	if (ui->selectedAgentMap.empty ()) {
		return;
	}

	count = 0;
	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);
		result = app->agentControl.invokeCommand (id, app->createCommand ("ClearDisplay", SystemInterface::Constant_Monitor, NULL), NULL, NULL, NULL, id);
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

void WebKioskUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	Button *button;
	App *app;
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
	if (button == ui->writePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::runProgram).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (app->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			color.assign (uiconfig->errorTextColor);
			text.assign (uitext->getText (UiTextString::webKioskNoAgentsSelectedPrompt));
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_DISPLAY_ICON), text, UiConfiguration::CAPTION, color));
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
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_INTENT_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearDisplayButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::clear).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (app->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskNoAgentsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_DISPLAY_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->addUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::addWebsite).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		item = WebPlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
		if (! item) {
			text.assign (uitext->getText (UiTextString::webKioskNoPlaylistsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (item->playlistName);
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_INTENT_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskNoAddressEnteredPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			Label::truncateText (&text, UiConfiguration::CAPTION, app->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::WEB_LINK_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->showUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::showWebsite).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (app->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskNoAgentsSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_DISPLAY_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskNoAddressEnteredPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			Label::truncateText (&text, UiConfiguration::CAPTION, app->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::WEB_LINK_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->browseUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::browseWebsite).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::webKioskNoAddressEnteredPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			Label::truncateText (&text, UiConfiguration::CAPTION, app->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::WEB_LINK_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->addPlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::createProgram).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));
	}

	if (label) {
		label->setPadding (0.0f, 0.0f);
	}
	panel->setLayout (Panel::VERTICAL_RIGHT_JUSTIFIED);
	panel->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	panel->position.assign (app->windowWidth - panel->width - uiconfig->paddingSize, app->windowHeight - app->bottomBarHeight - panel->height - uiconfig->marginSize);
}

void WebKioskUi::commandButtonMouseExited (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	Button *button;

	ui = (WebKioskUi *) uiPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);

	if (ui->commandButton.widget == button) {
		ui->commandPopup.destroyAndClear ();
		ui->commandButton.clear ();
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
	Label::truncateText (&text, UiConfiguration::CAPTION, maxWidth, StdString::createSprintf ("... (%i)", count));

	return (text);
}

void WebKioskUi::agentExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->cardView->refresh ();
}
