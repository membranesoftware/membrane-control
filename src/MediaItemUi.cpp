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
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Resource.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "SpriteGroup.h"
#include "AgentControl.h"
#include "Button.h"
#include "SystemInterface.h"
#include "Menu.h"
#include "Ui.h"
#include "UiConfiguration.h"
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
#include "IconLabelWindow.h"
#include "MediaDetailWindow.h"
#include "MediaItemUi.h"

const char *MediaItemUi::VideoQualityKey = "MediaItem_VideoQuality";
const char *MediaItemUi::ImageSizeKey = "MediaItem_ImageSize";

MediaItemUi::MediaItemUi (const StdString &mediaId, const StdString &mediaName)
: Ui ()
, mediaId (mediaId)
, mediaName (mediaName)
, duration (0)
, bitrate (0)
, frameWidth (0)
, frameHeight (0)
, thumbnailCount (0)
, mediaSize (0)
, isRecordSynced (false)
, cardDetail (-1)
, isCreateStreamAvailable (true)
{
	streamServerAgentMap.sort (HashMap::sortAscending);
}

MediaItemUi::~MediaItemUi () {

}

StdString MediaItemUi::getSpritePath () {
	return (StdString ("ui/MediaItemUi/sprite"));
}

Widget *MediaItemUi::createBreadcrumbWidget () {
	return (new Chip (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (mediaName, ((float) App::instance->windowWidth) * 0.5f, Font::DotTruncateSuffix)));
}

void MediaItemUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextString::MediaItemUiHelpTitle), UiText::instance->getText (UiTextString::MediaItemUiHelpText));
	if (isCreateStreamAvailable) {
		helpWindow->addAction (UiText::instance->getText (UiTextString::MediaItemUiHelpAction1Text));
	}
	else {
		helpWindow->addAction (UiText::instance->getText (UiTextString::MediaItemUiHelpAction2Text));
	}
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::MediaStreaming).capitalized (), App::getHelpUrl ("media-streaming"));
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

int MediaItemUi::doLoad () {
	HashMap *prefs;
	MediaDetailWindow *detailwindow;

	prefs = App::instance->lockPrefs ();
	cardDetail = prefs->find (MediaItemUi::ImageSizeKey, (int) CardView::MediumDetail);
	App::instance->unlockPrefs ();

	cardView->setRowItemMarginSize (MediaItemUi::ImageRow, UiConfiguration::instance->marginSize / 2.0f);
	cardView->setRowDetail (MediaItemUi::ImageRow, cardDetail);

	detailwindow = new MediaDetailWindow (mediaId, &sprites);
	cardView->addItem (detailwindow, mediaId, MediaItemUi::InfoRow);

	cardView->refresh ();
	return (OsUtil::Result::Success);
}

void MediaItemUi::doUnload () {
	streamServerAgentMap.clear ();
	timelineWindow.clear ();
	configureStreamSizeIcon.clear ();
}

void MediaItemUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaItemUi::imageSizeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ThumbnailImageSizeTooltip));
	toolbar->addRightItem (button);
}

void MediaItemUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	Button *button;
	MediaTimelineWindow *timeline;

	toolbar->setLeftCorner (new Image (sprites.getSprite (MediaItemUi::TimeIconSprite)));

	timeline = new MediaTimelineWindow (App::instance->windowWidth * 0.5f, mediaId);
	toolbar->addLeftItem (timeline);
	timelineWindow.assign (timeline);

	button = new Button (sprites.getSprite (MediaItemUi::ConfigureStreamButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (MediaItemUi::configureStreamButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaItemUiConfigureStreamTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (button);
}

void MediaItemUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/MediaItemUi/bg");
}

void MediaItemUi::doPause () {

}

void MediaItemUi::doUpdate (int msElapsed) {
	configureStreamSizeIcon.compact ();
}

void MediaItemUi::doSyncRecordStore () {
	MediaTimelineWindow *timeline;

	RecordStore::instance->populateAgentMap (&streamServerAgentMap, StdString ("streamServerStatus"));

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
	MediaThumbnailWindow *thumbnail;
	int i;
	float dt;
	Json *mediaitem, *agentstatus, serverstatus, *params;
	StdString cardid, url, cmdjson;

	mediaitem = RecordStore::instance->findRecord (mediaId, SystemInterface::CommandId_MediaItem);
	if (! mediaitem) {
		return;
	}
	agentId = SystemInterface::instance->getCommandAgentId (mediaitem);
	if (agentId.empty ()) {
		return;
	}

	agentstatus = RecordStore::instance->findRecord (RecordStore::matchAgentStatusSource, &agentId);
	if (! agentstatus) {
		return;
	}
	if (SystemInterface::instance->getCommandObjectParam (agentstatus, "mediaServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
		thumbnailCount = serverstatus.getNumber ("thumbnailCount", (int) 0);
	}
	duration = SystemInterface::instance->getCommandNumberParam (mediaitem, "duration", (int64_t) 0);
	bitrate = SystemInterface::instance->getCommandNumberParam (mediaitem, "bitrate", (int64_t) 0);
	frameWidth = SystemInterface::instance->getCommandNumberParam (mediaitem, "width", (int) 0);
	frameHeight = SystemInterface::instance->getCommandNumberParam (mediaitem, "height", (int) 0);
	mediaSize = SystemInterface::instance->getCommandNumberParam (mediaitem, "size", (int64_t) 0);
	isCreateStreamAvailable = SystemInterface::instance->getCommandBooleanParam (mediaitem, "isCreateStreamAvailable", true);

	if ((thumbnailCount > 0) && (frameWidth > 0) && (frameHeight > 0) && (! thumbnailPath.empty ())) {
		dt = (float) duration / (float) thumbnailCount;
		for (i = 0; i < thumbnailCount; ++i) {
			params = new Json ();
			params->set ("id", mediaId);
			params->set ("thumbnailIndex", i);
			url = AgentControl::instance->getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, params), thumbnailPath);

			cardid.sprintf ("%i", i);
			thumbnail = new MediaThumbnailWindow (i, (dt / 2.0f) + ((float) i * dt), frameWidth, frameHeight, url);
			thumbnail->mouseEnterCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailMouseEntered, this);
			thumbnail->mouseExitCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailMouseExited, this);
			cardView->addItem (thumbnail, cardid, MediaItemUi::ImageRow, true);
		}

		cardView->refresh ();
	}

	isRecordSynced = true;
}

void MediaItemUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	Menu *menu;

	ui = (MediaItemUi *) uiPtr;
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, MediaItemUi::imageSizeButtonClicked)) {
		return;
	}

	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextString::Small).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), MediaItemUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Medium).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), MediaItemUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Large).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), MediaItemUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);

	ui->showActionPopup (menu, widgetPtr, MediaItemUi::imageSizeButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::BottomOfAlignment);
}

void MediaItemUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	HashMap *prefs;
	int detail;

	ui = (MediaItemUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaItemUi::ImageRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaItemUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void MediaItemUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	HashMap *prefs;
	int detail;

	ui = (MediaItemUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaItemUi::ImageRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaItemUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void MediaItemUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	HashMap *prefs;
	int detail;

	ui = (MediaItemUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (MediaItemUi::ImageRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaItemUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
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
	HashMap *prefs;
	ActionWindow *action;
	ComboBox *combobox;
	IconLabelWindow *icon;
	int profile;

	ui = (MediaItemUi *) uiPtr;
	if (ui->streamServerAgentMap.empty ()) {
		// TODO: Show an error message here
		return;
	}
	if (ui->clearActionPopup (widgetPtr, MediaItemUi::configureStreamButtonClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->setInverseColor (true);
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);

	if (ui->isCreateStreamAvailable) {
		action->setTitleText (UiText::instance->getText (UiTextString::ConfigureStream).capitalized ());
		action->setConfirmTooltipText (UiText::instance->getText (UiTextString::Apply).capitalized ());
		action->optionChangeCallback = Widget::EventCallbackContext (MediaItemUi::configureStreamOptionChanged, ui);
		action->closeCallback = Widget::EventCallbackContext (MediaItemUi::configureStreamActionClosed, ui);

	// TODO: Allow selection of a stream server (currently restricted to the server holding the source media item)
	//	action->addComboBoxOption (UiText::instance->getText (UiTextString::streamServer).capitalized (), &(ui->streamServerAgentMap));

		prefs = App::instance->lockPrefs ();
		profile = prefs->find (MediaItemUi::VideoQualityKey, SystemInterface::Constant_DefaultStreamProfile);
		App::instance->unlockPrefs ();

		combobox = new ComboBox ();
		combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_SourceMatchStreamProfile));
		combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_HighBitrateStreamProfile));
		combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_MediumBitrateStreamProfile));
		combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_LowBitrateStreamProfile));
		combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_LowestBitrateStreamProfile));
		combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_PreviewStreamProfile));
		combobox->addItem (MediaUtil::getStreamProfileName (SystemInterface::Constant_FastPreviewStreamProfile));
		combobox->setValue (MediaUtil::getStreamProfileName (profile));
		action->addOption (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), combobox, MediaUtil::getStreamProfileDescription (profile));

		icon = new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), OsUtil::getByteCountDisplayString (MediaUtil::getStreamSize (ui->mediaSize, ui->bitrate, ui->duration, profile)), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor);
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ConfigureStreamByteCountTooltip));
		ui->configureStreamSizeIcon.assign (icon);
		action->setFooterPanel (icon);
	}
	else {
		action->setTitleText (UiText::instance->getText (UiTextString::RemoveMedia).capitalized ());
		action->setDescriptionText (UiText::instance->getText (UiTextString::RemoveMediaActionText));
		action->setConfirmTooltipText (UiText::instance->getText (UiTextString::Remove).capitalized ());
		action->closeCallback = Widget::EventCallbackContext (MediaItemUi::removeMediaActionClosed, ui);
	}

	ui->showActionPopup (action, widgetPtr, MediaItemUi::configureStreamButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void MediaItemUi::configureStreamOptionChanged (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ActionWindow *action;
	IconLabelWindow *icon;
	StdString text;
	int profile;

	ui = (MediaItemUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	icon = (IconLabelWindow *) ui->configureStreamSizeIcon.widget;
	if (icon) {
		icon->setText (OsUtil::getByteCountDisplayString (MediaUtil::getStreamSize (ui->mediaSize, ui->bitrate, ui->duration, MediaUtil::getStreamProfile (action->getStringValue (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), "")))));
	}

	profile = MediaUtil::getStreamProfile (action->getStringValue (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), ""));
	text = MediaUtil::getStreamProfileDescription (profile);
	if (! text.empty ()) {
		action->setOptionDescriptionText (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), text);
	}
	action->refresh ();
}

void MediaItemUi::configureStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ActionWindow *action;
	HashMap *prefs;
	Json *params;
	int result, profile;

	ui = (MediaItemUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (action->isConfirmed && (! ui->agentId.empty ())) {
		profile = MediaUtil::getStreamProfile (action->getStringValue (UiText::instance->getText (UiTextString::VideoQuality).capitalized (), ""));
		prefs = App::instance->lockPrefs ();
		if (profile != SystemInterface::Constant_DefaultStreamProfile) {
			prefs->insert (MediaItemUi::VideoQualityKey, profile);
		}
		else {
			prefs->remove (MediaItemUi::VideoQualityKey);
		}
		App::instance->unlockPrefs ();

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
		result = AgentControl::instance->invokeCommand (ui->agentId, App::instance->createCommand (SystemInterface::Command_ConfigureMediaStream, params), MediaItemUi::configureMediaStreamComplete, ui);
		if (result != OsUtil::Result::Success) {
			ui->release ();
			Log::debug ("Failed to invoke ConfigureMediaStream command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
			App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::InternalError));
		}
	}
}

void MediaItemUi::configureMediaStreamComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	MediaItemUi *ui;
	StdString agentname, text;

	ui = (MediaItemUi *) uiPtr;
	if (responseCommand && (SystemInterface::instance->getCommandId (responseCommand) == SystemInterface::CommandId_CommandResult) && SystemInterface::instance->getCommandBooleanParam (responseCommand, "success", false)) {
		agentname = AgentControl::instance->getAgentDisplayName (agentId);
		if (! agentname.empty ()) {
			text.appendSprintf ("%s: ", agentname.c_str ());
		}
		text.appendSprintf ("%s, %s", UiText::instance->getText (UiTextString::StartedCreatingStream).capitalized ().c_str (), ui->mediaName.c_str ());
		App::instance->uiStack.showSnackbar (text);
	}
	else {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::InternalError));
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
		result = AgentControl::instance->invokeCommand (ui->agentId, App::instance->createCommand (SystemInterface::Command_RemoveMedia, params), MediaItemUi::removeMediaComplete, ui);
		if (result != OsUtil::Result::Success) {
			ui->release ();
			Log::debug ("Failed to invoke RemoveMedia command; err=%i agentId=\"%s\"", result, ui->agentId.c_str ());
			App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::InternalError));
		}
	}
}

void MediaItemUi::removeMediaComplete (void *uiPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	MediaItemUi *ui;

	ui = (MediaItemUi *) uiPtr;
	if (responseCommand && (SystemInterface::instance->getCommandId (responseCommand) == SystemInterface::CommandId_CommandResult) && SystemInterface::instance->getCommandBooleanParam (responseCommand, "success", false)) {
		RecordStore::instance->removeRecord (ui->mediaId);
		if (ui->removeMediaCallback.callback) {
			ui->removeMediaCallback.callback (ui->removeMediaCallback.callbackData, NULL);
		}
		App::instance->uiStack.popUi ();
	}
	else {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::InternalError));
	}

	ui->release ();
}
