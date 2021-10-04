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
#include <list>
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
#include "ListView.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "IconLabelWindow.h"
#include "SystemInterface.h"
#include "MediaUi.h"
#include "StreamPlaylistWindow.h"

const float StreamPlaylistWindow::WindowWidthMultiplier = 0.45f;
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
, windowWidth (0.0f)
, sprites (mediaUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, itemCountLabel (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, dividerPanel (NULL)
, shuffleToggle (NULL)
, startPositionSlider (NULL)
, playDurationSlider (NULL)
, itemListView (NULL)
, removeButton (NULL)
, addItemButton (NULL)
{
	Slider *slider;
	int i;

	classId = ClassId::StreamPlaylistWindow;
	setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	windowWidth = App::instance->windowWidth * StreamPlaylistWindow::WindowWidthMultiplier;

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::PlaylistIconSprite)));
	nameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor)));
	nameLabel->mouseClickCallback = Widget::EventCallbackContext (StreamPlaylistWindow::nameLabelClicked, this);
	nameLabel->setPadding (0.0f, 0.0f);
	nameLabel->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ClickRenameTooltip));

	itemCountLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString ("0"), UiConfiguration::TitleFont, UiConfiguration::instance->lightPrimaryTextColor));
	itemCountLabel->setPadding (0.0f, 0.0f);
	itemCountLabel->setMouseHoverTooltip (UiText::instance->getCountText (0, UiTextString::StreamPlaylistItem, UiTextString::StreamPlaylistItems));
	itemCountLabel->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);

	selectToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->stateChangeCallback = Widget::EventCallbackContext (StreamPlaylistWindow::selectToggleStateChanged, this);
	selectToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	selectToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::UnselectedToggleTooltip), UiText::instance->getText (UiTextString::SelectedToggleTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (StreamPlaylistWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	itemListView = (ListView *) addWidget (new ListView ((windowWidth - (widthPadding * 2.0f)), 6, 6, UiConfiguration::CaptionFont, UiText::instance->getText (UiTextString::EmptyStreamPlaylistText)));
	itemListView->listChangeCallback = Widget::EventCallbackContext (StreamPlaylistWindow::itemListChanged, this);
	itemListView->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	itemListView->isVisible = false;

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle ()));
	shuffleToggle->setIcon (sprites->getSprite (MediaUi::ShuffleIconSprite));
	shuffleToggle->setRightAligned (true);
	shuffleToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	shuffleToggle->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ShuffleTooltip));
	shuffleToggle->isVisible = false;

	slider = new Slider (0.0f, (float) (StreamPlaylistWindow::StartPositionCount - 1));
	for (i = 0; i < StreamPlaylistWindow::StartPositionCount; ++i) {
		slider->addSnapValue ((float) i);
	}
	startPositionSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	startPositionSlider->setIcon (sprites->getSprite (MediaUi::StartPositionIconSprite));
	startPositionSlider->setTrackWidthScale (0.75f);
	startPositionSlider->setMouseHoverTooltip (UiText::instance->getText (UiTextString::StartPosition).capitalized ());
	startPositionSlider->setValueNameFunction (StreamPlaylistWindow::startPositionSliderValueName);
	startPositionSlider->setValue (StreamPlaylistWindow::ZeroStartPosition);
	startPositionSlider->isVisible = false;

	slider = new Slider (0.0f, (float) (StreamPlaylistWindow::PlayDurationCount - 1));
	for (i = 0; i < StreamPlaylistWindow::PlayDurationCount; ++i) {
		slider->addSnapValue ((float) i);
	}
	playDurationSlider = (SliderWindow *) addWidget (new SliderWindow (slider));
	playDurationSlider->setIcon (sprites->getSprite (MediaUi::DurationIconSprite));
	playDurationSlider->setTrackWidthScale (0.75f);
	playDurationSlider->setMouseHoverTooltip (UiText::instance->getText (UiTextString::PlayDuration).capitalized ());
	playDurationSlider->setValueNameFunction (StreamPlaylistWindow::playDurationSliderValueName);
	playDurationSlider->setValue (StreamPlaylistWindow::MediumPlayDuration);
	playDurationSlider->isVisible = false;

	removeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (StreamPlaylistWindow::removeButtonClicked, this);
	removeButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::DeletePlaylist).capitalized ());
	removeButton->isVisible = false;

	addItemButton = (Button *) addWidget (new Button (sprites->getSprite (MediaUi::AddPlaylistItemButtonSprite)));
	addItemButton->mouseClickCallback = Widget::EventCallbackContext (StreamPlaylistWindow::addItemButtonClicked, this);
	addItemButton->mouseEnterCallback = Widget::EventCallbackContext (StreamPlaylistWindow::addItemButtonMouseEntered, this);
	addItemButton->mouseExitCallback = Widget::EventCallbackContext (StreamPlaylistWindow::addItemButtonMouseExited, this);
	addItemButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
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

void StreamPlaylistWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
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
		startPositionSlider->isVisible = true;
		playDurationSlider->isVisible = true;
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
		startPositionSlider->isVisible = false;
		playDurationSlider->isVisible = false;
		removeButton->isVisible = false;
		addItemButton->isVisible = false;
	}
	expandToggle->setChecked (isExpanded, shouldSkipStateChangeCallback);
	refreshLayout ();
	resetNameLabel ();
}

void StreamPlaylistWindow::doRefresh () {
	windowWidth = App::instance->windowWidth * StreamPlaylistWindow::WindowWidthMultiplier;
	itemListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
	Panel::doRefresh ();
}

void StreamPlaylistWindow::refreshLayout () {
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
		startPositionSlider->flowRight (&x, y, &x2, &y2);
		playDurationSlider->flowRight (&x, y, &x2, &y2);
		shuffleToggle->centerVertical (y0, y2);
		startPositionSlider->centerVertical (y0, y2);
		playDurationSlider->centerVertical (y0, y2);

		x = x0;
		x2 = 0.0f;
		y = y2 + UiConfiguration::instance->marginSize;
		y0 = y;
		itemCountLabel->flowRight (&x, y, &x2, &y2);
		addItemButton->flowRight (&x, y, &x2, &y2);
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

void StreamPlaylistWindow::resetNameLabel () {
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

void StreamPlaylistWindow::nameLabelClicked (void *windowPtr, Widget *widgetPtr) {
	((StreamPlaylistWindow *) windowPtr)->eventCallback (((StreamPlaylistWindow *) windowPtr)->renameClickCallback);
}

void StreamPlaylistWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	Toggle *toggle;

	window = (StreamPlaylistWindow *) windowPtr;
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

StdString StreamPlaylistWindow::startPositionSliderValueName (float sliderValue) {
	switch ((int) sliderValue) {
		case StreamPlaylistWindow::ZeroStartPosition: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistZeroStartPositionDescription));
		}
		case StreamPlaylistWindow::NearBeginningStartPosition: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistNearBeginningStartPositionDescription));
		}
		case StreamPlaylistWindow::MiddleStartPosition: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistMiddleStartPositionDescription));
		}
		case StreamPlaylistWindow::NearEndStartPosition: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistNearEndPositionDescription));
		}
		case StreamPlaylistWindow::FullRangeStartPosition: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistFullRangePositionDescription));
		}
	}

	return (StdString (""));
}

StdString StreamPlaylistWindow::playDurationSliderValueName (float sliderValue) {
	switch ((int) sliderValue) {
		case StreamPlaylistWindow::VeryShortPlayDuration: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistVeryShortPlayDurationDescription));
		}
		case StreamPlaylistWindow::ShortPlayDuration: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistShortPlayDurationDescription));
		}
		case StreamPlaylistWindow::MediumPlayDuration: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistMediumPlayDurationDescription));
		}
		case StreamPlaylistWindow::LongPlayDuration: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistLongPlayDurationDescription));
		}
		case StreamPlaylistWindow::VeryLongPlayDuration: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistVeryLongPlayDurationDescription));
		}
		case StreamPlaylistWindow::FullPlayDuration: {
			return (UiText::instance->getText (UiTextString::StreamPlaylistFullPlayDurationDescription));
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
	window->eventCallback (window->expandStateChangeCallback);
}

void StreamPlaylistWindow::itemListChanged (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	int count;

	window = (StreamPlaylistWindow *) windowPtr;
	count = window->itemListView->getItemCount ();
	window->itemCountLabel->setText (StdString::createSprintf ("%i", count));
	window->itemCountLabel->tooltipText.assign (UiText::instance->getCountText (count, UiTextString::StreamPlaylistItem, UiTextString::StreamPlaylistItems));
	window->refreshLayout ();
	window->eventCallback (window->listChangeCallback);
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
	itemListView->scrollToBottom ();
}

void StreamPlaylistWindow::freeItem (void *itemPtr) {
	delete ((StreamPlaylistWindow::Item *) itemPtr);
}

Widget::Rectangle StreamPlaylistWindow::getAddItemButtonScreenRect () {
	return (addItemButton->getScreenRect ());
}

Widget::Rectangle StreamPlaylistWindow::getRemoveButtonScreenRect () {
	return (removeButton->getScreenRect ());
}

void StreamPlaylistWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((StreamPlaylistWindow *) windowPtr)->eventCallback (((StreamPlaylistWindow *) windowPtr)->removeClickCallback);
}

void StreamPlaylistWindow::addItemButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((StreamPlaylistWindow *) windowPtr)->eventCallback (((StreamPlaylistWindow *) windowPtr)->addItemClickCallback);
}

void StreamPlaylistWindow::addItemButtonMouseEntered (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	Button *button;

	window = (StreamPlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseEntered (button, button);
	window->eventCallback (window->addItemMouseEnterCallback);
}

void StreamPlaylistWindow::addItemButtonMouseExited (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	Button *button;

	window = (StreamPlaylistWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);
	window->eventCallback (window->addItemMouseExitCallback);
}

Json *StreamPlaylistWindow::createState () {
	Json *state, *itemobj;
	StreamPlaylistWindow::Item *item;
	std::list<Json *> items;
	int i, count;

	state = new Json ();
	state->set (StreamPlaylistWindow::PlaylistNameKey, playlistName);
	state->set (StreamPlaylistWindow::IsSelectedKey, isSelected);
	state->set (StreamPlaylistWindow::IsExpandedKey, isExpanded);
	state->set (StreamPlaylistWindow::IsShuffleKey, shuffleToggle->isChecked);
	state->set (StreamPlaylistWindow::StartPositionKey, (int) startPositionSlider->value);
	state->set (StreamPlaylistWindow::PlayDurationKey, (int) playDurationSlider->value);

	count = itemListView->getItemCount ();
	for (i = 0; i < count; ++i) {
		item = (StreamPlaylistWindow::Item *) itemListView->getItemData (i);
		if (item) {
			itemobj = new Json ();
			itemobj->set (StreamPlaylistWindow::StreamUrlKey, item->streamUrl);
			itemobj->set (StreamPlaylistWindow::StreamIdKey, item->streamId);
			itemobj->set (StreamPlaylistWindow::MediaNameKey, item->mediaName);
			itemobj->set (StreamPlaylistWindow::StartPositionKey, item->startPosition);
			itemobj->set (StreamPlaylistWindow::ThumbnailUrlKey, item->thumbnailUrl);
			itemobj->set (StreamPlaylistWindow::ThumbnailIndexKey, item->thumbnailIndex);
			items.push_back (itemobj);
		}
	}
	state->set (StreamPlaylistWindow::ItemListKey, &items);

	return (state);
}

void StreamPlaylistWindow::readState (Json *state) {
	StdString val;
	StreamPlaylistWindow::Item item;
	Json itemobj;
	int i, count;
	bool b;

	val = state->getString (StreamPlaylistWindow::PlaylistNameKey, "");
	if (! val.empty ()) {
		setPlaylistName (val);
	}

	shuffleToggle->setChecked (state->getBoolean (StreamPlaylistWindow::IsShuffleKey, false), true);
	startPositionSlider->setValue ((float) state->getNumber (StreamPlaylistWindow::StartPositionKey, StreamPlaylistWindow::ZeroStartPosition));
	playDurationSlider->setValue ((float) state->getNumber (StreamPlaylistWindow::PlayDurationKey, StreamPlaylistWindow::MediumPlayDuration));

	itemListView->clearItems ();
	count = state->getArrayLength (StreamPlaylistWindow::ItemListKey);
	for (i = 0; i < count; ++i) {
		if (state->getArrayObject (StreamPlaylistWindow::ItemListKey, i, &itemobj)) {
			item.streamUrl.assign (itemobj.getString (StreamPlaylistWindow::StreamUrlKey, ""));
			item.streamId.assign (itemobj.getString (StreamPlaylistWindow::StreamIdKey, ""));
			item.mediaName.assign (itemobj.getString (StreamPlaylistWindow::MediaNameKey, ""));
			item.startPosition = itemobj.getNumber (StreamPlaylistWindow::StartPositionKey, 0.0f);
			item.thumbnailUrl.assign (itemobj.getString (StreamPlaylistWindow::ThumbnailUrlKey, ""));
			item.thumbnailIndex = itemobj.getNumber (StreamPlaylistWindow::ThumbnailIndexKey, (int) 0);

			addItem (item.streamUrl, item.streamId, item.mediaName, item.startPosition, item.thumbnailUrl, item.thumbnailIndex);
		}
	}

	b = state->getBoolean (StreamPlaylistWindow::IsSelectedKey, false);
	isSelected = ! b;
	setSelected (b, true);

	b = state->getBoolean (StreamPlaylistWindow::IsExpandedKey, false);
	isExpanded = ! b;
	setExpanded (b, true);
}

Json *StreamPlaylistWindow::createCommand () {
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

	return (App::instance->createCommand (SystemInterface::Command_CreateMediaDisplayIntent, obj));
}

void StreamPlaylistWindow::setStartPositionDelta (int startPosition, int *minStartPositionDelta, int *maxStartPositionDelta) {
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

	if (minStartPositionDelta) {
		*minStartPositionDelta = min;
	}
	if (maxStartPositionDelta) {
		*maxStartPositionDelta = max;
	}
}

void StreamPlaylistWindow::setStartPositionDelta (int startPosition, Json *createMediaDisplayIntentParams) {
	int min, max;

	StreamPlaylistWindow::setStartPositionDelta (startPosition, &min, &max);
	createMediaDisplayIntentParams->set ("minStartPositionDelta", min);
	createMediaDisplayIntentParams->set ("maxStartPositionDelta", max);
}

void StreamPlaylistWindow::setItemDisplayDuration (int playDuration, int *minItemDisplayDuration, int *maxItemDisplayDuration) {
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

	if (minItemDisplayDuration) {
		*minItemDisplayDuration = min;
	}
	if (maxItemDisplayDuration) {
		*maxItemDisplayDuration = max;
	}
}

void StreamPlaylistWindow::setItemDisplayDuration (int playDuration, Json *createMediaDisplayIntentParams) {
	int min, max;

	StreamPlaylistWindow::setItemDisplayDuration (playDuration, &min, &max);
	createMediaDisplayIntentParams->set ("minItemDisplayDuration", min);
	createMediaDisplayIntentParams->set ("maxItemDisplayDuration", max);
}
