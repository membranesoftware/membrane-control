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
#include "App.h"
#include "Util.h"
#include "StdString.h"
#include "StringList.h"
#include "Json.h"
#include "Widget.h"
#include "Resource.h"
#include "RecordStore.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Button.h"
#include "Toolbar.h"
#include "LabelWindow.h"
#include "ProgressBar.h"
#include "TooltipWindow.h"
#include "Menu.h"
#include "Chip.h"
#include "SettingsWindow.h"
#include "HelpWindow.h"
#include "Ui.h"

Ui::Ui ()
: rootPanel (NULL)
, isLoaded (false)
, isFirstResumeComplete (false)
, mouseHoverClock (0)
, isMouseHoverActive (false)
, isMouseHoverSuspended (false)
, refcount (0)
, refcountMutex (NULL)
{
	refcountMutex = SDL_CreateMutex ();

	rootPanel = new Panel ();
	rootPanel->setParentUi (this);
	rootPanel->setKeyEventCallback (Ui::keyEvent, this);
	rootPanel->retain ();
}

Ui::~Ui () {
	if (rootPanel) {
		rootPanel->release ();
		rootPanel = NULL;
	}

	if (refcountMutex) {
		SDL_DestroyMutex (refcountMutex);
		refcountMutex = NULL;
	}
}

void Ui::retain () {

	SDL_LockMutex (refcountMutex);
	++refcount;
	if (refcount < 1) {
		refcount = 1;
	}
	SDL_UnlockMutex (refcountMutex);

}

void Ui::release () {
	bool isdestroyed;

	isdestroyed = false;
	SDL_LockMutex (refcountMutex);
	--refcount;
	if (refcount <= 0) {
		refcount = 0;
		isdestroyed = true;
	}
	SDL_UnlockMutex (refcountMutex);

	if (isdestroyed) {
		delete (this);
	}
}

int Ui::load () {
	StdString path;
	int result;

	if (isLoaded) {
		return (Result::SUCCESS);
	}

	path = getSpritePath ();
	if (! path.empty ()) {
		result = sprites.load (path);
		if (result != Result::SUCCESS) {
			Log::write (Log::ERR, "Failed to load sprite resources");
			return (result);
		}
	}

	result = doLoad ();
	if (result != Result::SUCCESS) {
		return (result);
	}

	isLoaded = true;
	return (Result::SUCCESS);
}

void Ui::unload () {
	if (! isLoaded) {
		return;
	}

	clearPopupWidgets ();
	breadcrumbChip.destroyAndClear ();
	mouseHoverWidget.clear ();

	rootPanel->clear ();
	doUnload ();

	sprites.unload ();
	isLoaded = false;
}

StdString Ui::getSpritePath () {
	// Default implementation returns an empty path
	return (StdString (""));
}

int Ui::doLoad () {
	// Default implementation does nothing
	return (Result::SUCCESS);
}

void Ui::doUnload () {
	// Default implementation does nothing
}

void Ui::handleLinkClientConnect (const StdString &agentId) {
	// Default implementation does nothing
}

void Ui::handleLinkClientDisconnect (const StdString &agentId, const StdString &errorDescription) {
	// Default implementation does nothing
}

void Ui::handleLinkClientCommand (const StdString &agentId, Json *command) {
	// Default implementation does nothing
}

StdString Ui::getBreadcrumbTitle () {
	// Default implementation returns an empty string
	return (StdString (""));
}

Sprite *Ui::getBreadcrumbSprite () {
	// Default implementation returns NULL
	return (NULL);
}

void Ui::backButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App::getInstance ()->popUi ();
}

void Ui::mainMenuButtonClicked (void *uiPtr, Widget *widgetPtr) {
	((Ui *) uiPtr)->handleMainMenuButtonClick (widgetPtr);
}

void Ui::handleMainMenuButtonClick (Widget *buttonWidget) {
	App *app;
	UiText *uitext;
	UiConfiguration *uiconfig;
	Menu *menu;

	app = App::getInstance ();
	uitext = &(app->uiText);
	uiconfig = &(app->uiConfig);

	suspendMouseHover ();
	if (mainMenu.widget) {
		mainMenu.destroyAndClear ();
		return;
	}

	clearPopupWidgets ();
	menu = (Menu *) app->rootPanel->addWidget (new Menu ());
	menu->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	menu->isClickDestroyEnabled = true;
	menu->addItem (uitext->settings.capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::SETTINGS_BUTTON), Ui::settingsActionClicked, this);
	menu->addItem (uitext->about.capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::ABOUT_BUTTON), Ui::aboutActionClicked, this);
	menu->addItem (uitext->checkForUpdates.capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::UPDATE_BUTTON), Ui::updateActionClicked, this);
	menu->addItem (uitext->sendFeedback.capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::FEEDBACK_BUTTON), Ui::feedbackActionClicked, this);
	menu->addItem (uitext->help.capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::HELP_BUTTON), Ui::helpActionClicked, this);
	menu->addItem (uitext->exit.capitalized (), uiconfig->coreSprites.getSprite (UiConfiguration::EXIT_BUTTON), Ui::exitActionClicked, this);
	menu->position.assign (buttonWidget->position.x + buttonWidget->width - menu->width, buttonWidget->position.y + buttonWidget->height);
	mainMenu.assign (menu);
}

void Ui::newsButtonClicked (void *uiPtr, Widget *widgetPtr) {
	App::getInstance ()->toggleNewsWindow ();
}

bool Ui::matchOpenTaskItem (void *ptr, Json *record) {
	SystemInterface *interface;
	int commandid;

	interface = &(App::getInstance ()->systemInterface);
	commandid = interface->getCommandId (record);
	if (commandid != SystemInterface::Command_TaskItem) {
		return (false);
	}

	if (interface->isRecordClosed (record)) {
		return (false);
	}

	return (true);
}

void Ui::helpActionClicked (void *uiPtr, Widget *widgetPtr) {
	Ui *ui;

	ui = (Ui *) uiPtr;
	ui->toggleHelpWindow ();
}

void Ui::toggleHelpWindow () {
	App *app;
	UiConfiguration *uiconfig;
	HelpWindow *helpwindow;
	Panel *panel;

	app = App::getInstance ();
	darkenPanel.destroyAndClear ();
	if (helpWindow.widget) {
		helpWindow.destroyAndClear ();
		return;
	}
	settingsWindow.destroyAndClear ();

	uiconfig = &(app->uiConfig);
	helpwindow = new HelpWindow (app->rootPanel->width * 0.33f, app->rootPanel->height);
	setHelpWindowContent (helpwindow);

	panel = new Panel ();
	panel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	panel->transitionAlphaBlend (0.0f, uiconfig->overlayWindowAlpha, uiconfig->backgroundTransitionDuration);
	panel->setFixedSize (true, app->rootPanel->width - helpwindow->width, app->rootPanel->height);

	clearPopupWidgets ();
	app->rootPanel->addWidget (panel);
	app->rootPanel->addWidget (helpwindow);
	panel->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	helpwindow->zLevel = app->rootPanel->maxWidgetZLevel + 2;
	helpwindow->position.assign (app->rootPanel->width - helpwindow->width, 0.0f);
	darkenPanel.assign (panel);
	helpWindow.assign (helpwindow);
}

void Ui::setHelpWindowContent (Widget *helpWindowPtr) {
// TODO: Add default help content
/*
	HelpWindow *help;

	help = (HelpWindow *) helpWindowPtr;
*/
}

void Ui::showShutdownWindow () {
	App *app;
	UiConfiguration *uiconfig;
	UiText *uitext;
	LabelWindow *label;
	ProgressBar *bar;
	Panel *panel;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	uitext = &(app->uiText);

	darkenPanel.destroyAndClear ();
	clearPopupWidgets ();

	panel = new Panel ();
	panel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	panel->transitionAlphaBlend (0.0f, uiconfig->overlayWindowAlpha, uiconfig->backgroundTransitionDuration);
	panel->setFixedSize (true, app->rootPanel->width, app->rootPanel->height);

	label = new LabelWindow (new Label (uitext->shuttingDownApp, UiConfiguration::CAPTION, uiconfig->primaryTextColor));
	label->setFillBg (true, uiconfig->lightBackgroundColor);

	bar = new ProgressBar (label->width, uiconfig->progressBarHeight);
	bar->setIndeterminate (true);

	app->rootPanel->addWidget (panel);
	app->rootPanel->addWidget (label);
	app->rootPanel->addWidget (bar);
	panel->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	label->zLevel = app->rootPanel->maxWidgetZLevel + 2;
	bar->zLevel = app->rootPanel->maxWidgetZLevel + 2;
	label->position.assign ((app->rootPanel->width / 2.0f) - (label->width / 2.0f), (app->rootPanel->height / 2.0f) - (label->height / 2.0f));
	bar->position.assign (label->position.x, label->position.y + label->height);
}

void Ui::exitActionClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;

	app = App::getInstance ();
	app->shutdown ();
}

void Ui::settingsActionClicked (void *uiPtr, Widget *widgetPtr) {
	Ui *ui;

	ui = (Ui *) uiPtr;
	ui->toggleSettingsWindow ();
}

void Ui::aboutActionClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	StdString url;
	int result;

	app = App::getInstance ();
	url.assign (Util::getHelpUrl ("about-membrane-control"));
	result = Util::openUrl (url);
	if (result != Result::SUCCESS) {
		app->showSnackbar (app->uiText.openAboutUrlError);
	}
	else {
		app->showSnackbar (StdString::createSprintf ("%s - %s", app->uiText.launchedWebBrowser.capitalized ().c_str (), url.c_str ()));
	}
}

void Ui::toggleSettingsWindow () {
	App *app;
	UiConfiguration *uiconfig;
	SettingsWindow *settings;
	Panel *panel;

	app = App::getInstance ();
	darkenPanel.destroyAndClear ();
	if (settingsWindow.widget) {
		settingsWindow.destroyAndClear ();
		return;
	}
	helpWindow.destroyAndClear ();

	uiconfig = &(app->uiConfig);
	settings = new SettingsWindow (app->rootPanel->width * 0.33f, app->rootPanel->height);
	panel = new Panel ();
	panel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	panel->transitionAlphaBlend (0.0f, uiconfig->overlayWindowAlpha, uiconfig->backgroundTransitionDuration);
	panel->setFixedSize (true, app->rootPanel->width - settings->width, app->rootPanel->height);

	clearPopupWidgets ();
	app->rootPanel->addWidget (panel);
	app->rootPanel->addWidget (settings);
	panel->zLevel = app->rootPanel->maxWidgetZLevel + 1;
	settings->zLevel = app->rootPanel->maxWidgetZLevel + 2;
	settings->position.assign (app->rootPanel->width - settings->width, 0.0f);
	darkenPanel.assign (panel);
	settingsWindow.assign (settings);
}

void Ui::updateActionClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	int result;

	app = App::getInstance ();
	result = Util::openUrl (Util::getUpdateUrl ());
	if (result != Result::SUCCESS) {
		app->showSnackbar (app->uiText.openFeedbackUrlError);
	}
	else {
		app->showSnackbar (StdString::createSprintf ("%s - %s", app->uiText.launchedWebBrowser.capitalized ().c_str (), Util::serverUrl.c_str ()));
	}
}

void Ui::feedbackActionClicked (void *uiPtr, Widget *widgetPtr) {
	App *app;
	int result;

	app = App::getInstance ();
	result = Util::openUrl (Util::getFeedbackUrl (true));
	if (result != Result::SUCCESS) {
		app->showSnackbar (app->uiText.openFeedbackUrlError);
	}
	else {
		app->showSnackbar (StdString::createSprintf ("%s - %s", app->uiText.launchedWebBrowser.capitalized ().c_str (), Util::getFeedbackUrl ().c_str ()));
	}
}

bool Ui::keyEvent (void *uiPtr, SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	Ui *ui;

	ui = (Ui *) uiPtr;
	if (isControlDown) {
		switch (keycode) {
			case SDLK_s: {
				ui->toggleSettingsWindow ();
				return (true);
			}
			case SDLK_h: {
				ui->toggleHelpWindow ();
				return (true);
			}
			case SDLK_q: {
				App::getInstance ()->shutdown ();
				return (true);
			}
		}
	}

	return (false);
}

void Ui::resetMainToolbar (Toolbar *toolbar) {
	Button *button;
	UiText *uitext;
	UiConfiguration *uiconfig;

	uitext = &(App::getInstance ()->uiText);
	uiconfig = &(App::getInstance ()->uiConfig);

	toolbar->clearLeftOverlay ();
	toolbar->clearRightItems ();

	button = new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::MAIN_MENU_BUTTON));
	button->setMouseClickCallback (Ui::mainMenuButtonClicked, this);
	button->setMouseHoverTooltip (uitext->mainMenuTooltip);
	toolbar->addRightItem (button);

	button = new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::NEWS_BUTTON));
	button->setMouseClickCallback (Ui::newsButtonClicked, this);
	button->setMouseHoverTooltip (uitext->newsButtonTooltip);
	toolbar->addRightItem (button);

	doResetMainToolbar (toolbar);
}

void Ui::doResetMainToolbar (Toolbar *toolbar) {
	// Default implementation does nothing
}

void Ui::resetSecondaryToolbar (Toolbar *toolbar) {
	toolbar->clearAll ();
	doResetSecondaryToolbar (toolbar);
}

void Ui::doResetSecondaryToolbar (Toolbar *toolbar) {
	// Default implementation hides the toolbar
	toolbar->isVisible = false;
}

void Ui::resume () {
	App *app;
	StdString title;
	Sprite *sprite;

	if (! isLoaded) {
		return;
	}

	app = App::getInstance ();
	if (rootPanel->id <= 0) {
		rootPanel->id = app->getUniqueId ();
	}
	rootPanel->position.assign (0.0f, 0.0f);
	rootPanel->setFixedSize (true, app->windowWidth, app->windowHeight);
	rootPanel->resetInputState ();
	app->rootPanel->addWidget (rootPanel);

	if ((! isFirstResumeComplete) && (! breadcrumbChip.widget)) {
		title = getBreadcrumbTitle ();
		sprite = getBreadcrumbSprite ();
		if ((! title.empty ()) || sprite) {
			breadcrumbChip.assign (new Chip (title, sprite, true));
			app->mainToolbar->addLeftItem (breadcrumbChip.widget);
		}
	}

	doResume ();
	isFirstResumeComplete = true;
}

void Ui::pause () {
	App *app;

	if (! isLoaded) {
		return;
	}

	app = App::getInstance ();
	app->rootPanel->removeWidget (rootPanel);
	app->mainToolbar->clearLeftOverlay ();
	app->mainToolbar->clearRightItems ();
	app->secondaryToolbar->clearAll ();

	clearPopupWidgets ();
	mouseHoverWidget.clear ();

	doPause ();
}

void Ui::update (int msElapsed) {
	App *app;
	Widget *mousewidget;

	app = App::getInstance ();
#if ENABLE_TEST_KEYS
	if (app->isUiPaused) {
		return;
	}
#endif
	breadcrumbChip.compact ();
	tooltip.compact ();
	darkenPanel.compact ();
	mainMenu.compact ();
	settingsWindow.compact ();
	helpWindow.compact ();

	if (darkenPanel.widget && (! settingsWindow.widget) && (! helpWindow.widget)) {
		darkenPanel.destroyAndClear ();
	}

	mousewidget = app->rootPanel->findMouseHoverWidget ((float) app->input.mouseX, (float) app->input.mouseY);
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

	doUpdate (msElapsed);
}

void Ui::draw () {
	// Base class method takes no action
	doDraw ();
}

void Ui::refresh () {
	rootPanel->refresh ();
	doRefresh ();
}

void Ui::doRefresh () {
	// Default implementation does nothing
}

void Ui::resize () {
	App *app;
	Sprite *sprite;

	app = App::getInstance ();
	rootPanel->position.assign (0.0f, 0.0f);
	rootPanel->setFixedSize (true, app->windowWidth, app->windowHeight);

	sprites.resize ();

	if (settingsWindow.widget) {
		((SettingsWindow *) settingsWindow.widget)->setWindowSize (app->rootPanel->width * 0.33f, app->rootPanel->height);
		settingsWindow.widget->position.assign (app->windowWidth - settingsWindow.widget->width, 0.0f);
		if (darkenPanel.widget) {
			((Panel *) darkenPanel.widget)->setFixedSize (true, app->rootPanel->width - settingsWindow.widget->width, app->rootPanel->height);
		}
	}

	if (helpWindow.widget) {
		((HelpWindow *) helpWindow.widget)->setWindowSize (app->rootPanel->width * 0.33f, app->rootPanel->height);
		helpWindow.widget->position.assign (app->windowWidth - helpWindow.widget->width, 0.0f);
		if (darkenPanel.widget) {
			((Panel *) darkenPanel.widget)->setFixedSize (true, app->rootPanel->width - helpWindow.widget->width, app->rootPanel->height);
		}
	}

	if (breadcrumbChip.widget) {
		sprite = getBreadcrumbSprite ();
		if (sprite) {
			((Chip *) breadcrumbChip.widget)->setIconSprite (sprite, true);
		}
	}

	doResize ();
	app->secondaryToolbar->position.assign (0.0f, app->windowHeight - app->secondaryToolbar->height);
	rootPanel->resetInputState ();
}

void Ui::doResize () {
	// Default implementation does nothing
}

void Ui::syncRecordStore (RecordStore *store) {
	// Superclass method takes no action

	doSyncRecordStore (store);
}

void Ui::addMainToolbarBackButton (Toolbar *toolbar) {
	App *app;
	Button *button;

	app = App::getInstance ();

	button = new Button (StdString (""), App::getInstance ()->uiConfig.coreSprites.getSprite (UiConfiguration::BACK_BUTTON));
	button->setMouseClickCallback (Ui::backButtonClicked, this);
	button->setMouseHoverTooltip (app->uiText.uiBackTooltip);
	toolbar->addRightItem (button);
}

void Ui::doClearPopupWidgets () {
	// Default implementation does nothing
}

void Ui::doResume () {
	// Default implementation does nothing
}

void Ui::doPause () {
	// Default implementation does nothing
}

void Ui::doUpdate (int msElapsed) {
	// Default implementation does nothing
}

void Ui::doDraw () {
	// Default implementation does nothing
}

void Ui::doSyncRecordStore (RecordStore *store) {
	// Default implementation does nothing
}

void Ui::clearPopupWidgets () {
	tooltip.destroyAndClear ();
	mainMenu.destroyAndClear ();

	doClearPopupWidgets ();
}

void Ui::activateMouseHover () {
	App *app;
	UiConfiguration *uiconfig;
	Widget *widget;
	TooltipWindow *tooltipwindow;
	StdString text;
	float x, y, max;

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	tooltip.destroyAndClear ();

	widget = mouseHoverWidget.widget;
	if (widget) {
		text.assign (widget->tooltipText);
	}
	if (widget && (! text.empty ())) {
		tooltipwindow = (TooltipWindow *) app->rootPanel->addWidget (new TooltipWindow (text));
		tooltipwindow->zLevel = app->rootPanel->maxWidgetZLevel + 1;
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
				max = app->rootPanel->width - tooltipwindow->width - uiconfig->paddingSize;
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
				max = app->rootPanel->height - tooltipwindow->height - uiconfig->paddingSize;
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
				max = app->rootPanel->height - tooltipwindow->height - uiconfig->paddingSize;
				if (y > max) {
					y = max;
				}

				if ((x + tooltipwindow->width) >= (app->rootPanel->width - uiconfig->paddingSize)) {
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
				max = app->rootPanel->width - tooltipwindow->width - uiconfig->paddingSize;
				if (x > max) {
					x = max;
				}

				if ((y + tooltipwindow->height) >= (app->rootPanel->height - uiconfig->paddingSize)) {
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

void Ui::deactivateMouseHover () {
	tooltip.destroyAndClear ();
	mouseHoverClock = App::getInstance ()->uiConfig.mouseHoverThreshold;
	isMouseHoverSuspended = false;
	isMouseHoverActive = false;
}

void Ui::suspendMouseHover () {
	tooltip.destroyAndClear ();
	mouseHoverClock = App::getInstance ()->uiConfig.mouseHoverThreshold;
	isMouseHoverActive = false;
	isMouseHoverSuspended = true;
}

Widget *Ui::addWidget (Widget *widget, float positionX, float positionY, int zLevel) {
	return (rootPanel->addWidget (widget, positionX, positionY, zLevel));
}

Widget *Ui::findWidget (uint64_t widgetId, const StdString &widgetTypeName) {
	return (rootPanel->findChildWidget (widgetId, widgetTypeName, true));
}
