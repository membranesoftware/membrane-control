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
#include "Widget.h"
#include "Color.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Image.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "SettingsWindow.h"

SettingsWindow::SettingsWindow (float windowWidth, float windowHeight)
: Panel ()
, headerImage (NULL)
, isHeaderImageLoaded (false)
, titleLabel (NULL)
, closeButton (NULL)
, windowSizeLabel (NULL)
, windowSizeSlider (NULL)
, textSizeLabel (NULL)
, textSizeSlider (NULL)
{
	App *app;
	UiText *uitext;
	UiConfiguration *uiconfig;
	Slider *slider;
	float scale;

	app = App::getInstance ();
	uitext = &(app->uiText);
	uiconfig = &(app->uiConfig);

	setFixedSize (true, windowWidth, windowHeight);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	headerImage = (ImageWindow *) addWidget (new ImageWindow ());
	headerImage->setLoadResourcePath (StdString::createSprintf ("bg/settings/%i.png", app->imageScale));

	titleLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::settings).capitalized (), UiConfiguration::TITLE, uiconfig->primaryTextColor)));
	titleLabel->setPadding (0.0f, 0.0f);

	closeButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::EXIT_BUTTON)));
	closeButton->zLevel = 1;
	closeButton->setMouseClickCallback (SettingsWindow::closeButtonClicked, this);
	closeButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);

	windowSizeLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::windowSize).capitalized (), UiConfiguration::TITLE, uiconfig->primaryTextColor));
	slider = new Slider ();
	slider->addSnapValue (0.0f);
	slider->addSnapValue (0.25f);
	slider->addSnapValue (0.5f);
	slider->addSnapValue (0.75f);
	slider->addSnapValue (1.0f);
	if ((app->windowWidth == App::windowWidths[0]) && (app->windowHeight == App::windowHeights[0])) {
		slider->setValue (0.0f, true);
	}
	else if ((app->windowWidth == App::windowWidths[1]) && (app->windowHeight == App::windowHeights[1])) {
		slider->setValue (0.25f, true);
	}
	else if ((app->windowWidth == App::windowWidths[2]) && (app->windowHeight == App::windowHeights[2])) {
		slider->setValue (0.5f, true);
	}
	else if ((app->windowWidth == App::windowWidths[3]) && (app->windowHeight == App::windowHeights[3])) {
		slider->setValue (0.75f, true);
	}
	else if ((app->windowWidth == App::windowWidths[4]) && (app->windowHeight == App::windowHeights[4])) {
		slider->setValue (1.0f, true);
	}
	windowSizeSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	windowSizeSlider->setValueChangeCallback (SettingsWindow::windowSizeSliderChanged, this);
	windowSizeSlider->setValueNameFunction (SettingsWindow::windowSizeSliderValueName);

	textSizeLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::textSize).capitalized (), UiConfiguration::TITLE, uiconfig->primaryTextColor));
	slider = new Slider ();
	slider->addSnapValue (0.0f);
	slider->addSnapValue (0.25f);
	slider->addSnapValue (0.5f);
	slider->addSnapValue (0.75f);
	slider->addSnapValue (1.0f);
	scale = app->fontScale;
	if (FLOAT_EQUALS (scale, App::fontScales[0])) {
		slider->setValue (0.0f);
	}
	else if (FLOAT_EQUALS (scale, App::fontScales[1])) {
		slider->setValue (0.25f);
	}
	else if (FLOAT_EQUALS (scale, App::fontScales[2])) {
		slider->setValue (0.5f);
	}
	else if (FLOAT_EQUALS (scale, App::fontScales[3])) {
		slider->setValue (0.75f);
	}
	else if (FLOAT_EQUALS (scale, App::fontScales[4])) {
		slider->setValue (1.0f);
	}
	textSizeSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	textSizeSlider->setValueChangeCallback (SettingsWindow::textSizeSliderChanged, this);
	textSizeSlider->setValueNameFunction (SettingsWindow::textSizeSliderValueName);

	showClockToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle (), uitext->getText (UiTextString::showClock).capitalized ()));
	showClockToggle->setPadding (0.0f, 0.0f);
	showClockToggle->setImageColor (uiconfig->flatButtonTextColor);
	showClockToggle->setChecked (app->prefsMap.find (App::prefsShowClock, false));
	showClockToggle->setStateChangeCallback (SettingsWindow::showClockToggleStateChanged, this);

	refreshLayout ();
}

SettingsWindow::~SettingsWindow () {

}

StdString SettingsWindow::toStringDetail () {
	return (StdString (" SettingsWindow"));
}

void SettingsWindow::setWindowSize (float windowWidth, float windowHeight) {
	setFixedSize (true, windowWidth, windowHeight);
	refreshLayout ();
}

void SettingsWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = 0.0f;
	y = 0.0f;

	if (! headerImage->isLoaded ()) {
		x += uiconfig->paddingSize;
		y += uiconfig->paddingSize;
		headerImage->isVisible = false;

		titleLabel->setFillBg (false);
		titleLabel->setAlphaBlend (false);
		titleLabel->setTextColor (uiconfig->primaryTextColor);
		titleLabel->position.assign (x, y);

		closeButton->position.assign (width - closeButton->width - uiconfig->paddingSize, y);
		y += closeButton->height + uiconfig->marginSize;
	}
	else {
		headerImage->position.assign (x, y);
		headerImage->isVisible = true;

		x += uiconfig->paddingSize;
		titleLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
		titleLabel->setTextColor (uiconfig->inverseTextColor);
		titleLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
		titleLabel->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
		titleLabel->position.assign (x, y + headerImage->height - titleLabel->height - uiconfig->paddingSize);
		closeButton->position.assign (width - closeButton->width - uiconfig->paddingSize, y + headerImage->height - closeButton->height - uiconfig->paddingSize);
		y += headerImage->height + uiconfig->marginSize;
	}

	windowSizeLabel->position.assign (x, y);
	y += windowSizeLabel->maxLineHeight + uiconfig->textLineHeightMargin;

	windowSizeSlider->position.assign (x, y);
	y += windowSizeSlider->height + uiconfig->marginSize;

	textSizeLabel->position.assign (x, y);
	y += textSizeLabel->maxLineHeight + uiconfig->textLineHeightMargin;

	textSizeSlider->position.assign (x, y);
	y += textSizeSlider->height + uiconfig->marginSize;

	showClockToggle->position.assign (x, y);
	y += showClockToggle->height + uiconfig->marginSize;
}

void SettingsWindow::doRefresh () {
	App *app;

	app = App::getInstance ();
	isHeaderImageLoaded = false;
	headerImage->setLoadResourcePath (StdString::createSprintf ("bg/settings/%i.png", app->imageScale));
	Panel::doRefresh ();
}

void SettingsWindow::doUpdate (int msElapsed, float originX, float originY) {
	Panel::doUpdate (msElapsed, originX, originY);

	if (! isHeaderImageLoaded) {
		if (headerImage->isLoaded ()) {
			isHeaderImageLoaded = true;
			refreshLayout ();
		}
	}
}

StdString SettingsWindow::windowSizeSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::getInstance ()->uiText.getText (UiTextString::smallest).capitalized ().c_str (), App::windowWidths[0], App::windowHeights[0]));
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::getInstance ()->uiText.getText (UiTextString::small).capitalized ().c_str (), App::windowWidths[1], App::windowHeights[1]));
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::getInstance ()->uiText.getText (UiTextString::medium).capitalized ().c_str (), App::windowWidths[2], App::windowHeights[2]));
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::getInstance ()->uiText.getText (UiTextString::large).capitalized ().c_str (), App::windowWidths[3], App::windowHeights[3]));
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::getInstance ()->uiText.getText (UiTextString::largest).capitalized ().c_str (), App::windowWidths[4], App::windowHeights[4]));
	}

	return (StdString (""));
}

StdString SettingsWindow::textSizeSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (App::getInstance ()->uiText.getText (UiTextString::smallest).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (App::getInstance ()->uiText.getText (UiTextString::small).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (App::getInstance ()->uiText.getText (UiTextString::medium).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (App::getInstance ()->uiText.getText (UiTextString::large).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (App::getInstance ()->uiText.getText (UiTextString::largest).capitalized ());
	}

	return (StdString (""));
}

void SettingsWindow::closeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	SettingsWindow *window;

	window = (SettingsWindow *) windowPtr;
	window->isDestroyed = true;
}

void SettingsWindow::windowSizeSliderChanged (void *windowPtr, Widget *widgetPtr) {
	App *app;
	SliderWindow *slider;

	slider = (SliderWindow *) widgetPtr;
	app = App::getInstance ();

	if (FLOAT_EQUALS (slider->value, 0.0f)) {
		app->nextWindowWidth = App::windowWidths[0];
		app->nextWindowHeight = App::windowHeights[0];
	}
	if (FLOAT_EQUALS (slider->value, 0.25f)) {
		app->nextWindowWidth = App::windowWidths[1];
		app->nextWindowHeight = App::windowHeights[1];
	}
	if (FLOAT_EQUALS (slider->value, 0.5f)) {
		app->nextWindowWidth = App::windowWidths[2];
		app->nextWindowHeight = App::windowHeights[2];
	}
	if (FLOAT_EQUALS (slider->value, 0.75f)) {
		app->nextWindowWidth = App::windowWidths[3];
		app->nextWindowHeight = App::windowHeights[3];
	}
	if (FLOAT_EQUALS (slider->value, 1.0f)) {
		app->nextWindowWidth = App::windowWidths[4];
		app->nextWindowHeight = App::windowHeights[4];
	}
}

void SettingsWindow::textSizeSliderChanged (void *windowPtr, Widget *widgetPtr) {
	SliderWindow *slider;

	slider = (SliderWindow *) widgetPtr;

	if (FLOAT_EQUALS (slider->value, 0.0f)) {
		App::getInstance ()->nextFontScale = App::fontScales[0];
	}
	if (FLOAT_EQUALS (slider->value, 0.25f)) {
		App::getInstance ()->nextFontScale = App::fontScales[1];
	}
	if (FLOAT_EQUALS (slider->value, 0.5f)) {
		App::getInstance ()->nextFontScale = App::fontScales[2];
	}
	if (FLOAT_EQUALS (slider->value, 0.75f)) {
		App::getInstance ()->nextFontScale = App::fontScales[3];
	}
	if (FLOAT_EQUALS (slider->value, 1.0f)) {
		App::getInstance ()->nextFontScale = App::fontScales[4];
	}
}

void SettingsWindow::showClockToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	App *app;
	ToggleWindow *toggle;

	toggle = (ToggleWindow *) widgetPtr;
	app = App::getInstance ();
	app->prefsMap.insert (App::prefsShowClock, toggle->isChecked);
	app->shouldRefreshUi = true;
}
