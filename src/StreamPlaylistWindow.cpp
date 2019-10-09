/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "StreamPlaylistWindow.h"

const float StreamPlaylistWindow::windowWidthMultiplier = 0.45f;
const char *StreamPlaylistWindow::PlaylistNameKey = "a";
const char *StreamPlaylistWindow::IsSelectedKey = "b";
const char *StreamPlaylistWindow::IsExpandedKey = "c";
const char *StreamPlaylistWindow::StartPositionMinKey = "d";
const char *StreamPlaylistWindow::StartPositionMaxKey = "e";
const char *StreamPlaylistWindow::PlayDurationMinKey = "f";
const char *StreamPlaylistWindow::PlayDurationMaxKey = "g";
const char *StreamPlaylistWindow::ItemListKey = "h";
const char *StreamPlaylistWindow::IsShuffleKey = "i";
const char *StreamPlaylistWindow::StreamUrlKey = "j";
const char *StreamPlaylistWindow::StreamIdKey = "k";
const char *StreamPlaylistWindow::MediaNameKey = "l";
const char *StreamPlaylistWindow::StartPositionKey = "m";
const char *StreamPlaylistWindow::ThumbnailUrlKey = "n";
const char *StreamPlaylistWindow::ThumbnailIndexKey = "o";

StreamPlaylistWindow::StreamPlaylistWindow ()
: Panel ()
, isSelected (false)
, isExpanded (false)
, windowWidth (0.0f)
, iconImage (NULL)
, nameLabel (NULL)
, itemCountLabel (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, shuffleToggle (NULL)
, startPositionMinSlider (NULL)
, startPositionMaxSlider (NULL)
, startPositionValueLabel (NULL)
, startPositionLabel (NULL)
, playDurationMinSlider (NULL)
, playDurationMaxSlider (NULL)
, playDurationValueLabel (NULL)
, playDurationLabel (NULL)
, itemListView (NULL)
, selectStateChangeCallback (NULL)
, selectStateChangeCallbackData (NULL)
, expandStateChangeCallback (NULL)
, expandStateChangeCallbackData (NULL)
, renameClickCallback (NULL)
, renameClickCallbackData (NULL)
, listChangeCallback (NULL)
, listChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

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

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle (), uitext->getText (UiTextString::shuffle).capitalized ()));
	shuffleToggle->setPadding (0.0f, 0.0f);
	shuffleToggle->setImageColor (uiconfig->flatButtonTextColor);
	shuffleToggle->setMouseHoverTooltip (uitext->getText (UiTextString::shuffleTooltip));
	shuffleToggle->isVisible = false;

	startPositionValueLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString ("WWWW - WWWW"), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor)));
	startPositionValueLabel->setWindowWidth (startPositionValueLabel->width);
	startPositionValueLabel->zLevel = 1;
	startPositionValueLabel->isVisible = false;

	startPositionLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::startPosition).capitalized (), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	startPositionLabel->zLevel = 1;
	startPositionLabel->isVisible = false;

	startPositionMinSlider = (Slider *) addWidget (new Slider ());
	startPositionMinSlider->setValueHoverCallback (StreamPlaylistWindow::startPositionSliderValueHovered, this);
	startPositionMinSlider->setValueChangeCallback (StreamPlaylistWindow::startPositionSliderValueChanged, this);
	startPositionMinSlider->setTrackWidthScale (0.6f);
	startPositionMinSlider->isVisible = false;

	startPositionMaxSlider = (Slider *) addWidget (new Slider ());
	startPositionMaxSlider->setValueHoverCallback (StreamPlaylistWindow::startPositionSliderValueHovered, this);
	startPositionMaxSlider->setValueChangeCallback (StreamPlaylistWindow::startPositionSliderValueChanged, this);
	startPositionMaxSlider->setTrackWidthScale (0.6f);
	startPositionMaxSlider->isVisible = false;

	playDurationValueLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString ("WWWW - WWWW"), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor)));
	playDurationValueLabel->setWindowWidth (playDurationValueLabel->width);
	playDurationValueLabel->zLevel = 1;
	playDurationValueLabel->isVisible = false;

	playDurationLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::playDuration).capitalized (), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	playDurationLabel->zLevel = 1;
	playDurationLabel->isVisible = false;

	playDurationMinSlider = (Slider *) addWidget (new Slider (15.0f, 10800.0f));
	playDurationMinSlider->setValueHoverCallback (StreamPlaylistWindow::playDurationSliderValueHovered, this);
	playDurationMinSlider->setValueChangeCallback (StreamPlaylistWindow::playDurationSliderValueChanged, this);
	playDurationMinSlider->setTrackWidthScale (0.6f);
	playDurationMinSlider->isVisible = false;

	playDurationMaxSlider = (Slider *) addWidget (new Slider (15.0f, 10800.0f));
	playDurationMaxSlider->setValueHoverCallback (StreamPlaylistWindow::playDurationSliderValueHovered, this);
	playDurationMaxSlider->setValueChangeCallback (StreamPlaylistWindow::playDurationSliderValueChanged, this);
	playDurationMaxSlider->setTrackWidthScale (0.6f);
	playDurationMaxSlider->isVisible = false;

	startPositionMinSlider->setValue (0.0f, true);
	startPositionMaxSlider->setValue (0.0f, true);
	playDurationMinSlider->setValue (60.0f, true);
	playDurationMaxSlider->setValue (300.0f, true);
	startPositionValueLabel->setText (StdString::createSprintf ("%i%%", (int) startPositionMinSlider->value));
	playDurationValueLabel->setText (StdString::createSprintf ("%s - %s", OsUtil::getDurationDisplayString ((int64_t) playDurationMinSlider->value * 1000).c_str (), OsUtil::getDurationDisplayString ((int64_t) playDurationMaxSlider->value * 1000).c_str ()));

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

void StreamPlaylistWindow::setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	selectStateChangeCallback = callback;
	selectStateChangeCallbackData = callbackData;
}

void StreamPlaylistWindow::setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	expandStateChangeCallback = callback;
	expandStateChangeCallbackData = callbackData;
}

void StreamPlaylistWindow::setRenameClickCallback (Widget::EventCallback callback, void *callbackData) {
	renameClickCallback = callback;
	renameClickCallbackData = callbackData;
}

void StreamPlaylistWindow::setListChangeCallback (Widget::EventCallback callback, void *callbackData) {
	listChangeCallback = callback;
	listChangeCallbackData = callbackData;
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
		startPositionMinSlider->isVisible = true;
		startPositionMaxSlider->isVisible = true;
		startPositionValueLabel->isVisible = true;
		startPositionLabel->isVisible = true;
		playDurationMinSlider->isVisible = true;
		playDurationMaxSlider->isVisible = true;
		playDurationValueLabel->isVisible = true;
		playDurationLabel->isVisible = true;
	}
	else {
		setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		itemCountLabel->setTextFont (UiConfiguration::BodyFont);
		itemListView->isVisible = false;
		shuffleToggle->isVisible = false;
		startPositionMinSlider->isVisible = false;
		startPositionMaxSlider->isVisible = false;
		startPositionValueLabel->isVisible = false;
		startPositionLabel->isVisible = false;
		playDurationMinSlider->isVisible = false;
		playDurationMaxSlider->isVisible = false;
		playDurationValueLabel->isVisible = false;
		playDurationLabel->isVisible = false;
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
		startPositionValueLabel->flowRight (&x, y, &x2, &y2);
		startPositionMinSlider->flowRight (&x, y, &x2, &y2);
		startPositionMaxSlider->flowRight (&x, y, &x2, &y2);
		startPositionLabel->flowRight (&x, y, &x2, &y2);

		startPositionValueLabel->centerVertical (y0, y2);
		startPositionMinSlider->centerVertical (y0, y2);
		startPositionMaxSlider->centerVertical (y0, y2);
		startPositionLabel->centerVertical (y0, y2);

		x = x0;
		x2 = 0.0f;
		y = y2;
		y0 = y;
		playDurationValueLabel->flowRight (&x, y, &x2, &y2);
		playDurationMinSlider->flowRight (&x, y, &x2, &y2);
		playDurationMaxSlider->flowRight (&x, y, &x2, &y2);
		playDurationLabel->flowRight (&x, y, &x2, &y2);

		playDurationValueLabel->centerVertical (y0, y2);
		playDurationMinSlider->centerVertical (y0, y2);
		playDurationMaxSlider->centerVertical (y0, y2);
		playDurationLabel->centerVertical (y0, y2);

		x = x0;
		x2 = 0.0f;
		y = y2;
		y0 = y;
		itemListView->flowRight (&x, y, &x2, &y2);
		setFixedSize (true, windowWidth, y2 + heightPadding);
	}
	else {
		setFixedSize (false);
		resetSize ();
	}

	x = width - widthPadding;
	selectToggle->flowLeft (&x);
	expandToggle->flowLeft (&x);
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
	if (window->renameClickCallback) {
		window->renameClickCallback (window->renameClickCallbackData, window);
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
	if (window->selectStateChangeCallback) {
		window->selectStateChangeCallback (window->selectStateChangeCallbackData, window);
	}
}

void StreamPlaylistWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	Toggle *toggle;

	window = (StreamPlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback) {
		window->expandStateChangeCallback (window->expandStateChangeCallbackData, window);
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
	if (window->listChangeCallback) {
		window->listChangeCallback (window->listChangeCallbackData, window);
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

void StreamPlaylistWindow::startPositionSliderValueHovered (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	UiConfiguration *uiconfig;
	float min, max;

	window = (StreamPlaylistWindow *) windowPtr;
	uiconfig = &(App::instance->uiConfig);

	if (window->startPositionMinSlider->isHovering || window->startPositionMaxSlider->isHovering) {
		window->startPositionValueLabel->translateTextColor (uiconfig->raisedButtonTextColor, uiconfig->shortColorTranslateDuration);
		if (window->startPositionMinSlider->isHovering) {
			min = window->startPositionMinSlider->hoverValue;
		}
		else {
			min = window->startPositionMinSlider->value;
		}
		if (window->startPositionMaxSlider->isHovering) {
			max = window->startPositionMaxSlider->hoverValue;
		}
		else {
			max = window->startPositionMaxSlider->value;
		}
		if (max < min) {
			max = min;
		}
	}
	else {
		window->startPositionValueLabel->translateTextColor (uiconfig->lightPrimaryTextColor, uiconfig->shortColorTranslateDuration);
		min = window->startPositionMinSlider->value;
		max = window->startPositionMaxSlider->value;
	}

	min *= 99.0f;
	max *= 99.0f;
	if ((int) min == (int) max) {
		window->startPositionValueLabel->setText (StdString::createSprintf ("%i%%", (int) min));
	}
	else {
		window->startPositionValueLabel->setText (StdString::createSprintf ("%i - %i%%", (int) min, (int) max));
	}
	window->refresh ();
}

void StreamPlaylistWindow::startPositionSliderValueChanged (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	float min, max;

	window = (StreamPlaylistWindow *) windowPtr;
	if (window->startPositionMaxSlider->value < window->startPositionMinSlider->value) {
		window->startPositionMaxSlider->setValue (window->startPositionMinSlider->value, true);
	}
	min = window->startPositionMinSlider->value;
	max = window->startPositionMaxSlider->value;
	min *= 99.0f;
	max *= 99.0f;
	if ((int) min == (int) max) {
		window->startPositionValueLabel->setText (StdString::createSprintf ("%i%%", (int) min));
	}
	else {
		window->startPositionValueLabel->setText (StdString::createSprintf ("%i - %i%%", (int) min, (int) max));
	}
	window->refresh ();
}

void StreamPlaylistWindow::playDurationSliderValueHovered (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	UiConfiguration *uiconfig;
	float min, max;

	window = (StreamPlaylistWindow *) windowPtr;
	uiconfig = &(App::instance->uiConfig);

	if (window->playDurationMinSlider->isHovering || window->playDurationMaxSlider->isHovering) {
		window->playDurationValueLabel->translateTextColor (uiconfig->raisedButtonTextColor, uiconfig->shortColorTranslateDuration);
		if (window->playDurationMinSlider->isHovering) {
			min = window->playDurationMinSlider->hoverValue;
		}
		else {
			min = window->playDurationMinSlider->value;
		}
		if (window->playDurationMaxSlider->isHovering) {
			max = window->playDurationMaxSlider->hoverValue;
		}
		else {
			max = window->playDurationMaxSlider->value;
		}
		if (max < min) {
			max = min;
		}
	}
	else {
		window->playDurationValueLabel->translateTextColor (uiconfig->lightPrimaryTextColor, uiconfig->shortColorTranslateDuration);
		min = window->playDurationMinSlider->value;
		max = window->playDurationMaxSlider->value;
	}

	min *= 1000.0f;
	max *= 1000.0f;
	if ((int64_t) min == (int64_t) max) {
		window->playDurationValueLabel->setText (StdString::createSprintf ("%s", OsUtil::getDurationDisplayString ((int64_t) min).c_str ()));
	}
	else {
		window->playDurationValueLabel->setText (StdString::createSprintf ("%s - %s", OsUtil::getDurationDisplayString ((int64_t) min).c_str (), OsUtil::getDurationDisplayString ((int64_t) max).c_str ()));
	}
	window->refresh ();
}

void StreamPlaylistWindow::playDurationSliderValueChanged (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	float min, max;

	window = (StreamPlaylistWindow *) windowPtr;
	if (window->playDurationMaxSlider->value < window->playDurationMinSlider->value) {
		window->playDurationMaxSlider->setValue (window->playDurationMinSlider->value, true);
	}
	min = window->playDurationMinSlider->value;
	max = window->playDurationMaxSlider->value;
	min *= 1000.0f;
	max *= 1000.0f;
	if ((int64_t) min == (int64_t) max) {
		window->playDurationValueLabel->setText (StdString::createSprintf ("%s", OsUtil::getDurationDisplayString ((int64_t) min).c_str ()));
	}
	else {
		window->playDurationValueLabel->setText (StdString::createSprintf ("%s - %s", OsUtil::getDurationDisplayString ((int64_t) min).c_str (), OsUtil::getDurationDisplayString ((int64_t) max).c_str ()));
	}
	window->refresh ();
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
	obj->set (StreamPlaylistWindow::StartPositionMinKey, startPositionMinSlider->value);
	obj->set (StreamPlaylistWindow::StartPositionMaxKey, startPositionMaxSlider->value);
	obj->set (StreamPlaylistWindow::PlayDurationMinKey, playDurationMinSlider->value);
	obj->set (StreamPlaylistWindow::PlayDurationMaxKey, playDurationMaxSlider->value);

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
	startPositionMinSlider->setValue (stateObject->getNumber (StreamPlaylistWindow::StartPositionMinKey, 60.0f));
	startPositionMaxSlider->setValue (stateObject->getNumber (StreamPlaylistWindow::StartPositionMaxKey, 300.0f));
	playDurationMinSlider->setValue (stateObject->getNumber (StreamPlaylistWindow::PlayDurationMinKey, 60.0f));
	playDurationMaxSlider->setValue (stateObject->getNumber (StreamPlaylistWindow::PlayDurationMaxKey, 300.0f));

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
	obj->set ("minStartPositionDelta", (int) (startPositionMinSlider->value * 99.0f));
	obj->set ("maxStartPositionDelta", (int) (startPositionMaxSlider->value * 99.0f));
	obj->set ("minItemDisplayDuration", (int) playDurationMinSlider->value);
	obj->set ("maxItemDisplayDuration", (int) playDurationMaxSlider->value);

	return (App::instance->createCommand (SystemInterface::Command_CreateMediaDisplayIntent, SystemInterface::Constant_Monitor, obj));
}
