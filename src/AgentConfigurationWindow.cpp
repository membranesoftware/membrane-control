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
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "OsUtil.h"
#include "Json.h"
#include "SystemInterface.h"
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
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::ConfigurationIconSprite)));
	titleLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::Configuration).capitalized (), UiConfiguration::HeadlineFont, uiconfig->primaryTextColor));

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (AgentConfigurationWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::Expand).capitalized (), uitext->getText (UiTextString::Minimize).capitalized ());
	expandToggle->isVisible = false;

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, uiconfig->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, uiconfig->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	applyButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::OkButtonSprite)));
	applyButton->mouseClickCallback = Widget::EventCallbackContext (AgentConfigurationWindow::applyButtonClicked, this);
	applyButton->setTextColor (uiconfig->raisedButtonTextColor);
	applyButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	applyButton->setMouseHoverTooltip (uitext->getText (UiTextString::Apply).capitalized ());
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
	if ((! shouldSkipStateChangeCallback) && expandStateChangeCallback.callback) {
		expandStateChangeCallback.callback (expandStateChangeCallback.callbackData, this);
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
	if (dividerPanel->isVisible) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
	}
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

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, uiconfig->headlineDividerLineWidth);
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
	if (result != Result::Success) {
		release ();
		Log::debug ("Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", result, agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
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

	if (invokeResult != Result::Success) {
		Log::debug ("Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", invokeResult, agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::GetAgentConfigurationServerContactError));
	}
	else {
		result = window->populateConfiguration (responseCommand);
		if (result != Result::Success) {
			Log::debug ("Failed to invoke GetAgentConfiguration command; err=%i agentId=\"%s\"", result, agentId.c_str ());
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
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
	TextFieldWindow *textfield;
	Toggle *toggle;
	SliderWindow *slider;
	Json cfg;
	StdString prompt;
	int i, numperiods, period;

	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	if ((! command) || (interface->getCommandId (command) != SystemInterface::CommandId_AgentConfiguration)) {
		return (Result::InvalidParamError);
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
	toggle->setChecked (interface->getCommandBooleanParam (&agentConfiguration, "isEnabled", false));
	actionWindow->addOption (uitext->getText (UiTextString::Enabled).capitalized (), toggle, uitext->getText (UiTextString::AgentEnabledDescription));

	textfield = new TextFieldWindow (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, uitext->getText (UiTextString::AgentDisplayNamePrompt));
	textfield->setPadding (uiconfig->paddingSize, 0.0f);
	textfield->setButtonsEnabled (false, false, true, true, false);
	textfield->setPromptErrorColor (true);
	textfield->setValue (interface->getCommandStringParam (&agentConfiguration, "displayName", ""));
	actionWindow->addOption (uitext->getText (UiTextString::DisplayName).capitalized (), textfield, uitext->getText (UiTextString::AgentDisplayNameDescription));
	actionWindow->setOptionNotEmptyString (uitext->getText (UiTextString::DisplayName).capitalized ());

	if (interface->getCommandObjectParam (&agentConfiguration, "mediaServerConfiguration", &cfg)) {
		if (interface->isWindowsPlatform (agentPlatform)) {
			prompt = uitext->getText (UiTextString::SourceMediaPathPromptWindows);
		}
		else {
			prompt = uitext->getText (UiTextString::SourceMediaPathPrompt);
		}
		textfield = new TextFieldWindow (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, prompt);
		textfield->setPadding (uiconfig->paddingSize, 0.0f);
		textfield->setButtonsEnabled (false, false, true, true, false);
		textfield->setPromptErrorColor (true);
		textfield->setValue (cfg.getString ("mediaPath", ""));
		actionWindow->addOption (uitext->getText (UiTextString::SourceMediaPath).capitalized (), textfield, uitext->getText (UiTextString::SourceMediaPathDescription));
		actionWindow->setOptionNotEmptyString (uitext->getText (UiTextString::SourceMediaPath).capitalized ());

		if (interface->isWindowsPlatform (agentPlatform)) {
			prompt = uitext->getText (UiTextString::MediaDataPathPromptWindows);
		}
		else {
			prompt = uitext->getText (UiTextString::MediaDataPathPrompt);
		}
		textfield = new TextFieldWindow (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, prompt);
		textfield->setPadding (uiconfig->paddingSize, 0.0f);
		textfield->setButtonsEnabled (false, false, true, true, false);
		textfield->setPromptErrorColor (true);
		textfield->setValue (cfg.getString ("dataPath", ""));
		actionWindow->addOption (uitext->getText (UiTextString::MediaDataPath).capitalized (), textfield, uitext->getText (UiTextString::MediaDataPathDescription));
		actionWindow->setOptionNotEmptyString (uitext->getText (UiTextString::MediaDataPath).capitalized ());

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
		actionWindow->addOption (uitext->getText (UiTextString::MediaScanPeriod).capitalized (), slider, uitext->getText (UiTextString::MediaScanPeriodDescription));
	}
	else if (interface->getCommandObjectParam (&agentConfiguration, "streamServerConfiguration", &cfg)) {
		if (interface->isWindowsPlatform (agentPlatform)) {
			prompt = uitext->getText (UiTextString::StreamDataPathPromptWindows);
		}
		else {
			prompt = uitext->getText (UiTextString::StreamDataPathPrompt);
		}
		textfield = new TextFieldWindow (uiconfig->textFieldMediumLineLength * uiconfig->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, prompt);
		textfield->setPadding (uiconfig->paddingSize, 0.0f);
		textfield->setButtonsEnabled (false, false, true, true, false);
		textfield->setPromptErrorColor (true);
		textfield->setValue (cfg.getString ("dataPath", ""));
		actionWindow->addOption (uitext->getText (UiTextString::StreamDataPath).capitalized (), textfield, uitext->getText (UiTextString::StreamDataPathDescription));
		actionWindow->setOptionNotEmptyString (uitext->getText (UiTextString::StreamDataPath).capitalized ());
	}

	if (! actionWindow->isOptionDataValid) {
		applyButton->setDisabled (true);
		applyButton->setMouseHoverTooltip (StdString::createSprintf ("%s %s", uitext->getText (UiTextString::Apply).capitalized ().c_str (), uitext->getText (UiTextString::ActionWindowInvalidDataTooltip).c_str ()));
	}

	expandToggle->isVisible = true;
	if ((! interface->getCommandBooleanParam (&agentConfiguration, "isEnabled", false)) || (! actionWindow->isOptionDataValid)) {
		setExpanded (true, true);
	}
	else {
		setExpanded (false, true);
	}

	refreshLayout ();
	return (Result::Success);
}

StdString AgentConfigurationWindow::mediaScanPeriodSliderValueName (float sliderValue) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (uitext->getText (UiTextString::MediaScanPeriodNeverDescription).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::Slowest).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 2.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::Slow).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 3.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::Medium).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 4.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::Fast).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 5.0f)) {
		return (StdString::createSprintf ("%s - %s", uitext->getText (UiTextString::Fastest).capitalized ().c_str (), OsUtil::getDurationDisplayString (AgentConfigurationWindow::MediaScanPeriods[(int) sliderValue] * 1000).c_str ()));
	}

	return (StdString (""));
}

void AgentConfigurationWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	AgentConfigurationWindow *window;
	Toggle *toggle;

	window = (AgentConfigurationWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback.callback) {
		window->expandStateChangeCallback.callback (window->expandStateChangeCallback.callbackData, window);
	}
}

void AgentConfigurationWindow::actionOptionChanged (void *windowPtr, Widget *widgetPtr) {
	AgentConfigurationWindow *window;
	ActionWindow *action;
	UiText *uitext;

	window = (AgentConfigurationWindow *) windowPtr;
	action = (ActionWindow *) widgetPtr;
	uitext = &(App::instance->uiText);

	if (! action->isOptionDataValid) {
		window->applyButton->setDisabled (true);
		window->applyButton->setMouseHoverTooltip (StdString::createSprintf ("%s %s", uitext->getText (UiTextString::Apply).capitalized ().c_str (), uitext->getText (UiTextString::ActionWindowInvalidDataTooltip).c_str ()));
	}
	else {
		window->applyButton->setDisabled (false);
		window->applyButton->setMouseHoverTooltip (uitext->getText (UiTextString::Apply).capitalized ());
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

	if (window->isWaiting || (! action->isOptionDataValid)) {
		return;
	}

	params = new Json ();
	agentconfig = new Json ();
	agentconfig->set ("isEnabled", action->getBooleanValue (uitext->getText (UiTextString::Enabled), true));
	agentconfig->set ("displayName", action->getStringValue (uitext->getText (UiTextString::DisplayName), ""));
	if (interface->getCommandObjectParam (&(window->agentConfiguration), "mediaServerConfiguration", NULL)) {
		cfg = new Json ();
		cfg->set ("mediaPath", action->getStringValue (uitext->getText (UiTextString::SourceMediaPath), ""));
		cfg->set ("dataPath", action->getStringValue (uitext->getText (UiTextString::MediaDataPath), ""));

		period = 0;
		numperiods = sizeof (AgentConfigurationWindow::MediaScanPeriods) / sizeof (AgentConfigurationWindow::MediaScanPeriods[0]);
		index = action->getNumberValue (uitext->getText (UiTextString::MediaScanPeriod), (int) -1);
		if ((index >= 0) && (index < numperiods)) {
			period = AgentConfigurationWindow::MediaScanPeriods[index];
		}
		cfg->set ("scanPeriod", period);
		agentconfig->set ("mediaServerConfiguration", cfg);

		cfg = new Json ();
		cfg->set ("dataPath", action->getStringValue (uitext->getText (UiTextString::MediaDataPath), ""));
		agentconfig->set ("streamServerConfiguration", cfg);
	}
	else if (interface->getCommandObjectParam (&(window->agentConfiguration), "streamServerConfiguration", NULL)) {
		cfg = new Json ();
		cfg->set ("dataPath", action->getStringValue (uitext->getText (UiTextString::StreamDataPath), ""));
		agentconfig->set ("streamServerConfiguration", cfg);
	}
	params->set ("agentConfiguration", agentconfig);

	window->retain ();
	result = App::instance->agentControl.invokeCommand (window->agentId, App::instance->createCommand (SystemInterface::Command_UpdateAgentConfiguration, SystemInterface::Constant_DefaultCommandType, params), AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete, window);
	if (result != Result::Success) {
		window->release ();
		Log::debug ("Failed to invoke UpdateAgentConfiguration command; err=%i agentId=\"%s\"", result, window->agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
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
	if (invokeResult != Result::Success) {
		Log::debug ("Failed to invoke UpdateAgentConfiguration command; err=%i agentId=\"%s\"", invokeResult, agentId.c_str ());
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::ServerContactError));
	}
	else {
		if ((! responseCommand) || (interface->getCommandId (responseCommand) != SystemInterface::CommandId_AgentConfiguration)) {
			Log::debug ("Failed to invoke UpdateAgentConfiguration command; err=\"Invalid response data\" agentId=\"%s\"", agentId.c_str ());
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::InternalError));
		}
		else {
			App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::UpdateAgentConfigurationMessage));
			App::instance->agentControl.refreshAgentStatus (window->agentId);
		}
	}

	window->setWaiting (false);
	window->release ();
}
