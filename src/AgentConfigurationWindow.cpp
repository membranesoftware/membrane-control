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
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Util.h"
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
, iconImage (NULL)
, titleLabel (NULL)
, actionWindow (NULL)
, applyButton (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	populate ();
	refreshLayout ();
}

AgentConfigurationWindow::~AgentConfigurationWindow () {

}

StdString AgentConfigurationWindow::toStringDetail () {
	return (StdString (" AgentConfigurationWindow"));
}

void AgentConfigurationWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::CONFIGURATION_ICON)));
	titleLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::configuration).capitalized (), UiConfiguration::HEADLINE, uiconfig->primaryTextColor));

	applyButton = (Button *) addWidget (new Button (uitext->getText (UiTextString::apply).uppercased ()));
	applyButton->setMouseClickCallback (AgentConfigurationWindow::applyButtonClicked, this);
	applyButton->setTextColor (uiconfig->raisedButtonTextColor);
	applyButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	applyButton->isVisible = false;
}

void AgentConfigurationWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, x2, y2;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	titleLabel->flowDown (x, &y, &x2, &y2);

	x2 = 0.0f;
	x = x0;
	y = y2 + uiconfig->marginSize;
	if (actionWindow) {
		actionWindow->flowDown (x, &y, &x2, &y2);
		y = y2 + uiconfig->marginSize;
	}
	if (applyButton->isVisible) {
		applyButton->flowDown (x, &y, &x2, &y2);
	}

	resetSize ();

	x = width - widthPadding;
	if (applyButton->isVisible) {
		applyButton->flowLeft (&x);
	}
}

void AgentConfigurationWindow::syncRecordStore (RecordStore *store) {
	SystemInterface *interface;
	Json *record;

	interface = &(App::getInstance ()->systemInterface);
	record = store->findRecord (agentId, SystemInterface::Command_AgentStatus);
	if (! record) {
		return;
	}

	agentPlatform = interface->getCommandStringParam (record, "platform", "");
	Panel::syncRecordStore (store);
}

void AgentConfigurationWindow::loadConfiguration () {
	App *app;
	int result;

	if (isWaiting) {
		return;
	}

	app = App::getInstance ();
	retain ();
	result = app->agentControl.invokeCommand (agentId, app->createCommand ("GetAgentConfiguration", SystemInterface::Constant_DefaultCommandType), AgentConfigurationWindow::invokeGetAgentConfigurationComplete, this);
	if (result != Result::SUCCESS) {
		release ();
		Log::write (Log::DEBUG, "Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", result, agentId.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
		return;
	}
	setWaiting (true);
}

void AgentConfigurationWindow::invokeGetAgentConfigurationComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	App *app;
	AgentConfigurationWindow *window;
	int result;

	app = App::getInstance ();
	window = (AgentConfigurationWindow *) windowPtr;
	if (window->isDestroyed) {
		window->release ();
		return;
	}

	if (jobResult != Result::SUCCESS) {
		Log::write (Log::DEBUG, "Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", jobResult, agentId.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::getAgentConfigurationServerContactError));
	}
	else {
		result = window->populateConfiguration (responseCommand);
		if (result != Result::SUCCESS) {
			Log::write (Log::DEBUG, "Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", result, agentId.c_str ());
			app->showSnackbar (app->uiText.getText (UiTextString::internalError));
		}
	}

	window->setWaiting (false);
	window->release ();
	app->shouldRefreshUi = true;
}

int AgentConfigurationWindow::populateConfiguration (Json *command) {
	App *app;
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	TextField *textfield;
	Toggle *toggle;
	SliderWindow *slider;
	Json cfg;
	StdString prompt;
	int result, i, numperiods, period;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	if ((! command) || (interface->getCommandId (command) != SystemInterface::Command_AgentConfiguration)) {
		return (Result::ERROR_INVALID_PARAM);
	}

	result = agentConfiguration.copy (command);
	if (result != Result::SUCCESS) {
		return (result);
	}

	if (actionWindow) {
		actionWindow->isDestroyed = true;
	}
	actionWindow = (ActionWindow *) addWidget (new ActionWindow ());
	actionWindow->setPadding (0.0f, 0.0f);
	actionWindow->setFillBg (false);
	actionWindow->setButtonsVisible (false);
	actionWindow->setOptionChangeCallback (AgentConfigurationWindow::actionOptionChanged, this);

	toggle = new Toggle ();
	toggle->setChecked (interface->getCommandBooleanParam (&agentConfiguration, "isEnabled", true));
	actionWindow->addOption (uitext->getText (UiTextString::enabled).capitalized (), toggle, uitext->getText (UiTextString::agentEnabledDescription));

	textfield = new TextField (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CAPTION]->maxGlyphWidth, uitext->getText (UiTextString::agentDisplayNamePrompt));
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
		textfield->setValue (cfg.getString ("dataPath", ""));
		actionWindow->addOption (uitext->getText (UiTextString::streamDataPath).capitalized (), textfield, uitext->getText (UiTextString::streamDataPathDescription));
		actionWindow->setOptionNotEmptyString (uitext->getText (UiTextString::streamDataPath).capitalized ());
	}

	if (! actionWindow->isOptionDataValid) {
		applyButton->setDisabled (true);
	}
	applyButton->isVisible = true;
	refreshLayout ();
	return (Result::SUCCESS);
}

StdString AgentConfigurationWindow::mediaScanPeriodSliderValueName (float sliderValue) {
	UiText *uitext;

	uitext = &(App::getInstance ()->uiText);
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (uitext->getText (UiTextString::mediaScanPeriodNeverDescription).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::slowest).capitalized ().c_str (), Util::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 2.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::slow).capitalized ().c_str (), Util::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 3.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::medium).capitalized ().c_str (), Util::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 4.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::fast).capitalized ().c_str (), Util::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 5.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::fastest).capitalized ().c_str (), Util::getDurationDisplayString (AgentConfigurationWindow::mediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}

	return (StdString (""));
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
	App *app;
	SystemInterface *interface;
	UiText *uitext;
	Json *params, *agentconfig, *cfg;
	int result, numperiods, period, index;

	window = (AgentConfigurationWindow *) windowPtr;
	app = App::getInstance ();
	interface = &(app->systemInterface);
	uitext = &(app->uiText);
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
	result = app->agentControl.invokeCommand (window->agentId, app->createCommand ("UpdateAgentConfiguration", SystemInterface::Constant_DefaultCommandType, params), AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete, window);
	if (result != Result::SUCCESS) {
		window->release ();
		Log::write (Log::DEBUG, "Failed to invoke UpdateAgentConfiguration command; err=%i agentId=\"%s\"", result, window->agentId.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::internalError));
		return;
	}
	window->setWaiting (true);
}

void AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	AgentConfigurationWindow *window;
	App *app;
	SystemInterface *interface;

	window = (AgentConfigurationWindow *) windowPtr;
	if (window->isDestroyed) {
		window->release ();
		return;
	}

	app = App::getInstance ();
	interface = &(app->systemInterface);
	if (jobResult != Result::SUCCESS) {
		Log::write (Log::DEBUG, "Failed to invoke UpdateAgentConfiguration command; err=%i agentId=\"%s\"", jobResult, agentId.c_str ());
		app->showSnackbar (app->uiText.getText (UiTextString::serverContactError));
	}
	else {
		if ((! responseCommand) || (interface->getCommandId (responseCommand) != SystemInterface::Command_AgentConfiguration)) {
			Log::write (Log::DEBUG, "Failed to invoke UpdateAgentConfiguration command; err=\"Invalid response data\" agentId=\"%s\"", agentId.c_str ());
			app->showSnackbar (app->uiText.getText (UiTextString::internalError));
		}
		else {
			app->showSnackbar (app->uiText.getText (UiTextString::updateAgentConfigurationMessage));
			app->agentControl.refreshAgentStatus (window->agentId);
		}
	}

	window->setWaiting (false);
	window->release ();
}
