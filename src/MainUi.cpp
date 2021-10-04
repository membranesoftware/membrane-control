/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "StdString.h"
#include "StringList.h"
#include "Log.h"
#include "App.h"
#include "OsUtil.h"
#include "Network.h"
#include "SystemInterface.h"
#include "SequenceList.h"
#include "Ui.h"
#include "UiText.h"
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
#include "MainUi.h"

const char *MainUi::ExpandedUiTypesKey = "Main_ExpandedUiTypes";
const char *MainUi::ShortcutUiTypeKey = "Main_ShortcutUiType";
const char *MainUi::ApplicationNewsItemsKey = "Main_ApplicationNewsItems";

const int MainUi::UiLaunchWindowTypes[] = {
	UiLaunchWindow::ServerUi,
	UiLaunchWindow::MediaUi,
	UiLaunchWindow::WebKioskUi,
	UiLaunchWindow::CameraUi
};
const StdString MainUi::AnnouncementIconType = StdString ("a");
const StdString MainUi::UpdateIconType = StdString ("b");
const StdString MainUi::TextMessageIconType = StdString ("c");
const StdString MainUi::VideoMessageIconType = StdString ("d");
const StdString MainUi::OpenUrlActionType = StdString ("a");
const StdString MainUi::HelpActionType = StdString ("b");

MainUi::MainUi ()
: Ui ()
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
	helpWindow->setHelpText (UiText::instance->getText (UiTextString::MainUiHelpTitle), UiText::instance->getText (UiTextString::MainUiHelpText));
	if (agentCount <= 0) {
		helpWindow->addAction (UiText::instance->getText (UiTextString::MainUiServersHelpActionText), UiText::instance->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
	}

	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::MembraneSoftwareOverviewHelpTitle), App::getHelpUrl ("membrane-software-overview"));
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

int MainUi::doLoad () {
	cardView->addItem (createRowHeaderPanel (UiText::instance->getText (UiTextString::MainMenu).capitalized ()), StdString (""), MainUi::TitleRow);
	cardView->setRowReverseSorted (MainUi::ExpandedUiRow, true);

	bannerList.randomizeOrder (&(App::instance->prng));
	bannerIconTypeMap.insert (MainUi::AnnouncementIconType, MainUi::AnnouncementIconSprite);
	bannerIconTypeMap.insert (MainUi::UpdateIconType, MainUi::UpdateIconSprite);
	bannerIconTypeMap.insert (MainUi::TextMessageIconType, MainUi::TextMessageIconSprite);
	bannerIconTypeMap.insert (MainUi::VideoMessageIconType, MainUi::VideoMessageIconSprite);
	bannerActionCallbackMap.insert (std::pair<StdString, Widget::EventCallback> (MainUi::OpenUrlActionType, MainUi::openUrlActionClicked));
	bannerActionCallbackMap.insert (std::pair<StdString, Widget::EventCallback> (MainUi::HelpActionType, MainUi::helpActionClicked));
	bannerClock = 1200;
	resetBanners ();

	App::instance->shouldSyncRecordStore = true;

	return (OsUtil::Result::Success);
}

void MainUi::doUnload () {
	bannerWindow.clear ();
	bannerActionButton.clear ();
}

void MainUi::doAddMainToolbarItems (Toolbar *toolbar) {

}

void MainUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	Button *button;
	BannerWindow *banner;

	button = new Button (sprites.getSprite (MainUi::NextItemButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MainUi::nextBannerButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MainUiNextBannerTooltip));
	button->zLevel = 1;
	toolbar->addRightItem (button);

	banner = new BannerWindow (toolbar->getLeftWidth ());
	bannerWindow.assign (banner);
	toolbar->addLeftItem (banner);
}

void MainUi::doClearPopupWidgets () {

}

void MainUi::doResume () {
	HashMap *prefs;
	UiLaunchWindow *uiwindow;
	StringList items;
	int i, len, row, pos;
	bool shouldgetnews;

	App::instance->setNextBackgroundTexturePath ("ui/MainUi/bg");
	bannerClock = 1200;

	if (! isFirstResumeComplete) {
		shouldgetnews = false;

		prefs = App::instance->lockPrefs ();
		if (! prefs->find (App::IsFirstLaunchCompleteKey, false)) {
			prefs->insert (App::IsFirstLaunchCompleteKey, true);
		}
		else {
			shouldgetnews = true;
		}
		prefs->find (MainUi::ExpandedUiTypesKey, &items);
		App::instance->unlockPrefs ();

		len = sizeof (MainUi::UiLaunchWindowTypes) / sizeof (MainUi::UiLaunchWindowTypes[0]);
		for (i = 0; i < len; ++i) {
			uiwindow = new UiLaunchWindow (MainUi::UiLaunchWindowTypes[i], &sprites);
			uiwindow->expandStateChangeCallback = Widget::EventCallbackContext (MainUi::uiExpandStateChanged, this);
			uiwindow->openCallback = Widget::EventCallbackContext (MainUi::uiOpenClicked, this);
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

		if (shouldgetnews) {
			Network::instance->sendHttpGet (App::getApplicationNewsUrl (), Network::HttpRequestCallbackContext (MainUi::getApplicationNewsComplete, this));
		}
	}
}

void MainUi::doPause () {
	HashMap *prefs;
	StringList items;

	cardView->processItems (MainUi::appendExpandedUiType, &items);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MainUi::ExpandedUiTypesKey, &items);
	App::instance->unlockPrefs ();
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
	if (bannerWindow.widget) {
		((BannerWindow *) bannerWindow.widget)->setWindowWidth (App::instance->uiStack.secondaryToolbar->getLeftWidth ());
	}
}

void MainUi::doSyncRecordStore () {
	agentCount = RecordStore::instance->countCommandRecords (SystemInterface::CommandId_AgentStatus);
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
	}

	if (ui) {
		App::instance->uiStack.pushUi (ui);
	}
}

void MainUi::showNextBanner () {
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

	spriteindex = bannerIconTypeMap.find (activeBanner.iconType, (int) -1);
	bannerActionButton.destroyAndClear ();
	if (! activeBanner.actionText.empty ()) {
		pos = bannerActionCallbackMap.find (activeBanner.actionType);
		if (pos != bannerActionCallbackMap.end ()) {
			callback = pos->second;
			if (callback) {
				button = new Button (NULL, activeBanner.actionText.uppercased ());
				button->mouseClickCallback = Widget::EventCallbackContext (callback, this);
				button->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
				button->setTextColor (UiConfiguration::instance->raisedButtonTextColor);
				button->zLevel = 1;
				bannerActionButton.assign (button);
				App::instance->uiStack.secondaryToolbar->addRightItem (button);
			}
		}
	}

	banner->setWindowWidth (App::instance->uiStack.secondaryToolbar->getLeftWidth () - (UiConfiguration::instance->marginSize * 2.0f));
	banner->setBanner (activeBanner.messageText, sprites.getSprite (spriteindex));
	if (activeBanner.actionType.equals (MainUi::HelpActionType)) {
		bannerClock = 300000;
	}
	else {
		bannerClock = App::instance->getRandomInt (75000, 90000);
	}
}

void MainUi::getApplicationNewsComplete (void *uiPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	MainUi *ui;
	HashMap *prefs;
	Json *cmd, *item;
	StringList items;
	int i, count;

	if ((statusCode != Network::HttpOkCode) || (responseData->length <= 0)) {
		return;
	}
	if (! SystemInterface::instance->parseCommand (StdString ((char *) responseData->data, responseData->length), &cmd)) {
		return;
	}

	ui = (MainUi *) uiPtr;
	count = SystemInterface::instance->getCommandArrayLength (cmd, "items");
	for (i = 0; i < count; ++i) {
		item = new Json ();
		if (SystemInterface::instance->getCommandObjectArrayItem (cmd, "items", i, item)) {
			items.push_back (item->toString ());
		}
		delete (item);
	}
	delete (cmd);

	prefs = App::instance->lockPrefs ();
	prefs->insert (MainUi::ApplicationNewsItemsKey, &items);
	App::instance->unlockPrefs ();
	ui->resetBanners ();
}

void MainUi::resetBanners () {
	HashMap *prefs;
	MainUi::Banner item;
	StringList items;
	StringList::iterator i, end;
	Json *json;

	bannerList.clear ();
	prefs = App::instance->lockPrefs ();
	if (! prefs->find (App::IsFirstLaunchCompleteKey, false)) {
		item = MainUi::Banner ();
		item.messageText.assign (UiText::instance->getText (UiTextString::FirstLaunchBannerMessage));
		item.iconType.assign (MainUi::AnnouncementIconType);
		item.actionType.assign (MainUi::HelpActionType);
		item.actionText.assign (UiText::instance->getText (UiTextString::OpenHelp));
		bannerList.push_back (item);
		bannerList.nextItemIndex = 0;
	}
	else {
		item = MainUi::Banner ();
		item.messageText.assign (UiText::instance->getText (UiTextString::HelpKeyBannerMessage));
		item.iconType.assign (MainUi::TextMessageIconType);
		bannerList.push_back (item);
	}
	prefs->find (MainUi::ApplicationNewsItemsKey, &items);
	App::instance->unlockPrefs ();

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
	item.messageText.assign (UiText::instance->getText (UiTextString::FreeApplicationBannerMessage));
	item.iconType.assign (MainUi::TextMessageIconType);
	bannerList.push_back (item);
	item = MainUi::Banner ();
	item.messageText.assign (UiText::instance->getText (UiTextString::MembraneSoftwareBannerMessage));
	item.iconType.assign (MainUi::TextMessageIconType);
	item.actionType.assign (MainUi::OpenUrlActionType);
	item.actionText.assign (UiText::instance->getText (UiTextString::LearnMore));
	item.actionTarget.assign (App::getHelpUrl ("about-membrane-software"));
	bannerList.push_back (item);
	item = MainUi::Banner ();
	item.messageText.assign (UiText::instance->getText (UiTextString::DonateBannerMessage));
	item.iconType.assign (MainUi::AnnouncementIconType);
	item.actionType.assign (MainUi::OpenUrlActionType);
	item.actionText.assign (UiText::instance->getText (UiTextString::LearnMore));
	item.actionTarget.assign (App::getDonateUrl ());
	bannerList.push_back (item);
	item = MainUi::Banner ();
	item.messageText.assign (UiText::instance->getText (UiTextString::MouseHoverBannerMessage));
	item.iconType.assign (MainUi::TextMessageIconType);
	bannerList.push_back (item);
	item = MainUi::Banner ();
	item.messageText.assign (UiText::instance->getText (UiTextString::ImageLongPressBannerMessage));
	item.iconType.assign (MainUi::TextMessageIconType);
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
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::LaunchWebBrowserError));
		return;
	}

	result = OsUtil::openUrl (url);
	if (result != OsUtil::Result::Success) {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::LaunchWebBrowserError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::LaunchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}
