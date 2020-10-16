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
#include "CameraUi.h"
#include "CapturePlaylistWindow.h"

const float CapturePlaylistWindow::WindowWidthMultiplier = 0.45f;
const int CapturePlaylistWindow::ItemDisplayDurations[] = { 3 * 3600, 3600, 900, 420, 180 };
const char *CapturePlaylistWindow::PlaylistNameKey = "a";
const char *CapturePlaylistWindow::IsSelectedKey = "b";
const char *CapturePlaylistWindow::IsExpandedKey = "c";
const char *CapturePlaylistWindow::ItemListKey = "d";
const char *CapturePlaylistWindow::IsShuffleKey = "e";
const char *CapturePlaylistWindow::ItemDisplayDurationKey = "f";
const char *CapturePlaylistWindow::HostnameKey = "g";
const char *CapturePlaylistWindow::AuthorizePathKey = "h";
const char *CapturePlaylistWindow::AuthorizeSecretKey = "i";
const char *CapturePlaylistWindow::AuthorizeTokenKey = "j";
const char *CapturePlaylistWindow::CaptureIdKey = "k";
const char *CapturePlaylistWindow::CaptureNameKey = "l";

CapturePlaylistWindow::CapturePlaylistWindow (SpriteGroup *cameraUiSpriteGroup)
: Panel ()
, isSelected (false)
, isExpanded (false)
, windowWidth (0.0f)
, sprites (cameraUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, dividerPanel (NULL)
, itemCountLabel (NULL)
, shuffleToggle (NULL)
, itemDisplayDurationSlider (NULL)
, itemListView (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, removeButton (NULL)
, addItemButton (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::CapturePlaylistWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
	setCornerRadius (uiconfig->cornerRadius);
	setFillBg (true, uiconfig->mediumBackgroundColor);
	windowWidth = App::instance->windowWidth * CapturePlaylistWindow::WindowWidthMultiplier;

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::PlaylistIconSprite)));
	nameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor)));
	nameLabel->mouseClickCallback = Widget::EventCallbackContext (CapturePlaylistWindow::nameLabelClicked, this);
	nameLabel->setPadding (0.0f, 0.0f);
	nameLabel->setMouseHoverTooltip (uitext->getText (UiTextString::ClickRenameTooltip));

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, uiconfig->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, uiconfig->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle ()));
	shuffleToggle->setIcon (sprites->getSprite (CameraUi::ShuffleIconSprite));
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
	itemDisplayDurationSlider->setValueNameFunction (CapturePlaylistWindow::itemDisplayDurationSliderValueName);
	itemDisplayDurationSlider->setIcon (sprites->getSprite (CameraUi::SpeedIconSprite));
	itemDisplayDurationSlider->setMouseHoverTooltip (uitext->getText (UiTextString::PlaylistSpeedTooltip));
	itemDisplayDurationSlider->zLevel = 1;
	itemDisplayDurationSlider->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->stateChangeCallback = Widget::EventCallbackContext (CapturePlaylistWindow::selectToggleStateChanged, this);
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::UnselectedToggleTooltip), uitext->getText (UiTextString::SelectedToggleTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (CapturePlaylistWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::Expand).capitalized (), uitext->getText (UiTextString::Minimize).capitalized ());

	itemCountLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (CameraUi::TimelapseIconSprite), StdString ("0"), UiConfiguration::TitleFont, uiconfig->lightPrimaryTextColor));
	itemCountLabel->setPadding (0.0f, 0.0f);
	itemCountLabel->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::CapturePlaylistItem, UiTextString::CapturePlaylistItems));
	itemCountLabel->setTextChangeHighlight (true, uiconfig->primaryTextColor);

	itemListView = (ListView *) addWidget (new ListView (windowWidth - (widthPadding * 2.0f), 4, 7, UiConfiguration::CaptionFont, uitext->getText (UiTextString::EmptyCaptureListPrompt)));
	itemListView->listChangeCallback = Widget::EventCallbackContext (CapturePlaylistWindow::itemListChanged, this);
	itemListView->setDropShadow (true, uiconfig->dropShadowColor, uiconfig->dropShadowWidth);
	itemListView->isVisible = false;

	removeButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (CapturePlaylistWindow::removeButtonClicked, this);
	removeButton->setImageColor (uiconfig->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (uitext->getText (UiTextString::DeletePlaylist).capitalized ());
	removeButton->isVisible = false;

	addItemButton = (Button *) addWidget (new Button (sprites->getSprite (CameraUi::AddPlaylistItemButtonSprite)));
	addItemButton->mouseClickCallback = Widget::EventCallbackContext (CapturePlaylistWindow::addItemButtonClicked, this);
	addItemButton->mouseEnterCallback = Widget::EventCallbackContext (CapturePlaylistWindow::addItemButtonMouseEntered, this);
	addItemButton->mouseExitCallback = Widget::EventCallbackContext (CapturePlaylistWindow::addItemButtonMouseExited, this);
	addItemButton->setImageColor (uiconfig->flatButtonTextColor);
	addItemButton->isVisible = false;

	refreshLayout ();
}

CapturePlaylistWindow::~CapturePlaylistWindow () {

}

StdString CapturePlaylistWindow::toStringDetail () {
	return (StdString (" CapturePlaylistWindow"));
}

bool CapturePlaylistWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::CapturePlaylistWindow));
}

CapturePlaylistWindow *CapturePlaylistWindow::castWidget (Widget *widget) {
	return (CapturePlaylistWindow::isWidgetType (widget) ? (CapturePlaylistWindow *) widget : NULL);
}

void CapturePlaylistWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
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

void CapturePlaylistWindow::setPlaylistName (const StdString &name) {
	playlistName.assign (name);
	nameLabel->setText (playlistName);
	refreshLayout ();
	resetNameLabel ();
}

void CapturePlaylistWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	UiConfiguration *uiconfig;

	if (expanded == isExpanded) {
		return;
	}
	uiconfig = &(App::instance->uiConfig);
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
		itemCountLabel->setTextFont (UiConfiguration::HeadlineFont);
		dividerPanel->isVisible = true;
		itemListView->isVisible = true;
		shuffleToggle->isVisible = true;
		itemDisplayDurationSlider->isVisible = true;
		removeButton->isVisible = true;
		addItemButton->isVisible = true;
	}
	else {
		setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
		nameLabel->setFont (UiConfiguration::BodyFont);
		itemCountLabel->setTextFont (UiConfiguration::BodyFont);
		dividerPanel->isVisible = false;
		itemListView->isVisible = false;
		shuffleToggle->isVisible = false;
		itemDisplayDurationSlider->isVisible = false;
		removeButton->isVisible = false;
		addItemButton->isVisible = false;
	}
	expandToggle->setChecked (isExpanded, shouldSkipStateChangeCallback);
	refreshLayout ();
	resetNameLabel ();
}

void CapturePlaylistWindow::refreshLayout () {
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
		itemCountLabel->flowRight (&x, y, &x2, &y2);
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
		itemListView->flowDown (x, &y, &x2, &y2);

		y = y2 + uiconfig->marginSize;
		y0 = y;
		shuffleToggle->flowRight (&x, y, &x2, &y2);
		itemDisplayDurationSlider->flowRight (&x, y, &x2, &y2);
		itemCountLabel->flowRight (&x, y, &x2, &y2);
		addItemButton->flowRight (&x, y, &x2, &y2);

		shuffleToggle->centerVertical (y0, y2);
		itemDisplayDurationSlider->centerVertical (y0, y2);
		itemCountLabel->centerVertical (y0, y2);
		addItemButton->centerVertical (y0, y2);

		setFixedSize (true, windowWidth, y2 + heightPadding);

		dividerPanel->setFixedSize (true, windowWidth, uiconfig->headlineDividerLineWidth);

		x = width - widthPadding;
		addItemButton->flowLeft (&x);
		itemCountLabel->flowLeft (&x);
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

void CapturePlaylistWindow::resetNameLabel () {
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

void CapturePlaylistWindow::doRefresh () {
	windowWidth = App::instance->windowWidth * CapturePlaylistWindow::WindowWidthMultiplier;
	itemListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
	Panel::doRefresh ();
}

void CapturePlaylistWindow::nameLabelClicked (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;

	window = (CapturePlaylistWindow *) windowPtr;
	if (window->nameClickCallback.callback) {
		window->nameClickCallback.callback (window->nameClickCallback.callbackData, window);
	}
}

void CapturePlaylistWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	Toggle *toggle;
	UiConfiguration *uiconfig;

	window = (CapturePlaylistWindow *) windowPtr;
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

void CapturePlaylistWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	Toggle *toggle;

	window = (CapturePlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback.callback) {
		window->expandStateChangeCallback.callback (window->expandStateChangeCallback.callbackData, window);
	}
}

void CapturePlaylistWindow::itemListChanged (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	UiText *uitext;
	int count;

	window = (CapturePlaylistWindow *) windowPtr;
	uitext = &(App::instance->uiText);
	count = window->itemListView->getItemCount ();
	window->itemCountLabel->setText (StdString::createSprintf ("%i", count));
	window->itemCountLabel->tooltipText.assign (uitext->getCountText (count, UiTextString::CapturePlaylistItem, UiTextString::CapturePlaylistItems));
	window->refreshLayout ();

	if (window->itemListChangeCallback.callback) {
		window->itemListChangeCallback.callback (window->itemListChangeCallback.callbackData, window);
	}
}

void CapturePlaylistWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;

	window = (CapturePlaylistWindow *) windowPtr;
	if (window->removeClickCallback.callback) {
		window->removeClickCallback.callback (window->removeClickCallback.callbackData, window);
	}
}

void CapturePlaylistWindow::addItemButtonClicked (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;

	window = (CapturePlaylistWindow *) windowPtr;
	if (window->addItemClickCallback.callback) {
		window->addItemClickCallback.callback (window->addItemClickCallback.callbackData, window);
	}
}

void CapturePlaylistWindow::addItemButtonMouseEntered (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	Button *button;

	window = (CapturePlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseEntered (button, button);
	if (window->addItemMouseEnterCallback.callback) {
		window->addItemMouseEnterCallback.callback (window->addItemMouseEnterCallback.callbackData, window);
	}
}

void CapturePlaylistWindow::addItemButtonMouseExited (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	Button *button;

	window = (CapturePlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);
	if (window->addItemMouseExitCallback.callback) {
		window->addItemMouseExitCallback.callback (window->addItemMouseExitCallback.callbackData, window);
	}
}

int CapturePlaylistWindow::getItemCount () {
	return (itemListView->getItemCount ());
}

void CapturePlaylistWindow::addItem (const CapturePlaylistWindow::Item &item) {
	addItem (item.hostname, item.authorizePath, item.authorizeSecret, item.authorizeToken, item.captureId, item.captureName);
}

void CapturePlaylistWindow::addItem (const StdString &hostname, const StdString &authorizePath, const StdString &authorizeSecret, const StdString &authorizeToken, int captureId, const StdString &captureName) {
	CapturePlaylistWindow::Item *item;

	item = new CapturePlaylistWindow::Item ();
	item->hostname.assign (hostname);
	item->authorizePath.assign (authorizePath);
	item->authorizeSecret.assign (authorizeSecret);
	item->authorizeToken.assign (authorizeToken);
	item->captureId = captureId;
	item->captureName.assign (captureName);

	itemListView->addItem (item->captureName, item, CapturePlaylistWindow::freeItem);
	itemListView->scrollToBottom ();
}

void CapturePlaylistWindow::freeItem (void *itemPtr) {
	delete ((CapturePlaylistWindow::Item *) itemPtr);
}

Widget::Rectangle CapturePlaylistWindow::getAddItemButtonScreenRect () {
	return (addItemButton->getScreenRect ());
}

Widget::Rectangle CapturePlaylistWindow::getRemoveButtonScreenRect () {
	return (removeButton->getScreenRect ());
}

StdString CapturePlaylistWindow::itemDisplayDurationSliderValueName (float sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Slowest).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[0] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Slow).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[1] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Medium).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[2] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Fast).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[3] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::Fastest).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[4] * 1000).c_str ()));
	}

	return (StdString (""));
}

int CapturePlaylistWindow::getItemDisplayDuration () {
	float val;

	val = itemDisplayDurationSlider->value;
	if (FLOAT_EQUALS (val, 0.0f)) {
		return (CapturePlaylistWindow::ItemDisplayDurations[0]);
	}
	if (FLOAT_EQUALS (val, 0.25f)) {
		return (CapturePlaylistWindow::ItemDisplayDurations[1]);
	}
	if (FLOAT_EQUALS (val, 0.5f)) {
		return (CapturePlaylistWindow::ItemDisplayDurations[2]);
	}
	if (FLOAT_EQUALS (val, 0.75f)) {
		return (CapturePlaylistWindow::ItemDisplayDurations[3]);
	}
	if (FLOAT_EQUALS (val, 1.0f)) {
		return (CapturePlaylistWindow::ItemDisplayDurations[4]);
	}

	return (CapturePlaylistWindow::ItemDisplayDurations[2]);
}

Json *CapturePlaylistWindow::createState () {
	Json *state, *itemobj;
	CapturePlaylistWindow::Item *item;
	JsonList items;
	int i, count;

	state = new Json ();
	state->set (CapturePlaylistWindow::PlaylistNameKey, playlistName);
	state->set (CapturePlaylistWindow::IsShuffleKey, shuffleToggle->isChecked);
	state->set (CapturePlaylistWindow::ItemDisplayDurationKey, getItemDisplayDuration ());
	state->set (CapturePlaylistWindow::IsSelectedKey, isSelected);
	state->set (CapturePlaylistWindow::IsExpandedKey, isExpanded);

	count = itemListView->getItemCount ();
	for (i = 0; i < count; ++i) {
		item = (CapturePlaylistWindow::Item *) itemListView->getItemData (i);
		if (item) {
			itemobj = new Json ();
			itemobj->set (CapturePlaylistWindow::HostnameKey, item->hostname);
			itemobj->set (CapturePlaylistWindow::AuthorizePathKey, item->authorizePath);
			itemobj->set (CapturePlaylistWindow::AuthorizeSecretKey, item->authorizeSecret);
			itemobj->set (CapturePlaylistWindow::AuthorizeTokenKey, item->authorizeToken);
			itemobj->set (CapturePlaylistWindow::CaptureIdKey, item->captureId);
			itemobj->set (CapturePlaylistWindow::CaptureNameKey, item->captureName);
			items.push_back (itemobj);
		}
	}
	state->set (CapturePlaylistWindow::ItemListKey, &items);

	return (state);
}

void CapturePlaylistWindow::readState (Json *state) {
	CapturePlaylistWindow::Item item;
	Json itemobj;
	int duration, i, count;
	bool b;

	duration = state->getNumber (CapturePlaylistWindow::ItemDisplayDurationKey, CapturePlaylistWindow::ItemDisplayDurations[2]);
	if (duration == CapturePlaylistWindow::ItemDisplayDurations[0]) {
		itemDisplayDurationSlider->setValue (0.0f);
	}
	else if (duration == CapturePlaylistWindow::ItemDisplayDurations[1]) {
		itemDisplayDurationSlider->setValue (0.25f);
	}
	else if (duration == CapturePlaylistWindow::ItemDisplayDurations[2]) {
		itemDisplayDurationSlider->setValue (0.5f);
	}
	else if (duration == CapturePlaylistWindow::ItemDisplayDurations[3]) {
		itemDisplayDurationSlider->setValue (0.75f);
	}
	else if (duration == CapturePlaylistWindow::ItemDisplayDurations[4]) {
		itemDisplayDurationSlider->setValue (1.0f);
	}

	shuffleToggle->setChecked (state->getBoolean (CapturePlaylistWindow::IsShuffleKey, false), true);

	itemListView->clearItems ();
	count = state->getArrayLength (CapturePlaylistWindow::ItemListKey);
	for (i = 0; i < count; ++i) {
		if (state->getArrayObject (CapturePlaylistWindow::ItemListKey, i, &itemobj)) {
			item.hostname.assign (itemobj.getString (CapturePlaylistWindow::HostnameKey, ""));
			item.authorizePath.assign (itemobj.getString (CapturePlaylistWindow::AuthorizePathKey, ""));
			item.authorizeSecret.assign (itemobj.getString (CapturePlaylistWindow::AuthorizeSecretKey, ""));
			item.authorizeToken.assign (itemobj.getString (CapturePlaylistWindow::AuthorizeTokenKey, ""));
			item.captureId = itemobj.getNumber (CapturePlaylistWindow::CaptureIdKey, (int) 0);
			item.captureName.assign (itemobj.getString (CapturePlaylistWindow::CaptureNameKey, ""));
			addItem (item);
		}
	}

	b = state->getBoolean (CapturePlaylistWindow::IsSelectedKey, false);
	isSelected = ! b;
	setSelected (b, true);

	b = state->getBoolean (CapturePlaylistWindow::IsExpandedKey, false);
	isExpanded = ! b;
	setExpanded (b, true);

	setPlaylistName (state->getString (CapturePlaylistWindow::PlaylistNameKey, ""));
	refreshLayout ();
}

Json *CapturePlaylistWindow::createCommand () {
	CapturePlaylistWindow::Item *item;
	Json *params, *camera;
	JsonList cameras;
	int i, count;

	params = new Json ();
	params->set ("displayName", playlistName);

	count = itemListView->getItemCount ();
	for (i = 0; i < count; ++i) {
		item = (CapturePlaylistWindow::Item *) itemListView->getItemData (i);

		camera = new Json ();
		camera->set ("hostname", item->hostname);
		if (! item->authorizePath.empty ()) {
			camera->set ("authorizePath", item->authorizePath);
		}
		if (! item->authorizeSecret.empty ()) {
			camera->set ("authorizeSecret", item->authorizeSecret);
		}
		if (! item->authorizeToken.empty ()) {
			camera->set ("authorizeToken", item->authorizeToken);
		}
		cameras.push_back (camera);
	}
	params->set ("cameras", &cameras);

	params->set ("isShuffle", shuffleToggle->isChecked);
	params->set ("minItemDisplayDuration", getItemDisplayDuration ());
	params->set ("maxItemDisplayDuration", getItemDisplayDuration ());

	return (App::instance->createCommand (SystemInterface::Command_CreateCameraDisplayIntent, SystemInterface::Constant_Monitor, params));
}
