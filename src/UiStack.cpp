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
#include "SDL2/SDL.h"
#include "App.h"
#include "Result.h"
#include "StdString.h"
#include "Log.h"
#include "OsUtil.h"
#include "Ui.h"
#include "Menu.h"
#include "LogoWindow.h"
#include "TooltipWindow.h"
#include "SettingsWindow.h"
#include "HelpWindow.h"
#include "ImageWindow.h"
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
		snackbarWindow = (SnackbarWindow *) App::instance->rootPanel->addWidget (new SnackbarWindow (App::instance->windowWidth - (App::instance->uiConfig.paddingSize * 2.0f)));
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

#if ENABLE_TEST_KEYS
	if (App::instance->isUiPaused) {
		return;
	}
#endif
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
			mainMenu.destroyAndClear ();
			keyFocusTarget.clear ();
			mouseHoverTarget.clear ();
			darkenPanel.destroyAndClear ();
			settingsWindow.destroyAndClear ();
			helpWindow.destroyAndClear ();
			dialogWindow.destroyAndClear ();
			resetToolbars ();
			snackbarWindow->isVisible = false;
			activeUi->resume ();
			App::instance->shouldSyncRecordStore = true;
			isUiInputSuspended = false;
		}
	}
	SDL_UnlockMutex (uiMutex);

	tooltip.compact ();
	mainMenu.compact ();
	keyFocusTarget.compact ();
	mouseHoverTarget.compact ();
	darkenPanel.compact ();
	settingsWindow.compact ();
	helpWindow.compact ();
	dialogWindow.compact ();

	keywidget = keyFocusTarget.widget;
	if (keywidget && (! keywidget->isKeyFocused)) {
		keyFocusTarget.clear ();
	}

	mousewidget = App::instance->rootPanel->findWidget ((float) App::instance->input.mouseX, (float) App::instance->input.mouseY, true);
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

	if (darkenPanel.widget && (! settingsWindow.widget) && (! helpWindow.widget) && (! dialogWindow.widget)) {
		darkenPanel.destroyAndClear ();
	}

	if (isUiInputSuspended) {
		if ((! darkenPanel.widget) && (! settingsWindow.widget) && (! helpWindow.widget) && (! dialogWindow.widget)) {
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

	button = new Button (App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::MainMenuButtonSprite));
	button->setInverseColor (true);
	button->setMouseClickCallback (UiStack::mainMenuButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::mainMenuTooltip));
	mainToolbar->addRightItem (button);
	if (activeUi) {
		activeUi->addMainToolbarItems (mainToolbar);
		activeUi->addSecondaryToolbarItems (secondaryToolbar);
	}

	if (uiList.size () > 1) {
		button = new Button (App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::BackButtonSprite));
		button->setInverseColor (true);
		button->setMouseClickCallback (UiStack::backButtonClicked, this);
		button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::uiBackTooltip));
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
			if (result != Result::Success) {
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
			if (result != Result::Success) {
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
	if (helpWindow.widget) {
		((HelpWindow *) helpWindow.widget)->setWindowSize (App::instance->rootPanel->width * 0.33f, App::instance->rootPanel->height);
		helpWindow.widget->position.assign (App::instance->windowWidth - helpWindow.widget->width, 0.0f);
		if (darkenPanel.widget) {
			((Panel *) darkenPanel.widget)->setFixedSize (true, App::instance->rootPanel->width - helpWindow.widget->width, App::instance->rootPanel->height);
		}
	}

	mainToolbar->refresh ();
	secondaryToolbar->refresh ();
	secondaryToolbar->position.assign (0.0f, App::instance->windowHeight - secondaryToolbar->height);
}

void UiStack::showSnackbar (const StdString &messageText, const StdString &actionButtonText, Widget::EventCallback actionButtonClickCallback, void *actionButtonClickCallbackData) {
	float y;

	snackbarWindow->setMaxWidth (App::instance->windowWidth - (App::instance->uiConfig.paddingSize * 2.0f) - rightBarWidth);
	snackbarWindow->setMessageText (messageText);
	if ((! actionButtonText.empty ()) && actionButtonClickCallback) {
		snackbarWindow->setActionButtonEnabled (true, actionButtonText, actionButtonClickCallback, actionButtonClickCallbackData);
	}
	else {
		snackbarWindow->setActionButtonEnabled (false);
	}

	if (settingsWindow.widget || helpWindow.widget) {
		y = 0.0f;
	}
	else {
		y = topBarHeight;
	}
	snackbarWindow->position.assign (App::instance->windowWidth - rightBarWidth - snackbarWindow->width, y);
	snackbarWindow->startTimeout (App::instance->uiConfig.snackbarTimeout);
	snackbarWindow->startScroll (App::instance->uiConfig.snackbarScrollDuration);
	if (snackbarWindow->zLevel < App::instance->rootPanel->maxWidgetZLevel) {
		snackbarWindow->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	}
	snackbarWindow->isVisible = true;
}

bool UiStack::processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (settingsWindow.widget || helpWindow.widget || dialogWindow.widget) {
		if (keycode == SDLK_ESCAPE) {
			settingsWindow.destroyAndClear ();
			helpWindow.destroyAndClear ();
			dialogWindow.destroyAndClear ();
			darkenPanel.destroyAndClear ();
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
	if (target) {
		target->setKeyFocus (false);
	}

	keyFocusTarget.assign (widget);
	target = keyFocusTarget.widget;
	if (target) {
		target->setKeyFocus (true);
	}
}

void UiStack::activateMouseHover () {
	UiConfiguration *uiconfig;
	Widget *widget;
	TooltipWindow *tooltipwindow;
	StdString text;
	float x, y, max;

	uiconfig = &(App::instance->uiConfig);
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
				y -= (tooltipwindow->height + uiconfig->marginSize);

				if (x < uiconfig->paddingSize) {
					x = uiconfig->paddingSize;
				}
				max = App::instance->rootPanel->width - tooltipwindow->width - uiconfig->paddingSize;
				if (x > max) {
					x = max;
				}

				if (y < uiconfig->paddingSize) {
					y = widget->screenY + widget->height + uiconfig->marginSize;
				}
				break;
			}
			case Widget::LeftAlignment: {
				x -= (tooltipwindow->width + uiconfig->marginSize);
				y += ((widget->height / 2.0f) - (tooltipwindow->height / 2.0f));

				if (y < uiconfig->paddingSize) {
					y = uiconfig->paddingSize;
				}
				max = App::instance->rootPanel->height - tooltipwindow->height - uiconfig->paddingSize;
				if (y > max) {
					y = max;
				}

				if (x < uiconfig->paddingSize) {
					x = widget->screenX + widget->width + uiconfig->marginSize;
				}
				break;
			}
			case Widget::RightAlignment: {
				x += (widget->width + uiconfig->marginSize);
				y += ((widget->height / 2.0f) - (tooltipwindow->height / 2.0f));

				if (y < uiconfig->paddingSize) {
					y = uiconfig->paddingSize;
				}
				max = App::instance->rootPanel->height - tooltipwindow->height - uiconfig->paddingSize;
				if (y > max) {
					y = max;
				}

				if ((x + tooltipwindow->width) >= (App::instance->rootPanel->width - uiconfig->paddingSize)) {
					x = widget->screenX - (tooltipwindow->width + uiconfig->marginSize);
				}
				break;
			}
			case Widget::BottomAlignment:
			default: {
				x += ((widget->width / 2.0f) - (tooltipwindow->width / 2.0f));
				y += (widget->height + uiconfig->marginSize);

				if (x < uiconfig->paddingSize) {
					x = uiconfig->paddingSize;
				}
				max = App::instance->rootPanel->width - tooltipwindow->width - uiconfig->paddingSize;
				if (x > max) {
					x = max;
				}

				if ((y + tooltipwindow->height) >= (App::instance->rootPanel->height - uiconfig->paddingSize)) {
					y = widget->screenY - (tooltipwindow->height + uiconfig->marginSize);
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
	mouseHoverClock = App::instance->uiConfig.mouseHoverThreshold;
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
	mouseHoverClock = App::instance->uiConfig.mouseHoverThreshold;
	isMouseHoverActive = false;
	isMouseHoverSuspended = true;
}

void UiStack::mainMenuButtonClicked (void *uiStackPtr, Widget *widgetPtr) {
	UiStack *uistack;
	UiConfiguration *uiconfig;
	UiText *uitext;
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

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	menu = (Menu *) App::instance->rootPanel->addWidget (new Menu ());
	menu->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->getText (UiTextString::settings).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SettingsButtonSprite), UiStack::settingsActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::about).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::AboutButtonSprite), UiStack::aboutActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::checkForUpdates).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::UpdateButtonSprite), UiStack::updateActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::sendFeedback).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::FeedbackButtonSprite), UiStack::feedbackActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::help).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::HelpButtonSprite), UiStack::helpActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::exit).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::ExitButtonSprite), UiStack::exitActionClicked, uistack);
	menu->position.assign (widgetPtr->position.x + widgetPtr->width - menu->width, widgetPtr->position.y + widgetPtr->height);
	uistack->mainMenu.assign (menu);
}

void UiStack::backButtonClicked (void *uiStackPtr, Widget *widgetPtr) {
	App::instance->uiStack.popUi ();
}

void UiStack::settingsActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	App::instance->uiStack.toggleSettingsWindow ();
}

void UiStack::aboutActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	StdString url;
	int result;

	url.assign (App::getHelpUrl ("about-membrane-control"));
	result = OsUtil::openUrl (url);
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openAboutUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void UiStack::updateActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	int result;

	result = OsUtil::openUrl (App::getUpdateUrl ());
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openFeedbackUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), App::serverUrl.c_str ()));
	}
}

void UiStack::feedbackActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	int result;

	result = OsUtil::openUrl (App::getFeedbackUrl (true));
	if (result != Result::Success) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openFeedbackUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), App::getFeedbackUrl ().c_str ()));
	}
}

void UiStack::helpActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	App::instance->uiStack.toggleHelpWindow ();
}

void UiStack::exitActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	App::instance->shutdown ();
}

void UiStack::toggleSettingsWindow () {
	UiConfiguration *uiconfig;
	SettingsWindow *settings;
	Panel *panel;

	if (dialogWindow.widget) {
		return;
	}

	darkenPanel.destroyAndClear ();
	if (settingsWindow.widget) {
		settingsWindow.destroyAndClear ();
		return;
	}
	helpWindow.destroyAndClear ();
	mainMenu.destroyAndClear ();

	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
	}
	SDL_UnlockMutex (uiMutex);

	uiconfig = &(App::instance->uiConfig);
	settings = new SettingsWindow (App::instance->rootPanel->width * 0.33f, App::instance->rootPanel->height);
	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha, uiconfig->backgroundCrossFadeDuration);
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
	UiConfiguration *uiconfig;
	HelpWindow *helpwindow;
	Panel *panel;

	if (dialogWindow.widget) {
		return;
	}

	darkenPanel.destroyAndClear ();
	if (helpWindow.widget) {
		helpWindow.destroyAndClear ();
		return;
	}
	settingsWindow.destroyAndClear ();
	mainMenu.destroyAndClear ();

	uiconfig = &(App::instance->uiConfig);
	helpwindow = new HelpWindow (App::instance->rootPanel->width * 0.33f, App::instance->rootPanel->height);
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
		activeUi->setHelpWindowContent (helpwindow);
	}
	SDL_UnlockMutex (uiMutex);

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha, uiconfig->backgroundCrossFadeDuration);
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

void UiStack::showDialog (Panel *dialog) {
	UiConfiguration *uiconfig;
	Panel *panel;

	uiconfig = &(App::instance->uiConfig);

	darkenPanel.destroyAndClear ();
	dialogWindow.destroyAndClear ();
	helpWindow.destroyAndClear ();
	settingsWindow.destroyAndClear ();
	mainMenu.destroyAndClear ();

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, uiconfig->overlayWindowAlpha, uiconfig->backgroundCrossFadeDuration);
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
	UiConfiguration *uiconfig;

	if (imageUrl.empty ()) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	image = new ImageWindow (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite)));
	image->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	image->setFillBg (true, uiconfig->darkBackgroundColor);
	image->setMouseClickCallback (UiStack::imageDialogClicked, this);
	image->setLoadCallback (UiStack::imageDialogLoaded, this);
	image->setLoadSprite (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));
	image->setLoadResize (true, ((float) App::instance->windowWidth) * 0.95f);
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

	image = (ImageWindow *) widgetPtr;
	image->position.assign ((((float) App::instance->windowWidth) - image->width) / 2.0f, (((float) App::instance->windowHeight) - image->height) / 2.0f);
}
