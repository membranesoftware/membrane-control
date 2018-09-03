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
#include "TextFieldWindow.h"
#include "ActionWindow.h"
#include "HelpWindow.h"
#include "CardView.h"
#include "AgentConfigurationWindow.h"
#include "AgentStatusWindow.h"
#include "LinkUi.h"

LinkUi::LinkUi ()
: Ui ()
, cardView (NULL)
, linkServerCount (0)
, agentCount (0)
{

}

LinkUi::~LinkUi () {

}

StdString LinkUi::getSpritePath () {
	return (StdString ("ui/LinkUi/sprite"));
}

StdString LinkUi::getBreadcrumbTitle () {
	return (App::getInstance ()->uiText.servers.capitalized ());
}

Sprite *LinkUi::getBreadcrumbSprite () {
	return (sprites.getSprite (LinkUi::BREADCRUMB_ICON)->copy ());
}

void LinkUi::setHelpWindowContent (Widget *helpWindowPtr) {
	App *app;
	HelpWindow *help;
	UiText *uitext;

	help = (HelpWindow *) helpWindowPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);

	help->setHelpText (uitext->linkUiHelpTitle, uitext->linkUiHelpText);
	if (agentCount <= 0) {
		help->addAction (uitext->linkUiHelpAction1Text, uitext->learnMore.capitalized (), Util::getHelpUrl ("servers"));
	}
	else {
		help->addAction (uitext->linkUiHelpAction2Text, uitext->learnMore.capitalized (), Util::getHelpUrl ("servers"));
	}

	help->addTopicLink (uitext->membraneSoftwareOverview, Util::getHelpUrl ("membrane-software-overview"));
	help->addTopicLink (uitext->searchForHelp, Util::getHelpUrl (""));
}

AgentConfigurationWindow *LinkUi::createAgentConfigurationWindow (const StdString &serverHostname) {
	AgentConfigurationWindow *window;

	window = new AgentConfigurationWindow (serverHostname, &sprites);
	window->setConfigureCallback (LinkUi::agentConfigureActionClicked, this);
	window->setConfigureSuccessCallback (LinkUi::agentConfigureActionSucceeded, this);

	return (window);
}

int LinkUi::doLoad () {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StringList addresses;
	StringList::iterator i, end;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (0, uitext->linkUiNetworkAgentsTitle, UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->setRowHeader (1, uitext->linkUiAgentConfigurationTitle, UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->sort (LinkUi::sortAgentCards);
	cardView->position.assign (0.0f, app->topBarHeight);

	app->prefsMap.find (App::prefsLinkAddresses, &addresses);
	if (! addresses.empty ()) {
		i = addresses.begin ();
		end = addresses.end ();
		while (i != end) {
			invokeGetStatus (*i);
			++i;
		}
	}

	return (Result::SUCCESS);
}

bool LinkUi::sortAgentCards (Widget *a, Widget *b) {
	StdString *namea, *nameb;

	namea = (StdString *) a->extraData;
	nameb = (StdString *) b->extraData;
	if (!(namea && nameb)) {
		return (false);
	}

	if (namea->lowercased ().compare (nameb->lowercased ()) < 0) {
		return (true);
	}

	return (false);
}

void LinkUi::doUnload () {
	agentGetStatusMap.clear ();
	addressToggle.clear ();
	addressTextFieldWindow.clear ();
	actionWindow.clear ();
	localAgentWindow.clear ();
	emptyAgentStatusWindow.clear ();
}

void LinkUi::doResetMainToolbar (Toolbar *toolbar) {
	App *app;
	Button *button;

	app = App::getInstance ();
	button = new Button (StdString (""), app->uiConfig.coreSprites.getSprite (UiConfiguration::RELOAD_BUTTON));
	button->setMouseClickCallback (LinkUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.reloadTooltip);
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void LinkUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	UiConfiguration *uiconfig;
	Button *button;
	Toggle *toggle;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);

	button = new Button (StdString (""), sprites.getSprite (LinkUi::BROADCAST_BUTTON));
	button->shortcutKey = SDLK_F2;
	button->setMouseClickCallback (LinkUi::broadcastButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.linkUiBroadcastTooltip);
	toolbar->addRightItem (button);

	toggle = new Toggle (sprites.getSprite (LinkUi::ADDRESS_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::CANCEL_BUTTON));
	toggle->shortcutKey = SDLK_F1;
	toggle->setStateChangeCallback (LinkUi::addressToggleStateChanged, this);
	toggle->setMouseHoverTooltip (app->uiText.linkUiAddressTooltip);
	toolbar->addRightItem (toggle);

	addressToggle.destroyAndClear ();
	addressToggle.assign (toggle);

	toolbar->isVisible = true;
}

void LinkUi::doClearPopupWidgets () {
	actionWindow.destroyAndClear ();
}

void LinkUi::doResume () {
	App *app;
	Json *params;
	AgentConfigurationWindow *agentwindow;
	HashMap prefs;
	int result;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/LinkUi/bg");

	params = new Json ();
	params->set ("commandId", SystemInterface::Command_AgentStatus);
	app->agentControl.writeLinkCommand (app->createCommandJson ("ReadEvents", SystemInterface::Constant_Link, params));

	agentwindow = (AgentConfigurationWindow *) localAgentWindow.widget;
	if (! agentwindow) {
		agentwindow = createAgentConfigurationWindow (AgentControl::localHostname);
		localAgentWindow.assign (agentwindow);
	}
	result = AgentControl::readSystemAgentConfiguration (&prefs, &localAgentId);
	if (result != Result::SUCCESS) {
		if (! cardView->contains ("localAgentWindow")) {
			cardView->addItem (agentwindow, "localAgentWindow", 1);
		}
		agentwindow->setUninstalled ();
	}
	else {
		if (localAgentId.empty ()) {
			if (! cardView->contains ("localAgentWindow")) {
				cardView->addItem (agentwindow, "localAgentWindow", 1);
			}
			agentwindow->setStopped ();
		}
		else {
			if (! cardView->contains (localAgentId)) {
				cardView->addItem (agentwindow, localAgentId, 1);
			}
			agentwindow->setSystemAgentConfiguration (&prefs, localAgentId);
			agentwindow->invokeGetStatus ();
			agentwindow->invokeGetAgentConfiguration ();
		}
	}

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void LinkUi::doRefresh () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void LinkUi::doPause () {
	App *app;
	SystemInterface *interface;
	RecordStore *store;
	Json *record;
	StringList ids, addresses;
	StringList::iterator i, end;
	StdString hostname;
	int port;

	app = App::getInstance ();
	store = &(app->agentControl.recordStore);
	interface = &(app->systemInterface);
	agentGetStatusMap.getKeys (&ids, true);

	if (ids.empty ()) {
		app->prefsMap.remove (App::prefsLinkAddresses);
	}
	else {
		i = ids.begin ();
		end = ids.end ();
		store->lock ();
		while (i != end) {
			record = store->findRecord (*i, SystemInterface::Command_AgentStatus);
			if (record) {
				hostname = interface->getCommandStringParam (record, "urlHostname", "");
				port = interface->getCommandNumberParam (record, "tcpPort", SystemInterface::Constant_DefaultTcpPort);
				if (! hostname.empty ()) {
					addresses.push_back (StdString::createSprintf ("%s:%i", hostname.c_str (), port));
				}
			}
			++i;
		}
		store->unlock ();
		app->prefsMap.insert (App::prefsLinkAddresses, &addresses);
	}
}

void LinkUi::doUpdate (int msElapsed) {
	addressToggle.compact ();
	if (addressToggle.widget) {
		if (addressTextFieldWindow.widget && addressTextFieldWindow.widget->isDestroyed) {
			((Toggle *) addressToggle.widget)->setChecked (false);
			addressTextFieldWindow.clear ();
		}
	}

	actionWindow.compact ();
	localAgentWindow.compact ();
	emptyAgentStatusWindow.compact ();
}

void LinkUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void LinkUi::doSyncRecordStore (RecordStore *store) {
	App *app;
	AgentStatusWindow *window;

	app = App::getInstance ();

	linkServerCount = store->countAgentRecords ("linkServerStatus", app->prefsMap.find (App::prefsServerTimeout, App::defaultServerTimeout) * 1000);
	agentCount = 0;
	store->processRecords (SystemInterface::Command_AgentStatus, LinkUi::processAgentStatus, this);

	window = (AgentStatusWindow *) emptyAgentStatusWindow.widget;
	if (agentCount > 0) {
		if (window) {
			cardView->removeItem (window->itemId);
			emptyAgentStatusWindow.clear ();
		}
	}
	else {
		if (! window) {
			window = new AgentStatusWindow (StdString (""), &sprites);
			emptyAgentStatusWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, 0);
		}
	}

	cardView->syncRecordStore (store);
	cardView->refresh ();
}

void LinkUi::processAgentStatus (void *uiPtr, Json *record, const StdString &recordId) {
	LinkUi *ui;
	SystemInterface *interface;
	AgentStatusWindow *statuswindow;
	AgentConfigurationWindow *configwindow;
	StdString cardid;

	ui = (LinkUi *) uiPtr;
	interface = &(App::getInstance ()->systemInterface);
	++(ui->agentCount);

	cardid.sprintf ("%s-AgentStatusWindow", recordId.c_str ());
	if (! ui->cardView->contains (cardid)) {
		statuswindow = new AgentStatusWindow (recordId, &(ui->sprites));
		statuswindow->setConnectCallback (LinkUi::agentConnectButtonClicked, ui);
		statuswindow->setDisconnectCallback (LinkUi::agentDisconnectButtonClicked, ui);
		ui->cardView->addItem (statuswindow, cardid, 0);
	}

	if (! recordId.equals (ui->localAgentId)) {
		cardid.sprintf ("%s-AgentConfigurationWindow", recordId.c_str ());
		if (! ui->cardView->contains (cardid)) {
			configwindow = ui->createAgentConfigurationWindow (interface->getCommandStringParam (record, "urlHostname", ""));
			configwindow->setAgentStatus (record);
			ui->cardView->addItem (configwindow, cardid, 1);

			configwindow->invokeGetAgentConfiguration ();
		}
	}
}

void LinkUi::handleLinkClientConnect (const StdString &agentId) {
	StdString cardid;
	AgentStatusWindow *window;

	cardid.sprintf ("%s-AgentStatusWindow", agentId.c_str ());
	window = AgentStatusWindow::castWidget (cardView->getItem (cardid));
	if (! window) {
		return;
	}

	window->setConnected ();
}

void LinkUi::handleLinkClientDisconnect (const StdString &agentId, const StdString &errorDescription) {
	StdString cardid;
	AgentStatusWindow *window;

	cardid.sprintf ("%s-AgentStatusWindow", agentId.c_str ());
	window = AgentStatusWindow::castWidget (cardView->getItem (cardid));
	if (! window) {
		return;
	}

	window->setDisconnected ();
}

void LinkUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	LinkUi *ui;

	ui = (LinkUi *) uiPtr;
	ui->cardView->processItems (LinkUi::reloadAgent, ui);
}

void LinkUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	LinkUi *ui;
	AgentStatusWindow *window;
	App *app;
	int result;

	ui = (LinkUi *) uiPtr;
	window = AgentStatusWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	app = App::getInstance ();
	ui->retain ();
	result = app->agentControl.invokeCommand (window->agentId, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType, NULL), LinkUi::invokeGetStatusComplete, ui);
	if (result != Result::SUCCESS) {
		Log::write (Log::ERR, "Failed to execute GetStatus command; err=%i agentId=\"%s\"", result, window->agentId.c_str ());
		ui->release ();
		return;
	}
}

void LinkUi::broadcastButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;

	app = App::getInstance ();
	app->agentControl.broadcastContactMessage ();
	app->showSnackbar (app->uiText.sentBroadcast);
}

void LinkUi::addressToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	LinkUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	Toolbar *toolbar;
	Toggle *toggle;
	TextFieldWindow *textfield;

	ui = (LinkUi *) uiPtr;
	toggle = (Toggle *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	toolbar = app->secondaryToolbar;

	if (toggle->isChecked) {
		textfield = new TextFieldWindow (toolbar->getLeftWidth (), app->uiText.enterAddressPrompt, ui->sprites.getSprite (LinkUi::ADDRESS_ICON));
		textfield->setWindowHeight (toolbar->height);
		textfield->setButtonsEnabled (true, false, false, false);
		textfield->setFillBg (true, uiconfig->lightPrimaryColor);
		textfield->setEditCallback (LinkUi::addressTextFieldEdited, ui);
		textfield->setEditing (true);
		toggle->setFillBg (true, uiconfig->mediumPrimaryColor);

		toolbar->setLeftOverlay (textfield);
		ui->addressTextFieldWindow.assign (textfield);
	}
	else {
		ui->addressTextFieldWindow.destroyAndClear ();
		toggle->setFillBg (false);
	}
}

void LinkUi::addressTextFieldEdited (void *uiPtr, Widget *widgetPtr) {
	App *app;
	LinkUi *ui;
	TextFieldWindow *textfield;
	StdString address;

	ui = (LinkUi *) uiPtr;
	app = App::getInstance ();
	textfield = (TextFieldWindow *) widgetPtr;
	address.assign (textfield->getValue ());
	if (address.empty ()) {
		app->showSnackbar (app->uiText.invokeGetStatusAddressEmptyError, app->uiText.help.uppercased (), LinkUi::addressSnackbarHelpClicked, ui);
		return;
	}

	ui->invokeGetStatus (address, true);
}

void LinkUi::addressSnackbarHelpClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	StdString url;
	int result;

	app = App::getInstance ();
	url.assign (Util::getHelpUrl ("servers"));
	result = Util::openUrl (url);
	if (result != Result::SUCCESS) {
		app->showSnackbar (app->uiText.openHelpUrlError);
	}
	else {
		app->showSnackbar (StdString::createSprintf ("%s - %s", app->uiText.launchedWebBrowser.capitalized ().c_str (), url.c_str ()));
	}
}

void LinkUi::invokeGetStatus (const StdString &address, bool shouldShowSnackbar) {
	App *app;
	StdString hostname;
	int result, port;
	int64_t jobid;

	app = App::getInstance ();
	if (! StdString::parseAddress (address.c_str (), &hostname, &port)) {
		if (shouldShowSnackbar) {
			app->showSnackbar (app->uiText.invokeGetStatusAddressParseError, app->uiText.help.uppercased (), LinkUi::addressSnackbarHelpClicked, this);
		}
		return;
	}
	if (port > 65535) {
		if (shouldShowSnackbar) {
			app->showSnackbar (app->uiText.invokeGetStatusAddressParseError, app->uiText.help.uppercased (), LinkUi::addressSnackbarHelpClicked, this);
		}
		return;
	}
	if (port <= 0) {
		port = SystemInterface::Constant_DefaultTcpPort;
	}

	retain ();
	result = app->agentControl.invokeCommand (hostname, port, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), LinkUi::invokeGetStatusComplete, this, &jobid);
	if (result != Result::SUCCESS) {
		if (shouldShowSnackbar) {
			app->showSnackbar (app->uiText.invokeGetStatusInternalError, app->uiText.help.uppercased (), LinkUi::addressSnackbarHelpClicked, this);
		}
		release ();
		return;
	}

	if (shouldShowSnackbar) {
		app->showSnackbar (app->uiText.invokeGetStatusConfirmation);
	}
}

void LinkUi::invokeGetStatusComplete (void *uiPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	LinkUi *ui;
	App *app;
	StdString agentid;

	ui = (LinkUi *) uiPtr;
	app = App::getInstance ();

	if (responseCommand) {
		agentid = app->systemInterface.getCommandStringParam (responseCommand, "id", "");
		if (! agentid.empty ()) {
			ui->agentGetStatusMap.insert (agentid, true);
		}
		app->agentControl.storeAgentStatus (responseCommand);
		app->shouldSyncRecordStore = true;
	}
	ui->release ();
}

void LinkUi::agentConfigureActionClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	LinkUi *ui;
	AgentConfigurationWindow *window;
	UiConfiguration *uiconfig;
	ActionWindow *action;

	ui = (LinkUi *) uiPtr;
	window = (AgentConfigurationWindow *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	action = window->createConfigureActionWindow ();
	if (! action) {
		return;
	}

	ui->clearPopupWidgets ();
	ui->addWidget (action);
	ui->actionWindow.assign (action);

	action->setCloseCallback (LinkUi::agentConfigureActionClosed, ui);
	action->zLevel = ui->rootPanel->maxWidgetZLevel + 1;
	action->position.assignBounded (window->drawX + window->width - action->width, window->drawY + window->height - action->height, uiconfig->paddingSize, app->topBarHeight + uiconfig->paddingSize, ui->rootPanel->width - uiconfig->paddingSize - action->width, ui->rootPanel->height - uiconfig->paddingSize - action->height);
}

void LinkUi::agentConfigureActionClosed (void *uiPtr, Widget *widgetPtr) {
	LinkUi *ui;
	ActionWindow *action;
	AgentConfigurationWindow *window;

	ui = (LinkUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	window = AgentConfigurationWindow::castWidget (action->sourceWidget.widget);
	if (! window) {
		return;
	}

	if (action->isConfirmed) {
		window->invokeUpdateAgentConfiguration (action);
	}
	ui->actionWindow.clear ();
}

void LinkUi::agentConfigureActionSucceeded (void *uiPtr, Widget *widgetPtr) {
	LinkUi *ui;
	App *app;
	AgentConfigurationWindow *window;
	int result;

	ui = (LinkUi *) uiPtr;
	window = AgentConfigurationWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	app = App::getInstance ();
	app->shouldRefreshUi = true;
	ui->retain ();
	result = app->agentControl.invokeCommand (window->agentId, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), LinkUi::invokeGetStatusComplete, ui);
	if (result != Result::SUCCESS) {
		// TODO: Show an error message here
		Log::write (Log::ERR, "Failed to execute GetStatus command; err=%i agentId=\"%s\"", result, window->agentId.c_str ());
		ui->release ();
		return;
	}
}

void LinkUi::agentConnectButtonClicked (void *uiPtr, Widget *widgetPtr) {
	AgentStatusWindow *window;
	App *app;

	window = (AgentStatusWindow *) widgetPtr;
	app = App::getInstance ();
	if (app->agentControl.isLinkClientConnected (window->agentId)) {
		return;
	}

	app->agentControl.connectLinkClient (window->agentId);
	window->setConnecting ();
}

void LinkUi::agentDisconnectButtonClicked (void *uiPtr, Widget *widgetPtr) {
	AgentStatusWindow *window;
	App *app;

	window = (AgentStatusWindow *) widgetPtr;
	app = App::getInstance ();
	if (! app->agentControl.isLinkClientConnected (window->agentId)) {
		return;
	}

	app->agentControl.disconnectLinkClient (window->agentId);
	window->setDisconnected ();
}
