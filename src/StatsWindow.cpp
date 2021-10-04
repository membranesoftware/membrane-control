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
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Widget.h"
#include "Color.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "UiConfiguration.h"
#include "StatsWindow.h"

StatsWindow::StatsWindow ()
: Panel ()
, isEmptyItemValueIgnored (false)
, maxLineHeight (0.0f)
{
	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
}

StatsWindow::~StatsWindow () {

}

StdString StatsWindow::toStringDetail () {
	return (StdString (" StatsWindow"));
}

void StatsWindow::clearItems () {
	std::list<StatsWindow::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->nameLabel) {
			i->nameLabel->isDestroyed = true;
		}
		if (i->valueLabel) {
			i->valueLabel->isDestroyed = true;
		}
		++i;
	}
	itemList.clear ();
	maxLineHeight = 0.0f;
	resetSize ();
}

void StatsWindow::setItem (const StdString &itemName, const StdString &itemValue) {
	std::list<StatsWindow::Item>::iterator i, end;
	StatsWindow::Item item;
	Label *label;
	bool found;

	if (isEmptyItemValueIgnored && itemValue.empty ()) {
		return;
	}
	found = false;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->nameLabel->text.equals (itemName)) {
			found = true;
			label = i->valueLabel;
			if (! label->text.equals (itemValue)) {
				if (! label->text.empty ()) {
					label->textColor.assign (UiConfiguration::instance->primaryTextColor);
					label->textColor.translate (UiConfiguration::instance->lightPrimaryTextColor, UiConfiguration::instance->longColorTranslateDuration);
				}
				label->setText (itemValue);
				refreshLayout ();
			}
			break;
		}

		++i;
	}

	if (! found) {
		label = (Label *) addWidget (new Label (itemName, UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
		if (label->maxLineHeight > maxLineHeight) {
			maxLineHeight = label->maxLineHeight;
		}
		item.nameLabel = label;

		label = (Label *) addWidget (new Label (itemValue, UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
		if (label->maxLineHeight > maxLineHeight) {
			maxLineHeight = label->maxLineHeight;
		}
		item.valueLabel = label;

		itemList.push_back (item);
		refreshLayout ();
	}
}

void StatsWindow::setItemTextColor (const StdString &itemName, const Color &itemTextColor) {
	std::list<StatsWindow::Item>::iterator i, end;

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->nameLabel->text.equals (itemName)) {
			i->valueLabel->textColor.assign (itemTextColor);
			break;
		}
		++i;
	}
}

void StatsWindow::doRefresh () {
	std::list<StatsWindow::Item>::iterator i, end;

	maxLineHeight = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		if (i->nameLabel->maxLineHeight > maxLineHeight) {
			maxLineHeight = i->nameLabel->maxLineHeight;
		}
		++i;
	}
	Panel::doRefresh ();
}

void StatsWindow::refreshLayout () {
	std::list<StatsWindow::Item>::iterator i, end;
	Label *label;
	float x, y, w;

	x = widthPadding;
	y = heightPadding;
	w = 0.0f;
	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		label = i->nameLabel;
		if (label->width > w) {
			w = label->width;
		}
		++i;
	}

	i = itemList.begin ();
	end = itemList.end ();
	while (i != end) {
		label = i->nameLabel;
		label->position.assign (x + w - label->width, label->getLinePosition (y));
		label = i->valueLabel;
		label->position.assign (x + w + UiConfiguration::instance->marginSize, label->getLinePosition (y));
		y += maxLineHeight + UiConfiguration::instance->textLineHeightMargin;
		++i;
	}

	resetSize ();
}
