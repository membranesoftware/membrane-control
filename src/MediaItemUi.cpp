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
#include "AgentControl.h"
#include "Button.h"
#include "SystemInterface.h"
#include "Menu.h"
#include "Ui.h"
#include "UiText.h"
#include "HashMap.h"
#include "RecordStore.h"
#include "Toolbar.h"
#include "Chip.h"
#include "MediaTimelineWindow.h"
#include "CardView.h"
#include "ComboBox.h"
#include "HelpWindow.h"
#include "IconCardWindow.h"
#include "MediaThumbnailWindow.h"
#include "ActionWindow.h"
#include "MediaDetailWindow.h"
#include "MediaItemUi.h"

MediaItemUi::MediaItemUi (const StdString &mediaId, const StdString &mediaName)
: Ui ()
, mediaId (mediaId)
, mediaName (mediaName)
, duration (0)
, frameWidth (0)
, frameHeight (0)
, thumbnailCount (0)
, isRecordSynced (false)
, cardView (NULL)
, cardDetail (-1)
, isCreateStreamAvailable (true)
, removeMediaCallback (NULL)
, removeMediaCallbackData (NULL)
{
	streamServerAgentMap.sort (HashMap::sortAscending);
}

MediaItemUi::~MediaItemUi () {

}

StdString MediaItemUi::getSpritePath () {
	return (StdString ("ui/MediaItemUi/sprite"));
}

Widget *MediaItemUi::createBreadcrumbWidget () {
	return (new Chip (mediaName));
}

void MediaItemUi::setRemoveMediaCallback (Widget::EventCallback callback, void *callbackData) {
	removeMediaCallback = callback;
	removeMediaCallbackData = callbackData;
}

void MediaItemUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::mediaItemUiHelpTitle), uitext->getText (UiTextString::mediaItemUiHelpText));
	helpWindow->addTopicLink (uitext->getText (UiTextString::mediaStreaming).capitalized (), App::getHelpUrl ("media-streaming"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

int MediaItemUi::doLoad () {
	UiConfiguration *uiconfig;
	MediaDetailWindow *detailwindow;

	uiconfig = &(App::instance->uiConfig);
	cardDetail = App::instance->prefsMap.find (App::MediaItemUiImageSizeKey, (int) CardView::MediumDetail);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowItemMarginSize (MediaItemUi::ImageRow, uiconfig->marginSize / 2.0f);
	cardView->setRowDetail (MediaItemUi::ImageRow, cardDetail);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	detailwindow = new MediaDetailWindow (mediaId, &sprites);
	cardView->addItem (detailwindow, mediaId, MediaItemUi::InfoRow);

	cardView->refresh ();
	return (Result::Success);
}

void MediaItemUi::doUnload () {
	streamServerAgentMap.clear ();
}

void MediaItemUi::doAddMainToolbarItems (Toolbar *toolbar) {
	UiText *uitext;
	Button *button;

	uitext = &(App::instance->uiText);

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaItemUi::imageSizeButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);
}

void MediaItemUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiText *uitext;
	Button *button;
	MediaTimelineWindow *timeline;

	uitext = &(App::instance->uiText);
	toolbar->setLeftCorner (new Image (sprites.getSprite (MediaItemUi::TimeIconSprite)));

	timeline = new MediaTimelineWindow (App::instance->windowWidth * 0.5f, mediaId);
	toolbar->addLeftItem (timeline);
	timelineWindow.assign (timeline);

	button = new Button (StdString (""), sprites.getSprite (MediaItemUi::ConfigureStreamButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaItemUi::configureStreamButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::mediaItemUiConfigureStreamTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (button);
}

void MediaItemUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MediaItemUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/MediaItemUi/bg");

	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MediaItemUi::doPause () {

}

void MediaItemUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MediaItemUi::doSyncRecordStore () {
	RecordStore *store;
	MediaTimelineWindow *timeline;

	store = &(App::instance->agentControl.recordStore);
	store->populateAgentMap (&streamServerAgentMap, StdString ("streamServerStatus"));

	if (! isRecordSynced) {
		syncMediaItem ();
	}

	timeline = (MediaTimelineWindow *) timelineWindow.widget;
	if (timeline) {
		timeline->syncRecordStore ();
	}
	cardView->syncRecordStore ();
	cardView->refresh ();
}

void MediaItemUi::syncMediaItem () {
	RecordStore *store;
	SystemInterface *interface;
	AgentControl *agentcontrol;
	MediaThumbnailWindow *thumbnail;
	int i;
	float dt;
	Json *mediaitem, *agentstatus, serverstatus, *params;
	StdString cardid, url, cmdjson;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	agentcontrol = &(App::instance->agentControl);

	mediaitem = store->findRecord (mediaId, SystemInterface::CommandId_MediaItem);
	if (! mediaitem) {
		return;
	}

	agentId = interface->getCommandAgentId (mediaitem);
	if (agentId.empty ()) {
		return;
	}

	agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentId);
	if (! agentstatus) {
		return;
	}
	if (interface->getCommandObjectParam (agentstatus, "mediaServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
		thumbnailCount = serverstatus.getNumber ("thumbnailCount", (int) 0);
	}
	duration = interface->getCommandNumberParam (mediaitem, "duration", (int64_t) 0);
	frameWidth = interface->getCommandNumberParam (mediaitem, "width", (int) 0);
	frameHeight = interface->getCommandNumberParam (mediaitem, "height", (int) 0);
	isCreateStreamAvailable = interface->getCommandBooleanParam (mediaitem, "isCreateStreamAvailable", true);

	if ((thumbnailCount > 0) && (frameWidth > 0) && (frameHeight > 0) && (! thumbnailPath.empty ())) {
		dt = (float) duration / (float) thumbnailCount;
		for (i = 0; i < thumbnailCount; ++i) {
			params = new Json ();
			params->set ("id", mediaId);
			params->set ("thumbnailIndex", i);
			url = agentcontrol->getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, SystemInterface::Constant_Media, params), thumbnailPath);

			cardid.sprintf ("%i", i);
			thumbnail = new MediaThumbnailWindow (i, (dt / 2.0f) + ((float) i * dt), frameWidth, frameHeight, url);
			thumbnail->setMouseEnterCallback (MediaItemUi::thumbnailMouseEntered, this);
			thumbnail->setMouseExitCallback (MediaItemUi::thumbnailMouseExited, this);
			cardView->addItem (thumbnail, cardid, MediaItemUi::ImageRow, true);
		}

		cardView->refresh ();
	}

	isRecordSynced = true;
}

void MediaItemUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Menu *menu;
	bool show;

	ui = (MediaItemUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	App::instance->uiStack.suspendMouseHover ();

	show = true;
	if (Menu::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == widgetPtr)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	menu = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	menu->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), MediaItemUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), MediaItemUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), MediaItemUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);
	menu->position.assign (widgetPtr->position.x + widgetPtr->width - menu->width, widgetPtr->position.y + widgetPtr->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void MediaItemUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	int detail;

	ui = (MediaItemUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaItemUi::ImageRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::MediaItemUiImageSizeKey, ui->cardDetail);
}

void MediaItemUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	int detail;

	ui = (MediaItemUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaItemUi::ImageRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::MediaItemUiImageSizeKey, ui->cardDetail);
}

void MediaItemUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	int detail;

	ui = (MediaItemUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaItemUi::ImageRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::MediaItemUiImageSizeKey, ui->cardDetail);
}

void MediaItemUi::thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	MediaThumbnailWindow *thumbnail;
	MediaTimelineWindow *timeline;

	ui = (MediaItemUi *) uiPtr;
	thumbnail = (MediaThumbnailWindow *) widgetPtr;
	timeline = (MediaTimelineWindow *) ui->timelineWindow.widget;
	if (timeline) {
		timeline->setHighlightedMarker (thumbnail->thumbnailIndex);
	}
}

void MediaItemUi::thumbnailMouseExited (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	MediaThumbnailWindow *thumbnail;
	MediaTimelineWindow *timeline;

	ui = (MediaItemUi *) uiPtr;
	thumbnail = (MediaThumbnailWindow *) widgetPtr;
	timeline = (MediaTimelineWindow *) ui->timelineWindow.widget;
	if (timeline && (timeline->highlightedMarkerIndex == thumbnail->thumbnailIndex)) {
		timeline->setHighlightedMarker (-1);
	}
}

void MediaItemUi::configureStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;
	Widget *target;
	ComboBox *combobox;

	ui = (MediaItemUi *) uiPtr;
	action = ActionWindow::castWidget (ui->actionWidget.widget);
	target = ui->actionTarget.widget;
	if (action && (target == widgetPtr)) {
		ui->clearPopupWidgets ();
		ui->actionTarget.clear ();
		return;
	}

	if (ui->streamServerAgentMap.empty ()) {
		// TODO: Show an error message here
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	ui->clearPopupWidgets ();

	action = (ActionWindow *) App::instance->rootPanel->addWidget (new ActionWindow ());
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->setInverseColor (true);
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);

	if (ui->isCreateStreamAvailable) {
		action->setTitleText (uitext->getText (UiTextString::configureStream).capitalized ());
		action->setConfirmButtonText (uitext->getText (UiTextString::apply).uppercased ());
		action->setCloseCallback (MediaItemUi::configureStreamActionClosed, ui);

	// TODO: Allow selection of a stream server (currently restricted to the server holding the source media item)
	//	action->addComboBoxOption (uitext->getText (UiTextString::streamServer).capitalized (), &(ui->streamServerAgentMap));

		combobox = new ComboBox ();
		combobox->addItem (uitext->getText (UiTextString::normalVideoQualityDescription));
		combobox->addItem (uitext->getText (UiTextString::highVideoQualityDescription));
		combobox->addItem (uitext->getText (UiTextString::lowVideoQualityDescription));
		combobox->addItem (uitext->getText (UiTextString::lowestVideoQualityDescription));
		combobox->setValue (App::instance->prefsMap.find (App::MediaItemUiVideoQualityKey, uitext->getText (UiTextString::normalVideoQualityDescription)));
		action->addOption (uitext->getText (UiTextString::videoQuality).capitalized (), combobox, uitext->getText (UiTextString::videoQualityDescription));
	}
	else {
		action->setTitleText (uitext->getText (UiTextString::removeMedia).capitalized ());
		action->setDescriptionText (uitext->getText (UiTextString::removeMediaActionText));
		action->setConfirmButtonText (uitext->getText (UiTextString::remove).uppercased ());
		action->setCloseCallback (MediaItemUi::removeMediaActionClosed, ui);
	}

	action->position.assign (widgetPtr->screenX + widgetPtr->width - action->width, widgetPtr->screenY - action->height);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (widgetPtr);
}

void MediaItemUi::configureStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ActionWindow *action;
	UiText *uitext;
	StdString quality;
	Json *params;
	int result, profile;

	ui = (MediaItemUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	uitext = &(App::instance->uiText);
	if (action->isConfirmed && (! ui->agentId.empty ())) {
		quality = action->getStringValue (uitext->getText (UiTextString::videoQuality).capitalized (), "");
		if (! quality.empty ()) {
			App::instance->prefsMap.insert (App::MediaItemUiVideoQualityKey, quality);
		}
		profile = SystemInterface::Constant_DefaultStreamProfile;
		if (quality.equals (uitext->getText (UiTextString::highVideoQualityDescription))) {
			profile = SystemInterface::Constant_CompressedStreamProfile;
		}
		else if (quality.equals (uitext->getText (UiTextString::lowVideoQualityDescription))) {
			profile = SystemInterface::Constant_LowQualityStreamProfile;
		}
		else if (quality.equals (uitext->getText (UiTextString::lowestVideoQualityDescription))) {
			profile = SystemInterface::Constant_LowestQualityStreamProfile;
		}

		params = new Json ();
		params->set ("mediaId", ui->mediaId);
		params->set ("mediaServerAgentId", ui->agentId);
		params->set ("streamName", ui->mediaName);
		params->set ("mediaWidth", ui->frameWidth);
		params->set ("mediaHeight", ui->frameHeight);
		params->set ("profile", profile);

		// TODO: Populate the mediaUrl field (to allow streams to be populated on a stream server other than the source agent)
		//	params->set ("mediaUrl", url);

		ui->retain ();
		result = App::instance->agentControl.invokeCommand (ui->agentId, App::instance->createCommand (SystemInterface::Command_ConfigureMediaStream, SystemInterface::Constant_Stream, params), MediaItemUi::configureMediaStreamComplete, ui);
		if (result != Result::Success) {
			ui->release ();
			Log::debug ("Failed to invoke ConfigureMediaStream command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		}
	}
	ui->actionWidget.clear ();
}

void MediaItemUi::configureMediaStreamComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	MediaItemUi *ui;
	UiText *uitext;
	SystemInterface *interface;
	StdString recordid, agentname, text;

	ui = (MediaItemUi *) uiPtr;
	uitext = &(App::instance->uiText);
	interface = &(App::instance->systemInterface);
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::CommandId_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		recordid = interface->getCommandStringParam (responseCommand, "taskId", "");
		// TODO: Watch the task for events
/*
		if (! recordid.empty ()) {
			params = new Json ();
			params->set ("recordId", recordid);
			App::instance->agentControl.writeLinkCommand (App::instance->createCommandJson (SystemInterface::Command_WatchEvents, SystemInterface::Constant_Link, params));
		}
*/

		agentname = App::instance->agentControl.getAgentDisplayName (agentId);
		if (! agentname.empty ()) {
			text.appendSprintf ("%s: ", agentname.c_str ());
		}
		text.appendSprintf ("%s, %s", uitext->getText (UiTextString::startedCreatingStream).capitalized ().c_str (), ui->mediaName.c_str ());
		App::instance->uiStack.showSnackbar (text);
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}

	ui->release ();
}

void MediaItemUi::removeMediaActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ActionWindow *action;
	Json *params;
	int result;

	ui = (MediaItemUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (action->isConfirmed && (! ui->agentId.empty ())) {
		params = new Json ();
		params->set ("id", ui->mediaId);

		ui->retain ();
		result = App::instance->agentControl.invokeCommand (ui->agentId, App::instance->createCommand (SystemInterface::Command_RemoveMedia, SystemInterface::Constant_Media, params), MediaItemUi::removeMediaComplete, ui);
		if (result != Result::Success) {
			ui->release ();
			Log::debug ("Failed to invoke RemoveMedia command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		}
	}
	ui->actionWidget.clear ();
}

void MediaItemUi::removeMediaComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	MediaItemUi *ui;
	SystemInterface *interface;

	ui = (MediaItemUi *) uiPtr;
	interface = &(App::instance->systemInterface);
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::CommandId_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false)) {
		App::instance->agentControl.recordStore.removeRecord (ui->mediaId);
		if (ui->removeMediaCallback) {
			ui->removeMediaCallback (ui->removeMediaCallbackData, NULL);
		}
		App::instance->uiStack.popUi ();
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}

	ui->release ();
}
