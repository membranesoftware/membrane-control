/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "ServerAttachWindow.h"
#include "AdminSecretWindow.h"
#include "ServerAdminUi.h"
#include "ServerUi.h"

const char *ServerUi::UnexpandedAgentsKey = "Server_UnexpandedAgents";

ServerUi::ServerUi ()
: Ui ()
, adminSecretWindow (NULL)
, attachedServerCount (0)
, unattachedServerCount (0)
{

}

ServerUi::~ServerUi () {

}

StdString ServerUi::getSpritePath () {
	return (StdString ("ui/ServerUi/sprite"));
}

Widget *ServerUi::createBreadcrumbWidget () {
	return (new Chip (App::instance->uiText.getText (UiTextString::Servers).capitalized (), sprites.getSprite (ServerUi::BreadcrumbIconSprite)));
}

void ServerUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);

	helpWindow->setHelpText (uitext->getText (UiTextString::ServerUiHelpTitle), uitext->getText (UiTextString::ServerUiHelpText));
	if ((attachedServerCount <= 0) && (unattachedServerCount <= 0)) {
		helpWindow->addAction (uitext->getText (UiTextString::ServerUiHelpAction1Text), uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
	}
	else {
		helpWindow->addAction (uitext->getText (UiTextString::ServerUiHelpAction2Text), uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::ServersHelpTitle), App::getHelpUrl ("servers"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::MembraneSoftwareOverviewHelpTitle), App::getHelpUrl ("membrane-software-overview"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

int ServerUi::doLoad () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (ServerUi::expandServersToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::MinimizeAll).capitalized (), uitext->getText (UiTextString::ExpandAll).capitalized ());
	expandServersToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::ServerUiAttachedAgentsTitle), panel), StdString (""), ServerUi::AttachedServerToggleRow);

	cardView->setRowReverseSorted (ServerUi::ExpandedAttachedServerRow, true);

	cardView->setRowHeader (ServerUi::UnattachedServerRow, createRowHeaderPanel (uitext->getText (UiTextString::ServerUiUnattachedAgentsTitle)));

	cardView->setRowHeader (ServerUi::ServerPasswordsRow, createRowHeaderPanel (uitext->getText (UiTextString::ServerPasswords).capitalized ()));
	adminSecretWindow = new AdminSecretWindow ();
	adminSecretWindow->addButtonClickCallback = Widget::EventCallbackContext (ServerUi::adminSecretAddButtonClicked, this);
	adminSecretWindow->expandStateChangeCallback = Widget::EventCallbackContext (ServerUi::adminSecretExpandStateChanged, this);
	adminSecretWindow->setExpanded (false);
	cardView->addItem (adminSecretWindow, StdString (""), ServerUi::ServerPasswordsRow);

	return (Result::Success);
}

void ServerUi::doUnload () {
	addressToggle.clear ();
	addressTextFieldWindow.clear ();
	emptyServerWindow.clear ();
	expandServersToggle.clear ();
}

void ServerUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (ServerUi::reloadButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::ReloadTooltip));
	toolbar->addRightItem (button);
}

void ServerUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	Button *button;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);

	button = new Button (sprites.getSprite (ServerUi::BroadcastButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (ServerUi::broadcastButtonClicked, this);
	button->setInverseColor (true);
	button->shortcutKey = SDLK_F2;
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::ServerUiBroadcastTooltip));
	toolbar->addRightItem (button);

	toggle = new Toggle (sprites.getSprite (ServerUi::AddressButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::CancelButtonSprite));
	toggle->setInverseColor (true);
	toggle->shortcutKey = SDLK_F1;
	toggle->stateChangeCallback = Widget::EventCallbackContext (ServerUi::addressToggleStateChanged, this);
	toggle->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::ServerUiAddressTooltip));
	toolbar->addRightItem (toggle);

	addressToggle.destroyAndClear ();
	addressToggle.assign (toggle);
}

void ServerUi::doClearPopupWidgets () {

}

void ServerUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/ServerUi/bg");

	adminSecretWindow->readItems ();
	App::instance->shouldSyncRecordStore = true;

	resetExpandToggles ();
}

static void doPause_appendUnexpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
	ServerWindow *server;

	server = ServerWindow::castWidget (widgetPtr);
	if (server && (! server->isExpanded)) {
		((StringList *) stringListPtr)->push_back (server->agentId);
	}
}
void ServerUi::doPause () {
	HashMap *prefs;
	StringList items;

	items.clear ();
	cardView->processItems (doPause_appendUnexpandedAgentId, &items);
	prefs = App::instance->lockPrefs ();
	prefs->insert (ServerUi::UnexpandedAgentsKey, &items);
	App::instance->unlockPrefs ();
}

void ServerUi::doUpdate (int msElapsed) {
	AgentControl *agentcontrol;
	RecordStore *store;
	UiConfiguration *uiconfig;
	UiText *uitext;
	HashMap *prefs;
	StringList items;
	StringList::iterator i, end;
	StdString id;
	ServerWindow *server;
	ServerAttachWindow *attach;
	IconCardWindow *emptycard;
	int attachcount, unattachcount, row, pos;

	agentcontrol = &(App::instance->agentControl);
	store = &(App::instance->agentControl.recordStore);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	addressToggle.compact ();
	if (addressToggle.widget) {
		if (addressTextFieldWindow.widget && addressTextFieldWindow.widget->isDestroyed) {
			((Toggle *) addressToggle.widget)->setChecked (false);
			addressTextFieldWindow.clear ();
		}
	}
	emptyServerWindow.compact ();
	expandServersToggle.compact ();

	cardIdList.clear ();
	cardView->processItems (ServerUi::doUpdate_findDeletedWindows, this);
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

	attachcount = 0;
	unattachcount = 0;
	agentcontrol->getAgentIds (&agentIdList);
	i = agentIdList.begin ();
	end = agentIdList.end ();
	while (i != end) {
		if (agentcontrol->isAgentAttached (*i)) {
			++attachcount;
			id.assign (*i);
			if (! cardView->contains (id)) {
				server = new ServerWindow (id);
				server->expandStateChangeCallback = Widget::EventCallbackContext (ServerUi::serverExpandStateChanged, this);
				server->statusChangeCallback = Widget::EventCallbackContext (ServerUi::serverStatusChanged, this);
				server->checkForUpdatesClickCallback = Widget::EventCallbackContext (ServerUi::serverCheckForUpdatesActionClicked, this);
				server->adminClickCallback = Widget::EventCallbackContext (ServerUi::serverAdminActionClicked, this);
				server->detachClickCallback = Widget::EventCallbackContext (ServerUi::serverDetachActionClicked, this);
				server->removeClickCallback = Widget::EventCallbackContext (ServerUi::serverRemoveActionClicked, this);

				store->lock ();
				server->syncRecordStore ();
				store->unlock ();

				agentcontrol->checkAgentUpdates (id);

				prefs = App::instance->lockPrefs ();
				prefs->find (ServerUi::UnexpandedAgentsKey, &items);
				App::instance->unlockPrefs ();
				pos = items.indexOf (id);
				if (pos < 0) {
					row = ServerUi::ExpandedAttachedServerRow;
					server->setExpanded (true, true);
					server->sortKey.sprintf ("%016llx", (long long int) OsUtil::getTime ());
				}
				else {
					row = ServerUi::UnexpandedAttachedServerRow;
					server->sortKey.assign (server->agentDisplayName.lowercased ());
				}

				cardView->addItem (server, id, row);
				server->animateNewCard ();
			}
		}
		else {
			++unattachcount;
			id.sprintf ("%s_%i", (*i).c_str (), ServerUi::UnattachedServerRow);
			if (! cardView->contains (id)) {
				attach = new ServerAttachWindow (*i);
				attach->attachClickCallback = Widget::EventCallbackContext (ServerUi::serverAttachActionClicked, this);
				attach->removeClickCallback = Widget::EventCallbackContext (ServerUi::serverRemoveActionClicked, this);
				attach->refreshAgentData ();
				attach->sortKey.assign (attach->agentDisplayName.lowercased ());
				cardView->addItem (attach, id, ServerUi::UnattachedServerRow);
			}
		}
		++i;
	}

	if (attachedServerCount != attachcount) {
		resetExpandToggles ();
	}

	attachedServerCount = attachcount;
	unattachedServerCount = unattachcount;

	emptycard = (IconCardWindow *) emptyServerWindow.widget;
	if ((attachedServerCount <= 0) && (unattachedServerCount <= 0)) {
		if (! emptycard) {
			emptycard = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeErrorIconSprite), uitext->getText (UiTextString::ServerUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::ServerUiEmptyAgentStatusText1));
			emptycard->setLink (uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
			emptycard->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (emptycard, emptycard->itemId, ServerUi::UnexpandedAttachedServerRow);
			emptyServerWindow.assign (emptycard);
		}
	}
	else if (attachedServerCount <= 0) {
		if (! emptycard) {
			emptycard = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeErrorIconSprite), uitext->getText (UiTextString::ServerUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::ServerUiEmptyAgentStatusText2));
			emptycard->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (emptycard, emptycard->itemId, ServerUi::UnexpandedAttachedServerRow);
			emptyServerWindow.assign (emptycard);
		}
	}
	else {
		if (emptycard) {
			cardView->removeItem (emptycard->itemId);
			emptyServerWindow.clear ();
		}
	}
}

void ServerUi::doUpdate_findDeletedWindows (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerContactWindow *servercontactwindow;

	ui = (ServerUi *) uiPtr;
	servercontactwindow = ServerContactWindow::castWidget (widgetPtr);
	if (servercontactwindow) {
		if (servercontactwindow->isDeleted) {
			ui->cardIdList.push_back (servercontactwindow->itemId);
		}
	}
}

void ServerUi::doResize () {
	if (addressTextFieldWindow.widget) {
		((TextFieldWindow *) addressTextFieldWindow.widget)->setWindowWidth (App::instance->uiStack.secondaryToolbar->getLeftWidth ());
	}
}

void ServerUi::doSyncRecordStore () {
	cardView->syncRecordStore ();
	cardView->refresh ();
	resetExpandToggles ();
}

static void reloadButtonClicked_processItem (void *uiPtr, Widget *widgetPtr) {
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
void ServerUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;

	ui = (ServerUi *) uiPtr;
	ui->cardView->processItems (reloadButtonClicked_processItem, ui);
}

void ServerUi::broadcastButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App::instance->agentControl.broadcastContactMessage ();
	App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::SentBroadcast));
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
		textfield = new TextFieldWindow (toolbar->getLeftWidth (), App::instance->uiText.getText (UiTextString::EnterAddressPrompt), ui->sprites.getSprite (ServerUi::AddressIconSprite));
		textfield->setWindowHeight (toolbar->height);
		textfield->setButtonsEnabled (true, false, true, true);
		textfield->setFillBg (true, uiconfig->lightPrimaryColor);
		textfield->valueEditCallback = Widget::EventCallbackContext (ServerUi::addressTextFieldEdited, ui);
		textfield->shouldSkipTextClearCallbacks = true;
		textfield->assignKeyFocus ();
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
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokeGetStatusAddressEmptyError), App::instance->uiText.getText (UiTextString::Help).uppercased (), Widget::EventCallbackContext (ServerUi::addressSnackbarHelpClicked, ui));
		return;
	}

	if (! address.parseAddress (&hostname, &port, SystemInterface::Constant_DefaultTcpPort1)) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokeGetStatusAddressParseError), App::instance->uiText.getText (UiTextString::Help).uppercased (), Widget::EventCallbackContext (ServerUi::addressSnackbarHelpClicked, ui));
		return;
	}

	if (App::instance->agentControl.isHostContacted (hostname, port)) {
		App::instance->agentControl.refreshAgentStatus (hostname, port);
		return;
	}

	App::instance->agentControl.contactAgent (hostname, port);
	App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", App::instance->uiText.getText (UiTextString::ServerUiContactingAgentMessage).c_str (), address.c_str ()));

	key.sprintf ("%s:%i", hostname.c_str (), port);
	if (! ui->cardView->contains (key)) {
		window = new ServerContactWindow (address, hostname, port);
		window->stateChangeCallback = Widget::EventCallbackContext (ServerUi::serverContactWindowStateChanged, ui);
		window->sortKey.assign (address.lowercased ());
		window->itemId.assign (key);
		ui->cardView->addItem (window, window->itemId, ServerUi::ExpandedAttachedServerRow);
	}
	App::instance->shouldSyncRecordStore = true;
}

void ServerUi::addressSnackbarHelpClicked (void *uiPtr, Widget *widgetPtr) {
	StdString url;
	int result;

	url.assign (App::getHelpUrl ("servers"));
	result = OsUtil::openUrl (url);
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::OpenHelpUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::LaunchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void ServerUi::serverStatusChanged (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;

	ui = (ServerUi *) uiPtr;
	ui->cardView->refresh ();
}

void ServerUi::serverExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerWindow *server;

	ui = (ServerUi *) uiPtr;
	server = (ServerWindow *) widgetPtr;
	if (server->isExpanded) {
		server->sortKey.sprintf ("%016llx", (long long int) OsUtil::getTime ());
		ui->cardView->setItemRow (server->agentId, ServerUi::ExpandedAttachedServerRow);
	}
	else {
		server->sortKey.assign (server->agentDisplayName.lowercased ().c_str ());
		ui->cardView->setItemRow (server->agentId, ServerUi::UnexpandedAttachedServerRow);
	}
	server->resetInputState ();
	server->animateNewCard ();
	ui->resetExpandToggles ();
	ui->clearPopupWidgets ();
	ui->cardView->refresh ();
}

static void expandServersToggleStateChanged_appendAgentId (void *stringListPtr, Widget *widgetPtr) {
	ServerWindow *server;

	server = ServerWindow::castWidget (widgetPtr);
	if (server) {
		((StringList *) stringListPtr)->push_back (server->agentId);
	}
}
void ServerUi::expandServersToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	Toggle *toggle;
	ServerWindow *server;
	StringList idlist;
	StringList::iterator i, end;
	int64_t now;

	ui = (ServerUi *) uiPtr;
	toggle = (Toggle *) ui->expandServersToggle.widget;
	if (! toggle) {
		return;
	}

	now = OsUtil::getTime ();
	ui->cardView->processItems (expandServersToggleStateChanged_appendAgentId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		server = ServerWindow::castWidget (ui->cardView->getItem (*i));
		if (server) {
			if (toggle->isChecked) {
				server->setExpanded (false, true);
				server->sortKey.assign (server->agentDisplayName.lowercased ());
				ui->cardView->setItemRow (server->agentId, ServerUi::UnexpandedAttachedServerRow, true);
			}
			else {
				server->setExpanded (true, true);
				server->sortKey.sprintf ("%016llx%s", (long long int) now, server->agentDisplayName.lowercased ().c_str ());
				ui->cardView->setItemRow (server->agentId, ServerUi::ExpandedAttachedServerRow, true);
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

void ServerUi::serverAttachActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerAttachWindow *server;
	StdString agentid;

	ui = (ServerUi *) uiPtr;
	server = (ServerAttachWindow *) widgetPtr;
	agentid = server->agentId;

	App::instance->agentControl.setAgentAttached (agentid, true);
	ui->cardView->removeItem (agentid);
	ui->cardView->removeItem (StdString::createSprintf ("%s_%i", agentid.c_str (), ServerUi::UnattachedServerRow));
	ui->clearPopupWidgets ();
	App::instance->shouldSyncRecordStore = true;
}

void ServerUi::serverCheckForUpdatesActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerWindow *server;
	int result;

	ui = (ServerUi *) uiPtr;
	server = (ServerWindow *) widgetPtr;
	if (server->updateUrl.empty ()) {
		return;
	}

	ui->clearPopupWidgets ();
	result = OsUtil::openUrl (server->updateUrl);
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::OpenUpdateUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::LaunchedWebBrowser).capitalized ().c_str (), App::ServerUrl.c_str ()));
	}
}

void ServerUi::serverAdminActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerWindow *server;

	server = (ServerWindow *) widgetPtr;
	App::instance->uiStack.pushUi (new ServerAdminUi (server->agentId, server->agentDisplayName));
}

void ServerUi::serverDetachActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerWindow *server;
	StdString agentid;

	ui = (ServerUi *) uiPtr;
	server = (ServerWindow *) widgetPtr;
	agentid = server->agentId;

	App::instance->agentControl.setAgentAttached (agentid, false);
	ui->cardView->removeItem (agentid);
	ui->cardView->removeItem (StdString::createSprintf ("%s_%i", agentid.c_str (), ServerUi::UnattachedServerRow));
	ui->clearPopupWidgets ();
	App::instance->shouldSyncRecordStore = true;
}

void ServerUi::serverRemoveActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;
	ServerWindow *serverwindow;
	ServerAttachWindow *serverattachwindow;
	Widget::Rectangle rect;

	ui = (ServerUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	if (ui->clearActionPopup (widgetPtr, ServerUi::serverRemoveActionClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->closeCallback = Widget::EventCallbackContext (ServerUi::serverRemoveActionClosed, ui);
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	action->setConfirmTooltipText (uitext->getText (UiTextString::RemoveServer).capitalized ());

	serverwindow = ServerWindow::castWidget (widgetPtr);
	if (serverwindow) {
		action->setTitleText (serverwindow->agentDisplayName);
		action->setDescriptionText (uitext->getText (UiTextString::RemoveAttachedServerDescription));
		rect = serverwindow->getRemoveButtonScreenRect ();
	}
	else {
		serverattachwindow = ServerAttachWindow::castWidget (widgetPtr);
		if (serverattachwindow) {
			action->setTitleText (serverattachwindow->agentDisplayName);
			action->setDescriptionText (uitext->getText (UiTextString::RemoveUnattachedServerDescription));
			rect = serverattachwindow->getRemoveButtonScreenRect ();
		}
	}

	ui->showActionPopup (action, widgetPtr, ServerUi::serverRemoveActionClicked, rect, Ui::RightOfAlignment, Ui::YCenteredAlignment);
}

void ServerUi::serverRemoveActionClosed (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ActionWindow *action;
	Widget *target;
	StdString agentid;

	ui = (ServerUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	target = ui->actionTarget.widget;
	if (ServerWindow::isWidgetType (target)) {
		agentid = ((ServerWindow *) target)->agentId;
	}
	else if (ServerAttachWindow::isWidgetType (target)) {
		agentid = ((ServerAttachWindow *) target)->agentId;
	}
	if ((! action->isConfirmed) || agentid.empty ()) {
		ui->clearPopupWidgets ();
		return;
	}

	App::instance->agentControl.removeAgent (agentid);
	ui->cardView->removeItem (agentid);
	ui->cardView->removeItem (StdString::createSprintf ("%s_%i", agentid.c_str (), ServerUi::UnattachedServerRow));
	ui->clearPopupWidgets ();
	App::instance->shouldSyncRecordStore = true;
}

void ServerUi::adminSecretAddButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	AdminSecretWindow *target;
	ActionWindow *action;

	ui = (ServerUi *) uiPtr;
	if (ui->clearActionPopup (widgetPtr, ServerUi::adminSecretAddButtonClicked)) {
		return;
	}

	target = (AdminSecretWindow *) widgetPtr;
	action = target->createAddActionWindow ();
	action->closeCallback = Widget::EventCallbackContext (ServerUi::adminSecretAddActionClosed, ui);

	ui->showActionPopup (action, widgetPtr, ServerUi::adminSecretAddButtonClicked, widgetPtr->getScreenRect (), Ui::RightOfAlignment, Ui::BottomEdgeAlignment);
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
	ui->clearPopupWidgets ();
	ui->cardView->refresh ();
}

void ServerUi::serverContactWindowStateChanged (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;

	ui = (ServerUi *) uiPtr;
	ui->cardView->refresh ();
}

static void resetExpandToggles_countExpandedServers (void *intPtr, Widget *widgetPtr) {
	ServerWindow *server;
	int *count;

	server = ServerWindow::castWidget (widgetPtr);
	count = (int *) intPtr;
	if (server && count && server->isExpanded) {
		++(*count);
	}
}
void ServerUi::resetExpandToggles () {
	Toggle *toggle;
	int count;

	toggle = (Toggle *) expandServersToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (resetExpandToggles_countExpandedServers, &count);
		toggle->setChecked ((count <= 0), true);
	}
}
