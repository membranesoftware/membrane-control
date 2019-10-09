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

ServerUi::ServerUi ()
: Ui ()
, cardView (NULL)
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
	return (new Chip (App::instance->uiText.getText (UiTextString::servers).capitalized (), sprites.getSprite (ServerUi::BreadcrumbIconSprite)));
}

void ServerUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);

	helpWindow->setHelpText (uitext->getText (UiTextString::serverUiHelpTitle), uitext->getText (UiTextString::serverUiHelpText));
	if ((attachedServerCount <= 0) && (unattachedServerCount <= 0)) {
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
	Panel *panel;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	panel = new Panel ();
	panel->setFillBg (true, uiconfig->mediumBackgroundColor);
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->setImageColor (uiconfig->flatButtonTextColor);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::minimizeAll).capitalized (), uitext->getText (UiTextString::expandAll).capitalized ());
	toggle->setStateChangeCallback (ServerUi::expandServersToggleStateChanged, this);
	expandServersToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::serverUiAttachedAgentsTitle), panel), StdString (""), ServerUi::AttachedServerToggleRow);

	cardView->setRowReverseSorted (ServerUi::ExpandedAttachedServerRow, true);

	cardView->setRowHeader (ServerUi::UnattachedServerRow, createRowHeaderPanel (uitext->getText (UiTextString::serverUiUnattachedAgentsTitle)));

	adminSecretWindow = new AdminSecretWindow ();
	adminSecretWindow->setAddButtonClickCallback (ServerUi::adminSecretAddButtonClicked, this);
	adminSecretWindow->setExpandStateChangeCallback (ServerUi::adminSecretExpandStateChanged, this);
	adminSecretWindow->setExpanded (false);
	cardView->addItem (adminSecretWindow, StdString (""), ServerUi::ActionRow);

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

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
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

	button = new Button (StdString (""), sprites.getSprite (ServerUi::BroadcastButtonSprite));
	button->setInverseColor (true);
	button->shortcutKey = SDLK_F2;
	button->setMouseClickCallback (ServerUi::broadcastButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::serverUiBroadcastTooltip));
	toolbar->addRightItem (button);

	toggle = new Toggle (sprites.getSprite (ServerUi::AddressButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::CancelButtonSprite));
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
	App::instance->shouldSyncRecordStore = true;

	resetExpandToggles ();
}

void ServerUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void ServerUi::doPause () {
	StringList items;

	items.clear ();
	cardView->processItems (ServerUi::appendUnexpandedAgentId, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::ServerUiUnexpandedAgentsKey);
	}
	else {
		App::instance->prefsMap.insert (App::ServerUiUnexpandedAgentsKey, &items);
	}
}

void ServerUi::appendUnexpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
	ServerWindow *server;

	server = ServerWindow::castWidget (widgetPtr);
	if (server && (! server->isExpanded)) {
		((StringList *) stringListPtr)->push_back (server->agentId);
	}
}

void ServerUi::doUpdate (int msElapsed) {
	AgentControl *agentcontrol;
	RecordStore *store;
	UiConfiguration *uiconfig;
	UiText *uitext;
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
				server->setExpandStateChangeCallback (ServerUi::serverExpandStateChanged, this);
				server->setStatusChangeCallback (ServerUi::serverStatusChanged, this);
				server->setCheckForUpdatesClickCallback (ServerUi::serverCheckForUpdatesActionClicked, this);
				server->setAdminClickCallback (ServerUi::serverAdminActionClicked, this);
				server->setDetachClickCallback (ServerUi::serverDetachActionClicked, this);
				server->setRemoveClickCallback (ServerUi::serverRemoveActionClicked, this);

				store->lock ();
				server->syncRecordStore ();
				store->unlock ();

				App::instance->prefsMap.find (App::ServerUiUnexpandedAgentsKey, &items);
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
				attach->setAttachClickCallback (ServerUi::serverAttachActionClicked, this);
				attach->setRemoveClickCallback (ServerUi::serverRemoveActionClicked, this);
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
			emptycard = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeErrorIconSprite), uitext->getText (UiTextString::serverUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::serverUiEmptyAgentStatusText1));
			emptycard->setLink (uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
			emptycard->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (emptycard, emptycard->itemId, ServerUi::UnexpandedAttachedServerRow);
			emptyServerWindow.assign (emptycard);
		}
	}
	else if (attachedServerCount <= 0) {
		if (! emptycard) {
			emptycard = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeErrorIconSprite), uitext->getText (UiTextString::serverUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::serverUiEmptyAgentStatusText2));
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

void ServerUi::findDeletedWindows (void *uiPtr, Widget *widgetPtr) {
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
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	if (addressTextFieldWindow.widget) {
		((TextFieldWindow *) addressTextFieldWindow.widget)->setWindowWidth (App::instance->uiStack.secondaryToolbar->getLeftWidth ());
	}
}

void ServerUi::doSyncRecordStore () {
	cardView->syncRecordStore ();
	cardView->refresh ();
	resetExpandToggles ();
}

void ServerUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;

	ui = (ServerUi *) uiPtr;
	ui->cardView->processItems (ServerUi::reloadAgent, ui);
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
		textfield = new TextFieldWindow (toolbar->getLeftWidth (), App::instance->uiText.getText (UiTextString::enterAddressPrompt), ui->sprites.getSprite (ServerUi::AddressIconSprite));
		textfield->setWindowHeight (toolbar->height);
		textfield->setButtonsEnabled (true, false, true, true);
		textfield->setFillBg (true, uiconfig->lightPrimaryColor);
		textfield->setEditCallback (ServerUi::addressTextFieldEdited, ui);
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
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeGetStatusAddressEmptyError), App::instance->uiText.getText (UiTextString::help).uppercased (), ServerUi::addressSnackbarHelpClicked, ui);
		return;
	}

	if (! address.parseAddress (&hostname, &port, SystemInterface::Constant_DefaultTcpPort1)) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeGetStatusAddressParseError), App::instance->uiText.getText (UiTextString::help).uppercased (), ServerUi::addressSnackbarHelpClicked, ui);
		return;
	}

	if (App::instance->agentControl.isHostContacted (hostname, port)) {
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
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openHelpUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
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
	ui->cardView->refresh ();
	ui->resetExpandToggles ();
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
	ui->cardView->processItems (ServerUi::appendAgentId, &idlist);
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

void ServerUi::appendAgentId (void *stringListPtr, Widget *widgetPtr) {
	ServerWindow *server;

	server = ServerWindow::castWidget (widgetPtr);
	if (server) {
		((StringList *) stringListPtr)->push_back (server->agentId);
	}
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
	App::instance->shouldSyncRecordStore = true;
}

void ServerUi::serverCheckForUpdatesActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	ServerWindow *server;
	StdString url;
	int result;

	ui = (ServerUi *) uiPtr;
	server = (ServerWindow *) widgetPtr;

	ui->clearPopupWidgets ();
	url.assign (App::getUpdateUrl (server->applicationId));
	result = OsUtil::openUrl (url);
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openUpdateUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
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
	App::instance->shouldSyncRecordStore = true;
}

void ServerUi::serverRemoveActionClicked (void *uiPtr, Widget *widgetPtr) {
	ServerUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;
	bool show;
	float x, y;

	ui = (ServerUi *) uiPtr;
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

	action = (ActionWindow *) App::instance->rootPanel->addWidget (new ActionWindow ());
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	if (ServerWindow::isWidgetType (widgetPtr)) {
		action->setTitleText (((ServerWindow *) widgetPtr)->agentDisplayName);
		action->setDescriptionText (uitext->getText (UiTextString::removeAttachedServerDescription));
	}
	else if (ServerAttachWindow::isWidgetType (widgetPtr)) {
		action->setTitleText (((ServerAttachWindow *) widgetPtr)->agentDisplayName);
		action->setDescriptionText (uitext->getText (UiTextString::removeUnattachedServerDescription));
	}
	action->setConfirmButtonText (uitext->getText (UiTextString::removeServer).uppercased ());
	action->setCloseCallback (ServerUi::serverRemoveActionClosed, ui);

	x = widgetPtr->screenX + widgetPtr->width;
	if ((x + action->width) >= (float) App::instance->windowWidth) {
		x = widgetPtr->screenX + widgetPtr->width - action->width;
		y = widgetPtr->screenY + widgetPtr->height;
	}
	else {
		y = widgetPtr->screenY + widgetPtr->height - action->height;
	}
	action->position.assign (x, y);

	ui->actionWidget.assign (action);
	ui->actionTarget.assign (widgetPtr);
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
	action->position.assign (target->screenX + target->width, target->screenY + target->height - action->height);
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

void ServerUi::resetExpandToggles () {
	Toggle *toggle;
	int count;

	toggle = (Toggle *) expandServersToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (ServerUi::countExpandedServers, &count);
		toggle->setChecked ((count <= 0), true);
	}
}

void ServerUi::countExpandedServers (void *intPtr, Widget *widgetPtr) {
	ServerWindow *server;
	int *count;

	server = ServerWindow::castWidget (widgetPtr);
	count = (int *) intPtr;
	if (server && count && server->isExpanded) {
		++(*count);
	}
}
