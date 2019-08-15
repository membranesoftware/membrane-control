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
#include "StdString.h"
#include "App.h"
#include "Resource.h"
#include "StringList.h"
#include "OsUtil.h"
#include "AgentControl.h"
#include "CommandStore.h"
#include "Ui.h"
#include "SystemInterface.h"
#include "Chip.h"
#include "ActionWindow.h"
#include "TextFieldWindow.h"
#include "IconCardWindow.h"
#include "CommandWindow.h"
#include "CommandUi.h"

CommandUi::CommandUi ()
: Ui ()
, cardView (NULL)
{

}

CommandUi::~CommandUi () {

}

StdString CommandUi::getSpritePath () {
	return (StdString ("ui/CommandUi/sprite"));
}

Widget *CommandUi::createBreadcrumbWidget () {
	return (new Chip (App::instance->uiText.getText (UiTextString::commands).capitalized (), sprites.getSprite (CommandUi::BreadcrumbIconSprite)));
}

void CommandUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);

	helpWindow->setHelpText (uitext->getText (UiTextString::commandUiHelpTitle), uitext->getText (UiTextString::commandUiHelpText));
	helpWindow->addAction (uitext->getText (UiTextString::commandUiHelpActionText));
}

int CommandUi::doLoad () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	CommandStore *store;
	std::vector<int64_t> idlist;
	std::vector<int64_t>::iterator i, end;
	CommandWindow *command;
	IconCardWindow *emptycard;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	store = &(App::instance->agentControl.commandStore);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (CommandUi::StoredCommandRow, createRowHeaderPanel (uitext->getText (UiTextString::storedCommands).capitalized ()));
	cardView->setRowHeader (CommandUi::RecentCommandRow, createRowHeaderPanel (uitext->getText (UiTextString::recentCommands).capitalized ()));
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	store->getStoredCommandIds (&idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		command = new CommandWindow (*i, &sprites);
		command->setNameClickCallback (CommandUi::commandNameClicked, this);
		command->setExecuteClickCallback (CommandUi::commandExecuteActionClicked, this);
		command->setDeleteClickCallback (CommandUi::commandDeleteActionClicked, this);
		command->refreshCommandData ();

		command->itemId = cardView->getAvailableItemId ();
		command->animateNewCard ();
		cardView->addItem (command, command->itemId, CommandUi::StoredCommandRow);
		++i;
	}

	store->getRecentCommandIds (&idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		command = new CommandWindow (*i, &sprites);
		command->setSaveClickCallback (CommandUi::commandSaveActionClicked, this);
		command->refreshCommandData ();

		command->itemId = cardView->getAvailableItemId ();
		command->animateNewCard ();
		cardView->addItem (command, command->itemId, CommandUi::RecentCommandRow);
		++i;
	}

	if (cardView->getItemCount () <= 0) {
		emptycard = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeCommandIconSprite), uitext->getText (UiTextString::commandUiEmptyStatusTitle), StdString (""), uitext->getText (UiTextString::commandUiEmptyStatusText));
		cardView->addItem (emptycard, StdString (""), CommandUi::EmptyStateRow);
	}

	cardView->refresh ();
	return (Result::Success);
}

void CommandUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/CommandUi/bg");
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void CommandUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void CommandUi::doPause () {

}

void CommandUi::commandNameClicked (void *uiPtr, Widget *widgetPtr) {
	CommandUi *ui;
	CommandWindow *command;
	UiConfiguration *uiconfig;
	UiText *uitext;
	TextFieldWindow *action;

	ui = (CommandUi *) uiPtr;
	command = (CommandWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	ui->clearPopupWidgets ();
	action = (TextFieldWindow *) App::instance->rootPanel->addWidget (new TextFieldWindow ((float) App::instance->windowWidth * 0.40f, uitext->getText (UiTextString::enterStoredCommandNamePrompt)));
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (command);
	action->setValue (command->commandName);
	action->setEditCallback (CommandUi::commandNameEdited, ui);
	action->setFillBg (true, uiconfig->lightPrimaryColor);
	action->setButtonsEnabled (true, true, true, true);
	action->assignKeyFocus ();
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->position.assignBounded (command->screenX + uiconfig->marginSize, command->screenY, uiconfig->marginSize, uiconfig->marginSize, (float) App::instance->windowWidth - action->width - uiconfig->marginSize, (float) App::instance->windowHeight - action->height - uiconfig->marginSize - App::instance->uiStack.bottomBarHeight);
}

void CommandUi::commandNameEdited (void *uiPtr, Widget *widgetPtr) {
	CommandUi *ui;
	TextFieldWindow *action;
	CommandWindow *command;
	CommandStore *store;
	StdString name;

	ui = (CommandUi *) uiPtr;
	action = TextFieldWindow::castWidget (ui->actionWidget.widget);
	command = CommandWindow::castWidget (ui->actionTarget.widget);
	store = &(App::instance->agentControl.commandStore);
	if ((! action) || (! command)) {
		return;
	}

	name = action->getValue ();
	if (! name.empty ()) {
		store->setStoredCommandName (command->storeId, name);
		command->refreshCommandData ();
		ui->cardView->refresh ();
	}
	ui->clearPopupWidgets ();
}

void CommandUi::commandSaveActionClicked (void *uiPtr, Widget *widgetPtr) {
	CommandUi *ui;
	CommandWindow *command;
	CommandStore *store;
	int64_t storeid;

	ui = (CommandUi *) uiPtr;
	command = (CommandWindow *) widgetPtr;
	store = &(App::instance->agentControl.commandStore);
	storeid = command->storeId;
	if (store->storeRecentCommand (storeid)) {
		ui->cardView->removeItem (command->itemId);

		command = new CommandWindow (storeid, &(ui->sprites));
		command->setNameClickCallback (CommandUi::commandNameClicked, ui);
		command->setExecuteClickCallback (CommandUi::commandExecuteActionClicked, ui);
		command->setDeleteClickCallback (CommandUi::commandDeleteActionClicked, ui);
		command->refreshCommandData ();

		command->itemId = ui->cardView->getAvailableItemId ();
		command->animateNewCard ();
		ui->cardView->addItem (command, command->itemId, CommandUi::StoredCommandRow);
	}
}

void CommandUi::commandExecuteActionClicked (void *uiPtr, Widget *widgetPtr) {
	CommandWindow *command;
	CommandStore *store;
	UiText *uitext;
	int count;

	command = (CommandWindow *) widgetPtr;
	store = &(App::instance->agentControl.commandStore);
	uitext = &(App::instance->uiText);

	count = store->executeStoredCommand (command->storeId);
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (uitext->getText (UiTextString::storedCommandFailedExecutionError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", uitext->getText (UiTextString::storedCommandExecutedMessage).c_str (), command->commandName.c_str ()));
	}
}

void CommandUi::commandDeleteActionClicked (void *uiPtr, Widget *widgetPtr) {
	CommandUi *ui;
	CommandWindow *command;
	CommandStore *store;

	ui = (CommandUi *) uiPtr;
	command = (CommandWindow *) widgetPtr;
	store = &(App::instance->agentControl.commandStore);

	store->removeStoredCommand (command->storeId);
	ui->cardView->removeItem (command->itemId);
}
