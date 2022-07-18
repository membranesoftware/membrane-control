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
#include "UiText.h"
#include "OsUtil.h"
#include "AgentControl.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Color.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Label.h"
#include "HashMap.h"
#include "Button.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Image.h"
#include "Json.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Agent.h"
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
	classId = ClassId::ServerWindow;

	setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	statusIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ConnectionWaitingStateIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	statusIcon->setPadding (0.0f, 0.0f);
	statusIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	statusIcon->setIconImageColor (Color (0.0f, 0.0f, 0.0f));
	statusIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MonitorActivityIconTooltip));
	statusIcon->isVisible = false;

	unexpandedStatusIcon = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ConnectionWaitingStateIconSprite)));
	unexpandedStatusIcon->isVisible = false;

	authorizeIcon = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallKeyIconSprite)));
	authorizeIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::AuthorizeIconTooltip));
	authorizeIcon->isVisible = false;

	storageIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	storageIcon->setPadding (0.0f, 0.0f);
	storageIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	storageIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::StorageTooltip));
	storageIcon->isVisible = false;

	mediaCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallMediaIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	mediaCountIcon->setPadding (0.0f, 0.0f);
	mediaCountIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	mediaCountIcon->isVisible = false;

	streamCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	streamCountIcon->setPadding (0.0f, 0.0f);
	streamCountIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	streamCountIcon->isVisible = false;

	taskCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::TaskCountIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	taskCountIcon->setPadding (0.0f, 0.0f);
	taskCountIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	taskCountIcon->isVisible = false;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (UiConfiguration::instance->paddingSize, 0.0f);
	statsWindow->setItem (UiText::instance->getText (UiTextString::Uptime).capitalized (), StdString (""));
	statsWindow->setItem (UiText::instance->getText (UiTextString::Version).capitalized (), StdString (""));
	statsWindow->setItem (UiText::instance->getText (UiTextString::Address).capitalized (), StdString (""));
	statsWindow->setItem (UiText::instance->getText (UiTextString::Platform).capitalized (), StdString (""));
	statsWindow->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (ServerWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());

	checkForUpdatesButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::UpdateButtonSprite)));
	checkForUpdatesButton->mouseClickCallback = Widget::EventCallbackContext (ServerWindow::checkForUpdatesButtonClicked, this);
	checkForUpdatesButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	checkForUpdatesButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::UpdateServerTooltip));
	checkForUpdatesButton->isVisible = false;

	adminButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SettingsBoxButtonSprite)));
	adminButton->mouseClickCallback = Widget::EventCallbackContext (ServerWindow::adminButtonClicked, this);
	adminButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	adminButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::OpenAdminConsole).capitalized ());
	adminButton->isVisible = false;

	detachButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DetachServerButtonSprite)));
	detachButton->mouseClickCallback = Widget::EventCallbackContext (ServerWindow::detachButtonClicked, this);
	detachButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	detachButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::DetachServerTooltip));
	detachButton->isVisible = false;

	removeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (ServerWindow::removeButtonClicked, this);
	removeButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::RemoveServer).capitalized ());
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
	Json *record, serverstatus;
	StdString version, platform;
	int type, count;

	statsWindow->setItem (UiText::instance->getText (UiTextString::Address).capitalized (), AgentControl::instance->getAgentHostAddress (agentId));
	record = RecordStore::instance->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		isRecordLoaded = false;
		isAgentDisabled = false;
		agentDisplayName = AgentControl::instance->getAgentDisplayName (agentId);
		descriptionLabel->setText (AgentControl::instance->getAgentApplicationName (agentId));
	}
	else {
		isRecordLoaded = true;
		version = SystemInterface::instance->getCommandStringParam (record, "version", "");
		platform = SystemInterface::instance->getCommandStringParam (record, "platform", "");
		agentDisplayName = Agent::getCommandAgentName (record);
		descriptionLabel->setText (SystemInterface::instance->getCommandStringParam (record, "applicationName", ""));
		agentTaskCount = SystemInterface::instance->getCommandNumberParam (record, "taskCount", (int) 0);
		taskCountIcon->setText (StdString::createSprintf ("%i", agentTaskCount));
		taskCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (agentTaskCount, UiTextString::TaskInProgress, UiTextString::TasksInProgress));

		if (! SystemInterface::instance->getCommandBooleanParam (record, "isEnabled", false)) {
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

		statsWindow->setItem (UiText::instance->getText (UiTextString::Version).capitalized (), SystemInterface::instance->getCommandStringParam (record, "version", ""));
		statsWindow->setItem (UiText::instance->getText (UiTextString::Platform).capitalized (), SystemInterface::instance->getCommandStringParam (record, "platform", ""));
		statsWindow->setItem (UiText::instance->getText (UiTextString::Uptime).capitalized (), SystemInterface::instance->getCommandStringParam (record, "uptime", ""));
	}

	type = AgentControl::instance->getAgentServerType (agentId);
	if (type != serverType) {
		serverType = type;
		iconImage->isDestroyed = true;
		switch (serverType) {
			case Agent::Monitor: {
				iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeDisplayIconSprite)));
				break;
			}
			case Agent::Media: {
				iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite)));
				break;
			}
			case Agent::Camera: {
				iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeCameraIconSprite)));
				break;
			}
			default: {
				iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite)));
				break;
			}
		}
	}

	if (isRecordLoaded) {
		switch (serverType) {
			case Agent::Monitor: {
				if (SystemInterface::instance->getCommandObjectParam (record, "monitorServerStatus", &serverstatus)) {
					storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));

					count = serverstatus.getNumber ("streamCount", (int) 0);
					streamCountIcon->setText (StdString::createSprintf ("%i", count));
					streamCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::CachedStream, UiTextString::CachedStreams));
				}
				break;
			}
			case Agent::Media: {
				if (SystemInterface::instance->getCommandObjectParam (record, "mediaServerStatus", &serverstatus)) {
					count = serverstatus.getNumber ("mediaCount", (int) 0);
					mediaCountIcon->setText (StdString::createSprintf ("%i", count));
					mediaCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::MediaFile, UiTextString::MediaFiles));
				}

				if (SystemInterface::instance->getCommandObjectParam (record, "streamServerStatus", &serverstatus)) {
					storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));

					count = serverstatus.getNumber ("streamCount", (int) 0);
					streamCountIcon->setText (StdString::createSprintf ("%i", count));
					streamCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::VideoStream, UiTextString::VideoStreams));
				}
				break;
			}
			case Agent::Camera: {
				if (SystemInterface::instance->getCommandObjectParam (record, "cameraServerStatus", &serverstatus)) {
					storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));
				}
				break;
			}
		}

		updateUrl = AgentControl::instance->getAgentUpdateUrl (agentId);
	}

	resetVisibility ();
	refreshLayout ();
	resetNameLabel ();
	Panel::syncRecordStore ();
}

void ServerWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
		expandToggle->setChecked (true, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
	}
	else {
		setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
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
			case Agent::Monitor: {
				mediaCountIcon->isVisible = false;
				storageIcon->isVisible = isRecordLoaded;
				streamCountIcon->isVisible = isRecordLoaded;
				break;
			}
			case Agent::Media: {
				storageIcon->isVisible = isRecordLoaded;
				mediaCountIcon->isVisible = isRecordLoaded;
				streamCountIcon->isVisible = isRecordLoaded;
				break;
			}
			case Agent::Camera: {
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

	authorizeIcon->isVisible = AgentControl::instance->isAgentAuthorized (agentId);
}

void ServerWindow::resetNameLabel () {
	float w;

	w = App::instance->windowWidth;
	if (isExpanded) {
		w *= ServerWindow::ExpandedNameTruncateScale;
		nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::HeadlineFont]->truncatedText (agentDisplayName, w, Font::DotTruncateSuffix));
	}
	else {
		w *= ServerWindow::UnexpandedNameTruncateScale;
		nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::BodyFont]->truncatedText (agentDisplayName, w, Font::DotTruncateSuffix));
	}
	refreshLayout ();
}

void ServerWindow::refreshLayout () {
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

	x = nameLabel->position.x;
	y = nameLabel->position.y + nameLabel->height + (UiConfiguration::instance->marginSize / 2.0f);
	if (unexpandedStatusIcon->isVisible) {
		unexpandedStatusIcon->flowRight (&x, y, &x2, &y2);
	}
	if ((! isExpanded) && authorizeIcon->isVisible) {
		authorizeIcon->flowRight (&x, y, &x2, &y2);
	}

	x = x2 + UiConfiguration::instance->marginSize;
	y = y0;
	expandToggle->flowDown (x, &y, &x2, &y2);
	if (dividerPanel->isVisible) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
	}

	x0 += UiConfiguration::instance->marginSize;
	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
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
	y = y2 + UiConfiguration::instance->marginSize;
	x2 = 0.0f;
	if (statusIcon->isVisible) {
		statusIcon->flowRight (&x, y, &x2, &y2);
	}
	if (isExpanded && authorizeIcon->isVisible) {
		authorizeIcon->position.assign (x0, y2 + UiConfiguration::instance->marginSize);
	}
	if (statsWindow->isVisible) {
		statsWindow->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
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
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
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
	Panel::doUpdate (msElapsed);

	if (statusTextString == UiTextString::Contacting) {
		if (! AgentControl::instance->isAgentContacting (agentId)) {
			statusTextString = -1;
		}
	}
	if (statusTextString < 0) {
		if (AgentControl::instance->isAgentContacting (agentId)) {
			setStatusIcons (UiConfiguration::ConnectionWaitingStateIconSprite, UiTextString::Contacting, UiConfiguration::instance->primaryTextColor);
		}
		else if (AgentControl::instance->isAgentUnauthorized (agentId)) {
			setStatusIcons (UiConfiguration::ConnectionFailedStateIconSprite, UiTextString::PasswordLocked, UiConfiguration::instance->errorTextColor);
			statsWindow->setItem (UiText::instance->getText (UiTextString::Uptime).capitalized (), StdString (""));
		}
		else if (isAgentDisabled) {
			setStatusIcons (UiConfiguration::ServerDisabledStateIconSprite, UiTextString::Disabled, UiConfiguration::instance->errorTextColor);
		}
		else if (! AgentControl::instance->isAgentContacted (agentId)) {
			setStatusIcons (UiConfiguration::ConnectionFailedStateIconSprite, UiTextString::Offline, UiConfiguration::instance->errorTextColor);
			statsWindow->setItem (UiText::instance->getText (UiTextString::Uptime).capitalized (), StdString (""));
		}
		else {
			setStatusIcons (UiConfiguration::ConnectedStateIconSprite, UiTextString::Online, UiConfiguration::instance->statusOkTextColor);
		}
	}
	else {
		if (AgentControl::instance->isAgentContacting (agentId)) {
			setStatusIcons (UiConfiguration::ConnectionWaitingStateIconSprite, UiTextString::Contacting, UiConfiguration::instance->primaryTextColor);
		}
	}

	resetVisibility ();
	refreshLayout ();
}

void ServerWindow::setStatusIcons (int spriteType, int textString, const Color &color) {
	StdString text;

	if ((spriteType == statusSpriteType) && (textString == statusTextString) && color.equals (statusColor)) {
		return;
	}
	text = UiText::instance->getText (textString).capitalized ();
	statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (spriteType));
	statusIcon->setIconImageColor (Color (0.0f, 0.0f, 0.0f));
	statusIcon->setText (text);
	statusIcon->setTextColor (color);
	statusIcon->setMouseHoverTooltip (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextString::Status).capitalized ().c_str (), text.c_str ()));

	unexpandedStatusIcon->isDestroyed = true;
	unexpandedStatusIcon = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (spriteType)));
	unexpandedStatusIcon->setDrawColor (true, color);
	unexpandedStatusIcon->setMouseHoverTooltip (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextString::Status).capitalized ().c_str (), text.c_str ()));

	statusSpriteType = spriteType;
	statusTextString = textString;
	statusColor.assign (color);
	eventCallback (statusChangeCallback);
}

void ServerWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	ServerWindow *window;
	Toggle *toggle;

	window = (ServerWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	window->eventCallback (window->expandStateChangeCallback);
}

void ServerWindow::checkForUpdatesButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((ServerWindow *) windowPtr)->eventCallback (((ServerWindow *) windowPtr)->checkForUpdatesClickCallback);
}

void ServerWindow::adminButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((ServerWindow *) windowPtr)->eventCallback (((ServerWindow *) windowPtr)->adminClickCallback);
}

void ServerWindow::detachButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((ServerWindow *) windowPtr)->eventCallback (((ServerWindow *) windowPtr)->detachClickCallback);
}

void ServerWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((ServerWindow *) windowPtr)->eventCallback (((ServerWindow *) windowPtr)->removeClickCallback);
}
