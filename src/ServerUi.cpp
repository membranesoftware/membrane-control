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
#include "ScrollView.h"
#include "TextFieldWindow.h"
#include "ActionWindow.h"
#include "HelpWindow.h"
#include "IconCardWindow.h"
#include "CardView.h"
#include "ServerWindow.h"
#include "ServerContactWindow.h"
#include "AdminSecretWindow.h"
#include "ServerAdminUi.h"
#include "ServerUi.h"

ServerUi::ServerUi ()
: Ui ()
, cardView (NULL)
, adminSecretWindow (NULL)
, agentCount (0)
{

}

ServerUi::~ServerUi () {

}

StdString ServerUi::getSpritePath () {
	return (StdString ("ui/ServerUi/sprite"));
}

StdString ServerUi::getBreadcrumbTitle () {
	return (App::getInstance ()->uiText.getText (UiTextString::servers).capitalized ());
}

Sprite *ServerUi::getBreadcrumbSprite () {
	return (sprites.getSprite (ServerUi::BREADCRUMB_ICON)->copy ());
}

void ServerUi::setHelpWindowContent (Widget *helpWindowPtr) {
	App *app;
	HelpWindow *help;
	UiText *uitext;

	help = (HelpWindow *) helpWindowPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);

	help->setHelpText (uitext->getText (UiTextString::serverUiHelpTitle), uitext->getText (UiTextString::serverUiHelpText));
	if (agentCount <= 0) {
		help->addAction (uitext->getText (UiTextString::serverUiHelpAction1Text), uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("servers"));
	}
	else {
		help->addAction (uitext->getText (UiTextString::serverUiHelpAction2Text), uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("servers"));
	}

	help->addTopicLink (uitext->getText (UiTextString::serversHelpTitle), Util::getHelpUrl ("servers"));
	help->addTopicLink (uitext->getText (UiTextString::membraneSoftwareOverviewHelpTitle), Util::getHelpUrl ("membrane-software-overview"));
	help->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), Util::getHelpUrl (""));
}

int ServerUi::doLoad () {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (0, uitext->getText (UiTextString::serverUiNetworkAgentsTitle), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, app->topBarHeight);

	adminSecretWindow = new AdminSecretWindow ();
	adminSecretWindow->setAddButtonClickCallback (ServerUi::adminSecretAddButtonClicked, this);
	adminSecretWindow->setExpandStateChangeCallback (ServerUi::adminSecretExpandStateChanged, this);
	adminSecretWindow->setExpanded (false);
	cardView->addItem (adminSecretWindow, StdString (""), 1);

	return (Result::SUCCESS);
}

void ServerUi::doUnload () {
	addressToggle.clear ();
	addressTextFieldWindow.clear ();
	emptyServerWindow.clear ();
}

void ServerUi::doResetMainToolbar (Toolbar *toolbar) {
	App *app;
	Button *button;

	app = App::getInstance ();
	button = new Button (StdString (""), app->uiConfig.coreSprites.getSprite (UiConfiguration::RELOAD_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (ServerUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void ServerUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	UiConfiguration *uiconfig;
	Button *button;
	Toggle *toggle;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);

	button = new Button (StdString (""), sprites.getSprite (ServerUi::BROADCAST_BUTTON));
	button->setInverseColor (true);
	button->shortcutKey = SDLK_F2;
	button->setMouseClickCallback (ServerUi::broadcastButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.getText (UiTextString::serverUiBroadcastTooltip));
	toolbar->addRightItem (button);

	toggle = new Toggle (sprites.getSprite (ServerUi::ADDRESS_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::CANCEL_BUTTON));
	toggle->setInverseColor (true);
	toggle->shortcutKey = SDLK_F1;
	toggle->setStateChangeCallback (ServerUi::addressToggleStateChanged, this);
	toggle->setMouseHoverTooltip (app->uiText.getText (UiTextString::serverUiAddressTooltip));
	toolbar->addRightItem (toggle);

	addressToggle.destroyAndClear ();
	addressToggle.assign (toggle);

	toolbar->isVisible = true;
}

void ServerUi::doClearPopupWidgets () {

}

void ServerUi::doResume () {
	App *app;

	app = App::getInstance ();

	app->setNextBackgroundTexturePath ("ui/ServerUi/bg");
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);

	adminSecretWindow->readItems ();
}

void ServerUi::doRefresh () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void ServerUi::doPause () {

}

void ServerUi::doUpdate (int msElapsed) {
	StringList::iterator i, end;

	addressToggle.compact ();
	if (addressToggle.widget) {
		if (addressTextFieldWindow.widget && addressTextFieldWindow.widget->isDestroyed) {
			((Toggle *) addressToggle.widget)->setChecked (false);
			addressTextFieldWindow.clear ();
		}
	}
	emptyServerWindow.compact ();

	cardIdList.clear ();
	cardView->processItems (ServerUi::findDeletedServerContactWindows, this);
	if (! cardIdList.empty ()) {
		i = cardIdList.begin ();
		end = cardIdList.end ();
		while (i != end) {
			cardView->removeItem (*i);
			++i;
		}
		cardView->refresh ();
		cardIdList.clear ();
	}
}

void ServerUi::findDeletedServerContactWindows (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerContactWindow *window;

	ui = (ServerUi *) uiPtr;
	window = ServerContactWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	if (window->isDeleted) {
		ui->cardIdList.push_back (window->itemId);
	}
}

void ServerUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void ServerUi::doSyncRecordStore (RecordStore *store) {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	IconCardWindow *window;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	agentCount = 0;
	store->processCommandRecords (SystemInterface::Command_AgentStatus, ServerUi::processAgentStatus, this);
	cardView->processItems (ServerUi::countServerContactWindows, this);

	window = (IconCardWindow *) emptyServerWindow.widget;
	if (agentCount > 0) {
		if (window) {
			cardView->removeItem (window->itemId);
			emptyServerWindow.clear ();
		}
	}
	else {
		if (! window) {
			window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::ERROR_ICON), uitext->getText (UiTextString::serverUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::serverUiEmptyAgentStatusText));
			window->setLink (uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("servers"));
			emptyServerWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, 0);
		}
	}

	cardView->syncRecordStore (store);
	cardView->refresh ();
}

void ServerUi::countServerContactWindows (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;

	ui = (ServerUi *) uiPtr;
	if (ServerContactWindow::isWidgetType (widgetPtr)) {
		++(ui->agentCount);
	}
}

void ServerUi::processAgentStatus (void *uiPtr, Json *record, const StdString &recordId) {
	ServerUi *ui;
	SystemInterface *interface;
	ServerWindow *window;

	ui = (ServerUi *) uiPtr;
	interface = &(App::getInstance ()->systemInterface);
	++(ui->agentCount);

	if (! ui->cardView->contains (recordId)) {
		window = new ServerWindow (recordId);
		window->sortKey.assign (interface->getCommandAgentName (record).lowercased ());
		window->setMenuClickCallback (ServerUi::agentMenuClicked, ui);
		ui->cardView->addItem (window, recordId, 0);
	}
}

void ServerUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	App *app;

	ui = (ServerUi *) uiPtr;
	app = App::getInstance ();
	ui->cardView->processItems (ServerUi::reloadAgent, ui);
	app->agentControl.retryAgents ();
}

void ServerUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	ServerWindow *serverwindow;
	ServerContactWindow *servercontactwindow;
	App *app;

	app = App::getInstance ();
	serverwindow = ServerWindow::castWidget (widgetPtr);
	if (serverwindow) {
		app->agentControl.refreshAgentStatus (serverwindow->agentId);
	}
	else {
		servercontactwindow = ServerContactWindow::castWidget (widgetPtr);
		if (servercontactwindow) {
			app->agentControl.contactAgent (servercontactwindow->agentHostname, servercontactwindow->agentPort);
		}
	}
}

void ServerUi::broadcastButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;

	app = App::getInstance ();
	app->agentControl.broadcastContactMessage ();
	app->showSnackbar (app->uiText.getText (UiTextString::sentBroadcast));
}

void ServerUi::addressToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	Toolbar *toolbar;
	Toggle *toggle;
	TextFieldWindow *textfield;

	ui = (ServerUi *) uiPtr;
	toggle = (Toggle *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	toolbar = app->secondaryToolbar;

	if (toggle->isChecked) {
		textfield = new TextFieldWindow (toolbar->getLeftWidth (), app->uiText.getText (UiTextString::enterAddressPrompt), ui->sprites.getSprite (ServerUi::ADDRESS_ICON));
		textfield->setWindowHeight (toolbar->height);
		textfield->setButtonsEnabled (true, false, false, false);
		textfield->setFillBg (true, uiconfig->lightPrimaryColor);
		textfield->setEditCallback (ServerUi::addressTextFieldEdited, ui);
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

void ServerUi::addressTextFieldEdited (void *uiPtr, Widget *widgetPtr) {
	App *app;
	ServerUi *ui;
	TextFieldWindow *textfield;
	ServerContactWindow *window;
	StdString address, hostname, key;
	int port;

	ui = (ServerUi *) uiPtr;
	app = App::getInstance ();
	textfield = (TextFieldWindow *) widgetPtr;
	address.assign (textfield->getValue ());
	if (address.empty ()) {
		app->showSnackbar (app->uiText.getText (UiTextString::invokeGetStatusAddressEmptyError), app->uiText.getText (UiTextString::help).uppercased (), ServerUi::addressSnackbarHelpClicked, ui);
		return;
	}

	if (! StdString::parseAddress (address.c_str (), &hostname, &port, SystemInterface::Constant_DefaultTcpPort1)) {
		app->showSnackbar (app->uiText.getText (UiTextString::invokeGetStatusAddressParseError), app->uiText.getText (UiTextString::help).uppercased (), ServerUi::addressSnackbarHelpClicked, ui);
		return;
	}

	if (app->agentControl.isContacted (hostname, port)) {
		app->agentControl.refreshAgentStatus (hostname, port);
		return;
	}

	app->agentControl.contactAgent (hostname, port);
	app->showSnackbar (StdString::createSprintf ("%s: %s", app->uiText.getText (UiTextString::serverUiContactingAgentMessage).c_str (), address.c_str ()));

	key.sprintf ("%s:%i", hostname.c_str (), port);
	if (! ui->cardView->contains (key)) {
		window = new ServerContactWindow (address, hostname, port);
		window->setStateChangeCallback (ServerUi::serverContactWindowStateChanged, ui);
		window->sortKey.assign (address.lowercased ());
		window->itemId.assign (key);
		ui->cardView->addItem (window, window->itemId, 0);
	}
	app->shouldSyncRecordStore = true;
}

void ServerUi::addressSnackbarHelpClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	StdString url;
	int result;

	app = App::getInstance ();
	url.assign (Util::getHelpUrl ("servers"));
	result = Util::openUrl (url);
	if (result != Result::SUCCESS) {
		app->showSnackbar (app->uiText.getText (UiTextString::openHelpUrlError));
	}
	else {
		app->showSnackbar (StdString::createSprintf ("%s - %s", app->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void ServerUi::agentMenuClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ServerWindow *target;
	Menu *action;
	bool show;

	ui = (ServerUi *) uiPtr;
	target = (ServerWindow *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == target)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = (Menu *) app->rootPanel->addWidget (new Menu ());
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (target);

	action->addItem (uitext->getText (UiTextString::adminConsole).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::AGENT_ADMIN_BUTTON), ServerUi::agentAdminActionClicked, ui);
	if (! target->applicationId.empty ()) {
		action->addItem (uitext->getText (UiTextString::checkForUpdates).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::UPDATE_BUTTON), ServerUi::agentUpdateActionClicked, ui);
	}

	action->addItem (uitext->getText (UiTextString::remove).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON), ServerUi::agentRemoveActionClicked, ui);

	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->position.assign (target->drawX + target->width - action->width - uiconfig->marginSize, target->drawY + target->menuPositionY);
}

void ServerUi::agentAdminActionClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	ServerUi *ui;
	ServerWindow *target;

	ui = (ServerUi *) uiPtr;
	app = App::getInstance ();
	target = ServerWindow::castWidget (ui->actionTarget.widget);
	if (! target) {
		return;
	}

	app->pushUi (new ServerAdminUi (target->agentId, target->agentDisplayName));
}

void ServerUi::agentUpdateActionClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	ServerUi *ui;
	ServerWindow *target;
	StdString url;
	int result;

	ui = (ServerUi *) uiPtr;
	app = App::getInstance ();
	target = ServerWindow::castWidget (ui->actionTarget.widget);
	if (! target) {
		return;
	}

	ui->clearPopupWidgets ();
	url.assign (Util::getUpdateUrl (target->applicationId));
	result = Util::openUrl (url);
	if (result != Result::SUCCESS) {
		app->showSnackbar (app->uiText.getText (UiTextString::openUpdateUrlError));
	}
	else {
		app->showSnackbar (StdString::createSprintf ("%s - %s", app->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void ServerUi::agentRemoveActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerWindow *target;
	App *app;

	ui = (ServerUi *) uiPtr;
	app = App::getInstance ();
	target = ServerWindow::castWidget (ui->actionTarget.widget);
	if (target) {
		app->agentControl.removeAgent (target->agentId);
		ui->cardView->removeItem (target->agentId);
	}
	ui->clearPopupWidgets ();
	app->shouldSyncRecordStore = true;
}

void ServerUi::adminSecretAddButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	AdminSecretWindow *target;
	App *app;
	ActionWindow *action;
	bool show;

	ui = (ServerUi *) uiPtr;
	target = (AdminSecretWindow *) widgetPtr;
	app = App::getInstance ();

	show = true;
	if (ActionWindow::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == target)) {
		show = false;
	}
	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = target->createAddActionWindow ();
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->setCloseCallback (ServerUi::adminSecretAddActionClosed, ui);

	app->rootPanel->addWidget (action);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (target);
	action->position.assign (target->drawX + target->width, target->drawY + target->height - action->height);
}

void ServerUi::adminSecretAddActionClosed (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ActionWindow *action;
	AdminSecretWindow *target;

	ui = (ServerUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}

	target = AdminSecretWindow::castWidget (ui->actionTarget.widget);
	if (! target) {
		return;
	}

	target->addItem (action);
	ui->cardView->refresh ();
}

void ServerUi::adminSecretExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;

	ui = (ServerUi *) uiPtr;
	ui->cardView->refresh ();
}

void ServerUi::serverContactWindowStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;

	ui = (ServerUi *) uiPtr;
	ui->cardView->refresh ();
}
