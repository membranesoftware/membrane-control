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
#include <math.h>
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
, dropShadowColor (0.0f, 0.0f, 0.0f, 0.8f)
, shouldRefreshTexture (false)
, isTextureRenderEnabled (false)
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
, isFilledBg (false)
, isBordered (false)
, borderWidth (0.0f)
, isDropShadowed (false)
, dropShadowWidth (0.0f)
, isFixedSize (false)
, isMouseDragScrollEnabled (false)
, isWaiting (false)
, layout (-1)
, isAnimating (false)
, drawTexture (NULL)
, drawTextureWidth (0)
, drawTextureHeight (0)
, isResettingDrawTexture (false)
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
	widgetType.assign ("Panel");

	widgetListMutex = SDL_CreateMutex ();
	widgetAddListMutex = SDL_CreateMutex ();
	animationScale.assign (1.0f, 1.0f);
}

Panel::~Panel () {
	clear ();

	if (! drawTexturePath.empty ()) {
		App::instance->resource.unloadTexture (drawTexturePath);
		drawTexturePath.assign ("");
	}
	drawTexture = NULL;

	if (widgetListMutex) {
		SDL_DestroyMutex (widgetListMutex);
		widgetListMutex = NULL;
	}

	if (widgetAddListMutex) {
		SDL_DestroyMutex (widgetAddListMutex);
		widgetAddListMutex = NULL;
	}
}

void Panel::setTextureRender (bool enable) {
	if (! App::instance->isInterfaceAnimationEnabled) {
		return;
	}
	if (isTextureRenderEnabled == enable) {
		return;
	}
	isTextureRenderEnabled = enable;
	animationScale.assignX (1.0f);
}

void Panel::resetDrawTexture (void *panelPtr) {
	Panel *panel;
	SDL_Texture *texture;

	panel = (Panel *) panelPtr;
	if (! panel->isTextureRenderEnabled) {
		if (! panel->drawTexturePath.empty ()) {
			App::instance->resource.unloadTexture (panel->drawTexturePath);
			panel->drawTexturePath.assign ("");
		}
		panel->drawTexture = NULL;
		panel->isResettingDrawTexture = false;
		panel->release ();
		return;
	}

	texture = panel->drawTexture;
	if (texture) {
		if (((int) panel->width) != panel->drawTextureWidth || (((int) panel->height) != panel->drawTextureHeight)) {
			texture = NULL;
		}
	}

	if (! texture) {
		if (! panel->drawTexturePath.empty ()) {
			App::instance->resource.unloadTexture (panel->drawTexturePath);
		}
		panel->drawTexture = NULL;

		panel->drawTexturePath.sprintf ("*_Panel_%llx_%llx", (long long int) panel->id, (long long int) App::instance->getUniqueId ());
		panel->drawTextureWidth = (int) panel->width;
		panel->drawTextureHeight = (int) panel->height;
		texture = App::instance->resource.createTexture (panel->drawTexturePath, panel->drawTextureWidth, panel->drawTextureHeight);
	}
	if (! texture) {
		panel->drawTexturePath.assign ("");
		panel->isTextureRenderEnabled = false;
	}
	else {
		panel->drawTexture = texture;
		panel->draw (texture, -(panel->position.x), -(panel->position.y));
	}
	panel->shouldRefreshTexture = false;
	panel->isResettingDrawTexture = false;
	panel->release ();
}

void Panel::animateScale (float startScale, float targetScale, int duration) {
	if (! App::instance->isInterfaceAnimationEnabled) {
		return;
	}

	isAnimating = true;
	setTextureRender (true);
	animationScale.translateX (startScale, targetScale, duration);
}

void Panel::animateNewCard () {
	if (! App::instance->isInterfaceAnimationEnabled) {
		return;
	}

	isAnimating = true;
	setTextureRender (true);
	animationScale.assignX (0.8f);
	animationScale.plotX (0.4f, 80);
	animationScale.plotX (-0.2f, 80);
}

void Panel::clear () {
	std::list<Widget *>::iterator i, end;
	Widget *widget;

	SDL_LockMutex (widgetAddListMutex);
	i = widgetAddList.begin ();
	end = widgetAddList.end ();
	while (i != end) {
		widget = *i;
		widget->isDestroyed = true;
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
		widget->isDestroyed = true;
		widget->release ();
		++i;
	}
	widgetList.clear ();
	SDL_UnlockMutex (widgetListMutex);

	resetSize ();
}

Widget *Panel::addWidget (Widget *widget, float positionX, float positionY, int zLevel) {
	if (widget->id <= 0) {
		widget->id = App::instance->getUniqueId ();
	}
	if (widget->sortKey.empty ()) {
		widget->sortKey.sprintf ("%016llx", (unsigned long long) widget->id);
	}

	widget->position.assign (positionX, positionY);
	widget->zLevel = zLevel;
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

Widget *Panel::findWidget (float screenPositionX, float screenPositionY, bool requireMouseHoverEnabled) {
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
		if (widget->isDestroyed || (! widget->isVisible) || (! widget->hasScreenPosition)) {
			continue;
		}

		w = widget->width;
		h = widget->height;
		if ((w <= 0.0f) || (h <= 0.0f)) {
			continue;
		}
		x = widget->screenX;
		y = widget->screenY;
		if ((screenPositionX >= x) && (screenPositionX <= (x + w)) && (screenPositionY >= y) && (screenPositionY <= (y + h))) {
			item = widget;
			break;
		}
	}
	SDL_UnlockMutex (widgetListMutex);

	if (item) {
		nextitem = item->findWidget (screenPositionX, screenPositionY, requireMouseHoverEnabled);
		if (nextitem) {
			item = nextitem;
		}
	}
	if (item && (requireMouseHoverEnabled && (! item->isMouseHoverEnabled))) {
		item = NULL;
	}

	return (item);
}

void Panel::doUpdate (int msElapsed) {
	std::list<Widget *> addlist;
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	Panel *panel;
	ProgressBar *bar;
	bool found;

	bgColor.update (msElapsed);
	borderColor.update (msElapsed);
	if (isAnimating) {
		animationScale.update (msElapsed);
		if (! animationScale.isTranslating) {
			isAnimating = false;
			if (FLOAT_EQUALS (animationScale.x, 1.0f)) {
				setTextureRender (false);
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
			bar->setSize (width, App::instance->uiConfig.progressBarHeight);
		}
	}

	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		widget->update (msElapsed, screenX - viewOriginX, screenY - viewOriginY);
		++i;
	}
	SDL_UnlockMutex (widgetListMutex);

	if (! isResettingDrawTexture) {
		if ((isTextureRenderEnabled && (! drawTexture)) || ((! isTextureRenderEnabled) && drawTexture)) {
			isResettingDrawTexture = true;
			retain ();
			App::instance->addRenderTask (Panel::resetDrawTexture, this);
		}
	}
}

void Panel::processInput () {
	Input *input;
	std::list<Widget *>::reverse_iterator i, iend;
	std::vector<SDL_Keycode> keyevents;
	std::vector<SDL_Keycode>::iterator j, jend;
	Widget *widget, *mousewidget;
	Widget::MouseState mousestate;
	float x, y, enterdx, enterdy;
	bool isshiftdown, iscontroldown;

	input = &(App::instance->input);

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

	mousewidget = NULL;
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
		if (widget->isDestroyed || widget->isInputSuspended || (! widget->isVisible) || (! widget->hasScreenPosition)) {
			continue;
		}

		if ((widget->width > 0.0f) && (widget->height > 0.0f)) {
			if ((x >= (int) widget->screenX) && (x <= (int) (widget->screenX + widget->width)) && (y >= (int) widget->screenY) && (y <= (int) (widget->screenY + widget->height))) {
				mousewidget = widget;
				enterdx = x - widget->screenX;
				enterdy = y - widget->screenY;
				break;
			}
		}
	}

	if (mousestate.isLeftClicked) {
		if (mousewidget) {
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
		if (widget->isDestroyed || widget->isInputSuspended || (! widget->isVisible) || (! widget->hasScreenPosition)) {
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

		mousestate.isLeftClickEntered = false;
		if (widget == mousewidget) {
			mousestate.isEntered = true;
			mousestate.enterDeltaX = enterdx;
			mousestate.enterDeltaY = enterdy;

			if (mousestate.isLeftClickReleased && (lastMouseDownX >= 0)) {
				if ((lastMouseDownX >= (int) widget->screenX) && (lastMouseDownX <= (int) (widget->screenX + widget->width)) && (lastMouseDownY >= (int) widget->screenY) && (lastMouseDownY <= (int) (widget->screenY + widget->height))) {
					mousestate.isLeftClickEntered = true;
					lastMouseDownX = -1;
					lastMouseDownY = -1;
				}
			}
		}
		else {
			mousestate.isEntered = false;
			mousestate.enterDeltaX = 0.0f;
			mousestate.enterDeltaY = 0.0f;
		}

		widget->processMouseState (mousestate);
	}
	SDL_UnlockMutex (widgetListMutex);
}

bool Panel::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	bool result;

	if (isTextureRenderEnabled) {
		return (false);
	}

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

	if (isTextureRenderEnabled) {
		return;
	}

	input = &(App::instance->input);
	x = input->mouseX;
	y = input->mouseY;

	found = false;
	SDL_LockMutex (widgetListMutex);
	i = widgetList.rbegin ();
	end = widgetList.rend ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed || widget->isInputSuspended || (! widget->isVisible) || (! widget->hasScreenPosition)) {
			continue;
		}

		m = mouseState;
		m.isEntered = false;
		m.enterDeltaX = 0.0f;
		m.enterDeltaY = 0.0f;
		if ((! found) && mouseState.isEntered) {
			if ((widget->width > 0.0f) && (widget->height > 0.0f)) {
				if ((x >= (int) widget->screenX) && (x <= (int) (widget->screenX + widget->width)) && (y >= (int) widget->screenY) && (y <= (int) (widget->screenY + widget->height))) {
					m.isEntered = true;
					m.enterDeltaX = x - widget->screenX;
					m.enterDeltaY = y - widget->screenY;
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

	input = &(App::instance->input);
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

void Panel::doDraw (SDL_Texture *targetTexture, float originX, float originY) {
	SDL_Renderer *render;
	SDL_Rect rect;
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	int x0, y0;
	float w, h;

	render = App::instance->render;
	x0 = (int) (originX + position.x);
	y0 = (int) (originY + position.y);

	if ((! targetTexture) && isTextureRenderEnabled) {
		if (drawTexture) {
			if (shouldRefreshTexture) {
				shouldRefreshTexture = false;
				doDraw (drawTexture, -(position.x), -(position.y));
			}
			rect.x = x0;
			rect.y = y0;
			w = drawTextureWidth;
			h = drawTextureHeight;
			if (! FLOAT_EQUALS (animationScale.x, 1.0f)) {
				w *= animationScale.x;
				h *= animationScale.x;
				rect.x += (int) ((width - w) / 2.0f);
				rect.y += (int) ((height - h) / 2.0f);
			}

			rect.w = (int) w;
			rect.h = (int) h;
			SDL_RenderCopy (render, drawTexture, NULL, &rect);
		}
		return;
	}

	SDL_SetRenderTarget (render, targetTexture);
	rect.x = x0;
	rect.y = y0;
	rect.w = (int) width;
	rect.h = (int) height;
	App::instance->pushClipRect (&rect);

	if (isFilledBg && (bgColor.aByte > 0)) {
		rect.x = x0;
		rect.y = y0;
		rect.w = (int) width;
		rect.h = (int) height;

		SDL_SetRenderTarget (render, targetTexture);
		if (bgColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
		}
		SDL_SetRenderDrawColor (render, bgColor.rByte, bgColor.gByte, bgColor.bByte, bgColor.aByte);
		SDL_RenderFillRect (render, &rect);
		if (bgColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);
		}
		SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
		SDL_SetRenderTarget (render, NULL);
	}

	SDL_LockMutex (widgetListMutex);
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed || (! widget->isVisible)) {
			continue;
		}

		widget->draw (targetTexture, x0 - (int) viewOriginX, y0 - (int) viewOriginY);
	}
	SDL_UnlockMutex (widgetListMutex);

	App::instance->popClipRect ();

	if (isBordered && (borderColor.aByte > 0) && (borderWidth >= 1.0f)) {
		SDL_SetRenderTarget (render, targetTexture);
		if (borderColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
		}
		SDL_SetRenderDrawColor (render, borderColor.rByte, borderColor.gByte, borderColor.bByte, borderColor.aByte);

		rect.x = x0;
		rect.y = y0;
		rect.w = (int) width;
		rect.h = (int) borderWidth;
		SDL_RenderFillRect (render, &rect);

		rect.y = y0 + (int) (height - borderWidth);
		SDL_RenderFillRect (render, &rect);

		rect.y = y0 + (int) borderWidth;
		rect.w = (int) borderWidth;
		rect.h = ((int) height) - (int) (borderWidth * 2.0f);
		SDL_RenderFillRect (render, &rect);

		rect.x = x0 + (int) (width - borderWidth);
		SDL_RenderFillRect (render, &rect);

		if (borderColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);
		}
		SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
		SDL_SetRenderTarget (render, NULL);
	}

	if (isDropShadowed && (dropShadowColor.aByte > 0) && (dropShadowWidth >= 1.0f)) {
		SDL_SetRenderTarget (render, targetTexture);
		App::instance->suspendClipRect ();
		SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor (render, dropShadowColor.rByte, dropShadowColor.gByte, dropShadowColor.bByte, dropShadowColor.aByte);

		rect.x = x0 + (int) width;
		rect.y = y0 + (int) dropShadowWidth;
		rect.w = (int) dropShadowWidth;
		rect.h = (int) height;
		SDL_RenderFillRect (render, &rect);

		rect.x = x0 + (int) dropShadowWidth;
		rect.y = y0 + (int) height;
		rect.w = (int) (width - dropShadowWidth);
		rect.h = (int) dropShadowWidth;
		SDL_RenderFillRect (render, &rect);

		SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
		App::instance->unsuspendClipRect ();
		SDL_SetRenderTarget (render, NULL);
	}
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

	refreshLayout ();
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

void Panel::refreshLayout () {
	UiConfiguration *uiconfig;
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	float x, y, maxw;

	switch (layout) {
		case Panel::VerticalLayout: {
			uiconfig = &(App::instance->uiConfig);
			x = widthPadding;
			y = heightPadding;

			SDL_LockMutex (widgetListMutex);
			i = widgetList.begin ();
			end = widgetList.end ();
			while (i != end) {
				widget = *i;
				++i;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}

				widget->position.assign (x, y);
				y += widget->height + uiconfig->marginSize;
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i = widgetAddList.begin ();
			end = widgetAddList.end ();
			while (i != end) {
				widget = *i;
				++i;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}

				widget->position.assign (x, y);
				y += widget->height + uiconfig->marginSize;
			}
			SDL_UnlockMutex (widgetAddListMutex);
			break;
		}
		case Panel::VerticalRightJustifiedLayout: {
			uiconfig = &(App::instance->uiConfig);
			x = widthPadding;
			y = heightPadding;
			maxw = 0.0f;

			SDL_LockMutex (widgetListMutex);
			i = widgetList.begin ();
			end = widgetList.end ();
			while (i != end) {
				widget = *i;
				++i;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}

				widget->position.assign (x, y);
				y += widget->height + uiconfig->marginSize;
				if (widget->width > maxw) {
					maxw = widget->width;
				}
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i = widgetAddList.begin ();
			end = widgetAddList.end ();
			while (i != end) {
				widget = *i;
				++i;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}

				widget->position.assign (x, y);
				y += widget->height + uiconfig->marginSize;
				if (widget->width > maxw) {
					maxw = widget->width;
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

				widget->position.assignX (x + maxw - widget->width);
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i = widgetAddList.begin ();
			end = widgetAddList.end ();
			while (i != end) {
				widget = *i;
				++i;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}

				widget->position.assignX (x + maxw - widget->width);
			}
			SDL_UnlockMutex (widgetAddListMutex);
			break;
		}
		case Panel::HorizontalLayout: {
			uiconfig = &(App::instance->uiConfig);
			x = widthPadding;
			y = heightPadding;

			SDL_LockMutex (widgetListMutex);
			i = widgetList.begin ();
			end = widgetList.end ();
			while (i != end) {
				widget = *i;
				++i;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}

				widget->position.assign (x, y);
				x += widget->width + uiconfig->marginSize;
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i = widgetAddList.begin ();
			end = widgetAddList.end ();
			while (i != end) {
				widget = *i;
				++i;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}

				widget->position.assign (x, y);
				x += widget->width + uiconfig->marginSize;
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

void Panel::setLayout (int layoutType) {
	if (layout == layoutType) {
		return;
	}
	layout = layoutType;
	refresh ();
}

void Panel::setFillBg (bool enable, const Color &color) {
	if (enable) {
		bgColor.assign (color);
		isFilledBg = true;
	}
	else {
		isFilledBg = false;
	}
}

void Panel::setBorder (bool enable, const Color &color, float borderWidthValue) {
	if (enable) {
		borderColor.assign (color);
		if (borderWidthValue < 1.0f) {
			borderWidthValue = 1.0f;
		}
		borderWidth = borderWidthValue;
		isBordered = true;
	}
	else {
		isBordered = false;
	}
}

void Panel::setDropShadow (bool enable, const Color &color, float dropShadowWidthValue) {
	if (enable) {
		dropShadowColor.assign (color);
		if (dropShadowWidthValue < 1.0f) {
			dropShadowWidthValue = 1.0f;
		}
		dropShadowWidth = dropShadowWidthValue;
		isDropShadowed = true;
	}
	else {
		isDropShadowed = false;
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

	uiconfig = &(App::instance->uiConfig);
	isWaiting = enable;
	if (isWaiting) {
		isInputSuspended = true;

		panel = (Panel *) addWidget (new Panel ());
		panel->setFixedSize (true, width, height);
		panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->waitingShadeAlpha));
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

void Panel::syncRecordStore () {
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

		widget->syncRecordStore ();
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

		widget->syncRecordStore ();
	}
	SDL_UnlockMutex (widgetAddListMutex);
}
