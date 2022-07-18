/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "App.h"
#include "OsUtil.h"
#include "StdString.h"
#include "StringList.h"
#include "Resource.h"
#include "SpriteGroup.h"
#include "AgentControl.h"
#include "Button.h"
#include "ComboBox.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "HashMap.h"
#include "Ui.h"
#include "UiConfiguration.h"
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
	return (new Chip (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (agentDisplayName, ((float) App::instance->windowWidth) * 0.5f, Font::DotTruncateSuffix), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SettingsBoxButtonSprite)));
}

void ServerAdminUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextString::ServerAdminUiHelpTitle), UiText::instance->getText (UiTextString::ServerAdminUiHelpText));
	helpWindow->addAction (UiText::instance->getText (UiTextString::ServerAdminUiHelpAction1Text));
	helpWindow->addAction (UiText::instance->getText (UiTextString::ServerAdminUiHelpAction2Text), UiText::instance->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("server-passwords"));
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

OsUtil::Result ServerAdminUi::doLoad () {
	cardView->setRowHeader (ServerAdminUi::TaskRow, createRowHeaderPanel (UiText::instance->getText (UiTextString::Tasks).capitalized ()));

	cardView->setRowHeader (ServerAdminUi::ServerPasswordsRow, createRowHeaderPanel (UiText::instance->getText (UiTextString::ServerPasswords).capitalized ()));
	adminSecretWindow = new AdminSecretWindow ();
	adminSecretWindow->addButtonClickCallback = Widget::EventCallbackContext (ServerAdminUi::adminSecretAddButtonClicked, this);
	adminSecretWindow->expandStateChangeCallback = Widget::EventCallbackContext (ServerAdminUi::cardExpandStateChanged, this);
	adminSecretWindow->setExpanded (false);
	cardView->addItem (adminSecretWindow, StdString (""), ServerAdminUi::ServerPasswordsRow);

	App::instance->shouldSyncRecordStore = true;

	return (OsUtil::Success);
}

void ServerAdminUi::doUnload () {
	emptyTaskWindow.clear ();
}

void ServerAdminUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	lockButton = new Button (sprites.getSprite (ServerAdminUi::LockButtonSprite));
	lockButton->mouseClickCallback = Widget::EventCallbackContext (ServerAdminUi::lockButtonClicked, this);
	lockButton->setInverseColor (true);
	lockButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ServerAdminUiLockTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (lockButton);
}

void ServerAdminUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/ServerAdminUi/bg");
	AgentControl::instance->connectLinkClient (agentId);
}

void ServerAdminUi::doPause () {
	AgentControl::instance->disconnectLinkClient (agentId);
}

void ServerAdminUi::doUpdate (int msElapsed) {
	emptyTaskWindow.compact ();
}

void ServerAdminUi::handleLinkClientConnect (const StdString &agentId) {
	AgentControl::instance->writeLinkCommand (App::instance->createCommand (SystemInterface::Command_ReadTasks), agentId);
}

void ServerAdminUi::doSyncRecordStore () {
	Json *record, *params;
	ServerWindow *serverwindow;
	AgentConfigurationWindow *configwindow;
	IconCardWindow *iconcardwindow;

	if (! cardView->contains (agentId)) {
		record = RecordStore::instance->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
		if (record) {
			serverwindow = new ServerWindow (agentId);
			serverwindow->expandStateChangeCallback = Widget::EventCallbackContext (ServerAdminUi::cardExpandStateChanged, this);
			serverwindow->statusChangeCallback = Widget::EventCallbackContext (ServerAdminUi::serverStatusChanged, this);
			serverwindow->setExpanded (true, true);
			cardView->addItem (serverwindow, agentId, ServerAdminUi::ServerRow);

			configwindow = new AgentConfigurationWindow (agentId);
			configwindow->expandStateChangeCallback = Widget::EventCallbackContext (ServerAdminUi::cardExpandStateChanged, this);
			cardView->addItem (configwindow, StdString (""), ServerAdminUi::ServerRow);
			configwindow->loadConfiguration ();
		}
	}

	watchIdList.clear ();
	taskCount = 0;
	RecordStore::instance->processCommandRecords (SystemInterface::CommandId_TaskItem, ServerAdminUi::processTaskItem, this);
	if (! watchIdList.empty ()) {
		params = new Json ();
		params->set ("taskIds", watchIdList);
		AgentControl::instance->writeLinkCommand (App::instance->createCommand (SystemInterface::Command_WatchTasks, params), agentId);
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
			iconcardwindow = new IconCardWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::InfoIconSprite));
			iconcardwindow->setName (UiText::instance->getText (UiTextString::ServerAdminUiEmptyTaskListTitle));
			iconcardwindow->setDetailText (UiText::instance->getText (UiTextString::ServerAdminUiEmptyTaskListText));
			emptyTaskWindow.assign (iconcardwindow);
			iconcardwindow->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (iconcardwindow, iconcardwindow->itemId, ServerAdminUi::TaskRow);
		}
	}

	cardView->syncRecordStore ();
	cardView->refresh ();
}

void ServerAdminUi::processTaskItem (void *uiPtr, Json *record, const StdString &recordId) {
	ServerAdminUi *ui;
	TaskWindow *window;

	ui = (ServerAdminUi *) uiPtr;

	if (ui->agentId.equals (SystemInterface::instance->getCommandAgentId (record))) {
		++(ui->taskCount);
		if (! ui->cardView->contains (recordId)) {
			window = new TaskWindow (recordId);
			ui->cardView->addItem (window, recordId, ServerAdminUi::TaskRow);
			ui->watchIdList.push_back (recordId);
		}
	}
}

void ServerAdminUi::cardExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;

	ui = (ServerAdminUi *) uiPtr;
	ui->clearPopupWidgets ();
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

	ui = (ServerAdminUi *) uiPtr;
	target = (AdminSecretWindow *) widgetPtr;

	if (ui->clearActionPopup (widgetPtr, ServerAdminUi::adminSecretAddButtonClicked)) {
		return;
	}

	action = target->createAddActionWindow ();
	action->closeCallback = Widget::EventCallbackContext (ServerAdminUi::adminSecretAddActionClosed, ui);

	ui->showActionPopup (action, widgetPtr, ServerAdminUi::adminSecretAddButtonClicked, widgetPtr->getScreenRect (), Ui::RightOfAlignment, Ui::BottomEdgeAlignment);
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
	ActionWindow *action;
	ComboBox *combobox;
	StringList items;
	StringList::iterator i, end;
	int listindex;

	ui = (ServerAdminUi *) uiPtr;
	if (ui->clearActionPopup (widgetPtr, ServerAdminUi::lockButtonClicked)) {
		return;
	}
	action = new ActionWindow ();
	action->setInverseColor (true);
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setTitleText (UiText::instance->getText (UiTextString::SetAdminPassword).capitalized ());
	action->setConfirmTooltipText (UiText::instance->getText (UiTextString::Apply).capitalized ());
	action->closeCallback = Widget::EventCallbackContext (ServerAdminUi::setAdminPasswordActionClosed, ui);

	AgentControl::instance->getAdminSecretNames (&items);
	combobox = new ComboBox ();
	combobox->addItem (StdString (" "), StdString (""));
	combobox->addItem (UiText::instance->getText (UiTextString::ServerAdminUiEmptyAdminPasswordText), StdString ("-1"));
	if (! items.empty ()) {
		listindex = 0;
		i = items.begin ();
		end = items.end ();
		while (i != end) {
			combobox->addItem (*i, StdString::createSprintf ("%i", listindex));
			++listindex;
			++i;
		}
	}
	action->addOption (UiText::instance->getText (UiTextString::Password).capitalized (), combobox, UiText::instance->getText (UiTextString::ServerAdminUiAdminPasswordDescription));
	action->setOptionNotEmptyString (UiText::instance->getText (UiTextString::Password).capitalized ());

	ui->showActionPopup (action, widgetPtr, ServerAdminUi::lockButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void ServerAdminUi::setAdminPasswordActionClosed (void *uiPtr, Widget *widgetPtr) {
	ServerAdminUi *ui;
	ActionWindow *action;
	Json *params;
	StdString adminsecret;
	int secretindex;

	ui = (ServerAdminUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}
	ui->lastSecretIndex = action->getStringValue (UiText::instance->getText (UiTextString::Password).capitalized (), "");
	if (ui->lastSecretIndex.empty ()) {
		return;
	}
	if (! ui->lastSecretIndex.parseInt (&secretindex)) {
		return;
	}

	adminsecret = AgentControl::instance->getAdminSecretValue (secretindex);
	params = new Json ();
	params->set ("secret", adminsecret);
	ui->invokeCommand (CommandHistory::instance->setAdminSecret (StringList (ui->agentId)), ui->agentId, App::instance->createCommand (SystemInterface::Command_SetAdminSecret, params), ServerAdminUi::setAdminSecretComplete);
}

void ServerAdminUi::setAdminSecretComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess) {
	ServerAdminUi *ui;
	StdString val;
	int secretindex;

	ui = (ServerAdminUi *) invokeUi;
	if (isResponseCommandSuccess) {
		val.assign (ui->lastSecretIndex);
		ui->lastSecretIndex.assign ("");
		if (! val.parseInt (&secretindex)) {
			secretindex = -1;
		}
		AgentControl::instance->setAgentAuthorization (agentId, secretindex);
	}
}
