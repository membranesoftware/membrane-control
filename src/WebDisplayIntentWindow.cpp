/*
* Copyright 2018 Membrane Software <author@membranesoftware.com>
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
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "WebKioskUi.h"
#include "WebDisplayIntentWindow.h"

const float WebDisplayIntentWindow::windowWidthMultiplier = 0.45f;
const int WebDisplayIntentWindow::itemDisplayDurations[] = { 3 * 3600, 3600, 900, 420, 180 };

WebDisplayIntentWindow::WebDisplayIntentWindow (SpriteGroup *webKioskUiSpriteGroup)
: Panel ()
, isSelected (false)
, isExpanded (false)
, menuPositionY (0.0f)
, windowWidth (0.0f)
, spriteGroup (webKioskUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, urlCountLabel (NULL)
, shuffleToggle (NULL)
, itemDisplayDurationLabel (NULL)
, itemDisplayDurationSlider (NULL)
, urlListEmptyText (NULL)
, urlListView (NULL)
, selectToggle (NULL)
, menuButton (NULL)
, selectStateChangeCallback (NULL)
, selectStateChangeCallbackData (NULL)
, nameClickCallback (NULL)
, nameClickCallbackData (NULL)
, menuClickCallback (NULL)
, menuClickCallbackData (NULL)
, urlListChangeCallback (NULL)
, urlListChangeCallbackData (NULL)
{
	App *app;
	UiConfiguration *uiconfig;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	windowWidth = app->windowWidth * WebDisplayIntentWindow::windowWidthMultiplier;

	populate ();
	resetLayout ();
}

WebDisplayIntentWindow::~WebDisplayIntentWindow () {

}

StdString WebDisplayIntentWindow::toStringDetail () {
	return (StdString (" WebDisplayIntentWindow"));
}

bool WebDisplayIntentWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" WebDisplayIntentWindow"));
}

WebDisplayIntentWindow *WebDisplayIntentWindow::castWidget (Widget *widget) {
	return (WebDisplayIntentWindow::isWidgetType (widget) ? (WebDisplayIntentWindow *) widget : NULL);
}

void WebDisplayIntentWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	Slider *slider;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (WebKioskUi::INTENT_ICON)));
	nameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::HEADLINE, uiconfig->primaryTextColor)));
	nameLabel->setMouseClickCallback (WebDisplayIntentWindow::nameLabelClicked, this);
	nameLabel->setMouseHoverTooltip (uitext->clickRenameTooltip);

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle (), uitext->shuffle.capitalized ()));
	shuffleToggle->setPadding (0.0f, 0.0f);
	shuffleToggle->setImageColor (uiconfig->flatButtonTextColor);
	shuffleToggle->setMouseHoverTooltip (uitext->webKioskUiShuffleTooltip);

	itemDisplayDurationLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (uitext->speed.capitalized (), UiConfiguration::CAPTION, uiconfig->primaryTextColor)));
	itemDisplayDurationLabel->setPadding (0.0f, 0.0f);
	itemDisplayDurationLabel->zLevel = 1;

	slider = new Slider ();
	slider->addSnapValue (0.0f);
	slider->addSnapValue (0.25f);
	slider->addSnapValue (0.5f);
	slider->addSnapValue (0.75f);
	slider->addSnapValue (1.0f);
	itemDisplayDurationSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	itemDisplayDurationSlider->setPadding (0.0f, 0.0f);
	itemDisplayDurationSlider->setValueNameFunction (WebDisplayIntentWindow::itemDisplayDurationSliderValueName);

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::STAR_OUTLINE_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::STAR_BUTTON)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (WebDisplayIntentWindow::selectToggleStateChanged, this);
	selectToggle->setMouseHoverTooltip (uitext->selectToggleTooltip);

	menuButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::MAIN_MENU_BUTTON)));
	menuButton->setMouseClickCallback (WebDisplayIntentWindow::menuButtonClicked, this);
	menuButton->setImageColor (uiconfig->flatButtonTextColor);
	menuButton->setMouseHoverTooltip (uitext->moreActionsTooltip);

	urlCountLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::WEB_LINK_ICON), StdString ("0"), UiConfiguration::TITLE));
	urlCountLabel->setPadding (0.0f, 0.0f);
	urlCountLabel->setMouseHoverTooltip (uitext->getCountText (UiText::WEB_DISPLAY_INTENT_ITEMS, 0));

	urlListEmptyText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	urlListEmptyText->setText (uitext->emptyWebDisplayIntentAddressList);

	urlListView = (ListView *) addWidget (new ListView (windowWidth - (widthPadding * 2.0f), 6, UiConfiguration::BODY, StdString (""), uitext->emptyWebDisplayIntentAddressList));
	urlListView->setListChangeCallback (WebDisplayIntentWindow::urlListChanged, this);
	urlListView->isVisible = false;
}

void WebDisplayIntentWindow::setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	selectStateChangeCallback = callback;
	selectStateChangeCallbackData = callbackData;
}

void WebDisplayIntentWindow::setNameClickCallback (Widget::EventCallback callback, void *callbackData) {
	nameClickCallback = callback;
	nameClickCallbackData = callbackData;
}

void WebDisplayIntentWindow::setMenuClickCallback (Widget::EventCallback callback, void *callbackData) {
	menuClickCallback = callback;
	menuClickCallbackData = callbackData;
}

void WebDisplayIntentWindow::setUrlListChangeCallback (Widget::EventCallback callback, void *callbackData) {
	urlListChangeCallback = callback;
	urlListChangeCallbackData = callbackData;
}

void WebDisplayIntentWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
	Widget::EventCallback callback;
	void *callbackdata;

	if (selected == isSelected) {
		return;
	}
	callback = NULL;
	callbackdata = NULL;
	if (shouldSkipStateChangeCallback) {
		callback = selectStateChangeCallback;
		callbackdata = selectStateChangeCallbackData;
		selectStateChangeCallback = NULL;
		selectStateChangeCallbackData = NULL;
	}
	isSelected = selected;
	selectToggle->setChecked (isSelected);
	resetLayout ();
	if (shouldSkipStateChangeCallback) {
		selectStateChangeCallback = callback;
		selectStateChangeCallbackData = callbackdata;
	}
}

void WebDisplayIntentWindow::setIntentName (const StdString &name) {
	intentName.assign (name);
	nameLabel->setText (intentName);
	resetLayout ();
}

void WebDisplayIntentWindow::setExpanded (bool expanded) {
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	if (isExpanded) {
		urlListView->isVisible = true;
		urlListEmptyText->isVisible = false;
	}
	else {
		urlListView->isVisible = false;
		urlListEmptyText->isVisible = true;
	}
	resetLayout ();
}

void WebDisplayIntentWindow::resetLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, y0, h;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;

	iconImage->position.assign (x, y);
	x += iconImage->width + uiconfig->marginSize;

	nameLabel->position.assign (x, y);
	x += nameLabel->width + uiconfig->marginSize;

	selectToggle->position.assign (x, y);

	y += nameLabel->height + uiconfig->textLineHeightMargin;

	if (y < (y0 + iconImage->height)) {
		y = y0 + iconImage->height;
	}

	h = 0.0f;
	x = x0 + widthPadding;
	urlCountLabel->position.assign (x, y);
	x += urlCountLabel->width + uiconfig->marginSize;
	if (urlCountLabel->height > h) {
		h = urlCountLabel->height;
	}

	shuffleToggle->position.assign (x, y);
	x += shuffleToggle->width + uiconfig->marginSize;
	if (shuffleToggle->height > h) {
		h = shuffleToggle->height;
	}

	x += uiconfig->marginSize;
	itemDisplayDurationSlider->position.assign (x, y);
	itemDisplayDurationLabel->position.assign (x + itemDisplayDurationSlider->width - itemDisplayDurationLabel->width, y);
	x += itemDisplayDurationSlider->width + uiconfig->marginSize;
	if (itemDisplayDurationSlider->height > h) {
		h = itemDisplayDurationSlider->height;
	}

	menuButton->position.assign (x, y);
	menuPositionY = y + menuButton->height;
	x += menuButton->width + uiconfig->marginSize;
	if (menuButton->height > h) {
		h = menuButton->height;
	}

	urlCountLabel->position.move (0.0f, (h / 2.0f) - (urlCountLabel->height / 2.0f));
	itemDisplayDurationSlider->position.move (0.0f, (h / 2.0f) - (itemDisplayDurationSlider->height / 2.0f));
	itemDisplayDurationLabel->position.assignY (itemDisplayDurationSlider->position.y);
	y += h + uiconfig->marginSize;

	if (urlListEmptyText->isVisible) {
		x = x0;
		urlListEmptyText->position.assign (x, y);
		y += urlListEmptyText->height + uiconfig->marginSize;
	}
	if (urlListView->isVisible) {
		x = x0;
		urlListView->position.assign (x, y);
		y += urlListView->height + uiconfig->marginSize;
	}

	resetSize ();

	x = width - widthPadding;
	x -= selectToggle->width;
	selectToggle->position.assignX (x);
	x -= uiconfig->marginSize;

	x = width - widthPadding;
	x -= menuButton->width;
	menuButton->position.assignX (x);
	x -= uiconfig->marginSize;
}

void WebDisplayIntentWindow::doRefresh () {
	App *app;

	app = App::getInstance ();
	windowWidth = app->windowWidth * WebDisplayIntentWindow::windowWidthMultiplier;
	urlListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
	Panel::doRefresh ();
}

void WebDisplayIntentWindow::nameLabelClicked (void *windowPtr, Widget *widgetPtr) {
	WebDisplayIntentWindow *window;

	window = (WebDisplayIntentWindow *) windowPtr;
	if (window->nameClickCallback) {
		window->nameClickCallback (window->nameClickCallbackData, window);
	}
}

void WebDisplayIntentWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	WebDisplayIntentWindow *window;
	Toggle *toggle;

	window = (WebDisplayIntentWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->isSelected = toggle->isChecked;
	if (window->selectStateChangeCallback) {
		window->selectStateChangeCallback (window->selectStateChangeCallbackData, window);
	}
}

void WebDisplayIntentWindow::urlListChanged (void *windowPtr, Widget *widgetPtr) {
	WebDisplayIntentWindow *window;
	UiText *uitext;
	int count;

	window = (WebDisplayIntentWindow *) windowPtr;
	uitext = &(App::getInstance ()->uiText);
	count = window->urlListView->getItemCount ();
	window->urlCountLabel->setText (StdString::createSprintf ("%i", count));
	window->urlCountLabel->tooltipText.assign (uitext->getCountText (UiText::WEB_DISPLAY_INTENT_ITEMS, count));
	window->resetLayout ();

	if (window->urlListChangeCallback) {
		window->urlListChangeCallback (window->urlListChangeCallbackData, window);
	}
}

void WebDisplayIntentWindow::addUrl (const StdString &url) {
	urlListView->addItem (url);
}

Json *WebDisplayIntentWindow::getCreateCommand () {
	Json *obj;
	StringList urls;

	obj = new Json ();
	urlListView->getItems (&urls);
	obj->set ("displayName", intentName);
	obj->set ("urls", &urls);
	obj->set ("isShuffle", shuffleToggle->isChecked);
	obj->set ("minItemDisplayDuration", getItemDisplayDuration ());
	obj->set ("maxItemDisplayDuration", getItemDisplayDuration ());

	return (App::getInstance ()->createCommand ("CreateMonitorIntent", SystemInterface::Constant_Display, obj));
}

Json *WebDisplayIntentWindow::getState () {
	Json *obj;
	StringList urls;

	obj = new Json ();
	urlListView->getItems (&urls);
	obj->set ("di", intentName);
	obj->set ("ur", &urls);
	obj->set ("sh", shuffleToggle->isChecked);
	obj->set ("sp", getItemDisplayDuration ());
	obj->set ("se", isSelected);

	return (obj);
}

void WebDisplayIntentWindow::setState (Json *stateObject) {
	StdString val;
	StringList urls;
	int duration;

	val = stateObject->getString ("di", "");
	if (! val.empty ()) {
		setIntentName (val);
	}

	stateObject->getStringList ("ur", &urls);
	urlListView->setItems (&urls);

	duration = stateObject->getNumber ("sp", WebDisplayIntentWindow::itemDisplayDurations[2]);
	if (duration == WebDisplayIntentWindow::itemDisplayDurations[0]) {
		itemDisplayDurationSlider->setValue (0.0f);
	}
	else if (duration == WebDisplayIntentWindow::itemDisplayDurations[1]) {
		itemDisplayDurationSlider->setValue (0.25f);
	}
	else if (duration == WebDisplayIntentWindow::itemDisplayDurations[2]) {
		itemDisplayDurationSlider->setValue (0.5f);
	}
	else if (duration == WebDisplayIntentWindow::itemDisplayDurations[3]) {
		itemDisplayDurationSlider->setValue (0.75f);
	}
	else if (duration == WebDisplayIntentWindow::itemDisplayDurations[4]) {
		itemDisplayDurationSlider->setValue (1.0f);
	}

	shuffleToggle->setChecked (stateObject->getBoolean ("sh", false));
	setSelected (stateObject->getBoolean ("se", false), true);
}

void WebDisplayIntentWindow::menuButtonClicked (void *windowPtr, Widget *widgetPtr) {
	WebDisplayIntentWindow *window;

	window = (WebDisplayIntentWindow *) windowPtr;
	if (window->menuClickCallback) {
		window->menuClickCallback (window->menuClickCallbackData, window);
	}
}

StdString WebDisplayIntentWindow::itemDisplayDurationSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.slowest.capitalized ().c_str (), Util::getDurationDisplayString (WebDisplayIntentWindow::itemDisplayDurations[0] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.slow.capitalized ().c_str (), Util::getDurationDisplayString (WebDisplayIntentWindow::itemDisplayDurations[1] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.medium.capitalized ().c_str (), Util::getDurationDisplayString (WebDisplayIntentWindow::itemDisplayDurations[2] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.fast.capitalized ().c_str (), Util::getDurationDisplayString (WebDisplayIntentWindow::itemDisplayDurations[3] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.fastest.capitalized ().c_str (), Util::getDurationDisplayString (WebDisplayIntentWindow::itemDisplayDurations[4] * 1000).c_str ()));
	}

	return (StdString (""));
}

int WebDisplayIntentWindow::getItemDisplayDuration () {
	float val;

	val = itemDisplayDurationSlider->value;
	if (FLOAT_EQUALS (val, 0.0f)) {
		return (WebDisplayIntentWindow::itemDisplayDurations[0]);
	}
	if (FLOAT_EQUALS (val, 0.25f)) {
		return (WebDisplayIntentWindow::itemDisplayDurations[1]);
	}
	if (FLOAT_EQUALS (val, 0.5f)) {
		return (WebDisplayIntentWindow::itemDisplayDurations[2]);
	}
	if (FLOAT_EQUALS (val, 0.75f)) {
		return (WebDisplayIntentWindow::itemDisplayDurations[3]);
	}
	if (FLOAT_EQUALS (val, 1.0f)) {
		return (WebDisplayIntentWindow::itemDisplayDurations[4]);
	}

	return (WebDisplayIntentWindow::itemDisplayDurations[2]);
}
