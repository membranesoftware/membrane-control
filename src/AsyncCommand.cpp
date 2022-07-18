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
#include "SDL2/SDL.h"
#include "App.h"
#include "Log.h"
#include "OsUtil.h"
#include "StdString.h"
#include "Json.h"
#include "AgentControl.h"
#include "SystemInterface.h"
#include "AsyncCommand.h"

AsyncCommand::AsyncCommand (const StdString &agentId, Json *cmdInv)
: responseCommand (NULL)
, invokeResult (0)
, agentId (agentId)
, cmdInv (cmdInv)
, mutex (NULL)
, cond (NULL)
{
	mutex = SDL_CreateMutex ();
	cond = SDL_CreateCond ();
}

AsyncCommand::~AsyncCommand () {
	if (cond) {
		SDL_DestroyCond (cond);
		cond = NULL;
	}
	if (mutex) {
		SDL_DestroyMutex (mutex);
		mutex = NULL;
	}
	if (cmdInv) {
		delete (cmdInv);
		cmdInv = NULL;
	}
	if (responseCommand) {
		delete (responseCommand);
		responseCommand = NULL;
	}
}

int AsyncCommand::execute () {
	int result;

	Log::debug ("Execute async command; thread=0x%lx agentId=%s cmdInv=%s", SDL_ThreadID (), agentId.c_str (), TOSTRING_STRING (cmdInv));
	SDL_LockMutex (mutex);
	result = AgentControl::instance->invokeCommand (agentId, cmdInv, CommandList::InvokeCallbackContext (AsyncCommand::invokeComplete, this));
	cmdInv = NULL;
	if (result != OsUtil::Success) {
		SDL_UnlockMutex (mutex);
		return (result);
	}

	SDL_CondWait (cond, mutex);
	SDL_UnlockMutex (mutex);
	Log::debug ("Async command complete; thread=0x%lx", SDL_ThreadID ());
	return (OsUtil::Success);
}

void AsyncCommand::invokeComplete (void *cmdPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand, const StdString &invokeId) {
	AsyncCommand *cmd;

	cmd = (AsyncCommand *) cmdPtr;

	cmd->invokeResult = invokeResult;
	if (responseCommand) {
		if (cmd->responseCommand) {
			delete (cmd->responseCommand);
		}
		cmd->responseCommand = responseCommand->copy ();
	}

	SDL_LockMutex (cmd->mutex);
	SDL_CondSignal (cmd->cond);
	SDL_UnlockMutex (cmd->mutex);
}
