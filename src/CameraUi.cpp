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
#include <map>
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
#include "IconLabelWindow.h"
#include "IconCardWindow.h"
#include "ActionWindow.h"
#include "CameraWindow.h"
#include "CameraCaptureWindow.h"
#include "CameraTimelineUi.h"
#include "CameraUi.h"

const int CameraUi::capturePeriods[] = { 0, 15, 60, 2 * 60, 3 * 60, 5 * 60, 10 * 60, 15 * 60, 30 * 60, 3600, 2 * 3600, 4 * 3600, 8 * 3600, 24 * 3600 };
const int CameraUi::defaultCapturePeriodIndex = 1;
const int CameraUi::minAutoReloadDelay = 5000; // ms

CameraUi::CameraUi ()
: Ui ()
, cardView (NULL)
, configureTimelapseButton (NULL)
, clearTimelapseButton (NULL)
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
	return (new Chip (App::instance->uiText.getText (UiTextString::cameras).capitalized (), sprites.getSprite (CameraUi::BreadcrumbIconSprite)));
}

void CameraUi::setHelpWindowContent (HelpWindow *helpWindow) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	helpWindow->setHelpText (uitext->getText (UiTextString::cameraUiHelpTitle), uitext->getText (UiTextString::cameraUiHelpText));
	if (cameraCount <= 0) {
		helpWindow->addAction (uitext->getText (UiTextString::cameraUiHelpAction1Text), uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
	}
}

StdString CameraUi::getImageQualityDescription (int imageQuality) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	if (imageQuality == SystemInterface::Constant_DefaultImageProfile) {
		return (uitext->getText (UiTextString::normalImageQualityDescription));
	}
	if (imageQuality == SystemInterface::Constant_HighQualityImageProfile) {
		return (uitext->getText (UiTextString::highImageQualityDescription));
	}
	if (imageQuality == SystemInterface::Constant_LowQualityImageProfile) {
		return (uitext->getText (UiTextString::lowImageQualityDescription));
	}
	if (imageQuality == SystemInterface::Constant_LowestQualityImageProfile) {
		return (uitext->getText (UiTextString::lowestImageQualityDescription));
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
	UiText *uitext;

	uitext = &(App::instance->uiText);
	cameraCount = 0;
	cardDetail = App::instance->prefsMap.find (App::CameraUiImageSizeKey, (int) CardView::MediumDetail);
	isAutoReloading = App::instance->prefsMap.find (App::CameraUiAutoReloadKey, false);

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setRowHeader (CameraUi::AgentRow, createRowHeaderPanel (uitext->getText (UiTextString::cameras).capitalized ()));
	cardView->setRowHeader (CameraUi::CaptureRow, createRowHeaderPanel (uitext->getText (UiTextString::timelapseImages).capitalized ()));
	cardView->setRowDetail (CameraUi::CaptureRow, cardDetail);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);

	return (Result::Success);
}

void CameraUi::doUnload () {
	autoReloadToggle.clear ();
	emptyStateWindow.clear ();
	selectedCameraMap.clear ();
	autoReloadMap.clear ();
}

void CameraUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::SelectImageSizeButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (CameraUi::imageSizeButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::thumbnailImageSizeTooltip));
	toolbar->addRightItem (button);

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::ReloadButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (CameraUi::reloadButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::reloadTooltip));
	toolbar->addRightItem (button);
}

void CameraUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	UiConfiguration *uiconfig;
	UiText *uitext;
	Button *button;
	Toggle *toggle;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	button = new Button (StdString (""), sprites.getSprite (CameraUi::ConfigureTimelapseButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (CameraUi::configureTimelapseButtonClicked, this);
	button->setMouseEnterCallback (CameraUi::commandButtonMouseEntered, this);
	button->setMouseExitCallback (CameraUi::commandButtonMouseExited, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::cameraUiConfigureTimelapseTooltip), Widget::LeftAlignment);
	toolbar->addRightItem (button);
	configureTimelapseButton = button;

	toggle = new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::VisibilityOffButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::VisibilityOnButtonSprite));
	toggle->setInverseColor (true);
	toggle->setStateChangeCallback (CameraUi::autoReloadToggleStateChanged, this);
	toggle->setMouseHoverTooltip (uitext->getText (UiTextString::cameraUiAutoReloadTooltip));
	toolbar->addRightItem (toggle);
	if (isAutoReloading) {
		toggle->setChecked (true, true);
	}
	autoReloadToggle.assign (toggle);

	button = new Button (StdString (""), sprites.getSprite (CameraUi::ClearTimelapseButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (CameraUi::clearTimelapseButtonClicked, this);
	button->setMouseEnterCallback (CameraUi::commandButtonMouseEntered, this);
	button->setMouseExitCallback (CameraUi::commandButtonMouseExited, this);
	button->setMouseHoverTooltip (uitext->getText (UiTextString::cameraUiClearTimelapseTooltip), Widget::LeftAlignment);
	toolbar->addLeftItem (button);
	clearTimelapseButton = button;
}

void CameraUi::doClearPopupWidgets () {
	commandPopup.destroyAndClear ();
	commandPopupSource.clear ();
}

void CameraUi::doRefresh () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void CameraUi::doPause () {
	StringList items;

	items.clear ();
	cardView->processItems (CameraUi::appendSelectedAgentId, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::CameraUiSelectedAgentsKey);
	}
	else {
		App::instance->prefsMap.insert (App::CameraUiSelectedAgentsKey, &items);
	}

	items.clear ();
	cardView->processItems (CameraUi::appendExpandedAgentId, &items);
	if (items.empty ()) {
		App::instance->prefsMap.remove (App::CameraUiExpandedAgentsKey);
	}
	else {
		App::instance->prefsMap.insert (App::CameraUiExpandedAgentsKey, &items);
	}

	App::instance->prefsMap.insert (App::CameraUiAutoReloadKey, isAutoReloading);
}

void CameraUi::appendSelectedAgentId (void *stringListPtr, Widget *widgetPtr) {
	CameraWindow *camera;

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera && camera->isSelected) {
		((StringList *) stringListPtr)->push_back (camera->agentId);
		return;
	}
}

void CameraUi::appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr) {
	CameraWindow *camera;

	camera = CameraWindow::castWidget (widgetPtr);
	if (camera && camera->isExpanded) {
		((StringList *) stringListPtr)->push_back (camera->agentId);
		return;
	}
}

void CameraUi::doResume () {
	App::instance->setNextBackgroundTexturePath ("ui/CameraUi/bg");
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
}

void CameraUi::doUpdate (int msElapsed) {
	std::map<StdString, CameraUi::AutoReloadInfo>::iterator i, end;
	AgentControl *agentcontrol;
	int64_t now, t;

	autoReloadToggle.compact ();
	emptyStateWindow.compact ();
	commandPopup.compact ();

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
				if ((i->second.capturePeriod * 1000) < CameraUi::minAutoReloadDelay) {
					t += CameraUi::minAutoReloadDelay;
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

void CameraUi::doResize () {
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
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
	store->processAgentRecords ("cameraServerStatus", CameraUi::processCameraAgent, this);

	window = (IconCardWindow *) emptyStateWindow.widget;
	if (cameraCount <= 0) {
		if (! window) {
			window = new IconCardWindow (uiconfig->coreSprites.getSprite (UiConfiguration::ServerIconSprite), uitext->getText (UiTextString::cameraUiEmptyAgentStatusTitle), StdString (""), uitext->getText (UiTextString::cameraUiEmptyAgentStatusText));
			window->setLink (uitext->getText (UiTextString::learnMore).capitalized (), App::getHelpUrl ("servers"));
			emptyStateWindow.assign (window);

			window->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (window, window->itemId, CameraUi::AgentRow);
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

void CameraUi::processCameraAgent (void *uiPtr, Json *record, const StdString &recordId) {
	CameraUi *ui;
	SystemInterface *interface;
	CameraWindow *camera;
	CameraCaptureWindow *capture;
	Json serverstatus;
	SystemInterface::Prefix prefix;
	std::map<StdString, CameraUi::AutoReloadInfo>::iterator info;
	StringList items;
	StdString id;

	ui = (CameraUi *) uiPtr;
	interface = &(App::instance->systemInterface);
	++(ui->cameraCount);
	if (! ui->cardView->contains (recordId)) {
		camera = new CameraWindow (recordId, &(ui->sprites));
		camera->setSelectStateChangeCallback (CameraUi::cameraSelectStateChanged, ui);
		camera->setExpandStateChangeCallback (CameraUi::cardExpandStateChanged, ui);
		camera->sortKey.sprintf ("a%s", App::instance->systemInterface.getCommandAgentName (record).lowercased ().c_str ());

		App::instance->prefsMap.find (App::CameraUiSelectedAgentsKey, &items);
		if (items.contains (recordId)) {
			camera->setSelected (true, true);
			ui->selectedCameraMap.insert (recordId, App::instance->systemInterface.getCommandAgentName (record));
		}

		App::instance->prefsMap.find (App::CameraUiExpandedAgentsKey, &items);
		if (items.contains (recordId)) {
			camera->setExpanded (true, true);
		}

		ui->cardView->addItem (camera, recordId, CameraUi::AgentRow);
		camera->animateNewCard ();
		App::instance->agentControl.refreshAgentStatus (recordId);
	}

	id.sprintf ("c_%s", recordId.c_str ());
	if (interface->getCommandObjectParam (record, "cameraServerStatus", &serverstatus) && (serverstatus.getNumber ("lastCaptureTime", (int64_t) 0) > 0)) {
		if (! ui->cardView->contains (id)) {
			capture = new CameraCaptureWindow (record);
			capture->setViewButtonClickCallback (CameraUi::captureViewButtonClicked, ui);
			capture->sortKey.assign (id);

			ui->cardView->addItem (capture, id, CameraUi::CaptureRow);
			capture->animateNewCard ();
		}
	}
	else {
		ui->cardView->removeItem (id, true);
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

void CameraUi::cardExpandStateChanged (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;

	ui = (CameraUi *) uiPtr;
	ui->cardView->refresh ();
}

void CameraUi::imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Menu *menu;
	bool show;

	ui = (CameraUi *) uiPtr;
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
	menu->addItem (uitext->getText (UiTextString::small).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SmallSizeButtonSprite), CameraUi::smallImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::LowDetail);
	menu->addItem (uitext->getText (UiTextString::medium).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::MediumSizeButtonSprite), CameraUi::mediumImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::MediumDetail);
	menu->addItem (uitext->getText (UiTextString::large).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::LargeSizeButtonSprite), CameraUi::largeImageSizeActionClicked, ui, 0, ui->cardDetail == CardView::HighDetail);
	menu->position.assign (widgetPtr->screenX + widgetPtr->width - menu->width, widgetPtr->screenY + widgetPtr->height);
	ui->actionWidget.assign (menu);
	ui->actionTarget.assign (widgetPtr);
}

void CameraUi::smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	int detail;

	ui = (CameraUi *) uiPtr;
	detail = CardView::LowDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraUi::CaptureRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::CameraUiImageSizeKey, ui->cardDetail);
}

void CameraUi::mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	int detail;

	ui = (CameraUi *) uiPtr;
	detail = CardView::MediumDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraUi::CaptureRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::CameraUiImageSizeKey, ui->cardDetail);
}

void CameraUi::largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	int detail;

	ui = (CameraUi *) uiPtr;
	detail = CardView::HighDetail;
	if (detail == ui->cardDetail) {
		return;
	}
	ui->cardDetail = detail;
	ui->cardView->setRowDetail (CameraUi::CaptureRow, ui->cardDetail);
	App::instance->prefsMap.insert (App::CameraUiImageSizeKey, ui->cardDetail);
}

void CameraUi::reloadButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;

	ui = (CameraUi *) uiPtr;
	ui->cardView->processRowItems (CameraUi::AgentRow, CameraUi::reloadAgent, ui);
}

void CameraUi::reloadAgent (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	CameraWindow *camera;
	std::map<StdString, CameraUi::AutoReloadInfo>::iterator info;

	ui = (CameraUi *) uiPtr;
	camera = CameraWindow::castWidget (widgetPtr);
	if (camera) {
		App::instance->agentControl.refreshAgentStatus (camera->agentId);
		if (ui->isAutoReloading) {
			info = ui->getAutoReloadInfo (camera->agentId);
			info->second.lastSendTime = OsUtil::getTime ();
		}
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
		ui->cardView->processRowItems (CameraUi::AgentRow, CameraUi::reloadAgent, ui);
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
}

void CameraUi::commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	Button *button;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Panel *panel;
	LabelWindow *label;
	IconLabelWindow *icon;
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
	if (button == ui->configureTimelapseButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::setTimelapse).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedCameraNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::cameraUiNoCameraSelectedPrompt));
			color.assign (uiconfig->errorTextColor);
		}
		icon = (IconLabelWindow *) panel->addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), text, UiConfiguration::CaptionFont, color));
		icon->setFillBg (true, uiconfig->lightBackgroundColor);
		icon->setRightAligned (true);
	}
	else if (button == ui->clearTimelapseButton) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::clearTimelapse).capitalized (), UiConfiguration::TitleFont, uiconfig->inverseTextColor)));

		text.assign (ui->getSelectedCameraNames (App::instance->rootPanel->width * 0.25f));
		color.assign (uiconfig->primaryTextColor);
		if (text.empty ()) {
			text.assign (uitext->getText (UiTextString::cameraUiNoCameraSelectedPrompt));
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

void CameraUi::configureTimelapseButtonClicked (void *uiPtr, Widget *widgetPtr) {
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
	int j, imagequality, count, captureperiod, t;
	bool show, enabled;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCameraMap.empty ()) {
		return;
	}

	App::instance->uiStack.suspendMouseHover ();
	show = true;
	if (ActionWindow::isWidgetType (ui->actionWidget.widget) && (ui->actionTarget.widget == widgetPtr)) {
		show = false;
	}

	ui->clearPopupWidgets ();
	if (! show) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	interface = &(App::instance->systemInterface);
	store = &(App::instance->agentControl.recordStore);

	enabled = true;
	imagequality = SystemInterface::Constant_DefaultImageProfile;
	captureperiod = CameraUi::defaultCapturePeriodIndex;
	store->lock ();
	i = ui->selectedCameraMap.begin ();
	while (ui->selectedCameraMap.hasNext (&i)) {
		id = ui->selectedCameraMap.next (&i);
		agentstatus = store->findRecord (id, SystemInterface::CommandId_AgentStatus);
		if (agentstatus && interface->getCommandObjectParam (agentstatus, "cameraServerStatus", &serverstatus)) {
			enabled = serverstatus.getBoolean ("isCapturing", false);
			if (enabled) {
				imagequality = serverstatus.getNumber ("imageProfile", SystemInterface::Constant_DefaultImageProfile);
				t = serverstatus.getNumber ("capturePeriod", CameraUi::capturePeriods[CameraUi::defaultCapturePeriodIndex]);
				count = sizeof (CameraUi::capturePeriods) / sizeof (CameraUi::capturePeriods[0]);
				for (j = 0; j < count; ++j) {
					if (t == CameraUi::capturePeriods[j]) {
						captureperiod = j;
						break;
					}
				}
			}
			break;
		}
	}
	store->unlock ();

	action = (ActionWindow *) App::instance->rootPanel->addWidget (new ActionWindow ());
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	action->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (uitext->getText (UiTextString::setTimelapse).capitalized ());
	action->setOptionChangeCallback (CameraUi::configureTimelapseActionOptionChanged, ui);
	action->setCloseCallback (CameraUi::configureTimelapseActionClosed, ui);

	toggle = new Toggle ();
	toggle->setChecked (enabled);
	action->addOption (uitext->getText (UiTextString::enableCapture).capitalized (), toggle, uitext->getText (UiTextString::enableCaptureDescription));

	combobox = new ComboBox ();
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_DefaultImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_DefaultImageProfile));
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_HighQualityImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_HighQualityImageProfile));
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_LowQualityImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_LowQualityImageProfile));
	combobox->addItem (getImageQualityDescription (SystemInterface::Constant_LowestQualityImageProfile), StdString::createSprintf ("%i", SystemInterface::Constant_LowestQualityImageProfile));
	combobox->setValueByItemData (StdString::createSprintf ("%i", imagequality));
	action->addOption (uitext->getText (UiTextString::imageQuality).capitalized (), combobox, uitext->getText (UiTextString::imageQualityDescription));

	count = sizeof (CameraUi::capturePeriods) / sizeof (CameraUi::capturePeriods[0]);
	slider = new SliderWindow (new Slider (0.0f, (float) count));
	slider->setPadding (0.0f, 0.0f);
	slider->setValueNameFunction (CameraUi::capturePeriodValueName);
	for (j = 0; j < count; ++j) {
		slider->addSnapValue ((float) j);
	}
	slider->setValue (captureperiod);
	action->addOption (uitext->getText (UiTextString::capturePeriod).capitalized (), slider, uitext->getText (UiTextString::capturePeriodDescription));

	if (! enabled) {
		action->setOptionDisabled (uitext->getText (UiTextString::imageQuality).capitalized (), true);
		action->setOptionDisabled (uitext->getText (UiTextString::capturePeriod).capitalized (), true);
	}

	action->position.assign (widgetPtr->screenX + widgetPtr->width - action->width, widgetPtr->screenY - action->height);
	ui->actionWidget.assign (action);
	ui->actionTarget.assign (widgetPtr);
}

StdString CameraUi::capturePeriodValueName (float sliderValue) {
	int i, count;

	i = (int) sliderValue;
	count = sizeof (CameraUi::capturePeriods) / sizeof (CameraUi::capturePeriods[0]);
	if ((i < 0) || (i >= count)) {
		return (StdString (""));
	}

	if (CameraUi::capturePeriods[i] == 0) {
		return (App::instance->uiText.getText (UiTextString::continuous).capitalized ());
	}

	return (OsUtil::getDurationDisplayString (CameraUi::capturePeriods[i] * 1000));
}

void CameraUi::configureTimelapseActionOptionChanged (void *uiPtr, Widget *widgetPtr) {
	ActionWindow *action;
	UiText *uitext;
	bool disabled;

	action = (ActionWindow *) widgetPtr;
	uitext = &(App::instance->uiText);
	disabled = (! action->getBooleanValue (uitext->getText (UiTextString::enableCapture).capitalized (), false));
	action->setOptionDisabled (uitext->getText (UiTextString::imageQuality).capitalized (), disabled);
	action->setOptionDisabled (uitext->getText (UiTextString::capturePeriod).capitalized (), disabled);
}

void CameraUi::configureTimelapseActionClosed (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	ActionWindow *action;
	UiText *uitext;
	HashMap::Iterator i;
	StdString id;
	Json *params;
	bool enabled;
	int result, count;

	action = (ActionWindow *) widgetPtr;
	ui = (CameraUi *) uiPtr;
	if ((! action->isConfirmed) || ui->selectedCameraMap.empty ()) {
		return;
	}

	uitext = &(App::instance->uiText);
	enabled = action->getBooleanValue (uitext->getText (UiTextString::enableCapture).capitalized (), false);
	count = 0;
	i = ui->selectedCameraMap.begin ();
	while (ui->selectedCameraMap.hasNext (&i)) {
		id = ui->selectedCameraMap.next (&i);
		if (! enabled) {
			result = App::instance->agentControl.invokeCommand (id, App::instance->createCommand (SystemInterface::Command_StopCapture, SystemInterface::Constant_Camera));
		}
		else {
			params = new Json ();
			params->set ("capturePeriod", CameraUi::capturePeriods[action->getNumberValue (uitext->getText (UiTextString::capturePeriod).capitalized (), CameraUi::defaultCapturePeriodIndex)]);
			params->set ("imageProfile", action->getNumberValue (uitext->getText (UiTextString::imageQuality).capitalized (), SystemInterface::Constant_DefaultImageProfile));
			result = App::instance->agentControl.invokeCommand (id, App::instance->createCommand (SystemInterface::Command_CreateTimelapseCaptureIntent, SystemInterface::Constant_Camera, params));
		}

		if (result !=  Result::Success) {
			Log::debug ("Failed to invoke camera command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
		else {
			++count;
			App::instance->agentControl.refreshAgentStatus (id);
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (enabled ? UiTextString::configureCameraTimelapseMessage : UiTextString::disableCameraTimelapseMessage));
	}
}

void CameraUi::clearTimelapseButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraUi *ui;
	HashMap::Iterator i;
	StdString id;
	int result, count;

	ui = (CameraUi *) uiPtr;
	if (ui->selectedCameraMap.empty ()) {
		return;
	}

	count = 0;
	i = ui->selectedCameraMap.begin ();
	while (ui->selectedCameraMap.hasNext (&i)) {
		id = ui->selectedCameraMap.next (&i);
		result = App::instance->agentControl.invokeCommand (id, App::instance->createCommand (SystemInterface::Command_ClearTimelapse, SystemInterface::Constant_Camera));

		if (result !=  Result::Success) {
			Log::debug ("Failed to invoke camera command; err=%i agentId=\"%s\"", result, id.c_str ());
		}
		else {
			++count;
			App::instance->agentControl.refreshAgentStatus (id);
		}
	}

	if (count <= 0) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
	}
	else {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::clearCameraTimelapseMessage));
	}
}

void CameraUi::captureViewButtonClicked (void *uiPtr, Widget *widgetPtr) {
	CameraCaptureWindow *capture;
	CameraTimelineUi *timelineui;

	capture = CameraCaptureWindow::castWidget (widgetPtr);
	if (! capture) {
		return;
	}

	timelineui = new CameraTimelineUi (capture->agentId, capture->agentName);
	App::instance->uiStack.pushUi (timelineui);
}
