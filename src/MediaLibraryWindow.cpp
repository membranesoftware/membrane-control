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
#include "StdString.h"
#include "UiText.h"
#include "OsUtil.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Agent.h"
#include "AgentControl.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "IconLabelWindow.h"
#include "AgentTaskWindow.h"
#include "MediaLibraryWindow.h"

const float MediaLibraryWindow::TextTruncateScale = 0.21f;

MediaLibraryWindow::MediaLibraryWindow (const StdString &agentId)
: Panel ()
, isExpanded (false)
, agentId (agentId)
, agentTaskCount (0)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, dividerPanel (NULL)
, catalogLinkIcon (NULL)
, taskCountIcon (NULL)
, storageIcon (NULL)
, mediaCountIcon (NULL)
, streamCountIcon (NULL)
, menuButton (NULL)
, expandToggle (NULL)
, agentTaskWindow (NULL)
{
	classId = ClassId::MediaLibraryWindow;

	setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	catalogLinkIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StreamCatalogIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	catalogLinkIcon->textClickCallback = Widget::EventCallbackContext (MediaLibraryWindow::catalogLinkClicked, this);
	catalogLinkIcon->setPadding (0.0f, 0.0f);
	catalogLinkIcon->setTextColor (UiConfiguration::instance->linkTextColor);
	catalogLinkIcon->setTextUnderlined (true);
	catalogLinkIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaServerCatalogTooltip));
	catalogLinkIcon->isVisible = false;

	taskCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::TaskCountIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	taskCountIcon->setPadding (0.0f, 0.0f);
	taskCountIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	taskCountIcon->isVisible = false;

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

	menuButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::MainMenuButtonSprite)));
	menuButton->mouseClickCallback = Widget::EventCallbackContext (MediaLibraryWindow::menuButtonClicked, this);
	menuButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	menuButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MoreActionsTooltip));
	menuButton->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (MediaLibraryWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());

	agentTaskWindow = (AgentTaskWindow *) addWidget (new AgentTaskWindow (agentId));

	refreshLayout ();
}

MediaLibraryWindow::~MediaLibraryWindow () {

}

StdString MediaLibraryWindow::toStringDetail () {
	return (StdString::createSprintf (" MediaLibraryWindow agentId=\"%s\"", agentId.c_str ()));
}

bool MediaLibraryWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::MediaLibraryWindow));
}

MediaLibraryWindow *MediaLibraryWindow::castWidget (Widget *widget) {
	return (MediaLibraryWindow::isWidgetType (widget) ? (MediaLibraryWindow *) widget : NULL);
}

Widget::Rectangle MediaLibraryWindow::getMenuButtonScreenRect () {
	return (menuButton->getScreenRect ());
}

void MediaLibraryWindow::syncRecordStore () {
	Json *record, mediaserverstatus, streamserverstatus;
	int count;

	record = RecordStore::instance->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}
	if (! SystemInterface::instance->getCommandObjectParam (record, "mediaServerStatus", &mediaserverstatus)) {
		return;
	}
	if (! SystemInterface::instance->getCommandObjectParam (record, "streamServerStatus", &streamserverstatus)) {
		return;
	}

	agentName.assign (Agent::getCommandAgentName (record));
	nameLabel->setText (agentName);
	agentTaskCount = SystemInterface::instance->getCommandNumberParam (record, "taskCount", (int) 0);
	descriptionLabel->setText (SystemInterface::instance->getCommandStringParam (record, "applicationName", ""));

	htmlCatalogPath = streamserverstatus.getString ("htmlCatalogPath", "");
	if (htmlCatalogPath.empty ()) {
		catalogLinkIcon->isVisible = false;
	}
	else {
		catalogLinkIcon->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (AgentControl::instance->getAgentSecondaryUrl (agentId, NULL, htmlCatalogPath), ((float) App::instance->windowWidth) * MediaLibraryWindow::TextTruncateScale, Font::DotTruncateSuffix));
		catalogLinkIcon->isVisible = isExpanded;
	}

	storageIcon->setText (OsUtil::getStorageAmountDisplayString (streamserverstatus.getNumber ("freeStorage", (int64_t) 0), streamserverstatus.getNumber ("totalStorage", (int64_t) 0)));

	count = mediaserverstatus.getNumber ("mediaCount", (int) 0);
	mediaCountIcon->setText (StdString::createSprintf ("%i", count));
	mediaCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::MediaFile, UiTextString::MediaFiles));

	count = streamserverstatus.getNumber ("streamCount", (int) 0);
	streamCountIcon->setText (StdString::createSprintf ("%i", count));
	streamCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::VideoStream, UiTextString::VideoStreams));

	taskCountIcon->setText (StdString::createSprintf ("%i", agentTaskCount));
	taskCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (agentTaskCount, UiTextString::TaskInProgress, UiTextString::TasksInProgress));

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

	if (menuClickCallback.callback) {
		menuButton->isVisible = true;
	}
	refreshLayout ();
	Panel::syncRecordStore ();
}

void MediaLibraryWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
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
		storageIcon->isVisible = true;
		mediaCountIcon->isVisible = true;
		streamCountIcon->isVisible = true;
		catalogLinkIcon->isVisible = (! htmlCatalogPath.empty ());
	}
	else {
		setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		descriptionLabel->isVisible = false;
		dividerPanel->isVisible = false;
		taskCountIcon->isVisible = false;
		storageIcon->isVisible = false;
		mediaCountIcon->isVisible = false;
		streamCountIcon->isVisible = false;
		catalogLinkIcon->isVisible = false;
		agentTaskWindow->isVisible = false;
	}

	refreshLayout ();
}

void MediaLibraryWindow::refreshLayout () {
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
	if (menuButton->isVisible) {
		menuButton->flowRight (&x, y, &x2, &y2);
	}

	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
	x2 = 0.0f;
	if (dividerPanel->isVisible) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
	}
	if (catalogLinkIcon->isVisible) {
		catalogLinkIcon->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
	x2 = 0.0f;
	if (storageIcon->isVisible) {
		storageIcon->flowRight (&x, y, &x2, &y2);
	}
	if (mediaCountIcon->isVisible) {
		mediaCountIcon->flowRight (&x, y, &x2, &y2);
	}
	if (streamCountIcon->isVisible) {
		streamCountIcon->flowRight (&x, y, &x2, &y2);
	}
	if (taskCountIcon->isVisible) {
		taskCountIcon->flowRight (&x, y, &x2, &y2);
	}

	if (agentTaskWindow->isVisible) {
		x = x0;
		y = y2 + UiConfiguration::instance->marginSize;
		x2 = 0.0f;
		agentTaskWindow->flowDown (x, &y, &x2, &y2);
	}

	resetSize ();

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	}

	x = width - widthPadding;
	if (menuButton->isVisible) {
		menuButton->flowLeft (&x);
	}
	expandToggle->flowLeft (&x);
}

void MediaLibraryWindow::menuButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((MediaLibraryWindow *) windowPtr)->eventCallback (((MediaLibraryWindow *) windowPtr)->menuClickCallback);
}

void MediaLibraryWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MediaLibraryWindow *window;
	Toggle *toggle;

	window = (MediaLibraryWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	window->eventCallback (window->expandStateChangeCallback);
}

void MediaLibraryWindow::catalogLinkClicked (void *windowPtr, Widget *widgetPtr) {
	MediaLibraryWindow *window;

	window = (MediaLibraryWindow *) windowPtr;
	if (window->htmlCatalogPath.empty ()) {
		return;
	}

	OsUtil::openUrl (AgentControl::instance->getAgentSecondaryUrl (window->agentId, NULL, window->htmlCatalogPath));
}
