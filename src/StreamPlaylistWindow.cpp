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
#include <list>
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
#include "IconLabelWindow.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "StreamPlaylistWindow.h"

const float StreamPlaylistWindow::windowWidthMultiplier = 0.45f;

StreamPlaylistWindow::StreamPlaylistWindow ()
: Panel ()
, isSelected (false)
, isExpanded (false)
, menuPositionX (0.0f)
, menuPositionY (0.0f)
, windowWidth (0.0f)
, iconImage (NULL)
, nameLabel (NULL)
, itemCountLabel (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, menuButton (NULL)
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
, menuClickCallback (NULL)
, menuClickCallbackData (NULL)
, listChangeCallback (NULL)
, listChangeCallbackData (NULL)
{
	App *app;
	UiConfiguration *uiconfig;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);
	windowWidth = app->windowWidth * StreamPlaylistWindow::windowWidthMultiplier;

	populate ();
	refreshLayout ();
}

StreamPlaylistWindow::~StreamPlaylistWindow () {

}

StdString StreamPlaylistWindow::toStringDetail () {
	return (StdString (" StreamPlaylistWindow"));
}

bool StreamPlaylistWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" StreamPlaylistWindow"));
}

StreamPlaylistWindow *StreamPlaylistWindow::castWidget (Widget *widget) {
	return (StreamPlaylistWindow::isWidgetType (widget) ? (StreamPlaylistWindow *) widget : NULL);
}

void StreamPlaylistWindow::populate () {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::PROGRAM_ICON)));
	nameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::HEADLINE, uiconfig->primaryTextColor)));
	nameLabel->setMouseClickCallback (StreamPlaylistWindow::nameLabelClicked, this);
	nameLabel->setMouseHoverTooltip (uitext->getText (UiTextString::clickRenameTooltip));

	itemCountLabel = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::STREAM_ICON), StdString ("0"), UiConfiguration::TITLE));
	itemCountLabel->setPadding (0.0f, 0.0f);
	itemCountLabel->setIconImageScale (0.5f);
	itemCountLabel->setMouseHoverTooltip (uitext->getCountText (0, UiTextString::streamPlaylistItem, UiTextString::streamPlaylistItems));

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::STAR_OUTLINE_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::STAR_BUTTON)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (StreamPlaylistWindow::selectToggleStateChanged, this);
	selectToggle->setMouseHoverTooltip (uitext->getText (UiTextString::selectToggleTooltip));

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::EXPAND_LESS_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::EXPAND_MORE_BUTTON)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (StreamPlaylistWindow::expandToggleStateChanged, this);
	expandToggle->setMouseHoverTooltip (uitext->getText (UiTextString::expandToggleTooltip));

	itemListView = (ListView *) addWidget (new ListView ((windowWidth - (widthPadding * 2.0f)), 6, UiConfiguration::CAPTION, StdString (""), uitext->getText (UiTextString::emptyStreamPlaylistText)));
	itemListView->setListChangeCallback (StreamPlaylistWindow::itemListChanged, this);
	itemListView->isVisible = false;

	menuButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::MAIN_MENU_BUTTON)));
	menuButton->setMouseClickCallback (StreamPlaylistWindow::menuButtonClicked, this);
	menuButton->setImageColor (uiconfig->flatButtonTextColor);
	menuButton->setMouseHoverTooltip (uitext->getText (UiTextString::moreActionsTooltip));

	shuffleToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle (), uitext->getText (UiTextString::shuffle).capitalized ()));
	shuffleToggle->setPadding (0.0f, 0.0f);
	shuffleToggle->setImageColor (uiconfig->flatButtonTextColor);
	shuffleToggle->setMouseHoverTooltip (uitext->getText (UiTextString::shuffleTooltip));
	shuffleToggle->isVisible = false;

	startPositionValueLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString ("WWWW - WWWW"), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor)));
	startPositionValueLabel->setWindowWidth (startPositionValueLabel->width);
	startPositionValueLabel->zLevel = 1;
	startPositionValueLabel->isVisible = false;

	startPositionLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::startPosition).capitalized (), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
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

	playDurationValueLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString ("WWWW - WWWW"), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor)));
	playDurationValueLabel->setWindowWidth (playDurationValueLabel->width);
	playDurationValueLabel->zLevel = 1;
	playDurationValueLabel->isVisible = false;

	playDurationLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::playDuration).capitalized (), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
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
	playDurationValueLabel->setText (StdString::createSprintf ("%s - %s", Util::getDurationDisplayString ((int64_t) playDurationMinSlider->value * 1000).c_str (), Util::getDurationDisplayString ((int64_t) playDurationMaxSlider->value * 1000).c_str ()));
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

void StreamPlaylistWindow::setMenuClickCallback (Widget::EventCallback callback, void *callbackData) {
	menuClickCallback = callback;
	menuClickCallbackData = callbackData;
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

void StreamPlaylistWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
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
		expandToggle->setChecked (false);
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

	if (shouldSkipStateChangeCallback) {
		expandStateChangeCallback = callback;
		expandStateChangeCallbackData = callbackdata;
	}
}

void StreamPlaylistWindow::doRefresh () {
	App *app;

	app = App::getInstance ();
	windowWidth = app->windowWidth * StreamPlaylistWindow::windowWidthMultiplier;
	itemListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
	Panel::doRefresh ();
}

void StreamPlaylistWindow::refreshLayout () {
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
		itemCountLabel->flowRight (&x, y, &x2, &y2);
	}
	expandToggle->flowRight (&x, y, &x2, &y2);
	selectToggle->flowRight (&x, y, &x2, &y2);
	if (! isExpanded) {
		itemCountLabel->centerVertical (y0, y2);
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
		menuButton->flowRight (&x, y, &x2, &y2);

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
	}

	setFixedSize (true, windowWidth, y2 + heightPadding);
	x = width - widthPadding;
	selectToggle->flowLeft (&x);
	expandToggle->flowLeft (&x);
	if (! isExpanded) {
		menuButton->flowLeft (&x);
		itemCountLabel->flowLeft (&x);
	}

	if (isExpanded) {
		x = width - widthPadding;
		menuButton->flowLeft (&x);
	}

	menuPositionX = menuButton->position.x;
	menuPositionY = menuButton->position.y + menuButton->height;
}

void StreamPlaylistWindow::resetNameLabel () {
	UiConfiguration *uiconfig;
	float w;

	uiconfig = &(App::getInstance ()->uiConfig);
	if (isExpanded) {
		w = expandToggle->position.x;
	}
	else {
		w = itemCountLabel->position.x;
	}
	w -= nameLabel->position.x;
	w -= uiconfig->marginSize;
	nameLabel->setText (Label::getTruncatedText (playlistName, UiConfiguration::HEADLINE, w, StdString ("...")));
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

	window = (StreamPlaylistWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->isSelected = toggle->isChecked;
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
	uitext = &(App::getInstance ()->uiText);
	count = window->itemListView->getItemCount ();
	window->itemCountLabel->setText (StdString::createSprintf ("%i", count));
	window->itemCountLabel->tooltipText.assign (uitext->getCountText (count, UiTextString::streamPlaylistItem, UiTextString::streamPlaylistItems));
	window->refreshLayout ();
	if (window->listChangeCallback) {
		window->listChangeCallback (window->listChangeCallbackData, window);
	}
}

void StreamPlaylistWindow::menuButtonClicked (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;

	window = (StreamPlaylistWindow *) windowPtr;
	if (window->menuClickCallback) {
		window->menuClickCallback (window->menuClickCallbackData, window);
	}
}

void StreamPlaylistWindow::addItem (const StdString &streamUrl, const StdString &streamId, const StdString &mediaName, float playPosition) {
	StreamPlaylistWindow::Item *item;

	item = new StreamPlaylistWindow::Item ();
	item->streamUrl.assign (streamUrl);
	item->streamId.assign (streamId);
	item->mediaName.assign (mediaName);
	item->playPosition = playPosition;
	itemListView->addItem (StdString::createSprintf ("%s %s", Util::getDurationString (playPosition, Util::HOURS).c_str (), mediaName.c_str ()), item, StreamPlaylistWindow::freeItem);
}

void StreamPlaylistWindow::freeItem (void *itemPtr) {
	delete ((StreamPlaylistWindow::Item *) itemPtr);
}

void StreamPlaylistWindow::startPositionSliderValueHovered (void *windowPtr, Widget *widgetPtr) {
	StreamPlaylistWindow *window;
	UiConfiguration *uiconfig;
	float min, max;

	window = (StreamPlaylistWindow *) windowPtr;
	uiconfig = &(App::getInstance ()->uiConfig);

	if (window->startPositionMinSlider->isHovering || window->startPositionMaxSlider->isHovering) {
		window->startPositionValueLabel->rotateTextColor (uiconfig->raisedButtonTextColor, uiconfig->shortColorRotateDuration);
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
		window->startPositionValueLabel->rotateTextColor (uiconfig->lightPrimaryTextColor, uiconfig->shortColorRotateDuration);
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
	uiconfig = &(App::getInstance ()->uiConfig);

	if (window->playDurationMinSlider->isHovering || window->playDurationMaxSlider->isHovering) {
		window->playDurationValueLabel->rotateTextColor (uiconfig->raisedButtonTextColor, uiconfig->shortColorRotateDuration);
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
		window->playDurationValueLabel->rotateTextColor (uiconfig->lightPrimaryTextColor, uiconfig->shortColorRotateDuration);
		min = window->playDurationMinSlider->value;
		max = window->playDurationMaxSlider->value;
	}

	min *= 1000.0f;
	max *= 1000.0f;
	if ((int64_t) min == (int64_t) max) {
		window->playDurationValueLabel->setText (StdString::createSprintf ("%s", Util::getDurationDisplayString ((int64_t) min).c_str ()));
	}
	else {
		window->playDurationValueLabel->setText (StdString::createSprintf ("%s - %s", Util::getDurationDisplayString ((int64_t) min).c_str (), Util::getDurationDisplayString ((int64_t) max).c_str ()));
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
		window->playDurationValueLabel->setText (StdString::createSprintf ("%s", Util::getDurationDisplayString ((int64_t) min).c_str ()));
	}
	else {
		window->playDurationValueLabel->setText (StdString::createSprintf ("%s - %s", Util::getDurationDisplayString ((int64_t) min).c_str (), Util::getDurationDisplayString ((int64_t) max).c_str ()));
	}
	window->refresh ();
}

static const char *PLAYLIST_NAME_KEY = "pn";
static const char *IS_SELECTED_KEY = "se";
static const char *IS_EXPANDED_KEY = "ex";
static const char *START_POSITION_MIN_KEY = "rs1";
static const char *START_POSITION_MAX_KEY = "rs2";
static const char *PLAY_DURATION_MIN_KEY = "rd1";
static const char *PLAY_DURATION_MAX_KEY = "rd2";
static const char *ITEM_LIST_KEY = "it";
static const char *IS_SHUFFLE_KEY = "sh";
static const char *STREAM_URL_KEY = "su";
static const char *STREAM_ID_KEY = "si";
static const char *MEDIA_NAME_KEY = "mn";
static const char *PLAY_POSITION_KEY = "pp";
Json *StreamPlaylistWindow::getState () {
	Json *obj, *itemobj;
	StreamPlaylistWindow::Item *item;
	std::list<Json *> items;
	int i, count;

	obj = new Json ();
	obj->set (PLAYLIST_NAME_KEY, playlistName);
	obj->set (IS_SELECTED_KEY, isSelected);
	obj->set (IS_EXPANDED_KEY, isExpanded);
	obj->set (IS_SHUFFLE_KEY, shuffleToggle->isChecked);
	obj->set (START_POSITION_MIN_KEY, startPositionMinSlider->value);
	obj->set (START_POSITION_MAX_KEY, startPositionMaxSlider->value);
	obj->set (PLAY_DURATION_MIN_KEY, playDurationMinSlider->value);
	obj->set (PLAY_DURATION_MAX_KEY, playDurationMaxSlider->value);

	count = itemListView->getItemCount ();
	for (i = 0; i < count; ++i) {
		item = (StreamPlaylistWindow::Item *) itemListView->getItemData (i);
		if (item) {
			itemobj = new Json ();
			itemobj->set (STREAM_URL_KEY, item->streamUrl.c_str ());
			itemobj->set (STREAM_ID_KEY, item->streamId.c_str ());
			itemobj->set (MEDIA_NAME_KEY, item->mediaName.c_str ());
			itemobj->set (PLAY_POSITION_KEY, item->playPosition);

			items.push_back (itemobj);
		}
	}
	obj->set (ITEM_LIST_KEY, &items);

	return (obj);
}

void StreamPlaylistWindow::setState (Json *stateObject) {
	StdString val;
	StreamPlaylistWindow::Item item;
	Json itemobj;
	int i, count;
	bool b;

	val = stateObject->getString (PLAYLIST_NAME_KEY, "");
	if (! val.empty ()) {
		setPlaylistName (val);
	}

	shuffleToggle->setChecked (stateObject->getBoolean (IS_SHUFFLE_KEY, false));
	startPositionMinSlider->setValue (stateObject->getNumber (START_POSITION_MIN_KEY, 60.0f));
	startPositionMaxSlider->setValue (stateObject->getNumber (START_POSITION_MAX_KEY, 300.0f));
	playDurationMinSlider->setValue (stateObject->getNumber (PLAY_DURATION_MIN_KEY, 60.0f));
	playDurationMaxSlider->setValue (stateObject->getNumber (PLAY_DURATION_MAX_KEY, 300.0f));

	count = stateObject->getArrayLength (ITEM_LIST_KEY);
	for (i = 0; i < count; ++i) {
		if (stateObject->getArrayObject (ITEM_LIST_KEY, i, &itemobj)) {
			item.streamUrl.assign (itemobj.getString (STREAM_URL_KEY, ""));
			item.streamId.assign (itemobj.getString (STREAM_ID_KEY, ""));
			item.mediaName.assign (itemobj.getString (MEDIA_NAME_KEY, ""));
			item.playPosition = itemobj.getNumber (PLAY_POSITION_KEY, 0.0f);

			addItem (item.streamUrl, item.streamId, item.mediaName, item.playPosition);
		}
	}

	b = stateObject->getBoolean (IS_SELECTED_KEY, false);
	isSelected = ! b;
	setSelected (b, true);

	b = stateObject->getBoolean (IS_EXPANDED_KEY, false);
	isExpanded = ! b;
	setExpanded (b, true);
}

Json *StreamPlaylistWindow::getCreateCommand () {
	App *app;
	std::list<Json *> items;
	StreamPlaylistWindow::Item *item;
	Json *obj, *itemparams, *playparams, *cmd;
	StdString streamurl;
	int i, count;

	app = App::getInstance ();
	obj = new Json ();
	obj->set ("displayName", playlistName);

	count = itemListView->getItemCount ();
	for (i = 0; i < count; ++i) {
		item = (StreamPlaylistWindow::Item *) itemListView->getItemData (i);
		if (item) {
			itemparams = new Json ();
			itemparams->set ("mediaName", item->mediaName);

			streamurl.assign (item->streamUrl);
			playparams = new Json ();
			playparams->set ("streamId", item->streamId);
			playparams->set ("startPosition", item->playPosition / 1000.0f);
			if ((startPositionMinSlider->value > 0.0f) || (startPositionMaxSlider->value > 0.0f)) {
				playparams->set ("minStartPositionDelta", (int) (startPositionMinSlider->value * 99.0f));
				playparams->set ("maxStartPositionDelta", (int) (startPositionMaxSlider->value * 99.0f));
			}

			cmd = app->createCommand ("GetHlsManifest", SystemInterface::Constant_Stream, playparams);
			if (cmd) {
				streamurl.appendSprintf ("?%s=%s", SystemInterface::Constant_UrlQueryParameter, cmd->toString ().urlEncoded ().c_str ());
				delete (cmd);
			}
			itemparams->set ("streamUrl", streamurl);
			items.push_back (itemparams);
		}
	}
	obj->set ("items", &items);
	obj->set ("isShuffle", shuffleToggle->isChecked);
	obj->set ("minItemDisplayDuration", (int) playDurationMinSlider->value);
	obj->set ("maxItemDisplayDuration", (int) playDurationMaxSlider->value);

	return (App::getInstance ()->createCommand ("CreateMediaDisplayIntent", SystemInterface::Constant_Monitor, obj));
}
