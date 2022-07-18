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
#include <list>
#include "App.h"
#include "StdString.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "CommandHistory.h"
#include "Widget.h"
#include "Color.h"
#include "Font.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "TextFlow.h"
#include "IconLabelWindow.h"
#include "HistoryWindow.h"

HistoryWindow::HistoryWindow (float windowWidth, float windowHeight)
: Panel ()
, headerImage (NULL)
, isHeaderImageLoaded (false)
, titleLabel (NULL)
, closeButton (NULL)
, clearButton (NULL)
, itemView (NULL)
, scrollBar (NULL)
{
	setFixedSize (true, windowWidth, windowHeight);
	setFillBg (true, UiConfiguration::instance->mediumPrimaryColor);

	headerImage = (ImageWindow *) addWidget (new ImageWindow ());
	headerImage->setImageFilePath (StdString::createSprintf ("bg/history/%i.png", App::instance->imageScale));

	titleLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (UiText::instance->getText (UiTextString::Log).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor)));
	titleLabel->setPadding (0.0f, 0.0f);

	closeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExitButtonSprite)));
	closeButton->mouseClickCallback = Widget::EventCallbackContext (HistoryWindow::closeButtonClicked, this);
	closeButton->zLevel = 1;
	closeButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);

	clearButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeClearButtonSprite)));
	clearButton->mouseClickCallback = Widget::EventCallbackContext (HistoryWindow::clearButtonClicked, this);
	clearButton->zLevel = 1;
	clearButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	clearButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ClearCommandLogTooltip));
	clearButton->isVisible = false;

	itemView = (ScrollView *) addWidget (new ScrollView ());
	itemView->isKeyboardScrollEnabled = true;
	itemView->isMouseWheelScrollEnabled = true;
	itemView->setFillBg (true, UiConfiguration::instance->darkPrimaryColor);
	itemView->layoutSpacing = UiConfiguration::instance->marginSize / 2.0f;

	scrollBar = (ScrollBar *) addWidget (new ScrollBar (windowHeight - (UiConfiguration::instance->paddingSize * 2.0f)));
	scrollBar->positionChangeCallback = Widget::EventCallbackContext (HistoryWindow::scrollBarPositionChanged, this);
	scrollBar->zLevel = 3;
	scrollBar->isVisible = false;

	readHistory ();
}

HistoryWindow::~HistoryWindow () {
}

void HistoryWindow::setWindowSize (float windowWidth, float windowHeight) {
	setFixedSize (true, windowWidth, windowHeight);
	refreshLayout ();
}

void HistoryWindow::readHistory_processRecord (void *windowPtr, CommandHistory::Record *record) {
	HistoryWindow *window;
	HistoryItemWindow *item;

	window = (HistoryWindow *) windowPtr;
	item = new HistoryItemWindow (window->width, window->scrollBar->width, *record);
	item->executeClickCallback = Widget::EventCallbackContext (HistoryWindow::itemExecuteClicked, window);
	item->expandStateChangeCallback = Widget::EventCallbackContext (HistoryWindow::itemExpandStateChanged, window);
	window->itemList.push_back (item);
}

void HistoryWindow::itemExecuteClicked (void *windowPtr, Widget *widgetPtr) {
	HistoryWindow *window;
	HistoryItemWindow *item;

	window = (HistoryWindow *) windowPtr;
	item = (HistoryItemWindow *) widgetPtr;
	window->executeCommandId.assign (item->recordId);
	window->eventCallback (window->executeClickCallback);
}

void HistoryWindow::itemExpandStateChanged (void *windowPtr, Widget *widgetPtr) {
	HistoryWindow *window;

	window = (HistoryWindow *) windowPtr;
	window->refresh ();
}

static bool readHistory_compareItems (HistoryItemWindow *a, HistoryItemWindow *b) {
	if ((a->isExecutable && a->isSaved) && (!(b->isExecutable && b->isSaved))) {
		return (true);
	}
	if ((b->isExecutable && b->isSaved) && (!(a->isExecutable && a->isSaved))) {
		return (false);
	}
	if (a->commandTime >= b->commandTime) {
		return (true);
	}
	return (false);
}
void HistoryWindow::readHistory () {
	std::list<HistoryItemWindow *>::iterator i, end;
	Panel *emptystate;
	TextFlow *textflow;
	float w, maxw;

	itemView->clear ();
	itemList.clear ();
	CommandHistory::instance->processRecords (HistoryWindow::readHistory_processRecord, this);

	if (itemList.empty ()) {
		emptystate = new Panel ();
		emptystate->setLayout (Panel::VerticalLayout);
		emptystate->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
		emptystate->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);

		emptystate->addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::HistoryIconSprite)));

		w = UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth;
		maxw = width - (UiConfiguration::instance->paddingSize * 4.0f);
		if (w > maxw) {
			w = maxw;
		}
		textflow = (TextFlow *) emptystate->addWidget (new TextFlow (w, UiConfiguration::CaptionFont));
		textflow->setText (UiText::instance->getText (UiTextString::HistoryWindowEmptyStatusText));

		itemView->setLayout (Panel::NoLayout);
		itemView->addWidget (emptystate);
		emptystate->position.assign (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	}
	else {
		itemView->setLayout (Panel::VerticalLayout);
		itemList.sort (readHistory_compareItems);
		i = itemList.begin ();
		end = itemList.end ();
		while (i != end) {
			itemView->addWidget (*i);
			++i;
		}
	}
	itemView->setViewOrigin (0.0f, 0.0f);
	itemView->refresh ();
	refreshLayout ();
}

void HistoryWindow::refreshLayout () {
	float x, y, x2, y2;

	x = 0.0f;
	y = 0.0f;
	x2 = 0.0f;
	y2 = 0.0f;
	if (! headerImage->isLoaded ()) {
		x += UiConfiguration::instance->paddingSize;
		y += UiConfiguration::instance->paddingSize;
		headerImage->isVisible = false;
		clearButton->isVisible = false;

		titleLabel->setFillBg (false);
		titleLabel->setTextColor (UiConfiguration::instance->primaryTextColor);
		titleLabel->position.assign (x, y);

		closeButton->position.assign (width - closeButton->width - UiConfiguration::instance->paddingSize, y);
		y += closeButton->height;
	}
	else {
		headerImage->position.assign (x, y);
		headerImage->isVisible = true;

		x += UiConfiguration::instance->paddingSize;
		titleLabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
		titleLabel->setTextColor (UiConfiguration::instance->inverseTextColor);
		titleLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
		titleLabel->position.assign (x, y + headerImage->height - titleLabel->height - UiConfiguration::instance->paddingSize);

		x = width - UiConfiguration::instance->paddingSize;
		closeButton->flowLeft (&x, y + headerImage->height - closeButton->height - UiConfiguration::instance->paddingSize);
		clearButton->flowLeft (&x, y + headerImage->height - clearButton->height - UiConfiguration::instance->paddingSize);
		clearButton->isVisible = true;

		y += headerImage->height;
	}

	x = 0.0f;
	itemView->setViewSize (width, height - y);
	itemView->flowDown (x, &y, &x2, &y2);

	if (itemView->maxWidgetY < itemView->height) {
		itemView->setVerticalScrollBounds (0.0f, 0.0f);
		scrollBar->isVisible = false;
	}
	else {
		itemView->setVerticalScrollBounds (0.0f, itemView->maxWidgetY - (height - itemView->position.y));
		scrollBar->setMaxTrackLength (height - itemView->position.y);
		scrollBar->setScrollBounds (itemView->height, itemView->maxWidgetY);
		scrollBar->position.assign (width - scrollBar->width, itemView->position.y);
		scrollBar->isVisible = true;
	}
}

void HistoryWindow::doRefresh () {
	isHeaderImageLoaded = false;
	headerImage->setImageFilePath (StdString::createSprintf ("bg/history/%i.png", App::instance->imageScale));
	Panel::doRefresh ();
}

void HistoryWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (! isHeaderImageLoaded) {
		if (headerImage->isLoaded ()) {
			isHeaderImageLoaded = true;
			refreshLayout ();
		}
	}
}

bool HistoryWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	bool consumed;
	float y1;

	y1 = itemView->viewOriginY;
	consumed = Panel::doProcessMouseState (mouseState);
	if (! FLOAT_EQUALS (y1, itemView->viewOriginY)) {
		scrollBar->setPosition (itemView->viewOriginY, true);
	}
	return (consumed);
}

bool HistoryWindow::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	bool consumed;
	float y1;

	y1 = itemView->viewOriginY;
	consumed = Panel::doProcessKeyEvent (keycode, isShiftDown, isControlDown);
	if (! FLOAT_EQUALS (y1, itemView->viewOriginY)) {
		scrollBar->setPosition (itemView->viewOriginY, true);
	}
	return (consumed);
}

void HistoryWindow::closeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	HistoryWindow *window;

	window = (HistoryWindow *) windowPtr;
	window->isDestroyed = true;
}

void HistoryWindow::clearButtonClicked (void *windowPtr, Widget *widgetPtr) {
	HistoryWindow *window;

	window = (HistoryWindow *) windowPtr;
	CommandHistory::instance->clearUnsaved ();
	window->readHistory ();
}

void HistoryWindow::scrollBarPositionChanged (void *windowPtr, Widget *widgetPtr) {
	HistoryWindow *window;
	ScrollBar *scrollbar;

	window = (HistoryWindow *) windowPtr;
	scrollbar = (ScrollBar *) widgetPtr;
	window->itemView->setViewOrigin (0.0f, scrollbar->scrollPosition);
}

HistoryItemWindow::HistoryItemWindow (float windowWidth, float rightMargin, const CommandHistory::Record &record)
: Panel ()
, windowWidth (windowWidth)
, rightMargin (rightMargin)
, isExpanded (false)
, commandTime (0)
, isExecutable (false)
, isSaved (false)
, dividerPanel (NULL)
, nameLabel (NULL)
, detailLabel (NULL)
, agentNameIcon (NULL)
, timeIcon (NULL)
, commandStatusIcon (NULL)
, agentStatusPanel (NULL)
, saveToggle (NULL)
, executeButton (NULL)
, expandToggle (NULL)
{
	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setFillBg (true, UiConfiguration::instance->lightBackgroundColor);

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, windowWidth, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;

	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	detailLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));

	agentNameIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallServerIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	agentNameIcon->setPadding (0.0f, 0.0f);
	agentNameIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::TargetServers).capitalized (), Widget::LeftAlignment);

	timeIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ClockIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	timeIcon->setPadding (0.0f, 0.0f);
	timeIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::TimeOfExecution).capitalized (), Widget::LeftAlignment);
	timeIcon->isVisible = false;

	commandStatusIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallLoadingIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	commandStatusIcon->setPadding (0.0f, 0.0f);
	commandStatusIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::HistoryItemWindowCommandStatusTooltip), Widget::LeftAlignment);

	agentStatusPanel = (Panel *) addWidget (new Panel ());
	agentStatusPanel->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	agentStatusPanel->setLayout (Panel::VerticalLayout);
	agentStatusPanel->layoutSpacing = UiConfiguration::instance->marginSize / 2.0f;
	agentStatusPanel->isVisible = false;

	saveToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	saveToggle->stateChangeCallback = Widget::EventCallbackContext (HistoryItemWindow::saveToggleStateChanged, this);
	saveToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	saveToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::HistoryItemWindowUnselectedToggleTooltip), UiText::instance->getText (UiTextString::HistoryItemWindowSelectedToggleTooltip), Widget::LeftAlignment);
	saveToggle->isVisible = false;

	executeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ReloadButtonSprite)));
	executeButton->mouseClickCallback = Widget::EventCallbackContext (HistoryItemWindow::executeButtonClicked, this);
	executeButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	executeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::RepeatCommand).capitalized ());
	executeButton->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (HistoryItemWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());

	readRecord (record);
}

HistoryItemWindow::~HistoryItemWindow () {
}

void HistoryItemWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	if (isExpanded) {
		expandToggle->setChecked (true, shouldSkipStateChangeCallback);
		agentNameIcon->isVisible = false;
		timeIcon->isVisible = true;
		agentStatusPanel->isVisible = true;
	}
	else {
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		agentNameIcon->isVisible = true;
		timeIcon->isVisible = false;
		agentStatusPanel->isVisible = false;
	}
	refreshLayout ();
}

void HistoryItemWindow::readRecord (const CommandHistory::Record &record) {
	CommandHistory::CommandAgentMap::const_iterator i, end;
	StringList agentnames;
	StdString text, suffix;
	Panel *agentitem;
	int agentcount, errorcount;
	IconLabelWindow *agentpanelicon, *agentpanelstatus;
	float textw;

	commandTime = record.commandTime;
	isExecutable = record.isExecutable;
	isSaved = record.isSaved;
	agentcount = (int) record.agentMap.size ();
	errorcount = 0;
	agentStatusPanel->clear ();
	i = record.agentMap.cbegin ();
	end = record.agentMap.cend ();
	while (i != end) {
		if (i->second.status == CommandHistory::Failed) {
			++errorcount;
		}
		agentitem = (Panel *) agentStatusPanel->addWidget (new Panel ());
		agentitem->setLayout (Panel::HorizontalLayout);

		agentpanelicon = (IconLabelWindow *) agentitem->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite ((record.agentIconType >= 0) ? record.agentIconType : UiConfiguration::SmallServerIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
		agentpanelicon->setPadding (0.0f, 0.0f);
		agentpanelicon->setMouseHoverTooltip (StdString::createSprintf ("%s (%i)", UiText::instance->getText (UiTextString::TargetServers).capitalized ().c_str (), agentcount), Widget::LeftAlignment);
		textw = windowWidth - expandToggle->width - UiConfiguration::instance->marginSize - rightMargin - agentpanelicon->textLabelX;
		if (agentcount > 1) {
			agentpanelstatus = (IconLabelWindow *) agentitem->addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallLoadingIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
			agentpanelstatus->setPadding (0.0f, 0.0f);
			agentpanelstatus->setMouseHoverTooltip (UiText::instance->getText (UiTextString::HistoryItemWindowAgentCommandStatusTooltip), Widget::LeftAlignment);
			setStatusIcon (agentpanelstatus, i->second.status);
			textw -= (agentpanelstatus->width + UiConfiguration::instance->marginSize);
		}

		agentpanelicon->setText (UiConfiguration::instance->fonts[agentpanelicon->textFontType]->truncatedText (i->second.displayName, textw, Font::DotTruncateSuffix));
		agentitem->refresh ();
		++i;
	}
	agentStatusPanel->refresh ();

	recordId.assign (record.recordId);
	nameLabel->setText (record.name);
	timeIcon->setText (OsUtil::getTimestampDisplayString (commandTime));
	if ((record.status == CommandHistory::CompleteWithError) && (errorcount > 0)) {
		commandStatusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallErrorIconSprite));
		commandStatusIcon->setTextColor (UiConfiguration::instance->warningTextColor);
		commandStatusIcon->setText (UiText::instance->getCountText (errorcount, UiTextString::Error, UiTextString::Errors));
	}
	else {
		setStatusIcon (commandStatusIcon, record.status);
	}

	CommandHistory::instance->getCommandAgentNameList (record, &agentnames);
	text.assign (agentnames.join (", "));
	textw = windowWidth - expandToggle->width - commandStatusIcon->width - UiConfiguration::instance->marginSize - UiConfiguration::instance->paddingSize - rightMargin - agentNameIcon->textLabelX;
	suffix.assign ("...");
	if (agentcount > 1) {
		suffix.appendSprintf (" (%i)", agentcount);
	}
	agentNameIcon->setText (UiConfiguration::instance->fonts[agentNameIcon->textFontType]->truncatedText (text, textw, suffix));
	agentNameIcon->setMouseHoverTooltip (StdString::createSprintf ("%s (%i)", UiText::instance->getText (UiTextString::TargetServers).capitalized ().c_str (), agentcount), Widget::LeftAlignment);
	if (record.agentIconType >= 0) {
		agentNameIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (record.agentIconType));
	}

	textw = windowWidth - UiConfiguration::instance->paddingSize - rightMargin;
	if (isExecutable) {
		saveToggle->setChecked (isSaved, true);
		saveToggle->isVisible = true;
		textw -= (saveToggle->width + UiConfiguration::instance->marginSize);

		executeButton->isVisible = true;
		textw -= (executeButton->width + UiConfiguration::instance->marginSize);
	}
	else {
		saveToggle->isVisible = false;
		executeButton->isVisible = false;
	}

	if (record.detailText.empty ()) {
		detailLabel->isVisible = false;
	}
	else {
		detailLabel->setText (UiConfiguration::instance->fonts[detailLabel->textFontType]->truncatedText (record.detailText, textw, Font::DotTruncateSuffix));
		detailLabel->isVisible = true;
	}

	refreshLayout ();
}

void HistoryItemWindow::setStatusIcon (IconLabelWindow *icon, CommandHistory::CommandStatus status) {
	switch (status) {
		case CommandHistory::InProgress: {
			icon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallLoadingIconSprite));
			icon->setTextColor (UiConfiguration::instance->lightPrimaryTextColor);
			icon->setText (UiText::instance->getText (UiTextString::InProgress).capitalized ());
			break;
		}
		case CommandHistory::Complete: {
			icon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::CheckmarkSprite));
			icon->setTextColor (UiConfiguration::instance->statusOkTextColor);
			icon->setText (UiText::instance->getText (UiTextString::Complete).capitalized ());
			break;
		}
		case CommandHistory::CompleteWithError: {
			icon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallErrorIconSprite));
			icon->setTextColor (UiConfiguration::instance->warningTextColor);
			icon->setText (UiText::instance->getText (UiTextString::ServerError).capitalized ());
			break;
		}
		default: {
			icon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallErrorIconSprite));
			icon->setTextColor (UiConfiguration::instance->errorTextColor);
			icon->setText (UiText::instance->getText (UiTextString::Failed).capitalized ());
			break;
		}
	}
}

void HistoryItemWindow::refreshLayout () {
	float x, y, y0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x2 = 0.0f;
	y2 = 0.0f;
	y0 = y;

	nameLabel->flowDown (x, &y, &x2, &y2);
	if (detailLabel->isVisible) {
		detailLabel->flowDown (x, &y, &x2, &y2);
	}

	y = y0;
	if (saveToggle->isVisible) {
		saveToggle->flowRight (&x, y, &x2, &y2);
	}
	if (executeButton->isVisible) {
		executeButton->flowRight (&x, y, &x2, &y2);
	}
	x = windowWidth - rightMargin;
	if (saveToggle->isVisible) {
		saveToggle->flowLeft (&x);
	}
	if (executeButton->isVisible) {
		executeButton->flowLeft (&x);
	}
	y = y2 + UiConfiguration::instance->marginSize;
	if (! detailLabel->isVisible) {
		nameLabel->centerVertical (y0, y);
	}
	dividerPanel->flowDown (0.0f, &y, &x2, &y2);

	x = widthPadding;
	x2 = 0.0f;
	y = y2 + UiConfiguration::instance->marginSize;
	if (isExpanded) {
		agentStatusPanel->flowRight (&x, y, &x2, &y2);
	}
	else {
		agentNameIcon->flowRight (&x, y, &x2, &y2);
		commandStatusIcon->flowRight (&x, y, &x2, &y2);
	}
	expandToggle->flowDown (x, &y, &x2, &y2);
	x = windowWidth - rightMargin;
	expandToggle->flowLeft (&x);

	if (isExpanded) {
		x = widthPadding;
		x2 = 0.0f;
		y = y2 + UiConfiguration::instance->marginSize;
		timeIcon->flowRight (&x, y, &x2, &y2);
		commandStatusIcon->flowRight (&x, y, &x2, &y2);
	}

	y2 += heightPadding;
	setFixedSize (true, windowWidth, y2);
}

void HistoryItemWindow::saveToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	HistoryItemWindow *window;
	Toggle *toggle;

	window = (HistoryItemWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	CommandHistory::instance->setCommandSaved (window->recordId, toggle->isChecked);
	window->isSaved = toggle->isChecked;
}

void HistoryItemWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	HistoryItemWindow *window;
	Toggle *toggle;

	window = (HistoryItemWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	window->eventCallback (window->expandStateChangeCallback);
}

void HistoryItemWindow::executeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	HistoryItemWindow *window;

	window = (HistoryItemWindow *) windowPtr;
	window->eventCallback (window->executeClickCallback);
}
