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
#include "UiLaunchWindow.h"
#include "HelpWindow.h"
#include "LinkUi.h"
#include "MediaUi.h"
#include "DisplayUi.h"
#include "WebKioskUi.h"
#include "MainUi.h"

const int MainUi::uiLaunchWindowTypes[] = {
	UiLaunchWindow::LINK_UI,
	UiLaunchWindow::WEB_KIOSK_UI,
	UiLaunchWindow::MEDIA_UI,
	UiLaunchWindow::DISPLAY_UI
};

MainUi::MainUi ()
: Ui ()
, cardView (NULL)
, showAllToggle (NULL)
, welcomeClock (0)
, shouldResetShowAll (false)
, readyItemCount (0)
{

}

MainUi::~MainUi () {

}

StdString MainUi::getSpritePath () {
	return (StdString ("ui/MainUi/sprite"));
}

void MainUi::setHelpWindowContent (Widget *helpWindowPtr) {
	App *app;
	HelpWindow *help;
	UiText *uitext;

	app = App::getInstance ();
	uitext = &(app->uiText);
	help = (HelpWindow *) helpWindowPtr;

	help->setHelpText (uitext->mainUiHelpTitle, uitext->mainUiHelpText);
	if (readyItemCount <= 0) {
		help->addAction (uitext->mainUiServersHelpActionText, uitext->learnMore.capitalized (), Util::getHelpUrl ("servers"));
	}

	help->addTopicLink (uitext->membraneSoftwareOverview, Util::getHelpUrl ("membrane-software-overview"));
	help->addTopicLink (uitext->searchForHelp, Util::getHelpUrl (""));
}

int MainUi::doLoad () {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (0, uitext->mainMenu.capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, app->topBarHeight);

	showAllToggle = (Toggle *) addWidget (new Toggle (sprites.getSprite (MainUi::SHOW_ALL_DISABLED_BUTTON), sprites.getSprite (MainUi::SHOW_ALL_ENABLED_BUTTON)));
	showAllToggle->setChecked (app->prefsMap.find (App::prefsMainUiShowAllEnabled, false));
	showAllToggle->setStateChangeCallback (MainUi::showAllToggleStateChanged, this);
	showAllToggle->position.assign (app->windowWidth - showAllToggle->width - uiconfig->marginSize - app->rightBarWidth, app->windowHeight - showAllToggle->height - uiconfig->marginSize);
	showAllToggle->setMouseHoverTooltip (uitext->mainUiShowAllTooltip);

	if (! app->prefsMap.find (App::prefsIsFirstLaunchComplete, false)) {
		app->prefsMap.insert (App::prefsIsFirstLaunchComplete, true);
		welcomeClock = 2400;
	}

	shouldResetShowAll = true;
	app->shouldSyncRecordStore = true;

	return (Result::SUCCESS);
}

void MainUi::doUnload () {

}

void MainUi::doResetMainToolbar (Toolbar *toolbar) {

}

void MainUi::doClearPopupWidgets () {

}

void MainUi::doResume () {
	App *app;
	StringList addresses;
	StringList::iterator i, end;

	// TODO: Gather AgentStatus records here (for correct display of agent counts)

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/MainUi/bg");

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);

	if (! isFirstResumeComplete) {
		app->prefsMap.find (App::prefsAutoConnectAddresses, &addresses);
		i = addresses.begin ();
		end = addresses.end ();
		while (i != end) {
			if (! i->empty ()) {
				app->agentControl.connectLinkClientToAddress (*i);
			}
			++i;
		}
	}
}

void MainUi::doRefresh () {
	App *app;
	UiConfiguration *uiconfig;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);

	showAllToggle->position.assign (app->windowWidth - showAllToggle->width - uiconfig->marginSize - app->rightBarWidth, app->windowHeight - showAllToggle->height - uiconfig->marginSize);
}

void MainUi::doPause () {
	App *app;

	app = App::getInstance ();
	app->prefsMap.insert (App::prefsMainUiShowAllEnabled, showAllToggle->isChecked);
	welcomeClock = 0;
}

void MainUi::doUpdate (int msElapsed) {
	App *app;

	if (welcomeClock > 0) {
		welcomeClock -= msElapsed;
		if (welcomeClock <= 0) {
			welcomeClock = 0;
			app = App::getInstance ();
			app->showSnackbar (app->uiText.mainUiWelcomeSnackbarText, app->uiText.openHelp.uppercased (), Ui::helpActionClicked, this);
		}
	}
}

void MainUi::doResize () {
	App *app;
	UiConfiguration *uiconfig;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
	showAllToggle->position.assign (app->windowWidth - showAllToggle->width - uiconfig->marginSize - app->rightBarWidth, app->windowHeight - showAllToggle->height - uiconfig->marginSize);
}

void MainUi::handleLinkClientConnect (const StdString &agentId) {
	App *app;
	Json *params;

	app = App::getInstance ();
	params = new Json ();
	params->set ("commandId", SystemInterface::Command_MediaItem);
	app->agentControl.writeLinkCommand (app->createCommandJson ("ReadEvents", SystemInterface::Constant_Link, params), agentId);

	params = new Json ();
	params->set ("commandId", SystemInterface::Command_StreamItem);
	app->agentControl.writeLinkCommand (app->createCommandJson ("ReadEvents", SystemInterface::Constant_Link, params), agentId);

	params = new Json ();
	params->set ("commandId", SystemInterface::Command_IntentState);
	app->agentControl.writeLinkCommand (app->createCommandJson ("ReadEvents", SystemInterface::Constant_Link, params), agentId);
}

void MainUi::doSyncRecordStore (RecordStore *store) {
	UiLaunchWindow *item;
	int i, len, type, readycount;
	bool skip;

	len = sizeof (MainUi::uiLaunchWindowTypes) / sizeof (MainUi::uiLaunchWindowTypes[0]);
	readycount = 0;
	for (i = 0; i < len; ++i) {
		type = MainUi::uiLaunchWindowTypes[i];
		if (UiLaunchWindow::isReadyState (type, store)) {
			++readycount;
			if (! cardView->findItem (MainUi::matchUiType, &type)) {
				shouldResetShowAll = true;
			}
		}
	}
	readyItemCount = readycount;

	if (shouldResetShowAll) {
		shouldResetShowAll = false;
		cardView->removeAllItems ();

		for (i = 0; i < len; ++i) {
			type = MainUi::uiLaunchWindowTypes[i];
			skip = false;
			if (! showAllToggle->isChecked) {
				if ((type != UiLaunchWindow::LINK_UI) && (! UiLaunchWindow::isReadyState (type, store))) {
					skip = true;
				}
			}

			if (! skip) {
				item = new UiLaunchWindow (type, &sprites);
				item->itemId.assign (cardView->getAvailableItemId ());
				item->setOpenCallback (MainUi::uiOpenClicked, this);
				cardView->addItem (item, item->itemId, 0, true);
			}
		}
	}

	cardView->syncRecordStore (store);
	cardView->refresh ();
}

bool MainUi::matchUiType (void *intPtr, Widget *widgetPtr) {
	UiLaunchWindow *item;
	int *type;

	item = UiLaunchWindow::castWidget (widgetPtr);
	if (! item) {
		return (false);
	}

	type = (int *) intPtr;
	return (item->uiType == *type);
}

void MainUi::uiOpenClicked (void *uiPtr, Widget *widgetPtr) {
	UiLaunchWindow *window;
	Ui *ui;

	window = (UiLaunchWindow *) widgetPtr;
	ui = NULL;
	switch (window->uiType) {
		case UiLaunchWindow::LINK_UI: {
			ui = (Ui *) new LinkUi ();
			break;
		}
		case UiLaunchWindow::MEDIA_UI: {
			ui = (Ui *) new MediaUi ();
			break;
		}
		case UiLaunchWindow::DISPLAY_UI: {
			ui = (Ui *) new DisplayUi ();
			break;
		}
		case UiLaunchWindow::WEB_KIOSK_UI: {
			ui = (Ui *) new WebKioskUi ();
			break;
		}
	}

	if (ui) {
		App::getInstance ()->pushUi (ui);
	}
}

void MainUi::showAllToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	MainUi *ui;
	App *app;

	ui = (MainUi *) uiPtr;
	app = App::getInstance ();
	ui->shouldResetShowAll = true;
	app->shouldSyncRecordStore = true;
}
