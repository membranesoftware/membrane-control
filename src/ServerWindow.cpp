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
#include "Result.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "OsUtil.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Color.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Label.h"
#include "HashMap.h"
#include "Button.h"
#include "TextArea.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Image.h"
#include "Json.h"
#include "SystemInterface.h"
#include "IconLabelWindow.h"
#include "ServerWindow.h"

const float ServerWindow::ExpandedNameTruncateScale = 0.24f;
const float ServerWindow::UnexpandedNameTruncateScale = 0.10f;

ServerWindow::ServerWindow (const StdString &agentId)
: Panel ()
, isExpanded (false)
, agentId (agentId)
, serverType (-1)
, isRecordLoaded (false)
, isAgentDisabled (false)
, agentTaskCount (0)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, dividerPanel (NULL)
, statusIcon (NULL)
, unexpandedStatusIcon (NULL)
, authorizeIcon (NULL)
, storageIcon (NULL)
, streamCountIcon (NULL)
, taskCountIcon (NULL)
, statsWindow (NULL)
, expandToggle (NULL)
, checkForUpdatesButton (NULL)
, adminButton (NULL)
, detachButton (NULL)
, removeButton (NULL)
, statusSpriteType (-1)
, statusTextString (-1)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::ServerWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
	setCornerRadius (uiconfig->cornerRadius);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, uiconfig->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, uiconfig->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	statusIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::ConnectionWaitingStateIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	statusIcon->setPadding (0.0f, 0.0f);
	statusIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	statusIcon->setIconImageColor (Color (0.0f, 0.0f, 0.0f));
	statusIcon->setMouseHoverTooltip (uitext->getText (UiTextString::MonitorActivityIconTooltip));
	statusIcon->isVisible = false;

	unexpandedStatusIcon = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::ConnectionWaitingStateIconSprite)));
	unexpandedStatusIcon->isVisible = false;

	authorizeIcon = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::SmallKeyIconSprite)));
	authorizeIcon->setMouseHoverTooltip (uitext->getText (UiTextString::AuthorizeIconTooltip));
	authorizeIcon->isVisible = false;

	storageIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	storageIcon->setPadding (0.0f, 0.0f);
	storageIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	storageIcon->setMouseHoverTooltip (uitext->getText (UiTextString::StorageTooltip));
	storageIcon->isVisible = false;

	mediaCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallMediaIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	mediaCountIcon->setPadding (0.0f, 0.0f);
	mediaCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	mediaCountIcon->isVisible = false;

	streamCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	streamCountIcon->setPadding (0.0f, 0.0f);
	streamCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	streamCountIcon->isVisible = false;

	taskCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::TaskCountIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	taskCountIcon->setPadding (0.0f, 0.0f);
	taskCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	taskCountIcon->isVisible = false;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
	statsWindow->setItem (uitext->getText (UiTextString::Uptime).capitalized (), StdString (""));
	statsWindow->setItem (uitext->getText (UiTextString::Version).capitalized (), StdString (""));
	statsWindow->setItem (uitext->getText (UiTextString::Address).capitalized (), StdString (""));
	statsWindow->setItem (uitext->getText (UiTextString::Platform).capitalized (), StdString (""));
	statsWindow->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (ServerWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::Expand).capitalized (), uitext->getText (UiTextString::Minimize).capitalized ());

	checkForUpdatesButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::UpdateButtonSprite)));
	checkForUpdatesButton->mouseClickCallback = Widget::EventCallbackContext (ServerWindow::checkForUpdatesButtonClicked, this);
	checkForUpdatesButton->setImageColor (uiconfig->flatButtonTextColor);
	checkForUpdatesButton->setMouseHoverTooltip (uitext->getText (UiTextString::UpdateServerTooltip));
	checkForUpdatesButton->isVisible = false;

	adminButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::SettingsBoxButtonSprite)));
	adminButton->mouseClickCallback = Widget::EventCallbackContext (ServerWindow::adminButtonClicked, this);
	adminButton->setImageColor (uiconfig->flatButtonTextColor);
	adminButton->setMouseHoverTooltip (uitext->getText (UiTextString::OpenAdminConsole).capitalized ());
	adminButton->isVisible = false;

	detachButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::DetachServerButtonSprite)));
	detachButton->mouseClickCallback = Widget::EventCallbackContext (ServerWindow::detachButtonClicked, this);
	detachButton->setImageColor (uiconfig->flatButtonTextColor);
	detachButton->setMouseHoverTooltip (uitext->getText (UiTextString::DetachServerTooltip));
	detachButton->isVisible = false;

	removeButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (ServerWindow::removeButtonClicked, this);
	removeButton->setImageColor (uiconfig->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (uitext->getText (UiTextString::RemoveServer).capitalized ());
	removeButton->isVisible = false;

	refreshLayout ();
}

ServerWindow::~ServerWindow () {

}

StdString ServerWindow::toStringDetail () {
	return (StdString::createSprintf (" ServerWindow agentId=%s", agentId.c_str ()));
}

bool ServerWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::ServerWindow));
}

ServerWindow *ServerWindow::castWidget (Widget *widget) {
	return (ServerWindow::isWidgetType (widget) ? (ServerWindow *) widget : NULL);
}

Widget::Rectangle ServerWindow::getRemoveButtonScreenRect () {
	return (removeButton->getScreenRect ());
}

void ServerWindow::syncRecordStore () {
	RecordStore *store;
	AgentControl *agentcontrol;
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Json *record, serverstatus;
	StdString version, platform;
	int type, count;

	store = &(App::instance->agentControl.recordStore);
	agentcontrol = &(App::instance->agentControl);
	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	statsWindow->setItem (uitext->getText (UiTextString::Address).capitalized (), agentcontrol->getAgentHostAddress (agentId));
	record = store->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		isRecordLoaded = false;
		isAgentDisabled = false;
		agentDisplayName = agentcontrol->getAgentDisplayName (agentId);
		descriptionLabel->setText (agentcontrol->getAgentApplicationName (agentId));
	}
	else {
		isRecordLoaded = true;
		version = interface->getCommandStringParam (record, "version", "");
		platform = interface->getCommandStringParam (record, "platform", "");
		agentDisplayName = interface->getCommandAgentName (record);
		descriptionLabel->setText (interface->getCommandStringParam (record, "applicationName", ""));
		agentTaskCount = interface->getCommandNumberParam (record, "taskCount", (int) 0);
		taskCountIcon->setText (StdString::createSprintf ("%i", agentTaskCount));
		taskCountIcon->setMouseHoverTooltip (uitext->getCountText (agentTaskCount, UiTextString::TaskInProgress, UiTextString::TasksInProgress));

		if (! interface->getCommandBooleanParam (record, "isEnabled", false)) {
			if (! isAgentDisabled) {
				isAgentDisabled = true;
				statusTextString = -1;
			}
		}
		else {
			if (isAgentDisabled) {
				isAgentDisabled = false;
				statusTextString = -1;
			}
		}

		statsWindow->setItem (uitext->getText (UiTextString::Version).capitalized (), interface->getCommandStringParam (record, "version", ""));
		statsWindow->setItem (uitext->getText (UiTextString::Platform).capitalized (), interface->getCommandStringParam (record, "platform", ""));
		statsWindow->setItem (uitext->getText (UiTextString::Uptime).capitalized (), interface->getCommandStringParam (record, "uptime", ""));
	}

	type = agentcontrol->getAgentServerType (agentId);
	if (type != serverType) {
		serverType = type;
		iconImage->isDestroyed = true;
		switch (serverType) {
			case SystemInterface::Constant_Monitor: {
				iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeDisplayIconSprite)));
				break;
			}
			case SystemInterface::Constant_Media: {
				iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite)));
				break;
			}
			case SystemInterface::Constant_Camera: {
				iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeCameraIconSprite)));
				break;
			}
			default: {
				iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite)));
				break;
			}
		}
	}

	if (isRecordLoaded) {
		switch (serverType) {
			case SystemInterface::Constant_Monitor: {
				if (interface->getCommandObjectParam (record, "monitorServerStatus", &serverstatus)) {
					storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));

					count = serverstatus.getNumber ("streamCount", (int) 0);
					streamCountIcon->setText (StdString::createSprintf ("%i", count));
					streamCountIcon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::CachedStream, UiTextString::CachedStreams));
				}
				break;
			}
			case SystemInterface::Constant_Media: {
				if (interface->getCommandObjectParam (record, "mediaServerStatus", &serverstatus)) {
					count = serverstatus.getNumber ("mediaCount", (int) 0);
					mediaCountIcon->setText (StdString::createSprintf ("%i", count));
					mediaCountIcon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::MediaFile, UiTextString::MediaFiles));
				}

				if (interface->getCommandObjectParam (record, "streamServerStatus", &serverstatus)) {
					storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));

					count = serverstatus.getNumber ("streamCount", (int) 0);
					streamCountIcon->setText (StdString::createSprintf ("%i", count));
					streamCountIcon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::VideoStream, UiTextString::VideoStreams));
				}
				break;
			}
			case SystemInterface::Constant_Camera: {
				if (interface->getCommandObjectParam (record, "cameraServerStatus", &serverstatus)) {
					storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));
				}
				break;
			}
		}

		updateUrl = agentcontrol->getAgentUpdateUrl (agentId);
	}

	resetVisibility ();
	refreshLayout ();
	resetNameLabel ();
	Panel::syncRecordStore ();
}

void ServerWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
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
	}
	else {
		setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
	}

	resetVisibility ();
	refreshLayout ();
	resetNameLabel ();
}

void ServerWindow::resetVisibility () {
	if (isExpanded) {
		unexpandedStatusIcon->isVisible = false;
		descriptionLabel->isVisible = true;
		dividerPanel->isVisible = true;
		statusIcon->isVisible = true;
		statsWindow->isVisible = isRecordLoaded;
		taskCountIcon->isVisible = isRecordLoaded && (agentTaskCount > 0);
		checkForUpdatesButton->isVisible = (checkForUpdatesClickCallback.callback && isRecordLoaded && (! updateUrl.empty ()));
		adminButton->isVisible = adminClickCallback.callback && isRecordLoaded;
		detachButton->isVisible = detachClickCallback.callback ? true : false;
		removeButton->isVisible = removeClickCallback.callback ? true : false;

		switch (serverType) {
			case SystemInterface::Constant_Monitor: {
				mediaCountIcon->isVisible = false;
				storageIcon->isVisible = isRecordLoaded;
				streamCountIcon->isVisible = isRecordLoaded;
				break;
			}
			case SystemInterface::Constant_Media: {
				storageIcon->isVisible = isRecordLoaded;
				mediaCountIcon->isVisible = isRecordLoaded;
				streamCountIcon->isVisible = isRecordLoaded;
				break;
			}
			case SystemInterface::Constant_Camera: {
				mediaCountIcon->isVisible = false;
				streamCountIcon->isVisible = false;
				storageIcon->isVisible = isRecordLoaded;
				break;
			}
			default: {
				storageIcon->isVisible = false;
				mediaCountIcon->isVisible = false;
				streamCountIcon->isVisible = false;
				break;
			}
		}
	}
	else {
		unexpandedStatusIcon->isVisible = true;
		descriptionLabel->isVisible = false;
		dividerPanel->isVisible = false;
		statusIcon->isVisible = false;
		statsWindow->isVisible = false;
		storageIcon->isVisible = false;
		mediaCountIcon->isVisible = false;
		streamCountIcon->isVisible = false;
		taskCountIcon->isVisible = false;
		checkForUpdatesButton->isVisible = false;
		adminButton->isVisible = false;
		detachButton->isVisible = false;
		removeButton->isVisible = false;
	}

	authorizeIcon->isVisible = App::instance->agentControl.isAgentAuthorized (agentId);
}

void ServerWindow::resetNameLabel () {
	float w;

	w = App::instance->windowWidth;
	if (isExpanded) {
		w *= ServerWindow::ExpandedNameTruncateScale;
		nameLabel->setText (Label::getTruncatedText (agentDisplayName, UiConfiguration::HeadlineFont, w, Label::DotTruncateSuffix));
	}
	else {
		w *= ServerWindow::UnexpandedNameTruncateScale;
		nameLabel->setText (Label::getTruncatedText (agentDisplayName, UiConfiguration::BodyFont, w, Label::DotTruncateSuffix));
	}
	refreshLayout ();
}

void ServerWindow::refreshLayout () {
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

	x = nameLabel->position.x;
	y = nameLabel->position.y + nameLabel->height + (uiconfig->marginSize / 2.0f);
	if (unexpandedStatusIcon->isVisible) {
		unexpandedStatusIcon->flowRight (&x, y, &x2, &y2);
	}
	if ((! isExpanded) && authorizeIcon->isVisible) {
		authorizeIcon->flowRight (&x, y, &x2, &y2);
	}

	x = x2 + uiconfig->marginSize;
	y = y0;
	expandToggle->flowDown (x, &y, &x2, &y2);
	if (dividerPanel->isVisible) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
	}

	x0 += uiconfig->marginSize;
	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (mediaCountIcon->isVisible) {
		mediaCountIcon->flowRight (&x, y, &x2, &y2);
	}
	if (streamCountIcon->isVisible) {
		streamCountIcon->flowRight (&x, y, &x2, &y2);
	}
	if (storageIcon->isVisible) {
		storageIcon->flowRight (&x, y, &x2, &y2);
	}
	if (taskCountIcon->isVisible) {
		taskCountIcon->flowRight (&x, y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (statusIcon->isVisible) {
		statusIcon->flowRight (&x, y, &x2, &y2);
	}
	if (isExpanded && authorizeIcon->isVisible) {
		authorizeIcon->position.assign (x0, y2 + uiconfig->marginSize);
	}
	if (statsWindow->isVisible) {
		statsWindow->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (removeButton->isVisible) {
		removeButton->flowRight (&x, y, &x2, &y2);
	}
	if (detachButton->isVisible) {
		detachButton->flowRight (&x, y, &x2, &y2);
	}
	if (checkForUpdatesButton->isVisible) {
		checkForUpdatesButton->flowRight (&x, y, &x2, &y2);
	}
	if (adminButton->isVisible) {
		adminButton->flowRight (&x, y, &x2, &y2);
	}
	resetSize ();

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, uiconfig->headlineDividerLineWidth);
	}

	x = width - widthPadding;
	expandToggle->flowLeft (&x);

	x = width - widthPadding;
	if (adminButton->isVisible) {
		adminButton->flowLeft (&x);
	}
	if (checkForUpdatesButton->isVisible) {
		checkForUpdatesButton->flowLeft (&x);
	}
	if (detachButton->isVisible) {
		detachButton->flowLeft (&x);
	}
	if (removeButton->isVisible) {
		removeButton->flowLeft (&x);
	}
}

void ServerWindow::doUpdate (int msElapsed) {
	AgentControl *agentcontrol;
	UiConfiguration *uiconfig;
	UiText *uitext;

	Panel::doUpdate (msElapsed);

	agentcontrol = &(App::instance->agentControl);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	if (statusTextString == UiTextString::Contacting) {
		if (! agentcontrol->isAgentContacting (agentId)) {
			statusTextString = -1;
		}
	}
	if (statusTextString < 0) {
		if (agentcontrol->isAgentContacting (agentId)) {
			setStatusIcons (UiConfiguration::ConnectionWaitingStateIconSprite, UiTextString::Contacting, uiconfig->primaryTextColor);
		}
		else if (agentcontrol->isAgentUnauthorized (agentId)) {
			setStatusIcons (UiConfiguration::ConnectionFailedStateIconSprite, UiTextString::PasswordLocked, uiconfig->errorTextColor);
			statsWindow->setItem (uitext->getText (UiTextString::Uptime).capitalized (), StdString (""));
		}
		else if (isAgentDisabled) {
			setStatusIcons (UiConfiguration::ServerDisabledStateIconSprite, UiTextString::Disabled, uiconfig->errorTextColor);
		}
		else if (! agentcontrol->isAgentContacted (agentId)) {
			setStatusIcons (UiConfiguration::ConnectionFailedStateIconSprite, UiTextString::Offline, uiconfig->errorTextColor);
			statsWindow->setItem (uitext->getText (UiTextString::Uptime).capitalized (), StdString (""));
		}
		else {
			setStatusIcons (UiConfiguration::ConnectedStateIconSprite, UiTextString::Online, uiconfig->statusOkTextColor);
		}
	}
	else {
		if (agentcontrol->isAgentContacting (agentId)) {
			setStatusIcons (UiConfiguration::ConnectionWaitingStateIconSprite, UiTextString::Contacting, uiconfig->primaryTextColor);
		}
	}

	resetVisibility ();
	refreshLayout ();
}

void ServerWindow::setStatusIcons (int spriteType, int textString, const Color &color) {
	UiConfiguration *uiconfig;
	UiText *uitext;
	StdString text;

	if ((spriteType == statusSpriteType) && (textString == statusTextString) && color.equals (statusColor)) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	text = uitext->getText (textString).capitalized ();
	statusIcon->setIconSprite (uiconfig->coreSprites.getSprite (spriteType));
	statusIcon->setIconImageColor (Color (0.0f, 0.0f, 0.0f));
	statusIcon->setText (text);
	statusIcon->setTextColor (color);
	statusIcon->setMouseHoverTooltip (StdString::createSprintf ("%s: %s", uitext->getText (UiTextString::Status).capitalized ().c_str (), text.c_str ()));

	unexpandedStatusIcon->isDestroyed = true;
	unexpandedStatusIcon = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (spriteType)));
	unexpandedStatusIcon->setDrawColor (true, color);
	unexpandedStatusIcon->setMouseHoverTooltip (StdString::createSprintf ("%s: %s", uitext->getText (UiTextString::Status).capitalized ().c_str (), text.c_str ()));

	statusSpriteType = spriteType;
	statusTextString = textString;
	statusColor.assign (color);

	if (statusChangeCallback.callback) {
		statusChangeCallback.callback (statusChangeCallback.callbackData, this);
	}
}

void ServerWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	ServerWindow *window;
	Toggle *toggle;

	window = (ServerWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback.callback) {
		window->expandStateChangeCallback.callback (window->expandStateChangeCallback.callbackData, window);
	}
}

void ServerWindow::checkForUpdatesButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ServerWindow *window;

	window = (ServerWindow *) windowPtr;
	if (window->checkForUpdatesClickCallback.callback) {
		window->checkForUpdatesClickCallback.callback (window->checkForUpdatesClickCallback.callbackData, window);
	}
}

void ServerWindow::adminButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ServerWindow *window;

	window = (ServerWindow *) windowPtr;
	if (window->adminClickCallback.callback) {
		window->adminClickCallback.callback (window->adminClickCallback.callbackData, window);
	}
}

void ServerWindow::detachButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ServerWindow *window;

	window = (ServerWindow *) windowPtr;
	if (window->detachClickCallback.callback) {
		window->detachClickCallback.callback (window->detachClickCallback.callbackData, window);
	}
}

void ServerWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ServerWindow *window;

	window = (ServerWindow *) windowPtr;
	if (window->removeClickCallback.callback) {
		window->removeClickCallback.callback (window->removeClickCallback.callbackData, window);
	}
}
