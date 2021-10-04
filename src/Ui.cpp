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
#include "Log.h"
#include "App.h"
#include "Input.h"
#include "OsUtil.h"
#include "StdString.h"
#include "StringList.h"
#include "Json.h"
#include "Widget.h"
#include "Resource.h"
#include "UiConfiguration.h"
#include "AgentControl.h"
#include "Panel.h"
#include "Button.h"
#include "Toolbar.h"
#include "LabelWindow.h"
#include "ProgressBar.h"
#include "HelpWindow.h"
#include "IconLabelWindow.h"
#include "CommandListener.h"
#include "Ui.h"

Ui::Ui ()
: rootPanel (NULL)
, isLoaded (false)
, isFirstResumeComplete (false)
, isLinkConnected (false)
, cardView (NULL)
, actionSource (NULL)
, refcount (0)
, refcountMutex (NULL)
, lastWindowCloseCount (0)
{
	refcountMutex = SDL_CreateMutex ();

	rootPanel = new Panel ();
	rootPanel->keyEventCallback = Widget::KeyEventCallbackContext (Ui::keyEvent, this);
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
		return (OsUtil::Result::Success);
	}
	path = getSpritePath ();
	if (! path.empty ()) {
		result = sprites.load (path);
		if (result != OsUtil::Result::Success) {
			Log::err ("Failed to load sprite resources");
			return (result);
		}
	}

	cardView = (CardView *) addWidget (new CardView (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->isMouseWheelScrollEnabled = true;
	cardView->isExitedMouseWheelScrollEnabled = true;

	result = doLoad ();
	if (result != OsUtil::Result::Success) {
		return (result);
	}

	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	isLoaded = true;
	return (OsUtil::Result::Success);
}

void Ui::unload () {
	if (! isLoaded) {
		return;
	}
	clearPopupWidgets ();
	actionWidget.destroyAndClear ();
	actionTarget.clear ();
	breadcrumbWidget.destroyAndClear ();

	rootPanel->clear ();
	doUnload ();

	sprites.unload ();
	isLoaded = false;
}

StdString Ui::getSpritePath () {
	return (StdString (""));
}

Widget *Ui::createBreadcrumbWidget () {
	return (NULL);
}

int Ui::doLoad () {
	// Default implementation does nothing
	return (OsUtil::Result::Success);
}

void Ui::doUnload () {
	// Default implementation does nothing
}

void Ui::showShutdownWindow () {
	LabelWindow *label;
	ProgressBar *bar;
	Panel *panel;

	clearPopupWidgets ();

	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width, App::instance->rootPanel->height);

	label = new LabelWindow (new Label (StdString::createSprintf ("%s %s", UiText::instance->getText (UiTextString::ShuttingDown).capitalized ().c_str (), APPLICATION_NAME), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	label->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);

	bar = new ProgressBar (label->width, UiConfiguration::instance->progressBarHeight);
	bar->setIndeterminate (true);

	App::instance->rootPanel->addWidget (panel);
	App::instance->rootPanel->addWidget (label);
	App::instance->rootPanel->addWidget (bar);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	label->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	bar->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	label->position.assign ((App::instance->rootPanel->width / 2.0f) - (label->width / 2.0f), (App::instance->rootPanel->height / 2.0f) - (label->height / 2.0f));
	bar->position.assign (label->position.x, label->position.y + label->height);
}

bool Ui::keyEvent (void *uiPtr, SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	return (((Ui *) uiPtr)->processKeyEvent (keycode, isShiftDown, isControlDown));
}

bool Ui::processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	// Base class method takes no action

	return (doProcessKeyEvent (keycode, isShiftDown, isControlDown));
}

bool Ui::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	// Default implementation does nothing
	return (false);
}

bool Ui::doProcessWindowCloseEvent () {
	// Default implementation does nothing
	return (false);
}

void Ui::resume () {
	StdString title;

	if (! isLoaded) {
		return;
	}
	if (rootPanel->id <= 0) {
		rootPanel->id = App::instance->getUniqueId ();
	}
	rootPanel->position.assign (0.0f, 0.0f);
	rootPanel->setFixedSize (true, App::instance->windowWidth, App::instance->windowHeight);
	rootPanel->resetInputState ();
	rootPanel->isInputSuspended = false;
	App::instance->rootPanel->addWidget (rootPanel);
	lastWindowCloseCount = Input::instance->windowCloseCount;

	setLinkConnected (true);
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	doResume ();
	isFirstResumeComplete = true;
}

void Ui::pause () {
	if (! isLoaded) {
		return;
	}
	App::instance->rootPanel->removeWidget (rootPanel);
	lastWindowCloseCount = Input::instance->windowCloseCount;
	clearPopupWidgets ();
	CommandListener::instance->unsubscribeContext (this);
	setLinkConnected (false);
	doPause ();
}

void Ui::update (int msElapsed) {
	int count;

	if (actionWidget.widget) {
		actionWidget.compact ();
		if (! actionWidget.widget) {
			actionTarget.clear ();
		}
	}
	actionTarget.compact ();
	breadcrumbWidget.compact ();

	count = Input::instance->windowCloseCount;
	if (lastWindowCloseCount != count) {
		lastWindowCloseCount = count;
		if (! doProcessWindowCloseEvent ()) {
			App::instance->shutdown ();
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
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	doRefresh ();
}

void Ui::doRefresh () {
	// Default implementation does nothing
}

void Ui::resize () {
	rootPanel->position.assign (0.0f, 0.0f);
	rootPanel->setFixedSize (true, App::instance->windowWidth, App::instance->windowHeight);
	sprites.resize ();
	cardView->setViewSize (App::instance->windowWidth - App::instance->uiStack.rightBarWidth, App::instance->windowHeight - App::instance->uiStack.topBarHeight - App::instance->uiStack.bottomBarHeight);
	cardView->position.assign (0.0f, App::instance->uiStack.topBarHeight);
	doResize ();
	rootPanel->resetInputState ();
}

void Ui::doResize () {
	// Default implementation does nothing
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

void Ui::clearPopupWidgets () {
	actionWidget.destroyAndClear ();
	actionTarget.clear ();
	actionSource = NULL;
	doClearPopupWidgets ();
}

Widget *Ui::addWidget (Widget *widget, float positionX, float positionY, int zLevel) {
	return (rootPanel->addWidget (widget, positionX, positionY, zLevel));
}

void Ui::addMainToolbarItems (Toolbar *toolbar) {
	Widget *widget;

	if (! breadcrumbWidget.widget) {
		widget = createBreadcrumbWidget ();
		if (widget) {
			breadcrumbWidget.assign (widget);
			toolbar->addLeftItem (breadcrumbWidget.widget);
		}
	}

	doAddMainToolbarItems (toolbar);
}

void Ui::addSecondaryToolbarItems (Toolbar *toolbar) {
	// Base class method takes no action
	doAddSecondaryToolbarItems (toolbar);
}

void Ui::doAddMainToolbarItems (Toolbar *toolbar) {
	// Default implementation does nothing
}

void Ui::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	// Default implementation does nothing
}

void Ui::setHelpWindowContent (HelpWindow *helpWindow) {
	// Default implementation does nothing
}

void Ui::handleLinkClientConnect (const StdString &agentId) {
	// Default implementation does nothing
}

void Ui::handleLinkClientDisconnect (const StdString &agentId, const StdString &errorDescription) {
	// Default implementation does nothing
}

void Ui::syncRecordStore () {
	// Superclass method takes no action
	doSyncRecordStore ();
}

void Ui::doSyncRecordStore () {
	// Default implementation does nothing
}

void Ui::setLinkConnected (bool connected) {
	StringList::iterator i, end;

	if (connected == isLinkConnected) {
		return;
	}

	isLinkConnected = connected;
	i = linkAgentIds.begin ();
	end = linkAgentIds.end ();
	while (i != end) {
		if (isLinkConnected) {
			AgentControl::instance->connectLinkClient (*i);
		}
		else {
			AgentControl::instance->disconnectLinkClient (*i);
		}
		++i;
	}
}

void Ui::addLinkAgent (const StdString &agentId) {
	if (linkAgentIds.contains (agentId)) {
		return;
	}

	linkAgentIds.push_back (agentId);
	if (isLinkConnected) {
		AgentControl::instance->connectLinkClient (agentId);
	}
}

void Ui::showActionPopup (Widget *action, Widget *target, Widget::EventCallback sourceFn, const Widget::Rectangle &sourceRect, int xAlignment, int yAlignment) {
	actionWidget.assign (action);
	actionTarget.assign (target);
	actionSource = sourceFn;

	App::instance->rootPanel->addWidget (action);
	action->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	assignPopupPosition (action, sourceRect, xAlignment, yAlignment);
}

bool Ui::clearActionPopup (Widget *target, Widget::EventCallback sourceFn) {
	bool match;

	match = actionWidget.widget && (actionTarget.widget == target) && (actionSource == sourceFn);
	clearPopupWidgets ();

	return (match);
}

void Ui::assignPopupPosition (Widget *popupWidget, const Widget::Rectangle &popupSourceRect, int xAlignment, int yAlignment) {
	float x, y;

	switch (xAlignment) {
		case Ui::LeftOfAlignment: {
			x = popupSourceRect.x - popupWidget->width;
			if (x <= UiConfiguration::instance->marginSize) {
				x = popupSourceRect.x + popupSourceRect.w;
			}
			break;
		}
		case Ui::LeftEdgeAlignment: {
			x = popupSourceRect.x;
			if (x < UiConfiguration::instance->marginSize) {
				x = UiConfiguration::instance->marginSize;
			}
			if ((x + popupWidget->width) >= (App::instance->windowWidth - UiConfiguration::instance->marginSize)) {
				x = App::instance->windowWidth - UiConfiguration::instance->marginSize - popupWidget->width;
			}
			break;
		}
		case Ui::RightEdgeAlignment: {
			x = popupSourceRect.x + popupSourceRect.w - popupWidget->width;
			if (x < UiConfiguration::instance->marginSize) {
				x = UiConfiguration::instance->marginSize;
			}
			if ((x + popupWidget->width) >= (App::instance->windowWidth - UiConfiguration::instance->marginSize)) {
				x = App::instance->windowWidth - UiConfiguration::instance->marginSize - popupWidget->width;
			}
			break;
		}
		case Ui::RightOfAlignment: {
			x = popupSourceRect.x + popupSourceRect.w;
			if ((x + popupWidget->width) >= (App::instance->windowWidth - UiConfiguration::instance->marginSize)) {
				x = popupSourceRect.x - popupWidget->width;
			}
			break;
		}
		default: {
			x = popupSourceRect.x + ((popupSourceRect.w / 2.0f) - (popupWidget->width / 2.0f));
			if (x < UiConfiguration::instance->marginSize) {
				x = UiConfiguration::instance->marginSize;
			}
			if ((x + popupWidget->width) > (App::instance->windowWidth - UiConfiguration::instance->marginSize)) {
				x = App::instance->windowWidth - UiConfiguration::instance->marginSize - popupWidget->width;
			}
			break;
		}
	}

	switch (yAlignment) {
		case Ui::TopOfAlignment: {
			y = popupSourceRect.y - popupWidget->height;
			if (y <= UiConfiguration::instance->marginSize) {
				y = popupSourceRect.y + popupSourceRect.h;
			}
			break;
		}
		case Ui::TopEdgeAlignment: {
			y = popupSourceRect.y;
			if (y < UiConfiguration::instance->marginSize) {
				y = UiConfiguration::instance->marginSize;
			}
			if ((y + popupWidget->height) > (App::instance->windowHeight - UiConfiguration::instance->marginSize)) {
				y = App::instance->windowHeight - UiConfiguration::instance->marginSize - popupWidget->height;
			}
			break;
		}
		case Ui::BottomEdgeAlignment: {
			y = popupSourceRect.y + popupSourceRect.h - popupWidget->height;
			if (y < UiConfiguration::instance->marginSize) {
				y = UiConfiguration::instance->marginSize;
			}
			if ((y + popupWidget->height) >= (App::instance->windowHeight - UiConfiguration::instance->marginSize)) {
				y = App::instance->windowHeight - UiConfiguration::instance->marginSize - popupWidget->height;
			}
			break;
		}
		case Ui::BottomOfAlignment: {
			y = popupSourceRect.y + popupSourceRect.h;
			if ((y + popupWidget->height) >= (App::instance->windowHeight - UiConfiguration::instance->marginSize)) {
				y = popupSourceRect.y - popupWidget->height;
			}
			break;
		}
		default: {
			y = popupSourceRect.y + ((popupSourceRect.h / 2.0f) - (popupWidget->height / 2.0f));
			if (y < UiConfiguration::instance->marginSize) {
				y = UiConfiguration::instance->marginSize;
			}
			if ((y + popupWidget->height) > (App::instance->windowHeight - UiConfiguration::instance->marginSize)) {
				y = App::instance->windowHeight - UiConfiguration::instance->marginSize - popupWidget->height;
			}
			break;
		}
	}

	popupWidget->position.assign (x, y);
	popupWidget->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
}

Panel *Ui::createRowHeaderPanel (const StdString &headerText, Panel *sidePanel) {
	Panel *panel;
	LabelWindow *label;

	panel = new Panel ();
	panel->setLayout (Panel::HorizontalLayout);
	if (! headerText.empty ()) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (headerText, UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));
		label->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	}
	if (sidePanel) {
		panel->addWidget (sidePanel);
	}
	panel->refresh ();

	return (panel);
}

Panel *Ui::createLoadingIconWindow () {
	IconLabelWindow *icon;

	icon = new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallLoadingIconSprite), UiText::instance->getText (UiTextString::Loading).capitalized (), UiConfiguration::CaptionFont);
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setProgressBar (true);

	return (icon);
}
