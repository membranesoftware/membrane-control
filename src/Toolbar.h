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
// Widget that shows a horizontal bar and allows other widgets to be added as clickable items

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <list>
#include "StdString.h"
#include "ImageWindow.h"
#include "WidgetHandle.h"
#include "Panel.h"

class Toolbar : public Panel {
public:
	Toolbar (float toolbarWidth);
	virtual ~Toolbar ();

	// Reset the toolbar's width
	void setWidth (float toolbarWidth);

	// Return a boolean value indicating if the toolbar contains no items
	bool empty ();

	// Remove all items, overlays, and corner widgets from the bar
	void clearAll ();

	// Remove all left side items from the bar
	void clearLeftItems ();

	// Remove all right side items from the bar
	void clearRightItems ();

	// Clear any left overlay widget present on the bar
	void clearLeftOverlay ();

	// Add the provided widget as an item on the left side of the bar
	void addLeftItem (Widget *itemWidget);

	// Add the provided widget as an item on the right side of the bar
	void addRightItem (Widget *itemWidget);

	// Add a spacer item to the right side of the bar
	void addRightSpacer ();

	// Add the provided widget as the left corner item
	void setLeftCorner (Widget *itemWidget);

	// Add the provided widget as the right corner item
	void setRightCorner (Widget *itemWidget);

	// Add the provided widget as an overlay that temporarily replaces the left side items, clearing any left overlay that might already be present
	void setLeftOverlay (Widget *itemWidget);

	// Add the provided widget as an overlay that temporarily replaces the right side items, clearing any right overlay that might already be present
	void setRightOverlay (Widget *itemWidget);

	// Return the width of the toolbar's left item area
	float getLeftWidth ();

protected:
	// Execute operations to update object state as appropriate for an elapsed millisecond time period
	virtual void doUpdate (int msElapsed);

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void refreshLayout ();

private:
	float barWidth;
	std::list<Widget *> leftItemList;
	std::list<Widget *> rightItemList;
	WidgetHandle leftCorner;
	WidgetHandle rightCorner;
	WidgetHandle leftOverlay;
	WidgetHandle rightOverlay;
};

#endif
