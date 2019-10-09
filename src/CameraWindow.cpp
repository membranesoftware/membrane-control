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
#include "Result.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "OsUtil.h"
#include "Widget.h"
#include "SystemInterface.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "CameraUi.h"
#include "CameraWindow.h"

CameraWindow::CameraWindow (const StdString &agentId, SpriteGroup *cameraUiSpriteGroup)
: Panel ()
, isSelected (false)
, isExpanded (false)
, agentId (agentId)
, isCapturing (false)
, sprites (cameraUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, statusIcon (NULL)
, storageIcon (NULL)
, imageQualityIcon (NULL)
, capturePeriodIcon (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, selectStateChangeCallback (NULL)
, selectStateChangeCallbackData (NULL)
, expandStateChangeCallback (NULL)
, expandStateChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::CameraWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
	setCornerRadius (uiconfig->cornerRadius);
  setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeCameraIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	statusIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::ActivityStateIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	statusIcon->setPadding (0.0f, 0.0f);
	statusIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	statusIcon->setMouseHoverTooltip (uitext->getText (UiTextString::cameraActivityIconTooltip));
	statusIcon->isVisible = false;

	storageIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	storageIcon->setPadding (0.0f, 0.0f);
	storageIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	storageIcon->setMouseHoverTooltip (uitext->getText (UiTextString::storageTooltip));
	storageIcon->isVisible = false;

	imageQualityIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (CameraUi::ImageQualityIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	imageQualityIcon->setPadding (0.0f, 0.0f);
	imageQualityIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	imageQualityIcon->setMouseHoverTooltip (uitext->getText (UiTextString::imageQuality).capitalized ());
	imageQualityIcon->isVisible = false;

	capturePeriodIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (CameraUi::CapturePeriodIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	capturePeriodIcon->setPadding (0.0f, 0.0f);
	capturePeriodIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	capturePeriodIcon->setMouseHoverTooltip (uitext->getText (UiTextString::capturePeriod).capitalized ());
	capturePeriodIcon->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (CameraWindow::selectToggleStateChanged, this);
	selectToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::unselectedToggleTooltip), uitext->getText (UiTextString::selectedToggleTooltip));
	selectToggle->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (CameraWindow::expandToggleStateChanged, this);
	expandToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::expand).capitalized (), uitext->getText (UiTextString::minimize).capitalized ());

	refreshLayout ();
}

CameraWindow::~CameraWindow () {

}

StdString CameraWindow::toStringDetail () {
	return (StdString::createSprintf (" CameraWindow agentId=\"%s\"", agentId.c_str ()));
}

bool CameraWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::CameraWindow));
}

CameraWindow *CameraWindow::castWidget (Widget *widget) {
	return (CameraWindow::isWidgetType (widget) ? (CameraWindow *) widget : NULL);
}

void CameraWindow::setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	selectStateChangeCallback = callback;
	selectStateChangeCallbackData = callbackData;
	selectToggle->isVisible = selectStateChangeCallback ? true : false;
}

void CameraWindow::setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	expandStateChangeCallback = callback;
	expandStateChangeCallbackData = callbackData;
}

void CameraWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, y0, x2, y2;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	descriptionLabel->flowRight (&x, y, &x2, &y2);

	x = x2 + uiconfig->marginSize;
	y = y0;
	expandToggle->flowRight (&x, y, &x2, &y2);
	if (selectToggle->isVisible) {
		selectToggle->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (statusIcon->isVisible) {
		statusIcon->flowRight (&x, y, &x2, &y2);
	}
	if (imageQualityIcon->isVisible) {
		imageQualityIcon->flowRight (&x, y, &x2, &y2);
	}
	if (capturePeriodIcon->isVisible) {
		capturePeriodIcon->flowRight (&x, y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (storageIcon->isVisible) {
		storageIcon->flowRight (&x, y, &x2, &y2);
	}

	resetSize ();

	x = width - widthPadding;
	if (selectToggle->isVisible) {
		selectToggle->flowLeft (&x);
	}
}

void CameraWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	UiText *uitext;
	Json *record, serverstatus;
	int captureperiod;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	uitext = &(App::instance->uiText);
	record = store->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}
	if (! interface->getCommandObjectParam (record, "cameraServerStatus", &serverstatus)) {
		return;
	}

	agentName.assign (interface->getCommandAgentName (record));
	nameLabel->setText (agentName);
	descriptionLabel->setText (interface->getCommandStringParam (record, "applicationName", ""));

	isCapturing = serverstatus.getBoolean ("isCapturing", false);
	storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));
	imageQualityIcon->setText (CameraUi::getImageQualityDescription (serverstatus.getNumber ("imageProfile", SystemInterface::Constant_DefaultImageProfile)));

	captureperiod = serverstatus.getNumber ("capturePeriod", (int) 0) * 1000;
	if (captureperiod <= 0) {
		capturePeriodIcon->setText (uitext->getText (UiTextString::continuous).capitalized ());
	}
	else {
		capturePeriodIcon->setText (OsUtil::getDurationDisplayString (captureperiod));
	}

	if (! isCapturing) {
		statusIcon->setIconImageFrame (UiConfiguration::InactiveStateIconFrame);
		statusIcon->setText (uitext->getText (UiTextString::inactive).capitalized ());
		imageQualityIcon->isVisible = false;
		capturePeriodIcon->isVisible = false;
	}
	else {
		statusIcon->setText (uitext->getText (UiTextString::active).capitalized ());
		statusIcon->setIconImageFrame (UiConfiguration::ActiveStateIconFrame);
		imageQualityIcon->isVisible = isExpanded;
		capturePeriodIcon->isVisible = isExpanded;
	}

	refreshLayout ();
	Panel::syncRecordStore ();
}

void CameraWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
	UiConfiguration *uiconfig;

	if (selected == isSelected) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isSelected = selected;
	selectToggle->setChecked (isSelected, shouldSkipStateChangeCallback);
	if (isSelected) {
		setCornerRadius (0, uiconfig->cornerRadius, 0, uiconfig->cornerRadius);
	}
	else {
		setCornerRadius (uiconfig->cornerRadius);
	}
	refreshLayout ();
}

void CameraWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	UiConfiguration *uiconfig;

	if (expanded == isExpanded) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
		expandToggle->setChecked (true, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
		descriptionLabel->isVisible = true;
		statusIcon->isVisible = true;
		storageIcon->isVisible = true;
		imageQualityIcon->isVisible = isCapturing;
		capturePeriodIcon->isVisible = isCapturing;
	}
	else {
		setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		statusIcon->isVisible = false;
		descriptionLabel->isVisible = false;
		storageIcon->isVisible = false;
		imageQualityIcon->isVisible = false;
		capturePeriodIcon->isVisible = false;
	}

	refreshLayout ();
}

void CameraWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	CameraWindow *window;
	Toggle *toggle;
	UiConfiguration *uiconfig;

	window = (CameraWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);

	window->isSelected = toggle->isChecked;
	if (window->isSelected) {
		window->setCornerRadius (0, uiconfig->cornerRadius, 0, uiconfig->cornerRadius);
	}
	else {
		window->setCornerRadius (uiconfig->cornerRadius);
	}
	if (window->selectStateChangeCallback) {
		window->selectStateChangeCallback (window->selectStateChangeCallbackData, window);
	}
}

void CameraWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	CameraWindow *window;
	Toggle *toggle;

	window = (CameraWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback) {
		window->expandStateChangeCallback (window->expandStateChangeCallbackData, window);
	}
}
