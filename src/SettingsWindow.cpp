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
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Image.h"
#include "Slider.h"
#include "SliderWindow.h"
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
, showClockToggle (NULL)
, showInterfaceAnimationsToggle (NULL)
{
	UiText *uitext;
	UiConfiguration *uiconfig;
	Slider *slider;
	float scale;

	uitext = &(App::instance->uiText);
	uiconfig = &(App::instance->uiConfig);

	setFixedSize (true, windowWidth, windowHeight);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	headerImage = (ImageWindow *) addWidget (new ImageWindow ());
	headerImage->setLoadResourcePath (StdString::createSprintf ("bg/settings/%i.png", App::instance->imageScale));

	titleLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (uitext->getText (UiTextString::settings).capitalized (), UiConfiguration::TitleFont, uiconfig->primaryTextColor)));
	titleLabel->setPadding (0.0f, 0.0f);

	closeButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::ExitButtonSprite)));
	closeButton->zLevel = 1;
	closeButton->setMouseClickCallback (SettingsWindow::closeButtonClicked, this);
	closeButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);

	windowSizeLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::windowSize).capitalized (), UiConfiguration::TitleFont, uiconfig->primaryTextColor));
	slider = new Slider ();
	slider->addSnapValue (0.0f);
	slider->addSnapValue (0.25f);
	slider->addSnapValue (0.5f);
	slider->addSnapValue (0.75f);
	slider->addSnapValue (1.0f);
	if ((App::instance->windowWidth == App::windowWidths[0]) && (App::instance->windowHeight == App::windowHeights[0])) {
		slider->setValue (0.0f, true);
	}
	else if ((App::instance->windowWidth == App::windowWidths[1]) && (App::instance->windowHeight == App::windowHeights[1])) {
		slider->setValue (0.25f, true);
	}
	else if ((App::instance->windowWidth == App::windowWidths[2]) && (App::instance->windowHeight == App::windowHeights[2])) {
		slider->setValue (0.5f, true);
	}
	else if ((App::instance->windowWidth == App::windowWidths[3]) && (App::instance->windowHeight == App::windowHeights[3])) {
		slider->setValue (0.75f, true);
	}
	else if ((App::instance->windowWidth == App::windowWidths[4]) && (App::instance->windowHeight == App::windowHeights[4])) {
		slider->setValue (1.0f, true);
	}
	windowSizeSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	windowSizeSlider->setValueChangeCallback (SettingsWindow::windowSizeSliderChanged, this);
	windowSizeSlider->setValueNameFunction (SettingsWindow::windowSizeSliderValueName);

	textSizeLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::textSize).capitalized (), UiConfiguration::TitleFont, uiconfig->primaryTextColor));
	slider = new Slider ();
	slider->addSnapValue (0.0f);
	slider->addSnapValue (0.25f);
	slider->addSnapValue (0.5f);
	slider->addSnapValue (0.75f);
	slider->addSnapValue (1.0f);
	scale = App::instance->fontScale;
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
	showClockToggle->setChecked (App::instance->prefsMap.find (App::ShowClockKey, false));
	showClockToggle->setStateChangeCallback (SettingsWindow::showClockToggleStateChanged, this);

	if (App::instance->isTextureRenderEnabled) {
		showInterfaceAnimationsToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle (), uitext->getText (UiTextString::showInterfaceAnimations).capitalized ()));
		showInterfaceAnimationsToggle->setPadding (0.0f, 0.0f);
		showInterfaceAnimationsToggle->setImageColor (uiconfig->flatButtonTextColor);
		showInterfaceAnimationsToggle->setChecked (App::instance->isInterfaceAnimationEnabled);
		showInterfaceAnimationsToggle->setStateChangeCallback (SettingsWindow::showInterfaceAnimationsToggleStateChanged, this);
	}

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

	uiconfig = &(App::instance->uiConfig);
	x = 0.0f;
	y = 0.0f;

	if (! headerImage->isLoaded ()) {
		x += uiconfig->paddingSize;
		y += uiconfig->paddingSize;
		headerImage->isVisible = false;

		titleLabel->setFillBg (false);
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
		titleLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
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

	if (showInterfaceAnimationsToggle) {
		showInterfaceAnimationsToggle->position.assign (x, y);
		y += showInterfaceAnimationsToggle->height + uiconfig->marginSize;
	}
}

void SettingsWindow::doRefresh () {
	isHeaderImageLoaded = false;
	headerImage->setLoadResourcePath (StdString::createSprintf ("bg/settings/%i.png", App::instance->imageScale));
	Panel::doRefresh ();
}

void SettingsWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (! isHeaderImageLoaded) {
		if (headerImage->isLoaded ()) {
			isHeaderImageLoaded = true;
			refreshLayout ();
		}
	}
}

StdString SettingsWindow::windowSizeSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::instance->uiText.getText (UiTextString::smallest).capitalized ().c_str (), App::windowWidths[0], App::windowHeights[0]));
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::instance->uiText.getText (UiTextString::small).capitalized ().c_str (), App::windowWidths[1], App::windowHeights[1]));
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::instance->uiText.getText (UiTextString::medium).capitalized ().c_str (), App::windowWidths[2], App::windowHeights[2]));
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::instance->uiText.getText (UiTextString::large).capitalized ().c_str (), App::windowWidths[3], App::windowHeights[3]));
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %ix%i", App::instance->uiText.getText (UiTextString::largest).capitalized ().c_str (), App::windowWidths[4], App::windowHeights[4]));
	}

	return (StdString (""));
}

StdString SettingsWindow::textSizeSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (App::instance->uiText.getText (UiTextString::smallest).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (App::instance->uiText.getText (UiTextString::small).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (App::instance->uiText.getText (UiTextString::medium).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (App::instance->uiText.getText (UiTextString::large).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (App::instance->uiText.getText (UiTextString::largest).capitalized ());
	}

	return (StdString (""));
}

void SettingsWindow::closeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	SettingsWindow *window;

	window = (SettingsWindow *) windowPtr;
	window->isDestroyed = true;
}

void SettingsWindow::windowSizeSliderChanged (void *windowPtr, Widget *widgetPtr) {
	SliderWindow *slider;

	slider = (SliderWindow *) widgetPtr;

	if (FLOAT_EQUALS (slider->value, 0.0f)) {
		App::instance->nextWindowWidth = App::windowWidths[0];
		App::instance->nextWindowHeight = App::windowHeights[0];
	}
	if (FLOAT_EQUALS (slider->value, 0.25f)) {
		App::instance->nextWindowWidth = App::windowWidths[1];
		App::instance->nextWindowHeight = App::windowHeights[1];
	}
	if (FLOAT_EQUALS (slider->value, 0.5f)) {
		App::instance->nextWindowWidth = App::windowWidths[2];
		App::instance->nextWindowHeight = App::windowHeights[2];
	}
	if (FLOAT_EQUALS (slider->value, 0.75f)) {
		App::instance->nextWindowWidth = App::windowWidths[3];
		App::instance->nextWindowHeight = App::windowHeights[3];
	}
	if (FLOAT_EQUALS (slider->value, 1.0f)) {
		App::instance->nextWindowWidth = App::windowWidths[4];
		App::instance->nextWindowHeight = App::windowHeights[4];
	}
}

void SettingsWindow::textSizeSliderChanged (void *windowPtr, Widget *widgetPtr) {
	SliderWindow *slider;

	slider = (SliderWindow *) widgetPtr;

	if (FLOAT_EQUALS (slider->value, 0.0f)) {
		App::instance->nextFontScale = App::fontScales[0];
	}
	if (FLOAT_EQUALS (slider->value, 0.25f)) {
		App::instance->nextFontScale = App::fontScales[1];
	}
	if (FLOAT_EQUALS (slider->value, 0.5f)) {
		App::instance->nextFontScale = App::fontScales[2];
	}
	if (FLOAT_EQUALS (slider->value, 0.75f)) {
		App::instance->nextFontScale = App::fontScales[3];
	}
	if (FLOAT_EQUALS (slider->value, 1.0f)) {
		App::instance->nextFontScale = App::fontScales[4];
	}
}

void SettingsWindow::showClockToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *toggle;

	toggle = (ToggleWindow *) widgetPtr;
	App::instance->prefsMap.insert (App::ShowClockKey, toggle->isChecked);
	App::instance->shouldRefreshUi = true;
}

void SettingsWindow::showInterfaceAnimationsToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *toggle;

	toggle = (ToggleWindow *) widgetPtr;
	App::instance->prefsMap.insert (App::ShowInterfaceAnimationsKey, toggle->isChecked);
	App::instance->isInterfaceAnimationEnabled = toggle->isChecked;
}
