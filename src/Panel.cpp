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
#include <math.h>
#include <list>
#include "SDL2/SDL.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiConfiguration.h"
#include "Input.h"
#include "OsUtil.h"
#include "Widget.h"
#include "ProgressBar.h"
#include "Panel.h"

const int Panel::LongPressDuration = 1000;

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
, topLeftCornerRadius (0)
, topRightCornerRadius (0)
, bottomLeftCornerRadius (0)
, bottomRightCornerRadius (0)
, isBordered (false)
, borderWidth (0.0f)
, isDropShadowed (false)
, dropShadowWidth (0.0f)
, isFixedSize (false)
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
, lastMouseDownTime (0)
, cornerCenterDx (0)
, cornerCenterDy (0)
, cornerCenterDw (0)
, cornerCenterDh (0)
, cornerTopDx (0)
, cornerTopDy (0)
, cornerTopDw (0)
, cornerTopDh (0)
, cornerLeftDx (0)
, cornerLeftDy (0)
, cornerLeftDw (0)
, cornerLeftDh (0)
, cornerRightDx (0)
, cornerRightDy (0)
, cornerRightDw (0)
, cornerRightDh (0)
, cornerBottomDx (0)
, cornerBottomDy (0)
, cornerBottomDw (0)
, cornerBottomDh (0)
, cornerSize (0)
, widgetListMutex (NULL)
, widgetAddListMutex (NULL)
{
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
			bar->setSize (width, UiConfiguration::instance->progressBarHeight);
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
		if ((isTextureRenderEnabled && (! drawTexture)) || ((! isTextureRenderEnabled) && drawTexture) || shouldRefreshTexture) {
			isResettingDrawTexture = true;
			shouldRefreshTexture = false;
			retain ();
			App::instance->addRenderTask (Panel::resetDrawTexture, this);
		}
	}
}

void Panel::processInput () {
	std::list<Widget *>::reverse_iterator i, iend;
	std::vector<SDL_Keycode> keyevents;
	std::vector<SDL_Keycode>::iterator j, jend;
	Widget *widget, *mousewidget;
	Widget::MouseState mousestate;
	float x, y, enterdx, enterdy;
	bool isshiftdown, iscontroldown, isleftdown, isconsumed;

	Input::instance->pollKeyPressEvents (&keyevents);
	isshiftdown = Input::instance->isShiftDown ();
	iscontroldown = Input::instance->isControlDown ();
	isleftdown = (Input::instance->mouseLeftDownCount != Input::instance->mouseLeftUpCount);

	if (isMouseInputStarted) {
		if (Input::instance->mouseLeftDownCount != lastMouseLeftDownCount) {
			mousestate.isLeftClicked = true;
		}
		if (Input::instance->mouseLeftUpCount != lastMouseLeftUpCount) {
			mousestate.isLeftClickReleased = true;
		}
		mousestate.positionDeltaX = Input::instance->mouseX - Input::instance->lastMouseX;
		mousestate.positionDeltaY = Input::instance->mouseY - Input::instance->lastMouseY;
		mousestate.wheelUp = Input::instance->mouseWheelUpCount - lastMouseWheelUpCount;
		mousestate.wheelDown = Input::instance->mouseWheelDownCount - lastMouseWheelDownCount;
	}
	lastMouseLeftUpCount = Input::instance->mouseLeftUpCount;
	lastMouseLeftDownCount = Input::instance->mouseLeftDownCount;
	lastMouseRightUpCount = Input::instance->mouseRightUpCount;
	lastMouseRightDownCount = Input::instance->mouseRightDownCount;
	lastMouseWheelUpCount = Input::instance->mouseWheelUpCount;
	lastMouseWheelDownCount = Input::instance->mouseWheelDownCount;
	isMouseInputStarted = true;

	mousewidget = NULL;
	x = Input::instance->mouseX;
	y = Input::instance->mouseY;
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
			lastMouseDownTime = OsUtil::getTime ();
		}
		else {
			lastMouseDownX = -1;
			lastMouseDownY = -1;
			lastMouseDownTime = 0;
		}
	}

	if (keyEventCallback.callback && (keyevents.size () > 0)) {
		isconsumed = false;
		j = keyevents.begin ();
		jend = keyevents.end ();
		while (j != jend) {
			if (keyEventCallback.callback (keyEventCallback.callbackData, *j, isshiftdown, iscontroldown)) {
				isconsumed = true;
			}
			++j;
		}
		if (isconsumed) {
			keyevents.clear ();
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
			isconsumed = false;
			j = keyevents.begin ();
			jend = keyevents.end ();
			while (j != jend) {
				if (widget->processKeyEvent (*j, isshiftdown, iscontroldown)) {
					isconsumed = true;
				}
				++j;
			}
			if (isconsumed) {
				keyevents.clear ();
			}
		}

		mousestate.isLeftClickEntered = false;
		mousestate.isLongPressed = false;
		if (widget == mousewidget) {
			mousestate.isEntered = true;
			mousestate.enterDeltaX = enterdx;
			mousestate.enterDeltaY = enterdy;

			if (mousestate.isLeftClickReleased) {
				if ((lastMouseDownX >= 0) && (lastMouseDownY >= 0) && (lastMouseDownX >= (int) widget->screenX) && (lastMouseDownX <= (int) (widget->screenX + widget->width)) && (lastMouseDownY >= (int) widget->screenY) && (lastMouseDownY <= (int) (widget->screenY + widget->height))) {
					mousestate.isLeftClickEntered = true;
					lastMouseDownX = -1;
					lastMouseDownY = -1;
				}
			}
			else if (isleftdown) {
				if ((lastMouseDownX >= 0) && (lastMouseDownY >= 0) && (lastMouseDownTime > 0) && (lastMouseDownX >= (int) widget->screenX) && (lastMouseDownX <= (int) (widget->screenX + widget->width)) && (lastMouseDownY >= (int) widget->screenY) && (lastMouseDownY <= (int) (widget->screenY + widget->height)) && ((OsUtil::getTime () - lastMouseDownTime) >= Panel::LongPressDuration)) {
					mousestate.isLongPressed = true;
					lastMouseDownTime = 0;
				}
			}
		}
		else {
			mousestate.isEntered = false;
			mousestate.enterDeltaX = 0.0f;
			mousestate.enterDeltaY = 0.0f;
		}

		if (widget->processMouseState (mousestate)) {
			mousestate.wheelUp = 0;
			mousestate.wheelDown = 0;
		}
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

bool Panel::doProcessMouseState (const Widget::MouseState &mouseState) {
	std::list<Widget *>::reverse_iterator i, end;
	Widget *widget;
	bool found, consumed;
	Widget::MouseState m;
	float x, y;

	if (isTextureRenderEnabled) {
		return (false);
	}
	x = Input::instance->mouseX;
	y = Input::instance->mouseY;
	consumed = false;
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

		if (consumed) {
			m.wheelUp = 0;
			m.wheelDown = 0;
		}
		if (widget->processMouseState (m)) {
			consumed = true;
		}
	}
	SDL_UnlockMutex (widgetListMutex);

	return (consumed);
}

void Panel::doResetInputState () {
	std::list<Widget *>::iterator i, end;
	Widget *widget;

	lastMouseLeftUpCount = Input::instance->mouseLeftUpCount;
	lastMouseLeftDownCount = Input::instance->mouseLeftDownCount;
	lastMouseRightUpCount = Input::instance->mouseRightUpCount;
	lastMouseRightDownCount = Input::instance->mouseRightDownCount;
	lastMouseWheelUpCount = Input::instance->mouseWheelUpCount;
	lastMouseWheelDownCount = Input::instance->mouseWheelDownCount;

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
	SDL_Texture *cornertexture;
	SDL_Rect rect;
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	int x0, y0, texturew, textureh;
	float w, h;

	render = App::instance->render;
	x0 = (int) (originX + position.x);
	y0 = (int) (originY + position.y);

	if ((! targetTexture) && isTextureRenderEnabled) {
		if (drawTexture) {
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
		SDL_SetRenderTarget (render, targetTexture);
		if (bgColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
		}
		SDL_SetRenderDrawColor (render, bgColor.rByte, bgColor.gByte, bgColor.bByte, bgColor.aByte);

		if ((cornerSize > 0) && ((int) width >= cornerSize) && ((int) height >= cornerSize)) {
			if (topLeftCornerRadius > 0) {
				cornertexture = App::instance->getRoundedCornerTexture (topLeftCornerRadius, &texturew, &textureh);
				if (cornertexture) {
					rect.x = x0;
					rect.y = y0;
					rect.w = texturew;
					rect.h = textureh;
					SDL_SetTextureColorMod (cornertexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
					SDL_SetTextureBlendMode (cornertexture, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy (render, cornertexture, NULL, &rect);
				}
			}
			if (topRightCornerRadius > 0) {
				cornertexture = App::instance->getRoundedCornerTexture (topRightCornerRadius, &texturew, &textureh);
				if (cornertexture) {
					rect.x = x0 + (int) width - texturew;
					rect.y = y0;
					rect.w = texturew;
					rect.h = textureh;
					SDL_SetTextureColorMod (cornertexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
					SDL_SetTextureBlendMode (cornertexture, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy (render, cornertexture, NULL, &rect);
				}
			}
			if (bottomLeftCornerRadius > 0) {
				cornertexture = App::instance->getRoundedCornerTexture (bottomLeftCornerRadius, &texturew, &textureh);
				if (cornertexture) {
					rect.x = x0;
					rect.y = y0 + (int) height - textureh;
					rect.w = texturew;
					rect.h = textureh;
					SDL_SetTextureColorMod (cornertexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
					SDL_SetTextureBlendMode (cornertexture, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy (render, cornertexture, NULL, &rect);
				}
			}
			if (bottomRightCornerRadius > 0) {
				cornertexture = App::instance->getRoundedCornerTexture (bottomRightCornerRadius, &texturew, &textureh);
				if (cornertexture) {
					rect.x = x0 + (int) width - texturew;
					rect.y = y0 + (int) height - textureh;
					rect.w = texturew;
					rect.h = textureh;
					SDL_SetTextureColorMod (cornertexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
					SDL_SetTextureBlendMode (cornertexture, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy (render, cornertexture, NULL, &rect);
				}
			}

			rect.x = x0;
			rect.y = y0;
			rect.w = (int) width;
			rect.h = (int) height;
			rect.x += cornerCenterDx;
			rect.y += cornerCenterDy;
			rect.w += cornerCenterDw;
			rect.h += cornerCenterDh;
			SDL_RenderFillRect (render, &rect);

			if (cornerTopDh > 0) {
				rect.x = x0 + cornerTopDx;
				rect.y = y0 + cornerTopDy;
				rect.w = ((int) width) + cornerTopDw;
				rect.h = cornerTopDh;
				SDL_RenderFillRect (render, &rect);
			}
			if (cornerLeftDw > 0) {
				rect.x = x0 + cornerLeftDx;
				rect.y = y0 + cornerLeftDy;
				rect.w = cornerLeftDw;
				rect.h = ((int) height) + cornerLeftDh;
				SDL_RenderFillRect (render, &rect);
			}
			if (cornerRightDw > 0) {
				rect.x = x0 + width + cornerRightDx;
				rect.y = y0 + cornerRightDy;
				rect.w = cornerRightDw;
				rect.h = ((int) height) + cornerRightDh;
				SDL_RenderFillRect (render, &rect);
			}
			if (cornerBottomDh > 0) {
				rect.x = x0 + cornerBottomDx;
				rect.y = y0 + height + cornerBottomDy;
				rect.w = ((int) width) + cornerBottomDw;
				rect.h = cornerBottomDh;
				SDL_RenderFillRect (render, &rect);
			}
		}
		else {
			rect.x = x0;
			rect.y = y0;
			rect.w = (int) width;
			rect.h = (int) height;
			SDL_RenderFillRect (render, &rect);
		}

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
	App::instance->popClipRect ();

	if (isDropShadowed && (dropShadowColor.aByte > 0) && (dropShadowWidth >= 1.0f)) {
		SDL_SetRenderTarget (render, targetTexture);
		SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor (render, dropShadowColor.rByte, dropShadowColor.gByte, dropShadowColor.bByte, dropShadowColor.aByte);

		rect.x = App::instance->clipRect.x;
		rect.y = App::instance->clipRect.y;
		rect.w = App::instance->clipRect.w + dropShadowWidth;
		rect.h = App::instance->clipRect.h + dropShadowWidth;
		App::instance->pushClipRect (&rect, true);

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

		App::instance->popClipRect ();

		SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
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

	SDL_LockMutex (widgetAddListMutex);
	i = widgetAddList.begin ();
	end = widgetAddList.end ();
	while (i != end) {
		widget = *i;
		++i;
		if (widget->isDestroyed) {
			continue;
		}
		widget->refresh ();
	}
	SDL_UnlockMutex (widgetAddListMutex);

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
		if (widget->isDestroyed || (! widget->isVisible) || widget->isPanelSizeClipEnabled) {
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
		if (widget->isDestroyed || (! widget->isVisible) || widget->isPanelSizeClipEnabled) {
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
	std::list<Widget *>::iterator i, end;
	Widget *widget;
	float x, y, maxw, maxh;

	switch (layout) {
		case Panel::VerticalLayout: {
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
				y += widget->height + UiConfiguration::instance->marginSize;
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
				y += widget->height + UiConfiguration::instance->marginSize;
			}
			SDL_UnlockMutex (widgetAddListMutex);
			break;
		}
		case Panel::VerticalRightJustifiedLayout: {
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
				y += widget->height + UiConfiguration::instance->marginSize;
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
				y += widget->height + UiConfiguration::instance->marginSize;
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
				x += widget->width + UiConfiguration::instance->marginSize;
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
				x += widget->width + UiConfiguration::instance->marginSize;
			}
			SDL_UnlockMutex (widgetAddListMutex);
			break;
		}
		case Panel::HorizontalVcenteredLayout: {
			x = widthPadding;
			y = heightPadding;
			maxh = 0.0f;

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
				x += widget->width + UiConfiguration::instance->marginSize;
				if (widget->height > maxh) {
					maxh = widget->height;
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
				x += widget->width + UiConfiguration::instance->marginSize;
				if (widget->height > maxh) {
					maxh = widget->height;
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
				widget->position.assignY (y + ((maxh - widget->height) / 2.0f));
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
				widget->position.assignY (y + ((maxh - widget->height) / 2.0f));
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
	minlevel = Widget::MinZLevel - 1;
	maxlevel = Widget::MinZLevel - 1;
	i = widgetList.begin ();
	end = widgetList.end ();
	while (i != end) {
		widget = *i;
		if (minlevel < Widget::MinZLevel) {
			minlevel = widget->zLevel;
		}
		if (maxlevel < Widget::MinZLevel) {
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

void Panel::setLayout (int layoutType, float maxPanelWidth) {
	if (layout == layoutType) {
		return;
	}
	// Default implementation ignores the maxPanelWidth parameter
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

void Panel::setCornerRadius (int radius) {
	setCornerRadius (radius, radius, radius, radius);
}

void Panel::setCornerRadius (int topLeftRadius, int topRightRadius, int bottomLeftRadius, int bottomRightRadius) {
	int centerx, centery, centerw, centerh, topx, topy, topw, toph, leftx, lefty, leftw, lefth, rightx, righty, rightw, righth, bottomx, bottomy, bottomw, bottomh, amt;

	if (topLeftRadius < 0) {
		topLeftRadius = 0;
	}
	if (topLeftRadius > App::MaxCornerRadius) {
		topLeftRadius = App::MaxCornerRadius;
	}
	if (topRightRadius < 0) {
		topRightRadius = 0;
	}
	if (topRightRadius > App::MaxCornerRadius) {
		topRightRadius = App::MaxCornerRadius;
	}
	if (bottomLeftRadius < 0) {
		bottomLeftRadius = 0;
	}
	if (bottomLeftRadius > App::MaxCornerRadius) {
		bottomLeftRadius = App::MaxCornerRadius;
	}
	if (bottomRightRadius < 0) {
		bottomRightRadius = 0;
	}
	if (bottomRightRadius > App::MaxCornerRadius) {
		bottomRightRadius = App::MaxCornerRadius;
	}
	if ((topLeftRadius <= 0) && (topRightRadius <= 0) && (bottomLeftRadius <= 0) && (bottomRightRadius <= 0)) {
		cornerSize = 0;
		return;
	}

	centerx = 0;
	centery = 0;
	centerw = 0;
	centerh = 0;
	topx = 0;
	topy = 0;
	topw = 0;
	toph = 0;
	leftx = 0;
	lefty = 0;
	leftw = 0;
	lefth = 0;
	rightx = 0;
	righty = 0;
	rightw = 0;
	righth = 0;
	bottomx = 0;
	bottomy = 0;
	bottomw = 0;
	bottomh = 0;

	if ((topLeftRadius > 0) || (topRightRadius > 0)) {
		amt = (topLeftRadius > topRightRadius) ? topLeftRadius : topRightRadius;
		centery += amt;
		centerh -= amt;
		toph = amt;
		if (topLeftRadius > 0) {
			topx = topLeftRadius;
			topw -= topLeftRadius;
		}
		if (topRightRadius > 0) {
			topw -= topRightRadius;
		}
	}
	if ((topLeftRadius > 0) || (bottomLeftRadius > 0)) {
		amt = (topLeftRadius > bottomLeftRadius) ? topLeftRadius : bottomLeftRadius;
		centerx += amt;
		centerw -= amt;
		leftw = amt;
		if (topLeftRadius > 0) {
			lefty = topLeftRadius;
			lefth -= topLeftRadius;
		}
		if (bottomLeftRadius > 0) {
			lefth -= bottomLeftRadius;
		}
	}
	if ((topRightRadius > 0) || (bottomRightRadius > 0)) {
		amt = (topRightRadius > bottomRightRadius) ? topRightRadius : bottomRightRadius;
		centerw -= amt;
		rightx -= amt;
		rightw = amt;
		if (topRightRadius > 0) {
			righty = topRightRadius;
			righth -= topRightRadius;
		}
		if (bottomRightRadius > 0) {
			righth -= bottomRightRadius;
		}
	}
	if ((bottomLeftRadius > 0) || (bottomRightRadius > 0)) {
		amt = (bottomLeftRadius > bottomRightRadius) ? bottomLeftRadius : bottomRightRadius;
		centerh -= amt;
		bottomy -= amt;
		bottomh = amt;
		if (bottomLeftRadius > 0) {
			bottomx = bottomLeftRadius;
			bottomw -= bottomLeftRadius;
		}
		if (bottomRightRadius > 0) {
			bottomw -= bottomRightRadius;
		}
	}

	if ((topLeftRadius <= 0) && (leftw > 0) && (toph > 0)) {
		if (topRightRadius > 0) {
			lefty += topRightRadius;
			lefth -= topRightRadius;
		}
		else if (bottomLeftRadius > 0) {
			topx += bottomLeftRadius;
			topw -= bottomLeftRadius;
		}
	}
	if ((topRightRadius <= 0) && (rightw > 0) && (toph > 0)) {
		if (topLeftRadius > 0) {
			righty += topLeftRadius;
			righth -= topLeftRadius;
		}
		else if (bottomRightRadius > 0) {
			topw -= bottomRightRadius;
		}
	}
	if ((bottomLeftRadius <= 0) && (leftw > 0) && (bottomh > 0)) {
		if (topLeftRadius > 0) {
			bottomx += topLeftRadius;
			bottomw -= topLeftRadius;
		}
		else if (bottomRightRadius > 0) {
			lefth -= bottomRightRadius;
		}
	}
	if ((bottomRightRadius <= 0) && (rightw > 0) && (bottomh > 0)) {
		if (topRightRadius > 0) {
			bottomw -= topRightRadius;
		}
		else if (bottomLeftRadius > 0) {
			righty += bottomLeftRadius;
			righth -= bottomLeftRadius;
		}
	}

	cornerCenterDx = centerx;
	cornerCenterDy = centery;
	cornerCenterDw = centerw;
	cornerCenterDh = centerh;
	cornerTopDx = topx;
	cornerTopDy = topy;
	cornerTopDw = topw;
	cornerTopDh = toph;
	cornerLeftDx = leftx;
	cornerLeftDy = lefty;
	cornerLeftDw = leftw;
	cornerLeftDh = lefth;
	cornerRightDx = rightx;
	cornerRightDy = righty;
	cornerRightDw = rightw;
	cornerRightDh = righth;
	cornerBottomDx = bottomx;
	cornerBottomDy = bottomy;
	cornerBottomDw = bottomw;
	cornerBottomDh = bottomh;
	topLeftCornerRadius = topLeftRadius;
	topRightCornerRadius = topRightRadius;
	bottomLeftCornerRadius = bottomLeftRadius;
	bottomRightCornerRadius = bottomRightRadius;

	amt = topLeftRadius;
	if (topRightRadius > amt) {
		amt = topRightRadius;
	}
	if (bottomLeftRadius > amt) {
		amt = bottomLeftRadius;
	}
	if (bottomRightRadius > amt) {
		amt = bottomRightRadius;
	}
	cornerSize = amt * 2;
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

void Panel::setWaiting (bool enable) {
	Panel *panel;
	ProgressBar *bar;

	if (isWaiting == enable) {
		return;
	}
	isWaiting = enable;
	if (isWaiting) {
		isInputSuspended = true;

		panel = (Panel *) addWidget (new Panel ());
		panel->setFixedSize (true, width, height);
		panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->waitingShadeAlpha));
		panel->zLevel = maxWidgetZLevel + 1;

		bar = (ProgressBar *) panel->addWidget (new ProgressBar (width, UiConfiguration::instance->progressBarHeight));
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
