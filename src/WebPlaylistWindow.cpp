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
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "OsUtil.h"
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

const float WebPlaylistWindow::WindowWidthMultiplier = 0.45f;
const int WebPlaylistWindow::ItemDisplayDurations[] = { 3 * 3600, 3600, 900, 420, 180 };
const char *WebPlaylistWindow::PlaylistNameKey = "a";
const char *WebPlaylistWindow::IsSelectedKey = "b";
const char *WebPlaylistWindow::IsExpandedKey = "c";
const char *WebPlaylistWindow::UrlsKey = "d";
const char *WebPlaylistWindow::IsShuffleKey = "e";
const char *WebPlaylistWindow::ItemDisplayDurationKey = "f";

WebPlaylistWindow::WebPlaylistWindow (SpriteGroup *webKioskUiSpriteGroup)
: Panel ()
, isSelected (false)
, isExpanded (false)
, windowWidth (0.0f)
, sprites (webKioskUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, dividerPanel (NULL)
, urlCountLabel (NULL)
, shuffleToggle (NULL)
, itemDisplayDurationSlider (NULL)
, urlListView (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, removeButton (NULL)
, addItemButton (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::WebPlaylistWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
	setCornerRadius (uiconfig->cornerRadius);
	setFillBg (true, uiconfig->mediumBackgroundColor);
	windowWidth = App::instance->windowWidth * WebPlaylistWindow::WindowWidthMultiplier;

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::PlaylistIconSprite)));
	nameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor)));
	nameLabel->mouseClickCallback = Widget::EventCallbackContext (WebPlaylistWindow::nameLabelClicked, this);
	nameLabel->setPadding (0.0f, 0.0f);
	nameLabel->setMouseHoverTooltip (uitext->getText (UiTextString::ClickRenameTooltip));

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, uiconfig->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, uiconfig->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle ()));
	shuffleToggle->setIcon (sprites->getSprite (WebKioskUi::ShuffleIconSprite));
	shuffleToggle->setRightAligned (true);
	shuffleToggle->setImageColor (uiconfig->flatButtonTextColor);
	shuffleToggle->setMouseHoverTooltip (uitext->getText (UiTextString::ShuffleTooltip));
	shuffleToggle->zLevel = 1;
	shuffleToggle->isVisible = false;

	itemDisplayDurationSlider = (SliderWindow *) addWidget (new SliderWindow (new Slider ()));
	itemDisplayDurationSlider->addSnapValue (0.0f);
	itemDisplayDurationSlider->addSnapValue (0.25f);
	itemDisplayDurationSlider->addSnapValue (0.5f);
	itemDisplayDurationSlider->addSnapValue (0.75f);
	itemDisplayDurationSlider->addSnapValue (1.0f);
	itemDisplayDurationSlider->setPadding (uiconfig->paddingSize, 0.0f);
	itemDisplayDurationSlider->setTrackWidthScale (0.75f);
	itemDisplayDurationSlider->setValueNameFunction (WebPlaylistWindow::itemDisplayDurationSliderValueName);
	itemDisplayDurationSlider->setIcon (sprites->getSprite (WebKioskUi::SpeedIconSprite));
	itemDisplayDurationSlider->setMouseHoverTooltip (uitext->getText (UiTextString::PlaylistSpeedTooltip));
	itemDisplayDurationSlider->zLevel = 1;
	itemDisplayDurationSlider->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->stateChangeCallback = Widget::EventCallbackContext (WebPlaylistWindow::selectToggleStateChanged, this);
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::UnselectedToggleTooltip), uitext->getText (UiTextString::SelectedToggleTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (WebPlaylistWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::Expand).capitalized (), uitext->getText (UiTextString::Minimize).capitalized ());

	urlCountLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite), StdString ("0"), UiConfiguration::TitleFont, uiconfig->lightPrimaryTextColor));
	urlCountLabel->setPadding (0.0f, 0.0f);
	urlCountLabel->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::WebPlaylistItem, UiTextString::WebPlaylistItems));
	urlCountLabel->setTextChangeHighlight (true, uiconfig->primaryTextColor);

	urlListView = (ListView *) addWidget (new ListView (windowWidth - (widthPadding * 2.0f), 8, 8, UiConfiguration::CaptionFont, uitext->getText (UiTextString::EmptyWebPlaylistAddressListPrompt)));
	urlListView->listChangeCallback = Widget::EventCallbackContext (WebPlaylistWindow::urlListChanged, this);
	urlListView->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	urlListView->isVisible = false;

	removeButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (WebPlaylistWindow::removeButtonClicked, this);
	removeButton->setImageColor (uiconfig->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (uitext->getText (UiTextString::DeletePlaylist).capitalized ());
	removeButton->isVisible = false;

	addItemButton = (Button *) addWidget (new Button (sprites->getSprite (WebKioskUi::AddUrlButtonSprite)));
	addItemButton->mouseClickCallback = Widget::EventCallbackContext (WebPlaylistWindow::addItemButtonClicked, this);
	addItemButton->mouseEnterCallback = Widget::EventCallbackContext (WebPlaylistWindow::addItemButtonMouseEntered, this);
	addItemButton->mouseExitCallback = Widget::EventCallbackContext (WebPlaylistWindow::addItemButtonMouseExited, this);
	addItemButton->setImageColor (uiconfig->flatButtonTextColor);
	addItemButton->isVisible = false;

	refreshLayout ();
}

WebPlaylistWindow::~WebPlaylistWindow () {

}

StdString WebPlaylistWindow::toStringDetail () {
	return (StdString (" WebPlaylistWindow"));
}

bool WebPlaylistWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::WebPlaylistWindow));
}

WebPlaylistWindow *WebPlaylistWindow::castWidget (Widget *widget) {
	return (WebPlaylistWindow::isWidgetType (widget) ? (WebPlaylistWindow *) widget : NULL);
}

void WebPlaylistWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
	UiConfiguration *uiconfig;

	if (selected == isSelected) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isSelected = selected;
	selectToggle->setChecked (isSelected, shouldSkipStateChangeCallback);
	if (isSelected) {
		setCornerRadius (0, uiconfig->cornerRadius, 0, uiconfig->cornerRadius);
	}
	else {
		setCornerRadius (uiconfig->cornerRadius);
	}
	refreshLayout ();
}

void WebPlaylistWindow::setPlaylistName (const StdString &name) {
	playlistName.assign (name);
	nameLabel->setText (playlistName);
	refreshLayout ();
	resetNameLabel ();
}

void WebPlaylistWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	UiConfiguration *uiconfig;

	if (expanded == isExpanded) {
		return;
	}
	uiconfig = &(App::instance->uiConfig);
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
		urlCountLabel->setTextFont (UiConfiguration::HeadlineFont);
		dividerPanel->isVisible = true;
		urlListView->isVisible = true;
		shuffleToggle->isVisible = true;
		itemDisplayDurationSlider->isVisible = true;
		removeButton->isVisible = true;
		addItemButton->isVisible = true;
	}
	else {
		setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
		nameLabel->setFont (UiConfiguration::BodyFont);
		urlCountLabel->setTextFont (UiConfiguration::BodyFont);
		dividerPanel->isVisible = false;
		urlListView->isVisible = false;
		shuffleToggle->isVisible = false;
		itemDisplayDurationSlider->isVisible = false;
		removeButton->isVisible = false;
		addItemButton->isVisible = false;
	}
	expandToggle->setChecked (isExpanded, shouldSkipStateChangeCallback);
	refreshLayout ();
	resetNameLabel ();
}

void WebPlaylistWindow::refreshLayout () {
	UiConfiguration *uiconfig;
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
	if (! isExpanded) {
		urlCountLabel->flowRight (&x, y, &x2, &y2);
	}

	x = x2 + uiconfig->marginSize;
	y = y0;
	if (isExpanded) {
		removeButton->flowRight (&x, y, &x2, &y2);
	}
	expandToggle->flowRight (&x, y, &x2, &y2);
	selectToggle->flowRight (&x, y, &x2, &y2);
	if (isExpanded) {
		nameLabel->centerVertical (y0, y2);
	}

	x = x0;
	x2 = 0.0f;
	y = y2 + uiconfig->marginSize;
	y0 = y;
	if (isExpanded) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
		urlListView->flowDown (x, &y, &x2, &y2);

		y = y2 + uiconfig->marginSize;
		y0 = y;
		shuffleToggle->flowRight (&x, y, &x2, &y2);
		itemDisplayDurationSlider->flowRight (&x, y, &x2, &y2);
		urlCountLabel->flowRight (&x, y, &x2, &y2);
		addItemButton->flowRight (&x, y, &x2, &y2);

		shuffleToggle->centerVertical (y0, y2);
		itemDisplayDurationSlider->centerVertical (y0, y2);
		urlCountLabel->centerVertical (y0, y2);
		addItemButton->centerVertical (y0, y2);

		setFixedSize (true, windowWidth, y2 + heightPadding);

		dividerPanel->setFixedSize (true, windowWidth, uiconfig->headlineDividerLineWidth);

		x = width - widthPadding;
		addItemButton->flowLeft (&x);
		urlCountLabel->flowLeft (&x);
	}
	else {
		setFixedSize (false);
		resetSize ();
	}

	x = width - widthPadding;
	selectToggle->flowLeft (&x);
	expandToggle->flowLeft (&x);
	if (isExpanded) {
		removeButton->flowLeft (&x);
	}
}

void WebPlaylistWindow::resetNameLabel () {
	UiConfiguration *uiconfig;
	float w;

	uiconfig = &(App::instance->uiConfig);
	if (isExpanded) {
		w = removeButton->position.x;
		w -= nameLabel->position.x;
		w -= uiconfig->marginSize;
		nameLabel->setText (Label::getTruncatedText (playlistName, UiConfiguration::HeadlineFont, w, Label::DotTruncateSuffix));
	}
	else {
		w = windowWidth * 0.63f;
		w -= (iconImage->width + expandToggle->width + selectToggle->width);
		nameLabel->setText (Label::getTruncatedText (playlistName, UiConfiguration::BodyFont, w, Label::DotTruncateSuffix));
	}
	refreshLayout ();
}

void WebPlaylistWindow::doRefresh () {
	windowWidth = App::instance->windowWidth * WebPlaylistWindow::WindowWidthMultiplier;
	urlListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
	Panel::doRefresh ();
}

void WebPlaylistWindow::nameLabelClicked (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;

	window = (WebPlaylistWindow *) windowPtr;
	if (window->nameClickCallback.callback) {
		window->nameClickCallback.callback (window->nameClickCallback.callbackData, window);
	}
}

void WebPlaylistWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Toggle *toggle;
	UiConfiguration *uiconfig;

	window = (WebPlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	uiconfig = &(App::instance->uiConfig);

	window->isSelected = toggle->isChecked;
	if (window->isSelected) {
		window->setCornerRadius (0, uiconfig->cornerRadius, 0, uiconfig->cornerRadius);
	}
	else {
		window->setCornerRadius (uiconfig->cornerRadius);
	}
	if (window->selectStateChangeCallback.callback) {
		window->selectStateChangeCallback.callback (window->selectStateChangeCallback.callbackData, window);
	}
}

void WebPlaylistWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Toggle *toggle;

	window = (WebPlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback.callback) {
		window->expandStateChangeCallback.callback (window->expandStateChangeCallback.callbackData, window);
	}
}

void WebPlaylistWindow::urlListChanged (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	UiText *uitext;
	int count;

	window = (WebPlaylistWindow *) windowPtr;
	uitext = &(App::instance->uiText);
	count = window->urlListView->getItemCount ();
	window->urlCountLabel->setText (StdString::createSprintf ("%i", count));
	window->urlCountLabel->tooltipText.assign (uitext->getCountText (count, UiTextString::WebPlaylistItem, UiTextString::WebPlaylistItems));
	window->refreshLayout ();

	if (window->urlListChangeCallback.callback) {
		window->urlListChangeCallback.callback (window->urlListChangeCallback.callbackData, window);
	}
}

void WebPlaylistWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;

	window = (WebPlaylistWindow *) windowPtr;
	if (window->removeClickCallback.callback) {
		window->removeClickCallback.callback (window->removeClickCallback.callbackData, window);
	}
}

void WebPlaylistWindow::addItemButtonClicked (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;

	window = (WebPlaylistWindow *) windowPtr;
	if (window->addItemClickCallback.callback) {
		window->addItemClickCallback.callback (window->addItemClickCallback.callbackData, window);
	}
}

void WebPlaylistWindow::addItemButtonMouseEntered (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Button *button;

	window = (WebPlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseEntered (button, button);
	if (window->addItemMouseEnterCallback.callback) {
		window->addItemMouseEnterCallback.callback (window->addItemMouseEnterCallback.callbackData, window);
	}
}

void WebPlaylistWindow::addItemButtonMouseExited (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Button *button;

	window = (WebPlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);
	if (window->addItemMouseExitCallback.callback) {
		window->addItemMouseExitCallback.callback (window->addItemMouseExitCallback.callbackData, window);
	}
}

int WebPlaylistWindow::getItemCount () {
	return (urlListView->getItemCount ());
}

void WebPlaylistWindow::addUrl (const StdString &url) {
	urlListView->addItem (url);
	urlListView->scrollToBottom ();
}

Widget::Rectangle WebPlaylistWindow::getAddItemButtonScreenRect () {
	return (addItemButton->getScreenRect ());
}

Widget::Rectangle WebPlaylistWindow::getRemoveButtonScreenRect () {
	return (removeButton->getScreenRect ());
}

StdString WebPlaylistWindow::itemDisplayDurationSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Slowest).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[0] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Slow).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[1] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Medium).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[2] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Fast).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[3] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Fastest).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[4] * 1000).c_str ()));
	}

	return (StdString (""));
}

int WebPlaylistWindow::getItemDisplayDuration () {
	float val;

	val = itemDisplayDurationSlider->value;
	if (FLOAT_EQUALS (val, 0.0f)) {
		return (WebPlaylistWindow::ItemDisplayDurations[0]);
	}
	if (FLOAT_EQUALS (val, 0.25f)) {
		return (WebPlaylistWindow::ItemDisplayDurations[1]);
	}
	if (FLOAT_EQUALS (val, 0.5f)) {
		return (WebPlaylistWindow::ItemDisplayDurations[2]);
	}
	if (FLOAT_EQUALS (val, 0.75f)) {
		return (WebPlaylistWindow::ItemDisplayDurations[3]);
	}
	if (FLOAT_EQUALS (val, 1.0f)) {
		return (WebPlaylistWindow::ItemDisplayDurations[4]);
	}

	return (WebPlaylistWindow::ItemDisplayDurations[2]);
}

Json *WebPlaylistWindow::createState () {
	Json *state;
	StringList urls;

	state = new Json ();
	urlListView->getItems (&urls);
	state->set (WebPlaylistWindow::PlaylistNameKey, playlistName);
	state->set (WebPlaylistWindow::UrlsKey, &urls);
	state->set (WebPlaylistWindow::IsShuffleKey, shuffleToggle->isChecked);
	state->set (WebPlaylistWindow::ItemDisplayDurationKey, getItemDisplayDuration ());
	state->set (WebPlaylistWindow::IsSelectedKey, isSelected);
	state->set (WebPlaylistWindow::IsExpandedKey, isExpanded);

	return (state);
}

void WebPlaylistWindow::readState (Json *state) {
	StringList urls;
	int duration;
	bool b;

	state->getStringList (WebPlaylistWindow::UrlsKey, &urls);
	urlListView->setItems (&urls);

	duration = state->getNumber (WebPlaylistWindow::ItemDisplayDurationKey, WebPlaylistWindow::ItemDisplayDurations[2]);
	if (duration == WebPlaylistWindow::ItemDisplayDurations[0]) {
		itemDisplayDurationSlider->setValue (0.0f);
	}
	else if (duration == WebPlaylistWindow::ItemDisplayDurations[1]) {
		itemDisplayDurationSlider->setValue (0.25f);
	}
	else if (duration == WebPlaylistWindow::ItemDisplayDurations[2]) {
		itemDisplayDurationSlider->setValue (0.5f);
	}
	else if (duration == WebPlaylistWindow::ItemDisplayDurations[3]) {
		itemDisplayDurationSlider->setValue (0.75f);
	}
	else if (duration == WebPlaylistWindow::ItemDisplayDurations[4]) {
		itemDisplayDurationSlider->setValue (1.0f);
	}

	shuffleToggle->setChecked (state->getBoolean (WebPlaylistWindow::IsShuffleKey, false), true);

	b = state->getBoolean (WebPlaylistWindow::IsSelectedKey, false);
	isSelected = ! b;
	setSelected (b, true);

	b = state->getBoolean (WebPlaylistWindow::IsExpandedKey, false);
	isExpanded = ! b;
	setExpanded (b, true);

	setPlaylistName (state->getString (WebPlaylistWindow::PlaylistNameKey, ""));
	refreshLayout ();
}

Json *WebPlaylistWindow::createCommand () {
	Json *obj;
	StringList urls;

	obj = new Json ();
	obj->set ("displayName", playlistName);

	urlListView->getItems (&urls);
	obj->set ("urls", &urls);

	obj->set ("isShuffle", shuffleToggle->isChecked);
	obj->set ("minItemDisplayDuration", getItemDisplayDuration ());
	obj->set ("maxItemDisplayDuration", getItemDisplayDuration ());

	return (App::instance->createCommand (SystemInterface::Command_CreateWebDisplayIntent, SystemInterface::Constant_Monitor, obj));
}
