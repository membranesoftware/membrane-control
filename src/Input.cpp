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
#include <vector>
#include "SDL2/SDL.h"
#include "App.h"
#include "OsUtil.h"
#include "Input.h"

Input *Input::instance = NULL;

Input::Input ()
: keyRepeatStartThreshold (680)
, keyRepeatDelay (48)
, mouseX (-1)
, mouseY (-1)
, lastMouseX (-1)
, lastMouseY (-1)
, isMouseLeftButtonDown (false)
, isMouseRightButtonDown (false)
, mouseLeftDownCount (0)
, mouseRightDownCount (0)
, mouseLeftUpCount (0)
, mouseRightUpCount (0)
, mouseWheelDownCount (0)
, mouseWheelUpCount (0)
, windowCloseCount (0)
, isKeyPressListPopulated (false)
, isKeyRepeating (false)
, keyRepeatCode (SDLK_UNKNOWN)
, keyRepeatStartTime (0)
{
	keyPressListMutex = SDL_CreateMutex ();
}

Input::~Input () {
	if (keyPressListMutex) {
		SDL_DestroyMutex (keyPressListMutex);
		keyPressListMutex = NULL;
	}
}

OsUtil::Result Input::start () {
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_a, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_b, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_c, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_d, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_e, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_f, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_g, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_h, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_i, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_j, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_k, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_l, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_m, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_n, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_o, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_p, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_q, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_r, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_s, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_t, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_u, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_v, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_w, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_x, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_y, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_z, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_0, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_1, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_2, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_3, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_4, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_5, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_6, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_7, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_8, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_9, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_SPACE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_SLASH, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_COMMA, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_PERIOD, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_MINUS, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_EQUALS, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LEFTBRACKET, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RIGHTBRACKET, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_SEMICOLON, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_QUOTE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_BACKSLASH, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_TAB, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LEFT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_UP, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_DOWN, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RIGHT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LSHIFT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RSHIFT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LCTRL, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RCTRL, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_ESCAPE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_BACKSPACE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RETURN, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_PAGEUP, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_PAGEDOWN, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_HOME, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_END, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_INSERT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_DELETE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F1, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F2, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F3, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F4, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F5, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F6, false));
#if PLATFORM_MACOS
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LGUI, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RGUI, false));
#endif
	return (OsUtil::Success);
}

void Input::stop () {
	keyDownMap.clear ();
}

void Input::pollEvents () {
	std::map<SDL_Keycode, bool>::iterator i;
	SDL_Event event;
	int64_t now;

	now = OsUtil::getTime ();
	while (SDL_PollEvent (&event)) {
		switch (event.type) {
			case SDL_KEYDOWN: {
				i = keyDownMap.find (event.key.keysym.sym);
				if (i == keyDownMap.end ()) {
					break;
				}
				if (! i->second) {
					i->second = true;
					isKeyRepeating = false;
					keyRepeatStartTime = now;
					keyRepeatCode = event.key.keysym.sym;

					SDL_LockMutex (keyPressListMutex);
					keyPressList.push_back (event.key.keysym.sym);
					isKeyPressListPopulated = true;
					SDL_UnlockMutex (keyPressListMutex);
				}
				break;
			}
			case SDL_KEYUP: {
				i = keyDownMap.find (event.key.keysym.sym);
				if (i == keyDownMap.end ()) {
					break;
				}
				i->second = false;
				if (event.key.keysym.sym == keyRepeatCode) {
					isKeyRepeating = false;
					keyRepeatCode = SDLK_UNKNOWN;
				}
				break;
			}
			case SDL_MOUSEBUTTONDOWN: {
				if (event.button.button == SDL_BUTTON_LEFT) {
					isMouseLeftButtonDown = true;
					++mouseLeftDownCount;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT) {
					isMouseRightButtonDown = true;
					++mouseRightDownCount;
				}
				break;
			}
			case SDL_MOUSEBUTTONUP: {
				if (event.button.button == SDL_BUTTON_LEFT) {
					isMouseLeftButtonDown = false;
					++mouseLeftUpCount;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT) {
					isMouseRightButtonDown = false;
					++mouseRightUpCount;
				}
				break;
			}
			case SDL_MOUSEWHEEL: {
				if (event.wheel.direction == SDL_MOUSEWHEEL_NORMAL) {
					if (event.wheel.y < 0) {
						++mouseWheelDownCount;
					}
					else if (event.wheel.y > 0) {
						++mouseWheelUpCount;
					}
				}
				else {
					if (event.wheel.y < 0) {
						++mouseWheelUpCount;
					}
					else if (event.wheel.y > 0) {
						++mouseWheelDownCount;
					}
				}
				break;
			}
			case SDL_WINDOWEVENT: {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					++windowCloseCount;
				}
				break;
			}
		}
	}

	if (keyRepeatCode == SDLK_UNKNOWN) {
		isKeyRepeating = false;
	}
	else {
		if (! isKeyRepeating) {
			if (((now - keyRepeatStartTime) >= keyRepeatStartThreshold) && isKeyDown (keyRepeatCode)) {
				isKeyRepeating = true;
				keyRepeatStartTime = 0;
			}
		}
		if (isKeyRepeating) {
			if ((keyRepeatStartTime <= 0) || ((now - keyRepeatStartTime) >= keyRepeatDelay)) {
				keyRepeatStartTime = now;
				SDL_LockMutex (keyPressListMutex);
				keyPressList.push_back (keyRepeatCode);
				isKeyPressListPopulated = true;
				SDL_UnlockMutex (keyPressListMutex);
			}
		}
	}

	lastMouseX = mouseX;
	lastMouseY = mouseY;
	SDL_GetMouseState (&mouseX, &mouseY);
}

bool Input::isKeyDown (SDL_Keycode keycode) {
	std::map<SDL_Keycode, bool>::iterator i;

	i = keyDownMap.find (keycode);
	if (i == keyDownMap.end ()) {
		return (false);
	}
	return (i->second);
}

bool Input::isShiftDown () {
	return (isKeyDown (SDLK_LSHIFT) || isKeyDown (SDLK_RSHIFT));
}

bool Input::isControlDown () {
	if (isKeyDown (SDLK_LCTRL) || isKeyDown (SDLK_RCTRL)) {
		return (true);
	}

#if PLATFORM_MACOS
	if (isKeyDown (SDLK_LGUI) || isKeyDown (SDLK_RGUI)) {
		return (true);
	}
#endif

	return (false);
}

void Input::pollKeyPressEvents (std::vector<SDL_Keycode> *destVector) {
	std::vector<SDL_Keycode>::iterator i, end;

	if (! isKeyPressListPopulated) {
		return;
	}
	SDL_LockMutex (keyPressListMutex);
	i = keyPressList.begin ();
	end = keyPressList.end ();
	while (i != end) {
		destVector->push_back (*i);
		++i;
	}
	keyPressList.clear ();
	isKeyPressListPopulated = false;
	SDL_UnlockMutex (keyPressListMutex);
}

void Input::windowClose () {
	++windowCloseCount;
}

char Input::getKeyCharacter (SDL_Keycode keycode, bool isShiftDown) {
	// TODO: Possibly use a different character map (i.e. for a non-US keyboard layout)
	switch (keycode) {
		case SDLK_a: { return (isShiftDown ? 'A' : 'a'); }
		case SDLK_b: { return (isShiftDown ? 'B' : 'b'); }
		case SDLK_c: { return (isShiftDown ? 'C' : 'c'); }
		case SDLK_d: { return (isShiftDown ? 'D' : 'd'); }
		case SDLK_e: { return (isShiftDown ? 'E' : 'e'); }
		case SDLK_f: { return (isShiftDown ? 'F' : 'f'); }
		case SDLK_g: { return (isShiftDown ? 'G' : 'g'); }
		case SDLK_h: { return (isShiftDown ? 'H' : 'h'); }
		case SDLK_i: { return (isShiftDown ? 'I' : 'i'); }
		case SDLK_j: { return (isShiftDown ? 'J' : 'j'); }
		case SDLK_k: { return (isShiftDown ? 'K' : 'k'); }
		case SDLK_l: { return (isShiftDown ? 'L' : 'l'); }
		case SDLK_m: { return (isShiftDown ? 'M' : 'm'); }
		case SDLK_n: { return (isShiftDown ? 'N' : 'n'); }
		case SDLK_o: { return (isShiftDown ? 'O' : 'o'); }
		case SDLK_p: { return (isShiftDown ? 'P' : 'p'); }
		case SDLK_q: { return (isShiftDown ? 'Q' : 'q'); }
		case SDLK_r: { return (isShiftDown ? 'R' : 'r'); }
		case SDLK_s: { return (isShiftDown ? 'S' : 's'); }
		case SDLK_t: { return (isShiftDown ? 'T' : 't'); }
		case SDLK_u: { return (isShiftDown ? 'U' : 'u'); }
		case SDLK_v: { return (isShiftDown ? 'V' : 'v'); }
		case SDLK_w: { return (isShiftDown ? 'W' : 'w'); }
		case SDLK_x: { return (isShiftDown ? 'X' : 'x'); }
		case SDLK_y: { return (isShiftDown ? 'Y' : 'y'); }
		case SDLK_z: { return (isShiftDown ? 'Z' : 'z'); }
		case SDLK_0: { return (isShiftDown ? ')' : '0'); }
		case SDLK_1: { return (isShiftDown ? '!' : '1'); }
		case SDLK_2: { return (isShiftDown ? '@' : '2'); }
		case SDLK_3: { return (isShiftDown ? '#' : '3'); }
		case SDLK_4: { return (isShiftDown ? '$' : '4'); }
		case SDLK_5: { return (isShiftDown ? '%' : '5'); }
		case SDLK_6: { return (isShiftDown ? '^' : '6'); }
		case SDLK_7: { return (isShiftDown ? '&' : '7'); }
		case SDLK_8: { return (isShiftDown ? '*' : '8'); }
		case SDLK_9: { return (isShiftDown ? '(' : '9'); }
		case SDLK_SPACE: { return (' '); }
		case SDLK_COMMA: { return (isShiftDown ? '<' : ','); }
		case SDLK_PERIOD: { return (isShiftDown ? '>' : '.'); }
		case SDLK_SLASH: { return (isShiftDown ? '?' : '/'); }
		case SDLK_MINUS: { return (isShiftDown ? '_' : '-'); }
		case SDLK_EQUALS: { return (isShiftDown ? '+' : '='); }
		case SDLK_LEFTBRACKET: { return (isShiftDown ? '{' : '['); }
		case SDLK_RIGHTBRACKET: { return (isShiftDown ? '}' : ']'); }
		case SDLK_SEMICOLON: { return (isShiftDown ? ':' : ';'); }
		case SDLK_QUOTE: { return (isShiftDown ? '"' : '\''); }
		case SDLK_BACKSLASH: { return (isShiftDown ? '|' : '\\'); }
	}
	return (0);
}
