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
#include "StdString.h"
#include "App.h"
#include "Resource.h"
#include "SpriteGroup.h"
#include "OsUtil.h"
#include "Button.h"
#include "Menu.h"
#include "Chip.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "CardView.h"
#include "MediaWindow.h"
#include "ActionWindow.h"
#include "HelpWindow.h"
#include "IconCardWindow.h"
#include "MediaLibraryWindow.h"
#include "TextFieldWindow.h"
#include "Ui.h"
#include "MediaItemUi.h"
#include "MediaUi.h"

const int MediaUi::pageSize = 64;

MediaUi::MediaUi ()
: Ui ()
, cardView (NULL)
, searchField (NULL)
, searchButton (NULL)
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
, agentCount (0)
, mediaCount (0)
, findMediaComplete (false)
, recordReceiveCount (0)
, nextRecordSyncTime (0)
{

}

MediaUi::~MediaUi () {

}

StdString MediaUi::getSpritePath () {
	return (StdString ("ui/MediaUi/sprite"));
}

Widget *MediaUi::createBreadcrumbWidget () {
	return (new Chip (App::instance->uiText.getText (UiTextString::media).capitalized (), sprites.getSprite (MediaUi::BREADCRUMB_ICON)));
}

void MediaUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::mediaUiHelpTitle), uitext->getText (UiTextString::mediaUiHelpText));
	if (agentCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::mediaUiHelpAction1Text), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
	}
	else {
		helpWindow->addAction (uitext->getText (UiTextString::mediaUiHelpAction2Text));
		helpWindow->addAction (uitext->getText (UiTextString::mediaUiHelpAction3Text), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("media-streaming"));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::mediaStreaming).capitalized (), App::getHelpUrl ("media-streaming"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

int MediaUi::doLoad () {
	int layout;

	agentCount = 0;
	mediaCount = 0;
	findMediaComplete = false;

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowItemMarginSize (1, 0.0f);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	layout = App::instance->prefsMap.find (App::prefsMediaImageSize, (int) MediaWindow::LOW_DETAIL);
	switch (layout) {
		case MediaWindow::LOW_DETAIL: {
			MediaUi::smallThumbnailActionClicked (this, NULL);
			break;
		}
		case MediaWindow::MEDIUM_DETAIL: {
			MediaUi::mediumThumbnailActionClicked (this, NULL);
			break;
		}
		case MediaWindow::HIGH_DETAIL: {
			MediaUi::largeThumbnailActionClicked (this, NULL);
			break;
		}
		default: {
			MediaUi::smallThumbnailActionClicked (this, NULL);
			break;
		}
	}

	return (Result::SUCCESS);
}

void MediaUi::doUnload () {
	thumbnailSizeMenu.clear ();
	emptyStateWindow.clear ();
	mediaServerResultOffsetMap.clear ();
	mediaServerSetSizeMap.clear ();
	mediaServerRecordCountMap.clear ();
}

void MediaUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (StdString (""), sprites.getSprite (MediaUi::THUMBNAIL_SIZE_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaUi::thumbnailSizeButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::RELOAD_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);
}

void MediaUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	searchField = new TextFieldWindow (App::instance->windowWidth * 0.37f, uitext->getText (UiTextString::enterSearchKeyPrompt));
	searchField->setPadding (uiconfig->paddingSize, 0.0f);
	searchField->setButtonsEnabled (false, false, true, true);
	searchField->setEditCallback (MediaUi::searchFieldEdited, this);
	toolbar->addLeftItem (searchField);

	searchButton = new Button (StdString (""), sprites.getSprite (MediaUi::SEARCH_BUTTON));
	searchButton->setInverseColor (true);
	searchButton->setMouseClickCallback (MediaUi::searchButtonClicked, this);
	searchButton->setMouseHoverTooltip (uitext->getText (UiTextString::mediaUiSearchTooltip));
	toolbar->addLeftItem (searchButton);
}

void MediaUi::doClearPopupWidgets () {
	thumbnailSizeMenu.destroyAndClear ();
}

void MediaUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/MediaUi/bg");

	searchField->setValue (searchKey);
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MediaUi::doPause () {
	StringList items;

	App::instance->prefsMap.insert (App::prefsMediaImageSize, cardLayout);

	items.clear ();
	cardView->processItems (MediaUi::appendExpandedAgentId, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::prefsMediaUiExpandedAgents);
	}
	else {
		App::instance->prefsMap.insert (App::prefsMediaUiExpandedAgents, &items);
	}
}

void MediaUi::appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
	MediaLibraryWindow *window;

	window = MediaLibraryWindow::castWidget (widgetPtr);
	if (window && window->isExpanded) {
		((StringList *) stringListPtr)->push_back (window->agentId);
	}
}

void MediaUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MediaUi::doUpdate (int msElapsed) {
	StringList idlist;
	StringList::iterator i, end;
	Json *params;
	int offset, setsize, recordcount;
	int64_t now;

	thumbnailSizeMenu.compact ();
	emptyStateWindow.compact ();

	if (recordReceiveCount > 0) {
		now = OsUtil::getTime ();
		if ((recordReceiveCount > 16) || (now >= nextRecordSyncTime)) {
			App::instance->shouldSyncRecordStore = true;
			recordReceiveCount = 0;
		}
	}
	else {
		if (cardView->isScrolledToBottom ()) {
			mediaServerResultOffsetMap.getKeys (&idlist, true);
			i = idlist.begin ();
			end = idlist.end ();
			while (i != end) {
				offset = mediaServerResultOffsetMap.find (*i, 0);
				setsize = mediaServerSetSizeMap.find (*i, 0);
				recordcount = mediaServerRecordCountMap.find (*i, 0);
				if ((recordcount >= offset) && (recordcount < setsize)) {
					params = new Json ();
					params->set ("searchKey", searchKey);
					params->set ("resultOffset", offset);
					params->set ("maxResults", MediaUi::pageSize);
					App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Media, params), *i);
					offset += MediaUi::pageSize;
					mediaServerResultOffsetMap.insert (*i, offset);
				}
				++i;
			}
		}
	}
}

void MediaUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	searchField->setWindowWidth (App::instance->windowWidth * 0.37f);
}

void MediaUi::doSyncRecordStore () {
	RecordStore *store;
	IconCardWindow *window;
	UiConfiguration *uiconfig;
	UiText *uitext;

	store = &(App::instance->agentControl.recordStore);
	agentCount = 0;
	store->processAgentRecords ("mediaServerStatus", MediaUi::processAgentStatus, this);

	mediaCount = 0;
	store->processCommandRecords (SystemInterface::CommandId_MediaItem, MediaUi::processMediaItem, this);

	window = (IconCardWindow *) emptyStateWindow.widget;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	if (agentCount <= 0) {
		if (! window) {
			window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SERVER_ICON), uitext->getText (UiTextString::mediaUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::mediaUiEmptyAgentStatusText));
			window->setLink (uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
			emptyStateWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, 0);
		}
	}
	else if ((mediaCount <= 0) && findMediaComplete) {
		if (! window) {
			window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::ERROR_ICON), uitext->getText (UiTextString::mediaUiEmptyMediaStatusTitle), StdString (""), uitext->getText (UiTextString::mediaUiEmptyMediaStatusText));
			window->setLink (uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
			emptyStateWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, 1);
		}
	}
	else {
		if (window) {
			cardView->removeItem (window->itemId);
			emptyStateWindow.clear ();
		}
	}

	cardView->syncRecordStore ();
	cardView->refresh ();
}

void MediaUi::processAgentStatus (void *uiPtr, Json *record, const StdString &recordId) {
	MediaUi *ui;
	MediaLibraryWindow *window;
	StringList items;

	ui = (MediaUi *) uiPtr;
	++(ui->agentCount);
	if (! ui->cardView->contains (recordId)) {
		window = new MediaLibraryWindow (recordId);
		window->setMenuClickCallback (MediaUi::mediaLibraryMenuClicked, ui);
		window->setExpandStateChangeCallback (MediaUi::agentExpandStateChanged, ui);

		App::instance->prefsMap.find (App::prefsMediaUiExpandedAgents, &items);
		if (items.contains (recordId)) {
			window->setExpanded (true, true);
		}

		ui->cardView->addItem (window, recordId, 0);
		ui->addLinkAgent (recordId);
	}
}

void MediaUi::processMediaItem (void *uiPtr, Json *record, const StdString &recordId) {
	MediaUi *ui;
	MediaWindow *window;
	SystemInterface *interface;
	StdString agentid;

	ui = (MediaUi *) uiPtr;
	++(ui->mediaCount);
	if (! ui->cardView->contains (recordId)) {
		interface = &(App::instance->systemInterface);
		agentid = interface->getCommandAgentId (record);
		if (ui->mediaServerResultOffsetMap.exists (agentid)) {
			window = new MediaWindow (record, ui->cardLayout, ui->cardMaxImageWidth);
			window->setMouseClickCallback (MediaUi::mediaWindowClicked, ui);
			window->sortKey.assign (window->mediaName);
			ui->cardView->addItem (window, recordId, 1);
		}
	}
}

void MediaUi::handleLinkClientConnect (const StdString &agentId) {
	Json *params;

	if (! mediaServerResultOffsetMap.exists (agentId)) {
		mediaServerResultOffsetMap.insert (agentId, MediaUi::pageSize);
		mediaServerSetSizeMap.insert (agentId, (int) 0);
		mediaServerRecordCountMap.insert (agentId, (int) 0);
		params = new Json ();
		params->set ("searchKey", searchKey);
		params->set ("maxResults", MediaUi::pageSize);
		App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Media, params), agentId);

		App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Stream), agentId);
	}
}

void MediaUi::handleLinkClientCommand (const StdString &agentId, int commandId, Json *command) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;

	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);

	switch (commandId) {
		case SystemInterface::CommandId_FindMediaResult: {
			mediaServerSetSizeMap.insert (agentId, interface->getCommandNumberParam (command, "setSize", (int) 0));
			findMediaComplete = true;
			break;
		}
		case SystemInterface::CommandId_MediaItem: {
			App::instance->agentControl.recordStore.addRecord (command);
			mediaServerRecordCountMap.insert (agentId, mediaServerRecordCountMap.find (agentId, 0) + 1);
			++recordReceiveCount;
			nextRecordSyncTime = OsUtil::getTime () + uiconfig->recordSyncDelayDuration;
			break;
		}
		case SystemInterface::CommandId_FindStreamsResult: {
			break;
		}
		case SystemInterface::CommandId_StreamItem: {
			App::instance->agentControl.recordStore.addRecord (command);
			++recordReceiveCount;
			nextRecordSyncTime = OsUtil::getTime () + uiconfig->recordSyncDelayDuration;
			break;
		}
	}
}

void MediaUi::searchFieldEdited (void *uiPtr, Widget *widgetPtr) {
	MediaUi::searchButtonClicked (uiPtr, NULL);
}

void MediaUi::searchButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;

	if (ui->searchKey.equals (ui->searchField->getValue ())) {
		return;
	}
	ui->searchKey.assign (ui->searchField->getValue ());
	ui->loadSearchResults ();
}

void MediaUi::loadSearchResults () {
	StringList idlist;
	StringList::iterator i, end;
	Json *params;

	mediaServerResultOffsetMap.getKeys (&idlist, true);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		mediaServerResultOffsetMap.insert (*i, MediaUi::pageSize);
		mediaServerSetSizeMap.insert (*i, (int) 0);
		mediaServerRecordCountMap.insert (*i, (int) 0);
		++i;
	}
	App::instance->agentControl.recordStore.removeRecords (SystemInterface::CommandId_MediaItem);
	cardView->removeRowItems (1);

	params = new Json ();
	params->set ("searchKey", searchKey);
	params->set ("maxResults", MediaUi::pageSize);
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Media, params));
}

void MediaUi::thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->handleThumbnailSizeButtonClick (widgetPtr);
}

void MediaUi::handleThumbnailSizeButtonClick (Widget *buttonWidget) {
	UiText *uitext;
	Menu *menu;

	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();
	if (thumbnailSizeMenu.widget) {
		thumbnailSizeMenu.widget->isDestroyed = true;
		thumbnailSizeMenu.clear ();
		return;
	}

	clearPopupWidgets ();
	menu = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	menu->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), sprites.getSprite (MediaUi::SMALL_THUMBNAIL_BUTTON), MediaUi::smallThumbnailActionClicked, this, 0, cardLayout == MediaWindow::LOW_DETAIL);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), sprites.getSprite (MediaUi::MEDIUM_THUMBNAIL_BUTTON), MediaUi::mediumThumbnailActionClicked, this, 0, cardLayout == MediaWindow::MEDIUM_DETAIL);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), sprites.getSprite (MediaUi::LARGE_THUMBNAIL_BUTTON), MediaUi::largeThumbnailActionClicked, this, 0, cardLayout == MediaWindow::HIGH_DETAIL);
	menu->position.assign (buttonWidget->position.x + buttonWidget->width - menu->width, buttonWidget->position.y + buttonWidget->height);
	thumbnailSizeMenu.assign (menu);
}

void MediaUi::smallThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MediaUi *) uiPtr;
	if (ui->cardLayout == MediaWindow::LOW_DETAIL) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = MediaWindow::LOW_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->smallThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaUi::resetMediaCardLayout, ui);
	ui->cardView->setRowItemMarginSize (1, 0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.1f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MediaUi::mediumThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MediaUi *) uiPtr;
	if (ui->cardLayout == MediaWindow::MEDIUM_DETAIL) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = MediaWindow::MEDIUM_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->mediumThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaUi::resetMediaCardLayout, ui);
	ui->cardView->setRowItemMarginSize (1, App::instance->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.2f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MediaUi::largeThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MediaUi *) uiPtr;
	if (ui->cardLayout == MediaWindow::HIGH_DETAIL) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = MediaWindow::HIGH_DETAIL;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->largeThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaUi::resetMediaCardLayout, ui);
	ui->cardView->setRowItemMarginSize (1, App::instance->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.3f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
	ui->cardView->refresh ();
}

void MediaUi::resetMediaCardLayout (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaWindow *window;

	ui = (MediaUi *) uiPtr;
	window = MediaWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	window->setLayout (ui->cardLayout, ui->cardMaxImageWidth);
}

void MediaUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->cardView->processRowItems (0, MediaUi::reloadAgent, ui);
	ui->loadSearchResults ();
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Stream));
}

void MediaUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	MediaLibraryWindow *medialibrarywindow;

	medialibrarywindow = MediaLibraryWindow::castWidget (widgetPtr);
	if (medialibrarywindow) {
		App::instance->agentControl.refreshAgentStatus (medialibrarywindow->agentId);
	}
}

void MediaUi::mediaWindowClicked (void *uiPtr, Widget *widgetPtr) {
	MediaWindow *target;

	target = MediaWindow::castWidget (widgetPtr);
	if ((! target) || (! target->hasThumbnails ())) {
		return;
	}

	App::instance->uiStack.pushUi (new MediaItemUi (target));
}

void MediaUi::agentExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;

	ui = (MediaUi *) uiPtr;
	ui->cardView->refresh ();
}

void MediaUi::mediaLibraryMenuClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	MediaLibraryWindow *target;
	Menu *action;
	bool show;

	ui = (MediaUi *) uiPtr;
	target = (MediaLibraryWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == target)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	action = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (target);

	action->addItem (uitext->getText (UiTextString::scanForMedia).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::UPDATE_BUTTON), MediaUi::mediaLibraryScanActionClicked, ui);
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->position.assign (target->drawX + target->menuPositionX, target->drawY + target->menuPositionY);
}

void MediaUi::mediaLibraryScanActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaUi *ui;
	MediaLibraryWindow *target;
	int result;

	ui = (MediaUi *) uiPtr;
	target = MediaLibraryWindow::castWidget (ui->actionTarget.widget);
	if (! target) {
		return;
	}

	result = App::instance->agentControl.invokeCommand (target->agentId, App::instance->createCommand (SystemInterface::Command_ScanMediaItems, SystemInterface::Constant_Media));
	if (result != Result::SUCCESS) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::scanForMediaStartedMessage));
	}
}
