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
#include "TextFieldWindow.h"
#include "ActionWindow.h"
#include "HelpWindow.h"
#include "CardView.h"
#include "IconLabelWindow.h"
#include "WebKioskWindow.h"
#include "WebDisplayIntentWindow.h"
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
, addIntentButton (NULL)
, writeIntentButton (NULL)
, clearDisplayButton (NULL)
{

}

WebKioskUi::~WebKioskUi () {

}

StdString WebKioskUi::getSpritePath () {
	return (StdString ("ui/WebKioskUi/sprite"));
}

StdString WebKioskUi::getBreadcrumbTitle () {
	return (App::getInstance ()->uiText.webKiosk.capitalized ());
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

	help->setHelpText (uitext->webKioskUiHelpTitle, uitext->webKioskUiHelpText);
	if (agentCount <= 0) {
		help->addAction (uitext->webKioskUiHelpAction1Text);
	}
	else {
		help->addAction (uitext->webKioskUiHelpAction2Text);
	}

	help->addTopicLink (uitext->membraneSoftwareOverview, Util::getHelpUrl ("membrane-software-overview"));
	help->addTopicLink (uitext->searchForHelp, Util::getHelpUrl (""));
}

WebDisplayIntentWindow *WebKioskUi::createIntentWindow () {
	WebDisplayIntentWindow *window;

	window = new WebDisplayIntentWindow (&sprites);
	window->setSelectStateChangeCallback (WebKioskUi::intentSelectStateChanged, this);
	window->setNameClickCallback (WebKioskUi::intentNameClicked, this);
	window->setMenuClickCallback (WebKioskUi::intentMenuClicked, this);
	window->setUrlListChangeCallback (WebKioskUi::intentUrlListChanged, this);

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
	cardView->setRowHeader (0, uitext->webKiosks.capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->setRowHeader (1, uitext->programs.capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
// TODO: Add a card sort function
//	cardView->sort (WebKioskUi::sortAgentCards);
	cardView->position.assign (0.0f, app->topBarHeight);

	return (Result::SUCCESS);
}

void WebKioskUi::doUnload () {
	selectedAgentMap.clear ();
	selectedIntentId.assign ("");
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
	button->setMouseClickCallback (WebKioskUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.reloadTooltip);
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

	addressField = new TextFieldWindow (app->windowWidth * 0.5f, uitext->enterUrlPrompt, uiconfig->coreSprites.getSprite (UiConfiguration::WEB_LINK_ICON));
	addressField->setPadding (uiconfig->paddingSize, 0.0f);
	addressField->setButtonsEnabled (false, false, true, true);
	toolbar->addLeftItem (addressField);

	addUrlButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::ADD_URL_BUTTON));
	addUrlButton->shortcutKey = SDLK_F1;
	addUrlButton->setMouseClickCallback (WebKioskUi::addUrlButtonClicked, this);
	addUrlButton->setMouseHoverTooltip (app->uiText.webKioskUiAddUrlTooltip);
	addUrlButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	addUrlButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addLeftItem (addUrlButton);

	browseUrlButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::BROWSE_URL_BUTTON));
	browseUrlButton->shortcutKey = SDLK_F2;
	browseUrlButton->setMouseClickCallback (WebKioskUi::browseUrlButtonClicked, this);
	browseUrlButton->setMouseHoverTooltip (app->uiText.webKioskUiBrowseUrlTooltip);
	browseUrlButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	browseUrlButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addLeftItem (browseUrlButton);

	showUrlButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::SHOW_URL_BUTTON));
	showUrlButton->shortcutKey = SDLK_F3;
	showUrlButton->setMouseClickCallback (WebKioskUi::showUrlButtonClicked, this);
	showUrlButton->setMouseHoverTooltip (app->uiText.webKioskUiShowUrlTooltip);
	showUrlButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	showUrlButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addLeftItem (showUrlButton);

	addIntentButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::ADD_INTENT_BUTTON));
	addIntentButton->shortcutKey = SDLK_F6;
	addIntentButton->setMouseClickCallback (WebKioskUi::addIntentButtonClicked, this);
	addIntentButton->setMouseHoverTooltip (app->uiText.webKioskUiAddIntentTooltip, Widget::LEFT);
	toolbar->addRightItem (addIntentButton);

	writeIntentButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::WRITE_INTENT_BUTTON));
	writeIntentButton->shortcutKey = SDLK_F5;
	writeIntentButton->setMouseClickCallback (WebKioskUi::writeIntentButtonClicked, this);
	writeIntentButton->setMouseHoverTooltip (app->uiText.webKioskUiWriteIntentTooltip, Widget::LEFT);
	writeIntentButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	writeIntentButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addRightItem (writeIntentButton);

	clearDisplayButton = new Button (StdString (""), sprites.getSprite (WebKioskUi::CLEAR_DISPLAY_BUTTON));
	clearDisplayButton->shortcutKey = SDLK_F4;
	clearDisplayButton->setMouseClickCallback (WebKioskUi::clearDisplayButtonClicked, this);
	clearDisplayButton->setMouseHoverTooltip (app->uiText.webKioskUiClearDisplayTooltip, Widget::LEFT);
	clearDisplayButton->setMouseEnterCallback (WebKioskUi::commandButtonMouseEntered, this);
	clearDisplayButton->setMouseExitCallback (WebKioskUi::commandButtonMouseExited, this);
	toolbar->addRightItem (clearDisplayButton);

	toolbar->isVisible = true;
}

void WebKioskUi::doClearPopupWidgets () {
	actionWidget.destroyAndClear ();
	actionTarget.clear ();
	commandPopup.destroyAndClear ();
}

void WebKioskUi::doResume () {
	App *app;
	Json *params, *obj;
	WebDisplayIntentWindow *window;
	StringList items;
	StringList::iterator i, end;
	StdString id;
	int count;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/WebKioskUi/bg");

	params = new Json ();
	params->set ("commandId", SystemInterface::Command_AgentStatus);
	app->agentControl.writeLinkCommand (app->createCommandJson ("ReadEvents", SystemInterface::Constant_Link, params));

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);

	if (! isFirstResumeComplete) {
		app->prefsMap.find (App::prefsWebKioskUiIntents, &items);
		count = 0;
		i = items.begin ();
		end = items.end ();
		while (i != end) {
			obj = new Json ();
			if (obj->parse (*i) == Result::SUCCESS) {
				window = createIntentWindow ();
				window->setState (obj);

				// TODO: Remove this operation (after window expand functionality is implemented)
				window->setExpanded (true);

				id = cardView->getAvailableItemId ();
				if (selectedIntentId.empty ()) {
					if (window->isSelected) {
						selectedIntentId.assign (id);
					}
				}
				else {
					if (window->isSelected) {
						window->setSelected (false, true);
					}
				}
				window->itemId.assign (id);
				cardView->addItem (window, id, 1);
			}
			delete (obj);
			++count;
			++i;
		}

		if (count <= 0) {
			WebKioskUi::addIntentButtonClicked (this, NULL);
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
	cardView->processItems (WebKioskUi::appendIntentJson, &items);
	if (items.empty ()) {
		app->prefsMap.remove (App::prefsWebKioskUiIntents);
	}
	else {
		app->prefsMap.insert (App::prefsWebKioskUiIntents, &items);
	}
}

void WebKioskUi::appendIntentJson (void *stringListPtr, Widget *widgetPtr) {
	WebDisplayIntentWindow *window;
	Json *json;

	window = WebDisplayIntentWindow::castWidget (widgetPtr);
	if (window) {
		json = window->getState ();
		((StringList *) stringListPtr)->push_back (json->toString ());
		delete (json);
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

	store->processRecords (SystemInterface::Command_AgentStatus, WebKioskUi::processAgentStatus, this);
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
	SystemInterface *interface;
	WebKioskWindow *window;
	StdString name;

	ui = (WebKioskUi *) uiPtr;
	interface = &(App::getInstance ()->systemInterface);

	if (! ui->cardView->contains (recordId)) {
		name = interface->getCommandStringParam (record, "applicationName", "");
		if (name.equals (WebKioskUi::serverApplicationName)) {
			if (interface->getCommandObjectParam (record, "monitorServerStatus", NULL)) {
				window = new WebKioskWindow (recordId, &(ui->sprites));
				window->setSelectStateChangeCallback (WebKioskUi::agentSelectStateChanged, ui);
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
	WebKioskUi *ui;
	App *app;
	WebKioskWindow *window;
	int result;

	ui = (WebKioskUi *) uiPtr;
	window = WebKioskWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	app = App::getInstance ();
	ui->retain ();
	result = app->agentControl.invokeCommand (window->agentId, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType, NULL), WebKioskUi::invokeGetStatusComplete, ui);
	if (result != Result::SUCCESS) {
		// TODO: Show an error message here
		Log::write (Log::ERR, "Failed to execute GetStatus command; err=%i agentId=\"%s\"", result, window->agentId.c_str ());
		ui->release ();
		return;
	}
}

void WebKioskUi::invokeGetStatusComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	WebKioskUi *ui;
	App *app;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();

	if (responseCommand) {
		app->agentControl.storeAgentStatus (responseCommand);
		app->shouldSyncRecordStore = true;
	}

	ui->release ();
}

void WebKioskUi::agentSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebKioskWindow *window;

	ui = (WebKioskUi *) uiPtr;
	window = (WebKioskWindow *) widgetPtr;
	if (window->isSelected) {
		ui->selectedAgentMap.insert (window->agentId, window->agentName);
	}
	else {
		ui->selectedAgentMap.remove (window->agentId);
	}
}

void WebKioskUi::intentSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebDisplayIntentWindow *window, *item;

	ui = (WebKioskUi *) uiPtr;
	window = (WebDisplayIntentWindow *) widgetPtr;
	if (window->isSelected) {
		if (! ui->selectedIntentId.equals (window->itemId)) {
			if (! ui->selectedIntentId.empty ()) {
				item = WebDisplayIntentWindow::castWidget (ui->cardView->getItem (ui->selectedIntentId));
				if (item) {
					item->setSelected (false, true);
				}
			}
			ui->selectedIntentId.assign (window->itemId);
		}
	}
	else {
		ui->selectedIntentId.assign ("");
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

	url = Util::getProtocolString (url);
	ui->cardView->processItems (WebKioskUi::addIntentUrl, &url);
}

void WebKioskUi::addIntentUrl (void *urlStringPtr, Widget *widgetPtr) {
	WebDisplayIntentWindow *window;
	StdString url;

	window = WebDisplayIntentWindow::castWidget (widgetPtr);
	if (window && window->isSelected) {
		url.assign (((StdString *) urlStringPtr)->c_str ());
		window->addUrl (url);
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
		app->showSnackbar (app->uiText.launchWebBrowserError);
	}
	else {
		app->showSnackbar (StdString::createSprintf ("%s - %s", app->uiText.launchedWebBrowser.capitalized ().c_str (), url.c_str ()));
	}
}

void WebKioskUi::showUrlButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	StdString url, id;
	HashMap::Iterator i;
	Json *params;
	int result;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
	url = ui->addressField->getValue ();
	if (url.empty ()) {
		return;
	}

	if (ui->selectedAgentMap.empty ()) {
		return;
	}

	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);

		result = app->agentControl.invokeCommand (id, app->createCommand ("ClearDisplay", SystemInterface::Constant_Display), NULL, NULL, NULL, StdString ("WebKioskUi::showUrlButtonClicked"));
		if (result != Result::SUCCESS) {
			// TODO: Show an error message here
			Log::write (Log::ERR, "Failed to execute ClearDisplay command; err=%i agentId=\"%s\"", result, id.c_str ());
			continue;
		}

		params = new Json ();
		params->set ("url", Util::getProtocolString (url));
		result = app->agentControl.invokeCommand (id, app->createCommand ("ShowWebUrl", SystemInterface::Constant_Display, params), NULL, NULL, NULL, StdString ("WebKioskUi::showUrlButtonClicked"));
		if (result != Result::SUCCESS) {
			// TODO: Show an error message here
			Log::write (Log::ERR, "Failed to execute ShowWebUrl command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
	}
}

void WebKioskUi::addIntentButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebDisplayIntentWindow *window;
	StdString name, id;

	ui = (WebKioskUi *) uiPtr;

	name = ui->getAvailableIntentName ();
	id = ui->cardView->getAvailableItemId ();
	window = ui->createIntentWindow ();
	window->itemId.assign (id);
	window->setIntentName (name);
	window->setExpanded (true);
	ui->cardView->addItem (window, id, 1);
	window->setSelected (true);
	ui->cardView->scrollToItem (id);
}

void WebKioskUi::writeIntentButtonClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	Json *cmd;
	WebDisplayIntentWindow *item;
	HashMap::Iterator i;
	StdString id;
	int result;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
	if (ui->selectedAgentMap.empty () || ui->selectedIntentId.empty ()) {
		return;
	}

	item = WebDisplayIntentWindow::castWidget (ui->cardView->getItem (ui->selectedIntentId));
	if (! item) {
		return;
	}

	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);
		cmd = item->getCreateCommand ();
		if (cmd) {
			ui->retain ();
			result = app->agentControl.invokeCommand (id, cmd, WebKioskUi::writeIntentComplete, ui);
			if (result != Result::SUCCESS) {
				// TODO: Show an error message here
				Log::write (Log::ERR, "Failed to create intent; err=%i agentId=\"%s\"", result, id.c_str ());
				ui->release ();
			}
		}
	}
}

void WebKioskUi::writeIntentComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	WebKioskUi *ui;
	App *app;
	SystemInterface *interface;
	int result;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
	interface = &(app->systemInterface);

	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		result = app->agentControl.invokeCommand (agentId, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType, NULL), WebKioskUi::invokeGetStatusComplete, ui);
		if (result != Result::SUCCESS) {
			// TODO: Show an error message here
			Log::write (Log::ERR, "Failed to execute GetStatus command; err=%i", result);
			ui->release ();
		}
	}
	else {
		ui->release ();
	}
}

void WebKioskUi::intentNameClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	WebDisplayIntentWindow *window;
	TextFieldWindow *action;

	ui = (WebKioskUi *) uiPtr;
	window = (WebDisplayIntentWindow *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	ui->clearPopupWidgets ();
	action = (TextFieldWindow *) app->rootPanel->addWidget (new TextFieldWindow (window->width - (uiconfig->marginSize * 2.0f), uitext->enterIntentNamePrompt));
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (window);
	action->setValue (window->intentName);
	action->setEditCallback (WebKioskUi::intentNameEdited, ui);
	action->setFillBg (true, uiconfig->lightPrimaryColor);
	action->setButtonsEnabled (true, true, true, true);
	action->setEditing (true);
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (window->drawX + uiconfig->marginSize, window->drawY);
}

void WebKioskUi::intentNameEdited (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebDisplayIntentWindow *target;
	TextFieldWindow *action;
	StdString name, id;

	ui = (WebKioskUi *) uiPtr;
	action = (TextFieldWindow *) ui->actionWidget.widget;
	target = (WebDisplayIntentWindow *) ui->actionTarget.widget;
	if ((! action) || (! target)) {
		return;
	}

	name = action->getValue ();
	if (name.empty ()) {
		name.assign (ui->getAvailableIntentName ());
	}
	target->setIntentName (name);
	ui->clearPopupWidgets ();
}

StdString WebKioskUi::getAvailableIntentName () {
	StdString basename, name, id;
	int i;

	basename.assign (App::getInstance ()->uiText.websiteList.capitalized ());
	name.assign (basename);
	cardView->processItems (WebKioskUi::matchIntentName, &name);
	if (name.empty ()) {
		i = 2;
		while (true) {
			name.sprintf ("%s %i", basename.c_str (), i);
			cardView->processItems (WebKioskUi::matchIntentName, &name);
			if (! name.empty ()) {
				break;
			}
			++i;
		}
	}

	return (name);
}

// Clear the provided string if the widget is of the correct type and matches its content by name
void WebKioskUi::matchIntentName (void *stringPtr, Widget *widgetPtr) {
	WebDisplayIntentWindow *window;
	StdString *name;

	window = WebDisplayIntentWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	name = (StdString *) stringPtr;
	if (name->lowercased ().equals (window->intentName.lowercased ())) {
		name->assign ("");
	}
}

void WebKioskUi::intentUrlListChanged (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;

	ui = (WebKioskUi *) uiPtr;
	ui->refresh ();
}

void WebKioskUi::intentMenuClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	WebDisplayIntentWindow *window;
	Menu *action;
	bool show;

	ui = (WebKioskUi *) uiPtr;
	window = (WebDisplayIntentWindow *) widgetPtr;
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

	action->addItem (uitext->rename.capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::RENAME_BUTTON), WebKioskUi::renameIntentActionClicked, ui);
	action->addItem (uitext->remove.capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON), WebKioskUi::removeIntentActionClicked, ui);

	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->position.assign (window->drawX + window->width - action->width - uiconfig->marginSize, window->drawY + window->menuPositionY);
}

void WebKioskUi::renameIntentActionClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebDisplayIntentWindow *window;

	ui = (WebKioskUi *) uiPtr;
	window = WebDisplayIntentWindow::castWidget (ui->actionTarget.widget);
	if (! window) {
		return;
	}

	WebKioskUi::intentNameClicked (ui, window);
}

void WebKioskUi::removeIntentActionClicked (void *uiPtr, Widget *widgetPtr) {
	WebKioskUi *ui;
	WebDisplayIntentWindow *target;

	ui = (WebKioskUi *) uiPtr;
	target = WebDisplayIntentWindow::castWidget (ui->actionTarget.widget);
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
	int result;

	ui = (WebKioskUi *) uiPtr;
	app = App::getInstance ();
	if (ui->selectedAgentMap.empty ()) {
		return;
	}

	i = ui->selectedAgentMap.begin ();
	while (ui->selectedAgentMap.hasNext (&i)) {
		id = ui->selectedAgentMap.next (&i);
		result = app->agentControl.invokeCommand (id, app->createCommand ("ClearDisplay", SystemInterface::Constant_Display, NULL));
		if (result != Result::SUCCESS) {
			// TODO: Show an error message here
			Log::write (Log::ERR, "Failed to create ClearDisplay command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
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
	WebDisplayIntentWindow *item;
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
	if (button == ui->writeIntentButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->writeProgram.capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (app->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			color.assign (uiconfig->errorTextColor);
			text.assign (uitext->webKioskNoAgentsSelectedPrompt);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_DISPLAY_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		item = WebDisplayIntentWindow::castWidget (ui->cardView->getItem (ui->selectedIntentId));
		if (! item) {
			text.assign (uitext->webKioskNoIntentsSelectedPrompt);
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (item->intentName);
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_INTENT_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearDisplayButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->clear.capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (app->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->webKioskNoAgentsSelectedPrompt);
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_DISPLAY_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->addUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->addWebsite.capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		item = WebDisplayIntentWindow::castWidget (ui->cardView->getItem (ui->selectedIntentId));
		if (! item) {
			text.assign (uitext->webKioskNoIntentsSelectedPrompt);
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (item->intentName);
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_INTENT_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->showUrlButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->showWebsite.capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedAgentNames (app->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->webKioskNoAgentsSelectedPrompt);
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (WebKioskUi::SMALL_DISPLAY_ICON), text, UiConfiguration::CAPTION, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->webKioskNoAddressEnteredPrompt);
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
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->browseWebsite.capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor)));

		text.assign (ui->addressField->getValue ());
		if (text.empty ()) {
			text.assign (uitext->webKioskNoAddressEnteredPrompt);
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

	if (label) {
		label->setPadding (0.0f, 0.0f);
	}
	panel->layout = Panel::VERTICAL_RIGHT_JUSTIFIED;
	panel->refresh ();
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
