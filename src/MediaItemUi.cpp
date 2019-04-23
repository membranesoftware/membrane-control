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
#include "TimelineWindow.h"
#include "CardView.h"
#include "ComboBox.h"
#include "HelpWindow.h"
#include "IconCardWindow.h"
#include "ThumbnailWindow.h"
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
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
, timelineWindow (NULL)
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
	int layout;

	uiconfig = &(App::instance->uiConfig);
	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowItemMarginSize (1, uiconfig->marginSize / 2.0f);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	layout = App::instance->prefsMap.find (App::MediaItemImageSizeKey, (int) ThumbnailWindow::MediumDetailLayout);
	switch (layout) {
		case ThumbnailWindow::LowDetailLayout: {
			MediaItemUi::smallThumbnailActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::MediumDetailLayout: {
			MediaItemUi::mediumThumbnailActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::HighDetailLayout: {
			MediaItemUi::largeThumbnailActionClicked (this, NULL);
			break;
		}
		default: {
			MediaItemUi::mediumThumbnailActionClicked (this, NULL);
			break;
		}
	}

	detailwindow = new MediaDetailWindow (mediaId, &sprites);
	cardView->addItem (detailwindow, mediaId, 0);

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

	button = new Button (StdString (""), sprites.getSprite (MediaItemUi::ThumbnailSizeButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (MediaItemUi::thumbnailSizeButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);
}

void MediaItemUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiText *uitext;
	Button *button;

	uitext = &(App::instance->uiText);
	toolbar->setLeftCorner (new Image (sprites.getSprite (MediaItemUi::TimeIconSprite)));

	timelineWindow = new TimelineWindow (App::instance->windowWidth * 0.5f, mediaId);
	toolbar->addLeftItem (timelineWindow);

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
	App::instance->prefsMap.insert (App::MediaItemImageSizeKey, cardLayout);
}

void MediaItemUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void MediaItemUi::doSyncRecordStore () {
	RecordStore *store;

	store = &(App::instance->agentControl.recordStore);
	store->populateAgentMap (&streamServerAgentMap, StdString ("streamServerStatus"));

	if (! isRecordSynced) {
		syncMediaItem ();
	}

	timelineWindow->syncRecordStore ();
	cardView->syncRecordStore ();
	cardView->refresh ();
}

void MediaItemUi::syncMediaItem () {
	RecordStore *store;
	SystemInterface *interface;
	AgentControl *agentcontrol;
	ThumbnailWindow *thumbnail;
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

	if ((thumbnailCount > 0) && (frameWidth > 0) && (frameHeight > 0) && (! thumbnailPath.empty ())) {
		dt = (float) duration / (float) thumbnailCount;
		for (i = 0; i < thumbnailCount; ++i) {
			params = new Json ();
			params->set ("id", mediaId);
			params->set ("thumbnailIndex", i);
			url = agentcontrol->getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, SystemInterface::Constant_Media, params), thumbnailPath);

			cardid.sprintf ("%i", i);
			thumbnail = new ThumbnailWindow (i, (dt / 2.0f) + ((float) i * dt), frameWidth, frameHeight, url, cardLayout, cardMaxImageWidth);
			thumbnail->setMouseEnterCallback (MediaItemUi::thumbnailMouseEntered, this);
			thumbnail->setMouseExitCallback (MediaItemUi::thumbnailMouseExited, this);
			cardView->addItem (thumbnail, cardid, 1, true);
		}

		cardView->refresh ();
	}

	isRecordSynced = true;
}

void MediaItemUi::thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	UiText *uitext;
	Menu *menu;
	bool show;

	ui = (MediaItemUi *) uiPtr;
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
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), ui->sprites.getSprite (MediaItemUi::SmallThumbnailsIconSprite), MediaItemUi::smallThumbnailActionClicked, ui, 0, ui->cardLayout == ThumbnailWindow::LowDetailLayout);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), ui->sprites.getSprite (MediaItemUi::MediumThumbnailsIconSprite), MediaItemUi::mediumThumbnailActionClicked, ui, 0, ui->cardLayout == ThumbnailWindow::MediumDetailLayout);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), ui->sprites.getSprite (MediaItemUi::LargeThumbnailsIconSprite), MediaItemUi::largeThumbnailActionClicked, ui, 0, ui->cardLayout == ThumbnailWindow::HighDetailLayout);
	menu->position.assign (widgetPtr->position.x + widgetPtr->width - menu->width, widgetPtr->position.y + widgetPtr->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void MediaItemUi::smallThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MediaItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::LowDetailLayout) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::LowDetailLayout;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->smallThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaItemUi::resetCardLayout, ui);
	ui->cardView->setRowItemMarginSize (1, 0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.1f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void MediaItemUi::mediumThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MediaItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::MediumDetailLayout) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::MediumDetailLayout;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->mediumThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaItemUi::resetCardLayout, ui);
	ui->cardView->setRowItemMarginSize (1, App::instance->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.2f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void MediaItemUi::largeThumbnailActionClicked (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (MediaItemUi *) uiPtr;
	if (ui->cardLayout == ThumbnailWindow::HighDetailLayout) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->cardLayout = ThumbnailWindow::HighDetailLayout;
	ui->cardMaxImageWidth = ui->cardView->cardAreaWidth * uiconfig->largeThumbnailImageScale;
	if (ui->cardMaxImageWidth < 1.0f) {
		ui->cardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (MediaItemUi::resetCardLayout, ui);
	ui->cardView->setRowItemMarginSize (1, App::instance->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.3f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void MediaItemUi::thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ThumbnailWindow *window;

	ui = (MediaItemUi *) uiPtr;
	window = (ThumbnailWindow *) widgetPtr;
	ui->timelineWindow->setHighlightedMarker (window->thumbnailIndex);
}

void MediaItemUi::thumbnailMouseExited (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ThumbnailWindow *window;

	ui = (MediaItemUi *) uiPtr;
	window = (ThumbnailWindow *) widgetPtr;
	if (ui->timelineWindow->highlightedMarkerIndex == window->thumbnailIndex) {
		ui->timelineWindow->setHighlightedMarker (-1);
	}
}

void MediaItemUi::resetCardLayout (void *uiPtr, Widget *widgetPtr) {
	MediaItemUi *ui;
	ThumbnailWindow *window;

	ui = (MediaItemUi *) uiPtr;
	window = ThumbnailWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	window->setLayout (ui->cardLayout, ui->cardMaxImageWidth);
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
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::CommandId_CommandResult)) {
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

	ui->release ();
}
