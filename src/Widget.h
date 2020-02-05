/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Base class for user interface elements

#ifndef WIDGET_H
#define WIDGET_H

#include <stdint.h>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "Position.h"

class Widget {
public:
	typedef void (*FreeFunction) (void *data);
	typedef void (*UpdateCallback) (int msElapsed, Widget *widget);
	typedef void (*EventCallback) (void *data, Widget *widget);
	typedef bool (*KeyEventCallback) (void *data, SDL_Keycode keycode, bool isShiftDown, bool isControlDown);
	struct EventCallbackContext {
		Widget::EventCallback callback;
		void *callbackData;
		EventCallbackContext (): callback (NULL), callbackData (NULL) { }
		EventCallbackContext (Widget::EventCallback callback, void *callbackData): callback (callback), callbackData (callbackData) { }
	};
	struct KeyEventCallbackContext {
		Widget::KeyEventCallback callback;
		void *callbackData;
		KeyEventCallbackContext (): callback (NULL), callbackData (NULL) { }
		KeyEventCallbackContext (Widget::KeyEventCallback callback, void *callbackData): callback (callback), callbackData (callbackData) { }
	};

	Widget ();
	virtual ~Widget ();

	static const int minZLevel;

	// Read-write data members
	uint64_t id;
	bool isDestroyed;
	bool isVisible;
	bool isTextureTargetDrawEnabled;
	bool isInputSuspended;
	Position position;
	int zLevel;
	bool isMouseHoverEnabled;
	StdString sortKey;

	// Read-only data members
	int classId;
	bool hasScreenPosition;
	float screenX, screenY;
	bool isKeyFocused;
	StdString tooltipText;
	int tooltipAlignment;

	// Read-only data members. Widget subclasses should maintain these values for proper layout handling.
	float width, height;

	// Increase the object's refcount
	void retain ();

	// Decrease the object's refcount. If this reduces the refcount to zero or less, delete the object.
	void release ();

	// Return a string description of the widget
	virtual StdString toString ();

	// Reset the widget's input state
	void resetInputState ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	void update (int msElapsed, float originX, float originY);

	// Add draw commands for execution by the App. If targetTexture is non-NULL, render to that texture instead the default render target.
	void draw (SDL_Texture *targetTexture = NULL, float originX = 0.0f, float originY = 0.0f);

	// Refresh the widget's layout as appropriate for the current set of UiConfiguration values
	void refresh ();

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	virtual void syncRecordStore ();

	// Return the topmost child widget at the specified screen position, or NULL if no such widget was found. If requireMouseHoverEnabled is true, return a widget only if it has enabled the isMouseHoverEnabled option.
	virtual Widget *findWidget (float screenPositionX, float screenPositionY, bool requireMouseHoverEnabled = false);

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	bool processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	struct MouseState {
		int positionDeltaX, positionDeltaY;
		int wheelUp;
		int wheelDown;
		bool isEntered;
		float enterDeltaX, enterDeltaY;
		bool isLeftClicked;
		bool isLeftClickReleased;
		bool isLeftClickEntered;
		bool isLongPressed;
		MouseState (): positionDeltaX (0), positionDeltaY (0), wheelUp (0), wheelDown (0), isEntered (false), enterDeltaX (0.0f), enterDeltaY (0.0f), isLeftClicked (false), isLeftClickReleased (false), isLeftClickEntered (false), isLongPressed (false) { }
	};
	// Update the widget as appropriate for the specified mouse state
	void processMouseState (const Widget::MouseState &mouseState);

	// Set the widget to display the specified tooltip text on mouse hover
	enum {
		TopAlignment = 0,
		LeftAlignment = 1,
		RightAlignment = 2,
		BottomAlignment = 3
	};
	void setMouseHoverTooltip (const StdString &text, int alignment = Widget::BottomAlignment);

	// Set the widget to destroy itself after the specified number of milliseconds passes
	void setDestroyDelay (int delayMs);

	// Set the widget's fixed center option. If enabled, the widget executes translations as needed to maintain its current centered position.
	void setFixedCenter (bool enable);

	// Set the widget's key focus mode, indicating whether it should handle keypress events with edit focus
	virtual void setKeyFocus (bool enable);

	// Set the callback function that should be invoked on mouse enter events
	void setMouseEnterCallback (Widget::EventCallback fn, void *data);

	// Invoke the widget's mouse enter callback
	void mouseEnter ();

	// Set the callback function that should be invoked on mouse exit events
	void setMouseExitCallback (Widget::EventCallback fn, void *data);

	// Invoke the widget's mouse exit callback
	void mouseExit ();

	// Set the callback function that should be invoked on mouse press events
	void setMousePressCallback (Widget::EventCallback fn, void *data);

	// Invoke the widget's mouse press callback
	void mousePress ();

	// Set the callback function that should be invoked on mouse release events
	void setMouseReleaseCallback (Widget::EventCallback fn, void *data);

	// Invoke the widget's mouse release callback
	void mouseRelease ();

	// Set the callback function that should be invoked on mouse click events
	void setMouseClickCallback (Widget::EventCallback fn, void *data);

	// Invoke the widget's mouse click callback
	void mouseClick ();

	// Set the callback function that should be invoked on mouse long press events
	void setMouseLongPressCallback (Widget::EventCallback fn, void *data);

	// Set the callback function that should be invoked on keyboard events
	void setKeyEventCallback (Widget::KeyEventCallback fn, void *data);

	// Set the callback function that should be invoked on each state update
	void setUpdateCallback (Widget::UpdateCallback fn);

	// Assign the widget's position to the provided x/y values, then reset positionX as appropriate for a rightward flow. If rightExtent and bottomExtent are provided, update them with the widget's right (x plus width) and bottom (y plus height) extents if greater.
	virtual void flowRight (float *positionX, float positionY, float *rightExtent = NULL, float *bottomExtent = NULL);

	// Assign the widget's position to the provided x/y values, then reset positionY as appropriate for a downward flow. If rightExtent and bottomExtent are provided, update them with the widget's right (x plus width) and bottom (y plus height) extents if greater.
	virtual void flowDown (float positionX, float *positionY, float *rightExtent = NULL, float *bottomExtent = NULL);

	// Assign the widget's x position to the provided value, then reset positionX as appropriate for a leftward flow
	virtual void flowLeft (float *positionX);

	// Assign the widget's y position to a centered value within the provided vertical extents
	virtual void centerVertical (float topExtent, float bottomExtent);

	// Callback functions
	static bool compareZLevel (Widget *first, Widget *second);

protected:
	// Execute subclass-specific operations to update object state as appropriate for an elapsed millisecond time period
	virtual void doUpdate (int msElapsed);

	// Add subclass-specific draw commands for execution by the App. If targetTexture is non-NULL, it has been set as the render target and draw commands should adjust coordinates as appropriate.
	virtual void doDraw (SDL_Texture *targetTexture, float originX, float originY);

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

	// Execute operations appropriate when the widget's input state is reset
	virtual void doResetInputState ();

	// Execute operations appropriate when the widget receives new mouse state
	virtual void doProcessMouseState (const Widget::MouseState &mouseState);

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Return a string that should be included as part of the toString method's output
	virtual StdString toStringDetail ();

	int destroyClock;
	bool isFixedCenter;
	Position fixedCenterPosition;
	bool isMouseEntered;
	bool isMousePressed;
	Widget::EventCallback mouseEnterCallback;
	void *mouseEnterCallbackData;
	Widget::EventCallback mouseExitCallback;
	void *mouseExitCallbackData;
	Widget::EventCallback mousePressCallback;
	void *mousePressCallbackData;
	Widget::EventCallback mouseReleaseCallback;
	void *mouseReleaseCallbackData;
	Widget::EventCallback mouseClickCallback;
	void *mouseClickCallbackData;
	Widget::EventCallback mouseLongPressCallback;
	void *mouseLongPressCallbackData;
	Widget::KeyEventCallback keyEventCallback;
	void *keyEventCallbackData;
	Widget::UpdateCallback updateCallback;

private:
	int refcount;
	SDL_mutex *refcountMutex;
};

#endif
