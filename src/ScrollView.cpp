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
#include "Log.h"
#include "StdString.h"
#include "Ui.h"
#include "Input.h"
#include "Sprite.h"
#include "Widget.h"
#include "Label.h"
#include "Panel.h"
#include "UiConfiguration.h"
#include "ScrollView.h"

ScrollView::ScrollView (float viewWidth, float viewHeight)
: Panel ()
, isKeyboardScrollEnabled (false)
, verticalScrollSpeed (0.0f)
{
	UiConfiguration *uiconfig;

	widgetType.assign ("ScrollView");

	uiconfig = &(App::getInstance ()->uiConfig);
	setFixedSize (true, viewWidth, viewHeight);
	setVerticalScrollSpeed (height * uiconfig->mouseWheelScrollSpeed);
}

ScrollView::~ScrollView () {

}

void ScrollView::setVerticalScrollSpeed (float speed) {
	verticalScrollSpeed = speed;
	if (verticalScrollSpeed < 1.0f) {
		verticalScrollSpeed = 1.0f;
	}
	if (verticalScrollSpeed > height) {
		verticalScrollSpeed = height;
	}
}

void ScrollView::doProcessMouseState (const Widget::MouseState &mouseState) {
	float dy, delta;

	Panel::doProcessMouseState (mouseState);

	delta = 0.0f;
	if (mouseState.wheelUp > 0) {
		dy = ((float) mouseState.wheelUp) * verticalScrollSpeed * -1.0f;
		if (dy > -1.0f) {
			dy = -1.0f;
		}
		delta += dy;
	}
	if (mouseState.wheelDown > 0) {
		dy = ((float) mouseState.wheelDown) * verticalScrollSpeed;
		if (dy < 1.0f) {
			dy = 1.0f;
		}
		delta += dy;
	}
	if (fabs (delta) > 0.0f) {
		setViewOrigin (0.0f, viewOriginY + delta);
	}
}

void ScrollView::setVerticalScrollBounds (float minY, float maxY) {
	setViewOriginBounds (0.0f, 0.0f, minY, maxY);
}

bool ScrollView::isScrolledToBottom () {
	if (! isViewOriginBoundEnabled) {
		return (false);
	}

	return (viewOriginY >= maxViewOriginY);
}

bool ScrollView::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (Panel::doProcessKeyEvent (keycode, isShiftDown, isControlDown)) {
		return (true);
	}

	if (isKeyboardScrollEnabled) {
		switch (keycode) {
			case SDLK_UP: {
				setViewOrigin (0.0f, viewOriginY - verticalScrollSpeed);
				return (true);
			}
			case SDLK_DOWN: {
				setViewOrigin (0.0f, viewOriginY + verticalScrollSpeed);
				return (true);
			}
			case SDLK_HOME: {
				setViewOrigin (0.0f, 0.0f);
				return (true);
			}
			case SDLK_END: {
				if (isViewOriginBoundEnabled) {
					setViewOrigin (0.0f, maxViewOriginY);
					return (true);
				}
				break;
			}
			case SDLK_PAGEUP: {
				setViewOrigin (0.0f, viewOriginY - height);
				return (true);
			}
			case SDLK_PAGEDOWN: {
				setViewOrigin (0.0f, viewOriginY + height);
				return (true);
			}
		}
	}

	return (false);
}
