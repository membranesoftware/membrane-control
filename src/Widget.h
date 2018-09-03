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
// Base class for widgets that can appear in a view

#ifndef WIDGET_H
#define WIDGET_H

#include <stdint.h>
#include "SDL2/SDL_video.h"
#include "StdString.h"
#include "Position.h"
#include "Json.h"
#include "RecordStore.h"

class Ui;

class Widget {
public:
	// Function types for use in widget event callbacks
	typedef void (*EventCallback) (void *data, Widget *widget);
	typedef bool (*KeyEventCallback) (void *data, SDL_Keycode keycode, bool isShiftDown, bool isControlDown);
	typedef void (*UpdateCallback) (int msElapsed, Widget *widget);

	Widget ();
	virtual ~Widget ();

	static const int minZLevel;

	// Subclasses should assign the typeName value in their constructor
	StdString typeName;

	// Read-write data members
	uint64_t id;
	uint64_t parentId;
	bool isDestroyed;
	bool isVisible;
	bool isInputSuspended;
	Position position;
	int zLevel;
	bool isFixedPosition;
	bool isMouseHoverEnabled;

	// Read-only data members
	bool isDrawable;
	float drawX, drawY;
	StdString tooltipText;
	int tooltipAlignment;
	void *extraData;
	Widget *parentWidget; // This pointer is maintained weakly and should not be referenced in contexts where the parent may have been destroyed

	// Read-only data members. Widget subclasses should maintain these values for proper layout handling.
	float width, height;

	// Increase the object's refcount
	void retain ();

	// Decrease the object's refcount. If this reduces the refcount to zero or less, delete the object.
	void release ();

	// Return a string description of the widget
	virtual StdString toString ();

	// Set the widget's parentUi pointer
	void setParentUi (Ui *ui);

	// Return a pointer to the widget's parent Ui object, or NULL if the parent UI could not be determined
	Ui *getParentUi ();

	// Reset the widget's input state
	void resetInputState ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	void update (int msElapsed, float originX, float originY);

	// Add draw commands for execution by the App
	void draw ();

	// Refresh the widget's layout as appropriate for the current set of UiConfiguration values
	void refresh ();

	// Execute operations appropriate to sync widget state with records present in the provided RecordStore object, which has been locked prior to invocation
	virtual void syncRecordStore (RecordStore *store);

	// Set the isDestroyed state for all child widgets, causing them to be removed during the next update cycle
	virtual void destroyAllChildWidgets ();

	// Return the child widget at the specified position in the widget's area, or NULL if no such widget was found. If shouldRecurse is true, the method finds the topmost child widget using recursive invocations.
	virtual Widget *findChildWidget (float positionX, float positionY, bool shouldRecurse = false);

	// Return the child widget matching the specified ID and type name, or NULL if no such widget was found
	virtual Widget *findChildWidget (uint64_t widgetId, const StdString &widgetTypeName, bool shouldRecurse = false);

	// Return the topmost child widget at the specified mouse position that holds a non-zero mouseHoverId value, or NULL if no such widget was found
	virtual Widget *findMouseHoverWidget (float mouseX, float mouseY);

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
		MouseState (): positionDeltaX (0), positionDeltaY (0), wheelUp (0), wheelDown (0), isEntered (false), enterDeltaX (0.0f), enterDeltaY (0.0f), isLeftClicked (false), isLeftClickReleased (false), isLeftClickEntered (false) { }
	};
	// Update the widget as appropriate for the specified mouse state
	void processMouseState (const Widget::MouseState &mouseState);

	// Set the widget's extraData value. If freeOnDestroy is true, this object becomes responsible for calling free on the data pointer when it's no longer needed.
	void setExtraData (void *data, bool freeOnDestroy = false);

	// Set the widget to display the specified tooltip text on mouse hover
	enum {
		TOP = 0,
		LEFT = 1,
		RIGHT = 2,
		BOTTOM = 3
	};
	void setMouseHoverTooltip (const StdString &text, int alignment = Widget::BOTTOM);

	// Set the widget to destroy itself after the specified number of milliseconds passes
	void setDestroyDelay (int delayMs);

	// Set the widget's fixed center option. If enabled, the widget executes translations as needed to maintain its current centered position.
	void setFixedCenter (bool enable);

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

	// Set the callback function that should be invoked on keyboard events
	void setKeyEventCallback (Widget::KeyEventCallback fn, void *data);

	// Set the callback function that should be invoked on each state update
	void setUpdateCallback (Widget::UpdateCallback fn);

	// Callback functions
	static bool compareZLevel (Widget *first, Widget *second);

protected:
	// Execute subclass-specific operations to update object state as appropriate for an elapsed millisecond time period and origin position
	virtual void doUpdate (int msElapsed, float originX, float originY);

	// Add subclass-specific draw commands for execution by the App
	virtual void doDraw ();

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

	Ui *parentUi; // This pointer is maintained weakly and should not be referenced in contexts where the parent UI may have been destroyed
	int destroyClock;
	bool isFreeingExtraData;
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
	Widget::KeyEventCallback keyEventCallback;
	void *keyEventCallbackData;
	Widget::UpdateCallback updateCallback;

private:
	int refcount;
	SDL_mutex *refcountMutex;
};

#endif
