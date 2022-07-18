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
#include "App.h"
#include "ClassId.h"
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
#include "Button.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "CameraUi.h"
#include "CameraWindow.h"

const int CameraWindow::MinAutoRefreshPeriod = 5000;
const float CameraWindow::ExpandedTextTruncateScale = 0.24f;
const float CameraWindow::UnexpandedTextTruncateScale = 0.10f;

CameraWindow::CameraWindow (const StdString &agentId, int sensor, SpriteGroup *cameraUiSpriteGroup)
: Panel ()
, isSelected (false)
, isExpanded (false)
, agentId (agentId)
, sensor (sensor)
, isCapturing (false)
, lastCaptureWidth (0)
, lastCaptureHeight (0)
, lastCaptureTime (0)
, displayCaptureWidth (0)
, displayCaptureHeight (0)
, displayCaptureTime (0)
, capturePeriod (-1)
, imageProfile (-1)
, flip (-1)
, lastAutoRefreshTime (0)
, autoRefreshCapture (false)
, autoRefreshPeriod (0)
, selectedTime (-1)
, sprites (cameraUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, dividerPanel (NULL)
, captureImage (NULL)
, emptyImagePanel (NULL)
, captureTimeIcon (NULL)
, statusIcon (NULL)
, storageIcon (NULL)
, imageQualityIcon (NULL)
, capturePeriodIcon (NULL)
, flipIcon (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, openButton (NULL)
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

	captureImage = (ImageWindow *) addWidget (new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	captureImage->mouseLongPressCallback = Widget::EventCallbackContext (CameraWindow::captureImageLongPressed, this);
	captureImage->loadCallback = Widget::EventCallbackContext (CameraWindow::captureImageLoaded, this);
	captureImage->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));
	captureImage->isVisible = false;

	emptyImagePanel = (Panel *) addWidget (new Panel ());
	emptyImagePanel->zLevel = -1;
	emptyImagePanel->setFillBg (true, Color (0.5f, 0.5f, 0.5f));
	emptyImagePanel->setBorder (true, Color (0.25f, 0.25f, 0.25f));
	emptyImagePanel->isVisible = false;

	captureTimeIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ClockIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	captureTimeIcon->setPadding (0.0f, 0.0f);
	captureTimeIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	captureTimeIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CaptureTime).capitalized ());
	captureTimeIcon->isVisible = false;

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

	openButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ImageButtonSprite)));
	openButton->mouseClickCallback = Widget::EventCallbackContext (CameraWindow::openButtonClicked, this);
	openButton->zLevel = 3;
	openButton->isTextureTargetDrawEnabled = false;
	openButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	openButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	openButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ViewTimelineImagesTooltip));
	openButton->isVisible = false;

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
	float x, y, x0, y0, x1, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	if (nameLabel->isVisible) {
		nameLabel->flowDown (x, &y, &x2, &y2);
	}
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

	switch (layout) {
		case CardView::LowDetail: {
			x = x0;
			if (captureImage->isVisible) {
				captureImage->flowDown (x, &y, &x2, &y2);
			}
			if (emptyImagePanel->isVisible) {
				emptyImagePanel->flowDown (x, &y, &x2, &y2);
			}
			if (captureTimeIcon->isVisible) {
				captureTimeIcon->flowRight (&x, y, &x2, &y2);
			}
			if (openButton->isVisible) {
				openButton->flowDown (x, &y, &x2, &y2);
			}
			break;
		}
		case CardView::MediumDetail: {
			x = x0;
			x1 = x0;
			if (captureImage->isVisible) {
				captureImage->flowRight (&x, y, &x2, &y2);
				x1 = captureImage->position.x + captureImage->width + UiConfiguration::instance->marginSize;
			}
			if (emptyImagePanel->isVisible) {
				emptyImagePanel->flowRight (&x, y, &x2, &y2);
				x1 = emptyImagePanel->position.x + emptyImagePanel->width + UiConfiguration::instance->marginSize;
			}

			if (statusIcon->isVisible) {
				statusIcon->flowDown (x, &y, &x2, &y2);
			}
			if (capturePeriodIcon->isVisible) {
				capturePeriodIcon->flowRight (&x, y, &x2, &y2);
			}
			if (flipIcon->isVisible) {
				flipIcon->flowDown (x, &y, &x2, &y2);
			}

			x = x1;
			if (imageQualityIcon->isVisible) {
				imageQualityIcon->flowDown (x, &y, &x2, &y2);
			}
			if (storageIcon->isVisible) {
				storageIcon->flowDown (x, &y, &x2, &y2);
			}
			if (captureTimeIcon->isVisible) {
				captureTimeIcon->flowDown (x, &y, &x2, &y2);
			}
			if (openButton->isVisible) {
				openButton->flowDown (x, &y, &x2, &y2);
			}
			break;
		}
		case CardView::HighDetail: {
			x = x0;
			if (captureImage->isVisible) {
				captureImage->flowDown (x, &y, &x2, &y2);
			}
			if (emptyImagePanel->isVisible) {
				emptyImagePanel->flowDown (x, &y, &x2, &y2);
			}

			x = x0;
			y = y2 + UiConfiguration::instance->marginSize;
			x2 = 0.0f;
			if (statusIcon->isVisible) {
				statusIcon->flowRight (&x, y, &x2, &y2);
			}
			if (captureTimeIcon->isVisible) {
				captureTimeIcon->flowDown (x, &y, &x2, &y2);
			}

			x = x0;
			y = y2 + UiConfiguration::instance->marginSize;
			x2 = 0.0f;
			if (capturePeriodIcon->isVisible) {
				capturePeriodIcon->flowRight (&x, y, &x2, &y2);
			}
			if (flipIcon->isVisible) {
				flipIcon->flowRight (&x, y, &x2, &y2);
			}
			if (imageQualityIcon->isVisible) {
				imageQualityIcon->flowRight (&x, y, &x2, &y2);
			}
			if (storageIcon->isVisible) {
				storageIcon->flowRight (&x, y, &x2, &y2);
			}
			if (openButton->isVisible) {
				openButton->flowDown (x, &y, &x2, &y2);
			}
			break;
		}
	}

	resetSize ();

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	}
	if (layout == CardView::LowDetail) {
		captureImage->centerHorizontal (0.0f, width);
		emptyImagePanel->centerHorizontal (0.0f, width);
		captureTimeIcon->centerVertical (captureTimeIcon->position.y, openButton->position.y + openButton->height);
	}

	x = width - widthPadding;
	selectToggle->flowLeft (&x);
	expandToggle->flowLeft (&x);

	if (openButton->isVisible) {
		x = width - widthPadding;
		openButton->flowLeft (&x);
	}
	if (layout == CardView::HighDetail) {
		if (captureTimeIcon->isVisible) {
			x = width - widthPadding;
			captureTimeIcon->flowLeft (&x);
		}
	}
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
	Json *record, serverstatus, sensorstatus, *params;
	StdString videomonitor, capturepath;

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
	agentName.assign (Agent::getCommandAgentName (record));
	descriptionLabel->setText (SystemInterface::instance->getCommandStringParam (record, "applicationName", ""));
	isCapturing = sensorstatus.getBoolean ("isCapturing", false);
	storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));
	imageProfile = sensorstatus.getNumber ("imageProfile", SystemInterface::Constant_DefaultImageProfile);
	imageQualityIcon->setText (CameraUi::getImageQualityDescription (imageProfile));

	capturePeriod = sensorstatus.getNumber ("capturePeriod", (int) 0);
	if (capturePeriod <= 0) {
		capturePeriodIcon->setText (UiText::instance->getText (UiTextString::Continuous).capitalized ());
	}
	else {
		capturePeriodIcon->setText (OsUtil::getDurationDisplayString (capturePeriod * 1000));
	}

	flip = sensorstatus.getNumber ("flip", SystemInterface::Constant_NoFlip);
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

	videomonitor = sensorstatus.getString ("videoMonitor", "");
	if (! videomonitor.empty ()) {
		statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ActiveStateIconSprite));
		statusIcon->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextString::ShowingLiveVideo).capitalized ().c_str (), videomonitor.c_str ()), ((float) App::instance->windowWidth) * CameraWindow::ExpandedTextTruncateScale, Font::DotTruncateSuffix));
	}
	else if (isCapturing) {
		statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ActiveStateIconSprite));
		statusIcon->setText (UiText::instance->getText (UiTextString::CapturingImages).capitalized ());
	}
	else {
		statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::InactiveStateIconSprite));
		statusIcon->setText (UiText::instance->getText (UiTextString::Inactive).capitalized ());
	}

	capturepath = serverstatus.getString ("captureImagePath", "");
	if ((! capturepath.empty ()) && (! captureImagePath.equals (capturepath))) {
		captureImagePath.assign (capturepath);

		params = (new Json ())->set ("sensor", sensor);
		if (selectedTime >= 0) {
			params->set ("imageTime", selectedTime);
		}
		captureImage->setImageUrl (AgentControl::instance->getAgentSecondaryUrl (agentId, captureImagePath, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, params)));
	}
	lastCaptureWidth = sensorstatus.getNumber ("lastCaptureWidth", (int) 0);
	lastCaptureHeight = sensorstatus.getNumber ("lastCaptureHeight", (int) 0);
	lastCaptureTime = sensorstatus.getNumber ("lastCaptureTime", (int64_t) 0);

	if (isCapturing && (lastCaptureTime <= 0)) {
		autoRefreshPeriod = CameraWindow::MinAutoRefreshPeriod;
	}
	else {
		autoRefreshPeriod = capturePeriod * 1000;
		if (autoRefreshPeriod < CameraWindow::MinAutoRefreshPeriod) {
			autoRefreshPeriod = CameraWindow::MinAutoRefreshPeriod;
		}
	}

	resetLayoutDisplay ();
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

void CameraWindow::setAutoRefresh (bool enable) {
	autoRefreshCapture = enable;
	lastAutoRefreshTime = 0;
}

void CameraWindow::setSelectedTime (int64_t timestamp) {
	Json *params;

	if (selectedTime == timestamp) {
		return;
	}
	selectedTime = timestamp;
	if (hasCaptureImage ()) {
		params = (new Json ())->set ("sensor", sensor);
		if (selectedTime >= 0) {
			params->set ("imageTime", selectedTime);
		}
		captureImage->setImageUrl (AgentControl::instance->getAgentSecondaryUrl (agentId, captureImagePath, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, params)));
		captureTimeIcon->setText (StdString (""));
	}
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
		dividerPanel->isVisible = true;
	}
	else {
		setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		dividerPanel->isVisible = false;
	}

	resetLayoutDisplay ();
	refreshLayout ();
	resetNameLabel ();
}

void CameraWindow::setLayout (int layoutType, float maxPanelWidth) {
	float srcw, srch, displayw, displayh, namew;

	if ((layoutType == layout) || (maxPanelWidth < 1.0f)) {
		return;
	}
	layout = layoutType;
	srcw = lastCaptureWidth;
	srch = lastCaptureHeight;
	if ((srcw <= 0.0f) || (srch <= 0.0f)) {
		srcw = maxPanelWidth;
		srch = srcw * 9.0f / 16.0f;
	}
	displayw = maxPanelWidth;
	displayh = srch;
	displayh *= maxPanelWidth;
	displayh /= srcw;
	displayw = floorf (displayw);
	displayh = floorf (displayh);
	displayCaptureWidth = (int) displayw;
	displayCaptureHeight = (int) displayh;
	captureImage->setWindowSize (true, displayw, displayh);
	captureImage->onLoadScale (displayw, displayh);
	if (isExpanded && hasCaptureImage ()) {
		captureImage->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite), displayCaptureWidth, displayCaptureHeight);
		captureImage->isVisible = true;
		captureImage->reload ();
	}
	emptyImagePanel->setFixedSize (true, displayw, displayh);

	namew = maxPanelWidth - selectToggle->width - iconImage->width - (UiConfiguration::instance->marginSize * 2.0f);
	nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::BodyFont]->truncatedText (agentName, namew, Font::DotTruncateSuffix));

	resetLayoutDisplay ();
	refreshLayout ();
}

void CameraWindow::resetLayoutDisplay () {
	if (! isExpanded) {
		captureTimeIcon->isVisible = false;
		statusIcon->isVisible = false;
		storageIcon->isVisible = false;
		flipIcon->isVisible = false;
		imageQualityIcon->isVisible = false;
		capturePeriodIcon->isVisible = false;
		captureImage->isVisible = false;
		emptyImagePanel->isVisible = false;
		openButton->isVisible = false;
		descriptionLabel->isVisible = false;
		nameLabel->isVisible = true;
		return;
	}
	switch (layout) {
		case CardView::LowDetail: {
			nameLabel->isVisible = false;
			descriptionLabel->isVisible = false;
			statusIcon->isVisible = false;
			storageIcon->isVisible = false;
			flipIcon->isVisible = false;
			imageQualityIcon->isVisible = false;
			capturePeriodIcon->isVisible = false;
			openButton->isVisible = true;
			captureTimeIcon->isVisible = hasCaptureImage ();
			break;
		}
		case CardView::MediumDetail: {
			nameLabel->isVisible = true;
			descriptionLabel->isVisible = true;
			statusIcon->isVisible = true;
			storageIcon->isVisible = true;
			flipIcon->isVisible = true;
			imageQualityIcon->isVisible = true;
			openButton->isVisible = true;
			capturePeriodIcon->isVisible = isCapturing;
			captureTimeIcon->isVisible = hasCaptureImage ();
			break;
		}
		case CardView::HighDetail: {
			nameLabel->isVisible = true;
			descriptionLabel->isVisible = true;
			statusIcon->isVisible = true;
			storageIcon->isVisible = true;
			flipIcon->isVisible = true;
			imageQualityIcon->isVisible = true;
			openButton->isVisible = true;
			capturePeriodIcon->isVisible = isCapturing;
			captureTimeIcon->isVisible = hasCaptureImage ();
			break;
		}
	}

	if ((displayCaptureWidth > 0) && (displayCaptureHeight > 0) && hasCaptureImage ()) {
		emptyImagePanel->isVisible = false;
		captureImage->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite), displayCaptureWidth, displayCaptureHeight);
		captureImage->isVisible = true;
	}
	else {
		captureImage->isVisible = false;
		emptyImagePanel->isVisible = true;
	}
}

void CameraWindow::doUpdate (int msElapsed) {
	int64_t now;

	Panel::doUpdate (msElapsed);

	if (selectedTime < 0) {
		if (isExpanded && (displayCaptureWidth > 0) && (displayCaptureHeight > 0) && hasCaptureImage ()) {
			if (displayCaptureTime != lastCaptureTime) {
				if (displayCaptureTime > 0) {
					captureImage->setLoadingSprite (NULL);
				}
				captureImage->reload ();
				displayCaptureTime = lastCaptureTime;
			}
		}
		if (autoRefreshCapture && isCapturing && (autoRefreshPeriod > 0)) {
			now = OsUtil::getTime ();
			if (lastAutoRefreshTime < (now - autoRefreshPeriod)) {
				lastAutoRefreshTime = now;
				AgentControl::instance->refreshAgentStatus (agentId);
			}
		}
	}
}

bool CameraWindow::hasCaptureImage () {
	return ((! captureImagePath.empty ()) && (lastCaptureWidth > 0) && (lastCaptureHeight > 0) && (lastCaptureTime > 0));
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

void CameraWindow::captureImageLongPressed (void *windowPtr, Widget *widgetPtr) {
	ImageWindow *image;

	image = (ImageWindow *) widgetPtr;
	App::instance->uiStack.showImageDialog (image->imageUrl);
}

void CameraWindow::captureImageLoaded (void *windowPtr, Widget *widgetPtr) {
	CameraWindow *window;

	window = (CameraWindow *) windowPtr;
	window->captureTimeIcon->setText (OsUtil::getTimestampDisplayString ((window->selectedTime >= 0) ? window->selectedTime : window->displayCaptureTime));
	window->refresh ();
}

void CameraWindow::openButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((CameraWindow *) windowPtr)->eventCallback (((CameraWindow *) windowPtr)->openButtonClickCallback);
}
