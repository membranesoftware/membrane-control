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
#include "App.h"
#include "StdString.h"
#include "StringList.h"
#include "Resource.h"
#include "SpriteGroup.h"
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
#include "Chip.h"
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

Widget *ServerAdminUi::createBreadcrumbWidget () {
	return (new Chip (agentDisplayName, sprites.getSprite (ServerAdminUi::BreadcrumbIconSprite)));
}

void ServerAdminUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::serverAdminUiHelpTitle), uitext->getText (UiTextString::serverAdminUiHelpText));
	helpWindow->addAction (uitext->getText (UiTextString::serverAdminUiHelpAction1Text), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("server-passwords"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

int ServerAdminUi::doLoad () {
	UiText *uitext;

	uitext = &(App::instance->uiText);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (2, createRowHeaderPanel (uitext->getText (UiTextString::tasks).capitalized ()));
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	adminSecretWindow = new AdminSecretWindow ();
	adminSecretWindow->setAddButtonClickCallback (ServerAdminUi::adminSecretAddButtonClicked, this);
	adminSecretWindow->setExpandStateChangeCallback (ServerAdminUi::cardExpandStateChanged, this);
	adminSecretWindow->setExpanded (false);
	cardView->addItem (adminSecretWindow, StdString (""), 1);

	App::instance->shouldSyncRecordStore = true;

	return (Result::Success);
}

void ServerAdminUi::doUnload () {
	emptyTaskWindow.clear ();
}

void ServerAdminUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	lockButton = new Button (sprites.getSprite (ServerAdminUi::LockButtonSprite));
	lockButton->setInverseColor (true);
	lockButton->setMouseClickCallback (ServerAdminUi::lockButtonClicked, this);
	lockButton->setMouseHoverTooltip (uitext->getText (UiTextString::serverAdminUiLockTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (lockButton);
}

void ServerAdminUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/ServerAdminUi/bg");
	App::instance->agentControl.connectLinkClient (agentId);

	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void ServerAdminUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void ServerAdminUi::doPause () {
	App::instance->agentControl.disconnectLinkClient (agentId);
}

void ServerAdminUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void ServerAdminUi::doUpdate (int msElapsed) {
	emptyTaskWindow.compact ();
}

void ServerAdminUi::handleLinkClientConnect (const StdString &agentId) {
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_ReadTasks, SystemInterface::Constant_Admin), agentId);
}

void ServerAdminUi::doSyncRecordStore () {
	RecordStore *store;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Json *record, *params;
	ServerWindow *serverwindow;
	AgentConfigurationWindow *configwindow;
	IconCardWindow *iconcardwindow;

	store = &(App::instance->agentControl.recordStore);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	if (! cardView->contains (agentId)) {
		record = store->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
		if (record) {
			serverwindow = new ServerWindow (agentId);
			serverwindow->setExpandStateChangeCallback (ServerAdminUi::cardExpandStateChanged, this);
			serverwindow->setStatusChangeCallback (ServerAdminUi::serverStatusChanged, this);
			serverwindow->setExpanded (true, true);
			cardView->addItem (serverwindow, agentId, 0);

			configwindow = new AgentConfigurationWindow (agentId);
			configwindow->setExpandStateChangeCallback (ServerAdminUi::cardExpandStateChanged, this);
			cardView->addItem (configwindow, StdString (""), 0);
			configwindow->loadConfiguration ();
		}
	}

	watchIdList.clear ();
	taskCount = 0;
	store->processCommandRecords (SystemInterface::CommandId_TaskItem, ServerAdminUi::processTaskItem, this);
	if (! watchIdList.empty ()) {
		params = new Json ();
		params->set ("taskIds", &watchIdList);
		App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_WatchTasks, SystemInterface::Constant_Admin, params), agentId);
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
			iconcardwindow = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::InfoIconSprite), uitext->getText (UiTextString::serverAdminUiEmptyTaskListTitle), StdString (""), uitext->getText (UiTextString::serverAdminUiEmptyTaskListText));
			emptyTaskWindow.assign (iconcardwindow);
			iconcardwindow->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (iconcardwindow, iconcardwindow->itemId, 2);
		}
	}

	cardView->syncRecordStore ();
	cardView->refresh ();
}

void ServerAdminUi::processTaskItem (void *uiPtr, Json *record, const StdString &recordId) {
	ServerAdminUi *ui;
	TaskWindow *window;

	ui = (ServerAdminUi *) uiPtr;

	if (ui->agentId.equals (App::instance->systemInterface.getCommandAgentId (record))) {
		++(ui->taskCount);
		if (! ui->cardView->contains (recordId)) {
			window = new TaskWindow (recordId);
			ui->cardView->addItem (window, recordId, 2);
			ui->watchIdList.push_back (recordId);
		}
	}
}

void ServerAdminUi::cardExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;

	ui = (ServerAdminUi *) uiPtr;
	ui->cardView->refresh ();
}

void ServerAdminUi::serverStatusChanged (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;

	ui = (ServerAdminUi *) uiPtr;
	ui->cardView->refresh ();
}

void ServerAdminUi::adminSecretAddButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;
	AdminSecretWindow *target;
	ActionWindow *action;
	bool show;

	ui = (ServerAdminUi *) uiPtr;
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
	action->closeCallback = Widget::EventCallbackContext (ServerAdminUi::adminSecretAddActionClosed, ui);

	App::instance->rootPanel->addWidget (action);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (target);
	action->position.assign (target->screenX + target->width, target->screenY + target->height - action->height);
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

void ServerAdminUi::lockButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;
	ComboBox *combobox;
	StringList items;
	StringList::iterator i, end;
	HashMap map;
	bool show;

	ui = (ServerAdminUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

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
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setTitleText (uitext->getText (UiTextString::setAdminPassword).capitalized ());
	action->setConfirmTooltipText (uitext->getText (UiTextString::apply).capitalized ());
	action->closeCallback = Widget::EventCallbackContext (ServerAdminUi::setAdminPasswordActionClosed, ui);

	App::instance->agentControl.getAdminSecretNames (&items);
	combobox = new ComboBox ();
	combobox->addItem (StdString (" "), StdString (""));

	while (true) {
		ui->emptyPasswordName.assign (App::instance->getRandomString (16));
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

	App::instance->rootPanel->addWidget (action);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (widgetPtr);
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->position.assign (widgetPtr->screenX + widgetPtr->width - action->width, widgetPtr->screenY - action->height);
}

void ServerAdminUi::setAdminPasswordActionClosed (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;
	UiText *uitext;
	ActionWindow *action;
	StdString name, secret;
	Json *params;
	int result;

	ui = (ServerAdminUi *) uiPtr;
	uitext = &(App::instance->uiText);
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}

	name = action->getStringValue (uitext->getText (UiTextString::password).capitalized (), "");
	if (name.empty ()) {
		return;
	}
	if (! name.equals (ui->emptyPasswordName)) {
		secret = App::instance->agentControl.getAdminSecret (name);
	}

	params = new Json ();
	params->set ("secret", secret);
	ui->retain ();
	result = App::instance->agentControl.invokeCommand (ui->agentId, App::instance->createCommand (SystemInterface::Command_SetAdminSecret, SystemInterface::Constant_DefaultCommandType, params), ServerAdminUi::setAdminSecretComplete, ui);
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		ui->release ();
	}
}

void ServerAdminUi::setAdminSecretComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	ServerAdminUi *ui;
	SystemInterface *interface;

	ui = (ServerAdminUi *) uiPtr;
	interface = &(App::instance->systemInterface);
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::CommandId_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::serverAdminUiSetPasswordCompleteMessage));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	ui->release ();
}
