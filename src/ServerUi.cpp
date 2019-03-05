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

Widget *ServerUi::createBreadcrumbWidget () {
	return (new Chip (App::instance->uiText.getText (UiTextString::servers).capitalized (), sprites.getSprite (ServerUi::BREADCRUMB_ICON)));
}

void ServerUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);

	helpWindow->setHelpText (uitext->getText (UiTextString::serverUiHelpTitle), uitext->getText (UiTextString::serverUiHelpText));
	if (agentCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::serverUiHelpAction1Text), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
	}
	else {
		helpWindow->addAction (uitext->getText (UiTextString::serverUiHelpAction2Text), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::serversHelpTitle), App::getHelpUrl ("servers"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::membraneSoftwareOverviewHelpTitle), App::getHelpUrl ("membrane-software-overview"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

int ServerUi::doLoad () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (0, uitext->getText (UiTextString::serverUiNetworkAgentsTitle), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

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

void ServerUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::RELOAD_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (ServerUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);
}

void ServerUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	Button *button;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);

	button = new Button (StdString (""), sprites.getSprite (ServerUi::BROADCAST_BUTTON));
	button->setInverseColor (true);
	button->shortcutKey = SDLK_F2;
	button->setMouseClickCallback (ServerUi::broadcastButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::serverUiBroadcastTooltip));
	toolbar->addRightItem (button);

	toggle = new Toggle (sprites.getSprite (ServerUi::ADDRESS_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::CANCEL_BUTTON));
	toggle->setInverseColor (true);
	toggle->shortcutKey = SDLK_F1;
	toggle->setStateChangeCallback (ServerUi::addressToggleStateChanged, this);
	toggle->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::serverUiAddressTooltip));
	toolbar->addRightItem (toggle);

	addressToggle.destroyAndClear ();
	addressToggle.assign (toggle);
}

void ServerUi::doClearPopupWidgets () {

}

void ServerUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/ServerUi/bg");
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	adminSecretWindow->readItems ();
}

void ServerUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
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
	cardView->processItems (ServerUi::findDeletedWindows, this);
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

void ServerUi::findDeletedWindows (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerWindow *serverwindow;
	ServerContactWindow *servercontactwindow;

	ui = (ServerUi *) uiPtr;
	serverwindow = ServerWindow::castWidget (widgetPtr);
	if (serverwindow) {
		if (serverwindow->isRecordDeleted) {
			ui->cardIdList.push_back (serverwindow->itemId);
		}
	}
	else {
		servercontactwindow = ServerContactWindow::castWidget (widgetPtr);
		if (servercontactwindow) {
			if (servercontactwindow->isDeleted) {
				ui->cardIdList.push_back (servercontactwindow->itemId);
			}
		}
	}
}

void ServerUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	if (addressTextFieldWindow.widget) {
		((TextFieldWindow *) addressTextFieldWindow.widget)->setWindowWidth (App::instance->uiStack.secondaryToolbar->getLeftWidth ());
	}
}

void ServerUi::doSyncRecordStore () {
	RecordStore *store;
	UiConfiguration *uiconfig;
	UiText *uitext;
	IconCardWindow *window;

	store = &(App::instance->agentControl.recordStore);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	agentCount = 0;
	store->processCommandRecords (SystemInterface::CommandId_AgentStatus, ServerUi::processAgentStatus, this);
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
			window->setLink (uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
			emptyServerWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, 0);
		}
	}

	cardView->syncRecordStore ();
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
	interface = &(App::instance->systemInterface);
	++(ui->agentCount);

	if (! ui->cardView->contains (recordId)) {
		window = new ServerWindow (recordId);
		window->sortKey.assign (interface->getCommandAgentName (record).lowercased ());
		window->setMenuClickCallback (ServerUi::agentMenuClicked, ui);
		window->itemId.assign (recordId);
		ui->cardView->addItem (window, recordId, 0);
	}
}

void ServerUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;

	ui = (ServerUi *) uiPtr;
	ui->cardView->processItems (ServerUi::reloadAgent, ui);
	App::instance->agentControl.retryAgents ();
}

void ServerUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	ServerWindow *serverwindow;
	ServerContactWindow *servercontactwindow;

	serverwindow = ServerWindow::castWidget (widgetPtr);
	if (serverwindow) {
		App::instance->agentControl.refreshAgentStatus (serverwindow->agentId);
	}
	else {
		servercontactwindow = ServerContactWindow::castWidget (widgetPtr);
		if (servercontactwindow) {
			App::instance->agentControl.contactAgent (servercontactwindow->agentHostname, servercontactwindow->agentPort);
		}
	}
}

void ServerUi::broadcastButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App::instance->agentControl.broadcastContactMessage ();
	App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::sentBroadcast));
}

void ServerUi::addressToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	UiConfiguration *uiconfig;
	Toolbar *toolbar;
	Toggle *toggle;
	TextFieldWindow *textfield;

	ui = (ServerUi *) uiPtr;
	toggle = (Toggle *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	toolbar = App::instance->uiStack.secondaryToolbar;

	if (toggle->isChecked) {
		textfield = new TextFieldWindow (toolbar->getLeftWidth (), App::instance->uiText.getText (UiTextString::enterAddressPrompt), ui->sprites.getSprite (ServerUi::ADDRESS_ICON));
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
	ServerUi *ui;
	TextFieldWindow *textfield;
	ServerContactWindow *window;
	StdString address, hostname, key;
	int port;

	ui = (ServerUi *) uiPtr;
	textfield = (TextFieldWindow *) widgetPtr;
	address.assign (textfield->getValue ());
	if (address.empty ()) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeGetStatusAddressEmptyError), App::instance->uiText.getText (UiTextString::help).uppercased (), ServerUi::addressSnackbarHelpClicked, ui);
		return;
	}

	if (! address.parseAddress (&hostname, &port, SystemInterface::Constant_DefaultTcpPort1)) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeGetStatusAddressParseError), App::instance->uiText.getText (UiTextString::help).uppercased (), ServerUi::addressSnackbarHelpClicked, ui);
		return;
	}

	if (App::instance->agentControl.isContacted (hostname, port)) {
		App::instance->agentControl.refreshAgentStatus (hostname, port);
		return;
	}

	App::instance->agentControl.contactAgent (hostname, port);
	App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", App::instance->uiText.getText (UiTextString::serverUiContactingAgentMessage).c_str (), address.c_str ()));

	key.sprintf ("%s:%i", hostname.c_str (), port);
	if (! ui->cardView->contains (key)) {
		window = new ServerContactWindow (address, hostname, port);
		window->setStateChangeCallback (ServerUi::serverContactWindowStateChanged, ui);
		window->sortKey.assign (address.lowercased ());
		window->itemId.assign (key);
		ui->cardView->addItem (window, window->itemId, 0);
	}
	App::instance->shouldSyncRecordStore = true;
}

void ServerUi::addressSnackbarHelpClicked (void *uiPtr, Widget *widgetPtr) {
	StdString url;
	int result;

	url.assign (App::getHelpUrl ("servers"));
	result = OsUtil::openUrl (url);
	if (result != Result::SUCCESS) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openHelpUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void ServerUi::agentMenuClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ServerWindow *target;
	Menu *action;
	bool show;

	ui = (ServerUi *) uiPtr;
	target = (ServerWindow *) widgetPtr;
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

	action->addItem (uitext->getText (UiTextString::adminConsole).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::AGENT_ADMIN_BUTTON), ServerUi::agentAdminActionClicked, ui);
	if (! target->applicationId.empty ()) {
		action->addItem (uitext->getText (UiTextString::checkForUpdates).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::UPDATE_BUTTON), ServerUi::agentUpdateActionClicked, ui);
	}

	action->addItem (uitext->getText (UiTextString::remove).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::DELETE_BUTTON), ServerUi::agentRemoveActionClicked, ui);

	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->position.assign (target->drawX + target->width - action->width - uiconfig->marginSize, target->drawY + target->menuPositionY);
}

void ServerUi::agentAdminActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerWindow *target;

	ui = (ServerUi *) uiPtr;
	target = ServerWindow::castWidget (ui->actionTarget.widget);
	if (! target) {
		return;
	}

	App::instance->uiStack.pushUi (new ServerAdminUi (target->agentId, target->agentDisplayName));
}

void ServerUi::agentUpdateActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerWindow *target;
	StdString url;
	int result;

	ui = (ServerUi *) uiPtr;
	target = ServerWindow::castWidget (ui->actionTarget.widget);
	if (! target) {
		return;
	}

	ui->clearPopupWidgets ();
	url.assign (App::getUpdateUrl (target->applicationId));
	result = OsUtil::openUrl (url);
	if (result != Result::SUCCESS) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openUpdateUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void ServerUi::agentRemoveActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerWindow *target;

	ui = (ServerUi *) uiPtr;
	target = ServerWindow::castWidget (ui->actionTarget.widget);
	if (target) {
		App::instance->agentControl.removeAgent (target->agentId);
		ui->cardView->removeItem (target->agentId);
	}
	ui->clearPopupWidgets ();
	App::instance->shouldSyncRecordStore = true;
}

void ServerUi::adminSecretAddButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	AdminSecretWindow *target;
	ActionWindow *action;
	bool show;

	ui = (ServerUi *) uiPtr;
	target = (AdminSecretWindow *) widgetPtr;

	show = true;
	if (ActionWindow::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == target)) {
		show = false;
	}
	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = target->createAddActionWindow ();
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->setCloseCallback (ServerUi::adminSecretAddActionClosed, ui);

	App::instance->rootPanel->addWidget (action);
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
