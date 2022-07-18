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
#include <map>
#include "SDL2/SDL.h"
#include "App.h"
#include "StdString.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "OsUtil.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "Agent.h"
#include "Chip.h"
#include "Button.h"
#include "CardView.h"
#include "ComboBox.h"
#include "Toggle.h"
#include "Menu.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "HelpWindow.h"
#include "LabelWindow.h"
#include "TextFieldWindow.h"
#include "IconLabelWindow.h"
#include "IconCardWindow.h"
#include "ActionWindow.h"
#include "CameraWindow.h"
#include "MonitorWindow.h"
#include "CameraThumbnailWindow.h"
#include "CameraTimelineUi.h"
#include "CameraUi.h"

const char *CameraUi::SelectedItemsKey = "Camera_SelectedItems";
const char *CameraUi::ExpandedItemsKey = "Camera_ExpandedItems";
const char *CameraUi::ImageSizeKey = "Camera_ImageSize";
const char *CameraUi::AutoReloadKey = "Camera_AutoReload";
const char *CameraUi::ToolbarModeKey = "Camera_ToolbarMode";
const int CameraUi::CapturePeriods[] = { 0, 15, 60, 2 * 60, 3 * 60, 5 * 60, 10 * 60, 15 * 60, 30 * 60, 3600, 2 * 3600, 4 * 3600, 8 * 3600, 24 * 3600 };
const int CameraUi::DefaultCapturePeriodIndex = 1;

CameraUi::CameraUi ()
: Ui ()
, isAutoReloading (false)
, toolbarMode (-1)
, configureCameraButton (NULL)
, clearTimelapseButton (NULL)
, showCameraImageButton (NULL)
, playCameraStreamButton (NULL)
, clearDisplayButton (NULL)
, cameraCount (0)
, cardDetail (-1)
{
}

CameraUi::~CameraUi () {

}

StdString CameraUi::getSpritePath () {
	return (StdString ("ui/CameraUi/sprite"));
}

Widget *CameraUi::createBreadcrumbWidget () {
	return (new Chip (UiText::instance->getText (UiTextString::Cameras).capitalized (), sprites.getSprite (CameraUi::BreadcrumbIconSprite)));
}

void CameraUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextString::CameraUiHelpTitle), UiText::instance->getText (UiTextString::CameraUiHelpText));
	if (cameraCount <= 0) {
		helpWindow->addAction (UiText::instance->getText (UiTextString::CameraUiHelpAction1Text), UiText::instance->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
	}
	else {
		helpWindow->addAction (UiText::instance->getText (UiTextString::CameraUiHelpAction2Text));
		helpWindow->addAction (UiText::instance->getText (UiTextString::CameraUiHelpAction3Text));
	}
	helpWindow->addTopicLink (UiText::instance->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

static bool findItem_matchCameraName (void *data, Widget *widget) {
	CameraWindow *camera;

	camera = CameraWindow::castWidget (widget);
	return (camera && camera->agentName.lowercased ().equals ((char *) data));
}
bool CameraUi::openWidget (const StdString &targetName) {
	CameraWindow *camera;
	StdString name;

	name.assign (targetName.lowercased ());
	camera = (CameraWindow *) cardView->findItem (findItem_matchCameraName, (char *) name.c_str ());
	if (camera) {
		camera->eventCallback (camera->openButtonClickCallback);
		return (true);
	}
	return (false);
}

StdString CameraUi::getImageQualityDescription (int imageQuality) {
	if (imageQuality == SystemInterface::Constant_DefaultImageProfile) {
		return (UiText::instance->getText (UiTextString::NormalImageQualityDescription));
	}
	if (imageQuality == SystemInterface::Constant_HighQualityImageProfile) {
		return (UiText::instance->getText (UiTextString::HighImageQualityDescription));
	}
	if (imageQuality == SystemInterface::Constant_LowQualityImageProfile) {
		return (UiText::instance->getText (UiTextString::LowImageQualityDescription));
	}
	if (imageQuality == SystemInterface::Constant_LowestQualityImageProfile) {
		return (UiText::instance->getText (UiTextString::LowestImageQualityDescription));
	}
	return (StdString (""));
}

OsUtil::Result CameraUi::doLoad () {
	HashMap *prefs;
	Panel *panel;
	Toggle *toggle;

	cameraCount = 0;
	prefs = App::instance->lockPrefs ();
	cardDetail = prefs->find (CameraUi::ImageSizeKey, (int) CardView::MediumDetail);
	isAutoReloading = prefs->find (CameraUi::AutoReloadKey, false);
	App::instance->unlockPrefs ();

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraUi::expandCamerasToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::MinimizeAll).capitalized (), UiText::instance->getText (UiTextString::ExpandAll).capitalized ());
	expandCamerasToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (UiText::instance->getText (UiTextString::Cameras).capitalized (), panel), StdString (""), CameraUi::CameraToggleRow);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraUi::expandMonitorsToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::MinimizeAll).capitalized (), UiText::instance->getText (UiTextString::ExpandAll).capitalized ());
	expandMonitorsToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (UiText::instance->getText (UiTextString::Monitors).capitalized (), panel), StdString (""), CameraUi::MonitorToggleRow);

	cardView->setRowReverseSorted (CameraUi::ExpandedCameraRow, true);
	cardView->setRowReverseSorted (CameraUi::ExpandedMonitorRow, true);

	cardView->setRowDetail (CameraUi::ExpandedCameraRow, cardDetail);

	return (OsUtil::Success);
}

void CameraUi::doUnload () {
	autoReloadToggle.clear ();
	emptyStateWindow.clear ();
	selectedCameraMap.clear ();
	selectedMonitorMap.clear ();
	expandCamerasToggle.clear ();
	expandMonitorsToggle.clear ();
	targetCamera.clear ();
}

void CameraUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraUi::imageSizeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ThumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraUi::reloadButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ReloadTooltip));
	toolbar->addRightItem (button);
}

void CameraUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	HashMap *prefs;
	Toggle *toggle;

	toggle = new Toggle (sprites.getSprite (CameraUi::TimelineRefreshOffButtonSprite), sprites.getSprite (CameraUi::TimelineRefreshOnButtonSprite));
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraUi::autoReloadToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::CameraUiAutoReloadOffTooltip), UiText::instance->getText (UiTextString::CameraUiAutoReloadOnTooltip));
	toolbar->addLeftItem (toggle);
	if (isAutoReloading) {
		toggle->setChecked (true, true);
	}
	autoReloadToggle.assign (toggle);

	prefs = App::instance->lockPrefs ();
	toolbarMode = prefs->find (CameraUi::ToolbarModeKey, (int) -1);
	App::instance->unlockPrefs ();
	if (toolbarMode < 0) {
		toolbarMode = CameraUi::CameraMode;
	}
	setToolbarMode (toolbarMode, true);
}

void CameraUi::setToolbarMode (int mode, bool forceReset) {
	Toolbar *toolbar;
	Button *button;

	if ((toolbarMode == mode) && (! forceReset)) {
		return;
	}
	toolbar = App::instance->uiStack.secondaryToolbar;
	toolbar->clearRightItems ();
	configureCameraButton = NULL;
	clearTimelapseButton = NULL;
	showCameraImageButton = NULL;
	playCameraStreamButton = NULL;
	clearDisplayButton = NULL;

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ToolsButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraUi::modeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ToolbarModeButtonTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (button);

	toolbarMode = mode;
	switch (toolbarMode) {
		case CameraUi::CameraMode: {
			configureCameraButton = new Button (sprites.getSprite (CameraUi::ConfigureTimelapseButtonSprite));
			configureCameraButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::configureCameraButtonClicked, this);
			configureCameraButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			configureCameraButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			configureCameraButton->setInverseColor (true);
			configureCameraButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CameraUiConfigureCameraTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (configureCameraButton);

			clearTimelapseButton = new Button (sprites.getSprite (CameraUi::ClearTimelapseButtonSprite));
			clearTimelapseButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::clearTimelapseButtonClicked, this);
			clearTimelapseButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			clearTimelapseButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			clearTimelapseButton->setInverseColor (true);
			clearTimelapseButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CameraUiClearTimelapseTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (clearTimelapseButton);
			break;
		}
		case CameraUi::MonitorMode: {
			showCameraImageButton = new Button (sprites.getSprite (CameraUi::ShowCameraImageButtonSprite));
			showCameraImageButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::showCameraImageButtonClicked, this);
			showCameraImageButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			showCameraImageButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			showCameraImageButton->setInverseColor (true);
			showCameraImageButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CameraUiShowCameraImageTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (showCameraImageButton);

			playCameraStreamButton = new Button (sprites.getSprite (CameraUi::PlayCameraStreamButtonSprite));
			playCameraStreamButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::playCameraStreamButtonClicked, this);
			playCameraStreamButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			playCameraStreamButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			playCameraStreamButton->setInverseColor (true);
			playCameraStreamButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CameraUiPlayCameraStreamTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (playCameraStreamButton);

			clearDisplayButton = new Button (sprites.getSprite (CameraUi::ClearDisplayButtonSprite));
			clearDisplayButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::clearDisplayButtonClicked, this);
			clearDisplayButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			clearDisplayButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			clearDisplayButton->setInverseColor (true);
			clearDisplayButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CameraUiClearDisplayTooltip), Widget::LeftAlignment);
			clearDisplayButton->shortcutKey = SDLK_F1;
			toolbar->addRightItem (clearDisplayButton);
			break;
		}
	}
}

void CameraUi::modeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Menu *menu;

	ui = (CameraUi *) uiPtr;
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraUi::modeButtonClicked)) {
		return;
	}
	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextString::ManageCameras).capitalized (), ui->sprites.getSprite (CameraUi::ConfigureTimelapseButtonSprite), Widget::EventCallbackContext (CameraUi::cameraModeActionClicked, ui), 0, ui->toolbarMode == CameraUi::CameraMode);
	menu->addItem (UiText::instance->getText (UiTextString::ViewCameraImages).capitalized (), ui->sprites.getSprite (CameraUi::ShowCameraImageButtonSprite), Widget::EventCallbackContext (CameraUi::monitorModeActionClicked, ui), 0, ui->toolbarMode == CameraUi::MonitorMode);

	ui->showActionPopup (menu, widgetPtr, CameraUi::modeButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void CameraUi::cameraModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;

	ui = (CameraUi *) uiPtr;
	if (ui->toolbarMode != CameraUi::CameraMode) {
		ui->setToolbarMode (CameraUi::CameraMode);
	}
}

void CameraUi::monitorModeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;

	ui = (CameraUi *) uiPtr;
	if (ui->toolbarMode != CameraUi::MonitorMode) {
		ui->setToolbarMode (CameraUi::MonitorMode);
	}
}

void CameraUi::doClearPopupWidgets () {
	commandPopup.destroyAndClear ();
	commandPopupSource.clear ();
}

static void doPause_appendSelectedItemId (void *stringListPtr, Widget *widgetPtr) {
	CameraWindow *camera;
	MonitorWindow *monitor;

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera && camera->isSelected) {
		((StringList *) stringListPtr)->push_back (camera->itemId);
		return;
	}
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor && monitor->isSelected) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
		return;
	}
}
static void doPause_appendExpandedItemId (void *stringListPtr, Widget *widgetPtr) {
	CameraWindow *camera;
	MonitorWindow *monitor;

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera && camera->isExpanded) {
		((StringList *) stringListPtr)->push_back (camera->itemId);
		return;
	}
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor && monitor->isExpanded) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
		return;
	}
}
void CameraUi::doPause () {
	HashMap *prefs;
	StringList items;

	items.clear ();
	cardView->processItems (doPause_appendSelectedItemId, &items);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::SelectedItemsKey, items);
	App::instance->unlockPrefs ();

	items.clear ();
	cardView->processItems (doPause_appendExpandedItemId, &items);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::ExpandedItemsKey, items);
	prefs->insert (CameraUi::ToolbarModeKey, toolbarMode);
	prefs->insert (CameraUi::AutoReloadKey, isAutoReloading);
	App::instance->unlockPrefs ();
}

void CameraUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/CameraUi/bg");
}

void CameraUi::doUpdate (int msElapsed) {
	autoReloadToggle.compact ();
	emptyStateWindow.compact ();
	commandPopup.compact ();
	expandCamerasToggle.compact ();
	expandMonitorsToggle.compact ();
	targetCamera.compact ();
}

void CameraUi::handleLinkClientConnect (const StdString &agentId) {
	AgentControl::instance->writeLinkCommand (App::instance->createCommand (SystemInterface::Command_WatchStatus), agentId);
}

bool CameraUi::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	int mode;

	if (keycode == SDLK_TAB) {
		mode = toolbarMode + 1;
		if (mode >= CameraUi::ModeCount) {
			mode = 0;
		}
		clearPopupWidgets ();
		setToolbarMode (mode);
		return (true);
	}
	return (false);
}

void CameraUi::doSyncRecordStore () {
	IconCardWindow *window;

	cameraCount = 0;
	RecordStore::instance->processAgentRecords ("cameraServerStatus", CameraUi::doSyncRecordStore_processCameraAgent, this);
	RecordStore::instance->processAgentRecords ("monitorServerStatus", CameraUi::doSyncRecordStore_processMonitorAgent, this);

	window = (IconCardWindow *) emptyStateWindow.widget;
	if (cameraCount <= 0) {
		if (! window) {
			window = new IconCardWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite));
			window->setName (UiText::instance->getText (UiTextString::CameraUiEmptyAgentStatusTitle));
			window->setDetailText (UiText::instance->getText (UiTextString::CameraUiEmptyAgentStatusText));
			window->setLink (UiText::instance->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
			emptyStateWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, CameraUi::UnexpandedCameraRow);
		}
	}
	else {
		if (window) {
			cardView->removeItem (window->itemId);
			emptyStateWindow.clear ();
		}
	}

	cardView->syncRecordStore ();
	cardView->refresh ();
	resetExpandToggles ();
}

StdString CameraUi::getSelectedCameraNames (float maxWidth) {
	StdString text, id;
	HashMap::Iterator i;
	StringList names;
	int count;

	count = selectedCameraMap.size ();
	if (count <= 0) {
		return (StdString (""));
	}

	i = selectedCameraMap.begin ();
	while (selectedCameraMap.hasNext (&i)) {
		id = selectedCameraMap.next (&i);
		names.push_back (selectedCameraMap.find (id, ""));
	}

	names.sort (StringList::compareCaseInsensitiveAscending);
	text.assign (names.join (", "));
	UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, maxWidth, StdString::createSprintf ("... (%i)", count));
	return (text);
}

StdString CameraUi::getSelectedMonitorNames (float maxWidth) {
	StdString text, id;
	HashMap::Iterator i;
	StringList names;
	int count;

	count = selectedMonitorMap.size ();
	if (count <= 0) {
		return (StdString (""));
	}

	i = selectedMonitorMap.begin ();
	while (selectedMonitorMap.hasNext (&i)) {
		id = selectedMonitorMap.next (&i);
		names.push_back (selectedMonitorMap.find (id, ""));
	}

	names.sort (StringList::compareCaseInsensitiveAscending);
	text.assign (names.join (", "));
	UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncateText (&text, maxWidth, StdString::createSprintf ("... (%i)", count));
	return (text);
}

void CameraUi::doSyncRecordStore_processCameraAgent (void *uiPtr, Json *record, const StdString &recordId) {
	CameraUi *ui;
	CameraWindow *camera;
	Json serverstatus, sensorstatus;
	HashMap *prefs;
	StringList items;
	StdString cardid;
	int sensor, row, pos;

	ui = (CameraUi *) uiPtr;
	if (! SystemInterface::instance->getCommandObjectParam (record, "cameraServerStatus", &serverstatus)) {
		return;
	}
	++(ui->cameraCount);

	sensor = 0;
	while (serverstatus.getArrayObject ("sensors", sensor, &sensorstatus)) {
		cardid.sprintf ("%s_%i", recordId.c_str (), sensor);
		if (! ui->cardView->contains (cardid)) {
			camera = new CameraWindow (recordId, sensor, &(ui->sprites));
			camera->itemId.assign (cardid);
			camera->syncRecordStore ();
			camera->selectStateChangeCallback = Widget::EventCallbackContext (CameraUi::cameraSelectStateChanged, ui);
			camera->expandStateChangeCallback = Widget::EventCallbackContext (CameraUi::cameraExpandStateChanged, ui);
			camera->openButtonClickCallback = Widget::EventCallbackContext (CameraUi::cameraOpenButtonClicked, ui);

			prefs = App::instance->lockPrefs ();
			prefs->find (CameraUi::SelectedItemsKey, &items);
			if (items.contains (camera->itemId)) {
				camera->setSelected (true, true);
				ui->selectedCameraMap.insert (camera->itemId, camera->agentName);
			}
			prefs->find (CameraUi::ExpandedItemsKey, &items);
			App::instance->unlockPrefs ();

			pos = items.indexOf (camera->itemId);
			if (pos < 0) {
				row = CameraUi::UnexpandedCameraRow;
				camera->sortKey.assign (camera->agentName.lowercased ());
			}
			else {
				row = CameraUi::ExpandedCameraRow;
				camera->setExpanded (true, true);
				camera->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
			}

			ui->cardView->addItem (camera, camera->itemId, row, true);
			camera->animateNewCard ();
			camera->setAutoRefresh (ui->isAutoReloading);
			AgentControl::instance->refreshAgentStatus (recordId);
		}
		++sensor;
	}
}

void CameraUi::doSyncRecordStore_processMonitorAgent (void *uiPtr, Json *record, const StdString &recordId) {
	CameraUi *ui;
	HashMap *prefs;
	MonitorWindow *monitor;
	StringList items;
	int pos, row;

	ui = (CameraUi *) uiPtr;
	if (! ui->cardView->contains (recordId)) {
		monitor = new MonitorWindow (recordId);
		monitor->selectStateChangeCallback = Widget::EventCallbackContext (CameraUi::monitorSelectStateChanged, ui);
		monitor->expandStateChangeCallback = Widget::EventCallbackContext (CameraUi::monitorExpandStateChanged, ui);
		monitor->screenshotLoadCallback = Widget::EventCallbackContext (CameraUi::monitorScreenshotLoaded, ui);
		monitor->setSelectEnabled (true);
		monitor->setScreenshotDisplayEnabled (true);

		prefs = App::instance->lockPrefs ();
		prefs->find (CameraUi::SelectedItemsKey, &items);
		if (items.contains (recordId)) {
			monitor->setSelected (true, true);
			ui->selectedMonitorMap.insert (recordId, Agent::getCommandAgentName (record));
		}
		prefs->find (CameraUi::ExpandedItemsKey, &items);
		App::instance->unlockPrefs ();

		pos = items.indexOf (recordId);
		if (pos < 0) {
			row = CameraUi::UnexpandedMonitorRow;
			monitor->sortKey.assign (Agent::getCommandAgentName (record).lowercased ());
		}
		else {
			row = CameraUi::ExpandedMonitorRow;
			monitor->setExpanded (true, true);
			monitor->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
		}

		ui->cardView->addItem (monitor, recordId, row, true);
		monitor->animateNewCard ();
		ui->addLinkAgent (recordId);
	}
}

void CameraUi::cameraSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraWindow *camera;

	ui = (CameraUi *) uiPtr;
	camera = (CameraWindow *) widgetPtr;
	if (camera->isSelected) {
		ui->selectedCameraMap.insert (camera->itemId, camera->agentName);
	}
	else {
		ui->selectedCameraMap.remove (camera->itemId);
	}
	ui->clearPopupWidgets ();
}

void CameraUi::cameraExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraWindow *camera;

	ui = (CameraUi *) uiPtr;
	camera = (CameraWindow *) widgetPtr;
	if (camera->isExpanded) {
		camera->sortKey.sprintf ("%016llx", (long long int) OsUtil::getTime ());
		camera->setSelectedTime (-1);
		ui->cardView->setItemRow (camera->itemId, CameraUi::ExpandedCameraRow, true);
	}
	else {
		camera->sortKey.assign (camera->agentName.lowercased ());
		ui->cardView->setItemRow (camera->itemId, CameraUi::UnexpandedCameraRow, true);
	}
	camera->resetInputState ();
	camera->animateNewCard ();
	ui->cardView->refresh ();
	ui->clearPopupWidgets ();
	ui->resetExpandToggles ();
}

void CameraUi::monitorSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	MonitorWindow *monitor;

	ui = (CameraUi *) uiPtr;
	monitor = (MonitorWindow *) widgetPtr;
	if (monitor->isSelected) {
		ui->selectedMonitorMap.insert (monitor->agentId, monitor->agentName);
	}
	else {
		ui->selectedMonitorMap.remove (monitor->agentId);
	}
	ui->clearPopupWidgets ();
}

void CameraUi::monitorExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	MonitorWindow *monitor;

	ui = (CameraUi *) uiPtr;
	monitor = (MonitorWindow *) widgetPtr;
	if (monitor->isExpanded) {
		monitor->sortKey.sprintf ("%016llx", (long long int) OsUtil::getTime ());
		ui->cardView->setItemRow (monitor->agentId, CameraUi::ExpandedMonitorRow);
	}
	else {
		monitor->sortKey.assign (monitor->agentName.lowercased ());
		ui->cardView->setItemRow (monitor->agentId, CameraUi::UnexpandedMonitorRow);
	}
	monitor->resetInputState ();
	monitor->animateNewCard ();
	ui->cardView->refresh ();
	ui->clearPopupWidgets ();
	ui->resetExpandToggles ();
}

void CameraUi::monitorScreenshotLoaded (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;

	ui = (CameraUi *) uiPtr;
	ui->cardView->refresh ();
}

void CameraUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Menu *menu;

	ui = (CameraUi *) uiPtr;
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraUi::imageSizeButtonClicked)) {
		return;
	}
	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextString::Small).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), Widget::EventCallbackContext (CameraUi::smallImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Medium).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), Widget::EventCallbackContext (CameraUi::mediumImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (UiText::instance->getText (UiTextString::Large).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), Widget::EventCallbackContext (CameraUi::largeImageSizeActionClicked, ui), 0, ui->cardDetail == CardView::HighDetail);

	ui->showActionPopup (menu, widgetPtr, CameraUi::imageSizeButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::BottomOfAlignment);
}

void CameraUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	HashMap *prefs;
	int detail;

	ui = (CameraUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraUi::ExpandedCameraRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void CameraUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	HashMap *prefs;
	int detail;

	ui = (CameraUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraUi::ExpandedCameraRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

void CameraUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	HashMap *prefs;
	int detail;

	ui = (CameraUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraUi::ExpandedCameraRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

static void expandCamerasToggleStateChanged_appendAgentId (void *stringListPtr, Widget *widgetPtr) {
	CameraWindow *camera;

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera) {
		((StringList *) stringListPtr)->push_back (camera->itemId);
		return;
	}
}
void CameraUi::expandCamerasToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Toggle *toggle;
	CameraWindow *camera;
	StringList idlist;
	StringList::iterator i, end;
	int64_t now;

	ui = (CameraUi *) uiPtr;
	toggle = (Toggle *) ui->expandCamerasToggle.widget;
	if (! toggle) {
		return;
	}
	now = OsUtil::getTime ();
	ui->cardView->processItems (expandCamerasToggleStateChanged_appendAgentId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		camera = CameraWindow::castWidget (ui->cardView->getItem (*i));
		if (camera) {
			if (toggle->isChecked) {
				camera->setExpanded (false, true);
				camera->sortKey.assign (camera->agentName.lowercased ());
				ui->cardView->setItemRow (camera->itemId, CameraUi::UnexpandedCameraRow, true);
			}
			else {
				camera->setExpanded (true, true);
				camera->sortKey.sprintf ("%016llx%s_%i", (long long int) now, camera->agentName.lowercased ().c_str (), camera->sensor);
				ui->cardView->setItemRow (camera->itemId, CameraUi::ExpandedCameraRow, true);
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

static void expandMonitorsToggleStateChanged_appendAgentId (void *stringListPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;

	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
		return;
	}
}
void CameraUi::expandMonitorsToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Toggle *toggle;
	MonitorWindow *monitor;
	StringList idlist;
	StringList::iterator i, end;
	int64_t now;

	ui = (CameraUi *) uiPtr;
	toggle = (Toggle *) ui->expandMonitorsToggle.widget;
	if (! toggle) {
		return;
	}
	now = OsUtil::getTime ();
	ui->cardView->processItems (expandMonitorsToggleStateChanged_appendAgentId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		monitor = MonitorWindow::castWidget (ui->cardView->getItem (*i));
		if (monitor) {
			if (toggle->isChecked) {
				monitor->setExpanded (false, true);
				monitor->sortKey.assign (monitor->agentName.lowercased ());
				ui->cardView->setItemRow (monitor->agentId, CameraUi::UnexpandedMonitorRow, true);
			}
			else {
				monitor->setExpanded (true, true);
				monitor->sortKey.sprintf ("%016llx%s", (long long int) now, monitor->agentName.lowercased ().c_str ());
				ui->cardView->setItemRow (monitor->agentId, CameraUi::ExpandedMonitorRow, true);
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

void CameraUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;

	ui = (CameraUi *) uiPtr;
	ui->cardView->processItems (CameraUi::reloadAgent, ui);
}

void CameraUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	CameraWindow *camera;
	MonitorWindow *monitor;

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera) {
		AgentControl::instance->refreshAgentStatus (camera->agentId);
		return;
	}
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		AgentControl::instance->refreshAgentStatus (monitor->agentId);
		return;
	}
}

static void autoReloadToggleStateChanged_processCameras (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraWindow *camera;

	ui = (CameraUi *) uiPtr;
	camera = CameraWindow::castWidget (widgetPtr);
	if (camera) {
		camera->setAutoRefresh (ui->isAutoReloading);
	}
}
void CameraUi::autoReloadToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Toggle *toggle;

	ui = (CameraUi *) uiPtr;
	toggle = (Toggle *) ui->autoReloadToggle.widget;
	if (! toggle) {
		return;
	}
	ui->isAutoReloading = toggle->isChecked;
	ui->cardView->processRowItems (CameraUi::ExpandedCameraRow, autoReloadToggleStateChanged_processCameras, ui);
	if (ui->isAutoReloading) {
		ui->cardView->processItems (CameraUi::reloadAgent, ui);
	}
}

void CameraUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Button *button;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	StdString text;
	Color color;

	ui = (CameraUi *) uiPtr;
	button = (Button *) widgetPtr;
	Button::mouseEntered (button, button);

	ui->commandPopup.destroyAndClear ();
	ui->commandPopupSource.assign (button);
	panel = (Panel *) App::instance->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	panel->setBorder (true, Color (UiConfiguration::instance->darkBackgroundColor.r, UiConfiguration::instance->darkBackgroundColor.g, UiConfiguration::instance->darkBackgroundColor.b, UiConfiguration::instance->overlayWindowAlpha));

	label = NULL;
	if (button == ui->configureCameraButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::ConfigureCamera).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedCameraNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoCameraSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearTimelapseButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::ClearTimelapse).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedCameraNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoCameraSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->showCameraImageButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::ShowCameraImageCommandName), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedCameraNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoCameraSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->playCameraStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::PlayCameraStreamCommandName), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedCameraNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoCameraSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearDisplayButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::Clear).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (UiConfiguration::instance->primaryTextColor);
		if (text.empty ()) {
			text.assign (UiText::instance->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (UiConfiguration::instance->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		icon->setRightAligned (true);
	}

	if (label) {
		label->setPadding (0.0f, 0.0f);
	}
	panel->setLayout (Panel::VerticalRightJustifiedLayout);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	panel->position.assign (App::instance->windowWidth - panel->width - UiConfiguration::instance->paddingSize, App::instance->windowHeight - App::instance->uiStack.bottomBarHeight - panel->height - UiConfiguration::instance->marginSize);
}

void CameraUi::commandButtonMouseExited (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Button *button;

	ui = (CameraUi *) uiPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);

	if (ui->commandPopupSource.widget == button) {
		ui->commandPopup.destroyAndClear ();
		ui->commandPopupSource.clear ();
	}
}

void CameraUi::configureCameraButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	Toggle *toggle;
	ComboBox *combobox;
	SliderWindow *slider;
	CameraWindow *camera;
	HashMap::Iterator i;
	StdString id;
	int j, imagequality, count, captureperiod, flip;
	bool enabled;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCameraMap.empty ()) {
		return;
	}
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraUi::configureCameraButtonClicked)) {
		return;
	}

	enabled = true;
	imagequality = SystemInterface::Constant_DefaultImageProfile;
	captureperiod = CameraUi::DefaultCapturePeriodIndex;
	flip = SystemInterface::Constant_NoFlip;
	i = ui->selectedCameraMap.begin ();
	while (ui->selectedCameraMap.hasNext (&i)) {
		id = ui->selectedCameraMap.next (&i);
		camera = CameraWindow::castWidget (ui->cardView->getItem (id));
		if (camera) {
			enabled = camera->isCapturing;
			imagequality = camera->imageProfile;

			count = sizeof (CameraUi::CapturePeriods) / sizeof (CameraUi::CapturePeriods[0]);
			for (j = 0; j < count; ++j) {
				if (camera->capturePeriod == CameraUi::CapturePeriods[j]) {
					captureperiod = j;
					break;
				}
			}

			flip = camera->flip;
			break;
		}
	}

	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextString::ConfigureCamera).capitalized ());
	action->optionChangeCallback = Widget::EventCallbackContext (CameraUi::configureCameraActionOptionChanged, ui);
	action->closeCallback = Widget::EventCallbackContext (CameraUi::configureCameraActionClosed, ui);

	toggle = new Toggle ();
	toggle->setChecked (enabled);
	action->addOption (UiText::instance->getText (UiTextString::EnableTimelapse).capitalized (), toggle);

	count = sizeof (CameraUi::CapturePeriods) / sizeof (CameraUi::CapturePeriods[0]);
	slider = new SliderWindow (new Slider (0.0f, (float) count));
	slider->setPadding (0.0f, 0.0f);
	slider->setValueNameFunction (CameraUi::capturePeriodValueName);
	for (j = 0; j < count; ++j) {
		slider->addSnapValue ((float) j);
	}
	slider->setValue (captureperiod);
	action->addOption (UiText::instance->getText (UiTextString::CapturePeriod).capitalized (), slider, UiText::instance->getText (UiTextString::CapturePeriodDescription));

	combobox = new ComboBox ();
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_DefaultImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_DefaultImageProfile));
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_HighQualityImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_HighQualityImageProfile));
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_LowQualityImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_LowQualityImageProfile));
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_LowestQualityImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_LowestQualityImageProfile));
	combobox->setValueByItemData (StdString::createSprintf ("%i", imagequality));
	action->addOption (UiText::instance->getText (UiTextString::CaptureQuality).capitalized (), combobox, UiText::instance->getText (UiTextString::CaptureQualityDescription));

	combobox = new ComboBox ();
	combobox->addItem (UiText::instance->getText (UiTextString::None).capitalized (), StdString::createSprintf ("%i", SystemInterface::Constant_NoFlip));
	combobox->addItem (UiText::instance->getText (UiTextString::Horizontal).capitalized (), StdString::createSprintf ("%i", SystemInterface::Constant_HorizontalFlip));
	combobox->addItem (UiText::instance->getText (UiTextString::Vertical).capitalized (), StdString::createSprintf ("%i", SystemInterface::Constant_VerticalFlip));
	combobox->addItem (StdString::createSprintf ("%s %s %s", UiText::instance->getText (UiTextString::Horizontal).capitalized ().c_str (), UiText::instance->getText (UiTextString::And).c_str (), UiText::instance->getText (UiTextString::Vertical).c_str ()), StdString::createSprintf ("%i", SystemInterface::Constant_HorizontalAndVerticalFlip));
	combobox->setValueByItemData (StdString::createSprintf ("%i", flip));
	action->addOption (UiText::instance->getText (UiTextString::ImageFlip).capitalized (), combobox);

	if (! enabled) {
		action->setOptionDisabled (UiText::instance->getText (UiTextString::CapturePeriod).capitalized (), true);
	}

	ui->showActionPopup (action, widgetPtr, CameraUi::configureCameraButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

StdString CameraUi::capturePeriodValueName (float sliderValue) {
	int i, count;

	i = (int) sliderValue;
	count = sizeof (CameraUi::CapturePeriods) / sizeof (CameraUi::CapturePeriods[0]);
	if ((i < 0) || (i >= count)) {
		return (StdString (""));
	}
	if (CameraUi::CapturePeriods[i] == 0) {
		return (UiText::instance->getText (UiTextString::Continuous).capitalized ());
	}
	return (OsUtil::getDurationDisplayString (CameraUi::CapturePeriods[i] * 1000));
}

void CameraUi::configureCameraActionOptionChanged (void *uiPtr, Widget *widgetPtr) {
	ActionWindow *action;
	bool disabled;

	action = (ActionWindow *) widgetPtr;
	disabled = (! action->getBooleanValue (UiText::instance->getText (UiTextString::EnableTimelapse).capitalized (), false));
	action->setOptionDisabled (UiText::instance->getText (UiTextString::CapturePeriod).capitalized (), disabled);
}

void CameraUi::configureCameraActionClosed (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	StringList idlist, agentids;
	StringList::iterator i, end;
	CameraWindow *camera;
	Json *params;
	JsonList commands;

	action = (ActionWindow *) widgetPtr;
	ui = (CameraUi *) uiPtr;
	ui->selectedCameraMap.getKeys (&idlist, true);
	if ((! action->isConfirmed) || idlist.empty ()) {
		return;
	}
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		camera = CameraWindow::castWidget (ui->cardView->getItem (*i));
		if (camera) {
			params = new Json ();
			params->set ("sensor", camera->sensor);
			params->set ("isCaptureEnabled", action->getBooleanValue (UiText::instance->getText (UiTextString::EnableTimelapse).capitalized (), false));
			params->set ("capturePeriod", CameraUi::CapturePeriods[action->getNumberValue (UiText::instance->getText (UiTextString::CapturePeriod).capitalized (), CameraUi::DefaultCapturePeriodIndex)]);
			params->set ("flip", action->getNumberValue (UiText::instance->getText (UiTextString::ImageFlip).capitalized (), SystemInterface::Constant_NoFlip));
			params->set ("imageProfile", action->getNumberValue (UiText::instance->getText (UiTextString::CaptureQuality).capitalized (), SystemInterface::Constant_DefaultImageProfile));
			agentids.push_back (camera->agentId);
			commands.push_back (App::instance->createCommand (SystemInterface::Command_ConfigureCamera, params));
		}
		++i;
	}
	ui->invokeCommand (CommandHistory::instance->configureCamera (agentids), agentids, &commands, CameraUi::invokeCameraCommandComplete);
}

void CameraUi::invokeCameraCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess) {
	if (isResponseCommandSuccess) {
		AgentControl::instance->refreshAgentStatus (agentId);
	}
}

void CameraUi::clearTimelapseButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCameraMap.empty ()) {
		return;
	}
	if (ui->clearActionPopup (widgetPtr, CameraUi::clearTimelapseButtonClicked)) {
		return;
	}
	action = new ActionWindow ();
	action->setInverseColor (true);
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setTitleText (UiText::instance->getText (UiTextString::ClearTimelapse).capitalized ());
	action->setDescriptionText (UiText::instance->getText (UiTextString::ClearTimelapseActionText));
	action->setConfirmTooltipText (UiText::instance->getText (UiTextString::Clear).capitalized ());
	action->closeCallback = Widget::EventCallbackContext (CameraUi::clearTimelapseActionClosed, ui);

	ui->showActionPopup (action, widgetPtr, CameraUi::clearTimelapseButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void CameraUi::clearTimelapseActionClosed (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	CameraWindow *camera;
	StringList idlist, agentids;
	StringList::iterator i, end;

	ui = (CameraUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	ui->selectedCameraMap.getKeys (&idlist, true);
	if ((! action->isConfirmed) || idlist.empty ()) {
		return;
	}
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		camera = CameraWindow::castWidget (ui->cardView->getItem (*i));
		if (camera) {
			agentids.push_back (camera->agentId);
		}
		++i;
	}
	if (agentids.empty ()) {
		return;
	}
	ui->invokeCommand (CommandHistory::instance->clearTimelapse (agentids), agentids, App::instance->createCommand (SystemInterface::Command_ClearTimelapse), CameraUi::invokeCameraCommandComplete);
}

void CameraUi::showCameraImageButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	Toggle *toggle;
	IconLabelWindow *icon;
	Panel *panel;
	int count;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCameraMap.empty () || ui->selectedMonitorMap.empty ()) {
		return;
	}
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraUi::showCameraImageButtonClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextString::ShowCameraImageCommandName));
	action->setConfirmTooltipText (UiText::instance->getText (UiTextString::Confirm).capitalized ());
	action->closeCallback = Widget::EventCallbackContext (CameraUi::showCameraImageActionClosed, ui);

	toggle = new Toggle ();
	action->addOption (UiText::instance->getText (UiTextString::CameraUiShowCameraImageAutoRefreshPrompt), toggle);

	panel = new Panel ();
	count = (int) ui->selectedCameraMap.size ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::CameraSelected, UiTextString::CamerasSelected));

	count = (int) ui->selectedMonitorMap.size ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::MonitorSelected, UiTextString::MonitorsSelected));

	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, CameraUi::showCameraImageButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void CameraUi::showCameraImageActionClosed (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	CameraWindow *camera;
	StringList cameraids, monitorids;
	StringList::const_iterator i, end, pos;
	StdString authpath, authsecret;
	JsonList commands;
	Json *cmd, *params, *agenthost;
	bool autorefresh;

	ui = (CameraUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	ui->selectedCameraMap.getKeys (&cameraids, true);
	ui->selectedMonitorMap.getKeys (&monitorids, true);
	if ((! action->isConfirmed) || cameraids.empty () || monitorids.empty ()) {
		return;
	}
	autorefresh = action->getBooleanValue (UiText::instance->getText (UiTextString::CameraUiShowCameraImageAutoRefreshPrompt), false);
	pos = cameraids.cbegin ();
	i = monitorids.cbegin ();
	end = monitorids.cend ();
	while (i != end) {
		camera = CameraWindow::castWidget (ui->cardView->getItem (cameraids.loopNext (&pos)));
		if (camera) {
			params = new Json ();
			agenthost = new Json ();
			agenthost->set ("hostname", AgentControl::instance->getAgentHostAddress (camera->agentId));
			if (AgentControl::instance->getAgentAuthorization (camera->agentId, &authpath, &authsecret)) {
				agenthost->set ("authorizePath", authpath);
				agenthost->set ("authorizeSecret", authsecret);
			}
			params->set ("host", agenthost);
			params->set ("sensor", camera->sensor);

			if (autorefresh) {
				params->set ("displayName", UiText::instance->getText (UiTextString::CreateCameraImageDisplayIntentCommandName));
				cmd = App::instance->createCommand (SystemInterface::Command_CreateCameraImageDisplayIntent, params);
			}
			else {
				if (camera->selectedTime > 0) {
					params->set ("imageTime", camera->selectedTime);
				}
				cmd = App::instance->createCommand (SystemInterface::Command_ShowCameraImage, params);
			}
			commands.push_back (cmd);
		}
		++i;
	}
	if (commands.empty ()) {
		return;
	}
	ui->invokeCommand (CommandHistory::instance->showCameraImage (monitorids), monitorids, &commands, CameraUi::invokeMonitorCommandComplete);
}

void CameraUi::playCameraStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	ComboBox *combobox;
	Toggle *toggle;
	IconLabelWindow *icon;
	Panel *panel;
	CameraWindow *camera;
	HashMap::Iterator i;
	StdString id;
	int streamprofile, flip, count;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCameraMap.empty () || ui->selectedMonitorMap.empty ()) {
		return;
	}
	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraUi::playCameraStreamButtonClicked)) {
		return;
	}

	streamprofile = SystemInterface::Constant_DefaultCameraStreamProfile;
	flip = SystemInterface::Constant_NoFlip;
	i = ui->selectedCameraMap.begin ();
	while (ui->selectedCameraMap.hasNext (&i)) {
		id = ui->selectedCameraMap.next (&i);
		camera = CameraWindow::castWidget (ui->cardView->getItem (id));
		if (camera) {
			switch (camera->imageProfile) {
				case SystemInterface::Constant_LowQualityImageProfile: {
					streamprofile = SystemInterface::Constant_LowQualityCameraStreamProfile;
					break;
				}
				case SystemInterface::Constant_LowestQualityImageProfile: {
					streamprofile = SystemInterface::Constant_LowestQualityCameraStreamProfile;
					break;
				}
			}
			flip = camera->flip;
			break;
		}
	}

	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextString::PlayCameraStreamCommandName));
	action->setConfirmTooltipText (UiText::instance->getText (UiTextString::Confirm).capitalized ());
	action->closeCallback = Widget::EventCallbackContext (CameraUi::playCameraStreamActionClosed, ui);

	combobox = new ComboBox ();
	combobox->addItem (UiText::instance->getText (UiTextString::NormalImageQualityDescription), StdString::createSprintf ("%i", SystemInterface::Constant_DefaultCameraStreamProfile));
	combobox->addItem (UiText::instance->getText (UiTextString::LowImageQualityDescription), StdString::createSprintf ("%i", SystemInterface::Constant_LowQualityCameraStreamProfile));
	combobox->addItem (UiText::instance->getText (UiTextString::LowestImageQualityDescription), StdString::createSprintf ("%i", SystemInterface::Constant_LowestQualityCameraStreamProfile));
	combobox->setValueByItemData (StdString::createSprintf ("%i", streamprofile));
	action->addOption (UiText::instance->getText (UiTextString::CaptureQuality).capitalized (), combobox);

	combobox = new ComboBox ();
	combobox->addItem (UiText::instance->getText (UiTextString::None).capitalized (), StdString::createSprintf ("%i", SystemInterface::Constant_NoFlip));
	combobox->addItem (UiText::instance->getText (UiTextString::Horizontal).capitalized (), StdString::createSprintf ("%i", SystemInterface::Constant_HorizontalFlip));
	combobox->addItem (UiText::instance->getText (UiTextString::Vertical).capitalized (), StdString::createSprintf ("%i", SystemInterface::Constant_VerticalFlip));
	combobox->addItem (StdString::createSprintf ("%s %s %s", UiText::instance->getText (UiTextString::Horizontal).capitalized ().c_str (), UiText::instance->getText (UiTextString::And).c_str (), UiText::instance->getText (UiTextString::Vertical).c_str ()), StdString::createSprintf ("%i", SystemInterface::Constant_HorizontalAndVerticalFlip));
	combobox->setValueByItemData (StdString::createSprintf ("%i", flip));
	action->addOption (UiText::instance->getText (UiTextString::ImageFlip).capitalized (), combobox);

	toggle = new Toggle ();
	action->addOption (UiText::instance->getText (UiTextString::CameraUiPlayCameraStreamAutoRefreshPrompt), toggle);

	panel = new Panel ();
	count = (int) ui->selectedCameraMap.size ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::CameraSelected, UiTextString::CamerasSelected));

	count = (int) ui->selectedMonitorMap.size ();
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), StdString::createSprintf ("%i", count), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::MonitorSelected, UiTextString::MonitorsSelected));

	panel->setLayout (Panel::HorizontalLayout);
	action->setFooterPanel (panel);

	ui->showActionPopup (action, widgetPtr, CameraUi::playCameraStreamButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void CameraUi::playCameraStreamActionClosed (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	CameraWindow *camera;
	StringList cameraids, monitorids;
	StringList::const_iterator i, end, pos;
	StdString authpath, authsecret;
	JsonList commands;
	Json *cmd, *params, *agenthost;
	bool autorefresh;

	ui = (CameraUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	ui->selectedCameraMap.getKeys (&cameraids, true);
	ui->selectedMonitorMap.getKeys (&monitorids, true);
	if ((! action->isConfirmed) || cameraids.empty () || monitorids.empty ()) {
		return;
	}
	autorefresh = action->getBooleanValue (UiText::instance->getText (UiTextString::CameraUiPlayCameraStreamAutoRefreshPrompt), false);
	pos = cameraids.cbegin ();
	i = monitorids.cbegin ();
	end = monitorids.cend ();
	while (i != end) {
		camera = CameraWindow::castWidget (ui->cardView->getItem (cameraids.loopNext (&pos)));
		if (camera) {
			params = new Json ();
			agenthost = new Json ();
			agenthost->set ("hostname", AgentControl::instance->getAgentHostAddress (camera->agentId));
			if (AgentControl::instance->getAgentAuthorization (camera->agentId, &authpath, &authsecret)) {
				agenthost->set ("authorizePath", authpath);
				agenthost->set ("authorizeSecret", authsecret);
			}
			params->set ("host", agenthost);
			params->set ("sensor", camera->sensor);
			params->set ("flip", action->getNumberValue (UiText::instance->getText (UiTextString::ImageFlip).capitalized (), SystemInterface::Constant_NoFlip));
			params->set ("streamProfile", action->getNumberValue (UiText::instance->getText (UiTextString::CaptureQuality).capitalized (), SystemInterface::Constant_DefaultCameraStreamProfile));

			if (autorefresh) {
				params->set ("displayName", UiText::instance->getText (UiTextString::PlayCameraStreamCommandName));
				cmd = App::instance->createCommand (SystemInterface::Command_CreateCameraStreamDisplayIntent, params);
			}
			else {
				cmd = App::instance->createCommand (SystemInterface::Command_PlayCameraStream, params);
			}
			commands.push_back (cmd);
		}
		++i;
	}
	if (commands.empty ()) {
		return;
	}
	ui->invokeCommand (CommandHistory::instance->playCameraStream (monitorids), monitorids, &commands, CameraUi::invokeMonitorCommandComplete);
}

void CameraUi::clearDisplayButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	StringList idlist;

	ui = (CameraUi *) uiPtr;
	ui->selectedMonitorMap.getKeys (&idlist);
	if (idlist.empty ()) {
		return;
	}
	ui->invokeCommand (CommandHistory::instance->clearDisplay (idlist), idlist, App::instance->createCommand (SystemInterface::Command_ClearDisplay), CameraUi::invokeMonitorCommandComplete);
}

void CameraUi::invokeMonitorCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess) {
	if (isResponseCommandSuccess) {
		AgentControl::instance->refreshAgentStatus (agentId);
	}
}

void CameraUi::cameraOpenButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraWindow *camera;
	CameraTimelineUi *timelineui;

	ui = (CameraUi *) uiPtr;
	camera = CameraWindow::castWidget (widgetPtr);
	if (! camera) {
		return;
	}
	ui->targetCamera.assign (camera);
	timelineui = new CameraTimelineUi (camera);
	timelineui->thumbnailClickCallback = Widget::EventCallbackContext (CameraUi::cameraTimelineThumbnailClicked, ui);
	App::instance->uiStack.pushUi (timelineui);
}

void CameraUi::cameraTimelineThumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraThumbnailWindow *thumbnail;
	CameraWindow *camera;

	ui = (CameraUi *) uiPtr;
	thumbnail = CameraThumbnailWindow::castWidget (widgetPtr);
	camera = CameraWindow::castWidget (ui->targetCamera.widget);
	if (thumbnail && camera) {
		camera->setSelectedTime (thumbnail->thumbnailTimestamp);
	}

	App::instance->uiStack.popUi ();
}

static void resetExpandToggles_countExpandedCameras (void *intPtr, Widget *widgetPtr) {
	CameraWindow *camera;
	int *count;

	count = (int *) intPtr;
	camera = CameraWindow::castWidget (widgetPtr);
	if (count && camera && camera->isExpanded) {
		++(*count);
	}
}
static void resetExpandToggles_countExpandedMonitors (void *intPtr, Widget *widgetPtr) {
	MonitorWindow *monitor;
	int *count;

	count = (int *) intPtr;
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (count && monitor && monitor->isExpanded) {
		++(*count);
	}
}
void CameraUi::resetExpandToggles () {
	Toggle *toggle;
	int count;

	toggle = (Toggle *) expandCamerasToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (resetExpandToggles_countExpandedCameras, &count);
		toggle->setChecked ((count <= 0), true);
	}

	toggle = (Toggle *) expandMonitorsToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (resetExpandToggles_countExpandedMonitors, &count);
		toggle->setChecked ((count <= 0), true);
	}
}
