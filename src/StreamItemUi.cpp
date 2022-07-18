/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "OsUtil.h"
#include "StdString.h"
#include "Resource.h"
#include "SpriteGroup.h"
#include "AgentControl.h"
#include "CommandHistory.h"
#include "Button.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "HashMap.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "MediaUtil.h"
#include "Toolbar.h"
#include "Menu.h"
#include "Chip.h"
#include "Toggle.h"
#include "Label.h"
#include "CardView.h"
#include "ActionWindow.h"
#include "LabelWindow.h"
#include "HelpWindow.h"
#include "MediaTimelineWindow.h"
#include "IconCardWindow.h"
#include "IconLabelWindow.h"
#include "MediaThumbnailWindow.h"
#include "TagWindow.h"
#include "StreamItemUi.h"

const char *StreamItemUi::ImageSizeKey = "StreamItem_ImageSize";
const float StreamItemUi::TextWidthMultiplier = 0.7f;
const float StreamItemUi::TimelinePopupWidthMultiplier = 0.12f;

StreamItemUi::StreamItemUi (const StdString &streamId, const StdString &streamName)
: Ui ()
, streamId (streamId)
, streamName (streamName)
, duration (0)
, frameWidth (0)
, frameHeight (0)
, frameRate (0.0f)
, bitrate (0)
, profile (0)
, streamSize (0)
, segmentCount (0)
, cardDetail (-1)
, timelineHoverPosition (-1)
, timelinePopupPositionStartTime (0)
, timelinePopupPosition (-1)
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

OsUtil::Result StreamItemUi::doLoad () {
	HashMap *prefs;
	Panel *panel;
	Button *button;

	prefs = App::instance->lockPrefs ();
	cardDetail = prefs->find (StreamItemUi::ImageSizeKey, (int) CardView::MediumDetail);
	App::instance->unlockPrefs ();
	timelineHoverPosition = -1;

	cardView->setItemMarginSize (UiConfiguration::instance->marginSize / 2.0f);
	cardView->setRowDetail (StreamItemUi::ImageRow, cardDetail);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	button = (Button *) panel->addWidget (new Button (sprites.getSprite (StreamItemUi::CreateTagButtonSprite)));
	button->mouseClickCallback = Widget::EventCallbackContext (StreamItemUi::createTagButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::AddMediaTagCommandName));

	panel->setLayout (Panel::HorizontalLayout);
	cardView->addItem (createRowHeaderPanel (UiText::instance->getText (UiTextString::SearchKeys).capitalized (), panel), StdString (""), StreamItemUi::TagButtonRow);

	return (OsUtil::Success);
}

void StreamItemUi::doUnload () {
	nameWindow.clear ();
	attributesWindow.clear ();
	streamSizeWindow.clear ();
	durationWindow.clear ();
	timelineWindow.clear ();
	timelinePopup.clear ();
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

void StreamItemUi::doClearPopupWidgets () {
	timelinePopup.destroyAndClear ();
}

void StreamItemUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/StreamItemUi/bg");
}

void StreamItemUi::doPause () {

}

void StreamItemUi::doUpdate (int msElapsed) {
	ImageWindow *image;
	MediaThumbnailWindow *target;
	MediaTimelineWindow *timeline;
	float x, y, w, h;

	timelineWindow.compact ();
	timelinePopup.compact ();
	commandCaption.compact ();

	timeline = (MediaTimelineWindow *) timelineWindow.widget;
	if ((! timeline) || (timelineHoverPosition < 0) || (frameWidth <= 0) || (frameHeight <= 0)) {
		if (timelinePopup.widget) {
			timelinePopup.destroyAndClear ();
			timelinePopupPosition = -1;
			timelinePopupPositionStartTime = 0;
		}
	}
	else {
		if (timelinePopupPosition != timelineHoverPosition) {
			if (timelinePopupPositionStartTime <= 0) {
				timelinePopupPositionStartTime = OsUtil::getTime ();
			}
			else if (OsUtil::getTime () >= (timelinePopupPositionStartTime + UiConfiguration::instance->mouseHoverThreshold)) {
				target = (MediaThumbnailWindow *) cardView->findItem (StreamItemUi::matchThumbnailIndex, &timelineHoverPosition);
				if (target) {
					timelinePopup.destroyAndClear ();
					w = ((float) App::instance->windowWidth) * StreamItemUi::TimelinePopupWidthMultiplier;
					h = w * ((float) frameHeight) / ((float) frameWidth);
					x = timeline->screenX + timeline->hoverPosition - (w / 2.0f);
					y = timeline->screenY - h - (UiConfiguration::instance->marginSize / 2.0f);
					image = (ImageWindow *) App::instance->rootPanel->addWidget (new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
					image->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
					image->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
					image->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite), w, h);
					image->onLoadScale (w);
					image->setImageUrl (target->sourceUrl);
					image->isInputSuspended = true;
					image->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
					image->position.assignBounded (x, y, 0.0f, y, ((float) App::instance->windowWidth) - w, y);
					timelinePopup.assign (image);
					timelinePopupPosition = timelineHoverPosition;
				}
			}
		}
	}
}

void StreamItemUi::doSyncRecordStore () {
	syncStreamItem ();
	if (timelineWindow.widget) {
		timelineWindow.widget->syncRecordStore ();
	}
	cardView->syncRecordStore ();
}

void StreamItemUi::syncStreamItem () {
	Json *streamitem, *mediaitem, *agentstatus, serverstatus, *params;
	MediaThumbnailWindow *thumbnail;
	IconCardWindow *iconcard;
	IconLabelWindow *iconlabel;
	TagWindow *tag;
	StdString url, cardid, text, profilename, rationame, framesizename;
	StringList attributes;
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

	mediaId = SystemInterface::instance->getCommandStringParam (streamitem, "sourceId", StdString (""));
	segmentCount = SystemInterface::instance->getCommandNumberParam (streamitem, "segmentCount", (int) 0);
	SystemInterface::instance->getCommandNumberArrayParam (streamitem, "segmentPositions", &segmentPositions, true);
	duration = SystemInterface::instance->getCommandNumberParam (streamitem, "duration", (int64_t) 0);
	frameWidth = SystemInterface::instance->getCommandNumberParam (streamitem, "width", (int) 0);
	frameHeight = SystemInterface::instance->getCommandNumberParam (streamitem, "height", (int) 0);
	frameRate = SystemInterface::instance->getCommandNumberParam (streamitem, "frameRate", (float) 0.0f);
	bitrate = SystemInterface::instance->getCommandNumberParam (streamitem, "bitrate", (int64_t) 0);
	profile = SystemInterface::instance->getCommandNumberParam (streamitem, "profile", (int) -1);
	streamSize = SystemInterface::instance->getCommandNumberParam (streamitem, "size", (int64_t) 0);

	if ((cardView->getRowItemCount (StreamItemUi::ImageRow) <= 0) && (segmentCount > 0) && (frameWidth > 0) && (frameHeight > 0) && (! thumbnailPath.empty ())) {
		for (i = 0; i < segmentCount; ++i) {
			params = new Json ();
			params->set ("id", streamId);
			params->set ("thumbnailIndex", i);
			url = AgentControl::instance->getAgentSecondaryUrl (agentId, thumbnailPath, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, params));

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
	}

	iconcard = (IconCardWindow *) nameWindow.widget;
	if (! iconcard) {
		iconcard = new IconCardWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeStreamIconSprite));
		iconcard->setCornerRadius (UiConfiguration::instance->cornerRadius);
		cardView->addItem (iconcard, streamId, StreamItemUi::InfoRow);
		nameWindow.assign (iconcard);
		iconcard = (IconCardWindow *) nameWindow.widget;
	}
	text = SystemInterface::instance->getCommandStringParam (streamitem, "name", "");
	iconcard->setName (UiConfiguration::instance->fonts[UiConfiguration::TitleFont]->truncatedText (text, App::instance->windowWidth * StreamItemUi::TextWidthMultiplier, Font::DotTruncateSuffix), UiConfiguration::TitleFont);
	iconcard->setSubtitle (AgentControl::instance->getAgentDisplayName (agentId));
	iconcard->setSubtitleColor (UiConfiguration::instance->lightPrimaryTextColor);

	if ((frameWidth > 0) && (frameHeight > 0)) {
		text.sprintf ("%ix%i", frameWidth, frameHeight);
		rationame = MediaUtil::getAspectRatioDisplayString (frameWidth, frameHeight);
		framesizename = MediaUtil::getFrameSizeName (frameWidth, frameHeight);
		if ((! rationame.empty ()) || (! framesizename.empty ())) {
			text.append (" (");
			if (! rationame.empty ()) {
				text.append (rationame);
			}
			if (! framesizename.empty ()) {
				if (! rationame.empty ()) {
					text.append (", ");
				}
				text.append (framesizename);
			}
			text.append (")");
		}
		attributes.push_back (text);
	}
	if (frameRate > 0.0f) {
		attributes.push_back (StdString::createSprintf ("%.2ffps", frameRate));
	}
	if (bitrate > 0.0f) {
		attributes.push_back (MediaUtil::getBitrateDisplayString (bitrate));
	}

	iconlabel = (IconLabelWindow *) attributesWindow.widget;
	if (! iconlabel) {
		iconlabel = new IconLabelWindow (sprites.getSprite (StreamItemUi::AttributesIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor);
		iconlabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
		iconlabel->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
		iconlabel->setCornerRadius (UiConfiguration::instance->cornerRadius);
		iconlabel->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaAttributesTooltip));
		cardView->addItem (iconlabel, "", StreamItemUi::InfoRow);
		attributesWindow.assign (iconlabel);
		iconlabel = (IconLabelWindow *) attributesWindow.widget;
	}
	text.assign ("");
	profilename = MediaUtil::getStreamProfileName (profile);
	if (! profilename.empty ()) {
		text.appendSprintf ("[%s] ", profilename.c_str ());
	}
	text.append (attributes.join (", "));
	iconlabel->setText (text);

	if (streamSize > 0) {
		iconlabel = (IconLabelWindow *) streamSizeWindow.widget;
		if (! iconlabel) {
			iconlabel = new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor);
			iconlabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
			iconlabel->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
			iconlabel->setCornerRadius (UiConfiguration::instance->cornerRadius);
			iconlabel->setMouseHoverTooltip (UiText::instance->getText (UiTextString::FileSize).capitalized ());
			cardView->addItem (iconlabel, "", StreamItemUi::InfoRow);
			streamSizeWindow.assign (iconlabel);
			iconlabel = (IconLabelWindow *) streamSizeWindow.widget;
		}
		iconlabel->setText (StdString::createSprintf ("%s (%lli)", OsUtil::getByteCountDisplayString (streamSize).c_str (), (long long int) streamSize));
	}

	if (duration > 0) {
		iconlabel = (IconLabelWindow *) durationWindow.widget;
		if (! iconlabel) {
			iconlabel = new IconLabelWindow (sprites.getSprite (StreamItemUi::DurationIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor);
			iconlabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
			iconlabel->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
			iconlabel->setCornerRadius (UiConfiguration::instance->cornerRadius);
			iconlabel->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Duration).capitalized ());
			cardView->addItem (iconlabel, "", StreamItemUi::InfoRow);
			durationWindow.assign (iconlabel);
			iconlabel = (IconLabelWindow *) durationWindow.widget;
		}
		iconlabel->setText (OsUtil::getDurationDisplayString (duration));
	}

	mediaitem = RecordStore::instance->findRecord (mediaId, SystemInterface::CommandId_MediaItem);
	if (mediaitem) {
		cardView->removeRowItems (StreamItemUi::TagRow);
		i = 0;
		while (true) {
			text = SystemInterface::instance->getCommandStringArrayItem (mediaitem, "tags", i, StdString (""));
			if (text.empty ()) {
				break;
			}
			tag = new TagWindow (text);
			tag->removeClickCallback = Widget::EventCallbackContext (StreamItemUi::removeTagButtonClicked, this);
			cardView->addItem (tag, StdString (""), StreamItemUi::TagRow);
			++i;
		}
	}

	cardView->refresh ();
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
	menu->addItem (UiText::instance->getText (UiTextString::Small).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), Widget::EventCallbackContext (StreamItemUi::smallImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Medium).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), Widget::EventCallbackContext (StreamItemUi::mediumImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Large).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), Widget::EventCallbackContext (StreamItemUi::largeImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::HighDetail);

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

	if (ui->timelineHoverPosition != index) {
		timeline->setHighlightedMarker (index);

		if (ui->timelineHoverPosition >= 0) {
			target = (MediaThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &(ui->timelineHoverPosition));
			if (target) {
				target->setHighlighted (false);
			}
		}

		ui->timelineHoverPosition = index;
		if (index >= 0) {
			target = (MediaThumbnailWindow *) ui->cardView->findItem (StreamItemUi::matchThumbnailIndex, &index);
			if (target) {
				target->setHighlighted (true);
			}
		}
		ui->timelinePopup.destroyAndClear ();
		ui->timelinePopupPosition = -1;
		ui->timelinePopupPositionStartTime = 0;
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

void StreamItemUi::createTagButtonClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	Button *button;
	ActionWindow *action;
	TextField *textfield;

	ui = (StreamItemUi *) uiPtr;
	button = (Button *) widgetPtr;
	if (ui->clearActionPopup (button, StreamItemUi::createTagButtonClicked)) {
		return;
	}
	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextString::AddMediaTagCommandName));
	action->setDescriptionText (UiText::instance->getText (UiTextString::AddMediaTagPrompt));
	textfield = new TextField (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth);
	action->addOption (UiText::instance->getText (UiTextString::SearchKey), textfield);
	action->setOptionNameText (UiText::instance->getText (UiTextString::SearchKey), StdString (""));
	action->setOptionNotEmptyString (UiText::instance->getText (UiTextString::SearchKey));
	action->closeCallback = Widget::EventCallbackContext (StreamItemUi::createTagActionClosed, ui);

	ui->showActionPopup (action, button, StreamItemUi::createTagButtonClicked, button->getScreenRect (), Ui::RightOfAlignment, Ui::TopEdgeAlignment);
	App::instance->uiStack.setKeyFocusTarget (textfield);
}

void StreamItemUi::createTagActionClosed (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ActionWindow *action;
	Json *params;

	ui = (StreamItemUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}
	if (ui->mediaId.empty ()) {
		return;
	}
	params = new Json ();
	params->set ("mediaId", ui->mediaId);
	params->set ("tag", action->getStringValue (UiText::instance->getText (UiTextString::SearchKey), StdString ("")));
	ui->invokeCommand (CommandHistory::instance->addMediaTag (StringList (ui->agentId), 1, ui->streamName), ui->agentId, App::instance->createCommand (SystemInterface::Command_AddMediaTag, params), StreamItemUi::invokeMediaCommandComplete);
}

void StreamItemUi::invokeMediaCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess) {
	Json record;

	if (isResponseCommandSuccess) {
		if (SystemInterface::instance->getCommandObjectParam (responseCommand, "item", &record)) {
			RecordStore::instance->addRecord (&record);
			App::instance->shouldSyncRecordStore = true;
		}
	}
}

void StreamItemUi::removeTagButtonClicked (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	TagWindow *tag;
	ActionWindow *action;

	ui = (StreamItemUi *) uiPtr;
	tag = (TagWindow *) widgetPtr;
	if (ui->clearActionPopup (tag, StreamItemUi::removeTagButtonClicked)) {
		return;
	}
	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiConfiguration::instance->fonts[UiConfiguration::TitleFont]->truncatedText (tag->tag, tag->width * 0.8f, Font::DotTruncateSuffix));
	action->setDescriptionText (UiText::instance->getText (UiTextString::RemoveMediaTagPrompt));
	action->closeCallback = Widget::EventCallbackContext (StreamItemUi::removeTagActionClosed, ui);

	ui->showActionPopup (action, tag, StreamItemUi::removeTagButtonClicked, tag->getScreenRect (), Ui::LeftEdgeAlignment, Ui::BottomOfAlignment);
}

void StreamItemUi::removeTagActionClosed (void *uiPtr, Widget *widgetPtr) {
	StreamItemUi *ui;
	ActionWindow *action;
	TagWindow *tag;
	Json *params;

	ui = (StreamItemUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}
	tag = TagWindow::castWidget (ui->actionTarget.widget);
	if (! tag) {
		return;
	}
	params = new Json ();
	params->set ("mediaId", ui->mediaId);
	params->set ("tag", tag->tag);
	ui->invokeCommand (CommandHistory::instance->removeMediaTag (StringList (ui->agentId), 1, ui->streamName), ui->agentId, App::instance->createCommand (SystemInterface::Command_RemoveMediaTag, params), StreamItemUi::invokeMediaCommandComplete);
}
