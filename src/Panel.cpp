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
#include "SDL2/SDL.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Input.h"
#include "Widget.h"
#include "ProgressBar.h"
#include "Panel.h"

Panel::Panel ()
: Widget ()
, bgColor (0.0f, 0.0f, 0.0f)
, borderColor (0.0f, 0.0f, 0.0f)
, layout (-1)
, maxWidgetX (0.0f)
, maxWidgetY (0.0f)
, maxWidgetZLevel (0)
, viewOriginX (0.0f)
, viewOriginY (0.0f)
, isViewOriginBoundEnabled (false)
, minViewOriginX (0.0f)
, minViewOriginY (0.0f)
, maxViewOriginX (0.0f)
, maxViewOriginY (0.0f)
, widthPadding (0.0f)
, heightPadding (0.0f)
, isAlphaBlended (false)
, alpha (1.0f)
, alphaTarget (1.0f)
, alphaTransitionDuration (0)
, isFilledBg (false)
, isBordered (false)
, isFixedSize (false)
, isMouseDragScrollEnabled (false)
, isWaiting (false)
, isMouseInputStarted (false)
, lastMouseLeftUpCount (0)
, lastMouseLeftDownCount (0)
, lastMouseRightUpCount (0)
, lastMouseRightDownCount (0)
, lastMouseWheelUpCount (0)
, lastMouseWheelDownCount (0)
, lastMouseDownX (-1)
, lastMouseDownY (-1)
, widgetListMutex (NULL)
, widgetAddListMutex (NULL)
{
	typeName.assign ("Panel");

	widgetListMutex = SDL_CreateMutex ();
	widgetAddListMutex = SDL_CreateMutex ();
}

Panel::~Panel () {
	clear ();
	if (widgetListMutex) {
		SDL_DestroyMutex (widgetListMutex);
		widgetListMutex = NULL;
	}

	if (widgetAddListMutex) {
		SDL_DestroyMutex (widgetAddListMutex);
		widgetAddListMutex = NULL;
	}
}

void Panel::clear () {
	std::list<Widget *>::iterator i, end;
	Widget *widget;

	SDL_LockMutex (widgetAddListMutex);
	i = widgetAddList.begin ();
	end = widgetAddList.end ();
	while (i != end) {
		widget = *i;
		widget->parentWidget = NULL;
		widget->release ();
		++i;
	}
	widgetAddList.clear ();
	SDL_UnlockMutex (widgetAddListMutex);

	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		widget->parentWidget = NULL;
		widget->release ();
		++i;
	}
	widgetList.clear ();
	SDL_UnlockMutex (widgetListMutex);

	resetSize ();
}

void Panel::destroyAllChildWidgets () {
	std::list<Widget *>::iterator i, end;
	Widget *widget;

	SDL_LockMutex (widgetAddListMutex);
	i = widgetAddList.begin ();
	end = widgetAddList.end ();
	while (i != end) {
		widget = *i;
		widget->isDestroyed = true;
		widget->destroyAllChildWidgets ();
		++i;
	}
	SDL_UnlockMutex (widgetAddListMutex);

	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		widget->isDestroyed = true;
		widget->destroyAllChildWidgets ();
		++i;
	}
	SDL_UnlockMutex (widgetListMutex);

	resetSize ();
}

Widget *Panel::addWidget (Widget *widget, float positionX, float positionY, int zLevel) {
	if (widget->id <= 0) {
		widget->id = App::getInstance ()->getUniqueId ();
	}

	widget->position.assign (positionX, positionY);
	widget->zLevel = zLevel;
	widget->parentWidget = this;
	if (parentUi) {
		widget->setParentUi (parentUi);
	}
	widget->retain ();
	SDL_LockMutex (widgetAddListMutex);
	widgetAddList.push_back (widget);
	SDL_UnlockMutex (widgetAddListMutex);

	resetSize ();
	return (widget);
}

void Panel::removeWidget (Widget *targetWidget) {
	std::list<Widget *>::iterator i, end;
	Widget *widget;

	SDL_LockMutex (widgetAddListMutex);
	i = widgetAddList.begin ();
	end = widgetAddList.end ();
	while (i != end) {
		widget = *i;
		if (widget == targetWidget) {
			widgetAddList.erase (i);
			widget->release ();
			break;
		}
		++i;
	}
	SDL_UnlockMutex (widgetAddListMutex);

	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		if (widget == targetWidget) {
			widgetList.erase (i);
			widget->release ();
			break;
		}
		++i;
	}
	SDL_UnlockMutex (widgetListMutex);
}

Widget *Panel::findChildWidget (float positionX, float positionY, bool shouldRecurse) {
	std::list<Widget *>::reverse_iterator i, end;
	Widget *widget, *item, *nextitem;
	float x, y, w, h;

	item = NULL;
	SDL_LockMutex (widgetListMutex);
	i = widgetList.rbegin ();
	end = widgetList.rend ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed || (! widget->isVisible) || (! widget->isDrawable)) {
			continue;
		}

		w = widget->width;
		h = widget->height;
		if ((w <= 0.0f) || (h <= 0.0f)) {
			continue;
		}
		x = widget->position.x;
		y = widget->position.y;
		if ((positionX >= x) && (positionX <= (x + w)) && (positionY >= y) && (positionY <= (y + h))) {
			item = widget;
			break;
		}
	}
	SDL_UnlockMutex (widgetListMutex);

	if (! item) {
		SDL_LockMutex (widgetAddListMutex);
		i = widgetAddList.rbegin ();
		end = widgetAddList.rend ();
		while (i != end) {
			widget = *i;
			++i;
			if (widget->isDestroyed || (! widget->isVisible) || (! widget->isDrawable)) {
				continue;
			}

			w = widget->width;
			h = widget->height;
			if ((w <= 0.0f) || (h <= 0.0f)) {
				continue;
			}
			x = widget->position.x;
			y = widget->position.y;
			if ((positionX >= x) && (positionX <= (x + w)) && (positionY >= y) && (positionY <= (y + h))) {
				item = widget;
				break;
			}
		}
		SDL_UnlockMutex (widgetAddListMutex);
	}

	if (item && shouldRecurse) {
		nextitem = item->findChildWidget (positionX - item->position.x, positionY - item->position.y, true);
		if (nextitem) {
			item = nextitem;
		}
	}

	return (item);
}

Widget *Panel::findChildWidget (uint64_t widgetId, const StdString &widgetTypeName, bool shouldRecurse) {
	std::list<Widget *>::iterator i, end;
	Widget *widget, *item, *nextitem;

	item = NULL;
	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		if ((widget->id == widgetId) && (widget->typeName.equals (widgetTypeName))) {
			item = widget;
			break;
		}

		if (shouldRecurse) {
			nextitem = widget->findChildWidget (widgetId, widgetTypeName, true);
			if (nextitem) {
				item = nextitem;
				break;
			}
		}
		++i;
	}
	SDL_UnlockMutex (widgetListMutex);

	if (! item) {
		SDL_LockMutex (widgetAddListMutex);
		i = widgetAddList.begin ();
		end = widgetAddList.end ();
		while (i != end) {
			widget = *i;
			if ((widget->id == widgetId) && (widget->typeName.equals (widgetTypeName))) {
				item = widget;
				break;
			}

			if (shouldRecurse) {
				nextitem = widget->findChildWidget (widgetId, widgetTypeName, true);
				if (nextitem) {
					item = nextitem;
					break;
				}
			}

			++i;
		}
		SDL_UnlockMutex (widgetAddListMutex);
	}

	return (item);
}

Widget *Panel::findMouseHoverWidget (float mouseX, float mouseY) {
	std::list<Widget *>::reverse_iterator i, end;
	Widget *widget, *item, *nextitem;
	float x, y, w, h;

	item = NULL;
	SDL_LockMutex (widgetListMutex);
	i = widgetList.rbegin ();
	end = widgetList.rend ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed || (! widget->isVisible) || (! widget->isDrawable)) {
			continue;
		}

		w = widget->width;
		h = widget->height;
		if ((w <= 0.0f) || (h <= 0.0f)) {
			continue;
		}
		x = widget->drawX;
		y = widget->drawY;
		if ((mouseX >= x) && (mouseX <= (x + w)) && (mouseY >= y) && (mouseY <= (y + h))) {
			item = widget;
			break;
		}
	}
	SDL_UnlockMutex (widgetListMutex);

	if (! item) {
		SDL_LockMutex (widgetAddListMutex);
		i = widgetAddList.rbegin ();
		end = widgetAddList.rend ();
		while (i != end) {
			widget = *i;
			++i;
			if (widget->isDestroyed || (! widget->isVisible) || (! widget->isDrawable)) {
				continue;
			}

			w = widget->width;
			h = widget->height;
			if ((w <= 0.0f) || (h <= 0.0f)) {
				continue;
			}
			x = widget->drawX;
			y = widget->drawY;
			if ((mouseX >= x) && (mouseX <= (x + w)) && (mouseY >= y) && (mouseY <= (y + h))) {
				item = widget;
				break;
			}
		}
		SDL_UnlockMutex (widgetAddListMutex);
	}

	if (item) {
		nextitem = item->findMouseHoverWidget (mouseX, mouseY);
		if (nextitem && nextitem->isMouseHoverEnabled) {
			item = nextitem;
		}
	}
	if (item && (! item->isMouseHoverEnabled)) {
		item = NULL;
	}

	return (item);
}

void Panel::doUpdate (int msElapsed, float originX, float originY) {
	std::list<Widget *> addlist;
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	Panel *panel;
	ProgressBar *bar;
	bool found;

	bgColor.update (msElapsed);
	borderColor.update (msElapsed);

	if (alphaTransitionDuration > 0) {
		if (alpha < alphaTarget) {
			alpha += (1.0f / ((float) alphaTransitionDuration)) * (float) msElapsed;
			if (alpha >= alphaTarget) {
				alpha = alphaTarget;
				alphaTransitionDuration = 0;
			}
		}
		else {
			alpha -= (1.0f / ((float) alphaTransitionDuration)) * (float) msElapsed;
			if (alpha <= alphaTarget) {
				alpha = alphaTarget;
				alphaTransitionDuration = 0;
			}
		}
	}

	SDL_LockMutex (widgetAddListMutex);
	addlist.swap (widgetAddList);
	SDL_UnlockMutex (widgetAddListMutex);

	SDL_LockMutex (widgetListMutex);
	widgetList.splice (widgetList.end (), addlist);
	addlist.clear ();
	while (true) {
		found = false;
		i = widgetList.begin ();
		end = widgetList.end ();
		while (i != end) {
			widget = *i;
			if (widget->isDestroyed) {
				found = true;
				widgetList.erase (i);
				widget->destroyAllChildWidgets ();
				widget->parentWidget = NULL;
				widget->release ();
				break;
			}
			++i;
		}

		if (! found) {
			break;
		}
	}

	sortWidgetList ();

	waitPanel.compact ();
	waitProgressBar.compact ();
	if (waitPanel.widget) {
		panel = (Panel *) waitPanel.widget;
		panel->setFixedSize (true, width, height);

		if (waitProgressBar.widget) {
			bar = (ProgressBar *) waitProgressBar.widget;
			bar->setSize (width, App::getInstance ()->uiConfig.progressBarHeight);
		}
	}

	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		widget->update (msElapsed, drawX - viewOriginX, drawY - viewOriginY);
		++i;
	}
	SDL_UnlockMutex (widgetListMutex);
}

void Panel::processInput () {
	App *app;
	Input *input;
	std::list<Widget *>::reverse_iterator i, iend;
	std::vector<SDL_Keycode> keyevents;
	std::vector<SDL_Keycode>::iterator j, jend;
	Widget *widget, *item;
	Widget::MouseState mousestate;
	float x, y, enterdx, enterdy;
	bool isshiftdown, iscontroldown;

	app = App::getInstance ();
	input = &(app->input);

	input->pollKeyPressEvents (&keyevents);
	isshiftdown = input->isShiftDown ();
	iscontroldown = input->isControlDown ();

	if (isMouseInputStarted) {
		if (input->mouseLeftDownCount != lastMouseLeftDownCount) {
			mousestate.isLeftClicked = true;
		}

		if (input->mouseLeftUpCount != lastMouseLeftUpCount) {
			mousestate.isLeftClickReleased = true;
		}

		mousestate.positionDeltaX = input->mouseX - input->lastMouseX;
		mousestate.positionDeltaY = input->mouseY - input->lastMouseY;
		mousestate.wheelUp = input->mouseWheelUpCount - lastMouseWheelUpCount;
		mousestate.wheelDown = input->mouseWheelDownCount - lastMouseWheelDownCount;
	}
	lastMouseLeftUpCount = input->mouseLeftUpCount;
	lastMouseLeftDownCount = input->mouseLeftDownCount;
	lastMouseRightUpCount = input->mouseRightUpCount;
	lastMouseRightDownCount = input->mouseRightDownCount;
	lastMouseWheelUpCount = input->mouseWheelUpCount;
	lastMouseWheelDownCount = input->mouseWheelDownCount;
	isMouseInputStarted = true;

	item = NULL;
	x = input->mouseX;
	y = input->mouseY;
	enterdx = 0.0f;
	enterdy = 0.0f;
	SDL_LockMutex (widgetListMutex);
	i = widgetList.rbegin ();
	iend = widgetList.rend ();
	while (i != iend) {
		widget = *i;
		++i;
		if (widget->isDestroyed || widget->isInputSuspended || (! widget->isVisible) || (! widget->isDrawable)) {
			continue;
		}

		if ((widget->width > 0.0f) && (widget->height > 0.0f)) {
			if ((x >= (int) widget->drawX) && (x <= (int) (widget->drawX + widget->width)) && (y >= (int) widget->drawY) && (y <= (int) (widget->drawY + widget->height))) {
				item = widget;
				enterdx = x - widget->drawX;
				enterdy = y - widget->drawY;
				break;
			}
		}
	}

	if (mousestate.isLeftClicked) {
		if (item) {
			lastMouseDownX = x;
			lastMouseDownY = y;
		}
		else {
			lastMouseDownX = -1;
			lastMouseDownY = -1;
		}
	}

	i = widgetList.rbegin ();
	iend = widgetList.rend ();
	while (i != iend) {
		widget = *i;
		++i;
		if (widget->isDestroyed || widget->isInputSuspended || (! widget->isVisible) || (! widget->isDrawable)) {
			continue;
		}

		if (keyevents.size () > 0) {
			j = keyevents.begin ();
			jend = keyevents.end ();
			while (j != jend) {
				if (widget->processKeyEvent (*j, isshiftdown, iscontroldown)) {
					break;
				}
				++j;
			}
		}

		if (widget == item) {
			mousestate.isEntered = true;
			mousestate.enterDeltaX = enterdx;
			mousestate.enterDeltaY = enterdy;
		}
		else {
			mousestate.isEntered = false;
			mousestate.enterDeltaX = 0.0f;
			mousestate.enterDeltaY = 0.0f;
		}

		mousestate.isLeftClickEntered = false;
		if (mousestate.isLeftClickReleased && (lastMouseDownX >= 0)) {
			if ((lastMouseDownX >= (int) widget->drawX) && (lastMouseDownX <= (int) (widget->drawX + widget->width)) && (lastMouseDownY >= (int) widget->drawY) && (lastMouseDownY <= (int) (widget->drawY + widget->height))) {
				mousestate.isLeftClickEntered = true;
				lastMouseDownX = -1;
				lastMouseDownY = -1;
			}
		}

		widget->processMouseState (mousestate);
	}

	SDL_UnlockMutex (widgetListMutex);
}

void Panel::syncRecordStore (RecordStore *store) {
	std::list<Widget *>::iterator i, end;
	Widget *widget;

	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed) {
			continue;
		}

		widget->syncRecordStore (store);
	}
	SDL_UnlockMutex (widgetListMutex);

	SDL_LockMutex (widgetAddListMutex);
	i = widgetAddList.begin ();
	end = widgetAddList.end ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed) {
			continue;
		}

		widget->syncRecordStore (store);
	}
	SDL_UnlockMutex (widgetAddListMutex);
}

bool Panel::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	bool result;

	result = false;
	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed || widget->isInputSuspended) {
			continue;
		}

		result = widget->processKeyEvent (keycode, isShiftDown, isControlDown);
		if (result) {
			break;
		}
	}
	SDL_UnlockMutex (widgetListMutex);

	return (result);
}

void Panel::doProcessMouseState (const Widget::MouseState &mouseState) {
	Input *input;
	std::list<Widget *>::reverse_iterator i, end;
	Widget *widget;
	bool found;
	Widget::MouseState m;
	float x, y;

	input = &(App::getInstance ()->input);
	x = input->mouseX;
	y = input->mouseY;

	found = false;
	SDL_LockMutex (widgetListMutex);
	i = widgetList.rbegin ();
	end = widgetList.rend ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed || widget->isInputSuspended || (! widget->isVisible) || (! widget->isDrawable)) {
			continue;
		}

		m = mouseState;
		m.isEntered = false;
		m.enterDeltaX = 0.0f;
		m.enterDeltaY = 0.0f;
		if ((! found) && mouseState.isEntered) {
			if ((widget->width > 0.0f) && (widget->height > 0.0f)) {
				if ((x >= (int) widget->drawX) && (x <= (int) (widget->drawX + widget->width)) && (y >= (int) widget->drawY) && (y <= (int) (widget->drawY + widget->height))) {
					m.isEntered = true;
					m.enterDeltaX = x - widget->drawX;
					m.enterDeltaY = y - widget->drawY;
					found = true;
				}
			}
		}
		widget->processMouseState (m);
	}
	SDL_UnlockMutex (widgetListMutex);

	if (isMouseDragScrollEnabled && (! isInputSuspended)) {
		// TODO: Don't scroll if a widget is present at the mouse position
		if (mouseState.isEntered && input->isMouseLeftButtonDown) {
			setViewOrigin (viewOriginX - (float) mouseState.positionDeltaX, viewOriginY - (float) mouseState.positionDeltaY);
		}
	}
}

void Panel::doResetInputState () {
	Input *input;
	std::list<Widget *>::iterator i, end;
	Widget *widget;

	input = &(App::getInstance ()->input);
	lastMouseLeftUpCount = input->mouseLeftUpCount;
	lastMouseLeftDownCount = input->mouseLeftDownCount;
	lastMouseRightUpCount = input->mouseRightUpCount;
	lastMouseRightDownCount = input->mouseRightDownCount;
	lastMouseWheelUpCount = input->mouseWheelUpCount;
	lastMouseWheelDownCount = input->mouseWheelDownCount;

	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		widget->resetInputState ();
		++i;
	}
	SDL_UnlockMutex (widgetListMutex);
}

void Panel::doDraw () {
	App *app;
	SDL_Rect rect;
	std::list<Widget *>::iterator i, end;
	Widget *widget;

	app = App::getInstance ();

	rect.x = (int) drawX;
	rect.y = (int) drawY;
	rect.w = (int) width;
	rect.h = (int) height;
	app->pushClipRect (&rect);

	if (isFilledBg) {
		rect.x = (int) drawX;
		rect.y = (int) drawY;
		rect.w = (int) width;
		rect.h = (int) height;
		if (isAlphaBlended) {
			SDL_SetRenderDrawColor (app->render, bgColor.rByte, bgColor.gByte, bgColor.bByte, (Uint8) (alpha * 255.0f));
			SDL_SetRenderDrawBlendMode (app->render, SDL_BLENDMODE_BLEND);
		}
		else {
			SDL_SetRenderDrawColor (app->render, bgColor.rByte, bgColor.gByte, bgColor.bByte, 255);
		}
		SDL_RenderFillRect (app->render, &rect);
		if (isAlphaBlended) {
			SDL_SetRenderDrawBlendMode (app->render, SDL_BLENDMODE_NONE);
		}
	}

	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		++i;

		if (widget->isDestroyed || (! widget->isVisible) || (! widget->isDrawable)) {
			continue;
		}

		widget->draw ();
	}
	SDL_UnlockMutex (widgetListMutex);

	if (isBordered) {
		rect.x = (int) drawX;
		rect.y = (int) drawY;
		rect.w = (int) width;
		rect.h = (int) height;

		if (isAlphaBlended) {
			SDL_SetRenderDrawColor (app->render, borderColor.rByte, borderColor.gByte, borderColor.bByte, (Uint8) (alpha * 255.0f));
			SDL_SetRenderDrawBlendMode (app->render, SDL_BLENDMODE_BLEND);
		}
		else {
			SDL_SetRenderDrawColor (app->render, borderColor.rByte, borderColor.gByte, borderColor.bByte, 255);
		}
		SDL_RenderDrawRect (app->render, &rect);
		if (isAlphaBlended) {
			SDL_SetRenderDrawBlendMode (app->render, SDL_BLENDMODE_NONE);
		}
	}

	app->popClipRect ();
}

void Panel::doRefresh () {
	std::list<Widget *>::iterator i, end;
	Widget *widget;

	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed) {
			continue;
		}

		widget->refresh ();
	}
	SDL_UnlockMutex (widgetListMutex);

	resetLayout ();
}

void Panel::resetSize () {
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	float xmax, ymax, wx, wy;

	xmax = 0.0f;
	ymax = 0.0f;

	SDL_LockMutex (widgetAddListMutex);
	i = widgetAddList.begin ();
	end = widgetAddList.end ();
	while (i != end) {
		widget = *i;
		++i;

		if (widget->isDestroyed || (! widget->isVisible)) {
			continue;
		}

		wx = widget->position.x + widget->width;
		wy = widget->position.y + widget->height;
		if ((xmax <= 0.0f) || (wx > xmax)) {
			xmax = wx;
		}
		if ((ymax <= 0.0f) || (wy > ymax)) {
			ymax = wy;
		}
	}
	SDL_UnlockMutex (widgetAddListMutex);

	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		++i;

		if (widget->isDestroyed || (! widget->isVisible)) {
			continue;
		}

		wx = widget->position.x + widget->width;
		wy = widget->position.y + widget->height;
		if ((xmax <= 0.0f) || (wx > xmax)) {
			xmax = wx;
		}
		if ((ymax <= 0.0f) || (wy > ymax)) {
			ymax = wy;
		}
	}
	SDL_UnlockMutex (widgetListMutex);

	maxWidgetX = xmax;
	maxWidgetY = ymax;
	if (! isFixedSize) {
		width = xmax + widthPadding;
		height = ymax + heightPadding;
	}
}

void Panel::resetLayout () {
	UiConfiguration *uiconfig;
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	float x, y, maxw;

	switch (layout) {
		case Panel::VERTICAL: {
			uiconfig = &(App::getInstance ()->uiConfig);
			x = widthPadding;
			y = heightPadding;

			SDL_LockMutex (widgetListMutex);
			i = widgetList.begin ();
			end = widgetList.end ();
			while (i != end) {
				widget = *i;
				widget->position.assign (x, y);
				y += widget->height + uiconfig->marginSize;
				++i;
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i = widgetAddList.begin ();
			end = widgetAddList.end ();
			while (i != end) {
				widget = *i;
				widget->position.assign (x, y);
				y += widget->height + uiconfig->marginSize;
				++i;
			}
			SDL_UnlockMutex (widgetAddListMutex);
			break;
		}
		case Panel::VERTICAL_RIGHT_JUSTIFIED: {
			uiconfig = &(App::getInstance ()->uiConfig);
			x = widthPadding;
			y = heightPadding;
			maxw = 0.0f;

			SDL_LockMutex (widgetListMutex);
			i = widgetList.begin ();
			end = widgetList.end ();
			while (i != end) {
				widget = *i;
				widget->position.assign (x, y);
				y += widget->height + uiconfig->marginSize;
				if (widget->width > maxw) {
					maxw = widget->width;
				}
				++i;
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i = widgetAddList.begin ();
			end = widgetAddList.end ();
			while (i != end) {
				widget = *i;
				widget->position.assign (x, y);
				y += widget->height + uiconfig->marginSize;
				if (widget->width > maxw) {
					maxw = widget->width;
				}
				++i;
			}
			SDL_UnlockMutex (widgetAddListMutex);

			SDL_LockMutex (widgetListMutex);
			i = widgetList.begin ();
			end = widgetList.end ();
			while (i != end) {
				widget = *i;
				widget->position.assignX (x + maxw - widget->width);
				++i;
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i = widgetAddList.begin ();
			end = widgetAddList.end ();
			while (i != end) {
				widget = *i;
				widget->position.assignX (x + maxw - widget->width);
				++i;
			}
			SDL_UnlockMutex (widgetAddListMutex);
			break;
		}
		case Panel::HORIZONTAL: {
			uiconfig = &(App::getInstance ()->uiConfig);
			x = widthPadding;
			y = heightPadding;

			SDL_LockMutex (widgetListMutex);
			i = widgetList.begin ();
			end = widgetList.end ();
			while (i != end) {
				widget = *i;
				widget->position.assign (x, y);
				x += widget->width + uiconfig->marginSize;
				++i;
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i = widgetAddList.begin ();
			end = widgetAddList.end ();
			while (i != end) {
				widget = *i;
				widget->position.assign (x, y);
				x += widget->width + uiconfig->marginSize;
				++i;
			}
			SDL_UnlockMutex (widgetAddListMutex);
			break;
		}
	}
	resetSize ();
}

void Panel::sortWidgetList () {
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	int minlevel, maxlevel;
	bool sorted;

	sorted = true;
	minlevel = Widget::minZLevel - 1;
	maxlevel = Widget::minZLevel - 1;
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		if (minlevel < Widget::minZLevel) {
			minlevel = widget->zLevel;
		}
		if (maxlevel < Widget::minZLevel) {
			maxlevel = widget->zLevel;
		}

		if (widget->zLevel < minlevel) {
			sorted = false;
			minlevel = widget->zLevel;
		}
		if (widget->zLevel < maxlevel) {
			sorted = false;
		}
		if (widget->zLevel > maxlevel) {
			maxlevel = widget->zLevel;
		}
		++i;
	}

	maxWidgetZLevel = maxlevel;
	if (sorted) {
		return;
	}

	widgetList.sort (Widget::compareZLevel);
}

void Panel::setAlphaBlend (bool enable, float blendAlpha) {
	isAlphaBlended = enable;
	if (isAlphaBlended) {
		alpha = blendAlpha;
	}
}

void Panel::transitionAlphaBlend (float startAlpha, float targetAlpha, int durationMs) {
	if (startAlpha < 0.0f) {
		startAlpha = 0.0f;
	}
	if (startAlpha > 1.0f) {
		startAlpha = 1.0f;
	}
	if (targetAlpha < 0.0f) {
		targetAlpha = 0.0f;
	}
	if (targetAlpha > 1.0f) {
		targetAlpha = 1.0f;
	}

	if (durationMs < 0) {
		durationMs = 0;
	}
	alphaTransitionDuration = durationMs;
	if (alphaTransitionDuration <= 0) {
		setAlphaBlend (true, targetAlpha);
		return;
	}

	setAlphaBlend (true, startAlpha);
	alphaTarget = targetAlpha;
}

void Panel::setFillBg (bool enable, const Color &color) {
	setFillBg (enable, color.r, color.g, color.b);
}

void Panel::setFillBg (bool enable, float r, float g, float b) {
	if (enable) {
		isFilledBg = true;
		bgColor.assign (r, g, b);
	}
	else {
		isFilledBg = false;
	}
}

void Panel::setBorder (bool enable, const Color &color) {
	setBorder (enable, color.r, color.g, color.b);
}

void Panel::setBorder (bool enable, float r, float g, float b) {
	if (enable) {
		isBordered = true;
		borderColor.assign (r, g, b);
	}
	else {
		isBordered = false;
	}
}

void Panel::setViewOrigin (float originX, float originY) {
	float x, y;

	x = originX;
	y = originY;

	if (isViewOriginBoundEnabled) {
		if (x < minViewOriginX) {
			x = minViewOriginX;
		}
		if (x > maxViewOriginX) {
			x = maxViewOriginX;
		}
		if (y < minViewOriginY) {
			y = minViewOriginY;
		}
		if (y > maxViewOriginY) {
			y = maxViewOriginY;
		}
	}

	viewOriginX = x;
	viewOriginY = y;
}

void Panel::setViewOriginBounds (float originX1, float originY1, float originX2, float originY2) {
	isViewOriginBoundEnabled = true;
	minViewOriginX = originX1;
	minViewOriginY = originY1;
	maxViewOriginX = originX2;
	maxViewOriginY = originY2;
}

void Panel::setPadding (float widthPaddingSize, float heightPaddingSize) {
	widthPadding = widthPaddingSize;
	heightPadding = heightPaddingSize;
	resetSize ();
}

void Panel::setFixedSize (bool enable, float fixedWidth, float fixedHeight) {
	if (enable) {
		isFixedSize = true;
		width = fixedWidth;
		height = fixedHeight;
	}
	else {
		isFixedSize = false;
	}
}

void Panel::setMouseDragScroll (bool enable) {
	isMouseDragScrollEnabled = enable;
}

void Panel::setWaiting (bool enable) {
	UiConfiguration *uiconfig;
	Panel *panel;
	ProgressBar *bar;

	if (isWaiting == enable) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	isWaiting = enable;
	if (isWaiting) {
		isInputSuspended = true;

		panel = (Panel *) addWidget (new Panel ());
		panel->setFixedSize (true, width, height);
		panel->setFillBg (true, 0.0f, 0.0f, 0.0f);
		panel->setAlphaBlend (true, uiconfig->waitingShadeAlpha);
		panel->zLevel = maxWidgetZLevel + 1;

		bar = (ProgressBar *) panel->addWidget (new ProgressBar (width, uiconfig->progressBarHeight));
		bar->setIndeterminate (true);
		bar->position.assign (0.0f, height - bar->height);

		waitPanel.assign (panel);
		waitProgressBar.assign (bar);
	}
	else {
		isInputSuspended = false;
		waitPanel.destroyAndClear ();
		waitProgressBar.clear ();
	}
}
