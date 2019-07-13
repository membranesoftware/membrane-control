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
#include "HashMap.h"
#include "Ui.h"
#include "UiText.h"
#include "Toolbar.h"
#include "Menu.h"
#include "Chip.h"
#include "CardView.h"
#include "HelpWindow.h"
#include "MediaTimelineWindow.h"
#include "StreamDetailWindow.h"
#include "MediaThumbnailWindow.h"
#include "StreamItemUi.h"

StreamItemUi::StreamItemUi (const StdString &streamId, const StdString &streamName, const StdString &captionText)
: Ui ()
, streamId (streamId)
, streamName (streamName)
, duration (0)
, frameWidth (0)
, frameHeight (0)
, segmentCount (0)
, captionText (captionText)
, isRecordSynced (false)
, cardView (NULL)
, cardDetail (-1)
, lastTimelineHoverPosition (-1)
, thumbnailClickCallback (NULL)
, thumbnailClickCallbackData (NULL)
{

}

StreamItemUi::~StreamItemUi () {

}

void StreamItemUi::setThumbnailClickCallback (Widget::EventCallback callback, void *callbackData) {
	thumbnailClickCallback = callback;
	thumbnailClickCallbackData = callbackData;
}

StdString StreamItemUi::getSpritePath () {
	return (StdString ("ui/StreamItemUi/sprite"));
}

Widget *StreamItemUi::createBreadcrumbWidget () {
	return (new Chip (streamName));
}

void StreamItemUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::streamItemUiHelpTitle), uitext->getText (UiTextString::streamItemUiHelpText));
	helpWindow->addAction (uitext->getText (UiTextString::streamItemUiHelpAction1Text));

	helpWindow->addTopicLink (uitext->getText (UiTextString::mediaStreaming).capitalized (), App::getHelpUrl ("media-streaming"));
	helpWindow->addTopicLink (uitext->getText (UiTextString::searchForHelp).capitalized (), App::getHelpUrl (""));
}

int StreamItemUi::doLoad () {
	UiConfiguration *uiconfig;
	StreamDetailWindow *detailwindow;

	uiconfig = &(App::instance->uiConfig);
	cardDetail = App::instance->prefsMap.find (App::StreamItemUiImageSizeKey, (int) CardView::MediumDetail);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setItemMarginSize (uiconfig->marginSize / 2.0f);
	cardView->setRowDetail (StreamItemUi::ImageRow, cardDetail);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	detailwindow = new StreamDetailWindow (streamId, &sprites);
	cardView->addItem (detailwindow, streamId, StreamItemUi::InfoRow);

	lastTimelineHoverPosition = -1;

	return (Result::Success);
}

void StreamItemUi::doUnload () {
	timelineWindow.clear ();
}

void StreamItemUi::doAddMainToolbarItems (Toolbar *toolbar) {
	UiText *uitext;
	Button *button;

	uitext = &(App::instance->uiText);

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (StreamItemUi::imageSizeButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);
}

void StreamItemUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	MediaTimelineWindow *timeline;
	Label *label;

	uiconfig = &(App::instance->uiConfig);

	toolbar->setLeftCorner (new Image (sprites.getSprite (StreamItemUi::TimeIconSprite)));

	timeline = new MediaTimelineWindow (App::instance->windowWidth * 0.5f, streamId);
	timeline->setPositionHoverCallback (StreamItemUi::timelineWindowPositionHovered, this);
	timeline->setPositionClickCallback (StreamItemUi::timelineWindowPositionClicked, this);
	toolbar->addLeftItem (timeline);
	timelineWindow.assign (timeline);

	if (! captionText.empty ()) {
		label = new Label (captionText, UiConfiguration::CaptionFont, uiconfig->mediumSecondaryColor);
		toolbar->addRightItem (label);
	}
}

void StreamItemUi::doClearPopupWidgets () {

}

void StreamItemUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void StreamItemUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/StreamItemUi/bg");

	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void StreamItemUi::doPause () {

}

void StreamItemUi::doUpdate (int msElapsed) {
	timelineWindow.compact ();
}

void StreamItemUi::doSyncRecordStore () {
	if (! isRecordSynced) {
		syncStreamItem ();
	}
	if (timelineWindow.widget) {
		timelineWindow.widget->syncRecordStore ();
	}
	cardView->syncRecordStore ();
}

void StreamItemUi::syncStreamItem () {
	RecordStore *store;
	SystemInterface *interface;
	AgentControl *agentcontrol;
	Json *streamitem, *agentstatus, serverstatus, *params;
	MediaThumbnailWindow *thumbnail;
	StdString url, cardid;
	float t;
	int i;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	agentcontrol = &(App::instance->agentControl);

	streamitem = store->findRecord (streamId, SystemInterface::CommandId_StreamItem);
	if (! streamitem) {
		return;
	}

	agentId = interface->getCommandAgentId (streamitem);
	if (agentId.empty ()) {
		return;
	}

	agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentId);
	if (! agentstatus) {
		return;
	}
	if (interface->getCommandObjectParam (agentstatus, "streamServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
	}
	else if (interface->getCommandObjectParam (agentstatus, "monitorServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
	}

	segmentCount = interface->getCommandNumberParam (streamitem, "segmentCount", (int) 0);
	interface->getCommandNumberArrayParam (streamitem, "segmentPositions", &segmentPositions, true);
	duration = interface->getCommandNumberParam (streamitem, "duration", (int64_t) 0);
	frameWidth = interface->getCommandNumberParam (streamitem, "width", (int) 0);
	frameHeight = interface->getCommandNumberParam (streamitem, "height", (int) 0);

	if ((segmentCount > 0) && (frameWidth > 0) && (frameHeight > 0) && (! thumbnailPath.empty ())) {
		for (i = 0; i < segmentCount; ++i) {
			params = new Json ();
			params->set ("id", streamId);
			params->set ("thumbnailIndex", i);
			url = agentcontrol->getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, SystemInterface::Constant_Stream, params), thumbnailPath);

			cardid.sprintf ("%i", i);
			if (i < (int) segmentPositions.size ()) {
				t = ((float) segmentPositions.at (i)) * 1000.0f;
			}
			else {
				t = i * ((float) duration / (float) segmentCount);
			}
			thumbnail = new MediaThumbnailWindow (i, t, frameWidth, frameHeight, url);
			thumbnail->setMouseClickCallback (StreamItemUi::thumbnailClicked, this);
			thumbnail->setMouseEnterCallback (StreamItemUi::thumbnailMouseEntered, this);
			thumbnail->setMouseExitCallback (StreamItemUi::thumbnailMouseExited, this);
			cardView->addItem (thumbnail, cardid, StreamItemUi::ImageRow, true);
		}
		cardView->refresh ();
	}

	isRecordSynced = true;
}

void StreamItemUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void StreamItemUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Menu *menu;
	bool show;

	ui = (StreamItemUi *) uiPtr;
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
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), StreamItemUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), StreamItemUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), StreamItemUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);
	menu->position.assign (widgetPtr->position.x + widgetPtr->width - menu->width, widgetPtr->position.y + widgetPtr->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void StreamItemUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	int detail;

	ui = (StreamItemUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (StreamItemUi::ImageRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::StreamItemUiImageSizeKey, ui->cardDetail);
}

void StreamItemUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	int detail;

	ui = (StreamItemUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (StreamItemUi::ImageRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::StreamItemUiImageSizeKey, ui->cardDetail);
}

void StreamItemUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	int detail;

	ui = (StreamItemUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (StreamItemUi::ImageRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::StreamItemUiImageSizeKey, ui->cardDetail);
}

void StreamItemUi::thumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	MediaThumbnailWindow *target;

	ui = (StreamItemUi *) uiPtr;
	target = MediaThumbnailWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	if (ui->thumbnailClickCallback) {
		ui->thumbnailClickCallback (ui->thumbnailClickCallbackData, target);
	}
}

void StreamItemUi::thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	MediaThumbnailWindow *thumbnail;
	MediaTimelineWindow *timeline;

	ui = (StreamItemUi *) uiPtr;
	thumbnail = (MediaThumbnailWindow *) widgetPtr;
	timeline = (MediaTimelineWindow *) ui->timelineWindow.widget;
	if (timeline) {
		timeline->setHighlightedMarker (thumbnail->thumbnailIndex);
	}
	thumbnail->setHighlighted (true);
}

void StreamItemUi::thumbnailMouseExited (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	MediaThumbnailWindow *thumbnail;
	MediaTimelineWindow *timeline;

	ui = (StreamItemUi *) uiPtr;
	thumbnail = (MediaThumbnailWindow *) widgetPtr;
	timeline = (MediaTimelineWindow *) ui->timelineWindow.widget;
	if (timeline && (timeline->highlightedMarkerIndex == thumbnail->thumbnailIndex)) {
		timeline->setHighlightedMarker (-1);
	}
	thumbnail->setHighlighted (false);
}

void StreamItemUi::timelineWindowPositionHovered (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	MediaTimelineWindow *timeline;
	MediaThumbnailWindow *target;
	float pos;
	int index;

	ui = (StreamItemUi *) uiPtr;
	timeline = (MediaTimelineWindow *) widgetPtr;

	pos = timeline->hoverPosition;
	if (pos < 0.0f) {
		index = -1;
	}
	else {
		index = (int) (pos / timeline->width * (float) ui->segmentCount);
	}

	if (ui->lastTimelineHoverPosition != index) {
		timeline->setHighlightedMarker (index);

		if (ui->lastTimelineHoverPosition >= 0) {
			target = (MediaThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &(ui->lastTimelineHoverPosition));
			if (target) {
				target->setHighlighted (false);
			}
		}

		ui->lastTimelineHoverPosition = index;
		if (index >= 0) {
			target = (MediaThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &index);
			if (target) {
				target->setHighlighted (true);
			}
		}
	}
}

void StreamItemUi::timelineWindowPositionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	MediaTimelineWindow *timeline;
	MediaThumbnailWindow *target;
	int index;

	ui = (StreamItemUi *) uiPtr;
	timeline = (MediaTimelineWindow *) widgetPtr;
	if (ui->thumbnailClickCallback) {
		index = (int) (timeline->clickPosition / timeline->width * (float) ui->segmentCount);
		target = (MediaThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &index);
		if (target) {
			ui->thumbnailClickCallback (ui->thumbnailClickCallbackData, target);
		}
	}
}

bool StreamItemUi::matchThumbnailIndex (void *intPtr, Widget *widgetPtr) {
	MediaThumbnailWindow *item;
	int *index;

	item = MediaThumbnailWindow::castWidget (widgetPtr);
	if (! item) {
		return (false);
	}

	index = (int *) intPtr;
	return (*index == item->thumbnailIndex);
}
