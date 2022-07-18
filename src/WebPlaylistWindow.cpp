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
#include "ClassId.h"
#include "StdString.h"
#include "UiConfiguration.h"
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
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "SystemInterface.h"
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
	classId = ClassId::WebPlaylistWindow;

	setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	windowWidth = App::instance->windowWidth * WebPlaylistWindow::WindowWidthMultiplier;

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::PlaylistIconSprite)));
	nameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor)));
	nameLabel->mouseClickCallback = Widget::EventCallbackContext (WebPlaylistWindow::nameLabelClicked, this);
	nameLabel->setPadding (0.0f, 0.0f);
	nameLabel->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ClickRenameTooltip));

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle ()));
	shuffleToggle->setIcon (sprites ? sprites->getSprite (WebKioskUi::ShuffleIconSprite) : UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallErrorIconSprite));
	shuffleToggle->setRightAligned (true);
	shuffleToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	shuffleToggle->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ShuffleTooltip));
	shuffleToggle->zLevel = 1;
	shuffleToggle->isVisible = false;

	itemDisplayDurationSlider = (SliderWindow *) addWidget (new SliderWindow (new Slider ()));
	itemDisplayDurationSlider->addSnapValue (0.0f);
	itemDisplayDurationSlider->addSnapValue (0.25f);
	itemDisplayDurationSlider->addSnapValue (0.5f);
	itemDisplayDurationSlider->addSnapValue (0.75f);
	itemDisplayDurationSlider->addSnapValue (1.0f);
	itemDisplayDurationSlider->setPadding (UiConfiguration::instance->paddingSize, 0.0f);
	itemDisplayDurationSlider->setTrackWidthScale (0.75f);
	itemDisplayDurationSlider->setValueNameFunction (WebPlaylistWindow::itemDisplayDurationSliderValueName);
	itemDisplayDurationSlider->setIcon (sprites ? sprites->getSprite (WebKioskUi::SpeedIconSprite) : UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallErrorIconSprite));
	itemDisplayDurationSlider->setMouseHoverTooltip (UiText::instance->getText (UiTextString::PlaylistSpeedTooltip));
	itemDisplayDurationSlider->zLevel = 1;
	itemDisplayDurationSlider->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->stateChangeCallback = Widget::EventCallbackContext (WebPlaylistWindow::selectToggleStateChanged, this);
	selectToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	selectToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::UnselectedToggleTooltip), UiText::instance->getText (UiTextString::SelectedToggleTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (WebPlaylistWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());

	urlCountLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite), StdString ("0"), UiConfiguration::TitleFont, UiConfiguration::instance->lightPrimaryTextColor));
	urlCountLabel->setPadding (0.0f, 0.0f);
	urlCountLabel->setMouseHoverTooltip (UiText::instance->getCountText (0, UiTextString::WebPlaylistItem, UiTextString::WebPlaylistItems));
	urlCountLabel->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);

	urlListView = (ListView *) addWidget (new ListView (windowWidth - (widthPadding * 2.0f), 8, 8, UiConfiguration::CaptionFont, UiText::instance->getText (UiTextString::EmptyWebPlaylistAddressListPrompt)));
	urlListView->listChangeCallback = Widget::EventCallbackContext (WebPlaylistWindow::urlListChanged, this);
	urlListView->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	urlListView->isVisible = false;

	removeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (WebPlaylistWindow::removeButtonClicked, this);
	removeButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::DeletePlaylist).capitalized ());
	removeButton->isVisible = false;

	addItemButton = (Button *) addWidget (new Button (sprites ? sprites->getSprite (WebKioskUi::AddUrlButtonSprite) : UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::OkButtonSprite)));
	addItemButton->mouseClickCallback = Widget::EventCallbackContext (WebPlaylistWindow::addItemButtonClicked, this);
	addItemButton->mouseEnterCallback = Widget::EventCallbackContext (WebPlaylistWindow::addItemButtonMouseEntered, this);
	addItemButton->mouseExitCallback = Widget::EventCallbackContext (WebPlaylistWindow::addItemButtonMouseExited, this);
	addItemButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
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
	if (selected == isSelected) {
		return;
	}
	isSelected = selected;
	selectToggle->setChecked (isSelected, shouldSkipStateChangeCallback);
	if (isSelected) {
		setCornerRadius (0, UiConfiguration::instance->cornerRadius, 0, UiConfiguration::instance->cornerRadius);
	}
	else {
		setCornerRadius (UiConfiguration::instance->cornerRadius);
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
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
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
		setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
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
	float x, y, x0, y0, x2, y2;

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

	x = x2 + UiConfiguration::instance->marginSize;
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
	y = y2 + UiConfiguration::instance->marginSize;
	y0 = y;
	if (isExpanded) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
		urlListView->flowDown (x, &y, &x2, &y2);

		y = y2 + UiConfiguration::instance->marginSize;
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

		dividerPanel->setFixedSize (true, windowWidth, UiConfiguration::instance->headlineDividerLineWidth);

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
	float w;

	if (isExpanded) {
		w = removeButton->position.x;
		w -= nameLabel->position.x;
		w -= UiConfiguration::instance->marginSize;
		nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::HeadlineFont]->truncatedText (playlistName, w, Font::DotTruncateSuffix));
	}
	else {
		w = windowWidth * 0.63f;
		w -= (iconImage->width + expandToggle->width + selectToggle->width);
		nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::BodyFont]->truncatedText (playlistName, w, Font::DotTruncateSuffix));
	}
	refreshLayout ();
}

void WebPlaylistWindow::doRefresh () {
	windowWidth = App::instance->windowWidth * WebPlaylistWindow::WindowWidthMultiplier;
	urlListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
	Panel::doRefresh ();
}

void WebPlaylistWindow::nameLabelClicked (void *windowPtr, Widget *widgetPtr) {
	((WebPlaylistWindow *) windowPtr)->eventCallback (((WebPlaylistWindow *) windowPtr)->nameClickCallback);
}

void WebPlaylistWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Toggle *toggle;

	window = (WebPlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;

	window->isSelected = toggle->isChecked;
	if (window->isSelected) {
		window->setCornerRadius (0, UiConfiguration::instance->cornerRadius, 0, UiConfiguration::instance->cornerRadius);
	}
	else {
		window->setCornerRadius (UiConfiguration::instance->cornerRadius);
	}
	window->eventCallback (window->selectStateChangeCallback);
}

void WebPlaylistWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Toggle *toggle;

	window = (WebPlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	window->eventCallback (window->expandStateChangeCallback);
}

void WebPlaylistWindow::urlListChanged (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	int count;

	window = (WebPlaylistWindow *) windowPtr;
	count = window->urlListView->getItemCount ();
	window->urlCountLabel->setText (StdString::createSprintf ("%i", count));
	window->urlCountLabel->tooltipText.assign (UiText::instance->getCountText (count, UiTextString::WebPlaylistItem, UiTextString::WebPlaylistItems));
	window->refreshLayout ();
	window->eventCallback (window->urlListChangeCallback);
}

void WebPlaylistWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((WebPlaylistWindow *) windowPtr)->eventCallback (((WebPlaylistWindow *) windowPtr)->removeClickCallback);
}

void WebPlaylistWindow::addItemButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((WebPlaylistWindow *) windowPtr)->eventCallback (((WebPlaylistWindow *) windowPtr)->addItemClickCallback);
}

void WebPlaylistWindow::addItemButtonMouseEntered (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Button *button;

	window = (WebPlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseEntered (button, button);
	window->eventCallback (window->addItemMouseEnterCallback);
}

void WebPlaylistWindow::addItemButtonMouseExited (void *windowPtr, Widget *widgetPtr) {
	WebPlaylistWindow *window;
	Button *button;

	window = (WebPlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);
	window->eventCallback (window->addItemMouseExitCallback);
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
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Slowest).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[0] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Slow).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[1] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Medium).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[2] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Fast).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[3] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Fastest).capitalized ().c_str (), OsUtil::getDurationDisplayString (WebPlaylistWindow::ItemDisplayDurations[4] * 1000).c_str ()));
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
	state->set (WebPlaylistWindow::UrlsKey, urls);
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
	obj->set ("urls", urls);

	obj->set ("isShuffle", shuffleToggle->isChecked);
	obj->set ("minItemDisplayDuration", getItemDisplayDuration ());
	obj->set ("maxItemDisplayDuration", getItemDisplayDuration ());

	return (App::instance->createCommand (SystemInterface::Command_CreateWebDisplayIntent, obj));
}
