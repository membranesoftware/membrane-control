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
#include "StdString.h"
#include "App.h"
#include "ClassId.h"
#include "UiText.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "UiConfiguration.h"
#include "TagWindow.h"

TagWindow::TagWindow (const StdString &tag)
: Panel ()
, tag (tag)
, label (NULL)
, image (NULL)
, removeButton (NULL)
, progressBar (NULL)
{
	classId = ClassId::TagWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	image = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::TagIconSprite)));
	label = (Label *) addWidget (new Label (tag, UiConfiguration::BodyFont, UiConfiguration::instance->lightPrimaryTextColor));

	removeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (TagWindow::removeButtonClicked, this);
	removeButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	removeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::RemoveMediaTagCommandName));

	refreshLayout ();
}

TagWindow::~TagWindow () {
}

bool TagWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::TagWindow));
}

TagWindow *TagWindow::castWidget (Widget *widget) {
	return (TagWindow::isWidgetType (widget) ? (TagWindow *) widget : NULL);
}

void TagWindow::refreshLayout () {
	float x, y, x0, y0, x2, y2;

	x0 = widthPadding;
	y0 = heightPadding;
	x = x0;
	y = y0;
	x2 = 0.0f;
	y2 = 0.0f;

	image->flowRight (&x, y, &x2, &y2);
	label->flowRight (&x, y, &x2, &y2);
	removeButton->flowDown (x, &y, &x2, &y2);

	resetSize ();

	image->centerVertical (y0, y2);
	label->centerVertical (y0, y2);
	removeButton->centerVertical (y0, y2);
}

void TagWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((TagWindow *) windowPtr)->eventCallback (((TagWindow *) windowPtr)->removeClickCallback);
}
