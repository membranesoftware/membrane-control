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
#include <vector>
#include "Result.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Button.h"
#include "TextArea.h"
#include "Toggle.h"
#include "Image.h"
#include "Json.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "MainUi.h"
#include "WebKioskUi.h"
#include "IconLabelWindow.h"
#include "UiLaunchWindow.h"

UiLaunchWindow::UiLaunchWindow (int uiType, SpriteGroup *mainUiSpriteGroup)
: Panel ()
, uiType (uiType)
, isExpanded (false)
, sprites (mainUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionText (NULL)
, expandToggle (NULL)
, openButton (NULL)
, expandStateChangeCallback (NULL)
, expandStateChangeCallbackData (NULL)
, openCallback (NULL)
, openCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;
	Sprite *iconsprite;
	IconLabelWindow *icon;
	StdString name, text;

	classId = ClassId::UiLaunchWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
	setCornerRadius (uiconfig->cornerRadius);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconsprite = NULL;
	switch (uiType) {
		case UiLaunchWindow::ServerUi: {
			name.assign (uitext->getText (UiTextString::servers).capitalized ());
			text.assign (uitext->getText (UiTextString::serverUiDescription));
			iconsprite = uiconfig->coreSprites.getSprite (UiConfiguration::LargeServerIconSprite);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ConnectionIconSprite), StdString ("0"), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->setPadding (0.0f, 0.0f);
			icon->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::serverConnected, UiTextString::serversConnected));
			countIcons.push_back (icon);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ConnectionIconSprite), uitext->getCountText (0, UiTextString::serverConnected, UiTextString::serversConnected), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->isVisible = false;
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::MediaUi: {
			name.assign (uitext->getText (UiTextString::media).capitalized ());
			text.assign (uitext->getText (UiTextString::mediaUiDescription));
			iconsprite = uiconfig->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ConnectionIconSprite), StdString ("0"), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->setPadding (0.0f, 0.0f);
			icon->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::mediaServerConnected, UiTextString::mediaServersConnected));
			countIcons.push_back (icon);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ConnectionIconSprite), uitext->getCountText (0, UiTextString::mediaServerConnected, UiTextString::mediaServersConnected), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->isVisible = false;
			noteIcons.push_back (icon);
			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallMediaIconSprite), uitext->getCountText (0, UiTextString::videoFileInCatalog, UiTextString::videoFilesInCatalog), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->isVisible = false;
			noteIcons.push_back (icon);
			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), uitext->getCountText (0, UiTextString::videoStreamPlayable, UiTextString::videoStreamsPlayable), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->isVisible = false;
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::WebKioskUi: {
			name.assign (uitext->getText (UiTextString::webKiosk).capitalized ());
			text.assign (uitext->getText (UiTextString::webKioskUiDescription));
			iconsprite = uiconfig->coreSprites.getSprite (UiConfiguration::LargeDisplayIconSprite);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ConnectionIconSprite), StdString ("0"), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->setPadding (0.0f, 0.0f);
			icon->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::monitorConnected, UiTextString::monitorsConnected));
			countIcons.push_back (icon);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ConnectionIconSprite), uitext->getCountText (0, UiTextString::monitorConnected, UiTextString::monitorsConnected), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->isVisible = false;
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::CameraUi: {
			name.assign (uitext->getText (UiTextString::cameras).capitalized ());
			text.assign (uitext->getText (UiTextString::cameraUiDescription));
			iconsprite = uiconfig->coreSprites.getSprite (UiConfiguration::LargeCameraIconSprite);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ConnectionIconSprite), StdString ("0"), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->setPadding (0.0f, 0.0f);
			icon->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::cameraConnected, UiTextString::camerasConnected));
			countIcons.push_back (icon);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ConnectionIconSprite), uitext->getCountText (0, UiTextString::cameraConnected, UiTextString::camerasConnected), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->isVisible = false;
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::CommandUi: {
			name.assign (uitext->getText (UiTextString::commands).capitalized ());
			text.assign (uitext->getText (UiTextString::commandUiDescription));
			iconsprite = uiconfig->coreSprites.getSprite (UiConfiguration::LargeCommandIconSprite);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallCommandIconSprite), StdString ("0"), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->setPadding (0.0f, 0.0f);
			icon->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::storedCommand, UiTextString::storedCommands));
			countIcons.push_back (icon);

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallCommandIconSprite), uitext->getCountText (0, UiTextString::recentCommand, UiTextString::recentCommands), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->isVisible = false;
			noteIcons.push_back (icon);
			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallCommandIconSprite), uitext->getCountText (0, UiTextString::storedCommand, UiTextString::storedCommands), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
			icon->isVisible = false;
			noteIcons.push_back (icon);
			break;
		}
	}

	if (! iconsprite) {
		iconsprite = sprites->getSprite (MainUi::UiIconSprite);
	}
	iconImage = (Image *) addWidget (new Image (iconsprite));
	nameLabel = (Label *) addWidget (new Label (name, UiConfiguration::BodyFont, uiconfig->primaryTextColor));

	descriptionText = (TextArea *) addWidget (new TextArea (UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	descriptionText->setText (text);
	descriptionText->isVisible = false;

	openButton = (Button *) addWidget (new Button (sprites->getSprite (MainUi::OpenButtonSprite)));
	openButton->zLevel = 1;
	openButton->setMouseClickCallback (UiLaunchWindow::openButtonClicked, this);
	openButton->setImageColor (uiconfig->flatButtonTextColor);
	openButton->setMouseHoverTooltip (uitext->getText (UiTextString::uiLaunchOpenButtonTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (UiLaunchWindow::expandToggleStateChanged, this);
	expandToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::expand).capitalized (), uitext->getText (UiTextString::minimize).capitalized ());

	refreshLayout ();
}

UiLaunchWindow::~UiLaunchWindow () {

}

StdString UiLaunchWindow::toStringDetail () {
  return (StdString (" UiLaunchWindow"));
}

bool UiLaunchWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::UiLaunchWindow));
}

UiLaunchWindow *UiLaunchWindow::castWidget (Widget *widget) {
	return (UiLaunchWindow::isWidgetType (widget) ? (UiLaunchWindow *) widget : NULL);
}

void UiLaunchWindow::setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	expandStateChangeCallback = callback;
	expandStateChangeCallbackData = callbackData;
}

void UiLaunchWindow::setOpenCallback (Widget::EventCallback callback, void *callbackData) {
	openCallback = callback;
	openCallbackData = callbackData;
}

void UiLaunchWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	UiConfiguration *uiconfig;
	std::vector<IconLabelWindow *>::iterator i, end;

	if (expanded == isExpanded) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
		expandToggle->setChecked (true, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
		descriptionText->isVisible = true;
	}
	else {
		setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		descriptionText->isVisible = false;
	}

	i = countIcons.begin ();
	end = countIcons.end ();
	while (i != end) {
		(*i)->isVisible = (! isExpanded);
		++i;
	}

	i = noteIcons.begin ();
	end = noteIcons.end ();
	while (i != end) {
		(*i)->isVisible = isExpanded;
		++i;
	}

	refreshLayout ();
}

void UiLaunchWindow::syncRecordStore () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	RecordStore *store;
	IconLabelWindow *icon;
	int count;
	int64_t maxage;

	uitext = &(App::instance->uiText);
	uiconfig = &(App::instance->uiConfig);
	store = &(App::instance->agentControl.recordStore);

	maxage = App::instance->prefsMap.find (App::ServerTimeoutKey, App::defaultServerTimeout) * 1000;
	switch (uiType) {
		case UiLaunchWindow::ServerUi: {
			count = store->countCommandRecords (SystemInterface::CommandId_AgentStatus, maxage);
			icon = countIcons.at (0);
			icon->setText (StdString::createSprintf ("%i", count));
			icon->setTextColor ((count > 0) ? uiconfig->lightPrimaryTextColor : uiconfig->errorTextColor);
			icon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::serverConnected, UiTextString::serversConnected));
			icon = noteIcons.at (0);
			icon->setText (uitext->getCountText (count, UiTextString::serverConnected, UiTextString::serversConnected));
			icon->setTextColor ((count > 0) ? uiconfig->lightPrimaryTextColor : uiconfig->errorTextColor);
			break;
		}
		case UiLaunchWindow::MediaUi: {
			count = store->countAgentRecords ("mediaServerStatus", maxage);
			icon = countIcons.at (0);
			icon->setText (StdString::createSprintf ("%i", count));
			icon->setTextColor ((count > 0) ? uiconfig->lightPrimaryTextColor : uiconfig->errorTextColor);
			icon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::mediaServerConnected, UiTextString::mediaServersConnected));
			icon = noteIcons.at (0);
			icon->setText (uitext->getCountText (count, UiTextString::mediaServerConnected, UiTextString::mediaServersConnected));
			icon->setTextColor ((count > 0) ? uiconfig->lightPrimaryTextColor : uiconfig->errorTextColor);

			count = countMediaItems (store);
			icon = noteIcons.at (1);
			icon->setText (uitext->getCountText (count, UiTextString::videoFileInCatalog, UiTextString::videoFilesInCatalog));

			count = countStreamItems (store);
			icon = noteIcons.at (2);
			icon->setText (uitext->getCountText (count, UiTextString::videoStreamPlayable, UiTextString::videoStreamsPlayable));
			break;
		}
		case UiLaunchWindow::WebKioskUi: {
			count = store->countAgentRecords ("monitorServerStatus", maxage);
			icon = countIcons.at (0);
			icon->setText (StdString::createSprintf ("%i", count));
			icon->setTextColor ((count > 0) ? uiconfig->lightPrimaryTextColor : uiconfig->errorTextColor);
			icon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::monitorConnected, UiTextString::monitorsConnected));
			icon = noteIcons.at (0);
			icon->setText (uitext->getCountText (count, UiTextString::monitorConnected, UiTextString::monitorsConnected));
			icon->setTextColor ((count > 0) ? uiconfig->lightPrimaryTextColor : uiconfig->errorTextColor);
			break;
		}
		case UiLaunchWindow::CameraUi: {
			count = store->countAgentRecords ("cameraServerStatus", maxage);
			icon = countIcons.at (0);
			icon->setText (StdString::createSprintf ("%i", count));
			icon->setTextColor ((count > 0) ? uiconfig->lightPrimaryTextColor : uiconfig->errorTextColor);
			icon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::cameraConnected, UiTextString::camerasConnected));
			icon = noteIcons.at (0);
			icon->setText (uitext->getCountText (count, UiTextString::cameraConnected, UiTextString::camerasConnected));
			icon->setTextColor ((count > 0) ? uiconfig->lightPrimaryTextColor : uiconfig->errorTextColor);
			break;
		}
		case UiLaunchWindow::CommandUi: {
			count = App::instance->agentControl.commandStore.getRecentCommandCount ();
			icon = noteIcons.at (0);
			icon->setText (uitext->getCountText (count, UiTextString::recentCommand, UiTextString::recentCommands));

			count = App::instance->agentControl.commandStore.getStoredCommandCount ();
			icon = countIcons.at (0);
			icon->setText (StdString::createSprintf ("%i", count));
			icon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::storedCommand, UiTextString::storedCommands));
			icon = noteIcons.at (1);
			icon->setText (uitext->getCountText (count, UiTextString::storedCommand, UiTextString::storedCommands));
			break;
		}
	}

	Panel::syncRecordStore ();
}

int UiLaunchWindow::countMediaItems (RecordStore *store) {
	int sum;

	sum = 0;
	store->processAgentRecords ("mediaServerStatus", UiLaunchWindow::addMediaCount, &sum);
	return (sum);
}

void UiLaunchWindow::addMediaCount (void *sumPtr, Json *record, const StdString &recordId) {
	SystemInterface *interface;
	Json serverstatus;
	int *sum;

	sum = (int *) sumPtr;
	interface = &(App::instance->systemInterface);
	if (interface->getCommandObjectParam (record, "mediaServerStatus", &serverstatus)) {
		*sum += serverstatus.getNumber ("mediaCount", 0);
	}
}

int UiLaunchWindow::countStreamItems (RecordStore *store) {
	int sum;

	sum = 0;
	store->processAgentRecords ("streamServerStatus", UiLaunchWindow::addStreamCount, &sum);
	return (sum);
}

void UiLaunchWindow::addStreamCount (void *sumPtr, Json *record, const StdString &recordId) {
	SystemInterface *interface;
	Json serverstatus;
	int *sum;

	sum = (int *) sumPtr;
	interface = &(App::instance->systemInterface);
	if (interface->getCommandObjectParam (record, "streamServerStatus", &serverstatus)) {
		*sum += serverstatus.getNumber ("streamCount", 0);
	}
}

void UiLaunchWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	IconLabelWindow *icon;
	std::vector<IconLabelWindow *>::iterator i, end;
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

	y = heightPadding;
	x = x2 + uiconfig->marginSize;
	expandToggle->flowRight (&x, y, &x2, &y2);
	if (! isExpanded) {
		openButton->flowRight (&x, y, &x2, &y2);
	}

	if (! isExpanded) {
		x = nameLabel->position.x;
		y = nameLabel->position.y + nameLabel->height + uiconfig->marginSize;
		x2 = 0.0f;
		y2 = 0.0f;
		i = countIcons.begin ();
		end = countIcons.end ();
		while (i != end) {
			icon = *i;
			icon->flowRight (&x, y, &x2, &y2);
			++i;
		}
	}
	else {
		x = x0;
		y = y2 + uiconfig->marginSize;
		descriptionText->flowDown (x, &y, &x2, &y2);

		x2 = 0.0f;
		y2 = 0.0f;
		i = noteIcons.begin ();
		end = noteIcons.end ();
		while (i != end) {
			icon = *i;
			icon->flowDown (x, &y, &x2, &y2);
			++i;
		}

		openButton->flowDown (x, &y, &x2, &y2);
	}

	resetSize ();

	if (isExpanded) {
		x = width - widthPadding;
		expandToggle->flowLeft (&x);
		x = width - widthPadding;
		openButton->flowLeft (&x);
	}
	else {
		x = width - widthPadding;
		openButton->flowLeft (&x);
		expandToggle->flowLeft (&x);
	}
}

void UiLaunchWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	UiLaunchWindow *window;
	Toggle *toggle;

	window = (UiLaunchWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback) {
		window->expandStateChangeCallback (window->expandStateChangeCallbackData, window);
	}
}

void UiLaunchWindow::openButtonClicked (void *windowPtr, Widget *widgetPtr) {
	UiLaunchWindow *window;

	window = (UiLaunchWindow *) windowPtr;
	if (window->openCallback) {
		window->openCallback (window->openCallbackData, window);
	}
}
