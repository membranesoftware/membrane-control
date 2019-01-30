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
// Widget that can hold other widgets

#ifndef PANEL_H
#define PANEL_H

#include <stdint.h>
#include <list>
#include "SDL2/SDL.h"
#include "Color.h"
#include "RecordStore.h"
#include "WidgetHandle.h"
#include "Widget.h"

class Panel : public Widget {
public:
	Panel ();
	virtual ~Panel ();

	// Constants to use for layout types
	enum {
		VERTICAL = 0,
		VERTICAL_RIGHT_JUSTIFIED = 1,
		HORIZONTAL = 2
	};

	// Read-write data members
	Color bgColor;
	Color borderColor;

	// Read-only data members
	float maxWidgetX, maxWidgetY;
	int maxWidgetZLevel;
	float viewOriginX, viewOriginY;
	bool isViewOriginBoundEnabled;
	float minViewOriginX, minViewOriginY;
	float maxViewOriginX, maxViewOriginY;
	float widthPadding, heightPadding;
	bool isAlphaBlended;
	float alpha;
	float alphaTarget;
	int alphaTransitionDuration;
	bool isFilledBg;
	bool isBordered;
	bool isFixedSize;
	bool isMouseDragScrollEnabled;
	bool isWaiting;
	int layout;

	// Set the panel's alpha blend option. If enabled, the panel's fill bg and border are drawn with alpha blending of the specified amount.
	void setAlphaBlend (bool enable, float blendAlpha = 1.0f);

	// Begin a transition of the panel's alpha blend using the specified start value, target value, and millisecond duration
	void transitionAlphaBlend (float startAlpha, float targetAlpha, int durationMs);

	// Set the panel's fill bg option. If enabled, the panel is drawn with a background fill using the specified color.
	void setFillBg (bool enable, float r = 0.0f, float g = 0.0f, float b = 0.0f);
	void setFillBg (bool enable, const Color &color);

	// Set the panel's border option. If enabled, the panel is drawn with a border using the specified color.
	void setBorder (bool enable, float r = 0.0f, float g = 0.0f, float b = 0.0f);
	void setBorder (bool enable, const Color &color);

	// Set the layout type that should be used to arrange the panel's widgets
	void setLayout (int layoutType);

	// Remove all widgets from the panel and mark them as destroyed
	void clear ();

	// Set the isDestroyed state for all widgets in the panel, causing them to be removed during the next update cycle
	void destroyAllChildWidgets ();

	// Add a widget to the panel. Returns the Widget pointer that was added.
	Widget *addWidget (Widget *widget, float positionX = 0.0f, float positionY = 0.0f, int zLevel = 0);

	// Remove the specified widget from the panel
	void removeWidget (Widget *targetWidget);

	// Return the topmost child widget at the specified mouse position that holds a non-zero mouseHoverId value, or NULL if no such widget was found
	Widget *findMouseHoverWidget (float mouseX, float mouseY);

	// Set the panel's view origin coordinates
	void setViewOrigin (float originX, float originY);

	// Set the minimum and maximum bounds for view origin coordinates
	void setViewOriginBounds (float originX1, float originY1, float originX2, float originY2);

	// Set the amount of size padding that should be applied to the panel's layout
	virtual void setPadding (float widthPaddingSize, float heightPaddingSize);

	// Set the fixed size option. If enabled, the panel uses the specified width and height values instead of dynamic resizing to fit contained elements.
	void setFixedSize (bool enable, float fixedWidth, float fixedHeight);

	// Set the mouse drag scroll option. If enabled, the panel responds to mouse drag actions by scrolling its view position.
	void setMouseDragScroll (bool enable);

	// Set the waiting option. If enabled, the panel disables input, shades it content, and shows a progress bar widget.
	void setWaiting (bool enable);

	// Update widget state to reflect the latest input events
	void processInput ();

	// Execute operations appropriate to sync widget state with records present in the provided RecordStore object, which has been locked prior to invocation
	void syncRecordStore (RecordStore *store);

protected:
	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
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

	// Reset the panel's w and h values as appropriate for its content and configuration
	void resetSize ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

	// Check if the widget list is correctly sorted for drawing by z-level, and sort the list if not. This method should only be invoked while holding a lock on widgetListMutex.
	void sortWidgetList ();

	bool isMouseInputStarted;
	int lastMouseLeftUpCount, lastMouseLeftDownCount;
	int lastMouseRightUpCount, lastMouseRightDownCount;
	int lastMouseWheelUpCount, lastMouseWheelDownCount;
	int lastMouseDownX, lastMouseDownY;
	SDL_mutex *widgetListMutex;
	std::list<Widget *> widgetList;
	SDL_mutex *widgetAddListMutex;
	std::list<Widget *> widgetAddList;
	WidgetHandle waitPanel;
	WidgetHandle waitProgressBar;
};

#endif
