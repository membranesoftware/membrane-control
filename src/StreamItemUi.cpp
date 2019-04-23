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
#include "TimelineWindow.h"
#include "StreamDetailWindow.h"
#include "ThumbnailWindow.h"
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
, cardLayout (-1)
, cardMaxImageWidth (0.0f)
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
	int layout;

	uiconfig = &(App::instance->uiConfig);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setItemMarginSize (uiconfig->marginSize / 2.0f);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	layout = App::instance->prefsMap.find (App::StreamItemImageSizeKey, (int) ThumbnailWindow::MediumDetailLayout);
	switch (layout) {
		case ThumbnailWindow::LowDetailLayout: {
			StreamItemUi::viewSmallActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::MediumDetailLayout: {
			StreamItemUi::viewMediumActionClicked (this, NULL);
			break;
		}
		case ThumbnailWindow::HighDetailLayout: {
			StreamItemUi::viewLargeActionClicked (this, NULL);
			break;
		}
		default: {
			StreamItemUi::viewMediumActionClicked (this, NULL);
			break;
		}
	}

	detailwindow = new StreamDetailWindow (streamId, &sprites);
	cardView->addItem (detailwindow, streamId, 0);

	lastTimelineHoverPosition = -1;

	return (Result::Success);
}

void StreamItemUi::doUnload () {

}

void StreamItemUi::doAddMainToolbarItems (Toolbar *toolbar) {
	UiText *uitext;
	Button *button;

	uitext = &(App::instance->uiText);

	button = new Button (StdString (""), sprites.getSprite (StreamItemUi::ThumbnailSizeButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (StreamItemUi::thumbnailSizeButtonClicked, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);
}

void StreamItemUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	TimelineWindow *timeline;
	Label *label;

	uiconfig = &(App::instance->uiConfig);

	toolbar->setLeftCorner (new Image (sprites.getSprite (StreamItemUi::TimeIconSprite)));

	timeline = new TimelineWindow (App::instance->windowWidth * 0.5f, streamId);
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
	App::instance->prefsMap.insert (App::StreamItemImageSizeKey, cardLayout);
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
	ThumbnailWindow *thumbnail;
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
			thumbnail = new ThumbnailWindow (i, t, frameWidth, frameHeight, url, cardLayout, cardMaxImageWidth);
			thumbnail->setMouseClickCallback (StreamItemUi::thumbnailClicked, this);
			thumbnail->setMouseEnterCallback (StreamItemUi::thumbnailMouseEntered, this);
			thumbnail->setMouseExitCallback (StreamItemUi::thumbnailMouseExited, this);
			cardView->addItem (thumbnail, cardid, 1, true);
		}
		cardView->refresh ();
	}

	isRecordSynced = true;
}

void StreamItemUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void StreamItemUi::thumbnailSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	UiText *uitext;
	Menu *menu;
	bool show;

	ui = (StreamItemUi *) uiPtr;
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
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), ui->sprites.getSprite (StreamItemUi::SmallThumbnailsIconSprite), StreamItemUi::viewSmallActionClicked, ui, 0, ui->cardLayout == ThumbnailWindow::LowDetailLayout);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), ui->sprites.getSprite (StreamItemUi::MediumThumbnailsIconSprite), StreamItemUi::viewMediumActionClicked, ui, 0, ui->cardLayout == ThumbnailWindow::MediumDetailLayout);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), ui->sprites.getSprite (StreamItemUi::LargeThumbnailsIconSprite), StreamItemUi::viewLargeActionClicked, ui, 0, ui->cardLayout == ThumbnailWindow::HighDetailLayout);
	menu->position.assign (widgetPtr->position.x + widgetPtr->width - menu->width, widgetPtr->position.y + widgetPtr->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void StreamItemUi::viewSmallActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (StreamItemUi *) uiPtr;
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
	ui->cardView->processItems (StreamItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.1f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void StreamItemUi::viewMediumActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (StreamItemUi *) uiPtr;
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
	ui->cardView->processItems (StreamItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (App::instance->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.2f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void StreamItemUi::viewLargeActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	UiConfiguration *uiconfig;
	float y0, h0;

	ui = (StreamItemUi *) uiPtr;
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
	ui->cardView->processItems (StreamItemUi::resetCardLayout, ui);
	ui->cardView->setItemMarginSize (App::instance->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.3f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void StreamItemUi::resetCardLayout (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ThumbnailWindow *card;

	ui = (StreamItemUi *) uiPtr;
	card = ThumbnailWindow::castWidget (widgetPtr);
	if (! card) {
		return;
	}

	card->setLayout (ui->cardLayout, ui->cardMaxImageWidth);
}

void StreamItemUi::thumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ThumbnailWindow *target;

	ui = (StreamItemUi *) uiPtr;
	target = ThumbnailWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	if (ui->thumbnailClickCallback) {
		ui->thumbnailClickCallback (ui->thumbnailClickCallbackData, target);
	}
}

void StreamItemUi::thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ThumbnailWindow *thumbnail;
	TimelineWindow *timeline;

	ui = (StreamItemUi *) uiPtr;
	thumbnail = (ThumbnailWindow *) widgetPtr;
	timeline = (TimelineWindow *) ui->timelineWindow.widget;
	if (timeline) {
		timeline->setHighlightedMarker (thumbnail->thumbnailIndex);
	}
	thumbnail->setHighlighted (true);
}

void StreamItemUi::thumbnailMouseExited (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ThumbnailWindow *thumbnail;
	TimelineWindow *timeline;

	ui = (StreamItemUi *) uiPtr;
	thumbnail = (ThumbnailWindow *) widgetPtr;
	timeline = (TimelineWindow *) ui->timelineWindow.widget;
	if (timeline && (timeline->highlightedMarkerIndex == thumbnail->thumbnailIndex)) {
		timeline->setHighlightedMarker (-1);
	}
	thumbnail->setHighlighted (false);
}

void StreamItemUi::timelineWindowPositionHovered (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	TimelineWindow *timeline;
	ThumbnailWindow *target;
	float pos;
	int index;

	ui = (StreamItemUi *) uiPtr;
	timeline = (TimelineWindow *) widgetPtr;

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
			target = (ThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &(ui->lastTimelineHoverPosition));
			if (target) {
				target->setHighlighted (false);
			}
		}

		ui->lastTimelineHoverPosition = index;
		if (index >= 0) {
			target = (ThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &index);
			if (target) {
				target->setHighlighted (true);
			}
		}
	}
}

void StreamItemUi::timelineWindowPositionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	TimelineWindow *timeline;
	ThumbnailWindow *target;
	int index;

	ui = (StreamItemUi *) uiPtr;
	timeline = (TimelineWindow *) widgetPtr;
	if (ui->thumbnailClickCallback) {
		index = (int) (timeline->clickPosition / timeline->width * (float) ui->segmentCount);
		target = (ThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &index);
		if (target) {
			ui->thumbnailClickCallback (ui->thumbnailClickCallbackData, target);
		}
	}
}

bool StreamItemUi::matchThumbnailIndex (void *intPtr, Widget *widgetPtr) {
	ThumbnailWindow *item;
	int *index;

	item = ThumbnailWindow::castWidget (widgetPtr);
	if (! item) {
		return (false);
	}

	index = (int *) intPtr;
	return (*index == item->thumbnailIndex);
}
