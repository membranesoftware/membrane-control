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
// Class that holds a lua_State object and executes Lua scripts

#ifndef LUA_SCRIPT_H
#define LUA_SCRIPT_H

extern "C" {
#include "lua.h"
}
#include "SDL2/SDL.h"
#include "StdString.h"

class LuaScript {
public:
	LuaScript (const StdString &script = StdString (""));
	~LuaScript ();

	// Read-only data members
	StdString script;
	int runResult;
	StdString runErrorText;

	// Run the provided LuaScript object and delete it when complete
	static void run (void *luaScriptPtr);

	// lua_register functions
	static int help (lua_State *L);
	static int print (lua_State *L);
	static int dofile (lua_State *L);
	static int quit (lua_State *L);
	static int sleep (lua_State *L);
	static int printservers (lua_State *L);
	static int attach (lua_State *L);
	static int detach (lua_State *L);
	static int info (lua_State *L);
	static int contact (lua_State *L);
	static int unlist (lua_State *L);

	struct Function {
		const lua_CFunction fn;
		const char *name;
		const char *parameters;
		const int helpText;
	};

	static void argvInteger (lua_State *L, int position, int *value);
	static void argvString (lua_State *L, int position, char **value);
	static void argvBoolean (lua_State *L, int position, bool *value);

	// sort predicate function
	static bool compareFunctions (const Function &a, const Function &b);

private:
	lua_State *state;
};

#endif
