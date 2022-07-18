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
#include "SDL2/SDL.h"
#include "App.h"
#include "StdString.h"
#include "OsUtil.h"
#include "Log.h"
#include "Input.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Menu.h"
#include "TextField.h"
#include "LogoWindow.h"
#include "TooltipWindow.h"
#include "HistoryWindow.h"
#include "SettingsWindow.h"
#include "HelpWindow.h"
#include "ImageWindow.h"
#include "ConsoleWindow.h"
#include "TextFieldWindow.h"
#include "UiStack.h"

UiStack::UiStack ()
: mainToolbar (NULL)
, secondaryToolbar (NULL)
, snackbarWindow (NULL)
, topBarHeight (0.0f)
, bottomBarHeight (0.0f)
, rightBarWidth (0.0f)
, activeUi (NULL)
, uiMutex (NULL)
, nextCommandType (-1)
, nextCommandUi (NULL)
, nextCommandMutex (NULL)
, isUiInputSuspended (false)
, mouseHoverClock (0)
, isMouseHoverActive (false)
, isMouseHoverSuspended (false)
{
	uiMutex = SDL_CreateMutex ();
	nextCommandMutex = SDL_CreateMutex ();
}

UiStack::~UiStack () {
	clear ();
	if (activeUi) {
		activeUi->release ();
		activeUi = NULL;
	}
	if (uiMutex) {
		SDL_DestroyMutex (uiMutex);
		uiMutex = NULL;
	}
	if (nextCommandMutex) {
		SDL_DestroyMutex (nextCommandMutex);
		nextCommandMutex = NULL;
	}
}

void UiStack::populateWidgets () {
	if (! mainToolbar) {
		mainToolbar = (Toolbar *) App::instance->rootPanel->addWidget (new Toolbar (App::instance->windowWidth));
		mainToolbar->zLevel = 1;
		mainToolbar->setLeftCorner (new LogoWindow ());
	}
	topBarHeight = mainToolbar->position.y + mainToolbar->height;

	if (! secondaryToolbar) {
		secondaryToolbar = (Toolbar *) App::instance->rootPanel->addWidget (new Toolbar (App::instance->windowWidth));
		secondaryToolbar->zLevel = 1;
		secondaryToolbar->isVisible = false;
		secondaryToolbar->position.assign (0.0f, App::instance->windowHeight - secondaryToolbar->height);
	}
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;

	if (! snackbarWindow) {
		snackbarWindow = (SnackbarWindow *) App::instance->rootPanel->addWidget (new SnackbarWindow (App::instance->windowWidth - (UiConfiguration::instance->paddingSize * 2.0f)));
		snackbarWindow->isVisible = false;
	}
}

void UiStack::clear () {
	Ui *ui;

	tooltip.destroyAndClear ();
	keyFocusTarget.clear ();
	mouseHoverTarget.clear ();

	SDL_LockMutex (uiMutex);
	while (! uiList.empty ()) {
		ui = uiList.back ();
		ui->pause ();
		ui->unload ();
		ui->release ();
		uiList.pop_back ();
	}
	SDL_UnlockMutex (uiMutex);
}

Ui *UiStack::getActiveUi () {
	Ui *ui;

	ui = NULL;
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		ui = activeUi;
		ui->retain ();
	}
	SDL_UnlockMutex (uiMutex);

	return (ui);
}

void UiStack::setUi (Ui *ui) {
	if (! ui) {
		return;
	}

	SDL_LockMutex (nextCommandMutex);
	nextCommandType = UiStack::SetUiCommand;
	if (nextCommandUi) {
		nextCommandUi->release ();
	}
	nextCommandUi = ui;
	nextCommandUi->retain ();
	SDL_UnlockMutex (nextCommandMutex);
}

void UiStack::pushUi (Ui *ui) {
	if (! ui) {
		return;
	}

	SDL_LockMutex (nextCommandMutex);
	nextCommandType = UiStack::PushUiCommand;
	if (nextCommandUi) {
		nextCommandUi->release ();
	}
	nextCommandUi = ui;
	nextCommandUi->retain ();
	SDL_UnlockMutex (nextCommandMutex);
}

void UiStack::popUi () {
	SDL_LockMutex (nextCommandMutex);
	nextCommandType = UiStack::PopUiCommand;
	if (nextCommandUi) {
		nextCommandUi->release ();
	}
	nextCommandUi = NULL;
	SDL_UnlockMutex (nextCommandMutex);
}

void UiStack::update (int msElapsed) {
	Ui *ui;
	Widget *keywidget, *mousewidget;

	SDL_LockMutex (uiMutex);
	if (! uiList.empty ()) {
		ui = uiList.back ();
		if (ui != activeUi) {
			if (activeUi) {
				activeUi->pause ();
				activeUi->release ();
			}
			activeUi = ui;
			activeUi->retain ();

			tooltip.destroyAndClear ();
			keyFocusTarget.clear ();
			mouseHoverTarget.clear ();
			clearOverlay ();
			resetToolbars ();
			snackbarWindow->isVisible = false;
			activeUi->resume ();
			App::instance->shouldSyncRecordStore = true;
			App::instance->isNetworkActive = false;
			mainToolbar->isInputSuspended = false;
			secondaryToolbar->isInputSuspended = false;
			isUiInputSuspended = false;
		}
	}
	SDL_UnlockMutex (uiMutex);

	tooltip.compact ();
	keyFocusTarget.compact ();
	mouseHoverTarget.compact ();
	mainMenu.compact ();
	darkenPanel.compact ();
	historyWindow.compact ();
	settingsWindow.compact ();
	helpWindow.compact ();
	dialogWindow.compact ();
	consoleWindow.compact ();

	keywidget = keyFocusTarget.widget;
	if (keywidget && (! keywidget->isKeyFocused)) {
		keyFocusTarget.clear ();
	}

	mousewidget = App::instance->rootPanel->findWidget ((float) Input::instance->mouseX, (float) Input::instance->mouseY, true);
	if (! mousewidget) {
		mouseHoverTarget.clear ();
		deactivateMouseHover ();
	}
	else {
		if (! mouseHoverTarget.equals (mousewidget)) {
			mouseHoverTarget.assign (mousewidget);
			deactivateMouseHover ();
		}
		else {
			if ((! isMouseHoverActive) && (! isMouseHoverSuspended)) {
				mouseHoverClock -= msElapsed;
				if (mouseHoverClock <= 0) {
					activateMouseHover ();
				}
			}
		}
	}

	if (darkenPanel.widget && (! isDarkenOverlayActive ())) {
		darkenPanel.destroyAndClear ();
	}

	if (isUiInputSuspended) {
		if ((! darkenPanel.widget) && (! isDarkenOverlayActive ())) {
			ui = getActiveUi ();
			if (ui) {
				ui->rootPanel->isInputSuspended = false;
				ui->release ();
			}
			mainToolbar->isInputSuspended = false;
			secondaryToolbar->isInputSuspended = false;
			isUiInputSuspended = false;
		}
	}
}

void UiStack::resetToolbars () {
	Button *button;

	mainToolbar->clearLeftOverlay ();
	mainToolbar->clearRightItems ();
	secondaryToolbar->clearAll ();

	button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::MainMenuButtonSprite));
	button->mouseClickCallback = Widget::EventCallbackContext (UiStack::mainMenuButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MainMenuTooltip));
	mainToolbar->addRightItem (button);
	if (activeUi) {
		activeUi->addMainToolbarItems (mainToolbar);
		activeUi->addSecondaryToolbarItems (secondaryToolbar);
	}

	if (uiList.size () > 1) {
		button = new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::BackButtonSprite));
		button->widgetName.assign ("backbutton");
		button->mouseClickCallback = Widget::EventCallbackContext (UiStack::backButtonClicked, this);
		button->setInverseColor (true);
		button->setMouseHoverTooltip (UiText::instance->getText (UiTextString::UiBackTooltip));
		mainToolbar->addRightItem (button);
	}

	if (secondaryToolbar->empty ()) {
		secondaryToolbar->isVisible = false;
	}
	else {
		secondaryToolbar->isVisible = true;
	}

	topBarHeight = mainToolbar->position.y + mainToolbar->height;
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;
	secondaryToolbar->position.assign (0.0f, App::instance->windowHeight - secondaryToolbar->height);
}

void UiStack::executeStackCommands () {
	Ui *ui, *item;
	int cmd, result;

	SDL_LockMutex (nextCommandMutex);
	cmd = nextCommandType;
	ui = nextCommandUi;
	nextCommandType = -1;
	nextCommandUi = NULL;
	SDL_UnlockMutex (nextCommandMutex);

	if (cmd < 0) {
		if (ui) {
			ui->release ();
		}
		return;
	}

	switch (cmd) {
		case UiStack::SetUiCommand: {
			if (! ui) {
				break;
			}
			result = ui->load ();
			if (result != OsUtil::Success) {
				Log::err ("Failed to load UI resources; err=%i", result);
				ui->release ();
				break;
			}

			App::instance->suspendUpdate ();
			SDL_LockMutex (uiMutex);
			while (! uiList.empty ()) {
				item = uiList.back ();
				item->pause ();
				item->unload ();
				item->release ();
				uiList.pop_back ();
			}
			uiList.push_back (ui);
			SDL_UnlockMutex (uiMutex);
			App::instance->unsuspendUpdate ();
			break;
		}
		case UiStack::PushUiCommand: {
			if (! ui) {
				break;
			}
			result = ui->load ();
			if (result != OsUtil::Success) {
				Log::err ("Failed to load UI resources; err=%i", result);
				ui->release ();
				break;
			}

			App::instance->suspendUpdate ();
			SDL_LockMutex (uiMutex);
			uiList.push_back (ui);
			SDL_UnlockMutex (uiMutex);
			App::instance->unsuspendUpdate ();
			break;
		}
		case UiStack::PopUiCommand: {
			if (ui) {
				ui->release ();
			}
			ui = NULL;

			App::instance->suspendUpdate ();
			SDL_LockMutex (uiMutex);
			if (! uiList.empty ()) {
				ui = uiList.back ();
				uiList.pop_back ();

				if (activeUi == ui) {
					activeUi->release ();
					activeUi = NULL;
				}
			}
			SDL_UnlockMutex (uiMutex);

			if (ui) {
				ui->pause ();
				ui->unload ();
				ui->release ();
			}
			App::instance->unsuspendUpdate ();
			break;
		}
		default: {
			if (ui) {
				ui->release ();
			}
			break;
		}
	}
}

void UiStack::refresh () {
	std::list<Ui *>::iterator i, end;

	SDL_LockMutex (uiMutex);
	i = uiList.begin ();
	end = uiList.end ();
	while (i != end) {
		(*i)->refresh ();
		++i;
	}
	SDL_UnlockMutex (uiMutex);

	mainToolbar->refresh ();
	secondaryToolbar->refresh ();
	snackbarWindow->refresh ();
	topBarHeight = mainToolbar->position.y + mainToolbar->height;
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;
	secondaryToolbar->position.assign (0.0f, App::instance->windowHeight - secondaryToolbar->height);
	snackbarWindow->position.assign (App::instance->windowWidth - rightBarWidth - snackbarWindow->width, topBarHeight);
}

void UiStack::resize () {
	std::list<Ui *>::iterator i, end;

	mainToolbar->setWidth (App::instance->windowWidth);
	secondaryToolbar->setWidth (App::instance->windowWidth);
	mainToolbar->refresh ();
	secondaryToolbar->refresh ();

	SDL_LockMutex (uiMutex);
	i = uiList.begin ();
	end = uiList.end ();
	while (i != end) {
		(*i)->resize ();
		++i;
	}
	SDL_UnlockMutex (uiMutex);

	if (settingsWindow.widget) {
		((SettingsWindow *) settingsWindow.widget)->setWindowSize (App::instance->rootPanel->width * 0.33f, App::instance->rootPanel->height);
		settingsWindow.widget->position.assign (App::instance->windowWidth - settingsWindow.widget->width, 0.0f);
		if (darkenPanel.widget) {
			((Panel *) darkenPanel.widget)->setFixedSize (true, App::instance->rootPanel->width - settingsWindow.widget->width, App::instance->rootPanel->height);
		}
	}

	mainToolbar->refresh ();
	secondaryToolbar->refresh ();
	secondaryToolbar->position.assign (0.0f, App::instance->windowHeight - secondaryToolbar->height);
}

void UiStack::showSnackbar (const StdString &messageText, const StdString &actionButtonText, Widget::EventCallbackContext actionButtonClickCallback) {
	float y;

	snackbarWindow->setMaxWidth (App::instance->windowWidth - (UiConfiguration::instance->paddingSize * 2.0f) - rightBarWidth);
	snackbarWindow->setMessageText (messageText);
	if ((! actionButtonText.empty ()) && actionButtonClickCallback.callback) {
		snackbarWindow->setActionButtonEnabled (true, actionButtonText, actionButtonClickCallback);
	}
	else {
		snackbarWindow->setActionButtonEnabled (false);
	}

	if (historyWindow.widget || settingsWindow.widget || helpWindow.widget) {
		y = 0.0f;
	}
	else {
		y = topBarHeight;
	}
	snackbarWindow->position.assign (App::instance->windowWidth - rightBarWidth - snackbarWindow->width, y);
	snackbarWindow->startTimeout (UiConfiguration::instance->snackbarTimeout);
	snackbarWindow->startScroll (UiConfiguration::instance->snackbarScrollDuration);
	if (snackbarWindow->zLevel < App::instance->rootPanel->maxWidgetZLevel) {
		snackbarWindow->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	}
	snackbarWindow->isVisible = true;
}

bool UiStack::processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (keycode == SDLK_ESCAPE) {
		if (isDarkenOverlayActive ()) {
			clearOverlay ();
			return (true);
		}
	}
	return (false);
}

void UiStack::setKeyFocusTarget (Widget *widget) {
	Widget *target;

	target = keyFocusTarget.widget;
	if (target == widget) {
		return;
	}
	if (target && (! target->isDestroyed)) {
		target->setKeyFocus (false);
	}

	keyFocusTarget.assign (widget);
	target = keyFocusTarget.widget;
	if (target) {
		target->setKeyFocus (true);
	}
}

void UiStack::activateMouseHover () {
	Widget *widget;
	TooltipWindow *tooltipwindow;
	StdString text;
	float x, y, max;

	tooltip.destroyAndClear ();
	widget = mouseHoverTarget.widget;
	if (widget) {
		text.assign (widget->tooltipText);
	}
	if (widget && (! text.empty ())) {
		tooltipwindow = (TooltipWindow *) App::instance->rootPanel->addWidget (new TooltipWindow (text));
		tooltipwindow->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
		tooltip.assign (tooltipwindow);

		x = widget->screenX;
		y = widget->screenY;
		switch (widget->tooltipAlignment) {
			case Widget::TopAlignment: {
				x += ((widget->width / 2.0f) - (tooltipwindow->width / 2.0f));
				y -= (tooltipwindow->height + UiConfiguration::instance->marginSize);

				if (x < UiConfiguration::instance->paddingSize) {
					x = UiConfiguration::instance->paddingSize;
				}
				max = App::instance->rootPanel->width - tooltipwindow->width - UiConfiguration::instance->paddingSize;
				if (x > max) {
					x = max;
				}

				if (y < UiConfiguration::instance->paddingSize) {
					y = widget->screenY + widget->height + UiConfiguration::instance->marginSize;
				}
				break;
			}
			case Widget::LeftAlignment: {
				x -= (tooltipwindow->width + UiConfiguration::instance->marginSize);
				y += ((widget->height / 2.0f) - (tooltipwindow->height / 2.0f));

				if (y < UiConfiguration::instance->paddingSize) {
					y = UiConfiguration::instance->paddingSize;
				}
				max = App::instance->rootPanel->height - tooltipwindow->height - UiConfiguration::instance->paddingSize;
				if (y > max) {
					y = max;
				}

				if (x < UiConfiguration::instance->paddingSize) {
					x = widget->screenX + widget->width + UiConfiguration::instance->marginSize;
				}
				break;
			}
			case Widget::RightAlignment: {
				x += (widget->width + UiConfiguration::instance->marginSize);
				y += ((widget->height / 2.0f) - (tooltipwindow->height / 2.0f));

				if (y < UiConfiguration::instance->paddingSize) {
					y = UiConfiguration::instance->paddingSize;
				}
				max = App::instance->rootPanel->height - tooltipwindow->height - UiConfiguration::instance->paddingSize;
				if (y > max) {
					y = max;
				}

				if ((x + tooltipwindow->width) >= (App::instance->rootPanel->width - UiConfiguration::instance->paddingSize)) {
					x = widget->screenX - (tooltipwindow->width + UiConfiguration::instance->marginSize);
				}
				break;
			}
			case Widget::BottomAlignment:
			default: {
				x += ((widget->width / 2.0f) - (tooltipwindow->width / 2.0f));
				y += (widget->height + UiConfiguration::instance->marginSize);

				if (x < UiConfiguration::instance->paddingSize) {
					x = UiConfiguration::instance->paddingSize;
				}
				max = App::instance->rootPanel->width - tooltipwindow->width - UiConfiguration::instance->paddingSize;
				if (x > max) {
					x = max;
				}

				if ((y + tooltipwindow->height) >= (App::instance->rootPanel->height - UiConfiguration::instance->paddingSize)) {
					y = widget->screenY - (tooltipwindow->height + UiConfiguration::instance->marginSize);
				}
				break;
			}
		}

		tooltipwindow->position.assign (x, y);
	}

	isMouseHoverSuspended = false;
	isMouseHoverActive = true;
}

void UiStack::deactivateMouseHover () {
	tooltip.destroyAndClear ();
	mouseHoverClock = UiConfiguration::instance->mouseHoverThreshold;
	isMouseHoverSuspended = false;
	isMouseHoverActive = false;
}

void UiStack::suspendUiInput () {
	Ui *ui;

	if (isUiInputSuspended) {
		return;
	}
	isUiInputSuspended = true;
	ui = getActiveUi ();
	if (ui) {
		ui->rootPanel->isInputSuspended = true;
		ui->release ();
	}
	mainToolbar->isInputSuspended = true;
	secondaryToolbar->isInputSuspended = true;
}

void UiStack::suspendMouseHover () {
	tooltip.destroyAndClear ();
	mouseHoverClock = UiConfiguration::instance->mouseHoverThreshold;
	isMouseHoverActive = false;
	isMouseHoverSuspended = true;
}

bool UiStack::executeMainMenuAction (const char *actionName) {
	StdString name;
	bool active;

	active = false;
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		active = true;
	}
	SDL_UnlockMutex (uiMutex);
	if (! active) {
		return (false);
	}

	name.assign (actionName);
	name.lowercase ();
	if (name.equals (UiText::instance->getText (UiTextString::Log).lowercased ())) {
		UiStack::historyActionClicked (&(App::instance->uiStack), NULL);
		return (true);
	}
	if (name.equals (UiText::instance->getText (UiTextString::Settings).lowercased ())) {
		UiStack::settingsActionClicked (&(App::instance->uiStack), NULL);
		return (true);
	}
	if (name.equals (UiText::instance->getText (UiTextString::About).lowercased ())) {
		UiStack::aboutActionClicked (&(App::instance->uiStack), NULL);
		return (true);
	}
	if (name.equals (UiText::instance->getText (UiTextString::CheckForUpdates).lowercased ())) {
		UiStack::updateActionClicked (&(App::instance->uiStack), NULL);
		return (true);
	}
	if (name.equals (UiText::instance->getText (UiTextString::SendFeedback).lowercased ())) {
		UiStack::feedbackActionClicked (&(App::instance->uiStack), NULL);
		return (true);
	}
	if (name.equals (UiText::instance->getText (UiTextString::Help).lowercased ())) {
		UiStack::helpActionClicked (&(App::instance->uiStack), NULL);
		return (true);
	}
	if (name.equals (UiText::instance->getText (UiTextString::Exit).lowercased ())) {
		UiStack::exitActionClicked (&(App::instance->uiStack), NULL);
		return (true);
	}

	return (false);
}

void UiStack::getWidgetNames (StringList *destList) {
	destList->clear ();
	App::instance->rootPanel->getWidgetNames (destList);
}

bool UiStack::openWidget (const StdString &targetName) {
	Ui *ui;
	bool result;

	result = false;
	ui = App::instance->uiStack.getActiveUi ();
	if (ui) {
		result = ui->openWidget (targetName);
		ui->release ();
	}
	return (result);
}

bool UiStack::selectWidget (const StdString &targetName) {
	Ui *ui;
	bool result;

	result = false;
	ui = App::instance->uiStack.getActiveUi ();
	if (ui) {
		result = ui->selectWidget (targetName);
		ui->release ();
	}
	return (result);
}

bool UiStack::unselectWidget (const StdString &targetName) {
	Ui *ui;
	bool result;

	result = false;
	ui = App::instance->uiStack.getActiveUi ();
	if (ui) {
		result = ui->unselectWidget (targetName);
		ui->release ();
	}
	return (result);
}

bool UiStack::clickWidget (const StdString &targetName) {
	Widget *target;

	target = App::instance->rootPanel->findWidget (targetName);
	if (! target) {
		return (false);
	}
	target->mouseClick ();
	return (true);
}

bool UiStack::setTextFieldValue (const StdString &targetName, const StdString &textValue) {
	Widget *target;
	TextField *textfield;
	TextFieldWindow *textfieldwindow;

	target = App::instance->rootPanel->findWidget (targetName);
	if (! target) {
		return (false);
	}
	textfield = TextField::castWidget (target);
	if (textfield) {
		textfield->setValue (textValue);
		return (true);
	}
	textfieldwindow = TextFieldWindow::castWidget (target);
	if (textfieldwindow) {
		textfieldwindow->setValue (textValue);
		return (true);
	}
	return (false);
}

void UiStack::mainMenuButtonClicked (void *uiStackPtr, Widget *widgetPtr) {
	UiStack *uistack;
	Menu *menu;

	uistack = (UiStack *) uiStackPtr;
	uistack->suspendMouseHover ();
	if (uistack->mainMenu.widget) {
		uistack->mainMenu.destroyAndClear ();
		return;
	}

	SDL_LockMutex (uistack->uiMutex);
	if (uistack->activeUi) {
		uistack->activeUi->clearPopupWidgets ();
	}
	SDL_UnlockMutex (uistack->uiMutex);

	menu = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	menu->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextString::Log).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::HistoryButtonSprite), Widget::EventCallbackContext (UiStack::historyActionClicked, uistack));
	menu->addItem (UiText::instance->getText (UiTextString::Settings).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SettingsGearButtonSprite), Widget::EventCallbackContext (UiStack::settingsActionClicked, uistack));
	menu->addItem (UiText::instance->getText (UiTextString::About).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::AboutButtonSprite), Widget::EventCallbackContext (UiStack::aboutActionClicked, uistack));
	menu->addItem (UiText::instance->getText (UiTextString::CheckForUpdates).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::UpdateButtonSprite), Widget::EventCallbackContext (UiStack::updateActionClicked, uistack));
	menu->addItem (UiText::instance->getText (UiTextString::SendFeedback).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::FeedbackButtonSprite), Widget::EventCallbackContext (UiStack::feedbackActionClicked, uistack));
	menu->addItem (UiText::instance->getText (UiTextString::Help).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::HelpButtonSprite), Widget::EventCallbackContext (UiStack::helpActionClicked, uistack));
	menu->addItem (UiText::instance->getText (UiTextString::Exit).capitalized (), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExitButtonSprite), Widget::EventCallbackContext (UiStack::exitActionClicked, uistack));
	menu->position.assign (widgetPtr->position.x + widgetPtr->width - menu->width, widgetPtr->position.y + widgetPtr->height);
	uistack->mainMenu.assign (menu);
}

void UiStack::backButtonClicked (void *uiStackPtr, Widget *widgetPtr) {
	App::instance->uiStack.popUi ();
}

void UiStack::historyActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	App::instance->uiStack.toggleHistoryWindow ();
}

void UiStack::settingsActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	App::instance->uiStack.toggleSettingsWindow ();
}

void UiStack::aboutActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	StdString url;
	int result;

	url.assign (App::getHelpUrl ("about-membrane-control"));
	result = OsUtil::openUrl (url);
	if (result != OsUtil::Success) {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::OpenAboutUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::LaunchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void UiStack::updateActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	int result;

	result = OsUtil::openUrl (App::getUpdateUrl ());
	if (result != OsUtil::Success) {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::OpenFeedbackUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::LaunchedWebBrowser).capitalized ().c_str (), App::ServerUrl.c_str ()));
	}
}

void UiStack::feedbackActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	int result;

	result = OsUtil::openUrl (App::getFeedbackUrl (true));
	if (result != OsUtil::Success) {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::OpenFeedbackUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::LaunchedWebBrowser).capitalized ().c_str (), App::getFeedbackUrl ().c_str ()));
	}
}

void UiStack::helpActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	App::instance->uiStack.toggleHelpWindow ();
}

void UiStack::exitActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	App::instance->shutdown ();
}

void UiStack::clearOverlay () {
	mainMenu.destroyAndClear ();
	darkenPanel.destroyAndClear ();
	historyWindow.destroyAndClear ();
	settingsWindow.destroyAndClear ();
	helpWindow.destroyAndClear ();
	dialogWindow.destroyAndClear ();
	consoleWindow.destroyAndClear ();
	App::instance->setConsoleWindow (NULL);
}

bool UiStack::isDarkenOverlayActive () {
	return (historyWindow.widget || settingsWindow.widget || helpWindow.widget || dialogWindow.widget || consoleWindow.widget);
}

void UiStack::toggleHistoryWindow () {
	HistoryWindow *history;
	Panel *panel;

	if (dialogWindow.widget) {
		return;
	}
	if (historyWindow.widget) {
		clearOverlay ();
		return;
	}

	clearOverlay ();
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
	}
	SDL_UnlockMutex (uiMutex);

	history = new HistoryWindow (App::instance->rootPanel->width * 0.33f, App::instance->rootPanel->height);
	history->executeClickCallback = Widget::EventCallbackContext (UiStack::historyExecuteClicked, this);
	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width - history->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (panel);
	App::instance->rootPanel->addWidget (history);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	history->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	history->position.assign (App::instance->rootPanel->width - history->width, 0.0f);
	darkenPanel.assign (panel);
	historyWindow.assign (history);
	suspendUiInput ();
}

void UiStack::historyExecuteClicked (void *uiStackPtr, Widget *widgetPtr) {
	UiStack *uistack;
	HistoryWindow *history;
	StringList idlist;
	Json *command;
	Ui *ui;
	OsUtil::Result result;

	uistack = (UiStack *) uiStackPtr;
	history = (HistoryWindow *) widgetPtr;
	ui = uistack->getActiveUi ();
	if (ui) {
		result = CommandHistory::instance->executeRepeatCommand (history->executeCommandId, &idlist, &command);
		if (result == OsUtil::Success) {
			ui->invokeCommand (history->executeCommandId, idlist, command);
		}
		else if (result == OsUtil::ProgramNotFoundError) {
			uistack->showSnackbar (UiText::instance->getText (UiTextString::PlaylistNotFoundError));
		}
		else {
			uistack->showSnackbar (UiText::instance->getText (UiTextString::InternalError));
		}
		ui->release ();
	}
}

void UiStack::toggleSettingsWindow () {
	SettingsWindow *settings;
	Panel *panel;

	if (dialogWindow.widget) {
		return;
	}
	if (settingsWindow.widget) {
		clearOverlay ();
		return;
	}

	clearOverlay ();
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
	}
	SDL_UnlockMutex (uiMutex);

	settings = new SettingsWindow (App::instance->rootPanel->width * 0.33f, App::instance->rootPanel->height);
	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width - settings->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (panel);
	App::instance->rootPanel->addWidget (settings);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	settings->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	settings->position.assign (App::instance->rootPanel->width - settings->width, 0.0f);
	darkenPanel.assign (panel);
	settingsWindow.assign (settings);
	suspendUiInput ();
}

void UiStack::toggleHelpWindow () {
	HelpWindow *helpwindow;
	Panel *panel;

	if (dialogWindow.widget) {
		return;
	}
	if (helpWindow.widget) {
		clearOverlay ();
		return;
	}

	clearOverlay ();
	helpwindow = new HelpWindow (App::instance->rootPanel->width * 0.33f, App::instance->rootPanel->height);
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
		activeUi->setHelpWindowContent (helpwindow);
	}
	SDL_UnlockMutex (uiMutex);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width - helpwindow->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (panel);
	App::instance->rootPanel->addWidget (helpwindow);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	helpwindow->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	helpwindow->position.assign (App::instance->rootPanel->width - helpwindow->width, 0.0f);
	darkenPanel.assign (panel);
	helpWindow.assign (helpwindow);
	suspendUiInput ();
}

void UiStack::toggleConsoleWindow () {
	Panel *panel;
	ConsoleWindow *consolewindow;

	if (dialogWindow.widget) {
		return;
	}
	if (consoleWindow.widget) {
		clearOverlay ();
		return;
	}

	clearOverlay ();
	consolewindow = new ConsoleWindow ();
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
	}
	SDL_UnlockMutex (uiMutex);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (panel);
	App::instance->rootPanel->addWidget (consolewindow);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	consolewindow->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	consolewindow->position.assign (App::instance->rootPanel->width - consolewindow->width, (App::instance->rootPanel->height - consolewindow->height) / 2.0f);
	darkenPanel.assign (panel);
	consoleWindow.assign (consolewindow);
	suspendUiInput ();
	App::instance->setConsoleWindow (consolewindow);
	consolewindow->assignKeyFocus ();
}

void UiStack::showDialog (Panel *dialog) {
	Panel *panel;

	clearOverlay ();
	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (panel);
	App::instance->rootPanel->addWidget (dialog, (((float) App::instance->windowWidth) - dialog->width) / 2.0f, (((float) App::instance->windowHeight) - dialog->height) / 2.0f);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	dialog->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	darkenPanel.assign (panel);
	dialogWindow.assign (dialog);
	suspendUiInput ();
}

void UiStack::showImageDialog (const StdString &imageUrl) {
	ImageWindow *image;

	if (imageUrl.empty ()) {
		return;
	}
	image = new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite)));
	image->loadCallback = Widget::EventCallbackContext (UiStack::imageDialogLoaded, this);
	image->mouseClickCallback = Widget::EventCallbackContext (UiStack::imageDialogClicked, this);
	image->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	image->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite), (float) App::instance->windowWidth * 0.15f, (float) App::instance->windowHeight * 0.15f);
	image->onLoadFit ((float) App::instance->windowWidth * 0.99f, (float) App::instance->windowHeight * 0.99f);
	image->setImageUrl (imageUrl);
	showDialog (image);
}

void UiStack::imageDialogClicked (void *uiStackPtr, Widget *widgetPtr) {
	UiStack *uistack;

	uistack = (UiStack *) uiStackPtr;
	uistack->dialogWindow.destroyAndClear ();
	uistack->darkenPanel.destroyAndClear ();
}

void UiStack::imageDialogLoaded (void *uiStackPtr, Widget *widgetPtr) {
	ImageWindow *image;
	UiStack *uistack;

	uistack = (UiStack *) uiStackPtr;
	image = (ImageWindow *) widgetPtr;
	if (! image->isLoaded ()) {
		uistack->dialogWindow.destroyAndClear ();
		uistack->darkenPanel.destroyAndClear ();
		uistack->showSnackbar (UiText::instance->getText (UiTextString::ImageDialogLoadError));
		return;
	}
	image->position.assign ((((float) App::instance->windowWidth) - image->width) / 2.0f, (((float) App::instance->windowHeight) - image->height) / 2.0f);
}
