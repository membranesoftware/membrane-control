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
// Widget that shows a linear progress indicator

#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include "Color.h"
#include "Widget.h"

class ProgressBar : public Widget {
public:
	ProgressBar (float barWidth = 0.0f, float barHeight = 0.0f);
	~ProgressBar ();

	static const float AnimationFactor;

	// Set the bar's size
	void setSize (float barWidth, float barHeight);

	// Set the bar's progress value, and optionally its progress target value as well
	void setProgress (float value);
	void setProgress (float value, float targetValue);

	// Set the bar's indeterminate state. If enabled, the bar animates without indicating how long it expects its task to take.
	void setIndeterminate (bool indeterminate);

protected:
	// Execute operations to update object state as appropriate for an elapsed millisecond time period
	virtual void doUpdate (int msElapsed);

	// Add subclass-specific draw commands for execution by the App. If targetTexture is non-NULL, it has been set as the render target and draw commands should adjust coordinates as appropriate.
	virtual void doDraw (SDL_Texture *targetTexture, float originX, float originY);

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	virtual void doRefresh ();

private:
	// Reset layout as appropriate for current state
	void refreshLayout ();

	Color bgColor;
	Color fillColor;
	bool isIndeterminate;
	float progressValue;
	float targetProgressValue;
	int fillStage;
	float fillStart;
	float fillStartTarget;
	float fillEnd;
	float fillEndTarget;
};

#endif
