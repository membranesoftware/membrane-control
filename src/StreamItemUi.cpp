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
#include "SDL2/SDL.h"
#include "App.h"
#include "Log.h"
#include "OsUtil.h"
#include "StdString.h"
#include "Resource.h"
#include "SpriteGroup.h"
#include "AgentControl.h"
#include "Button.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "HashMap.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Toolbar.h"
#include "Menu.h"
#include "Chip.h"
#include "Toggle.h"
#include "Label.h"
#include "CardView.h"
#include "LabelWindow.h"
#include "HelpWindow.h"
#include "MediaTimelineWindow.h"
#include "StreamDetailWindow.h"
#include "MediaThumbnailWindow.h"
#include "StreamItemUi.h"

const char *StreamItemUi::ImageSizeKey = "StreamItem_ImageSize";

StreamItemUi::StreamItemUi (const StdString &streamId, const StdString &streamName)
: Ui ()
, streamId (streamId)
, streamName (streamName)
, duration (0)
, frameWidth (0)
, frameHeight (0)
, segmentCount (0)
, isRecordSynced (false)
, cardDetail (-1)
, lastTimelineHoverPosition (-1)
, isSelectingPlayPosition (false)
{

}

StreamItemUi::~StreamItemUi () {

}

StdString StreamItemUi::getSpritePath () {
	return (StdString ("ui/StreamItemUi/sprite"));
}

Widget *StreamItemUi::createBreadcrumbWidget () {
	return (new Chip (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (streamName, ((float) App::instance->windowWidth) * 0.5f, Font::DotTruncateSuffix)));
}

void StreamItemUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextString::StreamItemUiHelpTitle), UiText::instance->getText (UiTextString::StreamItemUiHelpText));
	helpWindow->addAction (UiText::instance->getText (UiTextString::StreamItemUiHelpAction1Text));
	helpWindow->addAction (UiText::instance->getText (UiTextString::StreamItemUiHelpAction2Text));
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::MediaStreaming).capitalized (), App::getHelpUrl ("media-streaming"));
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

int StreamItemUi::doLoad () {
	HashMap *prefs;
	StreamDetailWindow *detailwindow;

	prefs = App::instance->lockPrefs ();
	cardDetail = prefs->find (StreamItemUi::ImageSizeKey, (int) CardView::MediumDetail);
	App::instance->unlockPrefs ();

	cardView->setItemMarginSize (UiConfiguration::instance->marginSize / 2.0f);
	cardView->setRowDetail (StreamItemUi::ImageRow, cardDetail);

	detailwindow = new StreamDetailWindow (streamId, &sprites);
	cardView->addItem (detailwindow, streamId, StreamItemUi::InfoRow);

	lastTimelineHoverPosition = -1;

	return (OsUtil::Result::Success);
}

void StreamItemUi::doUnload () {
	timelineWindow.clear ();
	commandCaption.destroyAndClear ();
}

void StreamItemUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (StreamItemUi::imageSizeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ThumbnailImageSizeTooltip));
	toolbar->addRightItem (button);
}

void StreamItemUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	MediaTimelineWindow *timeline;
	Toggle *toggle;

	toolbar->setLeftCorner (new Image (sprites.getSprite (StreamItemUi::TimeIconSprite)));

	timeline = new MediaTimelineWindow (App::instance->windowWidth * 0.5f, streamId);
	timeline->positionHoverCallback = Widget::EventCallbackContext (StreamItemUi::timelineWindowPositionHovered, this);
	timeline->positionClickCallback = Widget::EventCallbackContext (StreamItemUi::timelineWindowPositionClicked, this);
	toolbar->addLeftItem (timeline);
	timelineWindow.assign (timeline);

	toggle = new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarButtonSprite));
	toggle->shortcutKey = SDLK_TAB;
	toggle->stateChangeCallback = Widget::EventCallbackContext (StreamItemUi::selectToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setMouseHoverTooltip (UiText::instance->getText (UiTextString::StreamItemUiSelectTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (toggle);
}

void StreamItemUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/StreamItemUi/bg");
}

void StreamItemUi::doPause () {

}

void StreamItemUi::doUpdate (int msElapsed) {
	timelineWindow.compact ();
	commandCaption.compact ();
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
	Json *streamitem, *agentstatus, serverstatus, *params;
	MediaThumbnailWindow *thumbnail;
	StdString url, cardid;
	float t;
	int i;

	streamitem = RecordStore::instance->findRecord (streamId, SystemInterface::CommandId_StreamItem);
	if (! streamitem) {
		return;
	}

	agentId = SystemInterface::instance->getCommandAgentId (streamitem);
	if (agentId.empty ()) {
		return;
	}

	agentstatus = RecordStore::instance->findRecord (RecordStore::matchAgentStatusSource, &agentId);
	if (! agentstatus) {
		return;
	}
	if (SystemInterface::instance->getCommandObjectParam (agentstatus, "streamServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
	}
	else if (SystemInterface::instance->getCommandObjectParam (agentstatus, "monitorServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
	}

	segmentCount = SystemInterface::instance->getCommandNumberParam (streamitem, "segmentCount", (int) 0);
	SystemInterface::instance->getCommandNumberArrayParam (streamitem, "segmentPositions", &segmentPositions, true);
	duration = SystemInterface::instance->getCommandNumberParam (streamitem, "duration", (int64_t) 0);
	frameWidth = SystemInterface::instance->getCommandNumberParam (streamitem, "width", (int) 0);
	frameHeight = SystemInterface::instance->getCommandNumberParam (streamitem, "height", (int) 0);

	if ((segmentCount > 0) && (frameWidth > 0) && (frameHeight > 0) && (! thumbnailPath.empty ())) {
		for (i = 0; i < segmentCount; ++i) {
			params = new Json ();
			params->set ("id", streamId);
			params->set ("thumbnailIndex", i);
			url = AgentControl::instance->getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, params), thumbnailPath);

			cardid.sprintf ("%i", i);
			if (i < (int) segmentPositions.size ()) {
				t = ((float) segmentPositions.at (i)) * 1000.0f;
			}
			else {
				t = i * ((float) duration / (float) segmentCount);
			}
			thumbnail = new MediaThumbnailWindow (i, t, frameWidth, frameHeight, url);
			thumbnail->mouseClickCallback = Widget::EventCallbackContext (StreamItemUi::thumbnailClicked, this);
			thumbnail->mouseEnterCallback = Widget::EventCallbackContext (StreamItemUi::thumbnailMouseEntered, this);
			thumbnail->mouseExitCallback = Widget::EventCallbackContext (StreamItemUi::thumbnailMouseExited, this);
			cardView->addItem (thumbnail, cardid, StreamItemUi::ImageRow, true);
		}
		cardView->refresh ();
	}

	isRecordSynced = true;
}

void StreamItemUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	Menu *menu;

	ui = (StreamItemUi *) uiPtr;
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, StreamItemUi::imageSizeButtonClicked)) {
		return;
	}

	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextString::Small).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), StreamItemUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Medium).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), StreamItemUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Large).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), StreamItemUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);

	ui->showActionPopup (menu, widgetPtr, StreamItemUi::imageSizeButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::BottomOfAlignment);
}

void StreamItemUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	HashMap *prefs;
	int detail;

	ui = (StreamItemUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (StreamItemUi::ImageRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (StreamItemUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void StreamItemUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	HashMap *prefs;
	int detail;

	ui = (StreamItemUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (StreamItemUi::ImageRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (StreamItemUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void StreamItemUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	HashMap *prefs;
	int detail;

	ui = (StreamItemUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (StreamItemUi::ImageRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (StreamItemUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void StreamItemUi::thumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	MediaThumbnailWindow *target;

	ui = (StreamItemUi *) uiPtr;
	if (! ui->isSelectingPlayPosition) {
		return;
	}

	target = MediaThumbnailWindow::castWidget (widgetPtr);
	if (! target) {
		return;
	}

	if (ui->thumbnailClickCallback.callback) {
		ui->thumbnailClickCallback.callback (ui->thumbnailClickCallback.callbackData, target);
	}
}

void StreamItemUi::thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	MediaThumbnailWindow *thumbnail;
	MediaTimelineWindow *timeline;

	ui = (StreamItemUi *) uiPtr;
	thumbnail = (MediaThumbnailWindow *) widgetPtr;
	if (! ui->isSelectingPlayPosition) {
		return;
	}

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
	index = (int) (timeline->clickPosition / timeline->width * (float) ui->segmentCount);

	if (ui->isSelectingPlayPosition) {
		target = (MediaThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &index);
		if (target && ui->thumbnailClickCallback.callback) {
			ui->thumbnailClickCallback.callback (ui->thumbnailClickCallback.callbackData, target);
		}
	}
	else {
		ui->cardView->scrollToItem (StdString::createSprintf ("%i", index));
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

static void selectToggleStateChanged_processMediaThumbnails (void *uiPtr, Widget *widgetPtr) {
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (widgetPtr);
	if (thumbnail) {
		thumbnail->setHighlighted (false);
	}
}
void StreamItemUi::selectToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	Toggle *toggle;
	MediaTimelineWindow *timeline;
	LabelWindow *caption;

	ui = (StreamItemUi *) uiPtr;
	toggle = (Toggle *) widgetPtr;
	timeline = (MediaTimelineWindow *) ui->timelineWindow.widget;

	ui->isSelectingPlayPosition = toggle->isChecked;

	if (! ui->isSelectingPlayPosition) {
		ui->commandCaption.destroyAndClear ();
		ui->cardView->processRowItems (StreamItemUi::ImageRow, selectToggleStateChanged_processMediaThumbnails, ui);
		if (timeline) {
			timeline->setHighlightedMarker (-1);
			timeline->setHighlightColor (UiConfiguration::instance->darkBackgroundColor);
		}
		return;
	}

	if (timeline) {
		timeline->setHighlightColor (UiConfiguration::instance->mediumSecondaryColor);
	}
	caption = (LabelWindow *) App::instance->rootPanel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::SelectPlayPosition).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->mediumSecondaryColor)));
	ui->commandCaption.assign (caption);
	caption->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	caption->setBorder (true, Color (UiConfiguration::instance->darkBackgroundColor.r, UiConfiguration::instance->darkBackgroundColor.g, UiConfiguration::instance->darkBackgroundColor.b, UiConfiguration::instance->overlayWindowAlpha));

	caption->setLayout (Panel::VerticalRightJustifiedLayout);
	caption->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	caption->position.assign (App::instance->windowWidth - caption->width - UiConfiguration::instance->paddingSize, App::instance->windowHeight - App::instance->uiStack.bottomBarHeight - caption->height - UiConfiguration::instance->marginSize);
}
