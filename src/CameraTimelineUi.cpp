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
#include <vector>
#include "SDL2/SDL.h"
#include "App.h"
#include "StdString.h"
#include "OsUtil.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Ui.h"
#include "RecordStore.h"
#include "AgentControl.h"
#include "Chip.h"
#include "Toggle.h"
#include "Menu.h"
#include "CommandListener.h"
#include "IconCardWindow.h"
#include "CameraDetailWindow.h"
#include "CameraThumbnailWindow.h"
#include "CameraTimelineWindow.h"
#include "CameraTimelineUi.h"

const char *CameraTimelineUi::ImageSizeKey = "CameraTimeline_ImageSize";
const float CameraTimelineUi::TimelineWindowScale = 0.65f;
const int CameraTimelineUi::DisplayPageSize = 64;
const int CameraTimelineUi::TimelinePageSize = 1024;
const int CameraTimelineUi::CapturePlayPeriod = 250; // ms
const float CameraTimelineUi::TimelinePopupWidthMultiplier = 0.12f;

CameraTimelineUi::CameraTimelineUi (CameraWindow *cameraWindow)
: Ui ()
, sensor (0)
, cardDetail (-1)
, isSortDescending (true)
, isFindingCaptureImages (false)
, isFindingDisplayCaptureImages (false)
, findCaptureImagesTime (0)
, captureStartTime (-1)
, captureEndTime (-1)
, timelineHoverPosition (-1)
, timelinePopupPositionStartTime (0)
, timelinePopupPosition (-1)
, selectedTime (-1)
, displayStartTime (-1)
, displayEndTime (-1)
, isSelectingCaptureImage (false)
{
	agentId.assign (cameraWindow->agentId);
	agentName.assign (cameraWindow->agentName);
	sensor = cameraWindow->sensor;
}

CameraTimelineUi::~CameraTimelineUi () {

}

StdString CameraTimelineUi::getSpritePath () {
	return (StdString ("ui/CameraTimelineUi/sprite"));
}

Widget *CameraTimelineUi::createBreadcrumbWidget () {
	return (new Chip (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (agentName, ((float) App::instance->windowWidth) * 0.5f, Font::DotTruncateSuffix)));
}

void CameraTimelineUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextString::CameraTimelineUiHelpTitle), UiText::instance->getText (UiTextString::CameraTimelineUiHelpText));
	helpWindow->addAction (UiText::instance->getText (UiTextString::CameraTimelineUiHelpAction1Text));
	helpWindow->addAction (UiText::instance->getText (UiTextString::CameraTimelineUiHelpAction2Text));
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

OsUtil::Result CameraTimelineUi::doLoad () {
	HashMap *prefs;

	prefs = App::instance->lockPrefs ();
	cardDetail = prefs->find (CameraTimelineUi::ImageSizeKey, (int) CardView::MediumDetail);
	App::instance->unlockPrefs ();
	isSortDescending = true;
	isFindingCaptureImages = false;
	isFindingDisplayCaptureImages = false;
	findCaptureImagesTime = 0;
	captureStartTime = -1;
	captureEndTime = -1;
	timelineHoverPosition = -1;
	selectedTime = -1;
	displayStartTime = -1;
	displayEndTime = -1;

	cardView->setRowDetail (CameraTimelineUi::ImageRow, cardDetail);
	cardView->setRowItemMarginSize (CameraTimelineUi::ImageRow, UiConfiguration::instance->marginSize / 2.0f);

	return (OsUtil::Success);
}

void CameraTimelineUi::doUnload () {
	sortToggle.clear ();
	timelineWindow.clear ();
	cameraDetailWindow.clear ();
	emptyStateWindow.clear ();
	timelinePopup.clear ();
	commandCaption.destroyAndClear ();
	openImageButton.clear ();
	openVideoButton.clear ();
	targetImageButton.clear ();
	activeSelectButton.clear ();
}

void CameraTimelineUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraTimelineUi::imageSizeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ThumbnailImageSizeTooltip));
	toolbar->addRightItem (button);
}

void CameraTimelineUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	CameraTimelineWindow *timeline;
	Toggle *toggle;
	Button *button;

	timeline = new CameraTimelineWindow (App::instance->windowWidth * CameraTimelineUi::TimelineWindowScale);
	timeline->positionHoverCallback = Widget::EventCallbackContext (CameraTimelineUi::timelineWindowPositionHovered, this);
	timeline->positionClickCallback = Widget::EventCallbackContext (CameraTimelineUi::timelineWindowPositionClicked, this);
	toolbar->addLeftItem (timeline);
	timelineWindow.assign (timeline);

	toggle = new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::OrderAscendingButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::OrderDescendingButtonSprite));
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraTimelineUi::sortToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setMouseHoverTooltip (UiText::instance->getText (UiTextString::SortCaptureImagesTooltip));
	if (isSortDescending) {
		toggle->setChecked (true, true);
	}
	toolbar->addLeftItem (toggle);
	sortToggle.assign (toggle);

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarButtonSprite));
	button->shortcutKey = SDLK_TAB;
	button->mouseClickCallback = Widget::EventCallbackContext (CameraTimelineUi::selectModeButtonClicked, this);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CameraTimelineUiSelectTargetTooltip));
	button->setInverseColor (true);
	toolbar->addRightItem (button);
	targetImageButton.assign (button);

	button = new Button (sprites.getSprite (CameraTimelineUi::OpenVideoButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraTimelineUi::selectModeButtonClicked, this);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CameraTimelineUiOpenVideoTooltip));
	button->setInverseColor (true);
	toolbar->addRightItem (button);
	openVideoButton.assign (button);

	button = new Button (sprites.getSprite (CameraTimelineUi::OpenImageButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraTimelineUi::selectModeButtonClicked, this);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CameraTimelineUiOpenImageTooltip));
	button->setInverseColor (true);
	toolbar->addRightItem (button);
	openImageButton.assign (button);

	activeSelectButton.clear ();
}

void CameraTimelineUi::doClearPopupWidgets () {
	timelinePopup.destroyAndClear ();
}

void CameraTimelineUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/CameraTimelineUi/bg");
	CommandListener::instance->subscribe (SystemInterface::CommandId_FindCaptureImagesResult, CommandListener::CommandCallbackContext (CameraTimelineUi::receiveFindCaptureImagesResult, this));
}

void CameraTimelineUi::doUpdate (int msElapsed) {
	CameraTimelineWindow *timeline;
	ImageWindow *image;
	float x, y, w, h;
	int64_t t;

	sortToggle.compact ();
	timelineWindow.compact ();
	cameraDetailWindow.compact ();
	emptyStateWindow.compact ();
	timelinePopup.compact ();
	commandCaption.compact ();
	openImageButton.compact ();
	openVideoButton.compact ();
	targetImageButton.compact ();
	activeSelectButton.compact ();

	if ((! isFindingCaptureImages) && (captureStartTime > 0) && (captureEndTime > 0) && AgentControl::instance->isLinkClientConnected (agentId)) {
		if (isSortDescending) {
			if (displayEndTime < 0) {
				displayEndTime = (selectedTime >= 0) ? selectedTime : captureEndTime;
				findCaptureImages (displayEndTime, true);
			}
			else if (displayStartTime > captureStartTime) {
				if (cardView->isScrolledToBottom ()) {
					findCaptureImages (displayStartTime - 1, true);
				}
			}
		}
		else {
			if (displayStartTime < 0) {
				displayStartTime = (selectedTime >= 0) ? selectedTime : captureStartTime;
				findCaptureImages (displayStartTime, false);
			}
			else if ((displayEndTime >= 0) && (displayEndTime < captureEndTime)) {
				if (cardView->isScrolledToBottom ()) {
					findCaptureImages (displayEndTime + 1, false);
				}
			}
		}
		if ((! isFindingCaptureImages) && (! captureTimes.isFullyCovered ())) {
			findCaptureImages (captureTimes.getUncoveredMin (), false, false);
		}
	}

	timeline = (CameraTimelineWindow *) timelineWindow.widget;
	if ((! timeline) || (timelineHoverPosition < 0)) {
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
				t = captureTimes.findNearest (timelineHoverPosition, (UiConfiguration::instance->timelineMarkerWidth * timeline->timeRate) / 2);
				timelinePopup.destroyAndClear ();
				w = ((float) App::instance->windowWidth) * CameraTimelineUi::TimelinePopupWidthMultiplier;
				h = w * 9.0f / 16.0f;
				x = timeline->screenX + timeline->hoverPosition - (w / 2.0f);
				y = timeline->screenY - h - (UiConfiguration::instance->marginSize / 2.0f);
				image = (ImageWindow *) App::instance->rootPanel->addWidget (new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
				image->loadCallback = Widget::EventCallbackContext (CameraTimelineUi::timelineHoverImageLoaded, this);
				image->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
				image->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
				if (t == 0) {
					image->setWindowSize (true, w, h);
					if (captureTimes.isCovered (timelineHoverPosition)) {
						image->setImage (new Image (sprites.getSprite (CameraTimelineUi::NoTimelineImageIconSprite)));
					}
					else {
						image->setImage (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite)));
					}
				}
				else {
					image->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite), w, h);
					image->onLoadScale (w);
					image->setImageUrl (AgentControl::instance->getAgentSecondaryUrl (agentId, captureImagePath, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, (new Json ())->set ("sensor", sensor)->set ("imageTime", t))));
				}
				image->isInputSuspended = true;
				image->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
				image->position.assignBounded (x, y, 0.0f, y, ((float) App::instance->windowWidth) - w, y);
				timelinePopup.assign (image);
				timelinePopupPosition = timelineHoverPosition;
			}
		}
	}
}

void CameraTimelineUi::doResize () {
	CameraTimelineWindow *timeline;

	timeline = (CameraTimelineWindow *) timelineWindow.widget;
	if (timeline) {
		timeline->setBarWidth (App::instance->windowWidth * CameraTimelineUi::TimelineWindowScale);
	}
}

void CameraTimelineUi::doSyncRecordStore () {
	Json *record, serverstatus, sensorstatus;
	CameraDetailWindow *camera;
	CameraTimelineWindow *timeline;
	IconCardWindow *emptystate;

	record = RecordStore::instance->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}
	if (! SystemInterface::instance->getCommandObjectParam (record, "cameraServerStatus", &serverstatus)) {
		return;
	}
	if (! serverstatus.getArrayObject ("sensors", sensor, &sensorstatus)) {
		return;
	}
	captureImagePath = serverstatus.getString ("captureImagePath", "");
	captureVideoPath = serverstatus.getString ("captureVideoPath", "");
	captureStartTime = sensorstatus.getNumber ("minCaptureTime", (int64_t) 0);
	captureEndTime = sensorstatus.getNumber ("lastCaptureTime", (int64_t) 0);
	captureTimes.setBounds (captureStartTime, captureEndTime);
	if (selectedTime < 0) {
		selectedTime = isSortDescending ? captureEndTime : captureStartTime;
	}

	if (! cardView->contains (agentId)) {
		camera = new CameraDetailWindow (agentId, sensor, &sprites);
		camera->setSelectedTimespan (captureEndTime, isSortDescending);
		cardView->addItem (camera, agentId, CameraTimelineUi::InfoRow);
		cameraDetailWindow.assign (camera);
		AgentControl::instance->refreshAgentStatus (agentId);
		addLinkAgent (agentId);
		timeline = (CameraTimelineWindow *) timelineWindow.widget;
		if (timeline) {
			timeline->setTimespan (captureStartTime, captureEndTime);
		}
	}

	emptystate = (IconCardWindow *) emptyStateWindow.widget;
	if ((captureStartTime <= 0) || (captureEndTime <= 0)) {
		if (! emptystate) {
			emptystate = new IconCardWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeErrorIconSprite));
			emptystate->setName (UiText::instance->getText (UiTextString::CameraTimelineUiEmptyStatusTitle));
			emptystate->setDetailText (UiText::instance->getText (UiTextString::CameraTimelineUiEmptyStatusText));
			emptyStateWindow.assign (emptystate);

			emptystate->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (emptystate, emptystate->itemId, CameraTimelineUi::ImageRow);
		}
	}
	else {
		if (emptystate) {
			cardView->removeItem (emptystate->itemId);
			emptyStateWindow.clear ();
		}
	}

	cardView->syncRecordStore ();
	cardView->refresh ();
}

void CameraTimelineUi::findCaptureImages (int64_t captureTime, bool isDescending, bool isDisplayFind) {
	Json *params;

	if (isFindingCaptureImages) {
		return;
	}
	isFindingCaptureImages = true;
	findCaptureImagesTime = captureTime;
	isFindingDisplayCaptureImages = isDisplayFind;
	params = new Json ();
	params->set ("sensor", sensor);
	params->set ("maxResults", isDisplayFind ? CameraTimelineUi::DisplayPageSize : CameraTimelineUi::TimelinePageSize);
	params->set ("isDescending", isDescending);
	if (captureTime > 0) {
		if (isDescending) {
			params->set ("maxTime", captureTime);
		}
		else {
			params->set ("minTime", captureTime);
		}
	}
	AgentControl::instance->writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindCaptureImages, params), agentId);
}

void CameraTimelineUi::receiveFindCaptureImagesResult (void *uiPtr, const StdString &agentId, Json *command) {
	CameraTimelineUi *ui;
	std::vector<int64_t> items;
	std::vector<int64_t>::iterator i, end;
	CameraTimelineWindow *timeline;
	CameraThumbnailWindow *thumbnail;
	StdString id;
	int64_t t, mintime, maxtime;

	ui = (CameraTimelineUi *) uiPtr;
	ui->isFindingCaptureImages = false;
	if (! SystemInterface::instance->getCommandNumberArrayParam (command, "captureTimes", &items, true)) {
		return;
	}

	mintime = -1;
	maxtime = -1;
	timeline = (CameraTimelineWindow *) ui->timelineWindow.widget;
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		t = *i;
		ui->captureTimes.insert (t);
		if (timeline) {
			timeline->addTimelinePoint (t);
		}
		if ((mintime < 0) || (t < mintime)) {
			mintime = t;
		}
		if ((maxtime < 0) || (t > maxtime)) {
			maxtime = t;
		}
		if (ui->isFindingDisplayCaptureImages) {
			id.sprintf ("%016llx", (long long int) t);
			if (! ui->cardView->contains (id)) {
				thumbnail = new CameraThumbnailWindow (ui->agentId, ui->sensor, ui->captureImagePath, t);
				thumbnail->loadCallback = Widget::EventCallbackContext (CameraTimelineUi::thumbnailImageLoaded, ui);
				thumbnail->mouseClickCallback = Widget::EventCallbackContext (CameraTimelineUi::thumbnailImageClicked, ui);
				thumbnail->mouseEnterCallback = Widget::EventCallbackContext (CameraTimelineUi::thumbnailImageMouseEntered, ui);
				thumbnail->mouseExitCallback = Widget::EventCallbackContext (CameraTimelineUi::thumbnailImageMouseExited, ui);
				ui->cardView->addItem (thumbnail, id, CameraTimelineUi::ImageRow, true);
			}
		}

		++i;
	}

	if ((mintime >= 0) && (maxtime >= 0)) {
		if (ui->findCaptureImagesTime > 0) {
			if (ui->findCaptureImagesTime < mintime) {
				mintime = ui->findCaptureImagesTime;
			}
			if (ui->findCaptureImagesTime > maxtime) {
				maxtime = ui->findCaptureImagesTime;
			}
		}

		if (ui->isSortDescending) {
			if ((ui->displayStartTime < 0) || (mintime < ui->displayStartTime)) {
				ui->displayStartTime = mintime;
			}
		}
		else {
			if ((ui->displayEndTime < 0) || (maxtime > ui->displayEndTime)) {
				ui->displayEndTime = maxtime;
			}
		}

		ui->captureTimes.cover (mintime, maxtime);
		if (timeline) {
			timeline->addTimelineCoverRange (mintime, maxtime);
		}
	}

	ui->cardView->refresh ();
}

void CameraTimelineUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	Menu *menu;

	ui = (CameraTimelineUi *) uiPtr;
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraTimelineUi::imageSizeButtonClicked)) {
		return;
	}

	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextString::Small).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), Widget::EventCallbackContext (CameraTimelineUi::smallImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Medium).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), Widget::EventCallbackContext (CameraTimelineUi::mediumImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Large).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), Widget::EventCallbackContext (CameraTimelineUi::largeImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::HighDetail);

	ui->showActionPopup (menu, widgetPtr, CameraTimelineUi::imageSizeButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::BottomOfAlignment);
}

void CameraTimelineUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	HashMap *prefs;
	int detail;

	ui = (CameraTimelineUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraTimelineUi::ImageRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraTimelineUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void CameraTimelineUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	HashMap *prefs;
	int detail;

	ui = (CameraTimelineUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraTimelineUi::ImageRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraTimelineUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void CameraTimelineUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	HashMap *prefs;
	int detail;

	ui = (CameraTimelineUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraTimelineUi::ImageRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraTimelineUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void CameraTimelineUi::timelineWindowPositionHovered (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	CameraTimelineWindow *timeline;
	float pos;
	int64_t t;

	ui = (CameraTimelineUi *) uiPtr;
	timeline = (CameraTimelineWindow *) widgetPtr;
	if (timeline->width <= 0.0f) {
		return;
	}
	pos = timeline->hoverPosition;
	if (pos < 0.0f) {
		t = -1;
	}
	else {
		t = timeline->startTime + (timeline->timeRate * (int64_t) pos);
	}
	if (ui->timelineHoverPosition != t) {
		timeline->setHighlightedTime (t);
		ui->timelineHoverPosition = t;
		ui->timelinePopup.destroyAndClear ();
		ui->timelinePopupPosition = -1;
		ui->timelinePopupPositionStartTime = 0;
	}
}

void CameraTimelineUi::timelineWindowPositionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	CameraTimelineWindow *timeline;
	CameraDetailWindow *camera;
	float pos;
	int64_t t;

	ui = (CameraTimelineUi *) uiPtr;
	timeline = (CameraTimelineWindow *) widgetPtr;
	if (timeline->width <= 0.0f) {
		return;
	}
	pos = timeline->clickPosition;
	if (pos < 0.0f) {
		return;
	}
	t = timeline->startTime + (timeline->timeRate * (int64_t) pos);
	if (t == ui->selectedTime) {
		return;
	}
	ui->selectedTime = t;
	timeline->setSelectedTime (ui->selectedTime, ui->isSortDescending);

	camera = (CameraDetailWindow *) ui->cameraDetailWindow.widget;
	if (camera) {
		camera->setSelectedTimespan (ui->selectedTime, ui->isSortDescending);
	}

	ui->cardView->removeRowItems (CameraTimelineUi::ImageRow);
	ui->displayStartTime = -1;
	ui->displayEndTime = -1;
}

void CameraTimelineUi::timelineHoverImageLoaded (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	ImageWindow *image;
	CameraTimelineWindow *timeline;

	ui = (CameraTimelineUi *) uiPtr;
	image = (ImageWindow *) widgetPtr;
	timeline = (CameraTimelineWindow *) ui->timelineWindow.widget;
	if (timeline) {
		image->position.assignY (timeline->screenY - image->height - (UiConfiguration::instance->marginSize / 2.0f));
	}
}

void CameraTimelineUi::sortToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	CameraDetailWindow *camera;
	CameraTimelineWindow *timeline;
	Toggle *toggle;

	ui = (CameraTimelineUi *) uiPtr;
	toggle = (Toggle *) widgetPtr;
	ui->isSortDescending = toggle->isChecked;

	timeline = (CameraTimelineWindow *) ui->timelineWindow.widget;
	if (timeline) {
		timeline->setSelectedTime (ui->selectedTime, ui->isSortDescending);
	}

	camera = (CameraDetailWindow *) ui->cameraDetailWindow.widget;
	if (camera) {
		camera->setSelectedTimespan (ui->selectedTime, ui->isSortDescending);
	}

	ui->cardView->removeRowItems (CameraTimelineUi::ImageRow);
	ui->displayStartTime = -1;
	ui->displayEndTime = -1;
}

void CameraTimelineUi::selectModeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	((CameraTimelineUi *) uiPtr)->setSelectMode ((Button *) widgetPtr);
}

static void setSelectMode_updateButton (const WidgetHandle &targetSelectButton, const WidgetHandle &activeSelectButton) {
	Button *button;

	button = (Button *) targetSelectButton.widget;
	if (! button) {
		return;
	}
	if (button == activeSelectButton.widget) {
		button->setRaised (true, UiConfiguration::instance->raisedButtonInverseBackgroundColor);
	}
	else {
		button->setRaised (false);
	}
}
static void setSelectMode_processThumbnails (void *uiPtr, Widget *widgetPtr) {
	CameraThumbnailWindow *thumbnail;

	thumbnail = CameraThumbnailWindow::castWidget (widgetPtr);
	if (thumbnail) {
		thumbnail->setHighlighted (false);
	}
}
void CameraTimelineUi::setSelectMode (Button *clickedButton) {
	LabelWindow *caption;
	StdString captiontext;

	if (activeSelectButton.widget == clickedButton) {
		activeSelectButton.clear ();
	}
	else {
		activeSelectButton.assign (clickedButton);
	}

	setSelectMode_updateButton (openImageButton, activeSelectButton);
	setSelectMode_updateButton (openVideoButton, activeSelectButton);
	setSelectMode_updateButton (targetImageButton, activeSelectButton);

	if (activeSelectButton.widget) {
		if (activeSelectButton.widget == openImageButton.widget) {
			captiontext.assign (UiText::instance->getText (UiTextString::CameraTimelineUiOpenImagePrompt).capitalized ());
		}
		else if (activeSelectButton.widget == openVideoButton.widget) {
			captiontext.assign (UiText::instance->getText (UiTextString::CameraTimelineUiOpenVideoPrompt).capitalized ());
		}
		else if (activeSelectButton.widget == targetImageButton.widget) {
			captiontext.assign (UiText::instance->getText (UiTextString::SelectTargetImage).capitalized ());
		}
	}

	commandCaption.destroyAndClear ();
	if (captiontext.empty ()) {
		isSelectingCaptureImage = false;
		cardView->processRowItems (CameraTimelineUi::ImageRow, setSelectMode_processThumbnails, this);
	}
	else {
		isSelectingCaptureImage = true;
		App::instance->uiStack.suspendMouseHover ();

		caption = (LabelWindow *) App::instance->rootPanel->addWidget (new LabelWindow (new Label (captiontext, UiConfiguration::BodyFont, UiConfiguration::instance->mediumSecondaryColor)));
		commandCaption.assign (caption);
		caption->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
		caption->setBorder (true, Color (UiConfiguration::instance->darkBackgroundColor.r, UiConfiguration::instance->darkBackgroundColor.g, UiConfiguration::instance->darkBackgroundColor.b, UiConfiguration::instance->overlayWindowAlpha));
		caption->setLayout (Panel::VerticalRightJustifiedLayout);
		caption->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
		caption->position.assign (App::instance->windowWidth - caption->width - UiConfiguration::instance->paddingSize, App::instance->windowHeight - App::instance->uiStack.bottomBarHeight - caption->height - UiConfiguration::instance->marginSize);
	}
}

void CameraTimelineUi::thumbnailImageLoaded (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;

	ui = (CameraTimelineUi *) uiPtr;
	ui->cardView->refresh ();
}

void CameraTimelineUi::thumbnailImageMouseEntered (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	CameraThumbnailWindow *thumbnail;

	ui = (CameraTimelineUi *) uiPtr;
	thumbnail = (CameraThumbnailWindow *) widgetPtr;
	if (! ui->isSelectingCaptureImage) {
		return;
	}
	thumbnail->setHighlighted (true);
}

void CameraTimelineUi::thumbnailImageMouseExited (void *uiPtr, Widget *widgetPtr) {
	CameraThumbnailWindow *thumbnail;

	thumbnail = (CameraThumbnailWindow *) widgetPtr;
	thumbnail->setHighlighted (false);
}

void CameraTimelineUi::thumbnailImageClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	CameraThumbnailWindow *thumbnail;
	Button *button;
	Json *params;
	OsUtil::Result result;

	ui = (CameraTimelineUi *) uiPtr;
	thumbnail = (CameraThumbnailWindow *) widgetPtr;
	if (! ui->isSelectingCaptureImage) {
		return;
	}
	button = (Button *) ui->activeSelectButton.widget;
	if (button == ui->openImageButton.widget) {
		result = OsUtil::openUrl (AgentControl::instance->getAgentSecondaryUrl (ui->agentId, ui->captureImagePath, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, (new Json ())->set ("sensor", ui->sensor)->set ("imageTime", thumbnail->thumbnailTimestamp)), true));
		if (result != OsUtil::Success) {
			App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::LaunchWebBrowserError));
		}
		else {
			App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::LaunchedWebBrowser).capitalized ());
		}
	}
	else if (button == ui->openVideoButton.widget) {
		if (! ui->captureVideoPath.empty ()) {
			params = new Json ();
			params->set ("sensor", ui->sensor);
			if (ui->isSortDescending) {
				params->set ("minTime", thumbnail->thumbnailTimestamp);
				params->set ("maxTime", ui->displayEndTime);
			}
			else {
				params->set ("minTime", ui->displayStartTime);
				params->set ("maxTime", thumbnail->thumbnailTimestamp);
			}
			params->set ("isDescending", ui->isSortDescending);
			result = OsUtil::openUrl (AgentControl::instance->getAgentSecondaryUrl (ui->agentId, ui->captureVideoPath, App::instance->createCommand (SystemInterface::Command_GetCaptureVideo, params), true));
			if (result != OsUtil::Success) {
				App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::LaunchWebBrowserError));
			}
			else {
				App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::CameraTimelineUiOpenVideoSnackbarText));
			}
		}
	}
	else if (button == ui->targetImageButton.widget) {
		if (ui->thumbnailClickCallback.callback) {
			ui->thumbnailClickCallback.callback (ui->thumbnailClickCallback.callbackData, thumbnail);
		}
	}
}
