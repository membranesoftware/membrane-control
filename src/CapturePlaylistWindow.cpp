/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "Log.h"
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
	classId = ClassId::CapturePlaylistWindow;

	setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	windowWidth = App::instance->windowWidth * CapturePlaylistWindow::WindowWidthMultiplier;

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::PlaylistIconSprite)));
	nameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor)));
	nameLabel->mouseClickCallback = Widget::EventCallbackContext (CapturePlaylistWindow::nameLabelClicked, this);
	nameLabel->setPadding (0.0f, 0.0f);
	nameLabel->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ClickRenameTooltip));

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle ()));
	shuffleToggle->setIcon (sprites->getSprite (CameraUi::ShuffleIconSprite));
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
	itemDisplayDurationSlider->setValueNameFunction (CapturePlaylistWindow::itemDisplayDurationSliderValueName);
	itemDisplayDurationSlider->setIcon (sprites->getSprite (CameraUi::SpeedIconSprite));
	itemDisplayDurationSlider->setMouseHoverTooltip (UiText::instance->getText (UiTextString::PlaylistSpeedTooltip));
	itemDisplayDurationSlider->zLevel = 1;
	itemDisplayDurationSlider->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->stateChangeCallback = Widget::EventCallbackContext (CapturePlaylistWindow::selectToggleStateChanged, this);
	selectToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	selectToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::UnselectedToggleTooltip), UiText::instance->getText (UiTextString::SelectedToggleTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (CapturePlaylistWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());

	itemCountLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (CameraUi::TimelapseIconSprite), StdString ("0"), UiConfiguration::TitleFont, UiConfiguration::instance->lightPrimaryTextColor));
	itemCountLabel->setPadding (0.0f, 0.0f);
	itemCountLabel->setMouseHoverTooltip (UiText::instance->getCountText (0, UiTextString::CapturePlaylistItem, UiTextString::CapturePlaylistItems));
	itemCountLabel->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);

	itemListView = (ListView *) addWidget (new ListView (windowWidth - (widthPadding * 2.0f), 4, 7, UiConfiguration::CaptionFont, UiText::instance->getText (UiTextString::EmptyCaptureListPrompt)));
	itemListView->listChangeCallback = Widget::EventCallbackContext (CapturePlaylistWindow::itemListChanged, this);
	itemListView->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	itemListView->isVisible = false;

	removeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (CapturePlaylistWindow::removeButtonClicked, this);
	removeButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::DeletePlaylist).capitalized ());
	removeButton->isVisible = false;

	addItemButton = (Button *) addWidget (new Button (sprites->getSprite (CameraUi::AddPlaylistItemButtonSprite)));
	addItemButton->mouseClickCallback = Widget::EventCallbackContext (CapturePlaylistWindow::addItemButtonClicked, this);
	addItemButton->mouseEnterCallback = Widget::EventCallbackContext (CapturePlaylistWindow::addItemButtonMouseEntered, this);
	addItemButton->mouseExitCallback = Widget::EventCallbackContext (CapturePlaylistWindow::addItemButtonMouseExited, this);
	addItemButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
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

void CapturePlaylistWindow::setPlaylistName (const StdString &name) {
	playlistName.assign (name);
	nameLabel->setText (playlistName);
	refreshLayout ();
	resetNameLabel ();
}

void CapturePlaylistWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
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
		setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
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
		itemCountLabel->flowRight (&x, y, &x2, &y2);
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
		itemListView->flowDown (x, &y, &x2, &y2);

		y = y2 + UiConfiguration::instance->marginSize;
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

		dividerPanel->setFixedSize (true, windowWidth, UiConfiguration::instance->headlineDividerLineWidth);

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

void CapturePlaylistWindow::doRefresh () {
	windowWidth = App::instance->windowWidth * CapturePlaylistWindow::WindowWidthMultiplier;
	itemListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
	Panel::doRefresh ();
}

void CapturePlaylistWindow::nameLabelClicked (void *windowPtr, Widget *widgetPtr) {
	((CapturePlaylistWindow *) windowPtr)->eventCallback (((CapturePlaylistWindow *) windowPtr)->nameClickCallback);
}

void CapturePlaylistWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	Toggle *toggle;

	window = (CapturePlaylistWindow *) windowPtr;
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

void CapturePlaylistWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	Toggle *toggle;

	window = (CapturePlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	window->eventCallback (window->expandStateChangeCallback);
}

void CapturePlaylistWindow::itemListChanged (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	int count;

	window = (CapturePlaylistWindow *) windowPtr;
	count = window->itemListView->getItemCount ();
	window->itemCountLabel->setText (StdString::createSprintf ("%i", count));
	window->itemCountLabel->tooltipText.assign (UiText::instance->getCountText (count, UiTextString::CapturePlaylistItem, UiTextString::CapturePlaylistItems));
	window->refreshLayout ();
	window->eventCallback (window->itemListChangeCallback);
}

void CapturePlaylistWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((CapturePlaylistWindow *) windowPtr)->eventCallback (((CapturePlaylistWindow *) windowPtr)->removeClickCallback);
}

void CapturePlaylistWindow::addItemButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((CapturePlaylistWindow *) windowPtr)->eventCallback (((CapturePlaylistWindow *) windowPtr)->addItemClickCallback);
}

void CapturePlaylistWindow::addItemButtonMouseEntered (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	Button *button;

	window = (CapturePlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseEntered (button, button);
	window->eventCallback (window->addItemMouseEnterCallback);
}

void CapturePlaylistWindow::addItemButtonMouseExited (void *windowPtr, Widget *widgetPtr) {
	CapturePlaylistWindow *window;
	Button *button;

	window = (CapturePlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);
	window->eventCallback (window->addItemMouseExitCallback);
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
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Slowest).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[0] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Slow).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[1] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Medium).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[2] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Fast).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[3] * 1000).c_str ()));
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::Fastest).capitalized ().c_str (), OsUtil::getDurationDisplayString (CapturePlaylistWindow::ItemDisplayDurations[4] * 1000).c_str ()));
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

	return (App::instance->createCommand (SystemInterface::Command_CreateCameraDisplayIntent, params));
}
