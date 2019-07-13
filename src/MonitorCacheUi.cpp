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
#include "Ui.h"
#include "SystemInterface.h"
#include "StringList.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "Chip.h"
#include "Json.h"
#include "Menu.h"
#include "MonitorWindow.h"
#include "IconCardWindow.h"
#include "IconLabelWindow.h"
#include "StreamWindow.h"
#include "MediaThumbnailWindow.h"
#include "HelpWindow.h"
#include "StreamItemUi.h"
#include "MonitorCacheUi.h"

MonitorCacheUi::MonitorCacheUi (const StdString &agentId, const StdString &agentName)
: Ui ()
, agentId (agentId)
, agentName (agentName)
, cardView (NULL)
, stopButton (NULL)
, playButton (NULL)
, writePlaylistButton (NULL)
, deleteButton (NULL)
, cardDetail (-1)
, streamCount (0)
, streamSetSize (-1)
, recordReceiveCount (0)
, nextRecordSyncTime (0)
{
}

MonitorCacheUi::~MonitorCacheUi () {

}

StdString MonitorCacheUi::getSpritePath () {
	return (StdString ("ui/MonitorCacheUi/sprite"));
}

Widget *MonitorCacheUi::createBreadcrumbWidget () {
	return (new Chip (agentName, sprites.getSprite (MonitorCacheUi::BreadcrumbIconSprite)));
}

void MonitorCacheUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::monitorCacheUiHelpTitle), uitext->getText (UiTextString::monitorCacheUiHelpText));
	if ((streamCount <= 0) && (streamSetSize == 0)) {
		helpWindow->addAction (uitext->getText (UiTextString::monitorCacheUiHelpAction1Text));
	}
	else {
		helpWindow->addAction (uitext->getText (UiTextString::monitorCacheUiHelpAction2Text));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

int MonitorCacheUi::doLoad () {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	streamSetSize = -1;
	cardDetail = App::instance->prefsMap.find (App::MonitorCacheUiImageSizeKey, (int) CardView::MediumDetail);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (MonitorCacheUi::StreamRow, createRowHeaderPanel (uitext->getText (UiTextString::videoStreams).capitalized ()));
	cardView->setRowHeader (MonitorCacheUi::EmptyStreamRow, createRowHeaderPanel (uitext->getText (UiTextString::videoStreams).capitalized ()));
	cardView->setRowSelectionAnimated (MonitorCacheUi::StreamRow, true);
	cardView->setRowDetail (MonitorCacheUi::StreamRow, cardDetail);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	return (Result::Success);
}

void MonitorCacheUi::doUnload () {
	emptyStreamWindow.destroyAndClear ();
	selectedStreamWindow.clear ();
	commandPopup.destroyAndClear ();
	commandButton.clear ();
}

void MonitorCacheUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MonitorCacheUi::imageSizeButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MonitorCacheUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);
}

void MonitorCacheUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	deleteButton = new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite));
	deleteButton->setInverseColor (true);
	deleteButton->setMouseClickCallback (MonitorCacheUi::deleteButtonClicked, this);
	deleteButton->setMouseEnterCallback (MonitorCacheUi::commandButtonMouseEntered, this);
	deleteButton->setMouseExitCallback (MonitorCacheUi::commandButtonMouseExited, this);
	deleteButton->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::monitorCacheUiDeleteTooltip));
	toolbar->addLeftItem (deleteButton);

	playButton = new Button (StdString (""), sprites.getSprite (MonitorCacheUi::PlayButtonSprite));
	playButton->setInverseColor (true);
	playButton->setMouseClickCallback (MonitorCacheUi::playButtonClicked, this);
	playButton->setMouseEnterCallback (MonitorCacheUi::commandButtonMouseEntered, this);
	playButton->setMouseExitCallback (MonitorCacheUi::commandButtonMouseExited, this);
	playButton->shortcutKey = SDLK_F3;
	playButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorCacheUiPlayTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (playButton);

	writePlaylistButton = new Button (StdString (""), sprites.getSprite (MonitorCacheUi::WritePlaylistButtonSprite));
	writePlaylistButton->setInverseColor (true);
	writePlaylistButton->setMouseClickCallback (MonitorCacheUi::writePlaylistButtonClicked, this);
	writePlaylistButton->setMouseEnterCallback (MonitorCacheUi::commandButtonMouseEntered, this);
	writePlaylistButton->setMouseExitCallback (MonitorCacheUi::commandButtonMouseExited, this);
	writePlaylistButton->shortcutKey = SDLK_F2;
	writePlaylistButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorCacheUiWritePlaylistTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (writePlaylistButton);

	stopButton = new Button (StdString (""), sprites.getSprite (MonitorCacheUi::StopButtonSprite));
	stopButton->setInverseColor (true);
	stopButton->setMouseClickCallback (MonitorCacheUi::stopButtonClicked, this);
	stopButton->setMouseEnterCallback (MonitorCacheUi::commandButtonMouseEntered, this);
	stopButton->setMouseExitCallback (MonitorCacheUi::commandButtonMouseExited, this);
	stopButton->shortcutKey = SDLK_F1;
	stopButton->setMouseHoverTooltip (uitext->getText (UiTextString::monitorCacheUiStopTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (stopButton);
}

void MonitorCacheUi::doClearPopupWidgets () {
	commandPopup.destroyAndClear ();
	commandButton.clear ();
}

void MonitorCacheUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/MonitorCacheUi/bg");
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MonitorCacheUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MonitorCacheUi::doPause () {

}

void MonitorCacheUi::doUpdate (int msElapsed) {
	StreamWindow *window;
	int64_t now;

	emptyStreamWindow.compact ();
	commandPopup.compact ();

	selectedStreamWindow.compact ();
	window = StreamWindow::castWidget (selectedStreamWindow.widget);
	if (window && (! window->isSelected)) {
		selectedStreamWindow.clear ();
	}

	if (recordReceiveCount > 0) {
		now = OsUtil::getTime ();
		if ((recordReceiveCount > 16) || (now >= nextRecordSyncTime)) {
			App::instance->shouldSyncRecordStore = true;
			recordReceiveCount = 0;
		}
	}
}

void MonitorCacheUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MonitorCacheUi::doSyncRecordStore () {
	RecordStore *store;
	MonitorWindow *monitorwindow;
	IconCardWindow *iconwindow;
	UiConfiguration *uiconfig;
	UiText *uitext;

	store = &(App::instance->agentControl.recordStore);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	if (! cardView->contains (agentId)) {
		if (store->findRecord (agentId, SystemInterface::CommandId_AgentStatus)) {
			monitorwindow = new MonitorWindow (agentId);
			monitorwindow->setStorageDisplayEnabled (true);
			monitorwindow->setExpanded (true);
			cardView->addItem (monitorwindow, agentId, MonitorCacheUi::AgentRow);
			addLinkAgent (agentId);
		}
	}

	streamCount = 0;
	store->processCommandRecords (SystemInterface::CommandId_StreamItem, MonitorCacheUi::processStreamItem, this);

	iconwindow = (IconCardWindow *) emptyStreamWindow.widget;
	if ((streamCount <= 0) && (streamSetSize == 0)) {
		if (! iconwindow) {
			iconwindow = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeStreamIconSprite), uitext->getText (UiTextString::monitorCacheUiEmptyStreamStatusTitle), StdString (""), uitext->getText (UiTextString::monitorCacheUiEmptyStreamStatusText));
			iconwindow->setLink (uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("media-streaming"));
			emptyStreamWindow.assign (iconwindow);

			iconwindow->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (iconwindow, iconwindow->itemId, MonitorCacheUi::EmptyStreamRow);
		}
	}
	else {
		if (iconwindow) {
			cardView->removeItem (iconwindow->itemId);
			emptyStreamWindow.clear ();
		}
	}

	cardView->syncRecordStore ();
	cardView->refresh ();
}

void MonitorCacheUi::handleLinkClientConnect (const StdString &linkAgentId) {
	Json *params;

	if (! agentId.equals (linkAgentId)) {
		return;
	}

	params = new Json ();
	params->set ("maxResults", 0);
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Monitor, params), agentId);
}

void MonitorCacheUi::handleLinkClientCommand (const StdString &linkAgentId, int commandId, Json *command) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;

	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);
	if (! agentId.equals (linkAgentId)) {
		return;
	}

	switch (commandId) {
		case SystemInterface::CommandId_FindStreamsResult: {
			streamSetSize = interface->getCommandNumberParam (command, "setSize", (int) 0);
			App::instance->shouldSyncRecordStore = true;
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

void MonitorCacheUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Menu *action;
	bool show;

	ui = (MonitorCacheUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == widgetPtr)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	App::instance->uiStack.suspendMouseHover ();
	action = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->isClickDestroyEnabled = true;
	action->addItem (uitext->getText (UiTextString::small).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), MonitorCacheUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	action->addItem (uitext->getText (UiTextString::medium).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), MonitorCacheUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	action->addItem (uitext->getText (UiTextString::large).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), MonitorCacheUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);
	action->position.assign (widgetPtr->position.x + widgetPtr->width - action->width, widgetPtr->position.y + widgetPtr->height);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (widgetPtr);
}

void MonitorCacheUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	int detail;

	ui = (MonitorCacheUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MonitorCacheUi::StreamRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::MonitorCacheUiImageSizeKey, ui->cardDetail);
}

void MonitorCacheUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	int detail;

	ui = (MonitorCacheUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MonitorCacheUi::StreamRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::MonitorCacheUiImageSizeKey, ui->cardDetail);
}

void MonitorCacheUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	int detail;

	ui = (MonitorCacheUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MonitorCacheUi::StreamRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::MonitorCacheUiImageSizeKey, ui->cardDetail);
}

void MonitorCacheUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	Json *params;

	ui = (MonitorCacheUi *) uiPtr;
	App::instance->agentControl.refreshAgentStatus (ui->agentId);

	params = new Json ();
	params->set ("maxResults", 0);
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindItems, SystemInterface::Constant_Monitor, params), ui->agentId);
}

void MonitorCacheUi::processStreamItem (void *uiPtr, Json *record, const StdString &recordId) {
	MonitorCacheUi *ui;
	SystemInterface *interface;
	StreamWindow *window;

	ui = (MonitorCacheUi *) uiPtr;
	interface = &(App::instance->systemInterface);
	if (! ui->agentId.equals (interface->getCommandAgentId (record))) {
		return;
	}

	++(ui->streamCount);
	if (! ui->cardView->contains (recordId)) {
		window = new StreamWindow (record);
		window->setStreamImageClickCallback (MonitorCacheUi::streamWindowImageClicked, ui);
		window->setViewButtonClickCallback (MonitorCacheUi::streamWindowViewButtonClicked, ui);
		window->sortKey.assign (window->streamName);
		ui->cardView->addItem (window, recordId, MonitorCacheUi::StreamRow);
	}
}

void MonitorCacheUi::streamWindowImageClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	StreamWindow *target;

	ui = (MonitorCacheUi *) uiPtr;
	target = StreamWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	if (! target->isSelected) {
		ui->setSelectedStream (target);
	}
}

void MonitorCacheUi::streamWindowViewButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	StreamWindow *target;
	StreamItemUi *itemui;

	target = StreamWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	ui = (MonitorCacheUi *) uiPtr;
	ui->actionTarget.assign (target);
	itemui = new StreamItemUi (target->streamId, target->streamName, App::instance->uiText.getText (UiTextString::selectPlayPosition).capitalized ());
	itemui->setThumbnailClickCallback (MonitorCacheUi::streamItemUiThumbnailClicked, ui);
	App::instance->uiStack.pushUi (itemui);
}

void MonitorCacheUi::streamItemUiThumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	MediaThumbnailWindow *thumbnail;
	StreamWindow *target;

	ui = (MonitorCacheUi *) uiPtr;
	thumbnail = MediaThumbnailWindow::castWidget (widgetPtr);
	target = StreamWindow::castWidget (ui->actionTarget.widget);
	if ((! thumbnail) || (! target)) {
		return;
	}

	target->setThumbnailIndex (thumbnail->thumbnailIndex);
	target->setDisplayTimestamp (thumbnail->thumbnailTimestamp);
	ui->setSelectedStream (target);
	App::instance->uiStack.popUi ();
}

void MonitorCacheUi::setSelectedStream (StreamWindow *streamWindow) {
	selectedStreamWindow.assign (streamWindow);
	cardView->processRowItems (MonitorCacheUi::StreamRow, MonitorCacheUi::unselectStreamWindow, this);
	streamWindow->setSelected (true);
}

void MonitorCacheUi::unselectStreamWindow (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	StreamWindow *window;

	ui = (MonitorCacheUi *) uiPtr;
	window = StreamWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	if (window != ui->selectedStreamWindow.widget) {
		window->setSelected (false);
	}
}

void MonitorCacheUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	Button *button;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	StreamWindow *streamwindow;
	StdString text;
	Color color;

	ui = (MonitorCacheUi *) uiPtr;
	button = (Button *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	Button::mouseEntered (button, button);

	ui->commandPopup.destroyAndClear ();
	ui->commandButton.assign (button);
	panel = (Panel *) App::instance->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha));
	panel->setBorder (true, Color (uiconfig->darkBackgroundColor.r, uiconfig->darkBackgroundColor.g, uiconfig->darkBackgroundColor.b, uiconfig->overlayWindowAlpha));

	label = NULL;
	if (button == ui->stopButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::stop).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));
	}
	else if (button == ui->playButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::play).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		streamwindow = StreamWindow::castWidget (ui->selectedStreamWindow.widget);
		if (! streamwindow) {
			text.assign (uitext->getText (UiTextString::monitorCacheUiNoStreamSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (streamwindow->streamName);
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			text.appendSprintf (" %s", OsUtil::getDurationString ((streamwindow->displayTimestamp > 0.0f) ? (int64_t) streamwindow->displayTimestamp : 0, OsUtil::HoursUnit).c_str ());
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->writePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::playAll).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));
	}
	else if (button == ui->deleteButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::removeStream).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		streamwindow = StreamWindow::castWidget (ui->selectedStreamWindow.widget);
		if (! streamwindow) {
			text.assign (uitext->getText (UiTextString::monitorCacheUiNoStreamSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (streamwindow->streamName);
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, StdString ("..."));
			color.assign (uiconfig->primaryTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}

	if (label) {
		label->setPadding (0.0f, 0.0f);
	}
	panel->setLayout (Panel::VerticalRightJustifiedLayout);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	panel->position.assign (App::instance->windowWidth - panel->width - uiconfig->paddingSize, App::instance->windowHeight - App::instance->uiStack.bottomBarHeight - panel->height - uiconfig->marginSize);
}

void MonitorCacheUi::commandButtonMouseExited (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	Button *button;

	ui = (MonitorCacheUi *) uiPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);

	if (ui->commandButton.widget == button) {
		ui->commandPopup.destroyAndClear ();
		ui->commandButton.clear ();
	}
}

void MonitorCacheUi::stopButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	int result;

	ui = (MonitorCacheUi *) uiPtr;

	result = App::instance->agentControl.invokeCommand (ui->agentId, App::instance->createCommand (SystemInterface::Command_ClearDisplay, SystemInterface::Constant_Monitor));
	if (result != Result::Success) {
		Log::debug ("Failed to invoke ClearDisplay command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokeClearDisplayMessage));
	}

	App::instance->agentControl.refreshAgentStatus (ui->agentId);
}

void MonitorCacheUi::playButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	StreamWindow *streamwindow;
	Json *params;
	int result;

	ui = (MonitorCacheUi *) uiPtr;
	streamwindow = StreamWindow::castWidget (ui->selectedStreamWindow.widget);
	if (! streamwindow) {
		return;
	}

	params = new Json ();
	params->set ("streamId", streamwindow->streamId);
	params->set ("startPosition", (streamwindow->displayTimestamp > 0.0f) ? (streamwindow->displayTimestamp / 1000.0f) : 0.0f);
	result = App::instance->agentControl.invokeCommand (ui->agentId, App::instance->createCommand (SystemInterface::Command_PlayCacheStream, SystemInterface::Constant_Monitor, params));
	if (result != Result::Success) {
		Log::debug ("Failed to invoke PlayCacheStream command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::invokePlayMediaMessage));
	}

	App::instance->agentControl.refreshAgentStatus (ui->agentId);
}

void MonitorCacheUi::writePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;
	std::list<Json *> items;
	Json *params;
	int result;

	ui = (MonitorCacheUi *) uiPtr;

	if (ui->streamCount <= 0) {
		return;
	}

	ui->cardView->processItems (MonitorCacheUi::appendPlaylistItem, &items);
	if (items.empty ()) {
		return;
	}

	params = new Json ();
	params->set ("items", &items);
	params->set ("displayName", "Loop");
	params->set ("isShuffle", true);
	params->set ("minStartPositionDelta", 0);
	params->set ("maxStartPositionDelta", 85);
	params->set ("minItemDisplayDuration", 30);
	params->set ("maxItemDisplayDuration", 360);

	result = App::instance->agentControl.invokeCommand (ui->agentId, App::instance->createCommand (SystemInterface::Command_CreateMediaDisplayIntent, SystemInterface::Constant_Monitor, params));
	if (result != Result::Success) {
		Log::debug ("Failed to invoke CreateMediaDisplayIntent command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::monitorCacheUiWritePlaylistMessage));
	}

	App::instance->agentControl.refreshAgentStatus (ui->agentId);
}

void MonitorCacheUi::appendPlaylistItem (void *jsonListPtr, Widget *widgetPtr) {
	StreamWindow *window;
	std::list<Json *> *items;
	Json *obj;

	items = (std::list<Json *> *) jsonListPtr;
	window = StreamWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	obj = new Json ();
	obj->set ("mediaName", window->streamName);
	obj->set ("streamId", window->streamId);

	items->push_back (obj);
}

void MonitorCacheUi::deleteButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MonitorCacheUi *ui;

	ui = (MonitorCacheUi *) uiPtr;
	ui->clearPopupWidgets ();
	ui->invokeRemoveMediaStream ();
}

void MonitorCacheUi::invokeRemoveMediaStream () {
	StreamWindow *target;
	Json *params;
	int result;

	target = StreamWindow::castWidget (selectedStreamWindow.widget);
	if (! target) {
		return;
	}

	params = new Json ();
	params->set ("id", target->streamId);

	retain ();
	result = App::instance->agentControl.invokeCommand (target->agentId, App::instance->createCommand (SystemInterface::Command_RemoveStream, SystemInterface::Constant_Monitor, params), MonitorCacheUi::removeStreamComplete, this);
	if (result != Result::Success) {
		release ();
		Log::debug ("Failed to invoke RemoveStream command; err=%i agentId=\"%s\"", result, target->agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		return;
	}
}

void MonitorCacheUi::removeStreamComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	MonitorCacheUi *ui;
	SystemInterface *interface;
	UiText *uitext;
	StdString agentname, text, id;

	ui = (MonitorCacheUi *) uiPtr;

	interface = &(App::instance->systemInterface);
	uitext = &(App::instance->uiText);

	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::CommandId_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		id = interface->getCommandStringParam (invokeCommand, "id", "");
		if (! id.empty ()) {
			App::instance->agentControl.recordStore.removeRecord (id);
			ui->cardView->removeItem (id);
		}
		agentname = App::instance->agentControl.getAgentDisplayName (agentId);
		if (! agentname.empty ()) {
			text.appendSprintf ("%s: ", agentname.c_str ());
		}
		text.append (uitext->getText (UiTextString::removedStream).capitalized ());
		App::instance->uiStack.showSnackbar (text);
		App::instance->agentControl.refreshAgentStatus (agentId);
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	ui->release ();
}
