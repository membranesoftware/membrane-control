/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "Position.h"
#include "WidgetHandle.h"
#include "Widget.h"

class Panel : public Widget {
public:
	Panel ();
	virtual ~Panel ();

	static const int longPressDuration; // ms

	// Constants to use for layout types
	enum {
		VerticalLayout = 0,
		VerticalRightJustifiedLayout = 1,
		HorizontalLayout = 2
	};

	// Read-write data members
	Color bgColor;
	Color borderColor;
	Color dropShadowColor;
	bool shouldRefreshTexture;

	// Read-only data members
	bool isTextureRenderEnabled;
	float maxWidgetX, maxWidgetY;
	int maxWidgetZLevel;
	float viewOriginX, viewOriginY;
	bool isViewOriginBoundEnabled;
	float minViewOriginX, minViewOriginY;
	float maxViewOriginX, maxViewOriginY;
	float widthPadding, heightPadding;
	bool isFilledBg;
	int topLeftCornerRadius;
	int topRightCornerRadius;
	int bottomLeftCornerRadius;
	int bottomRightCornerRadius;
	bool isBordered;
	float borderWidth;
	bool isDropShadowed;
	float dropShadowWidth;
	bool isFixedSize;
	bool isMouseDragScrollEnabled;
	bool isWaiting;
	int layout;
	bool isAnimating;
	Position animationScale;

	// Set the panel's texture render option. If enabled, the panel renders content to a texture.
	void setTextureRender (bool enable);

	// Execute a texture render animation to translate the panel's draw scale over the specified duration
	void animateScale (float startScale, float targetScale, int duration);

	// Execute a texture render animation appropriate for a panel that has just appeared as a new item in a card view
	void animateNewCard ();

	// Set the panel's fill bg option. If enabled, the panel is drawn with a background fill using the specified color.
	void setFillBg (bool enable, const Color &color = Color ());

	// Set the panel's corner radius values. If the panel's fill bg option is enabled, each fill rectangle corner with a nonzero radius value is drawn with a rounded shape.
	// TODO: Fix incorrect rendering results when corner radius is enabled while a bgColor alpha blend or a border is present
	void setCornerRadius (int radius);
	void setCornerRadius (int topLeftRadius, int topRightRadius, int bottomLeftRadius, int bottomRightRadius);

	// Set the panel's border option. If enabled, the panel is drawn with a border using the specified color and width.
	void setBorder (bool enable, const Color &color = Color (), float borderWidthValue = 1.0f);

	// Set the panel's drop shadow option. If enabled, the panel is drawn with a drop shadow effect using the specified color and width.
	void setDropShadow (bool enable, const Color &color = Color (), float dropShadowWidthValue = 1.0f);

	// Set the layout type that should be used to arrange the panel's widgets
	virtual void setLayout (int layoutType, float maxPanelWidth = 0.0f);

	// Remove all widgets from the panel and mark them as destroyed
	void clear ();

	// Add a widget to the panel. Returns the Widget pointer that was added.
	Widget *addWidget (Widget *widget, float positionX = 0.0f, float positionY = 0.0f, int zLevel = 0);

	// Remove the specified widget from the panel
	void removeWidget (Widget *targetWidget);

	// Return the topmost child widget at the specified screen position, or NULL if no such widget was found. If requireMouseHoverEnabled is true, return a widget only if it has enabled the isMouseHoverEnabled option.
	Widget *findWidget (float screenPositionX, float screenPositionY, bool requireMouseHoverEnabled = false);

	// Set the panel's view origin coordinates
	void setViewOrigin (float originX, float originY);

	// Set the minimum and maximum bounds for view origin coordinates
	void setViewOriginBounds (float originX1, float originY1, float originX2, float originY2);

	// Set the amount of size padding that should be applied to the panel's layout
	virtual void setPadding (float widthPaddingSize, float heightPaddingSize);

	// Set the fixed size option. If enabled, the panel uses the specified width and height values instead of dynamic resizing to fit contained elements.
	void setFixedSize (bool enable, float fixedWidth = 0.0f, float fixedHeight = 0.0f);

	// Set the mouse drag scroll option. If enabled, the panel responds to mouse drag actions by scrolling its view position.
	void setMouseDragScroll (bool enable);

	// Set the waiting option. If enabled, the panel disables input, shades it content, and shows a progress bar widget.
	void setWaiting (bool enable);

	// Update widget state to reflect the latest input events
	void processInput ();

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	virtual void syncRecordStore ();

	// Reset the panel's draw texture as appropriate for a new enable state
	static void resetDrawTexture (void *panelPtr);

protected:
	// Execute operations to update object state as appropriate for an elapsed millisecond time period
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

	// Reset the panel's w and h values as appropriate for its content and configuration
	void resetSize ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

	// Check if the widget list is correctly sorted for drawing by z-level, and sort the list if not. This method must only be invoked while holding a lock on widgetListMutex.
	void sortWidgetList ();

	SDL_Texture *drawTexture;
	int drawTextureWidth, drawTextureHeight;
	StdString drawTexturePath;
	bool isResettingDrawTexture;
	bool isMouseInputStarted;
	int lastMouseLeftUpCount, lastMouseLeftDownCount;
	int lastMouseRightUpCount, lastMouseRightDownCount;
	int lastMouseWheelUpCount, lastMouseWheelDownCount;
	int lastMouseDownX, lastMouseDownY;
	int64_t lastMouseDownTime;
	int cornerCenterDx, cornerCenterDy, cornerCenterDw, cornerCenterDh;
	int cornerTopDx, cornerTopDy, cornerTopDw, cornerTopDh;
	int cornerLeftDx, cornerLeftDy, cornerLeftDw, cornerLeftDh;
	int cornerRightDx, cornerRightDy, cornerRightDw, cornerRightDh;
	int cornerBottomDx, cornerBottomDy, cornerBottomDw, cornerBottomDh;
	int cornerSize;
	SDL_mutex *widgetListMutex;
	std::list<Widget *> widgetList;
	SDL_mutex *widgetAddListMutex;
	std::list<Widget *> widgetAddList;
	WidgetHandle waitPanel;
	WidgetHandle waitProgressBar;
};

#endif
