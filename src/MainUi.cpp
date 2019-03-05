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
#include <map>
#include <vector>
#include <stdlib.h>
#include "Result.h"
#include "StdString.h"
#include "Log.h"
#include "App.h"
#include "OsUtil.h"
#include "SequenceList.h"
#include "Ui.h"
#include "UiLaunchWindow.h"
#include "RecordStore.h"
#include "CardView.h"
#include "Toggle.h"
#include "HelpWindow.h"
#include "BannerWindow.h"
#include "ServerUi.h"
#include "WebKioskUi.h"
#include "MediaUi.h"
#include "MonitorUi.h"
#include "MainUi.h"

const int MainUi::uiLaunchWindowTypes[] = {
	UiLaunchWindow::SERVER_UI,
	UiLaunchWindow::WEB_KIOSK_UI,
	UiLaunchWindow::MEDIA_UI,
	UiLaunchWindow::MONITOR_UI
};
const StdString MainUi::announcementIconType = StdString ("a");
const StdString MainUi::updateIconType = StdString ("b");
const StdString MainUi::textMessageIconType = StdString ("c");
const StdString MainUi::videoMessageIconType = StdString ("d");
const StdString MainUi::openUrlActionType = StdString ("a");
const StdString MainUi::helpActionType = StdString ("b");

MainUi::MainUi ()
: Ui ()
, cardView (NULL)
, shouldResetShowAll (false)
, readyItemCount (0)
, bannerClock (0)
{

}

MainUi::~MainUi () {

}

StdString MainUi::getSpritePath () {
	return (StdString ("ui/MainUi/sprite"));
}

void MainUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);

	helpWindow->setHelpText (uitext->getText (UiTextString::mainUiHelpTitle), uitext->getText (UiTextString::mainUiHelpText));
	if (readyItemCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::mainUiServersHelpActionText), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::membraneSoftwareOverviewHelpTitle), App::getHelpUrl ("membrane-software-overview"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

int MainUi::doLoad () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (0, uitext->getText (UiTextString::mainMenu).capitalized (), UiConfiguration::TITLE, uiconfig->inverseTextColor);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	bannerList.randomizeOrder (&(App::instance->prng));
	bannerIconTypeMap.insert (MainUi::announcementIconType, MainUi::ANNOUNCEMENT_ICON);
	bannerIconTypeMap.insert (MainUi::updateIconType, MainUi::UPDATE_ICON);
	bannerIconTypeMap.insert (MainUi::textMessageIconType, MainUi::TEXT_MESSAGE_ICON);
	bannerIconTypeMap.insert (MainUi::videoMessageIconType, MainUi::VIDEO_MESSAGE_ICON);
	bannerActionCallbackMap.insert (std::pair<StdString, Widget::EventCallback> (MainUi::openUrlActionType, MainUi::openUrlActionClicked));
	bannerActionCallbackMap.insert (std::pair<StdString, Widget::EventCallback> (MainUi::helpActionType, MainUi::helpActionClicked));
	bannerClock = 1200;
	resetBanners ();

	shouldResetShowAll = true;
	App::instance->shouldSyncRecordStore = true;

	return (Result::SUCCESS);
}

void MainUi::doUnload () {
	showAllToggle.clear ();
	bannerWindow.clear ();
	bannerActionButton.clear ();
}

void MainUi::doAddMainToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	UiText *uitext;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	toggle = new Toggle (sprites.getSprite (MainUi::SHOW_ALL_DISABLED_BUTTON), sprites.getSprite (MainUi::SHOW_ALL_ENABLED_BUTTON));
	toggle->setInverseColor (true);
	toggle->setChecked (App::instance->prefsMap.find (App::prefsMainUiShowAllEnabled, false));
	toggle->setStateChangeCallback (MainUi::showAllToggleStateChanged, this);
	toggle->position.assign (App::instance->windowWidth - toggle->width - uiconfig->marginSize - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - toggle->height - uiconfig->marginSize);
	toggle->setMouseHoverTooltip (uitext->getText (UiTextString::mainUiShowAllTooltip));
	showAllToggle.assign (toggle);
	toolbar->addRightItem (toggle);
}

void MainUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiText *uitext;
	Button *button;
	BannerWindow *banner;

	uitext = &(App::instance->uiText);

	button = new Button (StdString (""), sprites.getSprite (MainUi::NEXT_ITEM_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (MainUi::nextBannerButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::mainUiNextBannerTooltip));
	button->zLevel = 1;
	toolbar->addRightItem (button);

	banner = new BannerWindow (toolbar->getLeftWidth ());
	bannerWindow.assign (banner);
	toolbar->addLeftItem (banner);
}

void MainUi::doClearPopupWidgets () {

}

void MainUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/MainUi/bg");
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	bannerClock = 1200;

	if (! isFirstResumeComplete) {
		if (! App::instance->prefsMap.find (App::prefsIsFirstLaunchComplete, false)) {
			App::instance->prefsMap.insert (App::prefsIsFirstLaunchComplete, true);
		}
		else {
			App::instance->network.sendHttpGet (App::getApplicationNewsUrl (), MainUi::getApplicationNewsComplete, this);
		}
	}
}

void MainUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MainUi::doPause () {
	Toggle *toggle;

	toggle = (Toggle *) showAllToggle.widget;
	if (toggle) {
		App::instance->prefsMap.insert (App::prefsMainUiShowAllEnabled, toggle->isChecked);
	}
}

void MainUi::doUpdate (int msElapsed) {
	showAllToggle.compact ();
	bannerWindow.compact ();
	bannerActionButton.compact ();

	if (bannerClock > 0) {
		bannerClock -= msElapsed;
		if (bannerClock <= 0) {
			showNextBanner ();
		}
	}
}

void MainUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	if (bannerWindow.widget) {
		((BannerWindow *) bannerWindow.widget)->setWindowWidth (App::instance->uiStack.secondaryToolbar->getLeftWidth ());
	}
}

void MainUi::doSyncRecordStore () {
	RecordStore *store;
	UiLaunchWindow *item;
	Toggle *toggle;
	int i, len, type, readycount;
	bool skip;

	store = &(App::instance->agentControl.recordStore);
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

		toggle = (Toggle *) showAllToggle.widget;
		for (i = 0; i < len; ++i) {
			type = MainUi::uiLaunchWindowTypes[i];
			skip = false;
			if (toggle && (! toggle->isChecked)) {
				if ((type != UiLaunchWindow::SERVER_UI) && (! UiLaunchWindow::isReadyState (type, store))) {
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

	cardView->syncRecordStore ();
	cardView->refresh ();
}

void MainUi::helpActionClicked (void *uiPtr, Widget *widgetPtr) {
	App::instance->uiStack.toggleHelpWindow ();
}

void MainUi::uiOpenClicked (void *uiPtr, Widget *widgetPtr) {
	UiLaunchWindow *window;
	Ui *ui;

	window = (UiLaunchWindow *) widgetPtr;
	ui = NULL;
	switch (window->uiType) {
		case UiLaunchWindow::SERVER_UI: {
			ui = (Ui *) new ServerUi ();
			break;
		}
		case UiLaunchWindow::MEDIA_UI: {
			ui = (Ui *) new MediaUi ();
			break;
		}
		case UiLaunchWindow::MONITOR_UI: {
			ui = (Ui *) new MonitorUi ();
			break;
		}
		case UiLaunchWindow::WEB_KIOSK_UI: {
			ui = (Ui *) new WebKioskUi ();
			break;
		}
	}

	if (ui) {
		App::instance->uiStack.pushUi (ui);
	}
}

void MainUi::showAllToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	MainUi *ui;

	ui = (MainUi *) uiPtr;
	ui->shouldResetShowAll = true;
	App::instance->shouldSyncRecordStore = true;
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

void MainUi::showNextBanner () {
	UiConfiguration *uiconfig;
	BannerWindow *banner;
	Button *button;
	std::map<StdString, Widget::EventCallback>::iterator pos;
	Widget::EventCallback callback;
	int spriteindex;

	banner = (BannerWindow *) bannerWindow.widget;
	if ((! banner) || bannerList.empty ()) {
		return;
	}
	if (! bannerList.next (&activeBanner)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	spriteindex = bannerIconTypeMap.find (activeBanner.iconType, (int) -1);
	bannerActionButton.destroyAndClear ();
	if (! activeBanner.actionText.empty ()) {
		pos = bannerActionCallbackMap.find (activeBanner.actionType);
		if (pos != bannerActionCallbackMap.end ()) {
			callback = pos->second;
			if (callback) {
				button = new Button (activeBanner.actionText.uppercased ());
				button->setRaised (true, uiconfig->raisedButtonBackgroundColor);
				button->setTextColor (uiconfig->raisedButtonTextColor);
				button->setMouseClickCallback (callback, this);
				button->zLevel = 1;
				bannerActionButton.assign (button);
				App::instance->uiStack.secondaryToolbar->addRightItem (button);
			}
		}
	}

	banner->setWindowWidth (App::instance->uiStack.secondaryToolbar->getLeftWidth () - (uiconfig->marginSize * 2.0f));
	banner->setBanner (activeBanner.messageText, sprites.getSprite (spriteindex));
	if (activeBanner.actionType.equals (MainUi::helpActionType)) {
		bannerClock = 300000;
	}
	else {
		bannerClock = App::instance->getRandomInt (75000, 90000);
	}
}

void MainUi::getApplicationNewsComplete (void *uiPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	MainUi *ui;
	SystemInterface *interface;
	Json *cmd, *item;
	StringList items;
	int i, count;

	if ((statusCode != Network::HTTP_OK) || (responseData->length <= 0)) {
		return;
	}
	if (! App::instance->systemInterface.parseCommand (StdString ((char *) responseData->data, responseData->length), &cmd)) {
		return;
	}

	ui = (MainUi *) uiPtr;
	interface = &(App::instance->systemInterface);
	count = interface->getCommandArrayLength (cmd, "items");
	for (i = 0; i < count; ++i) {
		item = new Json ();
		if (interface->getCommandObjectArrayItem (cmd, "items", i, item)) {
			items.push_back (item->toString ());
		}
		delete (item);
	}
	delete (cmd);

	App::instance->prefsMap.insert (App::prefsMainUiApplicationNewsItems, &items);
	ui->resetBanners ();
}

void MainUi::resetBanners () {
	UiText *uitext;
	MainUi::Banner item;
	StringList items;
	StringList::iterator i, end;
	Json *json;

	uitext = &(App::instance->uiText);
	bannerList.clear ();
	if (! App::instance->prefsMap.find (App::prefsIsFirstLaunchComplete, false)) {
		item = MainUi::Banner ();
		item.messageText.assign (uitext->getText (UiTextString::firstLaunchBannerMessage));
		item.iconType.assign (MainUi::announcementIconType);
		item.actionType.assign (MainUi::helpActionType);
		item.actionText.assign (uitext->getText (UiTextString::openHelp));
		bannerList.push_back (item);
		bannerList.nextItemIndex = 0;
	}
	else {
		item = MainUi::Banner ();
		item.messageText.assign (uitext->getText (UiTextString::helpKeyBannerMessage));
		item.iconType.assign (MainUi::textMessageIconType);
		bannerList.push_back (item);
	}

	App::instance->prefsMap.find (App::prefsMainUiApplicationNewsItems, &items);
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		json = new Json ();
		if (json->parse (*i) == Result::SUCCESS) {
			item = MainUi::Banner ();
			item.messageText.assign (json->getString ("message", ""));
			item.iconType.assign (json->getString ("iconType", ""));
			item.actionText.assign (json->getString ("actionText", ""));
			item.actionType.assign (json->getString ("actionType", ""));
			item.actionTarget.assign (json->getString ("actionTarget", ""));
			if (! item.messageText.empty ()) {
				bannerList.push_back (item);
			}
		}
		delete (json);
		++i;
	}

	item = MainUi::Banner ();
	item.messageText.assign (uitext->getText (UiTextString::freeApplicationBannerMessage));
	item.iconType.assign (MainUi::textMessageIconType);
	bannerList.push_back (item);
	item = MainUi::Banner ();
	item.messageText.assign (uitext->getText (UiTextString::membraneSoftwareBannerMessage));
	item.iconType.assign (MainUi::textMessageIconType);
	item.actionType.assign (MainUi::openUrlActionType);
	item.actionText.assign (uitext->getText (UiTextString::learnMore));
	item.actionTarget.assign (App::getHelpUrl ("about-membrane-software"));
	bannerList.push_back (item);
	item = MainUi::Banner ();
	item.messageText.assign (uitext->getText (UiTextString::donateBannerMessage));
	item.iconType.assign (MainUi::announcementIconType);
	item.actionType.assign (MainUi::openUrlActionType);
	item.actionText.assign (uitext->getText (UiTextString::learnMore));
	item.actionTarget.assign (App::getDonateUrl ());
	bannerList.push_back (item);

	bannerClock = 1200;
}

void MainUi::nextBannerButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MainUi *ui;

	ui = (MainUi *) uiPtr;
	ui->showNextBanner ();
}

void MainUi::openUrlActionClicked (void *uiPtr, Widget *widgetPtr) {
	MainUi *ui;
	StdString url;
	int result;

	ui = (MainUi *) uiPtr;
	url.assign (ui->activeBanner.actionTarget);
	if (url.empty ()) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::launchWebBrowserError));
		return;
	}

	result = OsUtil::openUrl (url);
	if (result != Result::SUCCESS) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::launchWebBrowserError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}
