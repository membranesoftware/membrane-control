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
#include "Widget.h"
#include "Panel.h"
#include "UiConfiguration.h"
#include "RowView.h"

RowView::RowView ()
: Panel ()
{
	UiConfiguration *uiconfig;

	widgetType.assign ("RowView");
	uiconfig = &(App::getInstance ()->uiConfig);

	setPadding (uiconfig->paddingSize, 0.0f);
	refreshLayout ();
}

RowView::~RowView () {

}

void RowView::addItem (Widget *itemWidget) {
	addWidget (itemWidget);
	itemList.push_back (itemWidget);
	refreshLayout ();
}

void RowView::refreshLayout () {
	UiConfiguration *uiconfig;
	std::list<Widget *>::iterator i, end;
	Widget *item;
	float x;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = uiconfig->paddingSize;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		item = *i;
		item->position.assign (x, 0.0f);
		x += item->width + uiconfig->marginSize;
		++i;
	}

	resetSize ();
}
