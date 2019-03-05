/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
*                 https://membranesoftware.com
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
#include "Button.h"
#include "TextField.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "Image.h"
#include "Json.h"
#include "SystemInterface.h"
#include "AgentConfigurationWindow.h"

const int AgentConfigurationWindow::mediaScanPeriods[] = { 0, 24 * 3600, 4 * 3600, 3600, 900, 60 };

AgentConfigurationWindow::AgentConfigurationWindow (const StdString &agentId)
: Panel ()
, agentId (agentId)
, isExpanded (false)
, iconImage (NULL)
, titleLabel (NULL)
, expandToggle (NULL)
, actionWindow (NULL)
, applyButton (NULL)
, expandStateChangeCallback (NULL)
, expandStateChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::CONFIGURATION_ICON)));
	titleLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::configuration).capitalized (), UiConfiguration::HEADLINE, uiconfig->primaryTextColor));

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::EXPAND_MORE_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::EXPAND_LESS_BUTTON)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (AgentConfigurationWindow::expandToggleStateChanged, this);
	expandToggle->setMouseHoverTooltip (uitext->getText (UiTextString::expandToggleTooltip));
	expandToggle->isVisible = false;

	applyButton = (Button *) addWidget (new Button (uitext->getText (UiTextString::apply).uppercased ()));
	applyButton->setMouseClickCallback (AgentConfigurationWindow::applyButtonClicked, this);
	applyButton->setTextColor (uiconfig->raisedButtonTextColor);
	applyButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	applyButton->isVisible = false;

	refreshLayout ();
}

AgentConfigurationWindow::~AgentConfigurationWindow () {

}

StdString AgentConfigurationWindow::toStringDetail () {
	return (StdString (" AgentConfigurationWindow"));
}

void AgentConfigurationWindow::setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	expandStateChangeCallback = callback;
	expandStateChangeCallbackData = callbackData;
}

void AgentConfigurationWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	isExpanded = expanded;
	if (isExpanded) {
		expandToggle->setChecked (true, true);
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
		if (actionWindow) {
			actionWindow->isVisible = false;
		}
		applyButton->isVisible = false;
	}

	refreshLayout ();
	if ((! shouldSkipStateChangeCallback) && expandStateChangeCallback) {
		expandStateChangeCallback (expandStateChangeCallbackData, this);
	}
}

void AgentConfigurationWindow::refreshLayout () {
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
	titleLabel->flowRight (&x, y, &x2, &y2);
	if (expandToggle->isVisible) {
		expandToggle->flowRight (&x, y, &x2, &y2);
	}
	titleLabel->position.assignY (y0 + ((y2 - y0) / 2.0f) - (titleLabel->height / 2.0f));

	x2 = 0.0f;
	x = x0;
	y = y2 + uiconfig->marginSize;
	if (actionWindow && actionWindow->isVisible) {
		actionWindow->flowDown (x, &y, &x2, &y2);
		y = y2 + uiconfig->marginSize;
	}
	if (applyButton->isVisible) {
		applyButton->flowDown (x, &y, &x2, &y2);
	}

	if (actionWindow) {
		expandToggle->position.assignX (x + actionWindow->width - expandToggle->width);
	}
	resetSize ();

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
	RecordStore *store;
	SystemInterface *interface;
	Json *record;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	record = store->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}

	agentPlatform = interface->getCommandStringParam (record, "platform", "");
	Panel::syncRecordStore ();
}

void AgentConfigurationWindow::loadConfiguration () {
	int result;

	if (isWaiting) {
		return;
	}

	retain ();
	result = App::instance->agentControl.invokeCommand (agentId, App::instance->createCommand (SystemInterface::Command_GetAgentConfiguration, SystemInterface::Constant_DefaultCommandType), AgentConfigurationWindow::invokeGetAgentConfigurationComplete, this);
	if (result != Result::SUCCESS) {
		release ();
		Log::debug ("Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", result, agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		return;
	}
	setWaiting (true);
}

void AgentConfigurationWindow::invokeGetAgentConfigurationComplete (void *windowPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	AgentConfigurationWindow *window;
	int result;

	window = (AgentConfigurationWindow *) windowPtr;
	if (window->isDestroyed) {
		window->release ();
		return;
	}

	if (invokeResult != Result::SUCCESS) {
		Log::debug ("Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", invokeResult, agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::getAgentConfigurationServerContactError));
	}
	else {
		result = window->populateConfiguration (responseCommand);
		if (result != Result::SUCCESS) {
			Log::debug ("Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", result, agentId.c_str ());
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		}
	}

	window->setWaiting (false);
	window->release ();
	App::instance->shouldRefreshUi = true;
}

int AgentConfigurationWindow::populateConfiguration (Json *command) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	TextField *textfield;
	Toggle *toggle;
	SliderWindow *slider;
	Json cfg;
	StdString prompt;
	int i, numperiods, period;

	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	if ((! command) || (interface->getCommandId (command) != SystemInterface::CommandId_AgentConfiguration)) {
		return (Result::ERROR_INVALID_PARAM);
	}

	agentConfiguration.copy (command);
	if (actionWindow) {
		actionWindow->isDestroyed = true;
	}
	actionWindow = (ActionWindow *) addWidget (new ActionWindow ());
	actionWindow->setPadding (0.0f, 0.0f);
	actionWindow->setFillBg (false);
	actionWindow->setButtonsVisible (false);
	actionWindow->setOptionChangeCallback (AgentConfigurationWindow::actionOptionChanged, this);

	toggle = new Toggle ();
	toggle->setChecked (interface->getCommandBooleanParam (&agentConfiguration, "isEnabled", false));
	actionWindow->addOption (uitext->getText (UiTextString::enabled).capitalized (), toggle, uitext->getText (UiTextString::agentEnabledDescription));

	textfield = new TextField (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CAPTION]->maxGlyphWidth, uitext->getText (UiTextString::agentDisplayNamePrompt));
	textfield->setPromptErrorColor (true);
	textfield->setValue (interface->getCommandStringParam (&agentConfiguration, "displayName", ""));
	actionWindow->addOption (uitext->getText (UiTextString::displayName).capitalized (), textfield, uitext->getText (UiTextString::agentDisplayNameDescription));
	actionWindow->setOptionNotEmptyString (uitext->getText (UiTextString::displayName).capitalized ());

	if (interface->getCommandObjectParam (&agentConfiguration, "mediaServerConfiguration", &cfg)) {
		if (interface->isWindowsPlatform (agentPlatform)) {
			prompt = uitext->getText (UiTextString::sourceMediaPathPromptWindows);
		}
		else {
			prompt = uitext->getText (UiTextString::sourceMediaPathPrompt);
		}
		textfield = new TextField (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CAPTION]->maxGlyphWidth, prompt);
		textfield->setPromptErrorColor (true);
		textfield->setValue (cfg.getString ("mediaPath", ""));
		actionWindow->addOption (uitext->getText (UiTextString::sourceMediaPath).capitalized (), textfield, uitext->getText (UiTextString::sourceMediaPathDescription));
		actionWindow->setOptionNotEmptyString (uitext->getText (UiTextString::sourceMediaPath).capitalized ());

		if (interface->isWindowsPlatform (agentPlatform)) {
			prompt = uitext->getText (UiTextString::mediaDataPathPromptWindows);
		}
		else {
			prompt = uitext->getText (UiTextString::mediaDataPathPrompt);
		}
		textfield = new TextField (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CAPTION]->maxGlyphWidth, prompt);
		textfield->setPromptErrorColor (true);
		textfield->setValue (cfg.getString ("dataPath", ""));
		actionWindow->addOption (uitext->getText (UiTextString::mediaDataPath).capitalized (), textfield, uitext->getText (UiTextString::mediaDataPathDescription));
		actionWindow->setOptionNotEmptyString (uitext->getText (UiTextString::mediaDataPath).capitalized ());

		slider = new SliderWindow (new Slider (0.0f, 5.0f));
		slider->setPadding (0.0f, 0.0f);
		slider->setValueNameFunction (AgentConfigurationWindow::mediaScanPeriodSliderValueName);
		numperiods = sizeof (AgentConfigurationWindow::mediaScanPeriods) / sizeof (AgentConfigurationWindow::mediaScanPeriods[0]);
		for (i = 0; i < numperiods; ++i) {
			slider->addSnapValue ((float) i);
		}

		period = cfg.getNumber ("scanPeriod", (int) 0);
		for (i = 0; i < numperiods; ++i) {
			if (period == AgentConfigurationWindow::mediaScanPeriods[i]) {
				slider->setValue ((float) i);
				break;
			}
		}
		actionWindow->addOption (uitext->getText (UiTextString::mediaScanPeriod).capitalized (), slider, uitext->getText (UiTextString::mediaScanPeriodDescription));
	}
	if (interface->getCommandObjectParam (&agentConfiguration, "streamServerConfiguration", &cfg)) {
		if (interface->isWindowsPlatform (agentPlatform)) {
			prompt = uitext->getText (UiTextString::streamDataPathPromptWindows);
		}
		else {
			prompt = uitext->getText (UiTextString::streamDataPathPrompt);
		}
		textfield = new TextField (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CAPTION]->maxGlyphWidth, prompt);
		textfield->setPromptErrorColor (true);
		textfield->setValue (cfg.getString ("dataPath", ""));
		actionWindow->addOption (uitext->getText (UiTextString::streamDataPath).capitalized (), textfield, uitext->getText (UiTextString::streamDataPathDescription));
		actionWindow->setOptionNotEmptyString (uitext->getText (UiTextString::streamDataPath).capitalized ());
	}

	if (! actionWindow->isOptionDataValid) {
		applyButton->setDisabled (true);
	}

	expandToggle->isVisible = true;
	if ((! interface->getCommandBooleanParam (&agentConfiguration, "isEnabled", false)) || (! actionWindow->isOptionDataValid)) {
		setExpanded (true);
	}
	else {
		setExpanded (false);
	}

	refreshLayout ();
	return (Result::SUCCESS);
}

StdString AgentConfigurationWindow::mediaScanPeriodSliderValueName (float sliderValue) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (uitext->getText (UiTextString::mediaScanPeriodNeverDescription).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::slowest).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 2.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::slow).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 3.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::medium).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 4.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::fast).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 5.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::fastest).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}

	return (StdString (""));
}

void AgentConfigurationWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	AgentConfigurationWindow *window;
	Toggle *toggle;

	window = (AgentConfigurationWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback) {
		window->expandStateChangeCallback (window->expandStateChangeCallbackData, window);
	}
}

void AgentConfigurationWindow::actionOptionChanged (void *windowPtr, Widget *widgetPtr) {
	AgentConfigurationWindow *window;
	ActionWindow *action;

	window = (AgentConfigurationWindow *) windowPtr;
	action = (ActionWindow *) widgetPtr;

	if (! action->isOptionDataValid) {
		window->applyButton->setDisabled (true);
	}
	else {
		window->applyButton->setDisabled (false);
	}
}

void AgentConfigurationWindow::applyButtonClicked (void *windowPtr, Widget *widgetPtr) {
	AgentConfigurationWindow *window;
	ActionWindow *action;
	SystemInterface *interface;
	UiText *uitext;
	Json *params, *agentconfig, *cfg;
	int result, numperiods, period, index;

	window = (AgentConfigurationWindow *) windowPtr;
	interface = &(App::instance->systemInterface);
	uitext = &(App::instance->uiText);
	action = window->actionWindow;

	if (window->isWaiting) {
		return;
	}

	params = new Json ();
	agentconfig = new Json ();
	agentconfig->set ("isEnabled", action->getBooleanValue (uitext->getText (UiTextString::enabled), true));
	agentconfig->set ("displayName", action->getStringValue (uitext->getText (UiTextString::displayName), ""));
	if (interface->getCommandObjectParam (&(window->agentConfiguration), "mediaServerConfiguration", NULL)) {
		cfg = new Json ();
		cfg->set ("mediaPath", action->getStringValue (uitext->getText (UiTextString::sourceMediaPath), ""));
		cfg->set ("dataPath", action->getStringValue (uitext->getText (UiTextString::mediaDataPath), ""));

		period = 0;
		numperiods = sizeof (AgentConfigurationWindow::mediaScanPeriods) / sizeof (AgentConfigurationWindow::mediaScanPeriods[0]);
		index = action->getNumberValue (uitext->getText (UiTextString::mediaScanPeriod), (int) -1);
		if ((index >= 0) && (index < numperiods)) {
			period = AgentConfigurationWindow::mediaScanPeriods[index];
		}
		cfg->set ("scanPeriod", period);
		agentconfig->set ("mediaServerConfiguration", cfg);
	}
	if (interface->getCommandObjectParam (&(window->agentConfiguration), "streamServerConfiguration", NULL)) {
		cfg = new Json ();
		cfg->set ("dataPath", action->getStringValue (uitext->getText (UiTextString::streamDataPath), ""));
		agentconfig->set ("streamServerConfiguration", cfg);
	}
	params->set ("agentConfiguration", agentconfig);

	window->retain ();
	result = App::instance->agentControl.invokeCommand (window->agentId, App::instance->createCommand (SystemInterface::Command_UpdateAgentConfiguration, SystemInterface::Constant_DefaultCommandType, params), AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete, window);
	if (result != Result::SUCCESS) {
		window->release ();
		Log::debug ("Failed to invoke UpdateAgentConfiguration command; err=%i agentId=\"%s\"", result, window->agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		return;
	}
	window->setWaiting (true);
}

void AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete (void *windowPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	AgentConfigurationWindow *window;
	SystemInterface *interface;

	window = (AgentConfigurationWindow *) windowPtr;
	if (window->isDestroyed) {
		window->release ();
		return;
	}

	interface = &(App::instance->systemInterface);
	if (invokeResult != Result::SUCCESS) {
		Log::debug ("Failed to invoke UpdateAgentConfiguration command; err=%i agentId=\"%s\"", invokeResult, agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::serverContactError));
	}
	else {
		if ((! responseCommand) || (interface->getCommandId (responseCommand) != SystemInterface::CommandId_AgentConfiguration)) {
			Log::debug ("Failed to invoke UpdateAgentConfiguration command; err=\"Invalid response data\" agentId=\"%s\"", agentId.c_str ());
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::internalError));
		}
		else {
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::updateAgentConfigurationMessage));
			App::instance->agentControl.refreshAgentStatus (window->agentId);
		}
	}

	window->setWaiting (false);
	window->release ();
}
