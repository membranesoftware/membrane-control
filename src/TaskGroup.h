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
// Class that runs tasks in background threads

#ifndef TASK_GROUP_H
#define TASK_GROUP_H

#include <list>
#include "SDL2/SDL.h"

class TaskGroup {
public:
	TaskGroup ();
	~TaskGroup ();
	static TaskGroup *instance;

	// Read-only data members
	bool isStopped;

	typedef void (*RunFunction) (void *runPtr);
	struct RunContext {
		TaskGroup::RunFunction fn;
		void *fnData;
		RunContext ():
			fn (NULL),
			fnData (NULL) { }
		RunContext (TaskGroup::RunFunction fn, void *fnData):
			fn (fn),
			fnData (fnData) { }
	};
	typedef void (*EndCallback) (void *callbackData, void *runPtr);
	struct EndCallbackContext {
		TaskGroup::EndCallback callback;
		void *callbackData;
		EndCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		EndCallbackContext (TaskGroup::EndCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};

	// Add fn as a run task and invoke endCallback when complete. Returns a boolean value indicating if the task was successfully queued.
	bool run (TaskGroup::RunContext fn, TaskGroup::EndCallbackContext endCallback = TaskGroup::EndCallbackContext ());

	// Update state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Stop the task group and prevent further tasks from being started
	void stop ();

	// Return a boolean value indicating if the task group has been stopped and holds no active tasks
	bool isStopComplete ();

	// Join any running task threads, blocking until the operation completes
	void waitThreads ();

private:
	// Run a thread that executes a task
	static int executeTask (void *taskContextPtr);

	struct TaskContext {
		TaskGroup::RunContext fn;
		TaskGroup::EndCallbackContext endCallback;
		SDL_Thread *thread;
		bool isRunning;
		TaskContext ():
			fn (),
			endCallback (),
			thread (NULL),
			isRunning (false) { }
	};

	std::list<TaskGroup::TaskContext> contextList;
	SDL_mutex *contextListMutex;
};

#endif
