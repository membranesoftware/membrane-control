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
#include "StdString.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Slider.h"
#include "SliderWindow.h"
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
	Slider *slider;
	float scale;

	setFixedSize (true, windowWidth, windowHeight);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	headerImage = (ImageWindow *) addWidget (new ImageWindow ());
	headerImage->setImageFilePath (StdString::createSprintf ("bg/settings/%i.png", App::instance->imageScale));

	titleLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::Settings).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor)));
	titleLabel->setPadding (0.0f, 0.0f);

	closeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExitButtonSprite)));
	closeButton->mouseClickCallback = Widget::EventCallbackContext (SettingsWindow::closeButtonClicked, this);
	closeButton->zLevel = 1;
	closeButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);

	windowSizeLabel = (Label *) addWidget (new Label (UiText::instance->getText (UiTextString::WindowSize).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	slider = new Slider ();
	slider->addSnapValue (0.0f);
	slider->addSnapValue (0.25f);
	slider->addSnapValue (0.5f);
	slider->addSnapValue (0.75f);
	slider->addSnapValue (1.0f);
	if ((App::instance->windowWidth == App::WindowWidths[0]) && (App::instance->windowHeight == App::WindowHeights[0])) {
		slider->setValue (0.0f, true);
	}
	else if ((App::instance->windowWidth == App::WindowWidths[1]) && (App::instance->windowHeight == App::WindowHeights[1])) {
		slider->setValue (0.25f, true);
	}
	else if ((App::instance->windowWidth == App::WindowWidths[2]) && (App::instance->windowHeight == App::WindowHeights[2])) {
		slider->setValue (0.5f, true);
	}
	else if ((App::instance->windowWidth == App::WindowWidths[3]) && (App::instance->windowHeight == App::WindowHeights[3])) {
		slider->setValue (0.75f, true);
	}
	else if ((App::instance->windowWidth == App::WindowWidths[4]) && (App::instance->windowHeight == App::WindowHeights[4])) {
		slider->setValue (1.0f, true);
	}
	windowSizeSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	windowSizeSlider->valueChangeCallback = Widget::EventCallbackContext (SettingsWindow::windowSizeSliderChanged, this);
	windowSizeSlider->setValueNameFunction (SettingsWindow::windowSizeSliderValueName);

	textSizeLabel = (Label *) addWidget (new Label (UiText::instance->getText (UiTextString::TextSize).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	slider = new Slider ();
	slider->addSnapValue (0.0f);
	slider->addSnapValue (0.25f);
	slider->addSnapValue (0.5f);
	slider->addSnapValue (0.75f);
	slider->addSnapValue (1.0f);
	scale = App::instance->fontScale;
	if (FLOAT_EQUALS (scale, App::FontScales[0])) {
		slider->setValue (0.0f);
	}
	else if (FLOAT_EQUALS (scale, App::FontScales[1])) {
		slider->setValue (0.25f);
	}
	else if (FLOAT_EQUALS (scale, App::FontScales[2])) {
		slider->setValue (0.5f);
	}
	else if (FLOAT_EQUALS (scale, App::FontScales[3])) {
		slider->setValue (0.75f);
	}
	else if (FLOAT_EQUALS (scale, App::FontScales[4])) {
		slider->setValue (1.0f);
	}
	textSizeSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	textSizeSlider->valueChangeCallback = Widget::EventCallbackContext (SettingsWindow::textSizeSliderChanged, this);
	textSizeSlider->setValueNameFunction (SettingsWindow::textSizeSliderValueName);

	showClockToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle ()));
	showClockToggle->stateChangeCallback = Widget::EventCallbackContext (SettingsWindow::showClockToggleStateChanged, this);
	showClockToggle->setText (UiText::instance->getText (UiTextString::ShowClock).capitalized ());
	showClockToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	showClockToggle->setChecked (App::instance->isMainToolbarClockEnabled, true);

	if (App::instance->isTextureRenderEnabled) {
		showInterfaceAnimationsToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle ()));
		showInterfaceAnimationsToggle->stateChangeCallback = Widget::EventCallbackContext (SettingsWindow::showInterfaceAnimationsToggleStateChanged, this);
		showInterfaceAnimationsToggle->setText (UiText::instance->getText (UiTextString::ShowInterfaceAnimations).capitalized ());
		showInterfaceAnimationsToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
		showInterfaceAnimationsToggle->setChecked (App::instance->isInterfaceAnimationEnabled, true);
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
	float x, y;

	x = 0.0f;
	y = 0.0f;
	if (! headerImage->isLoaded ()) {
		x += UiConfiguration::instance->paddingSize;
		y += UiConfiguration::instance->paddingSize;
		headerImage->isVisible = false;

		titleLabel->setFillBg (false);
		titleLabel->setTextColor (UiConfiguration::instance->primaryTextColor);
		titleLabel->position.assign (x, y);

		closeButton->position.assign (width - closeButton->width - UiConfiguration::instance->paddingSize, y);
		y += closeButton->height + UiConfiguration::instance->marginSize;
	}
	else {
		headerImage->position.assign (x, y);
		headerImage->isVisible = true;

		x += UiConfiguration::instance->paddingSize;
		titleLabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
		titleLabel->setTextColor (UiConfiguration::instance->inverseTextColor);
		titleLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
		titleLabel->position.assign (x, y + headerImage->height - titleLabel->height - UiConfiguration::instance->paddingSize);
		closeButton->position.assign (width - closeButton->width - UiConfiguration::instance->paddingSize, y + headerImage->height - closeButton->height - UiConfiguration::instance->paddingSize);
		y += headerImage->height + UiConfiguration::instance->marginSize;
	}

	windowSizeLabel->position.assign (x, y);
	y += windowSizeLabel->maxLineHeight + UiConfiguration::instance->textLineHeightMargin;

	windowSizeSlider->position.assign (x, y);
	y += windowSizeSlider->height + UiConfiguration::instance->marginSize;

	textSizeLabel->position.assign (x, y);
	y += textSizeLabel->maxLineHeight + UiConfiguration::instance->textLineHeightMargin;

	textSizeSlider->position.assign (x, y);
	y += textSizeSlider->height + UiConfiguration::instance->marginSize;

	showClockToggle->position.assign (x, y);
	y += showClockToggle->height;

	if (showInterfaceAnimationsToggle) {
		showInterfaceAnimationsToggle->position.assign (x, y);
		y += showInterfaceAnimationsToggle->height;
	}
}

void SettingsWindow::doRefresh () {
	isHeaderImageLoaded = false;
	headerImage->setImageFilePath (StdString::createSprintf ("bg/settings/%i.png", App::instance->imageScale));
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
		return (StdString::createSprintf ("%s - %ix%i", UiText::instance->getText (UiTextString::Smallest).capitalized ().c_str (), App::WindowWidths[0], App::WindowHeights[0]));
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (StdString::createSprintf ("%s - %ix%i", UiText::instance->getText (UiTextString::Small).capitalized ().c_str (), App::WindowWidths[1], App::WindowHeights[1]));
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (StdString::createSprintf ("%s - %ix%i", UiText::instance->getText (UiTextString::Medium).capitalized ().c_str (), App::WindowWidths[2], App::WindowHeights[2]));
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (StdString::createSprintf ("%s - %ix%i", UiText::instance->getText (UiTextString::Large).capitalized ().c_str (), App::WindowWidths[3], App::WindowHeights[3]));
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %ix%i", UiText::instance->getText (UiTextString::Largest).capitalized ().c_str (), App::WindowWidths[4], App::WindowHeights[4]));
	}

	return (StdString (""));
}

StdString SettingsWindow::textSizeSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (UiText::instance->getText (UiTextString::Smallest).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (UiText::instance->getText (UiTextString::Small).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (UiText::instance->getText (UiTextString::Medium).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (UiText::instance->getText (UiTextString::Large).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (UiText::instance->getText (UiTextString::Largest).capitalized ());
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
		App::instance->nextWindowWidth = App::WindowWidths[0];
		App::instance->nextWindowHeight = App::WindowHeights[0];
	}
	if (FLOAT_EQUALS (slider->value, 0.25f)) {
		App::instance->nextWindowWidth = App::WindowWidths[1];
		App::instance->nextWindowHeight = App::WindowHeights[1];
	}
	if (FLOAT_EQUALS (slider->value, 0.5f)) {
		App::instance->nextWindowWidth = App::WindowWidths[2];
		App::instance->nextWindowHeight = App::WindowHeights[2];
	}
	if (FLOAT_EQUALS (slider->value, 0.75f)) {
		App::instance->nextWindowWidth = App::WindowWidths[3];
		App::instance->nextWindowHeight = App::WindowHeights[3];
	}
	if (FLOAT_EQUALS (slider->value, 1.0f)) {
		App::instance->nextWindowWidth = App::WindowWidths[4];
		App::instance->nextWindowHeight = App::WindowHeights[4];
	}
}

void SettingsWindow::textSizeSliderChanged (void *windowPtr, Widget *widgetPtr) {
	SliderWindow *slider;

	slider = (SliderWindow *) widgetPtr;

	if (FLOAT_EQUALS (slider->value, 0.0f)) {
		App::instance->nextFontScale = App::FontScales[0];
	}
	if (FLOAT_EQUALS (slider->value, 0.25f)) {
		App::instance->nextFontScale = App::FontScales[1];
	}
	if (FLOAT_EQUALS (slider->value, 0.5f)) {
		App::instance->nextFontScale = App::FontScales[2];
	}
	if (FLOAT_EQUALS (slider->value, 0.75f)) {
		App::instance->nextFontScale = App::FontScales[3];
	}
	if (FLOAT_EQUALS (slider->value, 1.0f)) {
		App::instance->nextFontScale = App::FontScales[4];
	}
}

void SettingsWindow::showClockToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *toggle;
	HashMap *prefs;

	toggle = (ToggleWindow *) widgetPtr;
	App::instance->isMainToolbarClockEnabled = toggle->isChecked;
	prefs = App::instance->lockPrefs ();
	prefs->insert (App::ShowClockKey, toggle->isChecked);
	App::instance->unlockPrefs ();
	App::instance->shouldRefreshUi = true;
}

void SettingsWindow::showInterfaceAnimationsToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	ToggleWindow *toggle;
	HashMap *prefs;

	toggle = (ToggleWindow *) widgetPtr;
	App::instance->isInterfaceAnimationEnabled = toggle->isChecked;
	prefs = App::instance->lockPrefs ();
	prefs->insert (App::ShowInterfaceAnimationsKey, toggle->isChecked);
	App::instance->unlockPrefs ();
}
