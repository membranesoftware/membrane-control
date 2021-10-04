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
// Panel that shows a set of name-value pairs

#ifndef STATS_WINDOW_H
#define STATS_WINDOW_H

#include <list>
#include "StdString.h"
#include "Color.h"
#include "Label.h"
#include "Panel.h"

class StatsWindow : public Panel {
public:
	StatsWindow ();
	virtual ~StatsWindow ();

	// Read-write data members
	bool isEmptyItemValueIgnored;

	// Remove all stats items
	void clearItems ();

	// Set the specified name-value pair as a stats item in the window, creating a new line if needed
	void setItem (const StdString &itemName, const StdString &itemValue);

	// Set the color that should be used for the specified item's value text
	void setItemTextColor (const StdString &itemName, const Color &itemTextColor);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	struct Item {
		Label *nameLabel;
		Label *valueLabel;
		Item (): nameLabel (NULL), valueLabel (NULL) { }
	};

	std::list<StatsWindow::Item> itemList;
	float maxLineHeight;
};

#endif
