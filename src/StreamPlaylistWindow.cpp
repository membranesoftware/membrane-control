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
#include <list>
#include "Result.h"
#include "ClassId.h"
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
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "IconLabelWindow.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "MediaUi.h"
#include "StreamPlaylistWindow.h"

const float StreamPlaylistWindow::windowWidthMultiplier = 0.45f;
const char *StreamPlaylistWindow::PlaylistNameKey = "a";
const char *StreamPlaylistWindow::IsSelectedKey = "b";
const char *StreamPlaylistWindow::IsExpandedKey = "c";
const char *StreamPlaylistWindow::PlayDurationKey = "f";
const char *StreamPlaylistWindow::ItemListKey = "h";
const char *StreamPlaylistWindow::IsShuffleKey = "i";
const char *StreamPlaylistWindow::StreamUrlKey = "j";
const char *StreamPlaylistWindow::StreamIdKey = "k";
const char *StreamPlaylistWindow::MediaNameKey = "l";
const char *StreamPlaylistWindow::StartPositionKey = "m";
const char *StreamPlaylistWindow::ThumbnailUrlKey = "n";
const char *StreamPlaylistWindow::ThumbnailIndexKey = "o";

StreamPlaylistWindow::StreamPlaylistWindow (SpriteGroup *mediaUiSpriteGroup)
: Panel ()
, isSelected (false)
, isExpanded (false)
, addItemButtonX1 (0.0f)
, addItemButtonX2 (0.0f)
, addItemButtonY (0.0f)
, sprites (mediaUiSpriteGroup)
, windowWidth (0.0f)
, iconImage (NULL)
, nameLabel (NULL)
, itemCountLabel (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, shuffleToggle (NULL)
, startPositionSlider (NULL)
, playDurationSlider (NULL)
, itemListView (NULL)
, removeButton (NULL)
, addItemButton (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;
	Slider *slider;
	int i;

	classId = ClassId::StreamPlaylistWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
	setCornerRadius (uiconfig->cornerRadius);
	setFillBg (true, uiconfig->mediumBackgroundColor);
	windowWidth = App::instance->windowWidth * StreamPlaylistWindow::windowWidthMultiplier;

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::PlaylistIconSprite)));
	nameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor)));
	nameLabel->setPadding (0.0f, 0.0f);
	nameLabel->setMouseClickCallback (StreamPlaylistWindow::nameLabelClicked, this);
	nameLabel->setMouseHoverTooltip (uitext->getText (UiTextString::clickRenameTooltip));

	itemCountLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString ("0"), UiConfiguration::TitleFont, uiconfig->lightPrimaryTextColor));
	itemCountLabel->setPadding (0.0f, 0.0f);
	itemCountLabel->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::streamPlaylistItem, UiTextString::streamPlaylistItems));
	itemCountLabel->setTextChangeHighlight (true, uiconfig->primaryTextColor);

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (StreamPlaylistWindow::selectToggleStateChanged, this);
	selectToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::unselectedToggleTooltip), uitext->getText (UiTextString::selectedToggleTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (StreamPlaylistWindow::expandToggleStateChanged, this);
	expandToggle->setStateMouseHoverTooltips (uitext->getText (UiTextString::expand).capitalized (), uitext->getText (UiTextString::minimize).capitalized ());

	itemListView = (ListView *) addWidget (new ListView ((windowWidth - (widthPadding * 2.0f)), 6, UiConfiguration::CaptionFont, StdString (""), uitext->getText (UiTextString::emptyStreamPlaylistText)));
	itemListView->setListChangeCallback (StreamPlaylistWindow::itemListChanged, this);
	itemListView->isVisible = false;

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle ()));
	shuffleToggle->setIcon (sprites->getSprite (MediaUi::ShuffleIconSprite));
	shuffleToggle->setRightAligned (true);
	shuffleToggle->setImageColor (uiconfig->flatButtonTextColor);
	shuffleToggle->setMouseHoverTooltip (uitext->getText (UiTextString::shuffleTooltip));
	shuffleToggle->isVisible = false;

	slider = new Slider (0.0f, (float) (StreamPlaylistWindow::StartPositionCount - 1));
	for (i = 0; i < StreamPlaylistWindow::StartPositionCount; ++i) {
		slider->addSnapValue ((float) i);
	}
	startPositionSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	startPositionSlider->setIcon (sprites->getSprite (MediaUi::StartPositionIconSprite));
	startPositionSlider->setMouseHoverTooltip (uitext->getText (UiTextString::startPosition).capitalized ());
	startPositionSlider->setValueNameFunction (StreamPlaylistWindow::startPositionSliderValueName);
	startPositionSlider->setValue (StreamPlaylistWindow::ZeroStartPosition);
	startPositionSlider->isVisible = false;

	slider = new Slider (0.0f, (float) (StreamPlaylistWindow::PlayDurationCount - 1));
	for (i = 0; i < StreamPlaylistWindow::PlayDurationCount; ++i) {
		slider->addSnapValue ((float) i);
	}
	playDurationSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	playDurationSlider->setIcon (sprites->getSprite (MediaUi::DurationIconSprite));
	playDurationSlider->setMouseHoverTooltip (uitext->getText (UiTextString::playDuration).capitalized ());
	playDurationSlider->setValueNameFunction (StreamPlaylistWindow::playDurationSliderValueName);
	playDurationSlider->setValue (StreamPlaylistWindow::MediumPlayDuration);
	playDurationSlider->isVisible = false;

	removeButton = (Button *) addWidget (new Button (uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->setMouseClickCallback (StreamPlaylistWindow::removeButtonClicked, this);
	removeButton->setImageColor (uiconfig->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (uitext->getText (UiTextString::deletePlaylist).capitalized ());
	removeButton->isVisible = false;

	addItemButton = (Button *) addWidget (new Button (sprites->getSprite (MediaUi::AddPlaylistItemButtonSprite)));
	addItemButton->setMouseClickCallback (StreamPlaylistWindow::addItemButtonClicked, this);
	addItemButton->setMouseEnterCallback (StreamPlaylistWindow::addItemButtonMouseEntered, this);
	addItemButton->setMouseExitCallback (StreamPlaylistWindow::addItemButtonMouseExited, this);
	addItemButton->setImageColor (uiconfig->flatButtonTextColor);
	addItemButton->isVisible = false;

	refreshLayout ();
}

StreamPlaylistWindow::~StreamPlaylistWindow () {

}

StdString StreamPlaylistWindow::toStringDetail () {
	return (StdString (" StreamPlaylistWindow"));
}

bool StreamPlaylistWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::StreamPlaylistWindow));
}

StreamPlaylistWindow *StreamPlaylistWindow::castWidget (Widget *widget) {
	return (StreamPlaylistWindow::isWidgetType (widget) ? (StreamPlaylistWindow *) widget : NULL);
}

void StreamPlaylistWindow::setPlaylistName (const StdString &name) {
	if (playlistName.equals (name)) {
		return;
	}
	playlistName.assign (name);
	nameLabel->setText (playlistName);
	refreshLayout ();
	resetNameLabel ();
}

void StreamPlaylistWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
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

void StreamPlaylistWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	UiConfiguration *uiconfig;

	if (expanded == isExpanded) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
		expandToggle->setChecked (true, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
		itemCountLabel->setTextFont (UiConfiguration::HeadlineFont);
		itemListView->isVisible = true;
		shuffleToggle->isVisible = true;
		startPositionSlider->isVisible = true;
		playDurationSlider->isVisible = true;
		removeButton->isVisible = true;
		addItemButton->isVisible = true;
	}
	else {
		setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		itemCountLabel->setTextFont (UiConfiguration::BodyFont);
		itemListView->isVisible = false;
		shuffleToggle->isVisible = false;
		startPositionSlider->isVisible = false;
		playDurationSlider->isVisible = false;
		removeButton->isVisible = false;
		addItemButton->isVisible = false;
	}
	refreshLayout ();
	resetNameLabel ();
}

void StreamPlaylistWindow::doRefresh () {
	windowWidth = App::instance->windowWidth * StreamPlaylistWindow::windowWidthMultiplier;
	itemListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
	Panel::doRefresh ();
}

void StreamPlaylistWindow::refreshLayout () {
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
	expandToggle->flowRight (&x, y, &x2, &y2);
	selectToggle->flowRight (&x, y, &x2, &y2);
	if (isExpanded) {
		nameLabel->centerVertical (y0, y2);
	}

	x = x0;
	x2 = 0.0f;
	y = y2;
	y0 = y;
	if (isExpanded) {
		x = x0 + uiconfig->marginSize;
		x2 = 0.0f;
		itemCountLabel->flowRight (&x, y, &x2, &y2);
		shuffleToggle->flowRight (&x, y, &x2, &y2);

		itemCountLabel->centerVertical (y0, y2);
		shuffleToggle->centerVertical (y0, y2);

		x = x0;
		x2 = 0.0f;
		y = y2;
		y0 = y;
		startPositionSlider->flowRight (&x, y, &x2, &y2);
		playDurationSlider->flowRight (&x, y, &x2, &y2);

		x = x0;
		x2 = 0.0f;
		y = y2;
		y0 = y;
		itemListView->flowRight (&x, y, &x2, &y2);

		y = y2 + uiconfig->marginSize;
		x = x0;
		x2 = 0.0f;
		removeButton->flowRight (&x, y, &x2, &y2);
		addItemButton->flowRight (&x, y, &x2, &y2);
		setFixedSize (true, windowWidth, y2 + heightPadding);

		x = width - widthPadding;
		addItemButton->flowLeft (&x);
		removeButton->flowLeft (&x);
	}
	else {
		setFixedSize (false);
		resetSize ();
	}

	x = width - widthPadding;
	selectToggle->flowLeft (&x);
	expandToggle->flowLeft (&x);
	addItemButtonX1 = addItemButton->position.x;
	addItemButtonX2 = addItemButton->position.x + addItemButton->width;
	addItemButtonY = addItemButton->position.y;
}

void StreamPlaylistWindow::resetNameLabel () {
	UiConfiguration *uiconfig;
	float w;

	uiconfig = &(App::instance->uiConfig);
	if (isExpanded) {
		w = expandToggle->position.x;
		w -= nameLabel->position.x;
		w -= uiconfig->marginSize;
		nameLabel->setText (Label::getTruncatedText (playlistName, UiConfiguration::HeadlineFont, w, StdString ("...")));
	}
	else {
		w = (windowWidth / 2.0f);
		w -= (iconImage->width + expandToggle->width + selectToggle->width);
		nameLabel->setText (Label::getTruncatedText (playlistName, UiConfiguration::BodyFont, w, StdString ("...")));
	}
	refreshLayout ();
}

void StreamPlaylistWindow::nameLabelClicked (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;

	window = (StreamPlaylistWindow *) windowPtr;
	if (window->renameClickCallback.callback) {
		window->renameClickCallback.callback (window->renameClickCallback.callbackData, window);
	}
}

void StreamPlaylistWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	Toggle *toggle;
	UiConfiguration *uiconfig;

	window = (StreamPlaylistWindow *) windowPtr;
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

StdString StreamPlaylistWindow::startPositionSliderValueName (float sliderValue) {
	switch ((int) sliderValue) {
		case StreamPlaylistWindow::ZeroStartPosition: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistZeroStartPositionDescription));
		}
		case StreamPlaylistWindow::NearBeginningStartPosition: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistNearBeginningStartPositionDescription));
		}
		case StreamPlaylistWindow::MiddleStartPosition: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistMiddleStartPositionDescription));
		}
		case StreamPlaylistWindow::NearEndStartPosition: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistNearEndPositionDescription));
		}
		case StreamPlaylistWindow::FullRangeStartPosition: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistFullRangePositionDescription));
		}
	}

	return (StdString (""));
}

StdString StreamPlaylistWindow::playDurationSliderValueName (float sliderValue) {
	switch ((int) sliderValue) {
		case StreamPlaylistWindow::VeryShortPlayDuration: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistVeryShortPlayDurationDescription));
		}
		case StreamPlaylistWindow::ShortPlayDuration: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistShortPlayDurationDescription));
		}
		case StreamPlaylistWindow::MediumPlayDuration: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistMediumPlayDurationDescription));
		}
		case StreamPlaylistWindow::LongPlayDuration: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistLongPlayDurationDescription));
		}
		case StreamPlaylistWindow::VeryLongPlayDuration: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistVeryLongPlayDurationDescription));
		}
		case StreamPlaylistWindow::FullPlayDuration: {
			return (App::instance->uiText.getText (UiTextString::streamPlaylistFullPlayDurationDescription));
		}
	}

	return (StdString (""));
}

void StreamPlaylistWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	Toggle *toggle;

	window = (StreamPlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback.callback) {
		window->expandStateChangeCallback.callback (window->expandStateChangeCallback.callbackData, window);
	}
}

void StreamPlaylistWindow::itemListChanged (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	UiText *uitext;
	int count;

	window = (StreamPlaylistWindow *) windowPtr;
	uitext = &(App::instance->uiText);
	count = window->itemListView->getItemCount ();
	window->itemCountLabel->setText (StdString::createSprintf ("%i", count));
	window->itemCountLabel->tooltipText.assign (uitext->getCountText (count, UiTextString::streamPlaylistItem, UiTextString::streamPlaylistItems));
	window->refreshLayout ();
	if (window->listChangeCallback.callback) {
		window->listChangeCallback.callback (window->listChangeCallback.callbackData, window);
	}
}

int StreamPlaylistWindow::getItemCount () {
	return (itemListView->getItemCount ());
}

void StreamPlaylistWindow::addItem (const StdString &streamUrl, const StdString &streamId, const StdString &mediaName, float startPosition, const StdString &thumbnailUrl, int thumbnailIndex) {
	StreamPlaylistWindow::Item *item;

	item = new StreamPlaylistWindow::Item ();
	item->streamUrl.assign (streamUrl);
	item->streamId.assign (streamId);
	item->mediaName.assign (mediaName);
	item->startPosition = startPosition;
	item->thumbnailUrl.assign (thumbnailUrl);
	item->thumbnailIndex = thumbnailIndex;
	itemListView->addItem (StdString::createSprintf ("%s %s", OsUtil::getDurationString (startPosition * 1000.0f, OsUtil::HoursUnit).c_str (), mediaName.c_str ()), item, StreamPlaylistWindow::freeItem);
}

void StreamPlaylistWindow::freeItem (void *itemPtr) {
	delete ((StreamPlaylistWindow::Item *) itemPtr);
}

void StreamPlaylistWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;

	window = (StreamPlaylistWindow *) windowPtr;
	if (window->removeClickCallback.callback) {
		window->removeClickCallback.callback (window->removeClickCallback.callbackData, window);
	}
}

void StreamPlaylistWindow::addItemButtonClicked (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;

	window = (StreamPlaylistWindow *) windowPtr;
	if (window->addItemClickCallback.callback) {
		window->addItemClickCallback.callback (window->addItemClickCallback.callbackData, window);
	}
}

void StreamPlaylistWindow::addItemButtonMouseEntered (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	Button *button;

	window = (StreamPlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseEntered (button, button);
	if (window->addItemMouseEnterCallback.callback) {
		window->addItemMouseEnterCallback.callback (window->addItemMouseEnterCallback.callbackData, window);
	}
}

void StreamPlaylistWindow::addItemButtonMouseExited (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	Button *button;

	window = (StreamPlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);
	if (window->addItemMouseExitCallback.callback) {
		window->addItemMouseExitCallback.callback (window->addItemMouseExitCallback.callbackData, window);
	}
}

Json *StreamPlaylistWindow::getState () {
	Json *obj, *itemobj;
	StreamPlaylistWindow::Item *item;
	std::list<Json *> items;
	int i, count;

	obj = new Json ();
	obj->set (StreamPlaylistWindow::PlaylistNameKey, playlistName);
	obj->set (StreamPlaylistWindow::IsSelectedKey, isSelected);
	obj->set (StreamPlaylistWindow::IsExpandedKey, isExpanded);
	obj->set (StreamPlaylistWindow::IsShuffleKey, shuffleToggle->isChecked);
	obj->set (StreamPlaylistWindow::StartPositionKey, (int) startPositionSlider->value);
	obj->set (StreamPlaylistWindow::PlayDurationKey, (int) playDurationSlider->value);

	count = itemListView->getItemCount ();
	for (i = 0; i < count; ++i) {
		item = (StreamPlaylistWindow::Item *) itemListView->getItemData (i);
		if (item) {
			itemobj = new Json ();
			itemobj->set (StreamPlaylistWindow::StreamUrlKey, item->streamUrl.c_str ());
			itemobj->set (StreamPlaylistWindow::StreamIdKey, item->streamId.c_str ());
			itemobj->set (StreamPlaylistWindow::MediaNameKey, item->mediaName.c_str ());
			itemobj->set (StreamPlaylistWindow::StartPositionKey, item->startPosition);
			itemobj->set (StreamPlaylistWindow::ThumbnailUrlKey, item->thumbnailUrl);
			itemobj->set (StreamPlaylistWindow::ThumbnailIndexKey, item->thumbnailIndex);

			items.push_back (itemobj);
		}
	}
	obj->set (StreamPlaylistWindow::ItemListKey, &items);

	return (obj);
}

void StreamPlaylistWindow::setState (Json *stateObject) {
	StdString val;
	StreamPlaylistWindow::Item item;
	Json itemobj;
	int i, count;
	bool b;

	val = stateObject->getString (StreamPlaylistWindow::PlaylistNameKey, "");
	if (! val.empty ()) {
		setPlaylistName (val);
	}

	shuffleToggle->setChecked (stateObject->getBoolean (StreamPlaylistWindow::IsShuffleKey, false));
	startPositionSlider->setValue ((float) stateObject->getNumber (StreamPlaylistWindow::StartPositionKey, StreamPlaylistWindow::ZeroStartPosition));
	playDurationSlider->setValue ((float) stateObject->getNumber (StreamPlaylistWindow::PlayDurationKey, StreamPlaylistWindow::MediumPlayDuration));

	count = stateObject->getArrayLength (StreamPlaylistWindow::ItemListKey);
	for (i = 0; i < count; ++i) {
		if (stateObject->getArrayObject (StreamPlaylistWindow::ItemListKey, i, &itemobj)) {
			item.streamUrl.assign (itemobj.getString (StreamPlaylistWindow::StreamUrlKey, ""));
			item.streamId.assign (itemobj.getString (StreamPlaylistWindow::StreamIdKey, ""));
			item.mediaName.assign (itemobj.getString (StreamPlaylistWindow::MediaNameKey, ""));
			item.startPosition = itemobj.getNumber (StreamPlaylistWindow::StartPositionKey, 0.0f);
			item.thumbnailUrl.assign (itemobj.getString (StreamPlaylistWindow::ThumbnailUrlKey, ""));
			item.thumbnailIndex = itemobj.getNumber (StreamPlaylistWindow::ThumbnailIndexKey, (int) 0);

			addItem (item.streamUrl, item.streamId, item.mediaName, item.startPosition, item.thumbnailUrl, item.thumbnailIndex);
		}
	}

	b = stateObject->getBoolean (StreamPlaylistWindow::IsSelectedKey, false);
	isSelected = ! b;
	setSelected (b, true);

	b = stateObject->getBoolean (StreamPlaylistWindow::IsExpandedKey, false);
	isExpanded = ! b;
	setExpanded (b, true);
}

Json *StreamPlaylistWindow::getCreateCommand () {
	std::list<Json *> items;
	StreamPlaylistWindow::Item *item;
	Json *obj, *itemparams;
	int i, count;

	obj = new Json ();
	obj->set ("displayName", playlistName);

	count = itemListView->getItemCount ();
	for (i = 0; i < count; ++i) {
		item = (StreamPlaylistWindow::Item *) itemListView->getItemData (i);
		if (item) {
			itemparams = new Json ();
			itemparams->set ("mediaName", item->mediaName);
			itemparams->set ("streamUrl", item->streamUrl);
			itemparams->set ("streamId", item->streamId);
			itemparams->set ("startPosition", item->startPosition);

			if ((! item->thumbnailUrl.empty ()) && (item->thumbnailIndex >= 0)) {
				itemparams->set ("thumbnailUrl", item->thumbnailUrl);
				itemparams->set ("thumbnailIndex", item->thumbnailIndex);
			}

			items.push_back (itemparams);
		}
	}
	obj->set ("items", &items);
	obj->set ("isShuffle", shuffleToggle->isChecked);
	StreamPlaylistWindow::setStartPositionDelta ((int) startPositionSlider->value, obj);
	StreamPlaylistWindow::setItemDisplayDuration ((int) playDurationSlider->value, obj);

	return (App::instance->createCommand (SystemInterface::Command_CreateMediaDisplayIntent, SystemInterface::Constant_Monitor, obj));
}

void StreamPlaylistWindow::setStartPositionDelta (int startPosition, Json *createMediaDisplayIntentParams) {
	int min, max;

	min = 0;
	max = 0;
	switch (startPosition) {
		case StreamPlaylistWindow::NearBeginningStartPosition: {
			min = 10;
			max = 20;
			break;
		}
		case StreamPlaylistWindow::MiddleStartPosition: {
			min = 35;
			max = 60;
			break;
		}
		case StreamPlaylistWindow::NearEndStartPosition: {
			min = 75;
			max = 90;
			break;
		}
		case StreamPlaylistWindow::FullRangeStartPosition: {
			min = 0;
			max = 99;
			break;
		}
	}
	createMediaDisplayIntentParams->set ("minStartPositionDelta", min);
	createMediaDisplayIntentParams->set ("maxStartPositionDelta", max);
}

void StreamPlaylistWindow::setItemDisplayDuration (int playDuration, Json *createMediaDisplayIntentParams) {
	int min, max;

	min = 15;
	max = 10800;
	switch (playDuration) {
		case StreamPlaylistWindow::VeryShortPlayDuration: {
			min = 15;
			max = 45;
			break;
		}
		case StreamPlaylistWindow::ShortPlayDuration: {
			min = 60;
			max = 180;
			break;
		}
		case StreamPlaylistWindow::MediumPlayDuration: {
			min = 300;
			max = 900;
			break;
		}
		case StreamPlaylistWindow::LongPlayDuration: {
			min = 1800;
			max = 3600;
			break;
		}
		case StreamPlaylistWindow::VeryLongPlayDuration: {
			min = 7200;
			max = 14400;
			break;
		}
		case StreamPlaylistWindow::FullPlayDuration: {
			min = 0;
			max = 0;
			break;
		}
	}
	createMediaDisplayIntentParams->set ("minItemDisplayDuration", min);
	createMediaDisplayIntentParams->set ("maxItemDisplayDuration", max);
}
