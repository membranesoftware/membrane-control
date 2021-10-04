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
#include "Config.h"
#include <stdlib.h>
#include "App.h"
#include "StdString.h"
#include "Log.h"
#include "Widget.h"
#include "WidgetHandle.h"

WidgetHandle::WidgetHandle (Widget *sourceWidget)
: widget (NULL)
{
	assign (sourceWidget);
}

WidgetHandle::~WidgetHandle () {
	assign (NULL);
}

void WidgetHandle::assign (Widget *sourceWidget) {
	if (sourceWidget == widget) {
		return;
	}
	if (widget) {
		widget->release ();
	}
	widget = sourceWidget;
	if (widget) {
		widget->retain ();
	}
}

void WidgetHandle::clear () {
	assign (NULL);
}

void WidgetHandle::destroyAndClear () {
	if (widget) {
		widget->isDestroyed = true;
		assign (NULL);
	}
}

void WidgetHandle::compact () {
	if (widget && widget->isDestroyed) {
		assign (NULL);
	}
}

bool WidgetHandle::equals (Widget *sourceWidget) const {
	return (widget == sourceWidget);
}
