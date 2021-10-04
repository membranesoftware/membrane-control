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
#include "App.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "OsUtil.h"
#include "Widget.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Agent.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "CameraUi.h"
#include "CameraWindow.h"

const float CameraWindow::ExpandedTextTruncateScale = 0.24f;
const float CameraWindow::UnexpandedTextTruncateScale = 0.10f;

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
, dividerPanel (NULL)
, statusIcon (NULL)
, storageIcon (NULL)
, imageQualityIcon (NULL)
, capturePeriodIcon (NULL)
, flipIcon (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
{
	classId = ClassId::CameraWindow;

	setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeCameraIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	statusIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::InactiveStateIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	statusIcon->setPadding (0.0f, 0.0f);
	statusIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	statusIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CameraActivityIconTooltip));
	statusIcon->isVisible = false;

	storageIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	storageIcon->setPadding (0.0f, 0.0f);
	storageIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	storageIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::StorageTooltip));
	storageIcon->isVisible = false;

	imageQualityIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (CameraUi::ImageQualityIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	imageQualityIcon->setPadding (0.0f, 0.0f);
	imageQualityIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	imageQualityIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CaptureQuality).capitalized ());
	imageQualityIcon->isVisible = false;

	capturePeriodIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (CameraUi::CapturePeriodIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	capturePeriodIcon->setPadding (0.0f, 0.0f);
	capturePeriodIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	capturePeriodIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CapturePeriod).capitalized ());
	capturePeriodIcon->isVisible = false;

	flipIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (CameraUi::FlipIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	flipIcon->setPadding (0.0f, 0.0f);
	flipIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	flipIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ImageFlip).capitalized ());
	flipIcon->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->stateChangeCallback = Widget::EventCallbackContext (CameraWindow::selectToggleStateChanged, this);
	selectToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	selectToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::UnselectedToggleTooltip), UiText::instance->getText (UiTextString::SelectedToggleTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (CameraWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());

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

void CameraWindow::refreshLayout () {
	float x, y, x0, y0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	if (descriptionLabel->isVisible) {
		descriptionLabel->flowRight (&x, y, &x2, &y2);
	}

	x = x2 + UiConfiguration::instance->marginSize;
	y = y0;
	expandToggle->flowRight (&x, y, &x2, &y2);
	selectToggle->flowDown (x, &y, &x2, &y2);
	if (dividerPanel->isVisible) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
	x2 = 0.0f;
	if (statusIcon->isVisible) {
		statusIcon->flowDown (x, &y, &x2, &y2);
	}
	if (capturePeriodIcon->isVisible) {
		capturePeriodIcon->flowRight (&x, y, &x2, &y2);
	}
	if (imageQualityIcon->isVisible) {
		imageQualityIcon->flowRight (&x, y, &x2, &y2);
	}
	if (flipIcon->isVisible) {
		flipIcon->flowRight (&x, y, &x2, &y2);
	}
	if (storageIcon->isVisible) {
		storageIcon->flowRight (&x, y, &x2, &y2);
	}

	resetSize ();

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	}

	x = width - widthPadding;
	selectToggle->flowLeft (&x);
	expandToggle->flowLeft (&x);
}

void CameraWindow::resetNameLabel () {
	float w;

	w = App::instance->windowWidth;
	if (isExpanded) {
		w *= CameraWindow::ExpandedTextTruncateScale;
		nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::HeadlineFont]->truncatedText (agentName, w, Font::DotTruncateSuffix));
	}
	else {
		w *= CameraWindow::UnexpandedTextTruncateScale;
		nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::BodyFont]->truncatedText (agentName, w, Font::DotTruncateSuffix));
	}
	refreshLayout ();
}

void CameraWindow::syncRecordStore () {
	Json *record, serverstatus;
	StdString videomonitor;
	int captureperiod, flip;

	record = RecordStore::instance->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}
	if (! SystemInterface::instance->getCommandObjectParam (record, "cameraServerStatus", &serverstatus)) {
		return;
	}

	agentName.assign (Agent::getCommandAgentName (record));
	descriptionLabel->setText (SystemInterface::instance->getCommandStringParam (record, "applicationName", ""));

	isCapturing = serverstatus.getBoolean ("isCapturing", false);
	storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));
	imageQualityIcon->setText (CameraUi::getImageQualityDescription (serverstatus.getNumber ("imageProfile", SystemInterface::Constant_DefaultImageProfile)));

	captureperiod = serverstatus.getNumber ("capturePeriod", (int) 0) * 1000;
	if (captureperiod <= 0) {
		capturePeriodIcon->setText (UiText::instance->getText (UiTextString::Continuous).capitalized ());
	}
	else {
		capturePeriodIcon->setText (OsUtil::getDurationDisplayString (captureperiod));
	}

	flip = serverstatus.getNumber ("flip", SystemInterface::Constant_NoFlip);
	switch (flip) {
		case SystemInterface::Constant_HorizontalFlip: {
			flipIcon->setText (UiText::instance->getText (UiTextString::HorizontalAbbreviation));
			break;
		}
		case SystemInterface::Constant_VerticalFlip: {
			flipIcon->setText (UiText::instance->getText (UiTextString::VerticalAbbreviation));
			break;
		}
		case SystemInterface::Constant_HorizontalAndVerticalFlip: {
			flipIcon->setText (StdString::createSprintf ("%s/%s", UiText::instance->getText (UiTextString::HorizontalAbbreviation).c_str (), UiText::instance->getText (UiTextString::VerticalAbbreviation).c_str ()));
			break;
		}
		default: {
			flipIcon->setText (UiText::instance->getText (UiTextString::None).capitalized ());
			break;
		}
	}

	videomonitor = serverstatus.getString ("videoMonitor", "");
	if (! videomonitor.empty ()) {
		statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ActiveStateIconSprite));
		statusIcon->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextString::ShowingLiveVideo).capitalized ().c_str (), videomonitor.c_str ()), ((float) App::instance->windowWidth) * CameraWindow::ExpandedTextTruncateScale, Font::DotTruncateSuffix));
		capturePeriodIcon->isVisible = false;
	}
	else if (isCapturing) {
		statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ActiveStateIconSprite));
		statusIcon->setText (UiText::instance->getText (UiTextString::CapturingImages).capitalized ());
		capturePeriodIcon->isVisible = isExpanded;
	}
	else {
		statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::InactiveStateIconSprite));
		statusIcon->setText (UiText::instance->getText (UiTextString::Inactive).capitalized ());
		capturePeriodIcon->isVisible = false;
	}

	refreshLayout ();
	resetNameLabel ();
	Panel::syncRecordStore ();
}

void CameraWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
	if (selected == isSelected) {
		return;
	}
	isSelected = selected;
	selectToggle->setChecked (isSelected, shouldSkipStateChangeCallback);
	if (isSelected) {
		setCornerRadius (0, UiConfiguration::instance->cornerRadius, 0, UiConfiguration::instance->cornerRadius);
	}
	else {
		setCornerRadius (UiConfiguration::instance->cornerRadius);
	}
	refreshLayout ();
}

void CameraWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
		expandToggle->setChecked (true, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
		descriptionLabel->isVisible = true;
		dividerPanel->isVisible = true;
		statusIcon->isVisible = true;
		storageIcon->isVisible = true;
		flipIcon->isVisible = true;
		imageQualityIcon->isVisible = true;
		capturePeriodIcon->isVisible = isCapturing;
	}
	else {
		setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		statusIcon->isVisible = false;
		descriptionLabel->isVisible = false;
		dividerPanel->isVisible = false;
		storageIcon->isVisible = false;
		flipIcon->isVisible = false;
		imageQualityIcon->isVisible = false;
		capturePeriodIcon->isVisible = false;
	}

	refreshLayout ();
	resetNameLabel ();
}

void CameraWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	CameraWindow *window;
	Toggle *toggle;

	window = (CameraWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;

	window->isSelected = toggle->isChecked;
	if (window->isSelected) {
		window->setCornerRadius (0, UiConfiguration::instance->cornerRadius, 0, UiConfiguration::instance->cornerRadius);
	}
	else {
		window->setCornerRadius (UiConfiguration::instance->cornerRadius);
	}
	window->eventCallback (window->selectStateChangeCallback);
}

void CameraWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	CameraWindow *window;
	Toggle *toggle;

	window = (CameraWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	window->eventCallback (window->expandStateChangeCallback);
}
