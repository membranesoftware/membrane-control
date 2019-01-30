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
#include "App.h"
#include "StdString.h"
#include "StringList.h"
#include "Resource.h"
#include "SpriteGroup.h"
#include "Util.h"
#include "AgentControl.h"
#include "Button.h"
#include "ComboBox.h"
#include "SystemInterface.h"
#include "HashMap.h"
#include "Ui.h"
#include "UiText.h"
#include "Toolbar.h"
#include "Menu.h"
#include "CardView.h"
#include "Toggle.h"
#include "ActionWindow.h"
#include "IconCardWindow.h"
#include "TaskWindow.h"
#include "ServerWindow.h"
#include "HelpWindow.h"
#include "AgentConfigurationWindow.h"
#include "AdminSecretWindow.h"
#include "ServerAdminUi.h"

ServerAdminUi::ServerAdminUi (const StdString &agentId, const StdString &agentDisplayName)
: Ui ()
, agentId (agentId)
, agentDisplayName (agentDisplayName)
, cardView (NULL)
, lockButton (NULL)
, adminSecretWindow (NULL)
, taskCount (0)
{

}

ServerAdminUi::~ServerAdminUi () {

}

StdString ServerAdminUi::getSpritePath () {
	return (StdString ("ui/ServerAdminUi/sprite"));
}

StdString ServerAdminUi::getBreadcrumbTitle () {
	return (agentDisplayName);
}

Sprite *ServerAdminUi::getBreadcrumbSprite () {
	return (sprites.getSprite (ServerAdminUi::BREADCRUMB_ICON)->copy ());
}

void ServerAdminUi::setHelpWindowContent (Widget *helpWindowPtr) {
	App *app;
	HelpWindow *help;
	UiText *uitext;

	app = App::getInstance ();
	uitext = &(app->uiText);
	help = (HelpWindow *) helpWindowPtr;

	help->setHelpText (uitext->getText (UiTextString::serverAdminUiHelpTitle), uitext->getText (UiTextString::serverAdminUiHelpText));
	help->addAction (uitext->getText (UiTextString::serverAdminUiHelpAction1Text), uitext->getText (UiTextString::learnMore).capitalized (), Util::getHelpUrl ("server-passwords"));
	help->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), Util::getHelpUrl (""));
}

int ServerAdminUi::doLoad () {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (2, uitext->getText (UiTextString::tasks).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, app->topBarHeight);

	adminSecretWindow = new AdminSecretWindow ();
	adminSecretWindow->setAddButtonClickCallback (ServerAdminUi::adminSecretAddButtonClicked, this);
	adminSecretWindow->setExpandStateChangeCallback (ServerAdminUi::adminSecretExpandStateChanged, this);
	adminSecretWindow->setExpanded (false);
	cardView->addItem (adminSecretWindow, StdString (""), 1);

	app->shouldSyncRecordStore = true;

	return (Result::SUCCESS);
}

void ServerAdminUi::doUnload () {
	emptyTaskWindow.clear ();
}

void ServerAdminUi::doResetMainToolbar (Toolbar *toolbar) {
	addMainToolbarBackButton (toolbar);
}

void ServerAdminUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	UiText *uitext;

	uitext = &(App::getInstance ()->uiText);
	lockButton = new Button (StdString (""), sprites.getSprite (ServerAdminUi::LOCK_BUTTON));
	lockButton->setInverseColor (true);
	lockButton->setMouseClickCallback (ServerAdminUi::lockButtonClicked, this);
	lockButton->setMouseHoverTooltip (uitext->getText (UiTextString::serverAdminUiLockTooltip), Widget::LEFT);
	toolbar->addRightItem (lockButton);

	toolbar->isVisible = true;
}

void ServerAdminUi::doClearPopupWidgets () {

}

void ServerAdminUi::doResume () {
	App *app;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/ServerAdminUi/bg");
	app->agentControl.connectLinkClient (agentId);

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void ServerAdminUi::doRefresh () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void ServerAdminUi::doPause () {
	App *app;

	app = App::getInstance ();
	app->agentControl.disconnectLinkClient (agentId);
}

void ServerAdminUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void ServerAdminUi::doUpdate (int msElapsed) {
	emptyTaskWindow.compact ();
}

void ServerAdminUi::handleLinkClientConnect (const StdString &agentId) {
	App *app;

	app = App::getInstance ();
	app->agentControl.writeLinkCommand (app->createCommand ("ReadTasks", SystemInterface::Constant_Admin), agentId);
}

void ServerAdminUi::doSyncRecordStore (RecordStore *store) {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Json *record, *params;
	ServerWindow *serverwindow;
	AgentConfigurationWindow *configwindow;
	IconCardWindow *iconcardwindow;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	if (! cardView->contains (agentId)) {
		record = store->findRecord (agentId, SystemInterface::Command_AgentStatus);
		if (record) {
			serverwindow = new ServerWindow (agentId);
			cardView->addItem (serverwindow, agentId, 0);

			configwindow = new AgentConfigurationWindow (agentId);
			configwindow->setExpandStateChangeCallback (ServerAdminUi::configurationExpandStateChanged, this);
			cardView->addItem (configwindow, StdString (""), 0);
			configwindow->loadConfiguration ();
		}
	}

	watchIdList.clear ();
	taskCount = 0;
	store->processCommandRecords (SystemInterface::Command_TaskItem, ServerAdminUi::processTaskItem, this);
	if (! watchIdList.empty ()) {
		params = new Json ();
		params->set ("taskIds", &watchIdList);
		app->agentControl.writeLinkCommand (app->createCommand ("WatchTasks", SystemInterface::Constant_Admin, params), agentId);
	}

	iconcardwindow = (IconCardWindow *) emptyTaskWindow.widget;
	if (taskCount > 0) {
		if (iconcardwindow) {
			cardView->removeItem (iconcardwindow->itemId);
			emptyTaskWindow.clear ();
		}
	}
	else {
		if (! iconcardwindow) {
			iconcardwindow = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::INFO_ICON), uitext->getText (UiTextString::serverAdminUiEmptyTaskListTitle), StdString (""), uitext->getText (UiTextString::serverAdminUiEmptyTaskListText));
			emptyTaskWindow.assign (iconcardwindow);
			iconcardwindow->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (iconcardwindow, iconcardwindow->itemId, 2);
		}
	}

	cardView->syncRecordStore (store);
	cardView->refresh ();
}

void ServerAdminUi::processTaskItem (void *uiPtr, Json *record, const StdString &recordId) {
	ServerAdminUi *ui;
	App *app;
	TaskWindow *window;

	ui = (ServerAdminUi *) uiPtr;
	app = App::getInstance ();

	if (ui->agentId.equals (app->systemInterface.getCommandAgentId (record))) {
		++(ui->taskCount);
		if (! ui->cardView->contains (recordId)) {
			window = new TaskWindow (recordId);
			ui->cardView->addItem (window, recordId, 2);
			ui->watchIdList.push_back (recordId);
		}
	}
}

void ServerAdminUi::configurationExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;

	ui = (ServerAdminUi *) uiPtr;
	ui->cardView->refresh ();
}

void ServerAdminUi::adminSecretAddButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;
	AdminSecretWindow *target;
	App *app;
	ActionWindow *action;
	bool show;

	ui = (ServerAdminUi *) uiPtr;
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
	action->setCloseCallback (ServerAdminUi::adminSecretAddActionClosed, ui);

	app->rootPanel->addWidget (action);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (target);
	action->position.assign (target->drawX + target->width, target->drawY + target->height - action->height);
}

void ServerAdminUi::adminSecretAddActionClosed (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;
	ActionWindow *action;
	AdminSecretWindow *target;

	ui = (ServerAdminUi *) uiPtr;
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

void ServerAdminUi::adminSecretExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;

	ui = (ServerAdminUi *) uiPtr;
	ui->cardView->refresh ();
}

void ServerAdminUi::lockButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;
	App *app;
	UiText *uitext;
	ActionWindow *action;
	ComboBox *combobox;
	StringList items;
	StringList::iterator i, end;
	HashMap map;
	bool show;

	ui = (ServerAdminUi *) uiPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);

	show = true;
	if (ActionWindow::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == widgetPtr)) {
		show = false;
	}
	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = new ActionWindow ();
	action->setInverseColor (true);
	action->setTitleText (uitext->getText (UiTextString::setAdminPassword).capitalized ());
	action->setConfirmButtonText (uitext->getText (UiTextString::apply).uppercased ());
	action->setCloseCallback (ServerAdminUi::setAdminPasswordActionClosed, ui);

	app->agentControl.getAdminSecretNames (&items);
	combobox = new ComboBox ();
	combobox->addItem (StdString (" "), StdString (""));

	while (true) {
		ui->emptyPasswordName.assign (app->getRandomString (16));
		if (! items.contains (ui->emptyPasswordName)) {
			break;
		}
	}
	combobox->addItem (uitext->getText (UiTextString::serverAdminUiEmptyAdminPasswordText), ui->emptyPasswordName);

	if (! items.empty ()) {
		i = items.begin ();
		end = items.end ();
		while (i != end) {
			map.insert (*i, *i);
			++i;
		}
		map.sort (HashMap::sortAscending);
		combobox->addItems (&map);
	}
	action->addOption (uitext->getText (UiTextString::password).capitalized (), combobox, uitext->getText (UiTextString::serverAdminUiAdminPasswordDescription));
	action->setOptionNotEmptyString (uitext->getText (UiTextString::password).capitalized ());

	app->rootPanel->addWidget (action);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (widgetPtr);
	action->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (widgetPtr->drawX + widgetPtr->width - action->width, widgetPtr->drawY - action->height);
}

void ServerAdminUi::setAdminPasswordActionClosed (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;
	App *app;
	UiText *uitext;
	ActionWindow *action;
	StdString name, secret;
	Json *params;
	int result;

	ui = (ServerAdminUi *) uiPtr;
	app = App::getInstance ();
	uitext = &(app->uiText);
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}

	name = action->getStringValue (uitext->getText (UiTextString::password).capitalized (), "");
	if (name.empty ()) {
		return;
	}
	if (! name.equals (ui->emptyPasswordName)) {
		secret = app->agentControl.getAdminSecret (name);
	}

	params = new Json ();
	params->set ("secret", secret);
	ui->retain ();
	result = app->agentControl.invokeCommand (ui->agentId, app->createCommand ("SetAdminSecret", SystemInterface::Constant_DefaultCommandType, params), ServerAdminUi::setAdminSecretComplete, ui);
	if (result != Result::SUCCESS) {
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
		ui->release ();
	}
}

void ServerAdminUi::setAdminSecretComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	ServerAdminUi *ui;
	App *app;
	SystemInterface *interface;

	ui = (ServerAdminUi *) uiPtr;
	app = App::getInstance ();
	interface = &(app->systemInterface);
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		app->showSnackbar (app->uiText.getText (UiTextString::serverAdminUiSetPasswordCompleteMessage));
	}
	else {
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
	}
	ui->release ();
}
