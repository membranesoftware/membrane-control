/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "Result.h"
#include "StdString.h"
#include "Log.h"
#include "App.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Ui.h"
#include "RecordStore.h"
#include "Chip.h"
#include "Toggle.h"
#include "Menu.h"
#include "CameraDetailWindow.h"
#include "CameraThumbnailWindow.h"
#include "CameraTimelineWindow.h"
#include "CameraTimelineUi.h"

const char *CameraTimelineUi::ImageSizeKey = "CameraTimeline_ImageSize";
const float CameraTimelineUi::TimelineWindowScale = 0.68f;
const int CameraTimelineUi::PageSize = 64;
const int CameraTimelineUi::CapturePlayPeriod = 250; // ms

CameraTimelineUi::CameraTimelineUi (const StdString &agentId, const StdString &agentName)
: Ui ()
, agentId (agentId)
, agentName (agentName)
, cardDetail (-1)
, isSortDescending (true)
, isFindingCaptureImages (false)
, isPlayingCapture (false)
, capturePlayClock (0)
, captureStartTime (-1)
, captureEndTime (-1)
, lastTimelineHoverTime (-1)
, selectedTime (-1)
, displayStartTime (-1)
, displayEndTime (-1)
, isSelectingCaptureImage (false)
{

}

CameraTimelineUi::~CameraTimelineUi () {

}

StdString CameraTimelineUi::getSpritePath () {
	return (StdString ("ui/CameraTimelineUi/sprite"));
}

Widget *CameraTimelineUi::createBreadcrumbWidget () {
	return (new Chip (Label::getTruncatedText (agentName, UiConfiguration::CaptionFont, ((float) App::instance->windowWidth) * 0.5f, Label::DotTruncateSuffix)));
}

void CameraTimelineUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::CameraTimelineUiHelpTitle), uitext->getText (UiTextString::CameraTimelineUiHelpText));
	helpWindow->addAction (uitext->getText (UiTextString::CameraTimelineUiHelpAction1Text));
	helpWindow->addAction (uitext->getText (UiTextString::CameraTimelineUiHelpAction2Text));

	helpWindow->addTopicLink (uitext->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

int CameraTimelineUi::doLoad () {
	HashMap *prefs;
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	prefs = App::instance->lockPrefs ();
	cardDetail = prefs->find (CameraTimelineUi::ImageSizeKey, (int) CardView::MediumDetail);
	App::instance->unlockPrefs ();
	isSortDescending = true;
	isFindingCaptureImages = false;
	isPlayingCapture = false;
	captureStartTime = -1;
	captureEndTime = -1;
	lastTimelineHoverTime = -1;
	selectedTime = -1;
	displayStartTime = -1;
	displayEndTime = -1;

	cardView->setRowDetail (CameraTimelineUi::ImageRow, cardDetail);
	cardView->setRowItemMarginSize (CameraTimelineUi::ImageRow, uiconfig->marginSize / 2.0f);

	return (Result::Success);
}

void CameraTimelineUi::doUnload () {
	sortToggle.clear ();
	selectToggle.clear ();
	timelineWindow.clear ();
	cameraDetailWindow.clear ();
	capturePlayWindow.clear ();
	commandCaption.destroyAndClear ();
}

void CameraTimelineUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraTimelineUi::imageSizeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::ThumbnailImageSizeTooltip));
	toolbar->addRightItem (button);
}

void CameraTimelineUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	UiText *uitext;
	CameraTimelineWindow *timeline;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	timeline = new CameraTimelineWindow (App::instance->windowWidth * CameraTimelineUi::TimelineWindowScale);
	timeline->positionHoverCallback = Widget::EventCallbackContext (CameraTimelineUi::timelineWindowPositionHovered, this);
	timeline->positionClickCallback = Widget::EventCallbackContext (CameraTimelineUi::timelineWindowPositionClicked, this);
	toolbar->addLeftItem (timeline);
	timelineWindow.assign (timeline);

	toggle = new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::OrderAscendingButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::OrderDescendingButtonSprite));
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraTimelineUi::sortToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setMouseHoverTooltip (uitext->getText (UiTextString::SortCaptureImagesTooltip));
	if (isSortDescending) {
		toggle->setChecked (true, true);
	}
	toolbar->addLeftItem (toggle);
	sortToggle.assign (toggle);

	toggle = new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::StarButtonSprite));
	toggle->shortcutKey = SDLK_TAB;
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraTimelineUi::selectToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setMouseHoverTooltip (uitext->getText (UiTextString::CameraTimelineUiSelectTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (toggle);
	selectToggle.assign (toggle);

	toggle = new Toggle (sprites.getSprite (CameraTimelineUi::PlayButtonSprite), sprites.getSprite (CameraTimelineUi::StopButtonSprite));
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraTimelineUi::playToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setMouseHoverTooltip (uitext->getText (UiTextString::PlayCaptureTooltip));
	toolbar->addRightItem (toggle);
}

void CameraTimelineUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/CameraTimelineUi/bg");
}

void CameraTimelineUi::doUpdate (int msElapsed) {
	Json *params;
	CameraThumbnailWindow *thumbnail;
	bool shouldfind;
	int64_t t;

	sortToggle.compact ();
	selectToggle.compact ();
	timelineWindow.compact ();
	cameraDetailWindow.compact ();
	capturePlayWindow.compact ();
	commandCaption.compact ();

	shouldfind = false;
	if (! isFindingCaptureImages) {
		if (isSortDescending) {
			if ((captureStartTime > 0) && (displayStartTime > captureStartTime)) {
				shouldfind = true;
			}
		}
		else {
			if ((captureEndTime > 0) && (displayEndTime < captureEndTime)) {
				shouldfind = true;
			}
		}

		if (shouldfind) {
			if (isPlayingCapture) {
				if (capturePlayTimes.size () > 8) {
					shouldfind = false;
				}
			}
			else {
				if (! cardView->isScrolledToBottom ()) {
					shouldfind = false;
				}
			}
		}
	}

	if (shouldfind) {
		params = new Json ();
		params->set ("maxResults", CameraTimelineUi::PageSize);
		params->set ("isDescending", isSortDescending);
		if (isSortDescending) {
			params->set ("maxTime", displayStartTime - 1);
		}
		else {
			params->set ("minTime", displayEndTime + 1);
		}

		isFindingCaptureImages = true;
		App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindCaptureImages, SystemInterface::Constant_Camera, params), agentId);
	}

	if (isPlayingCapture && (! capturePlayTimes.empty ()) && (capturePlayClock >= 0)) {
		capturePlayClock -= msElapsed;
		if (capturePlayClock < 0) {
			t = capturePlayTimes.front ();
			capturePlayTimes.pop ();
			thumbnail = (CameraThumbnailWindow *) capturePlayWindow.widget;
			if (! thumbnail) {
				thumbnail = new CameraThumbnailWindow (agentId, captureImagePath, t);
				thumbnail->loadCallback = Widget::EventCallbackContext (CameraTimelineUi::capturePlayThumbnailImageLoaded, this);
				cardView->addItem (thumbnail, StdString (""), CameraTimelineUi::ImageRow);
				capturePlayWindow.assign (thumbnail);
			}
			else {
				thumbnail->setThumbnailTimestamp (t);
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
	RecordStore *store;
	SystemInterface *interface;
	Json *record, serverstatus;
	CameraDetailWindow *camera;
	CameraTimelineWindow *timeline;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	record = store->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}

	if (interface->getCommandObjectParam (record, "cameraServerStatus", &serverstatus)) {
		captureImagePath = serverstatus.getString ("captureImagePath", "");
		captureStartTime = serverstatus.getNumber ("minCaptureTime", (int64_t) 0);
		captureEndTime = serverstatus.getNumber ("lastCaptureTime", (int64_t) 0);
	}

	if (! cardView->contains (agentId)) {
		camera = new CameraDetailWindow (agentId, &sprites);
		camera->setSelectedTimespan (captureEndTime, isSortDescending);
		cardView->addItem (camera, agentId, CameraTimelineUi::InfoRow);
		cameraDetailWindow.assign (camera);
		App::instance->agentControl.refreshAgentStatus (agentId);
		addLinkAgent (agentId);
		timeline = (CameraTimelineWindow *) timelineWindow.widget;
		if (timeline) {
			timeline->setTimespan (captureStartTime, captureEndTime);
		}
	}

	cardView->syncRecordStore ();
	cardView->refresh ();
}

void CameraTimelineUi::handleLinkClientConnect (const StdString &agentId) {
	Json *params;

	selectedTime = captureEndTime;
	displayStartTime = -1;
	displayEndTime = -1;
	isFindingCaptureImages = true;
	params = new Json ();
	params->set ("maxResults", CameraTimelineUi::PageSize);
	params->set ("isDescending", true);
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindCaptureImages, SystemInterface::Constant_Camera, params), agentId);
}

void CameraTimelineUi::handleLinkClientCommand (const StdString &agentId, int commandId, Json *command) {
	SystemInterface *interface;
	std::vector<int64_t> capturetimes;
	std::vector<int64_t>::iterator i, end;
	CameraThumbnailWindow *thumbnail;
	StdString id;

	interface = &(App::instance->systemInterface);

	switch (commandId) {
		case SystemInterface::CommandId_FindCaptureImagesResult: {
			if (interface->getCommandNumberArrayParam (command, "captureTimes", &capturetimes, true)) {
				i = capturetimes.begin ();
				end = capturetimes.end ();
				while (i != end) {
					if ((displayStartTime < 0) || (*i < displayStartTime)) {
						displayStartTime = *i;
					}
					if ((displayEndTime < 0) || (*i > displayEndTime)) {
						displayEndTime = *i;
					}

					if (isPlayingCapture) {
						capturePlayTimes.push (*i);
					}
					else {
						id.sprintf ("%016llx", (long long int) *i);
						if (! cardView->contains (id)) {
							thumbnail = new CameraThumbnailWindow (agentId, captureImagePath, *i);
							thumbnail->loadCallback = Widget::EventCallbackContext (CameraTimelineUi::thumbnailImageLoaded, this);
							thumbnail->mouseClickCallback = Widget::EventCallbackContext (CameraTimelineUi::thumbnailImageClicked, this);
							thumbnail->mouseEnterCallback = Widget::EventCallbackContext (CameraTimelineUi::thumbnailImageMouseEntered, this);
							thumbnail->mouseExitCallback = Widget::EventCallbackContext (CameraTimelineUi::thumbnailImageMouseExited, this);
							cardView->addItem (thumbnail, id, CameraTimelineUi::ImageRow, true);
						}
					}
					++i;
				}

				cardView->refresh ();
			}
			isFindingCaptureImages = false;
			break;
		}
	}
}

void CameraTimelineUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Menu *menu;

	ui = (CameraTimelineUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraTimelineUi::imageSizeButtonClicked)) {
		return;
	}

	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::Small).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), CameraTimelineUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (uitext->getText (UiTextString::Medium).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), CameraTimelineUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (uitext->getText (UiTextString::Large).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), CameraTimelineUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);

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

	ui = (CameraTimelineUi *) uiPtr;
	thumbnail = (CameraThumbnailWindow *) widgetPtr;
	if (! ui->isSelectingCaptureImage) {
		return;
	}

	if (ui->thumbnailClickCallback.callback) {
		ui->thumbnailClickCallback.callback (ui->thumbnailClickCallback.callbackData, thumbnail);
	}
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
		pos *= (float) (timeline->endTime - timeline->startTime);
		pos /= timeline->width;
		t = timeline->startTime + (int64_t) pos;
	}
	if (ui->lastTimelineHoverTime != t) {
		timeline->setHighlightedTime (t);
		ui->lastTimelineHoverTime = t;
	}
}

void CameraTimelineUi::timelineWindowPositionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	CameraTimelineWindow *timeline;
	CameraDetailWindow *camera;
	Json *params;
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
	pos *= (float) (timeline->endTime - timeline->startTime);
	pos /= timeline->width;
	t = timeline->startTime + (int64_t) pos;
	if (t == ui->selectedTime) {
		return;
	}
	ui->selectedTime = t;
	timeline->setSelectedTime (ui->selectedTime, ui->isSortDescending);
	params = new Json ();
	params->set ("maxResults", CameraTimelineUi::PageSize);
	params->set ("isDescending", ui->isSortDescending);
	if (ui->isSortDescending) {
		params->set ("maxTime", ui->selectedTime);
	}
	else {
		params->set ("minTime", ui->selectedTime);
	}

	ui->isFindingCaptureImages = true;
	ui->displayStartTime = -1;
	ui->displayEndTime = -1;
	ui->cardView->removeRowItems (CameraTimelineUi::ImageRow);
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindCaptureImages, SystemInterface::Constant_Camera, params), ui->agentId);

	camera = (CameraDetailWindow *) ui->cameraDetailWindow.widget;
	if (camera) {
		camera->setSelectedTimespan (ui->selectedTime, ui->isSortDescending);
	}
}

void CameraTimelineUi::sortToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	CameraDetailWindow *camera;
	CameraTimelineWindow *timeline;
	Toggle *toggle;
	Json *params;

	ui = (CameraTimelineUi *) uiPtr;
	toggle = (Toggle *) widgetPtr;
	ui->isSortDescending = toggle->isChecked;

	params = new Json ();
	params->set ("maxResults", CameraTimelineUi::PageSize);
	params->set ("isDescending", ui->isSortDescending);
	if (ui->isSortDescending) {
		params->set ("maxTime", ui->selectedTime);
	}
	else {
		params->set ("minTime", ui->selectedTime);
	}

	ui->isFindingCaptureImages = true;
	ui->displayStartTime = -1;
	ui->displayEndTime = -1;
	ui->cardView->removeRowItems (CameraTimelineUi::ImageRow);
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindCaptureImages, SystemInterface::Constant_Camera, params), ui->agentId);

	camera = (CameraDetailWindow *) ui->cameraDetailWindow.widget;
	if (camera) {
		camera->setSelectedTimespan (ui->selectedTime, ui->isSortDescending);
	}

	timeline = (CameraTimelineWindow *) ui->timelineWindow.widget;
	if (timeline) {
		timeline->setSelectedTime (ui->selectedTime, ui->isSortDescending);
	}
}

void CameraTimelineUi::playToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	Toggle *toggle;
	CameraTimelineWindow *timeline;
	Json *params;

	ui = (CameraTimelineUi *) uiPtr;
	toggle = (Toggle *) widgetPtr;
	if (toggle->isChecked) {
		while (! ui->capturePlayTimes.empty ()) {
			ui->capturePlayTimes.pop ();
		}
		ui->capturePlayClock = 0;
		ui->isPlayingCapture = true;
	}
	else {
		ui->isPlayingCapture = false;
	}

	toggle = (Toggle *) ui->sortToggle.widget;
	if (toggle) {
		toggle->setDisabled (ui->isPlayingCapture);
	}

	toggle = (Toggle *) ui->selectToggle.widget;
	if (toggle) {
		toggle->setChecked (false, true);
		toggle->setDisabled (ui->isPlayingCapture);
	}
	ui->commandCaption.destroyAndClear ();
	ui->isSelectingCaptureImage = false;

	timeline = (CameraTimelineWindow *) ui->timelineWindow.widget;
	if (timeline) {
		timeline->setDisabled (ui->isPlayingCapture);
	}

	params = new Json ();
	params->set ("maxResults", CameraTimelineUi::PageSize);
	params->set ("isDescending", ui->isSortDescending);
	if (ui->isSortDescending) {
		params->set ("maxTime", ui->selectedTime);
	}
	else {
		params->set ("minTime", ui->selectedTime);
	}

	ui->cardView->removeRowItems (CameraTimelineUi::ImageRow);
	ui->isFindingCaptureImages = true;
	ui->displayStartTime = -1;
	ui->displayEndTime = -1;
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_FindCaptureImages, SystemInterface::Constant_Camera, params), ui->agentId);
}

void CameraTimelineUi::capturePlayThumbnailImageLoaded (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;

	ui = (CameraTimelineUi *) uiPtr;
	ui->capturePlayClock = CameraTimelineUi::CapturePlayPeriod;
	ui->cardView->refresh ();
}

static void selectToggleStateChanged_processThumbnails (void *uiPtr, Widget *widgetPtr) {
	CameraThumbnailWindow *thumbnail;

	thumbnail = CameraThumbnailWindow::castWidget (widgetPtr);
	if (thumbnail) {
		thumbnail->setHighlighted (false);
	}
}
void CameraTimelineUi::selectToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	Toggle *toggle;
	UiConfiguration *uiconfig;
	UiText *uitext;
	LabelWindow *caption;

	ui = (CameraTimelineUi *) uiPtr;
	toggle = (Toggle *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	ui->isSelectingCaptureImage = toggle->isChecked;
	if (! ui->isSelectingCaptureImage) {
		ui->commandCaption.destroyAndClear ();
		ui->cardView->processRowItems (CameraTimelineUi::ImageRow, selectToggleStateChanged_processThumbnails, ui);
		return;
	}

	caption = (LabelWindow *) App::instance->rootPanel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::SelectTargetImage).capitalized (), UiConfiguration::BodyFont, uiconfig->mediumSecondaryColor)));
	ui->commandCaption.assign (caption);
	caption->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha));
	caption->setBorder (true, Color (uiconfig->darkBackgroundColor.r, uiconfig->darkBackgroundColor.g, uiconfig->darkBackgroundColor.b, uiconfig->overlayWindowAlpha));

	caption->setLayout (Panel::VerticalRightJustifiedLayout);
	caption->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	caption->position.assign (App::instance->windowWidth - caption->width - uiconfig->paddingSize, App::instance->windowHeight - App::instance->uiStack.bottomBarHeight - caption->height - uiconfig->marginSize);
}
