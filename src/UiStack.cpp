/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
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
, mouseHoverClock (0)
, isMouseHoverActive (false)
, isMouseHoverSuspended (false)
{
	uiMutex = SDL_CreateMutex ();
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

	SDL_LockMutex (uiMutex);
	while (! uiList.empty ()) {
		ui = uiList.back ();
		ui->pause ();
		ui->unload ();
		ui->release ();
		uiList.pop_back ();
	}
	while (! uiAddList.empty ()) {
		ui = uiAddList.back ();
		ui->release ();
		uiAddList.pop_back ();
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
	ui->retain ();
	SDL_LockMutex (uiMutex);
	uiAddList.push_back (ui);
	SDL_UnlockMutex (uiMutex);
	App::instance->addRenderTask (UiStack::doSetUi, this);
}

void UiStack::doSetUi (void *uiStackPtr) {
	UiStack *uistack;
	Ui *ui;
	int result;

	uistack = (UiStack *) uiStackPtr;
	SDL_LockMutex (uistack->uiMutex);
	if (! uistack->uiAddList.empty ()) {
		ui = uistack->uiAddList.front ();
		uistack->uiAddList.pop_front ();
		result = ui->load ();
		if (result != Result::SUCCESS) {
			Log::err ("Failed to load UI resources; err=%i", result);
			ui->release ();
		}
		else {
			while (! uistack->uiList.empty ()) {
				ui = uistack->uiList.back ();
				ui->pause ();
				ui->unload ();
				ui->release ();
				uistack->uiList.pop_back ();
			}
			uistack->uiList.push_back (ui);
		}
	}
	SDL_UnlockMutex (uistack->uiMutex);
}

void UiStack::pushUi (Ui *ui) {
	ui->retain ();
	SDL_LockMutex (uiMutex);
	uiAddList.push_back (ui);
	SDL_UnlockMutex (uiMutex);
	App::instance->addRenderTask (UiStack::doPushUi, this);
}

void UiStack::doPushUi (void *uiStackPtr) {
	UiStack *uistack;
	Ui *ui;
	int result;

	uistack = (UiStack *) uiStackPtr;
	SDL_LockMutex (uistack->uiMutex);
	if (! uistack->uiAddList.empty ()) {
		ui = uistack->uiAddList.front ();
		uistack->uiAddList.pop_front ();
		result = ui->load ();
		if (result != Result::SUCCESS) {
			Log::err ("Failed to load UI resources; err=%i", result);
			ui->release ();
		}
		else {
			uistack->uiList.push_back (ui);
		}
	}
	SDL_UnlockMutex (uistack->uiMutex);
}

void UiStack::popUi () {
	App::instance->addRenderTask (UiStack::doPopUi, this);
}

void UiStack::doPopUi (void *uiStackPtr) {
	UiStack *uistack;
	Ui *ui;

	uistack = (UiStack *) uiStackPtr;
	SDL_LockMutex (uistack->uiMutex);
	if (! uistack->uiList.empty ()) {
		ui = uistack->uiList.back ();
		uistack->uiList.pop_back ();

		if (uistack->activeUi == ui) {
			uistack->activeUi->release ();
			uistack->activeUi = NULL;
		}
		ui->pause ();
		ui->unload ();
		ui->release ();
	}
	SDL_UnlockMutex (uistack->uiMutex);
}

void UiStack::update (int msElapsed) {
	Ui *ui;
	Widget *mousewidget;

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
			mouseHoverWidget.clear ();
			darkenPanel.destroyAndClear ();
			settingsWindow.destroyAndClear ();
			helpWindow.destroyAndClear ();
			resetToolbars ();
			snackbarWindow->isVisible = false;
			activeUi->resume ();
			App::instance->shouldSyncRecordStore = true;
		}
	}
	SDL_UnlockMutex (uiMutex);

	tooltip.compact ();
	mainMenu.compact ();
	mouseHoverWidget.compact ();
	darkenPanel.compact ();
	settingsWindow.compact ();
	helpWindow.compact ();

	if (darkenPanel.widget && (! settingsWindow.widget) && (! helpWindow.widget)) {
		darkenPanel.destroyAndClear ();
	}

	mousewidget = App::instance->rootPanel->findMouseHoverWidget ((float) App::instance->input.mouseX, (float) App::instance->input.mouseY);
	if (! mousewidget) {
		mouseHoverWidget.clear ();
		deactivateMouseHover ();
	}
	else {
		if (! mouseHoverWidget.equals (mousewidget)) {
			mouseHoverWidget.assign (mousewidget);
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
}

void UiStack::resetToolbars () {
	Button *button;

	mainToolbar->clearLeftOverlay ();
	mainToolbar->clearRightItems ();
	secondaryToolbar->clearAll ();

	button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::MAIN_MENU_BUTTON));
	button->setInverseColor (true);
	button->setMouseClickCallback (UiStack::mainMenuButtonClicked, this);
	button->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::mainMenuTooltip));
	mainToolbar->addRightItem (button);
	if (activeUi) {
		activeUi->addMainToolbarItems (mainToolbar);
		activeUi->addSecondaryToolbarItems (secondaryToolbar);
	}

	if (uiList.size () > 1) {
		button = new Button (StdString (""), App::instance->uiConfig.coreSprites.getSprite (UiConfiguration::BACK_BUTTON));
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

void UiStack::activateMouseHover () {
	UiConfiguration *uiconfig;
	Widget *widget;
	TooltipWindow *tooltipwindow;
	StdString text;
	float x, y, max;

	uiconfig = &(App::instance->uiConfig);
	tooltip.destroyAndClear ();

	widget = mouseHoverWidget.widget;
	if (widget) {
		text.assign (widget->tooltipText);
	}
	if (widget && (! text.empty ())) {
		tooltipwindow = (TooltipWindow *) App::instance->rootPanel->addWidget (new TooltipWindow (text));
		tooltipwindow->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
		tooltip.assign (tooltipwindow);

		x = widget->drawX;
		y = widget->drawY;
		switch (widget->tooltipAlignment) {
			case Widget::TOP: {
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
					y = widget->drawY + widget->height + uiconfig->marginSize;
				}
				break;
			}
			case Widget::LEFT: {
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
					x = widget->drawX + widget->width + uiconfig->marginSize;
				}
				break;
			}
			case Widget::RIGHT: {
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
					x = widget->drawX - (tooltipwindow->width + uiconfig->marginSize);
				}
				break;
			}
			case Widget::BOTTOM:
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
					y = widget->drawY - (tooltipwindow->height + uiconfig->marginSize);
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
	menu->addItem (uitext->getText (UiTextString::settings).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SETTINGS_BUTTON), UiStack::settingsActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::about).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::ABOUT_BUTTON), UiStack::aboutActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::checkForUpdates).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::UPDATE_BUTTON), UiStack::updateActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::sendFeedback).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::FEEDBACK_BUTTON), UiStack::feedbackActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::help).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::HELP_BUTTON), UiStack::helpActionClicked, uistack);
	menu->addItem (uitext->getText (UiTextString::exit).capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::EXIT_BUTTON), UiStack::exitActionClicked, uistack);
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
	if (result != Result::SUCCESS) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openAboutUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}

void UiStack::updateActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	int result;

	result = OsUtil::openUrl (App::getUpdateUrl ());
	if (result != Result::SUCCESS) {
		App::instance->uiStack.showSnackbar (App::instance->uiText.getText (UiTextString::openFeedbackUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", App::instance->uiText.getText (UiTextString::launchedWebBrowser).capitalized ().c_str (), App::serverUrl.c_str ()));
	}
}

void UiStack::feedbackActionClicked (void *uiStackPtr, Widget *widgetPtr) {
	int result;

	result = OsUtil::openUrl (App::getFeedbackUrl (true));
	if (result != Result::SUCCESS) {
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
	panel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	panel->translateAlphaBlend (0.0f, uiconfig->overlayWindowAlpha, uiconfig->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width - settings->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (panel);
	App::instance->rootPanel->addWidget (settings);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	settings->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	settings->position.assign (App::instance->rootPanel->width - settings->width, 0.0f);
	darkenPanel.assign (panel);
	settingsWindow.assign (settings);
}

void UiStack::toggleHelpWindow () {
	UiConfiguration *uiconfig;
	HelpWindow *helpwindow;
	Panel *panel;

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
	panel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	panel->translateAlphaBlend (0.0f, uiconfig->overlayWindowAlpha, uiconfig->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width - helpwindow->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (panel);
	App::instance->rootPanel->addWidget (helpwindow);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	helpwindow->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	helpwindow->position.assign (App::instance->rootPanel->width - helpwindow->width, 0.0f);
	darkenPanel.assign (panel);
	helpWindow.assign (helpwindow);
}
