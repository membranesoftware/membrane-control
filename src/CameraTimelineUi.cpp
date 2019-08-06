/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include <queue>
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

const int CameraTimelineUi::pageSize = 64;
const int CameraTimelineUi::capturePlayPeriod = 250; // ms

CameraTimelineUi::CameraTimelineUi (const StdString &agentId, const StdString &agentName)
: Ui ()
, agentId (agentId)
, agentName (agentName)
, cardView (NULL)
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
{

}

CameraTimelineUi::~CameraTimelineUi () {

}

StdString CameraTimelineUi::getSpritePath () {
	return (StdString ("ui/CameraTimelineUi/sprite"));
}

Widget *CameraTimelineUi::createBreadcrumbWidget () {
	return (new Chip (agentName));
}

void CameraTimelineUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::cameraTimelineUiHelpTitle), uitext->getText (UiTextString::cameraTimelineUiHelpText));
}

int CameraTimelineUi::doLoad () {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	cardDetail = App::instance->prefsMap.find (App::CameraTimelineUiImageSizeKey, (int) CardView::MediumDetail);
	isSortDescending = true;
	isFindingCaptureImages = false;
	isPlayingCapture = false;
	captureStartTime = -1;
	captureEndTime = -1;
	lastTimelineHoverTime = -1;
	selectedTime = -1;
	displayStartTime = -1;
	displayEndTime = -1;

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowDetail (CameraTimelineUi::ImageRow, cardDetail);
	cardView->setRowItemMarginSize (CameraTimelineUi::ImageRow, uiconfig->marginSize / 2.0f);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	return (Result::Success);
}

void CameraTimelineUi::doUnload () {
	sortToggle.clear ();
	timelineWindow.clear ();
	cameraDetailWindow.clear ();
	capturePlayWindow.clear ();
}

void CameraTimelineUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (CameraTimelineUi::imageSizeButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);
}

void CameraTimelineUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	UiText *uitext;
	CameraTimelineWindow *timeline;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	timeline = new CameraTimelineWindow (App::instance->windowWidth * 0.78f);
	timeline->setPositionHoverCallback (CameraTimelineUi::timelineWindowPositionHovered, this);
	timeline->setPositionClickCallback (CameraTimelineUi::timelineWindowPositionClicked, this);
	toolbar->addLeftItem (timeline);
	timelineWindow.assign (timeline);

	toggle = new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::OrderAscendingButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::OrderDescendingButtonSprite));
	toggle->setInverseColor (true);
	toggle->setStateChangeCallback (CameraTimelineUi::sortToggleStateChanged, this);
	toggle->setMouseHoverTooltip (uitext->getText (UiTextString::sortCaptureImagesTooltip));
	if (isSortDescending) {
		toggle->setChecked (true, true);
	}
	toolbar->addLeftItem (toggle);
	sortToggle.assign (toggle);

	toggle = new Toggle (sprites.getSprite (CameraTimelineUi::PlayButtonSprite), sprites.getSprite (CameraTimelineUi::StopButtonSprite));
	toggle->setInverseColor (true);
	toggle->setStateChangeCallback (CameraTimelineUi::playToggleStateChanged, this);
	toggle->setMouseHoverTooltip (uitext->getText (UiTextString::playCaptureTooltip));
	toolbar->addRightItem (toggle);
}

void CameraTimelineUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
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
	timelineWindow.compact ();
	cameraDetailWindow.compact ();
	capturePlayWindow.compact ();

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
		params->set ("maxResults", CameraTimelineUi::pageSize);
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
				thumbnail->setLoadCallback (CameraTimelineUi::capturePlayThumbnailImageLoaded, this);
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
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
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
	params->set ("maxResults", CameraTimelineUi::pageSize);
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
							thumbnail->setLoadCallback (CameraTimelineUi::thumbnailImageLoaded, this);
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
	bool show;

	ui = (CameraTimelineUi *) uiPtr;
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
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), CameraTimelineUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), CameraTimelineUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), CameraTimelineUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);
	menu->position.assign (widgetPtr->screenX + widgetPtr->width - menu->width, widgetPtr->screenY + widgetPtr->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void CameraTimelineUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	int detail;

	ui = (CameraTimelineUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraTimelineUi::ImageRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::CameraTimelineUiImageSizeKey, ui->cardDetail);
}

void CameraTimelineUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	int detail;

	ui = (CameraTimelineUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraTimelineUi::ImageRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::CameraTimelineUiImageSizeKey, ui->cardDetail);
}

void CameraTimelineUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;
	int detail;

	ui = (CameraTimelineUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraTimelineUi::ImageRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::CameraTimelineUiImageSizeKey, ui->cardDetail);
}

void CameraTimelineUi::thumbnailImageLoaded (void *uiPtr, Widget *widgetPtr) {
	CameraTimelineUi *ui;

	ui = (CameraTimelineUi *) uiPtr;
	ui->cardView->refresh ();
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
	params->set ("maxResults", CameraTimelineUi::pageSize);
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
	params->set ("maxResults", CameraTimelineUi::pageSize);
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

	timeline = (CameraTimelineWindow *) ui->timelineWindow.widget;
	if (timeline) {
		timeline->setDisabled (ui->isPlayingCapture);
	}

	params = new Json ();
	params->set ("maxResults", CameraTimelineUi::pageSize);
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
	ui->capturePlayClock = CameraTimelineUi::capturePlayPeriod;
	ui->cardView->refresh ();
}
