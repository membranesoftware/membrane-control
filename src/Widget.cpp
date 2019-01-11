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
#include "SDL2/SDL.h"
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "Input.h"
#include "UiConfiguration.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "Widget.h"

const int Widget::minZLevel = -10;

Widget::Widget ()
: widgetType ("")
, id (0)
, isDestroyed (false)
, isVisible (true)
, isInputSuspended (false)
, zLevel (0)
, isFixedPosition (false)
, isMouseHoverEnabled (false)
, isDrawable (false)
, drawX (0.0f)
, drawY (0.0f)
, tooltipAlignment (Widget::BOTTOM)
, width (0.0f)
, height (0.0f)
, destroyClock (0)
, isFixedCenter (false)
, isMouseEntered (false)
, isMousePressed (false)
, mouseEnterCallback (NULL)
, mouseEnterCallbackData (NULL)
, mouseExitCallback (NULL)
, mouseExitCallbackData (NULL)
, mousePressCallback (NULL)
, mousePressCallbackData (NULL)
, mouseReleaseCallback (NULL)
, mouseReleaseCallbackData (NULL)
, mouseClickCallback (NULL)
, mouseClickCallbackData (NULL)
, keyEventCallback (NULL)
, keyEventCallbackData (NULL)
, updateCallback (NULL)
, refcount (0)
, refcountMutex (NULL)
{
	refcountMutex = SDL_CreateMutex ();
}

Widget::~Widget () {
	if (refcountMutex) {
		SDL_DestroyMutex (refcountMutex);
		refcountMutex = NULL;
	}
}

void Widget::retain () {

	SDL_LockMutex (refcountMutex);
	++refcount;
	if (refcount < 1) {
		refcount = 1;
	}
	SDL_UnlockMutex (refcountMutex);
}

void Widget::release () {
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

void Widget::resetInputState () {
	isMouseEntered = false;
	isMousePressed = false;

	doResetInputState ();
}

void Widget::doResetInputState () {
	// Default implementation does nothing
}

void Widget::setMouseHoverTooltip (const StdString &text, int alignment) {
	tooltipText.assign (text);
	tooltipAlignment = alignment;
	isMouseHoverEnabled = true;
}

void Widget::update (int msElapsed, float originX, float originY) {
	float x, y;

#if ENABLE_TEST_KEYS
	if (App::getInstance ()->isUiPaused) {
		return;
	}
#endif

	if (destroyClock > 0) {
		destroyClock -= msElapsed;
		if (destroyClock <= 0) {
			isDestroyed = true;
		}
	}
	if (isDestroyed) {
		return;
	}

	position.update (msElapsed);
	drawX = position.x;
	drawY = position.y;
	if (! isFixedPosition) {
		drawX += originX;
		drawY += originY;
	}
	isDrawable = true;

	doUpdate (msElapsed, originX, originY);

	if (isFixedCenter) {
		fixedCenterPosition.update (msElapsed);
		x = fixedCenterPosition.x - (width / 2.0f);
		y = fixedCenterPosition.y - (height / 2.0f);

		// TODO: Possibly use a smooth translation here
		if (! position.equals (x, y)) {
			position.assign (x, y);
			drawX = originX + position.x;
			drawY = originY + position.y;
		}
	}

	if (updateCallback) {
		updateCallback (msElapsed, this);
	}
}

void Widget::doUpdate (int msElapsed, float originX, float originY) {
	// Default implementation does nothing
}

void Widget::draw () {
	if (isDestroyed) {
		return;
	}
	doDraw ();
}

void Widget::doDraw () {
	// Default implementation does nothing
}

void Widget::refresh () {
	// Superclass method takes no action
	doRefresh ();
}

void Widget::doRefresh () {
	// Default implementation does nothing
}

StdString Widget::toString () {
	return (StdString::createSprintf ("<%s #%llu%s>", widgetType.c_str (), (unsigned long long) id, toStringDetail ().c_str ()));
}

StdString Widget::toStringDetail () {
  return (StdString (""));
}

void Widget::setDestroyDelay (int delayMs) {
	if (delayMs <= 0) {
		isDestroyed = true;
		return;
	}
	destroyClock = delayMs;
}

void Widget::setFixedCenter (bool enable) {
	isFixedCenter = enable;
	if (isFixedCenter) {
		fixedCenterPosition.assign (position.x + (width / 2.0f), position.y + (height / 2.0f));
	}
}

void Widget::syncRecordStore (RecordStore *store) {
	// Default implementation does nothing
}

void Widget::destroyAllChildWidgets () {
	// Default implementation does nothing
}

Widget *Widget::findMouseHoverWidget (float mouseX, float mouseY) {
	// Default implementation returns NULL
	return (NULL);
}

bool Widget::processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
#if ENABLE_TEST_KEYS
	if (App::getInstance ()->isUiPaused) {
		return (false);
	}
#endif

	if (isInputSuspended) {
		return (false);
	}

	if (keyEventCallback && keyEventCallback (keyEventCallbackData, keycode, isShiftDown, isControlDown)) {
		return (true);
	}

	return (doProcessKeyEvent (keycode, isShiftDown, isControlDown));
}

bool Widget::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	// Default implementation does nothing
	return (false);
}

void Widget::processMouseState (const Widget::MouseState &mouseState) {
#if ENABLE_TEST_KEYS
	if (App::getInstance ()->isUiPaused) {
		return;
	}
#endif

	if (isInputSuspended) {
		return;
	}

	if (mouseState.isEntered) {
		if (! isMouseEntered) {
			isMouseEntered = true;
			if (mouseEnterCallback) {
				mouseEnterCallback (mouseEnterCallbackData, this);
			}
		}

		if (mouseState.isLeftClicked) {
			if (! isMousePressed) {
				isMousePressed = true;
				if (mousePressCallback) {
					mousePressCallback (mousePressCallbackData, this);
				}
			}
		}

		if (isMousePressed && mouseState.isLeftClickReleased) {
			isMousePressed = false;
			if (mouseReleaseCallback) {
				mouseReleaseCallback (mouseReleaseCallbackData, this);
			}

			if (mouseState.isLeftClickEntered) {
				if (mouseClickCallback) {
					mouseClickCallback (mouseClickCallbackData, this);
				}
			}
		}
	}
	else {
		if (isMousePressed) {
			isMousePressed = false;
			if (mouseReleaseCallback) {
				mouseReleaseCallback (mouseReleaseCallbackData, this);
			}
		}

		if (isMouseEntered) {
			isMouseEntered = false;
			if (mouseExitCallback) {
				mouseExitCallback (mouseExitCallbackData, this);
			}
		}
	}

	doProcessMouseState (mouseState);
}

void Widget::doProcessMouseState (const Widget::MouseState &mouseState) {
	// Default implementation does nothing
}

void Widget::setMouseEnterCallback (Widget::EventCallback fn, void *data) {
	mouseEnterCallback = fn;
	mouseEnterCallbackData = data;
}

void Widget::setMouseExitCallback (Widget::EventCallback fn, void *data) {
	mouseExitCallback = fn;
	mouseExitCallbackData = data;
}

void Widget::setMousePressCallback (Widget::EventCallback fn, void *data) {
	mousePressCallback = fn;
	mousePressCallbackData = data;
}

void Widget::setMouseReleaseCallback (Widget::EventCallback fn, void *data) {
	mouseReleaseCallback = fn;
	mouseReleaseCallbackData = data;
}

void Widget::setMouseClickCallback (Widget::EventCallback fn, void *data) {
	mouseClickCallback = fn;
	mouseClickCallbackData = data;
}

void Widget::setKeyEventCallback (Widget::KeyEventCallback fn, void *data) {
	keyEventCallback = fn;
	keyEventCallbackData = data;
}

void Widget::setUpdateCallback (Widget::UpdateCallback fn) {
	updateCallback = fn;
}

bool Widget::compareZLevel (Widget *first, Widget *second) {
	return (first->zLevel < second->zLevel);
}

void Widget::mouseEnter () {
	if (mouseEnterCallback) {
		mouseEnterCallback (mouseEnterCallbackData, this);
	}
}

void Widget::mouseExit () {
	if (mouseExitCallback) {
		mouseExitCallback (mouseExitCallbackData, this);
	}
}

void Widget::mousePress () {
	if (mousePressCallback) {
		mousePressCallback (mousePressCallbackData, this);
	}
}

void Widget::mouseRelease () {
	if (mouseReleaseCallback) {
		mouseReleaseCallback (mouseReleaseCallbackData, this);
	}
}

void Widget::mouseClick () {
	if (mouseClickCallback) {
		mouseClickCallback (mouseClickCallbackData, this);
	}
}

void Widget::flowRight (float *positionX, float positionY, float *rightExtent, float *bottomExtent) {
	UiConfiguration *uiconfig;
	float pos;

	uiconfig = &(App::getInstance ()->uiConfig);
	position.assign (*positionX, positionY);
	*positionX += width + uiconfig->marginSize;
	if (rightExtent) {
		pos = position.x + width;
		if (pos > *rightExtent) {
			*rightExtent = pos;
		}
	}
	if (bottomExtent) {
		pos = position.y + height;
		if (pos > *bottomExtent) {
			*bottomExtent = pos;
		}
	}
}

void Widget::flowDown (float positionX, float *positionY, float *rightExtent, float *bottomExtent) {
	UiConfiguration *uiconfig;
	float pos;

	uiconfig = &(App::getInstance ()->uiConfig);
	position.assign (positionX, *positionY);
	*positionY += height + uiconfig->marginSize;
	if (rightExtent) {
		pos = position.x + width;
		if (pos > *rightExtent) {
			*rightExtent = pos;
		}
	}
	if (bottomExtent) {
		pos = position.y + height;
		if (pos > *bottomExtent) {
			*bottomExtent = pos;
		}
	}
}

void Widget::flowLeft (float *positionX) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	*positionX -= width;
	position.assignX (*positionX);
	*positionX -= uiconfig->marginSize;
}

void Widget::centerVertical (float topExtent, float bottomExtent) {
	position.assignY (topExtent + ((bottomExtent - topExtent) / 2.0f) - (height / 2.0f));
}
