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
#include "ServerAdminUi.h"

ServerAdminUi::ServerAdminUi (const StdString &agentId, const StdString &agentDisplayName)
: Ui ()
, agentId (agentId)
, agentDisplayName (agentDisplayName)
, cardView (NULL)
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
	cardView->setRowHeader (1, uitext->getText (UiTextString::tasks).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, app->topBarHeight);

	app->shouldSyncRecordStore = true;

	return (Result::SUCCESS);
}

void ServerAdminUi::doUnload () {
	emptyTaskWindow.clear ();
}

void ServerAdminUi::doResetMainToolbar (Toolbar *toolbar) {
	addMainToolbarBackButton (toolbar);
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
	app->agentControl.writeLinkCommand (app->createCommandJson ("ReadTasks", SystemInterface::Constant_Admin), agentId);
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
		app->agentControl.writeLinkCommand (app->createCommandJson ("WatchTasks", SystemInterface::Constant_Admin, params), agentId);
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
			cardView->addItem (iconcardwindow, iconcardwindow->itemId, 1);
		}
	}

	cardView->syncRecordStore (store);
	cardView->refresh ();
}

void ServerAdminUi::processTaskItem (void *uiPtr, Json *record, const StdString &recordId) {
	ServerAdminUi *ui;
	TaskWindow *window;

	ui = (ServerAdminUi *) uiPtr;
	++(ui->taskCount);
	if (! ui->cardView->contains (recordId)) {
		window = new TaskWindow (recordId);
		ui->cardView->addItem (window, recordId, 1);
		ui->watchIdList.push_back (recordId);
	}
}
