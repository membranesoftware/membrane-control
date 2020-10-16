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
#include <math.h>
#include "Result.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "OsUtil.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "Toggle.h"
#include "Json.h"
#include "ImageWindow.h"
#include "CameraUi.h"
#include "CameraCaptureWindow.h"

CameraCaptureWindow::CameraCaptureWindow (Json *agentStatus, int captureId, SpriteGroup *cameraUiSpriteGroup)
: Panel ()
, captureId (captureId)
, lastCaptureWidth (0)
, lastCaptureHeight (0)
, lastCaptureTime (0)
, displayCaptureWidth (0)
, displayCaptureHeight (0)
, displayCaptureTime (0)
, selectedTime (-1)
, isSelected (false)
, sprites (cameraUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, timeLabel (NULL)
, thumbnailImage (NULL)
, selectToggle (NULL)
, viewButton (NULL)
{
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Json serverstatus;

	classId = ClassId::CameraCaptureWindow;
	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	setFillBg (true, uiconfig->mediumBackgroundColor);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setCornerRadius (uiconfig->cornerRadius);
	agentId = interface->getCommandAgentId (agentStatus);
	agentName = interface->getCommandAgentName (agentStatus);
	captureName.assign (agentName);
	if (interface->getCommandObjectParam (agentStatus, "cameraServerStatus", &serverstatus)) {
		captureImagePath = serverstatus.getString ("captureImagePath", "");
		lastCaptureWidth = serverstatus.getNumber ("lastCaptureWidth", (int) 0);
		lastCaptureHeight = serverstatus.getNumber ("lastCaptureHeight", (int) 0);
		lastCaptureTime = serverstatus.getNumber ("lastCaptureTime", (int64_t) 0);
		displayCaptureWidth = lastCaptureWidth;
		displayCaptureHeight = lastCaptureHeight;
		displayCaptureTime = lastCaptureTime;
	}

	iconImage = (Image *) addWidget (new Image (sprites->getSprite (CameraUi::TimelapseIconSprite)));

	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor));
	nameLabel->isInputSuspended = true;

	timeLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	timeLabel->isInputSuspended = true;
	if (displayCaptureTime > 0) {
		timeLabel->setText (OsUtil::getTimestampDisplayString (displayCaptureTime));
	}

	thumbnailImage = (ImageWindow *) addWidget (new ImageWindow (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	thumbnailImage->mouseLongPressCallback = Widget::EventCallbackContext (CameraCaptureWindow::thumbnailImageLongPressed, this);
	thumbnailImage->loadCallback = Widget::EventCallbackContext (CameraCaptureWindow::thumbnailImageLoaded, this);
	thumbnailImage->setLoadSprite (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->stateChangeCallback = Widget::EventCallbackContext (CameraCaptureWindow::selectToggleStateChanged, this);
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::UnselectedToggleTooltip), uitext->getText (UiTextString::SelectedToggleTooltip));

	viewButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::ImageButtonSprite)));
	viewButton->mouseClickCallback = Widget::EventCallbackContext (CameraCaptureWindow::viewButtonClicked, this);
	viewButton->zLevel = 3;
	viewButton->isTextureTargetDrawEnabled = false;
	viewButton->setImageColor (uiconfig->flatButtonTextColor);
	viewButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	viewButton->setMouseHoverTooltip (uitext->getText (UiTextString::ViewTimelineImagesTooltip));
}

CameraCaptureWindow::~CameraCaptureWindow () {

}

StdString CameraCaptureWindow::toStringDetail () {
	return (StdString::createSprintf (" CameraCaptureWindow id=\"%s\"", agentId.c_str ()));
}

bool CameraCaptureWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::CameraCaptureWindow));
}

CameraCaptureWindow *CameraCaptureWindow::castWidget (Widget *widget) {
	return (CameraCaptureWindow::isWidgetType (widget) ? (CameraCaptureWindow *) widget : NULL);
}

void CameraCaptureWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
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

void CameraCaptureWindow::setSelectedTimestamp (int64_t timestamp) {
	UiConfiguration *uiconfig;
	Json *params;

	uiconfig = &(App::instance->uiConfig);
	if (selectedTime == timestamp) {
		return;
	}
	selectedTime = timestamp;
	if (selectedTime != displayCaptureTime) {
		displayCaptureTime = selectedTime;
		timeLabel->setText (OsUtil::getTimestampDisplayString (selectedTime));
		timeLabel->textColor.assign (uiconfig->primaryTextColor);
		timeLabel->textColor.translate (uiconfig->lightPrimaryTextColor, uiconfig->longColorTranslateDuration);
		if (! captureImagePath.empty ()) {
			thumbnailImage->setLoadSprite (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));
			params = new Json ();
			if (selectedTime >= 0) {
				params->set ("imageTime", selectedTime);
			}
			thumbnailImage->setImageUrl (App::instance->agentControl.getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, SystemInterface::Constant_Camera, params), captureImagePath));
		}
	}
}

void CameraCaptureWindow::setLayout (int layoutType, float maxPanelWidth) {
	UiConfiguration *uiconfig;
	float w, h;

	if ((layoutType == layout) || (displayCaptureWidth <= 0) || (maxPanelWidth < 1.0f)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	layout = layoutType;
	w = maxPanelWidth;
	h = displayCaptureHeight;
	h *= maxPanelWidth;
	h /= displayCaptureWidth;
	w = floorf (w);
	h = floorf (h);
	thumbnailImage->setWindowSize (w, h);
	if (hasCaptureImage () && thumbnailImage->isImageUrlEmpty ()) {
		thumbnailImage->setImageUrl (App::instance->agentControl.getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, SystemInterface::Constant_Camera), captureImagePath));
	}
	thumbnailImage->reload ();

	w = maxPanelWidth - selectToggle->width - iconImage->width - (uiconfig->marginSize * 2.0f);
	nameLabel->setText (Label::getTruncatedText (agentName, UiConfiguration::BodyFont, w, Label::DotTruncateSuffix));
	refreshLayout ();
}

void CameraCaptureWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, y0, x2, y2;

	uiconfig = &(App::instance->uiConfig);
	x0 = widthPadding;
	y0 = heightPadding;
	x = x0;
	y = y0;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	timeLabel->flowDown (x, &y, &x2, &y2);
	iconImage->centerVertical (y0, y2);
	y = y0;
	selectToggle->flowRight (&x, y, &x2, &y2);

	x = x0;
	y = y2 + uiconfig->marginSize;
	thumbnailImage->flowDown (x, &y, &x2, &y2);
	viewButton->flowDown (x, &y, &x2, &y2);

	resetSize ();

	x = width - widthPadding;
	selectToggle->flowLeft (&x);

	x = width - widthPadding;
	viewButton->flowLeft (&x);
}

void CameraCaptureWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	Json *agentstatus, serverstatus;
	int64_t t;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentId);
	if (! agentstatus) {
		return;
	}
	if (interface->getCommandObjectParam (agentstatus, "cameraServerStatus", &serverstatus)) {
		captureImagePath = serverstatus.getString ("captureImagePath", "");
		lastCaptureWidth = serverstatus.getNumber ("lastCaptureWidth", (int) 0);
		lastCaptureHeight = serverstatus.getNumber ("lastCaptureHeight", (int) 0);

		t = serverstatus.getNumber ("lastCaptureTime", (int64_t) 0);
		if (t > 0) {
			if ((selectedTime < 0) && (t != lastCaptureTime)) {
				displayCaptureWidth = lastCaptureWidth;
				displayCaptureHeight = lastCaptureHeight;
				displayCaptureTime = t;

				if (! captureImagePath.empty ()) {
					thumbnailImage->setLoadSprite (NULL);
					thumbnailImage->setImageUrl (App::instance->agentControl.getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, SystemInterface::Constant_Camera), captureImagePath));
					thumbnailImage->reload ();
				}
			}
		}
		lastCaptureTime = t;
	}
}

bool CameraCaptureWindow::hasCaptureImage () {
	return ((! captureImagePath.empty ()) && (displayCaptureWidth > 0) && (displayCaptureHeight > 0) && (displayCaptureTime > 0));
}

void CameraCaptureWindow::reloadCaptureImage () {
	if (hasCaptureImage ()) {
		thumbnailImage->reload ();
	}
}

void CameraCaptureWindow::viewButtonClicked (void *windowPtr, Widget *widgetPtr) {
	CameraCaptureWindow *window;

	window = (CameraCaptureWindow *) windowPtr;
	if (window->viewButtonClickCallback.callback) {
		window->viewButtonClickCallback.callback (window->viewButtonClickCallback.callbackData, window);
	}
}

void CameraCaptureWindow::thumbnailImageLongPressed (void *windowPtr, Widget *widgetPtr) {
	ImageWindow *image;

	image = (ImageWindow *) widgetPtr;
	App::instance->uiStack.showImageDialog (image->imageUrl);
}

void CameraCaptureWindow::thumbnailImageLoaded (void *windowPtr, Widget *widgetPtr) {
	CameraCaptureWindow *window;
	ImageWindow *image;
	UiConfiguration *uiconfig;
	StdString text;

	window = (CameraCaptureWindow *) windowPtr;
	image = (ImageWindow *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);
	window->displayCaptureWidth = image->imageLoadSourceWidth;
	window->displayCaptureHeight = image->imageLoadSourceHeight;

	text = OsUtil::getTimestampDisplayString (window->displayCaptureTime);
	if (! text.equals (window->timeLabel->text)) {
		window->timeLabel->setText (text);
		window->timeLabel->textColor.assign (uiconfig->primaryTextColor);
		window->timeLabel->textColor.translate (uiconfig->lightPrimaryTextColor, uiconfig->longColorTranslateDuration);
	}
}

void CameraCaptureWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	CameraCaptureWindow *window;
	Toggle *toggle;
	UiConfiguration *uiconfig;

	window = (CameraCaptureWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);

	window->isSelected = toggle->isChecked;
	if (window->isSelected) {
		window->setCornerRadius (0, uiconfig->cornerRadius, 0, uiconfig->cornerRadius);
	}
	else {
		window->setCornerRadius (uiconfig->cornerRadius);
	}
	if (window->selectStateChangeCallback.callback) {
		window->selectStateChangeCallback.callback (window->selectStateChangeCallback.callbackData, window);
	}
}
