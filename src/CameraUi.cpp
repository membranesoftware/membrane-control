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
#include <map>
#include "SDL2/SDL.h"
#include "Result.h"
#include "StdString.h"
#include "Log.h"
#include "App.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "OsUtil.h"
#include "RecordStore.h"
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
#include "CameraCaptureWindow.h"
#include "CapturePlaylistWindow.h"
#include "CameraThumbnailWindow.h"
#include "CameraTimelineUi.h"
#include "CameraUi.h"

const char *CameraUi::SelectedAgentsKey = "Camera_SelectedAgents";
const char *CameraUi::ExpandedAgentsKey = "Camera_ExpandedAgents";
const char *CameraUi::SelectedCapturesKey = "Camera_SelectedCaptures";
const char *CameraUi::ImageSizeKey = "Camera_ImageSize";
const char *CameraUi::AutoReloadKey = "Camera_AutoReload";
const char *CameraUi::ToolbarModeKey = "Camera_ToolbarMode";
const char *CameraUi::PlaylistsKey = "Camera_Playlists";
const int CameraUi::CapturePeriods[] = { 0, 15, 60, 2 * 60, 3 * 60, 5 * 60, 10 * 60, 15 * 60, 30 * 60, 3600, 2 * 3600, 4 * 3600, 8 * 3600, 24 * 3600 };
const int CameraUi::DefaultCapturePeriodIndex = 1;
const int CameraUi::MinAutoReloadDelay = 5000; // ms

CameraUi::CameraUi ()
: Ui ()
, toolbarMode (-1)
, configureCameraButton (NULL)
, clearTimelapseButton (NULL)
, showCameraImageButton (NULL)
, showCapturePlaylistButton (NULL)
, playCameraStreamButton (NULL)
, clearDisplayButton (NULL)
, cameraCount (0)
, cardDetail (-1)
, isAutoReloading (false)
{
}

CameraUi::~CameraUi () {

}

StdString CameraUi::getSpritePath () {
	return (StdString ("ui/CameraUi/sprite"));
}

Widget *CameraUi::createBreadcrumbWidget () {
	return (new Chip (App::instance->uiText.getText (UiTextString::Cameras).capitalized (), sprites.getSprite (CameraUi::BreadcrumbIconSprite)));
}

void CameraUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::CameraUiHelpTitle), uitext->getText (UiTextString::CameraUiHelpText));
	if (cameraCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::CameraUiHelpAction1Text), uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
	}
	else {
		helpWindow->addAction (uitext->getText (UiTextString::CameraUiHelpAction2Text));
		helpWindow->addAction (uitext->getText (UiTextString::CameraUiHelpAction3Text));
	}

	helpWindow->addTopicLink (uitext->getText (UiTextString::SearchForHelp).capitalized (), App::getHelpUrl (""));
}

StdString CameraUi::getImageQualityDescription (int imageQuality) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	if (imageQuality == SystemInterface::Constant_DefaultImageProfile) {
		return (uitext->getText (UiTextString::NormalImageQualityDescription));
	}
	if (imageQuality == SystemInterface::Constant_HighQualityImageProfile) {
		return (uitext->getText (UiTextString::HighImageQualityDescription));
	}
	if (imageQuality == SystemInterface::Constant_LowQualityImageProfile) {
		return (uitext->getText (UiTextString::LowImageQualityDescription));
	}
	if (imageQuality == SystemInterface::Constant_LowestQualityImageProfile) {
		return (uitext->getText (UiTextString::LowestImageQualityDescription));
	}

	return (StdString (""));
}

std::map<StdString, CameraUi::AutoReloadInfo>::iterator CameraUi::getAutoReloadInfo (const StdString &agentId) {
	std::map<StdString, CameraUi::AutoReloadInfo>::iterator i;

	i = autoReloadMap.find (agentId);
	if (i == autoReloadMap.end ()) {
		autoReloadMap.insert (std::pair<StdString, CameraUi::AutoReloadInfo> (agentId, CameraUi::AutoReloadInfo ()));
		i = autoReloadMap.find (agentId);
	}

	return (i);
}

int CameraUi::doLoad () {
	HashMap *prefs;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	Toggle *toggle;
	Button *button;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	cameraCount = 0;
	prefs = App::instance->lockPrefs ();
	cardDetail = prefs->find (CameraUi::ImageSizeKey, (int) CardView::MediumDetail);
	isAutoReloading = prefs->find (CameraUi::AutoReloadKey, false);
	App::instance->unlockPrefs ();

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraUi::expandAgentsToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::MinimizeAll).capitalized (), uitext->getText (UiTextString::ExpandAll).capitalized ());
	expandAgentsToggle.assign (toggle);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::CameraUiAgentRowHeaderText), panel), StdString (""), CameraUi::AgentToggleRow);

	cardView->setRowReverseSorted (CameraUi::ExpandedAgentRow, true);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	toggle = (Toggle *) panel->addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllLessButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandAllMoreButtonSprite)));
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraUi::expandPlaylistsToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::MinimizeAll).capitalized (), uitext->getText (UiTextString::ExpandAll).capitalized ());
	expandPlaylistsToggle.assign (toggle);

	button = (Button *) panel->addWidget (new Button (sprites.getSprite (CameraUi::CreatePlaylistButtonSprite)));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraUi::createPlaylistButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::CameraUiAddPlaylistTooltip));

	panel->setLayout (Panel::HorizontalLayout);
	cardView->addItem (createRowHeaderPanel (uitext->getText (UiTextString::Playlists).capitalized (), panel), StdString (""), CameraUi::PlaylistToggleRow);

	cardView->setRowReverseSorted (CameraUi::ExpandedPlaylistRow, true);

	cardView->setRowHeader (CameraUi::CaptureRow, createRowHeaderPanel (uitext->getText (UiTextString::TimelapseImages).capitalized ()));
	cardView->setRowDetail (CameraUi::CaptureRow, cardDetail);

	return (Result::Success);
}

void CameraUi::doUnload () {
	autoReloadToggle.clear ();
	emptyStateWindow.clear ();
	selectedCameraMap.clear ();
	selectedMonitorMap.clear ();
	selectedCaptureMap.clear ();
	selectedPlaylistId.assign ("");
	expandAgentsToggle.clear ();
	expandPlaylistsToggle.clear ();
	targetCapture.clear ();
	autoReloadMap.clear ();
}

void CameraUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraUi::imageSizeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::ThumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraUi::reloadButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::ReloadTooltip));
	toolbar->addRightItem (button);
}

void CameraUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	HashMap *prefs;
	UiText *uitext;
	Toggle *toggle;

	uitext = &(App::instance->uiText);

	toggle = new Toggle (sprites.getSprite (CameraUi::TimelineRefreshOffButtonSprite), sprites.getSprite (CameraUi::TimelineRefreshOnButtonSprite));
	toggle->stateChangeCallback = Widget::EventCallbackContext (CameraUi::autoReloadToggleStateChanged, this);
	toggle->setInverseColor (true);
	toggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::CameraUiAutoReloadOffTooltip), uitext->getText (UiTextString::CameraUiAutoReloadOnTooltip));
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
	UiConfiguration *uiconfig;
	UiText *uitext;
	Toolbar *toolbar;
	Button *button;

	if ((toolbarMode == mode) && (! forceReset)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	toolbar = App::instance->uiStack.secondaryToolbar;
	toolbar->clearRightItems ();
	configureCameraButton = NULL;
	clearTimelapseButton = NULL;
	showCameraImageButton = NULL;
	showCapturePlaylistButton = NULL;
	playCameraStreamButton = NULL;
	clearDisplayButton = NULL;

	button = new Button (uiconfig->coreSprites.getSprite (UiConfiguration::ToolsButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (CameraUi::modeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::ToolbarModeButtonTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (button);

	toolbarMode = mode;
	switch (toolbarMode) {
		case CameraUi::CameraMode: {
			configureCameraButton = new Button (sprites.getSprite (CameraUi::ConfigureTimelapseButtonSprite));
			configureCameraButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::configureCameraButtonClicked, this);
			configureCameraButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			configureCameraButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			configureCameraButton->setInverseColor (true);
			configureCameraButton->setMouseHoverTooltip (uitext->getText (UiTextString::CameraUiConfigureCameraTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (configureCameraButton);

			clearTimelapseButton = new Button (sprites.getSprite (CameraUi::ClearTimelapseButtonSprite));
			clearTimelapseButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::clearTimelapseButtonClicked, this);
			clearTimelapseButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			clearTimelapseButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			clearTimelapseButton->setInverseColor (true);
			clearTimelapseButton->setMouseHoverTooltip (uitext->getText (UiTextString::CameraUiClearTimelapseTooltip), Widget::LeftAlignment);
			toolbar->addRightItem (clearTimelapseButton);
			break;
		}
		case CameraUi::MonitorMode: {
			showCameraImageButton = new Button (sprites.getSprite (CameraUi::ShowCameraImageButtonSprite));
			showCameraImageButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::showCameraImageButtonClicked, this);
			showCameraImageButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			showCameraImageButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			showCameraImageButton->setInverseColor (true);
			showCameraImageButton->setMouseHoverTooltip (uitext->getText (UiTextString::CameraUiShowCameraImageTooltip), Widget::LeftAlignment);
			showCameraImageButton->shortcutKey = SDLK_F4;
			toolbar->addRightItem (showCameraImageButton);

			showCapturePlaylistButton = new Button (sprites.getSprite (CameraUi::ShowCapturePlaylistButtonSprite));
			showCapturePlaylistButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::showCapturePlaylistButtonClicked, this);
			showCapturePlaylistButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			showCapturePlaylistButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			showCapturePlaylistButton->setInverseColor (true);
			showCapturePlaylistButton->setMouseHoverTooltip (uitext->getText (UiTextString::CameraUiShowCapturePlaylistTooltip), Widget::LeftAlignment);
			showCapturePlaylistButton->shortcutKey = SDLK_F3;
			toolbar->addRightItem (showCapturePlaylistButton);

			playCameraStreamButton = new Button (sprites.getSprite (CameraUi::PlayCameraStreamButtonSprite));
			playCameraStreamButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::playCameraStreamButtonClicked, this);
			playCameraStreamButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			playCameraStreamButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			playCameraStreamButton->setInverseColor (true);
			playCameraStreamButton->setMouseHoverTooltip (uitext->getText (UiTextString::CameraUiPlayCameraStreamTooltip), Widget::LeftAlignment);
			playCameraStreamButton->shortcutKey = SDLK_F2;
			toolbar->addRightItem (playCameraStreamButton);

			clearDisplayButton = new Button (sprites.getSprite (CameraUi::ClearDisplayButtonSprite));
			clearDisplayButton->mouseClickCallback = Widget::EventCallbackContext (CameraUi::clearDisplayButtonClicked, this);
			clearDisplayButton->mouseEnterCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseEntered, this);
			clearDisplayButton->mouseExitCallback = Widget::EventCallbackContext (CameraUi::commandButtonMouseExited, this);
			clearDisplayButton->setInverseColor (true);
			clearDisplayButton->setMouseHoverTooltip (uitext->getText (UiTextString::CameraUiClearDisplayTooltip), Widget::LeftAlignment);
			clearDisplayButton->shortcutKey = SDLK_F1;
			toolbar->addRightItem (clearDisplayButton);
			break;
		}
	}
}

void CameraUi::modeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	UiText *uitext;
	Menu *menu;

	ui = (CameraUi *) uiPtr;
	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraUi::modeButtonClicked)) {
		return;
	}

	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::ManageCameras).capitalized (), ui->sprites.getSprite (CameraUi::ConfigureTimelapseButtonSprite), CameraUi::cameraModeActionClicked, ui, 0, ui->toolbarMode == CameraUi::CameraMode);
	menu->addItem (uitext->getText (UiTextString::ViewCameraImages).capitalized (), ui->sprites.getSprite (CameraUi::ShowCameraImageButtonSprite), CameraUi::monitorModeActionClicked, ui, 0, ui->toolbarMode == CameraUi::MonitorMode);

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

static void doPause_appendPlaylistState (void *jsonListPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;

	window = CapturePlaylistWindow::castWidget (widgetPtr);
	if (window) {
		((JsonList *) jsonListPtr)->push_back (window->createState ());
	}
}
static void doPause_appendSelectedAgentId (void *stringListPtr, Widget *widgetPtr) {
	CameraWindow *camera;
	MonitorWindow *monitor;

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera && camera->isSelected) {
		((StringList *) stringListPtr)->push_back (camera->agentId);
		return;
	}
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor && monitor->isSelected) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
		return;
	}
}
static void doPause_appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
	CameraWindow *camera;
	MonitorWindow *monitor;

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera && camera->isExpanded) {
		((StringList *) stringListPtr)->push_back (camera->agentId);
		return;
	}
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor && monitor->isExpanded) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
		return;
	}
}
static void doPause_appendSelectedCaptureId (void *stringListPtr, Widget *widgetPtr) {
	CameraCaptureWindow *capture;

	capture = CameraCaptureWindow::castWidget (widgetPtr);
	if (capture && capture->isSelected) {
		((StringList *) stringListPtr)->push_back (capture->itemId);
	}
}
void CameraUi::doPause () {
	HashMap *prefs;
	JsonList playlists;
	StringList items;

	cardView->processItems (doPause_appendPlaylistState, &playlists);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::PlaylistsKey, &playlists);
	App::instance->unlockPrefs ();

	items.clear ();
	cardView->processItems (doPause_appendSelectedAgentId, &items);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::SelectedAgentsKey, &items);
	App::instance->unlockPrefs ();

	items.clear ();
	cardView->processItems (doPause_appendExpandedAgentId, &items);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::ExpandedAgentsKey, &items);
	App::instance->unlockPrefs ();

	items.clear ();
	cardView->processItems (doPause_appendSelectedCaptureId, &items);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::SelectedCapturesKey, &items);
	prefs->insert (CameraUi::ToolbarModeKey, toolbarMode);
	prefs->insert (CameraUi::AutoReloadKey, isAutoReloading);
	App::instance->unlockPrefs ();
}

void CameraUi::doResume () {
	HashMap *prefs;
	CapturePlaylistWindow *playlist;
	JsonList playlists;
	JsonList::iterator i, end;
	StdString name;
	int64_t now;

	App::instance->setNextBackgroundTexturePath ("ui/CameraUi/bg");

	if (! isFirstResumeComplete) {
		prefs = App::instance->lockPrefs ();
		prefs->find (CameraUi::PlaylistsKey, &playlists);
		App::instance->unlockPrefs ();

		now = OsUtil::getTime ();
		i = playlists.begin ();
		end = playlists.end ();
		while (i != end) {
			playlist = createCapturePlaylistWindow ();
			playlist->readState (*i);

			playlist->itemId.assign (cardView->getAvailableItemId ());
			if (selectedPlaylistId.empty ()) {
				if (playlist->isSelected) {
					selectedPlaylistId.assign (playlist->itemId);
				}
			}
			else {
				if (playlist->isSelected) {
					playlist->setSelected (false, true);
				}
			}
			if (playlist->isExpanded) {
				playlist->sortKey.sprintf ("%016llx%s", (long long int) now, playlist->playlistName.lowercased ().c_str ());
				cardView->addItem (playlist, playlist->itemId, CameraUi::ExpandedPlaylistRow, true);
			}
			else {
				playlist->sortKey.assign (playlist->playlistName.lowercased ());
				cardView->addItem (playlist, playlist->itemId, CameraUi::UnexpandedPlaylistRow, true);
			}
			playlist->animateNewCard ();
			++i;
		}
	}

	cardView->refresh ();
	resetExpandToggles ();
}

void CameraUi::doUpdate (int msElapsed) {
	std::map<StdString, CameraUi::AutoReloadInfo>::iterator i, end;
	AgentControl *agentcontrol;
	int64_t now, t;

	autoReloadToggle.compact ();
	expandAgentsToggle.compact ();
	expandPlaylistsToggle.compact ();
	emptyStateWindow.compact ();
	commandPopup.compact ();
	targetCapture.compact ();

	if (isAutoReloading) {
		agentcontrol = &(App::instance->agentControl);
		now = OsUtil::getTime ();
		i = autoReloadMap.begin ();
		end = autoReloadMap.end ();
		while (i != end) {
			if (i->second.isCapturing) {
				if (i->second.lastSendTime > i->second.lastReceiveTime) {
					t = i->second.lastSendTime;
				}
				else {
					t = i->second.lastReceiveTime;
				}
				if ((i->second.capturePeriod * 1000) < CameraUi::MinAutoReloadDelay) {
					t += CameraUi::MinAutoReloadDelay;
				}
				else {
					t += (i->second.capturePeriod * 1000);
				}

				if ((now >= t) && (! agentcontrol->isAgentInvoking (i->first))) {
					agentcontrol->refreshAgentStatus (i->first);
					i->second.lastSendTime = now;
				}
			}
			++i;
		}
	}
}

void CameraUi::handleLinkClientConnect (const StdString &agentId) {
	App::instance->agentControl.writeLinkCommand (App::instance->createCommand (SystemInterface::Command_WatchStatus, SystemInterface::Constant_Admin), agentId);
}

bool CameraUi::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	int mode;

	if (keycode == SDLK_TAB) {
		mode = toolbarMode + 1;
		if (mode >= CameraUi::ModeCount) {
			mode = 0;
		}
		setToolbarMode (mode);
		return (true);
	}

	return (false);
}

void CameraUi::doSyncRecordStore () {
	RecordStore *store;
	UiConfiguration *uiconfig;
	UiText *uitext;
	IconCardWindow *window;

	store = &(App::instance->agentControl.recordStore);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	cameraCount = 0;
	store->processAgentRecords ("cameraServerStatus", CameraUi::doSyncRecordStore_processCameraAgent, this);
	store->processAgentRecords ("monitorServerStatus", CameraUi::doSyncRecordStore_processMonitorAgent, this);

	window = (IconCardWindow *) emptyStateWindow.widget;
	if (cameraCount <= 0) {
		if (! window) {
			window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite), uitext->getText (UiTextString::CameraUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::CameraUiEmptyAgentStatusText));
			window->setLink (uitext->getText (UiTextString::LearnMore).capitalized (), App::getHelpUrl ("servers"));
			emptyStateWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, CameraUi::UnexpandedAgentRow);
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
	Label::truncateText (&text, UiConfiguration::CaptionFont, maxWidth, StdString::createSprintf ("... (%i)", count));

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
	Label::truncateText (&text, UiConfiguration::CaptionFont, maxWidth, StdString::createSprintf ("... (%i)", count));

	return (text);
}

StdString CameraUi::getSelectedCaptureNames (float maxWidth) {
	StdString text, id;
	HashMap::Iterator i;
	StringList names;
	int count;

	count = selectedCaptureMap.size ();
	if (count <= 0) {
		return (StdString (""));
	}

	i = selectedCaptureMap.begin ();
	while (selectedCaptureMap.hasNext (&i)) {
		id = selectedCaptureMap.next (&i);
		names.push_back (selectedCaptureMap.find (id, ""));
	}

	names.sort (StringList::compareCaseInsensitiveAscending);
	text.assign (names.join (", "));
	Label::truncateText (&text, UiConfiguration::CaptionFont, maxWidth, StdString::createSprintf ("... (%i)", count));

	return (text);
}

void CameraUi::doSyncRecordStore_processCameraAgent (void *uiPtr, Json *record, const StdString &recordId) {
	CameraUi *ui;
	SystemInterface *interface;
	HashMap *prefs;
	CameraWindow *camera;
	CameraCaptureWindow *capture;
	Json serverstatus;
	SystemInterface::Prefix prefix;
	std::map<StdString, CameraUi::AutoReloadInfo>::iterator info;
	StringList items;
	StdString cardid;
	int row, pos, captureid;

	ui = (CameraUi *) uiPtr;
	interface = &(App::instance->systemInterface);
	++(ui->cameraCount);
	if (! ui->cardView->contains (recordId)) {
		camera = new CameraWindow (recordId, &(ui->sprites));
		camera->selectStateChangeCallback = Widget::EventCallbackContext (CameraUi::cameraSelectStateChanged, ui);
		camera->expandStateChangeCallback = Widget::EventCallbackContext (CameraUi::cameraExpandStateChanged, ui);

		prefs = App::instance->lockPrefs ();
		prefs->find (CameraUi::SelectedAgentsKey, &items);
		if (items.contains (recordId)) {
			camera->setSelected (true, true);
			ui->selectedCameraMap.insert (recordId, interface->getCommandAgentName (record));
		}
		prefs->find (CameraUi::ExpandedAgentsKey, &items);
		App::instance->unlockPrefs ();

		pos = items.indexOf (recordId);
		if (pos < 0) {
			row = CameraUi::UnexpandedAgentRow;
			camera->sortKey.sprintf ("a%s", interface->getCommandAgentName (record).lowercased ().c_str ());
		}
		else {
			row = CameraUi::ExpandedAgentRow;
			camera->setExpanded (true, true);
			camera->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
		}

		ui->cardView->addItem (camera, recordId, row);
		camera->animateNewCard ();
		App::instance->agentControl.refreshAgentStatus (recordId);
	}

	captureid = 0;
	cardid.sprintf ("%s_%i", recordId.c_str (), captureid);
	if (interface->getCommandObjectParam (record, "cameraServerStatus", &serverstatus) && (serverstatus.getNumber ("lastCaptureTime", (int64_t) 0) > 0)) {
		if (! ui->cardView->contains (cardid)) {
			capture = new CameraCaptureWindow (record, captureid, &(ui->sprites));
			capture->selectStateChangeCallback = Widget::EventCallbackContext (CameraUi::captureSelectStateChanged, ui);
			capture->viewButtonClickCallback = Widget::EventCallbackContext (CameraUi::captureViewButtonClicked, ui);
			capture->itemId.assign (cardid);
			capture->sortKey.assign (capture->captureName);

			prefs = App::instance->lockPrefs ();
			prefs->find (CameraUi::SelectedCapturesKey, &items);
			if (items.contains (capture->itemId)) {
				capture->setSelected (true, true);
				ui->selectedCaptureMap.insert (capture->itemId, capture->captureName);
			}
			App::instance->unlockPrefs ();

			ui->cardView->addItem (capture, capture->itemId, CameraUi::CaptureRow);
			capture->animateNewCard ();
		}
	}
	else {
		ui->cardView->removeItem (cardid, true);
	}

	if (ui->isAutoReloading) {
		prefix = interface->getCommandPrefix (record);
		info = ui->getAutoReloadInfo (recordId);
		if (info->second.lastStatusCreateTime != prefix.createTime) {
			info->second.lastStatusCreateTime = prefix.createTime;
			info->second.lastReceiveTime = OsUtil::getTime ();
		}

		if (interface->getCommandObjectParam (record, "cameraServerStatus", &serverstatus)) {
			info->second.isCapturing = serverstatus.getBoolean ("isCapturing", false);
			info->second.capturePeriod = serverstatus.getNumber ("capturePeriod", (int) 0);
		}
	}
}

void CameraUi::doSyncRecordStore_processMonitorAgent (void *uiPtr, Json *record, const StdString &recordId) {
	CameraUi *ui;
	SystemInterface *interface;
	HashMap *prefs;
	MonitorWindow *monitor;
	StringList items;
	int pos, row;

	ui = (CameraUi *) uiPtr;
	interface = &(App::instance->systemInterface);
	if (! ui->cardView->contains (recordId)) {
		monitor = new MonitorWindow (recordId);
		monitor->selectStateChangeCallback = Widget::EventCallbackContext (CameraUi::monitorSelectStateChanged, ui);
		monitor->expandStateChangeCallback = Widget::EventCallbackContext (CameraUi::monitorExpandStateChanged, ui);
		monitor->screenshotLoadCallback = Widget::EventCallbackContext (CameraUi::monitorScreenshotLoaded, ui);
		monitor->setSelectEnabled (true);
		monitor->setScreenshotDisplayEnabled (true);

		prefs = App::instance->lockPrefs ();
		prefs->find (CameraUi::SelectedAgentsKey, &items);
		if (items.contains (recordId)) {
			monitor->setSelected (true, true);
			ui->selectedMonitorMap.insert (recordId, interface->getCommandAgentName (record));
		}
		prefs->find (CameraUi::ExpandedAgentsKey, &items);
		App::instance->unlockPrefs ();

		pos = items.indexOf (recordId);
		if (pos < 0) {
			row = CameraUi::UnexpandedAgentRow;
			monitor->sortKey.sprintf ("b%s", interface->getCommandAgentName (record).lowercased ().c_str ());
		}
		else {
			row = CameraUi::ExpandedAgentRow;
			monitor->setExpanded (true, true);
			monitor->sortKey.sprintf ("%016llx", (long long int) (items.size () - pos));
		}

		ui->cardView->addItem (monitor, recordId, row);
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
		ui->selectedCameraMap.insert (camera->agentId, camera->agentName);
	}
	else {
		ui->selectedCameraMap.remove (camera->agentId);
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
		ui->cardView->setItemRow (camera->agentId, CameraUi::ExpandedAgentRow);
	}
	else {
		camera->sortKey.sprintf ("a%s", camera->agentName.lowercased ().c_str ());
		ui->cardView->setItemRow (camera->agentId, CameraUi::UnexpandedAgentRow);
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
		ui->cardView->setItemRow (monitor->agentId, CameraUi::ExpandedAgentRow);
	}
	else {
		monitor->sortKey.sprintf ("b%s", monitor->agentName.lowercased ().c_str ());
		ui->cardView->setItemRow (monitor->agentId, CameraUi::UnexpandedAgentRow);
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
	UiConfiguration *uiconfig;
	UiText *uitext;
	Menu *menu;

	ui = (CameraUi *) uiPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraUi::imageSizeButtonClicked)) {
		return;
	}

	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::Small).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), CameraUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (uitext->getText (UiTextString::Large).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), CameraUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);

	ui->showActionPopup (menu, widgetPtr, CameraUi::imageSizeButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::BottomOfAlignment);
}

void CameraUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	HashMap *prefs;
	int detail;

	ui = (CameraUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraUi::CaptureRow, ui->cardDetail);
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
	ui->cardView->setRowDetail (CameraUi::CaptureRow, ui->cardDetail);
	prefs = App::instance->lockPrefs ();
	prefs->insert (CameraUi::ImageSizeKey, ui->cardDetail);
	App::instance->unlockPrefs ();
}

static void expandAgentsToggleStateChanged_appendAgentId (void *stringListPtr, Widget *widgetPtr) {
	CameraWindow *camera;
	MonitorWindow *monitor;

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera) {
		((StringList *) stringListPtr)->push_back (camera->agentId);
		return;
	}
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		((StringList *) stringListPtr)->push_back (monitor->agentId);
		return;
	}
}
void CameraUi::expandAgentsToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Toggle *toggle;
	CameraWindow *camera;
	MonitorWindow *monitor;
	Widget *item;
	StringList idlist;
	StringList::iterator i, end;
	int64_t now;

	ui = (CameraUi *) uiPtr;
	toggle = (Toggle *) ui->expandAgentsToggle.widget;
	if (! toggle) {
		return;
	}

	now = OsUtil::getTime ();
	ui->cardView->processItems (expandAgentsToggleStateChanged_appendAgentId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		item = ui->cardView->getItem (*i);
		camera = CameraWindow::castWidget (item);
		if (camera) {
			if (toggle->isChecked) {
				camera->setExpanded (false, true);
				camera->sortKey.sprintf ("a%s", camera->agentName.lowercased ().c_str ());
				ui->cardView->setItemRow (camera->agentId, CameraUi::UnexpandedAgentRow, true);
			}
			else {
				camera->setExpanded (true, true);
				camera->sortKey.sprintf ("%016llx%s", (long long int) now, camera->agentName.lowercased ().c_str ());
				ui->cardView->setItemRow (camera->agentId, CameraUi::ExpandedAgentRow, true);
			}
		}
		else {
			monitor = MonitorWindow::castWidget (item);
			if (monitor) {
				if (toggle->isChecked) {
					monitor->setExpanded (false, true);
					monitor->sortKey.sprintf ("b%s", monitor->agentName.lowercased ().c_str ());
					ui->cardView->setItemRow (monitor->agentId, CameraUi::UnexpandedAgentRow, true);
				}
				else {
					monitor->setExpanded (true, true);
					monitor->sortKey.sprintf ("%016llx%s", (long long int) now, monitor->agentName.lowercased ().c_str ());
					ui->cardView->setItemRow (monitor->agentId, CameraUi::ExpandedAgentRow, true);
				}
			}
		}
		++i;
	}
	ui->cardView->refresh ();
}

static void expandPlaylistsToggleStateChanged_appendPlaylistId (void *stringListPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *playlist;

	playlist = CapturePlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		((StringList *) stringListPtr)->push_back (playlist->itemId);
	}
}
void CameraUi::expandPlaylistsToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Toggle *toggle;
	CapturePlaylistWindow *playlist;
	StringList idlist;
	StringList::iterator i, end;
	int64_t now;

	ui = (CameraUi *) uiPtr;
	toggle = (Toggle *) ui->expandPlaylistsToggle.widget;
	if (! toggle) {
		return;
	}

	now = OsUtil::getTime ();
	ui->cardView->processItems (expandPlaylistsToggleStateChanged_appendPlaylistId, &idlist);
	i = idlist.begin ();
	end = idlist.end ();
	while (i != end) {
		playlist = CapturePlaylistWindow::castWidget (ui->cardView->getItem (*i));
		if (playlist) {
			playlist->setExpanded (! toggle->isChecked, true);
			if (playlist->isExpanded) {
				playlist->sortKey.sprintf ("%016llx%s", (long long int) now, playlist->playlistName.lowercased ().c_str ());
				ui->cardView->setItemRow (playlist->itemId, CameraUi::ExpandedPlaylistRow, true);
			}
			else {
				playlist->sortKey.assign (playlist->playlistName.lowercased ());
				ui->cardView->setItemRow (playlist->itemId, CameraUi::UnexpandedPlaylistRow, true);
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
	CameraUi *ui;
	CameraWindow *camera;
	MonitorWindow *monitor;
	std::map<StdString, CameraUi::AutoReloadInfo>::iterator info;

	ui = (CameraUi *) uiPtr;
	camera = CameraWindow::castWidget (widgetPtr);
	if (camera) {
		App::instance->agentControl.refreshAgentStatus (camera->agentId);
		if (ui->isAutoReloading) {
			info = ui->getAutoReloadInfo (camera->agentId);
			info->second.lastSendTime = OsUtil::getTime ();
		}
		return;
	}
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor) {
		App::instance->agentControl.refreshAgentStatus (monitor->agentId);
		return;
	}
}

static void autoReloadToggleStateChanged_processCaptures (void *uiPtr, Widget *widgetPtr) {
	CameraCaptureWindow *capture;

	capture = CameraCaptureWindow::castWidget (widgetPtr);
	if (capture) {
		capture->setSelectedTimestamp (-1);
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
	ui->autoReloadMap.clear ();
	if (ui->isAutoReloading) {
		ui->cardView->processItems (CameraUi::reloadAgent, ui);
		ui->cardView->processRowItems (CameraUi::CaptureRow, autoReloadToggleStateChanged_processCaptures, ui);
	}
}

void CameraUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Button *button;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	CapturePlaylistWindow *playlist;
	StdString text;
	Color color;

	ui = (CameraUi *) uiPtr;
	button = (Button *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	Button::mouseEntered (button, button);

	ui->commandPopup.destroyAndClear ();
	ui->commandPopupSource.assign (button);
	panel = (Panel *) App::instance->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha));
	panel->setBorder (true, Color (uiconfig->darkBackgroundColor.r, uiconfig->darkBackgroundColor.g, uiconfig->darkBackgroundColor.b, uiconfig->overlayWindowAlpha));

	label = NULL;
	if (button == ui->configureCameraButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::ConfigureCamera).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedCameraNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoCameraSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearTimelapseButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::ClearTimelapse).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedCameraNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoCameraSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->showCameraImageButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::ShowImage).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedCaptureNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoCaptureSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (CameraUi::TimelapseIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->showCapturePlaylistButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::RunPlaylist).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		playlist = CapturePlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
		if (! playlist) {
			text.assign (uitext->getText (UiTextString::NoPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else if (playlist->getItemCount () <= 0) {
			text.assign (uitext->getText (UiTextString::EmptyPlaylistSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		else {
			text.assign (playlist->playlistName);
			color.assign (uiconfig->primaryTextColor);
		}

		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->playCameraStreamButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::ShowLiveVideo).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedCameraNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoCameraSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearDisplayButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::Clear).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedMonitorNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::NoMonitorSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}

	if (label) {
		label->setPadding (0.0f, 0.0f);
	}
	panel->setLayout (Panel::VerticalRightJustifiedLayout);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	panel->position.assign (App::instance->windowWidth - panel->width - uiconfig->paddingSize, App::instance->windowHeight - App::instance->uiStack.bottomBarHeight - panel->height - uiconfig->marginSize);
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
	UiConfiguration *uiconfig;
	UiText *uitext;
	SystemInterface *interface;
	RecordStore *store;
	ActionWindow *action;
	Toggle *toggle;
	ComboBox *combobox;
	SliderWindow *slider;
	HashMap::Iterator i;
	StdString id;
	Json *agentstatus, serverstatus;
	int j, imagequality, count, captureperiod, flip, t;
	bool enabled;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCameraMap.empty ()) {
		return;
	}

	App::instance->uiStack.suspendMouseHover ();
	if (ui->clearActionPopup (widgetPtr, CameraUi::configureCameraButtonClicked)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	interface = &(App::instance->systemInterface);
	store = &(App::instance->agentControl.recordStore);

	enabled = true;
	imagequality = SystemInterface::Constant_DefaultImageProfile;
	captureperiod = CameraUi::DefaultCapturePeriodIndex;
	flip = SystemInterface::Constant_NoFlip;
	store->lock ();
	i = ui->selectedCameraMap.begin ();
	while (ui->selectedCameraMap.hasNext (&i)) {
		id = ui->selectedCameraMap.next (&i);
		agentstatus = store->findRecord (id, SystemInterface::CommandId_AgentStatus);
		if (agentstatus && interface->getCommandObjectParam (agentstatus, "cameraServerStatus", &serverstatus)) {
			enabled = serverstatus.getBoolean ("isCapturing", false);
			imagequality = serverstatus.getNumber ("imageProfile", SystemInterface::Constant_DefaultImageProfile);

			t = serverstatus.getNumber ("capturePeriod", CameraUi::CapturePeriods[CameraUi::DefaultCapturePeriodIndex]);
			count = sizeof (CameraUi::CapturePeriods) / sizeof (CameraUi::CapturePeriods[0]);
			for (j = 0; j < count; ++j) {
				if (t == CameraUi::CapturePeriods[j]) {
					captureperiod = j;
					break;
				}
			}

			flip = serverstatus.getNumber ("flip", SystemInterface::Constant_NoFlip);
			break;
		}
	}
	store->unlock ();

	action = new ActionWindow ();
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (uitext->getText (UiTextString::ConfigureCamera).capitalized ());
	action->optionChangeCallback = Widget::EventCallbackContext (CameraUi::configureCameraActionOptionChanged, ui);
	action->closeCallback = Widget::EventCallbackContext (CameraUi::configureCameraActionClosed, ui);

	toggle = new Toggle ();
	toggle->setChecked (enabled);
	action->addOption (uitext->getText (UiTextString::EnableTimelapse).capitalized (), toggle);

	count = sizeof (CameraUi::CapturePeriods) / sizeof (CameraUi::CapturePeriods[0]);
	slider = new SliderWindow (new Slider (0.0f, (float) count));
	slider->setPadding (0.0f, 0.0f);
	slider->setValueNameFunction (CameraUi::capturePeriodValueName);
	for (j = 0; j < count; ++j) {
		slider->addSnapValue ((float) j);
	}
	slider->setValue (captureperiod);
	action->addOption (uitext->getText (UiTextString::CapturePeriod).capitalized (), slider, uitext->getText (UiTextString::CapturePeriodDescription));

	combobox = new ComboBox ();
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_DefaultImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_DefaultImageProfile));
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_HighQualityImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_HighQualityImageProfile));
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_LowQualityImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_LowQualityImageProfile));
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_LowestQualityImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_LowestQualityImageProfile));
	combobox->setValueByItemData (StdString::createSprintf ("%i", imagequality));
	action->addOption (uitext->getText (UiTextString::CaptureQuality).capitalized (), combobox, uitext->getText (UiTextString::CaptureQualityDescription));

	combobox = new ComboBox ();
	combobox->addItem (uitext->getText (UiTextString::None).capitalized (), StdString::createSprintf ("%i", SystemInterface::Constant_NoFlip));
	combobox->addItem (uitext->getText (UiTextString::Horizontal).capitalized (), StdString::createSprintf ("%i", SystemInterface::Constant_HorizontalFlip));
	combobox->addItem (uitext->getText (UiTextString::Vertical).capitalized (), StdString::createSprintf ("%i", SystemInterface::Constant_VerticalFlip));
	combobox->addItem (StdString::createSprintf ("%s %s %s", uitext->getText (UiTextString::Horizontal).capitalized ().c_str (), uitext->getText (UiTextString::And).c_str (), uitext->getText (UiTextString::Vertical).c_str ()), StdString::createSprintf ("%i", SystemInterface::Constant_HorizontalAndVerticalFlip));
	combobox->setValueByItemData (StdString::createSprintf ("%i", flip));
	action->addOption (uitext->getText (UiTextString::ImageFlip).capitalized (), combobox);

	if (! enabled) {
		action->setOptionDisabled (uitext->getText (UiTextString::CapturePeriod).capitalized (), true);
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
		return (App::instance->uiText.getText (UiTextString::Continuous).capitalized ());
	}

	return (OsUtil::getDurationDisplayString (CameraUi::CapturePeriods[i] * 1000));
}

void CameraUi::configureCameraActionOptionChanged (void *uiPtr, Widget *widgetPtr) {
	ActionWindow *action;
	UiText *uitext;
	bool disabled;

	action = (ActionWindow *) widgetPtr;
	uitext = &(App::instance->uiText);
	disabled = (! action->getBooleanValue (uitext->getText (UiTextString::EnableTimelapse).capitalized (), false));
	action->setOptionDisabled (uitext->getText (UiTextString::CapturePeriod).capitalized (), disabled);
}

void CameraUi::configureCameraActionClosed (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	UiText *uitext;
	HashMap::Iterator i;
	StdString id;
	Json *params;
	int result, count, quality;

	action = (ActionWindow *) widgetPtr;
	ui = (CameraUi *) uiPtr;
	if ((! action->isConfirmed) || ui->selectedCameraMap.empty ()) {
		return;
	}

	uitext = &(App::instance->uiText);
	count = 0;
	i = ui->selectedCameraMap.begin ();
	while (ui->selectedCameraMap.hasNext (&i)) {
		id = ui->selectedCameraMap.next (&i);

		params = new Json ();
		params->set ("isCaptureEnabled", action->getBooleanValue (uitext->getText (UiTextString::EnableTimelapse).capitalized (), false));
		params->set ("capturePeriod", CameraUi::CapturePeriods[action->getNumberValue (uitext->getText (UiTextString::CapturePeriod).capitalized (), CameraUi::DefaultCapturePeriodIndex)]);
		params->set ("flip", action->getNumberValue (uitext->getText (UiTextString::ImageFlip).capitalized (), SystemInterface::Constant_NoFlip));

		quality = action->getNumberValue (uitext->getText (UiTextString::CaptureQuality).capitalized (), SystemInterface::Constant_DefaultImageProfile);
		params->set ("imageProfile", quality);
		switch (quality) {
			case SystemInterface::Constant_HighQualityImageProfile: {
				params->set ("streamProfile", SystemInterface::Constant_DefaultStreamProfile);
				break;
			}
			case SystemInterface::Constant_LowQualityImageProfile: {
				params->set ("streamProfile", SystemInterface::Constant_LowQualityStreamProfile);
				break;
			}
			case SystemInterface::Constant_LowestQualityImageProfile: {
				params->set ("streamProfile", SystemInterface::Constant_LowestQualityStreamProfile);
				break;
			}
			default: {
				params->set ("streamProfile", SystemInterface::Constant_DefaultStreamProfile);
				break;
			}
		}

		result = App::instance->agentControl.invokeCommand (id, App::instance->createCommand (SystemInterface::Command_ConfigureCamera, SystemInterface::Constant_Camera, params));
		if (result != Result::Success) {
			Log::debug ("Failed to invoke camera command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
		else {
			++count;
			App::instance->agentControl.refreshAgentStatus (id);
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::ConfigureCameraMessage));
	}
}

void CameraUi::clearTimelapseButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCameraMap.empty ()) {
		return;
	}
	if (ui->clearActionPopup (widgetPtr, CameraUi::clearTimelapseButtonClicked)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	action = new ActionWindow ();
	action->setInverseColor (true);
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setTitleText (uitext->getText (UiTextString::ClearTimelapse).capitalized ());
	action->setDescriptionText (uitext->getText (UiTextString::ClearTimelapseActionText));
	action->setConfirmTooltipText (uitext->getText (UiTextString::Clear).capitalized ());
	action->closeCallback = Widget::EventCallbackContext (CameraUi::clearTimelapseActionClosed, ui);

	ui->showActionPopup (action, widgetPtr, CameraUi::clearTimelapseButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void CameraUi::clearTimelapseActionClosed (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	HashMap::Iterator i;
	StdString id;
	int result, count;

	ui = (CameraUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if ((! action->isConfirmed) || ui->selectedCameraMap.empty ()) {
		return;
	}

	count = 0;
	i = ui->selectedCameraMap.begin ();
	while (ui->selectedCameraMap.hasNext (&i)) {
		id = ui->selectedCameraMap.next (&i);
		result = App::instance->agentControl.invokeCommand (id, App::instance->createCommand (SystemInterface::Command_ClearTimelapse, SystemInterface::Constant_Camera));

		if (result != Result::Success) {
			Log::debug ("Failed to invoke camera command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
		else {
			++count;
			App::instance->agentControl.refreshAgentStatus (id);
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::ClearCameraTimelapseMessage));
	}
}

void CameraUi::showCameraImageButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraCaptureWindow *capture;
	HashMap::Iterator c, m;
	StdString captureid, monitorid, hostname, authpath, authsecret;
	Json *params, *agenthost;
	int count, result;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCaptureMap.empty () || ui->selectedMonitorMap.empty ()) {
		return;
	}

	ui->clearPopupWidgets ();
	count = 0;
	c = ui->selectedCaptureMap.begin ();
	m = ui->selectedMonitorMap.begin ();
	while (ui->selectedMonitorMap.hasNext (&m)) {
		monitorid = ui->selectedMonitorMap.next (&m);
		if (! ui->selectedCaptureMap.hasNext (&c)) {
			c = ui->selectedCaptureMap.begin ();
		}
		captureid = ui->selectedCaptureMap.next (&c);

		capture = CameraCaptureWindow::castWidget (ui->cardView->getItem (captureid));
		if (capture) {
			hostname = App::instance->agentControl.getAgentHostAddress (capture->agentId);
			if (! hostname.empty ()) {
				params = new Json ();
				agenthost = new Json ();
				agenthost->set ("hostname", hostname);
				if (App::instance->agentControl.getAgentAuthorization (capture->agentId, &authpath, &authsecret)) {
					agenthost->set ("authorizePath", authpath);
					agenthost->set ("authorizeSecret", authsecret);
				}

				params->set ("host", agenthost);
				if (capture->selectedTime >= 0) {
					params->set ("imageTime", capture->selectedTime);
				}
				result = App::instance->agentControl.invokeCommand (monitorid, App::instance->createCommand (SystemInterface::Command_ShowCameraImage, SystemInterface::Constant_Monitor, params));
				if (result != Result::Success) {
					Log::debug ("Failed to invoke monitor command; err=%i agentId=\"%s\"", result, monitorid.c_str ());
				}
				else {
					++count;
					App::instance->agentControl.refreshAgentStatus (monitorid);
				}
			}
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::ShowCameraImageMessage));
	}
}

void CameraUi::showCapturePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CapturePlaylistWindow *playlist;
	StringList idlist;
	int count;

	ui = (CameraUi *) uiPtr;
	ui->selectedMonitorMap.getKeys (&idlist);
	if (ui->selectedPlaylistId.empty () || idlist.empty ()) {
		return;
	}
	playlist = CapturePlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
	if ((! playlist) || (playlist->getItemCount () <= 0)) {
		return;
	}

	ui->clearPopupWidgets ();
	count = App::instance->agentControl.invokeCommand (&idlist, playlist->createCommand ());
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::ShowCapturePlaylistMessage));
	}
}

void CameraUi::playCameraStreamButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraWindow *camera;
	HashMap::Iterator c, m;
	StdString cameraid, monitorid, hostname, authpath, authsecret;
	Json *params, *agenthost;
	int count, result;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCameraMap.empty () || ui->selectedMonitorMap.empty ()) {
		return;
	}

	count = 0;
	c = ui->selectedCameraMap.begin ();
	m = ui->selectedMonitorMap.begin ();
	while (ui->selectedMonitorMap.hasNext (&m)) {
		monitorid = ui->selectedMonitorMap.next (&m);
		if (! ui->selectedCameraMap.hasNext (&c)) {
			c = ui->selectedCameraMap.begin ();
		}
		cameraid = ui->selectedCameraMap.next (&c);

		camera = CameraWindow::castWidget (ui->cardView->getItem (cameraid));
		if (camera) {
			hostname = App::instance->agentControl.getAgentHostAddress (camera->agentId);
			if (! hostname.empty ()) {
				params = new Json ();
				agenthost = new Json ();
				agenthost->set ("hostname", hostname);
				if (App::instance->agentControl.getAgentAuthorization (camera->agentId, &authpath, &authsecret)) {
					agenthost->set ("authorizePath", authpath);
					agenthost->set ("authorizeSecret", authsecret);
				}
				params->set ("host", agenthost);

				result = App::instance->agentControl.invokeCommand (monitorid, App::instance->createCommand (SystemInterface::Command_PlayCameraStream, SystemInterface::Constant_Monitor, params));
				if (result != Result::Success) {
					Log::debug ("Failed to invoke monitor command; err=%i agentId=\"%s\"", result, monitorid.c_str ());
				}
				else {
					++count;
					App::instance->agentControl.refreshAgentStatus (monitorid);
				}
			}
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::PlayCameraStreamMessage));
	}
}

void CameraUi::clearDisplayButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	StringList idlist;
	int count;

	ui = (CameraUi *) uiPtr;
	ui->selectedMonitorMap.getKeys (&idlist);
	if (idlist.empty ()) {
		return;
	}

	ui->clearPopupWidgets ();
	count = App::instance->agentControl.invokeCommand (&idlist, App::instance->createCommand (SystemInterface::Command_ClearDisplay, SystemInterface::Constant_Monitor));
	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else {
		App::instance->agentControl.refreshAgentStatus (&idlist);
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InvokeClearDisplayMessage));
	}
}

void CameraUi::createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	StdString name;
	CapturePlaylistWindow *playlist;

	ui = (CameraUi *) uiPtr;
	name = ui->getAvailablePlaylistName ();
	playlist = ui->createCapturePlaylistWindow ();
	playlist->itemId.assign (ui->cardView->getAvailableItemId ());
	playlist->setPlaylistName (name);
	playlist->setExpanded (true);
	ui->cardView->addItem (playlist, playlist->itemId, CameraUi::ExpandedPlaylistRow);

	playlist->setSelected (true);
	playlist->animateNewCard ();
	ui->cardView->scrollToItem (playlist->itemId);
	ui->cardView->refresh ();
	ui->clearPopupWidgets ();
	ui->resetExpandToggles ();
	App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", App::instance->uiText.getText (UiTextString::CreatedPlaylist).capitalized ().c_str (), name.c_str ()));
}

void CameraUi::captureViewButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraCaptureWindow *capture;
	CameraTimelineUi *timelineui;

	ui = (CameraUi *) uiPtr;
	capture = CameraCaptureWindow::castWidget (widgetPtr);
	if (! capture) {
		return;
	}

	ui->targetCapture.assign (capture);
	timelineui = new CameraTimelineUi (capture->agentId, capture->agentName);
	timelineui->thumbnailClickCallback = Widget::EventCallbackContext (CameraUi::cameraTimelineThumbnailClicked, ui);
	App::instance->uiStack.pushUi (timelineui);
}

void CameraUi::captureSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraCaptureWindow *capture;

	ui = (CameraUi *) uiPtr;
	capture = (CameraCaptureWindow *) widgetPtr;
	if (capture->isSelected) {
		ui->selectedCaptureMap.insert (capture->itemId, capture->captureName);
	}
	else {
		ui->selectedCaptureMap.remove (capture->itemId);
	}
	ui->clearPopupWidgets ();
}

void CameraUi::cameraTimelineThumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraThumbnailWindow *thumbnail;
	CameraCaptureWindow *capture;

	ui = (CameraUi *) uiPtr;
	thumbnail = CameraThumbnailWindow::castWidget (widgetPtr);
	capture = CameraCaptureWindow::castWidget (ui->targetCapture.widget);
	if (thumbnail && capture) {
		capture->setSelectedTimestamp (thumbnail->thumbnailTimestamp);
	}

	App::instance->uiStack.popUi ();
}

static void resetExpandToggles_countExpandedAgents (void *intPtr, Widget *widgetPtr) {
	CameraWindow *camera;
	MonitorWindow *monitor;
	int *count;

	count = (int *) intPtr;
	if (! count) {
		return;
	}

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera && camera->isExpanded) {
		++(*count);
		return;
	}
	monitor = MonitorWindow::castWidget (widgetPtr);
	if (monitor && monitor->isExpanded) {
		++(*count);
		return;
	}
}
static void resetExpandToggles_countExpandedPlaylists (void *intPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *playlist;
	int *count;

	playlist = CapturePlaylistWindow::castWidget (widgetPtr);
	count = (int *) intPtr;
	if (playlist && count && playlist->isExpanded) {
		++(*count);
	}
}
void CameraUi::resetExpandToggles () {
	Toggle *toggle;
	int count;

	toggle = (Toggle *) expandAgentsToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (resetExpandToggles_countExpandedAgents, &count);
		toggle->setChecked ((count <= 0), true);
	}

	toggle = (Toggle *) expandPlaylistsToggle.widget;
	if (toggle) {
		count = 0;
		cardView->processItems (resetExpandToggles_countExpandedPlaylists, &count);
		toggle->setChecked ((count <= 0), true);
	}
}

static void getAvailablePlaylistName_matchName (void *stringPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *playlist;
	StdString *name;

	playlist = CapturePlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		name = (StdString *) stringPtr;
		if (name->lowercased ().equals (playlist->playlistName.lowercased ())) {
			name->assign ("");
		}
	}
}
StdString CameraUi::getAvailablePlaylistName (const StdString &baseName) {
	StdString base, name;
	int i;

	if (baseName.empty ()) {
		base.assign (App::instance->uiText.getText (UiTextString::TimelapseList).capitalized ());
	}
	else {
		base.assign (baseName);
	}
	name.assign (base);
	cardView->processItems (getAvailablePlaylistName_matchName, &name);
	if (name.empty ()) {
		i = 2;
		while (true) {
			name.sprintf ("%s %i", base.c_str (), i);
			cardView->processItems (getAvailablePlaylistName_matchName, &name);
			if (! name.empty ()) {
				break;
			}
			++i;
		}
	}

	return (name);
}

CapturePlaylistWindow *CameraUi::createCapturePlaylistWindow () {
	CapturePlaylistWindow *playlist;

	playlist = new CapturePlaylistWindow (&sprites);
	playlist->selectStateChangeCallback = Widget::EventCallbackContext (CameraUi::playlistSelectStateChanged, this);
	playlist->expandStateChangeCallback = Widget::EventCallbackContext (CameraUi::playlistExpandStateChanged, this);
	playlist->nameClickCallback = Widget::EventCallbackContext (CameraUi::playlistNameClicked, this);
	playlist->itemListChangeCallback = Widget::EventCallbackContext (CameraUi::playlistItemListChanged, this);
	playlist->addItemClickCallback = Widget::EventCallbackContext (CameraUi::playlistAddItemActionClicked, this);
	playlist->removeClickCallback = Widget::EventCallbackContext (CameraUi::playlistRemoveActionClicked, this);
	playlist->addItemMouseEnterCallback = Widget::EventCallbackContext (CameraUi::playlistAddItemMouseEntered, this);
	playlist->addItemMouseExitCallback = Widget::EventCallbackContext (CameraUi::playlistAddItemMouseExited, this);

	return (playlist);
}

void CameraUi::playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CapturePlaylistWindow *playlist, *item;

	ui = (CameraUi *) uiPtr;
	playlist = (CapturePlaylistWindow *) widgetPtr;
	if (playlist->isSelected) {
		if (! ui->selectedPlaylistId.equals (playlist->itemId)) {
			if (! ui->selectedPlaylistId.empty ()) {
				item = CapturePlaylistWindow::castWidget (ui->cardView->getItem (ui->selectedPlaylistId));
				if (item) {
					item->setSelected (false, true);
				}
			}
			ui->selectedPlaylistId.assign (playlist->itemId);
		}
	}
	else {
		ui->selectedPlaylistId.assign ("");
	}
	ui->clearPopupWidgets ();
}

void CameraUi::playlistExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CapturePlaylistWindow *playlist;

	ui = (CameraUi *) uiPtr;
	playlist = (CapturePlaylistWindow *) widgetPtr;

	if (playlist->isExpanded) {
		playlist->sortKey.sprintf ("%016llx%s", (long long int) OsUtil::getTime (), playlist->playlistName.lowercased ().c_str ());
		ui->cardView->setItemRow (playlist->itemId, CameraUi::ExpandedPlaylistRow, true);
	}
	else {
		playlist->sortKey.assign (playlist->playlistName.lowercased ());
		ui->cardView->setItemRow (playlist->itemId, CameraUi::UnexpandedPlaylistRow, true);
	}
	playlist->resetInputState ();
	playlist->animateNewCard ();
	ui->resetExpandToggles ();
	ui->clearPopupWidgets ();
	ui->cardView->refresh ();
}

void CameraUi::playlistNameClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	CapturePlaylistWindow *playlist;
	TextFieldWindow *textfield;

	ui = (CameraUi *) uiPtr;
	playlist = (CapturePlaylistWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	ui->clearPopupWidgets ();
	textfield = new TextFieldWindow (playlist->windowWidth, uitext->getText (UiTextString::EnterPlaylistNamePrompt));
	textfield->setValue (playlist->playlistName);
	textfield->valueEditCallback = Widget::EventCallbackContext (CameraUi::playlistNameEdited, ui);
	textfield->enterButtonClickCallback = Widget::EventCallbackContext (CameraUi::playlistNameEditEnterButtonClicked, ui);
	textfield->setFillBg (true, uiconfig->lightPrimaryColor);
	textfield->setButtonsEnabled (true, true, true, true);
	textfield->shouldSkipTextClearCallbacks = true;
	textfield->assignKeyFocus ();

	ui->showActionPopup (textfield, playlist, CameraUi::playlistNameClicked, playlist->getScreenRect (), Ui::LeftEdgeAlignment, Ui::TopEdgeAlignment);
}

void CameraUi::playlistNameEdited (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CapturePlaylistWindow *playlist;
	TextFieldWindow *textfield;

	ui = (CameraUi *) uiPtr;
	textfield = TextFieldWindow::castWidget (ui->actionWidget.widget);
	playlist = CapturePlaylistWindow::castWidget (ui->actionTarget.widget);
	if ((! textfield) || (! playlist)) {
		return;
	}

	playlist->setPlaylistName (ui->getAvailablePlaylistName (textfield->getValue ()));
	playlist->sortKey.assign (playlist->playlistName.lowercased ());
	ui->clearPopupWidgets ();
}

void CameraUi::playlistNameEditEnterButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;

	ui = (CameraUi *) uiPtr;
	ui->clearPopupWidgets ();
}

void CameraUi::playlistItemListChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;

	ui = (CameraUi *) uiPtr;
	ui->refresh ();
}

void CameraUi::playlistAddItemActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CapturePlaylistWindow *playlist;
	CameraCaptureWindow *capture;
	HashMap::Iterator i;
	StdString id, hostname, authpath, authsecret;
	int count;

	ui = (CameraUi *) uiPtr;
	playlist = (CapturePlaylistWindow *) widgetPtr;
	if (ui->selectedCaptureMap.empty ()) {
		return;
	}

	count = 0;
	i = ui->selectedCaptureMap.begin ();
	while (ui->selectedCaptureMap.hasNext (&i)) {
		id = ui->selectedCaptureMap.next (&i);
		capture = CameraCaptureWindow::castWidget (ui->cardView->getItem (id));
		if (capture) {
			hostname = App::instance->agentControl.getAgentHostAddress (capture->agentId);
			if (! hostname.empty ()) {
				App::instance->agentControl.getAgentAuthorization (capture->agentId, &authpath, &authsecret);
				playlist->addItem (hostname, authpath, authsecret, StdString (""), capture->captureId, capture->captureName);
				++count;
			}
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
	}
	else if (count == 1) {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s", playlist->playlistName.c_str (), App::instance->uiText.getText (UiTextString::AddedPlaylistItem).c_str ()));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s: %s (%i)", playlist->playlistName.c_str (), App::instance->uiText.getText (UiTextString::AddedPlaylistItems).c_str (), count));
	}
}

void CameraUi::playlistRemoveActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CapturePlaylistWindow *playlist;
	UiConfiguration *uiconfig;
	UiText *uitext;
	ActionWindow *action;

	ui = (CameraUi *) uiPtr;
	playlist = (CapturePlaylistWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	if (ui->clearActionPopup (widgetPtr, CameraUi::playlistRemoveActionClicked)) {
		return;
	}

	action = new ActionWindow ();
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (Label::getTruncatedText (playlist->playlistName, UiConfiguration::TitleFont, playlist->width * 0.34f, Label::DotTruncateSuffix));
	action->setDescriptionText (uitext->getText (UiTextString::RemovePlaylistDescription));
	action->closeCallback = Widget::EventCallbackContext (CameraUi::playlistRemoveActionClosed, ui);

	ui->showActionPopup (action, widgetPtr, CameraUi::playlistRemoveActionClicked, playlist->getRemoveButtonScreenRect (), Ui::LeftEdgeAlignment, Ui::TopOfAlignment);
}

void CameraUi::playlistRemoveActionClosed (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	CapturePlaylistWindow *playlist;

	ui = (CameraUi *) uiPtr;
	action = (ActionWindow *) widgetPtr;
	if (! action->isConfirmed) {
		return;
	}

	playlist = CapturePlaylistWindow::castWidget (ui->actionTarget.widget);
	if (playlist) {
		ui->cardView->removeItem (playlist->itemId);
		ui->resetExpandToggles ();
	}
}

void CameraUi::playlistAddItemMouseEntered (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CapturePlaylistWindow *playlist;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
	StdString text;
	Color color;

	ui = (CameraUi *) uiPtr;
	playlist = (CapturePlaylistWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	ui->commandPopup.destroyAndClear ();
	ui->commandPopupSource.assign (widgetPtr);
	panel = (Panel *) App::instance->rootPanel->addWidget (new Panel ());
	ui->commandPopup.assign (panel);
	panel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha));
	panel->setBorder (true, Color (uiconfig->darkBackgroundColor.r, uiconfig->darkBackgroundColor.g, uiconfig->darkBackgroundColor.b, uiconfig->overlayWindowAlpha));

	label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::AddTimelapses).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));
	label->setPadding (0.0f, 0.0f);
	text.assign (playlist->playlistName);
	Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.20f, Label::DotTruncateSuffix);
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallPlaylistIconSprite), text, UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setRightAligned (true);

	text.assign (ui->getSelectedCaptureNames (App::instance->rootPanel->width * 0.25f));
	color.assign (uiconfig->primaryTextColor);
	if (text.empty ()) {
		text.assign (uitext->getText (UiTextString::NoCaptureSelectedPrompt));
		color.assign (uiconfig->errorTextColor);
	}
	icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (ui->sprites.getSprite (CameraUi::TimelapseIconSprite), text, UiConfiguration::CaptionFont, color));
	icon->setFillBg (true, uiconfig->lightBackgroundColor);
	icon->setRightAligned (true);

	panel->setLayout (Panel::VerticalRightJustifiedLayout);
	ui->assignPopupPosition (panel, playlist->getAddItemButtonScreenRect (), Ui::RightOfAlignment, Ui::YCenteredAlignment);
}

void CameraUi::playlistAddItemMouseExited (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;

	ui = (CameraUi *) uiPtr;
	if (ui->commandPopupSource.widget == widgetPtr) {
		ui->commandPopup.destroyAndClear ();
		ui->commandPopupSource.clear ();
	}
}
