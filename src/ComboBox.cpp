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
#include "Config.h"
#include <stdlib.h>
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "Ui.h"
#include "Input.h"
#include "Sprite.h"
#include "Widget.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Panel.h"
#include "UiConfiguration.h"
#include "ComboBox.h"

ComboBox::ComboBox ()
: Panel ()
, isExpanded (false)
, expandPanelId (0)
, expandDrawX (0.0f)
, expandDrawY (0.0f)
, isFocused (false)
, selectedItemLabel (NULL)
, maxTextWidth (0.0f)
{
	UiConfiguration *uiconfig;

	typeName.assign ("ComboBox");

	uiconfig = &(App::getInstance ()->uiConfig);
	setFillBg (true, uiconfig->lightBackgroundColor);
	setBorder (true, uiconfig->mediumBackgroundColor);
}

ComboBox::~ComboBox () {

}

void ComboBox::setValue (const StdString &value) {
	std::list<ComboBox::Item>::iterator i, end;
	bool found;

	found = false;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->value.equals (value)) {
			selectedItemLabel = i->label;
			selectedItemValue.assign (i->value);
			selectedItemData.assign (i->itemData);
			found = true;
			break;
		}
		++i;
	}

	if (found) {
		resetLayout ();
	}
}

StdString ComboBox::getValue () {
	if (! selectedItemData.empty ()) {
		return (selectedItemData);
	}

	return (selectedItemValue);
}

void ComboBox::addItem (const StdString &itemValue, const StdString &itemData) {
	UiConfiguration *uiconfig;
	ComboBox::Item item;
	LabelWindow *labelwindow;
	Label *label;

	uiconfig = &(App::getInstance ()->uiConfig);
	label = new Label (itemValue, UiConfiguration::CAPTION, uiconfig->primaryTextColor);
	if (label->width > maxTextWidth) {
		maxTextWidth = label->width;
	}

	labelwindow = (LabelWindow *) addWidget (new LabelWindow (label));
	if (! selectedItemLabel) {
		selectedItemLabel = labelwindow;
		selectedItemValue.assign (itemValue);
		selectedItemData.assign (itemData);
	}

	item.value.assign (itemValue);
	item.itemData.assign (itemData);
	item.label = labelwindow;
	itemList.push_back (item);
	resetLayout ();
}

void ComboBox::addItems (StringList *nameList) {
	StringList::iterator i, end;

	i = nameList->begin ();
	end = nameList->end ();
	while (i != end) {
		addItem (*i);
		++i;
	}
}

void ComboBox::addItems (HashMap *itemMap) {
	HashMap::Iterator i;
	StdString key, value;

	i = itemMap->begin ();
	while (itemMap->hasNext (&i)) {
		key = itemMap->next (&i);
		value = itemMap->find (key, StdString (""));
		addItem (key, value);
	}
}

void ComboBox::resetLayout () {
	UiConfiguration *uiconfig;
	std::list<ComboBox::Item>::iterator i, end;
	LabelWindow *label;
	float x, y;
	bool show;

	uiconfig = &(App::getInstance ()->uiConfig);
	if (isFocused) {
		borderColor.rotate (uiconfig->lightBackgroundColor, uiconfig->colorRotateDuration);
		bgColor.rotate (uiconfig->darkBackgroundColor, uiconfig->colorRotateDuration);
	}
	else {
		bgColor.rotate (uiconfig->lightBackgroundColor, uiconfig->colorRotateDuration);
		borderColor.rotate (uiconfig->mediumBackgroundColor, uiconfig->colorRotateDuration);
	}

	x = 0.0f;
	y = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		label = i->label;
		label->setWindowWidth (maxTextWidth + (uiconfig->paddingSize * 2.0f), true);

		show = false;
		if (label == selectedItemLabel) {
			show = true;
		}

		if (show) {
			label->position.assign (x, y);
			y += label->height;
			label->isVisible = true;
		}
		else {
			label->isVisible = false;
		}

		++i;
	}

	resetSize ();
}

void ComboBox::setFocused (bool enable) {
	if (enable == isFocused) {
		return;
	}
	isFocused = enable;
	resetLayout ();
}

void ComboBox::doProcessMouseState (const Widget::MouseState &mouseState) {
	App *app;
	Panel *expandpanel;
	bool shouldunexpand;
	int x, y, x1, x2, y1, y2;

	if (isExpanded) {
		setFocused (false);
		if (mouseState.isLeftClicked) {
			shouldunexpand = false;
			expandpanel = findExpandPanel ();

			if (mouseState.isEntered) {
				shouldunexpand = true;
			}

			if ((! shouldunexpand) && expandpanel) {
				app = App::getInstance ();
				x = app->input.mouseX;
				y = app->input.mouseY;
				x1 = (int) expandpanel->drawX;
				y1 = (int) expandpanel->drawY;
				x2 = x1 + (int) expandpanel->width;
				y2 = y1 + (int) expandpanel->height;
				if ((x < x1) || (x > x2) || (y < y1) || (y > y2)) {
					shouldunexpand = true;
				}
			}

			if (shouldunexpand) {
				if (expandpanel) {
					expandpanel->isDestroyed = true;
				}
				unexpand ();
			}
		}
		return;
	}

	setFocused (mouseState.isEntered);
	if (mouseState.isEntered && mouseState.isLeftClicked) {
		expand ();
	}
}

void ComboBox::expand () {
	Ui *ui;
	UiConfiguration *uiconfig;
	Panel *expandpanel;
	std::list<ComboBox::Item>::iterator i, end;
	LabelWindow *label;
	float x, y;

	if (isExpanded) {
		return;
	}

	if (itemList.size () < 2) {
		return;
	}

	ui = getParentUi ();
	if (! ui) {
		return;
	}

	uiconfig = &(App::getInstance ()->uiConfig);
	expandpanel = new Panel ();
	expandpanel->parentId = id;
	expandpanel->setFillBg (true, uiconfig->lightBackgroundColor);
	expandpanel->setBorder (true, uiconfig->mediumBackgroundColor);

	// TODO: Add scrolling for cases when there are many options to choose from

	x = 0.0f;
	y = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->label != selectedItemLabel) {
			label = (LabelWindow *) expandpanel->addWidget (new LabelWindow (new Label (i->value, UiConfiguration::CAPTION, uiconfig->primaryTextColor)), x, y);
			label->setWindowWidth (maxTextWidth + (uiconfig->paddingSize * 2.0f), true);
			label->setMouseoverHighlight (true, uiconfig->primaryTextColor, uiconfig->lightBackgroundColor, uiconfig->primaryTextColor, uiconfig->darkBackgroundColor, uiconfig->colorRotateDuration);
			label->setMouseClickCallback (ComboBox::expandItemClicked, expandpanel);
			label->parentId = id;
			y += label->height;
		}

		++i;
	}
	expandpanel->refresh ();
	expandDrawX = drawX;
	expandDrawY = drawY;
	ui->addWidget (expandpanel, expandDrawX, expandDrawY + height, ui->rootPanel->maxWidgetZLevel + 1);

	expandPanelId = expandpanel->id;
	isExpanded = true;
}

void ComboBox::unexpand (const StdString &value) {
	if (! value.empty ()) {
		setValue (value);
	}
	isExpanded = false;
	expandPanelId = 0;
}

void ComboBox::doUpdate (int msElapsed, float originX, float originY) {
	Panel *expandpanel;

	Panel::doUpdate (msElapsed, originX, originY);

	if (! isExpanded) {
		return;
	}
	expandpanel = findExpandPanel ();
	if (! expandpanel) {
		unexpand ();
		return;
	}

	if ((! FLOAT_EQUALS (drawX, expandDrawX)) || (! FLOAT_EQUALS (drawY, expandDrawY))) {
		expandpanel->isDestroyed = true;
		unexpand ();
		return;
	}
}

Panel *ComboBox::findExpandPanel () {
	Panel *expandpanel;
	Ui *ui;

	expandpanel = NULL;
	ui = getParentUi ();
	if (ui) {
		expandpanel = (Panel *) ui->findWidget (expandPanelId, StdString ("Panel"));
	}

	return (expandpanel);
}

void ComboBox::expandItemClicked (void *expandPanelPtr, Widget *labelWindowPtr) {
	LabelWindow *label;
	Panel *expandpanel;
	Ui *ui;
	ComboBox *combobox;

	label = (LabelWindow *) labelWindowPtr;
	expandpanel = (Panel *) expandPanelPtr;
	combobox = NULL;
	ui = label->getParentUi ();
	if (ui) {
		combobox = (ComboBox *) ui->findWidget (label->parentId, StdString ("ComboBox"));
	}
	if ((! combobox) || combobox->isDestroyed) {
		expandpanel->isDestroyed = true;
		return;
	}

	combobox->unexpand (label->getText ());
	expandpanel->isDestroyed = true;
}

void ComboBox::destroyAllChildWidgets () {
	Panel *expandpanel;

	expandpanel = findExpandPanel ();
	if (expandpanel) {
		expandpanel->isDestroyed = true;
	}
}
