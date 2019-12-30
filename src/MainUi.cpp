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
#include <map>
#include <list>
#include <vector>
#include "Result.h"
#include "StdString.h"
#include "StringList.h"
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
#include "CameraUi.h"
#include "MediaUi.h"
#include "CommandUi.h"
#include "MainUi.h"

const int MainUi::uiLaunchWindowTypes[] = {
	UiLaunchWindow::ServerUi,
	UiLaunchWindow::MediaUi,
	UiLaunchWindow::WebKioskUi,
	UiLaunchWindow::CameraUi,
	UiLaunchWindow::CommandUi
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
, bannerClock (0)
, agentCount (0)
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
	if (agentCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::mainUiServersHelpActionText), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::membraneSoftwareOverviewHelpTitle), App::getHelpUrl ("membrane-software-overview"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

int MainUi::doLoad () {
	UiText *uitext;

	uitext = &(App::instance->uiText);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	cardView->setRowHeader (MainUi::UnexpandedUiRow, createRowHeaderPanel (uitext->getText (UiTextString::mainMenu).capitalized ()));

	cardView->setRowReverseSorted (MainUi::ExpandedUiRow, true);

	bannerList.randomizeOrder (&(App::instance->prng));
	bannerIconTypeMap.insert (MainUi::announcementIconType, MainUi::AnnouncementIconSprite);
	bannerIconTypeMap.insert (MainUi::updateIconType, MainUi::UpdateIconSprite);
	bannerIconTypeMap.insert (MainUi::textMessageIconType, MainUi::TextMessageIconSprite);
	bannerIconTypeMap.insert (MainUi::videoMessageIconType, MainUi::VideoMessageIconSprite);
	bannerActionCallbackMap.insert (std::pair<StdString, Widget::EventCallback> (MainUi::openUrlActionType, MainUi::openUrlActionClicked));
	bannerActionCallbackMap.insert (std::pair<StdString, Widget::EventCallback> (MainUi::helpActionType, MainUi::helpActionClicked));
	bannerClock = 1200;
	resetBanners ();

	App::instance->shouldSyncRecordStore = true;

	return (Result::Success);
}

void MainUi::doUnload () {
	bannerWindow.clear ();
	bannerActionButton.clear ();
}

void MainUi::doAddMainToolbarItems (Toolbar *toolbar) {

}

void MainUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiText *uitext;
	Button *button;
	BannerWindow *banner;

	uitext = &(App::instance->uiText);

	button = new Button (sprites.getSprite (MainUi::NextItemButtonSprite));
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
	UiLaunchWindow *uiwindow;
	StringList items;
	int i, len, row, pos;

	App::instance->setNextBackgroundTexturePath ("ui/MainUi/bg");
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	bannerClock = 1200;

	if (! isFirstResumeComplete) {
		if (! App::instance->prefsMap.find (App::IsFirstLaunchCompleteKey, false)) {
			App::instance->prefsMap.insert (App::IsFirstLaunchCompleteKey, true);
		}
		else {
			App::instance->network.sendHttpGet (App::getApplicationNewsUrl (), MainUi::getApplicationNewsComplete, this);
		}

		App::instance->prefsMap.find (App::MainUiExpandedUiTypesKey, &items);
		len = sizeof (MainUi::uiLaunchWindowTypes) / sizeof (MainUi::uiLaunchWindowTypes[0]);
		for (i = 0; i < len; ++i) {
			uiwindow = new UiLaunchWindow (MainUi::uiLaunchWindowTypes[i], &sprites);
			uiwindow->setExpandStateChangeCallback (MainUi::uiExpandStateChanged, this);
			uiwindow->setOpenCallback (MainUi::uiOpenClicked, this);
			uiwindow->itemId.sprintf ("%08x", i);

			pos = items.indexOf (StdString::createSprintf ("%i", uiwindow->uiType));
			if (pos < 0) {
				row = MainUi::UnexpandedUiRow;
				uiwindow->sortKey.assign (uiwindow->itemId);
			}
			else {
				row = MainUi::ExpandedUiRow;
				uiwindow->setExpanded (true, true);
				uiwindow->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
			}

			cardView->addItem (uiwindow, uiwindow->itemId, row, true);
			uiwindow->animateNewCard ();
		}

		cardView->refresh ();
	}
}

void MainUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MainUi::doPause () {
	StringList items;

	cardView->processItems (MainUi::appendExpandedUiType, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::MainUiExpandedUiTypesKey);
	}
	else {
		App::instance->prefsMap.insert (App::MainUiExpandedUiTypesKey, &items);
	}
}

void MainUi::appendExpandedUiType (void *stringListPtr, Widget *widgetPtr) {
	UiLaunchWindow *uiwindow;

	uiwindow = UiLaunchWindow::castWidget (widgetPtr);
	if (uiwindow && uiwindow->isExpanded) {
		((StringList *) stringListPtr)->push_back (StdString::createSprintf ("%i", uiwindow->uiType));
	}
}

void MainUi::doUpdate (int msElapsed) {
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

	store = &(App::instance->agentControl.recordStore);
	agentCount = store->countCommandRecords (SystemInterface::CommandId_AgentStatus, App::instance->prefsMap.find (App::ServerTimeoutKey, App::defaultServerTimeout) * 1000);
	cardView->syncRecordStore ();
	cardView->refresh ();
}

void MainUi::helpActionClicked (void *uiPtr, Widget *widgetPtr) {
	App::instance->uiStack.toggleHelpWindow ();
}

void MainUi::uiExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	MainUi *ui;
	UiLaunchWindow *uiwindow;

	ui = (MainUi *) uiPtr;
	uiwindow = (UiLaunchWindow *) widgetPtr;
	if (uiwindow->isExpanded) {
		uiwindow->sortKey.sprintf ("%016llx", (long long int) OsUtil::getTime ());
		ui->cardView->setItemRow (uiwindow->itemId, MainUi::ExpandedUiRow);
	}
	else {
		uiwindow->sortKey.assign (uiwindow->itemId);
		ui->cardView->setItemRow (uiwindow->itemId, MainUi::UnexpandedUiRow);
	}
	uiwindow->resetInputState ();
	uiwindow->animateNewCard ();
	ui->cardView->refresh ();
}

void MainUi::uiOpenClicked (void *uiPtr, Widget *widgetPtr) {
	UiLaunchWindow *uiwindow;
	Ui *ui;

	uiwindow = (UiLaunchWindow *) widgetPtr;
	ui = NULL;
	switch (uiwindow->uiType) {
		case UiLaunchWindow::ServerUi: {
			ui = (Ui *) new ServerUi ();
			break;
		}
		case UiLaunchWindow::MediaUi: {
			ui = (Ui *) new MediaUi ();
			break;
		}
		case UiLaunchWindow::WebKioskUi: {
			ui = (Ui *) new WebKioskUi ();
			break;
		}
		case UiLaunchWindow::CameraUi: {
			ui = (Ui *) new CameraUi ();
			break;
		}
		case UiLaunchWindow::CommandUi: {
			ui = (Ui *) new CommandUi ();
			break;
		}
	}

	if (ui) {
		App::instance->uiStack.pushUi (ui);
	}
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
				button = new Button (NULL, activeBanner.actionText.uppercased ());
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

	if ((statusCode != Network::HttpOkCode) || (responseData->length <= 0)) {
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

	App::instance->prefsMap.insert (App::MainUiApplicationNewsItemsKey, &items);
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
	if (! App::instance->prefsMap.find (App::IsFirstLaunchCompleteKey, false)) {
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

	App::instance->prefsMap.find (App::MainUiApplicationNewsItemsKey, &items);
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		json = new Json ();
		if (json->parse (*i)) {
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
	item = MainUi::Banner ();
	item.messageText.assign (uitext->getText (UiTextString::mouseHoverBannerMessage));
	item.iconType.assign (MainUi::textMessageIconType);
	bannerList.push_back (item);
	item = MainUi::Banner ();
	item.messageText.assign (uitext->getText (UiTextString::imageLongPressBannerMessage));
	item.iconType.assign (MainUi::textMessageIconType);
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
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::launchWebBrowserError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}
