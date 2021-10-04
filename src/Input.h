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
// Class that tracks state of user input devices

#ifndef INPUT_H
#define INPUT_H

#include <map>
#include <vector>
#include "SDL2/SDL.h"
#include "OsUtil.h"

class Input {
public:
	Input ();
	~Input ();
	static Input *instance;

	// Read-write data members
	int keyRepeatStartThreshold;
	int keyRepeatDelay;

	// Read-only data members
	int mouseX, mouseY;
	int lastMouseX, lastMouseY;
	bool isMouseLeftButtonDown, isMouseRightButtonDown;
	int mouseLeftDownCount, mouseRightDownCount;
	int mouseLeftUpCount, mouseRightUpCount;
	int mouseWheelDownCount, mouseWheelUpCount;
	int windowCloseCount;

	// Initialize input functionality and acquire resources as needed. Returns a Result value.
	OsUtil::Result start ();

	// Stop the input engine and release acquired resources
	void stop ();

	// Poll events to update input state. This method must be invoked only from the application's main thread.
	void pollEvents ();

	// Return a boolean value indicating if the specified key is down
	bool isKeyDown (SDL_Keycode keycode);

	// Return a boolean value indicating if a shift key is down
	bool isShiftDown ();

	// Return a boolean value indicating if a control key is down
	bool isControlDown ();

	// Return the character value associated with the specified keycode, or 0 if no such character was found
	char getKeyCharacter (SDL_Keycode keycode, bool isShiftDown);

	// Consume keypress events that have occurred since the last poll and append the resulting items to the provided vector
	void pollKeyPressEvents (std::vector<SDL_Keycode> *destVector);

	// Generate a window close event for consumption by Ui classes
	void windowClose ();

private:
	std::map<SDL_Keycode, bool> keyDownMap;
	std::vector<SDL_Keycode> keyPressList;
	SDL_mutex *keyPressListMutex;
	bool isKeyPressListPopulated;
	bool isKeyRepeating;
	int keyRepeatCode;
	int64_t keyRepeatStartTime;
};

#endif
