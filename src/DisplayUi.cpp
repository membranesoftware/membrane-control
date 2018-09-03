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
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "StringList.h"
#include "App.h"
#include "Resource.h"
#include "SpriteGroup.h"
#include "Util.h"
#include "AgentControl.h"
#include "Button.h"
#include "ComboBox.h"
#include "Menu.h"
#include "Toggle.h"
#include "Chip.h"
#include "CardView.h"
#include "RowView.h"
#include "DisplayServerWindow.h"
#include "StreamWindow.h"
#include "ThumbnailWindow.h"
#include "TextFieldWindow.h"
#include "IconCardWindow.h"
#include "SystemInterface.h"
#include "Ui.h"
#include "StreamItemUi.h"
#include "DisplayUi.h"

// TODO: Remove references to this macro (when Membrane Media Server is available and the display interface should be enabled)
#ifndef ENABLE_INTERFACE
#define ENABLE_INTERFACE 0
#endif

const int DisplayUi::readEventsPeriod = 60 * 1000; // ms
const float DisplayUi::smallImageScale = 0.123f;
const float DisplayUi::mediumImageScale = 0.240f;
const float DisplayUi::largeImageScale = 0.480f;

DisplayUi::DisplayUi ()
: Ui ()
, cardView (NULL)
, selectedPlayPosition (-1.0f)
, streamCardLayout (-1)
, streamCardMaxImageWidth (0.0f)
, lastReadEventsTime (0)
{

}

DisplayUi::~DisplayUi () {

}

StdString DisplayUi::getSpritePath () {
	return (StdString ("ui/DisplayUi/sprite"));
}

StdString DisplayUi::getBreadcrumbTitle () {
	return (App::getInstance ()->uiText.displays.capitalized ());
}

Sprite *DisplayUi::getBreadcrumbSprite () {
	return (sprites.getSprite (DisplayUi::BREADCRUMB_ICON)->copy ());
}

int DisplayUi::doLoad () {
	App *app;
	UiConfiguration *uiconfig;
	int layout;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);

	cardView = (CardView *) addWidget (new CardView (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->setItemMarginSize (uiconfig->marginSize / 2.0f);
	cardView->sort (DisplayUi::sortCards);
	cardView->setItemClickCallback (DisplayUi::streamCardClicked, this);
	cardView->position.assign (0.0f, app->topBarHeight);

	layout = app->prefsMap.find (App::prefsDisplaysImageSize, (int) StreamWindow::MEDIUM_DETAIL);
	switch (layout) {
		case StreamWindow::LOW_DETAIL: {
			DisplayUi::viewSmallActionClicked (this, NULL);
			break;
		}
		case StreamWindow::MEDIUM_DETAIL: {
			DisplayUi::viewMediumActionClicked (this, NULL);
			break;
		}
		case StreamWindow::HIGH_DETAIL: {
			DisplayUi::viewLargeActionClicked (this, NULL);
			break;
		}
		default: {
			DisplayUi::viewMediumActionClicked (this, NULL);
			break;
		}
	}

#if (! ENABLE_INTERFACE)
	IconCardWindow *emptycard;
	UiText *uitext;

	uitext = &(app->uiText);
	emptycard = new IconCardWindow (sprites.getSprite (DisplayUi::EMPTY_INTERFACE_ICON), uitext->emptyInterfaceTitle, StdString (""), uitext->displayUiEmptyInterfaceText);
	emptycard->setLink (uitext->learnMore.capitalized (), Util::getFeatureUrl ("build-a-movie-wall"));
	cardView->addItem (emptycard, StdString ("EmptyCard"), 0);
#endif

	return (Result::SUCCESS);
}

void DisplayUi::doUnload () {
	viewMenu.clear ();
	agentNameChip.clear ();
	playTargetChip.clear ();
	selectedStreamCard.clear ();
	addressToggle.clear ();
	addressTextFieldWindow.clear ();
}

void DisplayUi::doResetMainToolbar (Toolbar *toolbar) {
	Button *button;

	button = new Button (StdString (""), sprites.getSprite (DisplayUi::VIEW_BUTTON));
	button->setMouseClickCallback (DisplayUi::viewButtonClicked, this);
// TODO: Assign tooltip text
//     button->tooltipText.assign (app->uiText.mediaUiViewTooltip);
//     button->isMouseHoverEnabled = true;
	toolbar->addRightItem (button);

	addMainToolbarBackButton (toolbar);
}

void DisplayUi::doResetSecondaryToolbar (Toolbar *toolbar) {
	App *app;
	UiConfiguration *uiconfig;
	Button *button;
	Toggle *toggle;
	RowView *row;
	Chip *chip;
	StreamWindow *streamcard;
	StdString text;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);

	toolbar->setLeftCorner (new Image (sprites.getSprite (DisplayUi::COMMAND_ICON)));

	row = new RowView ();
	toggle = new Toggle (sprites.getSprite (DisplayUi::ADDRESS_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::CANCEL_BUTTON));
	toggle->setStateChangeCallback (DisplayUi::addressToggleStateChanged, this);
	// TODO: Set tooltip text
//	toggle->tooltipText.assign (app->uiText.linkUiAddressTooltip);
//	toggle->isMouseHoverEnabled = true;
	addressToggle.assign (toggle);
	row->addItem (toggle);

	button = new Button (StdString (""), sprites.getSprite (DisplayUi::STOP_BUTTON));
	button->setMouseClickCallback (DisplayUi::stopButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.stop.capitalized ());
	row->addItem (button);

	button = new Button (StdString (""), sprites.getSprite (DisplayUi::PLAY_BUTTON));
	button->setMouseClickCallback (DisplayUi::playButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.play.capitalized ());
	row->addItem (button);

	toolbar->setRightCorner (row);

	cardView->processItems (DisplayUi::appendDisplayCardName, &text);
	if (! text.empty ()) {
		// TODO: Possibly show an agent count number in the truncate suffix, i.e. ...(5)
		Label::truncateText (&text, UiConfiguration::CAPTION, toolbar->width * 0.25f, StdString ("..."));
		chip = new Chip (text);
		agentNameChip.destroyAndClear ();
		agentNameChip.assign (chip);
		toolbar->addLeftItem (chip);
	}

	streamcard = (StreamWindow *) selectedStreamCard.widget;
	if (streamcard && (selectedPlayPosition >= 0.0f)) {
		text.sprintf ("%s @ %s", streamcard->streamName.c_str (), Util::getDurationString (selectedPlayPosition, Util::HOURS).c_str ());
		chip = new Chip (text);
		playTargetChip.destroyAndClear ();
		playTargetChip.assign (chip);
		toolbar->addRightItem (chip);
	}

	toolbar->isVisible = true;
}

void DisplayUi::doResize () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void DisplayUi::doClearPopupWidgets () {
	viewMenu.destroyAndClear ();
}

void DisplayUi::doResume () {
	App *app;
	Json *params;
	int64_t now;

	app = App::getInstance ();
	app->setNextBackgroundTexturePath ("ui/DisplayUi/bg");

	now = Util::getTime ();
	if ((now - lastReadEventsTime) >= DisplayUi::readEventsPeriod) {
		params = new Json ();
		params->set ("commandId", SystemInterface::Command_AgentStatus);
		app->agentControl.writeLinkCommand (app->createCommandJson ("ReadEvents", SystemInterface::Constant_Link, params));

		params = new Json ();
		params->set ("commandId", SystemInterface::Command_StreamItem);
		app->agentControl.writeLinkCommand (app->createCommandJson ("ReadEvents", SystemInterface::Constant_Link, params));

		lastReadEventsTime = now;
	}

	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void DisplayUi::doPause () {
	App *app;

	app = App::getInstance ();
	app->prefsMap.insert (App::prefsDisplaysImageSize, streamCardLayout);
}

void DisplayUi::doRefresh () {
	App *app;

	app = App::getInstance ();
	cardView->setViewSize (app->windowWidth - app->rightBarWidth, app->windowHeight - app->topBarHeight - app->bottomBarHeight);
	cardView->position.assign (0.0f, app->topBarHeight);
}

void DisplayUi::doUpdate (int msElapsed) {
	viewMenu.compact ();
	agentNameChip.compact ();
	playTargetChip.compact ();
	selectedStreamCard.compact ();
	addressToggle.compact ();
	addressTextFieldWindow.compact ();
}

void DisplayUi::doSyncRecordStore (RecordStore *store) {
#if ENABLE_INTERFACE
	store->processAgentRecords ("displayServerStatus", DisplayUi::processDisplayServerRecord, this);
	store->processRecords (SystemInterface::Command_StreamItem, DisplayUi::processStreamItemRecord, this);

	cardView->refresh ();
#endif
}

void DisplayUi::processDisplayServerRecord (void *uiPtr, Json *record, const StdString &agentId) {
	DisplayUi *ui;
	App *app;
	DisplayServerWindow *card;
	StdString cardid;

	ui = (DisplayUi *) uiPtr;
	app = App::getInstance ();

	if (! ui->cardView->contains (agentId)) {
		if (app->systemInterface.getCommandRecordAge (record, Util::getTime ()) <= (app->prefsMap.find (App::prefsServerTimeout, App::defaultServerTimeout) * 1000)) {
			card = (DisplayServerWindow *) ui->cardView->addItem (new DisplayServerWindow (record, ui->sprites.getSprite (DisplayUi::DISPLAY_ICON), DisplayServerWindow::DISPLAY_UI_CARD), agentId, 0, true);
			card->setExtraData (&(card->agentName));
			card->setSelectStateChangeCallback (DisplayUi::displayCardSelectToggleStateChanged, ui);
		}
	}
}

void DisplayUi::processStreamItemRecord (void *uiPtr, Json *record, const StdString &recordId) {
	DisplayUi *ui;

	ui = (DisplayUi *) uiPtr;
	if (! ui->cardView->contains (recordId)) {
		ui->cardView->addItem (new StreamWindow (record, ui->sprites.getSprite (DisplayUi::LOADING_IMAGE_ICON), ui->streamCardLayout, ui->streamCardMaxImageWidth), recordId, 1, true);
	}
}

void DisplayUi::viewButtonClicked (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;

	ui = (DisplayUi *) uiPtr;
	ui->handleViewButtonClick (widgetPtr);
}

void DisplayUi::handleViewButtonClick (Widget *buttonWidget) {
	App *app;
	UiText *uitext;
	Menu *menu;

	app = App::getInstance ();
	uitext = &(app->uiText);

	suspendMouseHover ();
	if (viewMenu.widget) {
		viewMenu.destroyAndClear ();
		return;
	}

	clearPopupWidgets ();
	menu = (Menu *) app->rootPanel->addWidget (new Menu ());
	menu->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->small.capitalized (), sprites.getSprite (DisplayUi::SMALL_THUMBNAILS_ICON), DisplayUi::viewSmallActionClicked, this, 0, streamCardLayout == StreamWindow::LOW_DETAIL);
	menu->addItem (uitext->medium.capitalized (), sprites.getSprite (DisplayUi::MEDIUM_THUMBNAILS_ICON), DisplayUi::viewMediumActionClicked, this, 0, streamCardLayout == StreamWindow::MEDIUM_DETAIL);
	menu->addItem (uitext->large.capitalized (), sprites.getSprite (DisplayUi::LARGE_THUMBNAILS_ICON), DisplayUi::viewLargeActionClicked, this, 0, streamCardLayout == StreamWindow::HIGH_DETAIL);
	menu->position.assign (buttonWidget->position.x + buttonWidget->width - menu->width, buttonWidget->position.y + buttonWidget->height);
	viewMenu.assign (menu);
}

void DisplayUi::viewSmallActionClicked (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;
	float y0, h0;

	ui = (DisplayUi *) uiPtr;
	if (ui->streamCardLayout == StreamWindow::LOW_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->streamCardLayout = StreamWindow::LOW_DETAIL;
	ui->streamCardMaxImageWidth = ui->cardView->cardAreaWidth * DisplayUi::smallImageScale;
	if (ui->streamCardMaxImageWidth < 1.0f) {
		ui->streamCardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (DisplayUi::resetStreamCardLayout, ui);
	ui->cardView->setItemMarginSize (0.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.12f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void DisplayUi::viewMediumActionClicked (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;
	float y0, h0;

	ui = (DisplayUi *) uiPtr;
	if (ui->streamCardLayout == StreamWindow::MEDIUM_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->streamCardLayout = StreamWindow::MEDIUM_DETAIL;
	ui->streamCardMaxImageWidth = ui->cardView->cardAreaWidth * DisplayUi::mediumImageScale;
	if (ui->streamCardMaxImageWidth < 1.0f) {
		ui->streamCardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (DisplayUi::resetStreamCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize / 2.0f);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.24f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void DisplayUi::viewLargeActionClicked (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;
	float y0, h0;

	ui = (DisplayUi *) uiPtr;
	if (ui->streamCardLayout == StreamWindow::HIGH_DETAIL) {
		return;
	}

	y0 = ui->cardView->viewOriginY;
	h0 = ui->cardView->maxWidgetY;
	ui->streamCardLayout = StreamWindow::HIGH_DETAIL;
	ui->streamCardMaxImageWidth = ui->cardView->cardAreaWidth * DisplayUi::largeImageScale;
	if (ui->streamCardMaxImageWidth < 1.0f) {
		ui->streamCardMaxImageWidth = 1.0f;
	}
	ui->cardView->processItems (DisplayUi::resetStreamCardLayout, ui);
	ui->cardView->setItemMarginSize (App::getInstance ()->uiConfig.marginSize);
	ui->cardView->setVerticalScrollSpeed (ui->cardView->height * 0.36f);
	if (h0 > 0.0f) {
		ui->cardView->setViewOrigin (0.0f, (y0 * ui->cardView->maxWidgetY) / h0);
	}
}

void DisplayUi::resetStreamCardLayout (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;
	StreamWindow *window;

	ui = (DisplayUi *) uiPtr;
	window = StreamWindow::castWidget (widgetPtr);
	if (! window) {
		return;
	}

	window->setLayout (ui->streamCardLayout, ui->streamCardMaxImageWidth);
}

bool DisplayUi::sortCards (Widget *a, Widget *b) {
	StreamWindow *wa, *wb;

	wa = StreamWindow::castWidget (a);
	wb = StreamWindow::castWidget (b);
	if (!(wa && wb)) {
		return (false);
	}

	if (wa->streamName.lowercased ().compare (wb->streamName.lowercased ()) < 0) {
		return (true);
	}

	return (false);
}

void DisplayUi::streamCardClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	DisplayUi *ui;
	StreamWindow *card;
	StreamItemUi *itemui;

	card = StreamWindow::castWidget (widgetPtr);
	if (! card) {
		return;
	}

	ui = (DisplayUi *) uiPtr;
	app = App::getInstance ();

	ui->selectedStreamCard.assign (card);
	itemui = new StreamItemUi (card, app->uiText.selectPlayPosition.capitalized ());
	itemui->setThumbnailClickCallback (DisplayUi::streamUiThumbnailClicked, ui);
	app->pushUi (itemui);
}

void DisplayUi::displayCardSelectToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;

	ui = (DisplayUi *) uiPtr;
	ui->handleDisplayCardSelectToggleStateChanged (widgetPtr);
}

void DisplayUi::handleDisplayCardSelectToggleStateChanged (Widget *cardWidget) {
	Toolbar *toolbar;
	Chip *chip;
	StdString text;

	cardView->processItems (DisplayUi::appendDisplayCardName, &text);
	if (text.empty ()) {
		agentNameChip.destroyAndClear ();
		return;
	}

	toolbar = App::getInstance ()->secondaryToolbar;

	// TODO: Possibly show an agent count number in the truncate suffix, i.e. ...(5)
	Label::truncateText (&text, UiConfiguration::CAPTION, toolbar->width * 0.25f, StdString ("..."));

	if (! agentNameChip.widget) {
		chip = new Chip (text);
		agentNameChip.assign (chip);
		toolbar->addLeftItem (chip);
	}
	else {
		chip = (Chip *) agentNameChip.widget;
		chip->setText (text);
		toolbar->refresh ();
	}
}

void DisplayUi::appendDisplayCardName (void *textStringPtr, Widget *widgetPtr) {
	StdString *text;
	DisplayServerWindow *card;

	card = DisplayServerWindow::castWidget (widgetPtr);
	if ((! card) || (! card->isSelected)) {
		return;
	}

	text = (StdString *) textStringPtr;
	if (! text->empty ()) {
		text->append (", ");
	}
	text->append (card->agentName);
}

void DisplayUi::playButtonClicked (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;
	StreamWindow *card;
	App *app;
	Json *params, *cmd;
	StdString cmdjson, streamurl;
	StringList agentids;
	StringList::iterator i, end;
	int result;

	ui = (DisplayUi *) uiPtr;
	card = (StreamWindow *) ui->selectedStreamCard.widget;
	if ((! card) || card->hlsStreamUrl.empty () || (ui->selectedPlayPosition < 0.0f)) {
		return;
	}

	app = App::getInstance ();
	ui->cardView->processItems (DisplayUi::appendDisplayCardAgentId, &agentids);
	if (agentids.empty ()) {
		return;
	}

	params = new Json ();
	params->set ("streamId", card->streamId);
	params->set ("startPosition", ui->selectedPlayPosition / 1000.0f);
	cmdjson = app->createCommandJson ("GetHlsManifest", SystemInterface::Constant_Stream, params);
	streamurl = app->systemInterface.getInvokeUrl (card->hlsStreamUrl, cmdjson);

	i = agentids.begin ();
	end = agentids.end ();
	while (i != end) {
		params = new Json ();
		params->set ("mediaName", card->streamName);
		params->set ("streamUrl", streamurl);
		cmd = app->createCommand ("PlayMedia", SystemInterface::Constant_Display, params);
		if (cmd) {
			result = app->agentControl.invokeCommand (*i, cmd);
			if (result != Result::SUCCESS) {
				// TODO: Show an error message here
				Log::write (Log::ERR, "Failed to execute PlayMedia command; err=%i agentId=\"%s\"", result, i->c_str ());
			}
		}
		++i;
	}
}

void DisplayUi::appendDisplayCardAgentId (void *stringListPtr, Widget *widgetPtr) {
	StringList *destlist;
	DisplayServerWindow *card;

	card = DisplayServerWindow::castWidget (widgetPtr);
	if ((! card) || (! card->isSelected)) {
		return;
	}

	destlist = (StringList *) stringListPtr;
	destlist->push_back (card->agentId);
}

void DisplayUi::stopButtonClicked (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;
	App *app;
	Json *cmd;
	StringList agentids;
	StringList::iterator i, end;
	int result;

	ui = (DisplayUi *) uiPtr;

	app = App::getInstance ();

	ui->cardView->processItems (DisplayUi::appendDisplayCardAgentId, &agentids);
	if (agentids.empty ()) {
		return;
	}

	i = agentids.begin ();
	end = agentids.end ();
	while (i != end) {
		cmd = app->createCommand ("ClearDisplay", SystemInterface::Constant_Display, NULL);
		if (cmd) {
			result = app->agentControl.invokeCommand (*i, cmd);
			if (result != Result::SUCCESS) {
				// TODO: Show an error message here
				Log::write (Log::ERR, "Failed to execute ClearDisplay command; err=%i agentId=\"%s\"", result, i->c_str ());
			}
		}
		++i;
	}
}

void DisplayUi::streamUiThumbnailClicked (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;

	ui = (DisplayUi *) uiPtr;
	ui->handleStreamUiThumbnailClicked (widgetPtr);
}

void DisplayUi::handleStreamUiThumbnailClicked (Widget *cardWidget) {
	App *app;
	ThumbnailWindow *card;

	app = App::getInstance ();
	card = ThumbnailWindow::castWidget (cardWidget);
	if (! card) {
		return;
	}

	selectedPlayPosition = card->thumbnailTimestamp;
	app->popUi ();
}

void DisplayUi::addressToggleStateChanged (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;
	App *app;
	UiConfiguration *uiconfig;
	Toolbar *toolbar;
	Toggle *toggle;
	TextFieldWindow *textfield;
	float w;

	ui = (DisplayUi *) uiPtr;
	toggle = (Toggle *) widgetPtr;
	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	toolbar = app->secondaryToolbar;
	if (toggle->isChecked) {
		w = toolbar->width * 0.5f;

		textfield = new TextFieldWindow (w, app->uiText.enterUrlPrompt, ui->sprites.getSprite (DisplayUi::ADDRESS_BUTTON));
		textfield->setWindowHeight (toolbar->height);
		textfield->setButtonsEnabled (true, false, false, false);
		textfield->setFillBg (true, uiconfig->lightPrimaryColor);
		textfield->setEditCallback (DisplayUi::addressTextFieldEdited, ui);
		textfield->setEditing (true);
		toggle->setFillBg (true, uiconfig->mediumPrimaryColor);

		toolbar->setRightOverlay (textfield);
		ui->addressTextFieldWindow.assign (textfield);
	}
	else {
		ui->addressTextFieldWindow.destroyAndClear ();
		toggle->setFillBg (false);
	}
}

void DisplayUi::addressTextFieldEdited (void *uiPtr, Widget *widgetPtr) {
	DisplayUi *ui;
	TextFieldWindow *textfield;
	App *app;
	Json *params, *cmd;
	StringList agentids;
	StringList::iterator i, end;
	int result;

	ui = (DisplayUi *) uiPtr;
	app = App::getInstance ();
	textfield = (TextFieldWindow *) widgetPtr;
	ui->cardView->processItems (DisplayUi::appendDisplayCardAgentId, &agentids);
	if (agentids.empty ()) {
		return;
	}

	i = agentids.begin ();
	end = agentids.end ();
	while (i != end) {
		params = new Json ();
		params->set ("url", textfield->getValue ());
		cmd = app->createCommand ("ShowWebUrl", SystemInterface::Constant_Display, params);
		if (cmd) {
			result = app->agentControl.invokeCommand (*i, cmd);
			if (result != Result::SUCCESS) {
				// TODO: Show an error message here
				Log::write (Log::ERR, "Failed to execute ShowWebUrl command; err=%i agentId=\"%s\"", result, i->c_str ());
			}
		}
		++i;
	}
}
