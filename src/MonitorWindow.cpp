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
#include "UiText.h"
#include "OsUtil.h"
#include "Widget.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Button.h"
#include "Toggle.h"
#include "IconLabelWindow.h"
#include "MonitorWindow.h"

const float MonitorWindow::screenshotImageScale = 0.27f;

MonitorWindow::MonitorWindow (const StdString &agentId)
: Panel ()
, isSelected (false)
, isExpanded (false)
, isSelectEnabled (false)
, isScreenshotDisplayEnabled (false)
, isStorageDisplayEnabled (false)
, agentId (agentId)
, agentTaskCount (0)
, screenshotTime (0)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, screenshotImage (NULL)
, statusIcon (NULL)
, taskCountIcon (NULL)
, storageIcon (NULL)
, streamCountIcon (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, actionButton (NULL)
, agentTaskWindow (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::MonitorWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
	setCornerRadius (uiconfig->cornerRadius);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeDisplayIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	screenshotImage = (ImageWindow *) addWidget (new ImageWindow (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	screenshotImage->setLoadCallback (MonitorWindow::screenshotImageLoaded, this);
	screenshotImage->setMouseLongPressCallback (MonitorWindow::screenshotImageLongPressed, this);
	screenshotImage->setLoadResize (true, ((float) App::instance->windowWidth) * MonitorWindow::screenshotImageScale);
	screenshotImage->setFillBg (true, uiconfig->darkBackgroundColor);
	screenshotImage->isVisible = false;

	statusIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::InactiveStateIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	statusIcon->setPadding (0.0f, 0.0f);
	statusIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	statusIcon->setMouseHoverTooltip (uitext->getText (UiTextString::monitorActivityIconTooltip));
	statusIcon->isVisible = false;

	taskCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::TaskCountIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	taskCountIcon->setPadding (0.0f, 0.0f);
	taskCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	taskCountIcon->isVisible = false;

	storageIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	storageIcon->setPadding (0.0f, 0.0f);
	storageIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	storageIcon->setMouseHoverTooltip (uitext->getText (UiTextString::storageTooltip));
	storageIcon->isVisible = false;

	streamCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	streamCountIcon->setPadding (0.0f, 0.0f);
	streamCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	streamCountIcon->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (MonitorWindow::selectToggleStateChanged, this);
	selectToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::unselectedToggleTooltip), uitext->getText (UiTextString::selectedToggleTooltip));
	selectToggle->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (MonitorWindow::expandToggleStateChanged, this);
	expandToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::expand).capitalized (), uitext->getText (UiTextString::minimize).capitalized ());

	agentTaskWindow = (AgentTaskWindow *) addWidget (new AgentTaskWindow (agentId));

	refreshLayout ();
}

MonitorWindow::~MonitorWindow () {

}

StdString MonitorWindow::toStringDetail () {
	return (StdString::createSprintf (" MonitorWindow agentId=\"%s\"", agentId.c_str ()));
}

bool MonitorWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::MonitorWindow));
}

MonitorWindow *MonitorWindow::castWidget (Widget *widget) {
	return (MonitorWindow::isWidgetType (widget) ? (MonitorWindow *) widget : NULL);
}

void MonitorWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Json *record, serverstatus;
	StdString path, displayname, intentname, text;
	int64_t t;
	int intentlen, displaylen, count;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	record = store->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}
	if (! interface->getCommandObjectParam (record, "monitorServerStatus", &serverstatus)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	agentName.assign (interface->getCommandAgentName (record));
	nameLabel->setText (agentName);
	agentTaskCount = interface->getCommandNumberParam (record, "taskCount", (int) 0);
	descriptionLabel->setText (interface->getCommandStringParam (record, "applicationName", ""));

	path = serverstatus.getString ("screenshotPath", "");
	if ((! isScreenshotDisplayEnabled) || path.empty ()) {
		screenshotImage->isVisible = false;
	}
	else {
		screenshotImage->setImageUrl (App::instance->agentControl.getAgentSecondaryUrl (agentId, NULL, path));
		screenshotImage->isVisible = isExpanded;
		t = serverstatus.getNumber ("screenshotTime", (int64_t) -1);
		if ((t > 0) && (t != screenshotTime)) {
			screenshotTime = t;
			if (screenshotImage->isVisible) {
				screenshotImage->reload ();
			}
		}
	}

	intentlen = 24;
	displaylen = 32;
	intentname = serverstatus.getString ("intentName", "");
	if (serverstatus.getBoolean ("isPlaying", false)) {
		displayname = serverstatus.getString ("mediaName", "");
		if (! displayname.empty ()) {
			if (intentname.empty ()) {
				text.assign (uitext->getText (UiTextString::playing).capitalized ());
				displaylen += 8;
			}
			else {
				text.sprintf ("[%s]", intentname.truncated (intentlen).c_str ());
			}
			text.appendSprintf (" %s", displayname.truncated (displaylen).c_str ());
		}
	}
	else if (serverstatus.getBoolean ("isShowingUrl", false)) {
		displayname = serverstatus.getString ("showUrl", "");
		if (! displayname.empty ()) {
			if (intentname.empty ()) {
				text.assign (uitext->getText (UiTextString::showing).capitalized ());
				displaylen += 8;
			}
			else {
				text.sprintf ("[%s]", intentname.truncated (intentlen).c_str ());
			}
			text.appendSprintf (" %s", displayname.truncated (displaylen).c_str ());
		}
	}
	if (text.empty ()) {
		if (! intentname.empty ()) {
			text.sprintf ("[%s]", intentname.truncated (intentlen).c_str ());
		}
	}
	if (text.empty ()) {
		statusIcon->setIconSprite (uiconfig->coreSprites.getSprite (UiConfiguration::InactiveStateIconSprite));
		statusIcon->setText (uitext->getText (UiTextString::inactive).capitalized ());
	}
	else {
		if (serverstatus.getBoolean ("isPlayPaused", false)) {
			statusIcon->setIconSprite (uiconfig->coreSprites.getSprite (UiConfiguration::PauseIconSprite));
		}
		else {
			statusIcon->setIconSprite (uiconfig->coreSprites.getSprite (UiConfiguration::ActiveStateIconSprite));
		}
		statusIcon->setText (text);
	}

	taskCountIcon->setText (StdString::createSprintf ("%i", agentTaskCount));
	taskCountIcon->setMouseHoverTooltip (uitext->getCountText (agentTaskCount, UiTextString::taskInProgress, UiTextString::tasksInProgress));

	agentTaskWindow->syncRecordStore ();
	if ((agentTaskCount > 0) && isExpanded) {
		taskCountIcon->isVisible = true;
		if (! agentTaskWindow->isTaskRunning) {
			agentTaskWindow->isVisible = false;
		}
		else {
			agentTaskWindow->isVisible = true;
		}
	}
	else {
		taskCountIcon->isVisible = false;
		agentTaskWindow->isVisible = false;
	}

	storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));

	count = serverstatus.getNumber ("streamCount", (int) 0);
	streamCountIcon->setText (StdString::createSprintf ("%i", count));
	streamCountIcon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::cachedStream, UiTextString::cachedStreams));

	refreshLayout ();
	Panel::syncRecordStore ();
}

void MonitorWindow::setSelectEnabled (bool enable) {
	if (isSelectEnabled == enable) {
		return;
	}
	isSelectEnabled = enable;
	if (isSelectEnabled) {
		selectToggle->isVisible = true;
	}
	else {
		selectToggle->isVisible = false;
	}
	refreshLayout ();
}

void MonitorWindow::setScreenshotDisplayEnabled (bool enable) {
	if (isScreenshotDisplayEnabled == enable) {
		return;
	}
	isScreenshotDisplayEnabled = enable;
	if (isScreenshotDisplayEnabled) {
		screenshotImage->isVisible = isExpanded;
	}
	else {
		screenshotImage->isVisible = false;
	}
	refreshLayout ();
}

void MonitorWindow::setStorageDisplayEnabled (bool enable) {
	if (isStorageDisplayEnabled == enable) {
		return;
	}
	isStorageDisplayEnabled = enable;
	if (isStorageDisplayEnabled) {
		if (isExpanded) {
			storageIcon->isVisible = true;
			streamCountIcon->isVisible = true;
		}
		else {
			storageIcon->isVisible = false;
			streamCountIcon->isVisible = false;
		}
	}
	else {
		storageIcon->isVisible = false;
		streamCountIcon->isVisible = false;
	}
	refreshLayout ();
}

void MonitorWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
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

void MonitorWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
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

		if (screenshotImage->isImageUrlEmpty ()) {
			screenshotImage->isVisible = false;
		}
		else {
			screenshotImage->isVisible = isScreenshotDisplayEnabled;
		}

		statusIcon->isVisible = true;

		if (agentTaskCount > 0) {
			taskCountIcon->isVisible = true;
			if (! agentTaskWindow->isTaskRunning) {
				agentTaskWindow->isVisible = false;
			}
			else {
				agentTaskWindow->isVisible = true;
			}
		}
		else {
			taskCountIcon->isVisible = false;
			agentTaskWindow->isVisible = false;
		}

		if (isStorageDisplayEnabled) {
			storageIcon->isVisible = true;
			streamCountIcon->isVisible = true;
		}
		else {
			storageIcon->isVisible = false;
			streamCountIcon->isVisible = false;
		}

		if (actionButton) {
			actionButton->isVisible = true;
		}
	}
	else {
		setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		descriptionLabel->isVisible = false;
		screenshotImage->isVisible = false;
		statusIcon->isVisible = false;
		taskCountIcon->isVisible = false;
		agentTaskWindow->isVisible = false;
		storageIcon->isVisible = false;
		streamCountIcon->isVisible = false;
		if (actionButton) {
			actionButton->isVisible = false;
		}
	}

	refreshLayout ();
}

void MonitorWindow::refreshLayout () {
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
	if (descriptionLabel->isVisible) {
		descriptionLabel->flowRight (&x, y, &x2, &y2);
	}

	x = x2 + uiconfig->marginSize;
	y = y0;
	expandToggle->flowRight (&x, y, &x2, &y2);
	if (selectToggle->isVisible) {
		selectToggle->flowRight (&x, y, &x2, &y2);
	}
	if (statusIcon->isVisible) {
		x = x0;
		y = y2 + uiconfig->marginSize;
		x2 = 0.0f;
		statusIcon->flowDown (x, &y, &x2, &y2);
	}

	if (screenshotImage->isVisible) {
		x = x0;
		y = y2 + uiconfig->marginSize;
		screenshotImage->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (storageIcon->isVisible && streamCountIcon->isVisible) {
		storageIcon->flowRight (&x, y, &x2, &y2);
		streamCountIcon->flowRight (&x, y, &x2, &y2);
	}
	if (taskCountIcon->isVisible) {
		taskCountIcon->flowRight (&x, y, &x2, &y2);
	}
	if (agentTaskWindow->isVisible) {
		x = x0;
		y = y2 + uiconfig->marginSize;
		x2 = 0.0f;
		agentTaskWindow->flowDown (x, &y, &x2, &y2);
	}
	if (actionButton && actionButton->isVisible) {
		actionButton->flowRight (&x, y, &x2, &y2);
	}

	resetSize ();

	x = width - widthPadding;
	if (actionButton && actionButton->isVisible) {
		actionButton->flowLeft (&x);
	}

	x = width - widthPadding;
	if (selectToggle->isVisible) {
		selectToggle->flowLeft (&x);
	}
	expandToggle->flowLeft (&x);
}

void MonitorWindow::screenshotImageLoaded (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = (MonitorWindow *) windowPtr;
	if (window->screenshotLoadCallback.callback) {
		window->screenshotLoadCallback.callback (window->screenshotLoadCallback.callbackData, window);
	}
}

void MonitorWindow::screenshotImageLongPressed (void *windowPtr, Widget *widgetPtr) {
	ImageWindow *image;

	image = (ImageWindow *) widgetPtr;
	App::instance->uiStack.showImageDialog (image->imageUrl);
}

void MonitorWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	Toggle *toggle;
	UiConfiguration *uiconfig;

	window = (MonitorWindow *) windowPtr;
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

void MonitorWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	Toggle *toggle;

	window = (MonitorWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback.callback) {
		window->expandStateChangeCallback.callback (window->expandStateChangeCallback.callbackData, window);
	}
}

void MonitorWindow::addActionButton (Sprite *sprite, Widget::EventCallback clickCallback, void *clickCallbackData, const StdString &tooltipText) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);

	if (actionButton) {
		actionButton->isDestroyed = true;
	}
	actionButton = (Button *) addWidget (new Button (sprite));
	actionButton->setImageColor (uiconfig->flatButtonTextColor);
	actionButton->setMouseClickCallback (MonitorWindow::actionButtonClicked, this);
	if (! tooltipText.empty ()) {
		actionButton->setMouseHoverTooltip (tooltipText);
	}
	actionClickCallback = Widget::EventCallbackContext (clickCallback, clickCallbackData);

	if (isExpanded) {
		actionButton->isVisible = true;
	}
	else {
		actionButton->isVisible = false;
	}
	refreshLayout ();
}

void MonitorWindow::actionButtonClicked (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = (MonitorWindow *) windowPtr;
	if (window->actionClickCallback.callback) {
		window->actionClickCallback.callback (window->actionClickCallback.callbackData, window);
	}
}
