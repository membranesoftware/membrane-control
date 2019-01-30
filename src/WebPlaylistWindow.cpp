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
#include "WebPlaylistWindow.h"

const float WebPlaylistWindow::windowWidthMultiplier = 0.45f;
const int WebPlaylistWindow::itemDisplayDurations[] = { 3 * 3600, 3600, 900, 420, 180 };

WebPlaylistWindow::WebPlaylistWindow ()
: Panel ()
, isSelected (false)
, isExpanded (false)
, menuPositionX (0.0f)
, menuPositionY (0.0f)
, windowWidth (0.0f)
, iconImage (NULL)
, nameLabel (NULL)
, urlCountLabel (NULL)
, shuffleToggle (NULL)
, itemDisplayDurationLabel (NULL)
, itemDisplayDurationSlider (NULL)
, urlListView (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, menuButton (NULL)
, selectStateChangeCallback (NULL)
, selectStateChangeCallbackData (NULL)
, expandStateChangeCallback (NULL)
, expandStateChangeCallbackData (NULL)
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

	windowWidth = app->windowWidth * WebPlaylistWindow::windowWidthMultiplier;

	populate ();
	refreshLayout ();
}

WebPlaylistWindow::~WebPlaylistWindow () {

}

StdString WebPlaylistWindow::toStringDetail () {
	return (StdString (" WebPlaylistWindow"));
}

bool WebPlaylistWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" WebPlaylistWindow"));
}

WebPlaylistWindow *WebPlaylistWindow::castWidget (Widget *widget) {
	return (WebPlaylistWindow::isWidgetType (widget) ? (WebPlaylistWindow *) widget : NULL);
}

void WebPlaylistWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::PROGRAM_ICON)));
	nameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::HEADLINE, uiconfig->primaryTextColor)));
	nameLabel->setMouseClickCallback (WebPlaylistWindow::nameLabelClicked, this);
	nameLabel->setMouseHoverTooltip (uitext->getText (UiTextString::clickRenameTooltip));

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle (), uitext->getText (UiTextString::shuffle).capitalized ()));
	shuffleToggle->setPadding (0.0f, 0.0f);
	shuffleToggle->setImageColor (uiconfig->flatButtonTextColor);
	shuffleToggle->setMouseHoverTooltip (uitext->getText (UiTextString::shuffleTooltip));
	shuffleToggle->isVisible = false;

	itemDisplayDurationLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::speed).capitalized (), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
	itemDisplayDurationLabel->zLevel = 1;
	itemDisplayDurationLabel->isVisible = false;

	itemDisplayDurationSlider = (SliderWindow *) addWidget (new SliderWindow (new Slider ()));
	itemDisplayDurationSlider->addSnapValue (0.0f);
	itemDisplayDurationSlider->addSnapValue (0.25f);
	itemDisplayDurationSlider->addSnapValue (0.5f);
	itemDisplayDurationSlider->addSnapValue (0.75f);
	itemDisplayDurationSlider->addSnapValue (1.0f);
	itemDisplayDurationSlider->setPadding (uiconfig->paddingSize, 0.0f);
	itemDisplayDurationSlider->setValueNameFunction (WebPlaylistWindow::itemDisplayDurationSliderValueName);
	itemDisplayDurationSlider->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::STAR_OUTLINE_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::STAR_BUTTON)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (WebPlaylistWindow::selectToggleStateChanged, this);
	selectToggle->setMouseHoverTooltip (uitext->getText (UiTextString::selectToggleTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::EXPAND_MORE_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::EXPAND_LESS_BUTTON)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (WebPlaylistWindow::expandToggleStateChanged, this);
	expandToggle->setMouseHoverTooltip (uitext->getText (UiTextString::expandToggleTooltip));

	menuButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::MAIN_MENU_BUTTON)));
	menuButton->setMouseClickCallback (WebPlaylistWindow::menuButtonClicked, this);
	menuButton->setImageColor (uiconfig->flatButtonTextColor);
	menuButton->setMouseHoverTooltip (uitext->getText (UiTextString::moreActionsTooltip));

	urlCountLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::WEB_LINK_ICON), StdString ("0"), UiConfiguration::TITLE));
	urlCountLabel->setPadding (0.0f, 0.0f);
	urlCountLabel->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::webPlaylistItem, UiTextString::webPlaylistItems));

	urlListView = (ListView *) addWidget (new ListView (windowWidth - (widthPadding * 2.0f), 6, UiConfiguration::BODY, StdString (""), uitext->getText (UiTextString::emptyWebPlaylistAddressList)));
	urlListView->setListChangeCallback (WebPlaylistWindow::urlListChanged, this);
	urlListView->isVisible = false;
}

void WebPlaylistWindow::setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	selectStateChangeCallback = callback;
	selectStateChangeCallbackData = callbackData;
}

void WebPlaylistWindow::setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	expandStateChangeCallback = callback;
	expandStateChangeCallbackData = callbackData;
}

void WebPlaylistWindow::setNameClickCallback (Widget::EventCallback callback, void *callbackData) {
	nameClickCallback = callback;
	nameClickCallbackData = callbackData;
}

void WebPlaylistWindow::setMenuClickCallback (Widget::EventCallback callback, void *callbackData) {
	menuClickCallback = callback;
	menuClickCallbackData = callbackData;
}

void WebPlaylistWindow::setUrlListChangeCallback (Widget::EventCallback callback, void *callbackData) {
	urlListChangeCallback = callback;
	urlListChangeCallbackData = callbackData;
}

void WebPlaylistWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
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
	refreshLayout ();
	if (shouldSkipStateChangeCallback) {
		selectStateChangeCallback = callback;
		selectStateChangeCallbackData = callbackdata;
	}
}

void WebPlaylistWindow::setPlaylistName (const StdString &name) {
	playlistName.assign (name);
	nameLabel->setText (playlistName);
	refreshLayout ();
	resetNameLabel ();
}

void WebPlaylistWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	Widget::EventCallback callback;
	void *callbackdata;

	if (expanded == isExpanded) {
		return;
	}
	callback = NULL;
	callbackdata = NULL;
	if (shouldSkipStateChangeCallback) {
		callback = expandStateChangeCallback;
		callbackdata = expandStateChangeCallbackData;
		expandStateChangeCallback = NULL;
		expandStateChangeCallbackData = NULL;
	}

	isExpanded = expanded;
	if (isExpanded) {
		expandToggle->setChecked (true);
		urlListView->isVisible = true;
		shuffleToggle->isVisible = true;
		itemDisplayDurationLabel->isVisible = true;
		itemDisplayDurationSlider->isVisible = true;
	}
	else {
		expandToggle->setChecked (false);
		urlListView->isVisible = false;
		shuffleToggle->isVisible = false;
		itemDisplayDurationLabel->isVisible = false;
		itemDisplayDurationSlider->isVisible = false;
	}
	refreshLayout ();
	resetNameLabel ();

	if (shouldSkipStateChangeCallback) {
		expandStateChangeCallback = callback;
		expandStateChangeCallbackData = callbackdata;
	}
}

void WebPlaylistWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, y0, x2, y2;

	uiconfig = &(App::getInstance ()->uiConfig);
	x0 = widthPadding;
	y0 = heightPadding;
	x = x0;
	y = y0;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowRight (&x, y, &x2, &y2);
	if (! isExpanded) {
		menuButton->flowRight (&x, y, &x2, &y2);
		urlCountLabel->flowRight (&x, y, &x2, &y2);
	}
	expandToggle->flowRight (&x, y, &x2, &y2);
	selectToggle->flowRight (&x, y, &x2, &y2);
	if (! isExpanded) {
		urlCountLabel->centerVertical (y0, y2);
	}

	x = x0;
	x2 = 0.0f;
	y = y2;
	y0 = y;
	if (isExpanded) {
		x = x0 + uiconfig->marginSize;
		x2 = 0.0f;
		urlCountLabel->flowRight (&x, y, &x2, &y2);
		shuffleToggle->flowRight (&x, y, &x2, &y2);
		itemDisplayDurationSlider->flowRight (&x, y, &x2, &y2);
		menuButton->flowRight (&x, y, &x2, &y2);

		urlCountLabel->centerVertical (y0, y2);
		shuffleToggle->centerVertical (y0, y2);
		itemDisplayDurationSlider->centerVertical (y0, y2);
		itemDisplayDurationLabel->position.assign (itemDisplayDurationSlider->position.x + itemDisplayDurationSlider->width - itemDisplayDurationLabel->width, itemDisplayDurationSlider->position.y);

		x = x0;
		x2 = 0.0f;
		y = y2;
		urlListView->flowRight (&x, y, &x2, &y2);
	}

	setFixedSize (true, windowWidth, y2 + heightPadding);
	x = width - widthPadding;
	selectToggle->flowLeft (&x);
	expandToggle->flowLeft (&x);
	if (! isExpanded) {
		menuButton->flowLeft (&x);
		urlCountLabel->flowLeft (&x);
	}

	if (isExpanded) {
		x = width - widthPadding;
		menuButton->flowLeft (&x);
	}

	menuPositionX = menuButton->position.x;
	menuPositionY = menuButton->position.y + menuButton->height;
}

void WebPlaylistWindow::resetNameLabel () {
	UiConfiguration *uiconfig;
	float w;

	uiconfig = &(App::getInstance ()->uiConfig);
	if (isExpanded) {
		w = expandToggle->position.x;
	}
	else {
		w = urlCountLabel->position.x;
	}
	w -= nameLabel->position.x;
	w -= uiconfig->marginSize;
	nameLabel->setText (Label::getTruncatedText (playlistName, UiConfiguration::HEADLINE, w, StdString ("...")));
	refreshLayout ();
}

void WebPlaylistWindow::doRefresh () {
	App *app;

	app = App::getInstance ();
	windowWidth = app->windowWidth * WebPlaylistWindow::windowWidthMultiplier;
	urlListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
	Panel::doRefresh ();
}

void WebPlaylistWindow::nameLabelClicked (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;

	window = (WebPlaylistWindow *) windowPtr;
	if (window->nameClickCallback) {
		window->nameClickCallback (window->nameClickCallbackData, window);
	}
}

void WebPlaylistWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Toggle *toggle;

	window = (WebPlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->isSelected = toggle->isChecked;
	if (window->selectStateChangeCallback) {
		window->selectStateChangeCallback (window->selectStateChangeCallbackData, window);
	}
}

void WebPlaylistWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Toggle *toggle;

	window = (WebPlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback) {
		window->expandStateChangeCallback (window->expandStateChangeCallbackData, window);
	}
}

void WebPlaylistWindow::urlListChanged (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	UiText *uitext;
	int count;

	window = (WebPlaylistWindow *) windowPtr;
	uitext = &(App::getInstance ()->uiText);
	count = window->urlListView->getItemCount ();
	window->urlCountLabel->setText (StdString::createSprintf ("%i", count));
	window->urlCountLabel->tooltipText.assign (uitext->getCountText (count, UiTextString::webPlaylistItem, UiTextString::webPlaylistItems));
	window->refreshLayout ();

	if (window->urlListChangeCallback) {
		window->urlListChangeCallback (window->urlListChangeCallbackData, window);
	}
}

void WebPlaylistWindow::addUrl (const StdString &url) {
	urlListView->addItem (url);
}

void WebPlaylistWindow::menuButtonClicked (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;

	window = (WebPlaylistWindow *) windowPtr;
	if (window->menuClickCallback) {
		window->menuClickCallback (window->menuClickCallbackData, window);
	}
}

StdString WebPlaylistWindow::itemDisplayDurationSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.getText (UiTextString::slowest).capitalized ().c_str (), Util::getDurationDisplayString (WebPlaylistWindow::itemDisplayDurations[0] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.getText (UiTextString::slow).capitalized ().c_str (), Util::getDurationDisplayString (WebPlaylistWindow::itemDisplayDurations[1] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.getText (UiTextString::medium).capitalized ().c_str (), Util::getDurationDisplayString (WebPlaylistWindow::itemDisplayDurations[2] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.getText (UiTextString::fast).capitalized ().c_str (), Util::getDurationDisplayString (WebPlaylistWindow::itemDisplayDurations[3] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", App::getInstance ()->uiText.getText (UiTextString::fastest).capitalized ().c_str (), Util::getDurationDisplayString (WebPlaylistWindow::itemDisplayDurations[4] * 1000).c_str ()));
	}

	return (StdString (""));
}

int WebPlaylistWindow::getItemDisplayDuration () {
	float val;

	val = itemDisplayDurationSlider->value;
	if (FLOAT_EQUALS (val, 0.0f)) {
		return (WebPlaylistWindow::itemDisplayDurations[0]);
	}
	if (FLOAT_EQUALS (val, 0.25f)) {
		return (WebPlaylistWindow::itemDisplayDurations[1]);
	}
	if (FLOAT_EQUALS (val, 0.5f)) {
		return (WebPlaylistWindow::itemDisplayDurations[2]);
	}
	if (FLOAT_EQUALS (val, 0.75f)) {
		return (WebPlaylistWindow::itemDisplayDurations[3]);
	}
	if (FLOAT_EQUALS (val, 1.0f)) {
		return (WebPlaylistWindow::itemDisplayDurations[4]);
	}

	return (WebPlaylistWindow::itemDisplayDurations[2]);
}

static const char *PLAYLIST_NAME_KEY = "a";
static const char *IS_SELECTED_KEY = "b";
static const char *IS_EXPANDED_KEY = "c";
static const char *URL_LIST_KEY = "d";
static const char *IS_SHUFFLE_KEY = "e";
static const char *ITEM_DISPLAY_DURATION_KEY = "f";
Json *WebPlaylistWindow::getState () {
	Json *obj;
	StringList urls;

	obj = new Json ();
	urlListView->getItems (&urls);
	obj->set (PLAYLIST_NAME_KEY, playlistName);
	obj->set (URL_LIST_KEY, &urls);
	obj->set (IS_SHUFFLE_KEY, shuffleToggle->isChecked);
	obj->set (ITEM_DISPLAY_DURATION_KEY, getItemDisplayDuration ());
	obj->set (IS_SELECTED_KEY, isSelected);
	obj->set (IS_EXPANDED_KEY, isExpanded);

	return (obj);
}

void WebPlaylistWindow::setState (Json *stateObject) {
	StdString val;
	StringList urls;
	int duration;
	bool b;

	val = stateObject->getString (PLAYLIST_NAME_KEY, "");
	if (! val.empty ()) {
		setPlaylistName (val);
	}

	stateObject->getStringList (URL_LIST_KEY, &urls);
	urlListView->setItems (&urls);

	duration = stateObject->getNumber (ITEM_DISPLAY_DURATION_KEY, WebPlaylistWindow::itemDisplayDurations[2]);
	if (duration == WebPlaylistWindow::itemDisplayDurations[0]) {
		itemDisplayDurationSlider->setValue (0.0f);
	}
	else if (duration == WebPlaylistWindow::itemDisplayDurations[1]) {
		itemDisplayDurationSlider->setValue (0.25f);
	}
	else if (duration == WebPlaylistWindow::itemDisplayDurations[2]) {
		itemDisplayDurationSlider->setValue (0.5f);
	}
	else if (duration == WebPlaylistWindow::itemDisplayDurations[3]) {
		itemDisplayDurationSlider->setValue (0.75f);
	}
	else if (duration == WebPlaylistWindow::itemDisplayDurations[4]) {
		itemDisplayDurationSlider->setValue (1.0f);
	}

	shuffleToggle->setChecked (stateObject->getBoolean (IS_SHUFFLE_KEY, false));

	b = stateObject->getBoolean (IS_SELECTED_KEY, false);
	isSelected = ! b;
	setSelected (b, true);

	b = stateObject->getBoolean (IS_EXPANDED_KEY, false);
	isExpanded = ! b;
	setExpanded (b, true);
}

Json *WebPlaylistWindow::getCreateCommand () {
	Json *obj;
	StringList urls;

	obj = new Json ();
	urlListView->getItems (&urls);
	obj->set ("displayName", playlistName);
	obj->set ("urls", &urls);
	obj->set ("isShuffle", shuffleToggle->isChecked);
	obj->set ("minItemDisplayDuration", getItemDisplayDuration ());
	obj->set ("maxItemDisplayDuration", getItemDisplayDuration ());

	return (App::getInstance ()->createCommand ("CreateWebDisplayIntent", SystemInterface::Constant_Monitor, obj));
}
