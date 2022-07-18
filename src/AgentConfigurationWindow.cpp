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
#include <math.h>
#include "App.h"
#include "Log.h"
#include "StdString.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "OsUtil.h"
#include "Json.h"
#include "SystemInterface.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Button.h"
#include "TextField.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "Image.h"
#include "TextFieldWindow.h"
#include "AgentConfigurationWindow.h"

const int AgentConfigurationWindow::MediaScanPeriods[] = { 0, 24 * 3600, 4 * 3600, 3600, 900, 60 };

AgentConfigurationWindow::AgentConfigurationWindow (const StdString &agentId)
: Panel ()
, agentId (agentId)
, isExpanded (false)
, iconImage (NULL)
, titleLabel (NULL)
, expandToggle (NULL)
, dividerPanel (NULL)
, actionWindow (NULL)
, applyButton (NULL)
{
	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ConfigurationIconSprite)));
	titleLabel = (Label *) addWidget (new Label (UiText::instance->getText (UiTextString::Configuration).capitalized (), UiConfiguration::HeadlineFont, UiConfiguration::instance->primaryTextColor));

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (AgentConfigurationWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());
	expandToggle->isVisible = false;

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	applyButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::OkButtonSprite)));
	applyButton->mouseClickCallback = Widget::EventCallbackContext (AgentConfigurationWindow::applyButtonClicked, this);
	applyButton->setTextColor (UiConfiguration::instance->raisedButtonTextColor);
	applyButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	applyButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Apply).capitalized ());
	applyButton->isVisible = false;

	refreshLayout ();
}

AgentConfigurationWindow::~AgentConfigurationWindow () {

}

StdString AgentConfigurationWindow::toStringDetail () {
	return (StdString (" AgentConfigurationWindow"));
}

void AgentConfigurationWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	isExpanded = expanded;
	if (isExpanded) {
		expandToggle->setChecked (true, true);
		dividerPanel->isVisible = true;
		if (actionWindow) {
			actionWindow->isVisible = true;
			applyButton->isVisible = true;
		}
		else {
			applyButton->isVisible = false;
		}
	}
	else {
		expandToggle->setChecked (false, true);
		dividerPanel->isVisible = false;
		if (actionWindow) {
			actionWindow->isVisible = false;
		}
		applyButton->isVisible = false;
	}

	refreshLayout ();
	if (! shouldSkipStateChangeCallback) {
		eventCallback (expandStateChangeCallback);
	}
}

void AgentConfigurationWindow::refreshLayout () {
	float x, y, x0, y0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	titleLabel->flowRight (&x, y, &x2, &y2);
	if (expandToggle->isVisible) {
		expandToggle->flowRight (&x, y, &x2, &y2);
	}
	titleLabel->position.assignY (y0 + ((y2 - y0) / 2.0f) - (titleLabel->height / 2.0f));

	x2 = 0.0f;
	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
	if (dividerPanel->isVisible) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
	}
	if (actionWindow && actionWindow->isVisible) {
		actionWindow->flowDown (x, &y, &x2, &y2);
		y = y2 + UiConfiguration::instance->marginSize;
	}
	if (applyButton->isVisible) {
		applyButton->flowDown (x, &y, &x2, &y2);
	}

	if (actionWindow) {
		expandToggle->position.assignX (x + actionWindow->width - expandToggle->width);
	}
	resetSize ();

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	}

	x = width - widthPadding;
	if (expandToggle->isVisible) {
		expandToggle->flowLeft (&x);
	}

	x = width - widthPadding;
	if (applyButton->isVisible) {
		applyButton->flowLeft (&x);
	}
}

void AgentConfigurationWindow::syncRecordStore () {
	Json *record;

	record = RecordStore::instance->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}
	agentPlatform = SystemInterface::instance->getCommandStringParam (record, "platform", "");
	Panel::syncRecordStore ();
}

void AgentConfigurationWindow::loadConfiguration () {
	int result;

	if (isWaiting) {
		return;
	}

	retain ();
	result = AgentControl::instance->invokeCommand (agentId, App::instance->createCommand (SystemInterface::Command_GetAgentConfiguration), CommandList::InvokeCallbackContext (AgentConfigurationWindow::invokeGetAgentConfigurationComplete, this));
	if (result != OsUtil::Success) {
		release ();
		Log::debug ("Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", result, agentId.c_str ());
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::InternalError));
		return;
	}
	setWaiting (true);
}

void AgentConfigurationWindow::invokeGetAgentConfigurationComplete (void *windowPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand, const StdString &invokeId) {
	AgentConfigurationWindow *window;
	int result;

	window = (AgentConfigurationWindow *) windowPtr;
	if (window->isDestroyed) {
		window->release ();
		return;
	}

	if (invokeResult != OsUtil::Success) {
		Log::debug ("Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", invokeResult, agentId.c_str ());
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::GetAgentConfigurationServerContactError));
	}
	else {
		result = window->populateConfiguration (responseCommand);
		if (result != OsUtil::Success) {
			Log::debug ("Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", result, agentId.c_str ());
			App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::InternalError));
		}
	}

	window->setWaiting (false);
	window->release ();
	App::instance->shouldRefreshUi = true;
}

int AgentConfigurationWindow::populateConfiguration (Json *command) {
	TextFieldWindow *textfield;
	Toggle *toggle;
	SliderWindow *slider;
	Json cfg;
	StdString prompt;
	int i, numperiods, period;

	if ((! command) || (SystemInterface::instance->getCommandId (command) != SystemInterface::CommandId_AgentConfiguration)) {
		return (OsUtil::InvalidParamError);
	}

	agentConfiguration.copyValue (command);
	if (actionWindow) {
		actionWindow->isDestroyed = true;
	}
	actionWindow = (ActionWindow *) addWidget (new ActionWindow ());
	actionWindow->setPadding (0.0f, 0.0f);
	actionWindow->setFillBg (false);
	actionWindow->setButtonsVisible (false);
	actionWindow->optionChangeCallback = Widget::EventCallbackContext (AgentConfigurationWindow::actionOptionChanged, this);

	toggle = new Toggle ();
	toggle->setChecked (SystemInterface::instance->getCommandBooleanParam (&agentConfiguration, "isEnabled", false));
	actionWindow->addOption (UiText::instance->getText (UiTextString::Enabled).capitalized (), toggle, UiText::instance->getText (UiTextString::AgentEnabledDescription));

	textfield = new TextFieldWindow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiText::instance->getText (UiTextString::AgentDisplayNamePrompt));
	textfield->setPadding (UiConfiguration::instance->paddingSize, 0.0f);
	textfield->setButtonsEnabled (false, false, true, true, false);
	textfield->setPromptErrorColor (true);
	textfield->setValue (SystemInterface::instance->getCommandStringParam (&agentConfiguration, "displayName", ""));
	actionWindow->addOption (UiText::instance->getText (UiTextString::DisplayName).capitalized (), textfield, UiText::instance->getText (UiTextString::AgentDisplayNameDescription));
	actionWindow->setOptionNotEmptyString (UiText::instance->getText (UiTextString::DisplayName).capitalized ());

	if (SystemInterface::instance->getCommandObjectParam (&agentConfiguration, "mediaServerConfiguration", &cfg)) {
		if (SystemInterface::instance->isWindowsPlatform (agentPlatform)) {
			prompt = UiText::instance->getText (UiTextString::SourceMediaPathPromptWindows);
		}
		else {
			prompt = UiText::instance->getText (UiTextString::SourceMediaPathPrompt);
		}
		textfield = new TextFieldWindow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, prompt);
		textfield->setPadding (UiConfiguration::instance->paddingSize, 0.0f);
		textfield->setButtonsEnabled (false, false, true, true, false);
		textfield->setPromptErrorColor (true);
		textfield->setValue (cfg.getString ("mediaPath", ""));
		actionWindow->addOption (UiText::instance->getText (UiTextString::SourceMediaPath).capitalized (), textfield, UiText::instance->getText (UiTextString::SourceMediaPathDescription));
		actionWindow->setOptionNotEmptyString (UiText::instance->getText (UiTextString::SourceMediaPath).capitalized ());

		if (SystemInterface::instance->isWindowsPlatform (agentPlatform)) {
			prompt = UiText::instance->getText (UiTextString::MediaDataPathPromptWindows);
		}
		else {
			prompt = UiText::instance->getText (UiTextString::MediaDataPathPrompt);
		}
		textfield = new TextFieldWindow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, prompt);
		textfield->setPadding (UiConfiguration::instance->paddingSize, 0.0f);
		textfield->setButtonsEnabled (false, false, true, true, false);
		textfield->setPromptErrorColor (true);
		textfield->setValue (cfg.getString ("dataPath", ""));
		actionWindow->addOption (UiText::instance->getText (UiTextString::MediaDataPath).capitalized (), textfield, UiText::instance->getText (UiTextString::MediaDataPathDescription));
		actionWindow->setOptionNotEmptyString (UiText::instance->getText (UiTextString::MediaDataPath).capitalized ());

		slider = new SliderWindow (new Slider (0.0f, 5.0f));
		slider->setPadding (0.0f, 0.0f);
		slider->setValueNameFunction (AgentConfigurationWindow::mediaScanPeriodSliderValueName);
		numperiods = sizeof (AgentConfigurationWindow::MediaScanPeriods) / sizeof (AgentConfigurationWindow::MediaScanPeriods[0]);
		for (i = 0; i < numperiods; ++i) {
			slider->addSnapValue ((float) i);
		}

		period = cfg.getNumber ("scanPeriod", (int) 0);
		for (i = 0; i < numperiods; ++i) {
			if (period == AgentConfigurationWindow::MediaScanPeriods[i]) {
				slider->setValue ((float) i);
				break;
			}
		}
		actionWindow->addOption (UiText::instance->getText (UiTextString::MediaScanPeriod).capitalized (), slider, UiText::instance->getText (UiTextString::MediaScanPeriodDescription));
	}
	else if (SystemInterface::instance->getCommandObjectParam (&agentConfiguration, "streamServerConfiguration", &cfg)) {
		if (SystemInterface::instance->isWindowsPlatform (agentPlatform)) {
			prompt = UiText::instance->getText (UiTextString::StreamDataPathPromptWindows);
		}
		else {
			prompt = UiText::instance->getText (UiTextString::StreamDataPathPrompt);
		}
		textfield = new TextFieldWindow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, prompt);
		textfield->setPadding (UiConfiguration::instance->paddingSize, 0.0f);
		textfield->setButtonsEnabled (false, false, true, true, false);
		textfield->setPromptErrorColor (true);
		textfield->setValue (cfg.getString ("dataPath", ""));
		actionWindow->addOption (UiText::instance->getText (UiTextString::StreamDataPath).capitalized (), textfield, UiText::instance->getText (UiTextString::StreamDataPathDescription));
		actionWindow->setOptionNotEmptyString (UiText::instance->getText (UiTextString::StreamDataPath).capitalized ());
	}

	if (! actionWindow->isOptionDataValid) {
		applyButton->setDisabled (true);
		applyButton->setMouseHoverTooltip (StdString::createSprintf ("%s %s", UiText::instance->getText (UiTextString::Apply).capitalized ().c_str (), UiText::instance->getText (UiTextString::ActionWindowInvalidDataTooltip).c_str ()));
	}

	expandToggle->isVisible = true;
	if ((! SystemInterface::instance->getCommandBooleanParam (&agentConfiguration, "isEnabled", false)) || (! actionWindow->isOptionDataValid)) {
		setExpanded (true, true);
	}
	else {
		setExpanded (false, true);
	}

	refreshLayout ();
	return (OsUtil::Success);
}

StdString AgentConfigurationWindow::mediaScanPeriodSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (UiText::instance->getText (UiTextString::MediaScanPeriodNeverDescription).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Slowest).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 2.0f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Slow).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 3.0f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Medium).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 4.0f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Fast).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 5.0f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Fastest).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}

	return (StdString (""));
}

void AgentConfigurationWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	AgentConfigurationWindow *window;
	Toggle *toggle;

	window = (AgentConfigurationWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	window->eventCallback (window->expandStateChangeCallback);
}

void AgentConfigurationWindow::actionOptionChanged (void *windowPtr, Widget *widgetPtr) {
	AgentConfigurationWindow *window;
	ActionWindow *action;

	window = (AgentConfigurationWindow *) windowPtr;
	action = (ActionWindow *) widgetPtr;
	if (! action->isOptionDataValid) {
		window->applyButton->setDisabled (true);
		window->applyButton->setMouseHoverTooltip (StdString::createSprintf ("%s %s", UiText::instance->getText (UiTextString::Apply).capitalized ().c_str (), UiText::instance->getText (UiTextString::ActionWindowInvalidDataTooltip).c_str ()));
	}
	else {
		window->applyButton->setDisabled (false);
		window->applyButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Apply).capitalized ());
	}
}

void AgentConfigurationWindow::applyButtonClicked (void *windowPtr, Widget *widgetPtr) {
	AgentConfigurationWindow *window;
	ActionWindow *action;
	Json *params, *agentconfig, *cfg;
	int result, numperiods, period, index;

	window = (AgentConfigurationWindow *) windowPtr;
	action = window->actionWindow;
	if (window->isWaiting || (! action->isOptionDataValid)) {
		return;
	}

	params = new Json ();
	agentconfig = new Json ();
	agentconfig->set ("isEnabled", action->getBooleanValue (UiText::instance->getText (UiTextString::Enabled), true));
	agentconfig->set ("displayName", action->getStringValue (UiText::instance->getText (UiTextString::DisplayName), ""));
	if (SystemInterface::instance->getCommandObjectParam (&(window->agentConfiguration), "mediaServerConfiguration", NULL)) {
		cfg = new Json ();
		cfg->set ("mediaPath", action->getStringValue (UiText::instance->getText (UiTextString::SourceMediaPath), ""));
		cfg->set ("dataPath", action->getStringValue (UiText::instance->getText (UiTextString::MediaDataPath), ""));

		period = 0;
		numperiods = sizeof (AgentConfigurationWindow::MediaScanPeriods) / sizeof (AgentConfigurationWindow::MediaScanPeriods[0]);
		index = action->getNumberValue (UiText::instance->getText (UiTextString::MediaScanPeriod), (int) -1);
		if ((index >= 0) && (index < numperiods)) {
			period = AgentConfigurationWindow::MediaScanPeriods[index];
		}
		cfg->set ("scanPeriod", period);
		agentconfig->set ("mediaServerConfiguration", cfg);

		cfg = new Json ();
		cfg->set ("dataPath", action->getStringValue (UiText::instance->getText (UiTextString::MediaDataPath), ""));
		agentconfig->set ("streamServerConfiguration", cfg);
	}
	else if (SystemInterface::instance->getCommandObjectParam (&(window->agentConfiguration), "streamServerConfiguration", NULL)) {
		cfg = new Json ();
		cfg->set ("dataPath", action->getStringValue (UiText::instance->getText (UiTextString::StreamDataPath), ""));
		agentconfig->set ("streamServerConfiguration", cfg);
	}
	params->set ("agentConfiguration", agentconfig);

	window->retain ();
	result = AgentControl::instance->invokeCommand (window->agentId, App::instance->createCommand (SystemInterface::Command_UpdateAgentConfiguration, params), CommandList::InvokeCallbackContext (AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete, window));
	if (result != OsUtil::Success) {
		window->release ();
		Log::debug ("Failed to invoke UpdateAgentConfiguration command; err=%i agentId=\"%s\"", result, window->agentId.c_str ());
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::InternalError));
		return;
	}
	window->setWaiting (true);
}

void AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete (void *windowPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand, const StdString &invokeId) {
	AgentConfigurationWindow *window;

	window = (AgentConfigurationWindow *) windowPtr;
	if (window->isDestroyed) {
		window->release ();
		return;
	}

	if (invokeResult != OsUtil::Success) {
		Log::debug ("Failed to invoke UpdateAgentConfiguration command; err=%i agentId=\"%s\"", invokeResult, agentId.c_str ());
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::ServerContactError));
	}
	else {
		if ((! responseCommand) || (SystemInterface::instance->getCommandId (responseCommand) != SystemInterface::CommandId_AgentConfiguration)) {
			Log::debug ("Failed to invoke UpdateAgentConfiguration command; err=\"Invalid response data\" agentId=\"%s\"", agentId.c_str ());
			App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::InternalError));
		}
		else {
			App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::UpdateAgentConfigurationMessage));
			AgentControl::instance->refreshAgentStatus (window->agentId);
		}
	}

	window->setWaiting (false);
	window->release ();
}
