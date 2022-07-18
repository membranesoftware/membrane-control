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
#include <string.h>
#include <list>
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "SDL2/SDL.h"
#include "App.h"
#include "Log.h"
#include "StdString.h"
#include "StringList.h"
#include "OsUtil.h"
#include "Agent.h"
#include "AgentControl.h"
#include "SystemInterface.h"
#include "UiText.h"
#include "AsyncCommand.h"
#include "UiStack.h"
#include "LuaScript.h"

int LuaScript::scriptTimeout = 7000; // ms
const int LuaScript::scriptWait = 100; // ms

const LuaScript::Function LuaFunctions[] = {
	{
		LuaScript::help,
		"help",
		"()",
		UiTextString::LuaScriptHelpHelpText
	},
	{
		LuaScript::quit,
		"quit",
		"()",
		UiTextString::LuaScriptQuitHelpText
	},
	{
		LuaScript::print,
		"print",
		"(...)",
		UiTextString::LuaScriptPrintHelpText
	},
	{
		LuaScript::sleep,
		"sleep",
		"(milliseconds)",
		UiTextString::LuaScriptSleepHelpText
	},
	{
		LuaScript::dofile,
		"dofile",
		"(filename)",
		UiTextString::LuaScriptDofileHelpText
	},
	{
		LuaScript::printservers,
		"printservers",
		"()",
		UiTextString::LuaScriptPrintserversHelpText
	},
	{
		LuaScript::attach,
		"attach",
		"(server)",
		UiTextString::LuaScriptAttachHelpText
	},
	{
		LuaScript::detach,
		"detach",
		"(server)",
		UiTextString::LuaScriptDetachHelpText
	},
	{
		LuaScript::info,
		"info",
		"(server)",
		UiTextString::LuaScriptInfoHelpText
	},
	{
		LuaScript::contact,
		"contact",
		"(address)",
		UiTextString::LuaScriptContactHelpText
	},
	{
		LuaScript::unlist,
		"unlist",
		"(server)",
		UiTextString::LuaScriptUnlistHelpText
	},
	{
		LuaScript::printcontrols,
		"printcontrols",
		"()",
		UiTextString::LuaScriptPrintcontrolsHelpText
	},
	{
		LuaScript::click,
		"click",
		"(controlName)",
		UiTextString::LuaScriptClickHelpText
	},
	{
		LuaScript::open,
		"open",
		"(windowName)",
		UiTextString::LuaScriptOpenHelpText
	},
	{
		LuaScript::target,
		"target",
		"(windowName)",
		UiTextString::LuaScriptTargetHelpText
	},
	{
		LuaScript::untarget,
		"untarget",
		"(windowName)",
		UiTextString::LuaScriptUntargetHelpText
	},
	{
		LuaScript::timeout,
		"timeout",
		"(milliseconds)",
		UiTextString::LuaScriptTimeoutHelpText
	},
	{
		LuaScript::topmenu,
		"topmenu",
		"(actionName)",
		UiTextString::LuaScriptTopmenuHelpText
	},
	{
		LuaScript::textvalue,
		"textvalue",
		"(controlName, value)",
		UiTextString::LuaScriptTextvalueHelpText
	}
};
const int LuaFunctionCount = sizeof (LuaFunctions) / sizeof (LuaScript::Function);

LuaScript::LuaScript (const StdString &script)
: script (script)
, runResult (-1)
, state (NULL)
{
	int i;

	state = luaL_newstate ();
	for (i = 0; i < LuaFunctionCount; ++i) {
		lua_register (state, LuaFunctions[i].name, LuaFunctions[i].fn);
	}
}

LuaScript::~LuaScript () {
	if (state) {
		lua_close (state);
		state = NULL;
	}
}

void LuaScript::run (void *luaScriptPtr) {
	LuaScript *lua;
	int result;

	lua = (LuaScript *) luaScriptPtr;
	result = luaL_loadstring (lua->state, lua->script.c_str ());
	if (result == LUA_ERRSYNTAX) {
		lua->runResult = OsUtil::MalformedDataError;
		Log::printf ("%s", UiText::instance->getText (UiTextString::LuaSyntaxErrorText).c_str ());
	}
	else if (result != LUA_OK) {
		lua->runResult = OsUtil::LuaOperationFailedError;
		Log::printf ("%s", UiText::instance->getText (UiTextString::LuaParseErrorText).c_str ());
	}
	else {
		result = lua_pcall (lua->state, 0, 0, 0);
		if (result != 0) {
			lua->runErrorText.assign (lua_tostring (lua->state, -1));
			lua->runResult = OsUtil::LuaOperationFailedError;
			Log::printf ("%s; %s", UiText::instance->getText (UiTextString::LuaScriptExecutionErrorText).c_str (), lua->runErrorText.c_str ());
		}
		else {
			lua->runResult = OsUtil::Success;
		}
	}

	delete (lua);
}

int LuaScript::help (lua_State *L) {
	std::list<LuaScript::Function> fns;
	std::list<LuaScript::Function>::iterator fi, end;
	StdString format, s;
	int i, w, maxw;

	maxw = 0;
	for (i = 0; i < LuaFunctionCount; ++i) {
		fns.push_back (LuaFunctions[i]);
		w = strlen (LuaFunctions[i].name) + strlen (LuaFunctions[i].parameters);
		if (w > maxw) {
			maxw = w;
		}
	}
	fns.sort (LuaScript::compareFunctions);

	maxw += 4;
	format.sprintf ("%%-%is%%s", maxw);
	fi = fns.begin ();
	end = fns.end ();
	while (fi != end) {
		s.sprintf ("%s%s", fi->name, fi->parameters);
		Log::printf (format.c_str (), s.c_str (), UiText::instance->getText (fi->helpText).c_str ());
		++fi;
	}
	Log::printf (" ");
	if (LuaScript::scriptTimeout > 0) {
		Log::printf ("* %s: %ims", UiText::instance->getText (UiTextString::FunctionTimeout).capitalized ().c_str (), LuaScript::scriptTimeout);
	}
	else {
		Log::printf ("* %s: %s", UiText::instance->getText (UiTextString::FunctionTimeout).capitalized ().c_str (), UiText::instance->getText (UiTextString::Disabled).c_str ());
	}
	Log::printf ("* %s", UiText::instance->getText (UiTextString::EnvironmentVariables).capitalized ().c_str ());
	format.sprintf ("%%-%is%%s", maxw);
	Log::printf (format.c_str (), "RUN_SCRIPT", UiText::instance->getText (UiTextString::LuaScriptRunScriptHelpText).c_str ());
	return (0);
}

bool LuaScript::compareFunctions (const Function &a, const Function &b) {
	return (strcmp (a.name, b.name) < 0);
}

void LuaScript::argvInteger (lua_State *L, int position, int *value) {
	int argc;
	char buf[512];

	argc = lua_gettop (L);
	if (argc < position) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		luaL_error (L, "%s", buf);
		return;
	}
	if (value) {
		*value = lua_tointeger (L, position);
	}
}

void LuaScript::argvString (lua_State *L, int position, char **value) {
	int argc;
	char buf[512], *s;

	argc = lua_gettop (L);
	if (argc < position) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		luaL_error (L, "%s", buf);
		return;
	}
	s = (char *) lua_tostring (L, position);
	if ((! s) || (s[0] == '\0')) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptEmptyStringArgumentErrorText).c_str ());
		luaL_error (L, "%s", buf);
		return;
	}
	if (value) {
		*value = s;
	}
}

void LuaScript::argvBoolean (lua_State *L, int position, bool *value) {
	int argc;
	char buf[512];

	argc = lua_gettop (L);
	if (argc < position) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		luaL_error (L, "%s", buf);
		return;
	}
	if (value) {
		*value = lua_toboolean (L, position);
	}
}

int LuaScript::print (lua_State *L) {
	StdString s;
	int argc, i;

	argc = lua_gettop (L);
	for (i = 1; i <= argc; ++i) {
		s.append (lua_tostring (L, i));
	}
	Log::printf ("%s", s.c_str ());
	return (0);
}

int LuaScript::dofile (lua_State *L) {
	LuaScript *lua;
	char *path, buf[4096];
	int result;

	LuaScript::argvString (L, 1, &path);
	buf[0] = '\0';
	lua = new LuaScript ();
	result = luaL_loadfile (lua->state, path);
	if (result == LUA_ERRFILE) {
		snprintf (buf, sizeof (buf), "%s: %s", UiText::instance->getText (UiTextString::LuaScriptLoadScriptFileErrorText).c_str (), path);
	}
	else if (result == LUA_ERRSYNTAX) {
		snprintf (buf, sizeof (buf), "%s: %s", UiText::instance->getText (UiTextString::LuaScriptLoadScriptSyntaxErrorText).c_str (), path);
	}
	else if (result != 0) {
		snprintf (buf, sizeof (buf), "%s: %s", UiText::instance->getText (UiTextString::LuaScriptLoadScriptErrorText).c_str (), path);
	}
	else {
		result = lua_pcall (lua->state, 0, 0, 0);
		if (result != 0) {
			snprintf (buf, sizeof (buf), "%s: %s", UiText::instance->getText (UiTextString::LuaScriptLoadScriptExecutionErrorText).c_str (), lua_tostring (lua->state, -1));
		}
	}

	delete (lua);
	if (buf[0] != '\0') {
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

int LuaScript::quit (lua_State *L) {
	App::instance->shutdown ();
	return (0);
}

int LuaScript::sleep (lua_State *L) {
	int ms;
	char buf[1024];

	LuaScript::argvInteger (L, 1, &ms);
	if (ms <= 0) {
		return (0);
	}
	if (App::instance->isShutdown || App::instance->isShuttingDown) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::ShuttingDown).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	SDL_Delay (ms);
	return (0);
}

int LuaScript::printservers (lua_State *L) {
	std::list<Agent> agents;
	std::list<Agent>::iterator i, end;
	StdString output, format, hostname;
	int displaynamew, appnamew, hostw;
	char status;

	AgentControl::instance->getAgents (&agents);
	if (agents.empty ()) {
		output.assign ("---- No servers connected ----");
	}
	else {
		displaynamew = 0;
		appnamew = 0;
		hostw = 0;
		agents.sort (Agent::compareAscending);
		i = agents.begin ();
		end = agents.end ();
		while (i != end) {
			if ((int) i->displayName.length () > displaynamew) {
				displaynamew = (int) i->displayName.length ();
			}
			if ((int) i->applicationName.length () > appnamew) {
				appnamew = (int) i->applicationName.length ();
			}

			hostname.assign (i->invokeHostname);
			if (hostname.empty ()) {
				hostname.assign (i->urlHostname);
			}
			if ((int) hostname.length () > hostw) {
				hostw = (int) hostname.length ();
			}
			++i;
		}

		displaynamew += 4;
		appnamew += 4;
		hostw += 4;
		format.sprintf ("\n%%-%is%%c    %%-%is%%-%is\n  %%s", displaynamew, appnamew, hostw);
		output.sprintf ("---- Server list (%i) ----\n# Name / Status / Application / Address / ID\n#        x=detached o=online !=contact failed\n#", (int) agents.size ());
		i = agents.begin ();
		end = agents.end ();
		while (i != end) {
			if (i->isAttached) {
				status = i->isContacted () ? 'o' : '!';
			}
			else {
				status = 'x';
			}
			hostname.assign (i->invokeHostname);
			if (hostname.empty ()) {
				hostname.assign (i->urlHostname);
			}
			output.appendSprintf (format.c_str (), i->displayName.c_str (), status, i->applicationName.c_str (), hostname.c_str (), i->id.c_str ());
			++i;
		}
		output.appendSprintf ("\n---- End server list (%i) ----", (int) agents.size ());
	}
	Log::printf ("%s", output.c_str ());
	return (0);
}

static int setAgentAttached (lua_State *L, bool isAttached) {
	Agent *agent;
	char *server, buf[1024];
	int argc, result;

	argc = lua_gettop (L);
	if (argc < 1) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	server = (char *) lua_tostring (L, 1);
	if ((! server) || (server[0] == '\0')) {
		snprintf (buf, sizeof (buf), "server %s", UiText::instance->getText (UiTextString::LuaScriptEmptyArgumentErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}

	agent = new Agent ();
	result = AgentControl::instance->findTargetAgent (StdString (server), agent);
	if (result == OsUtil::Success) {
		Log::debug ("lua %s; id=%s", isAttached ? "attach" : "detach", agent->id.c_str ());
		AgentControl::instance->setAgentAttached (agent->id, isAttached);
	}
	delete (agent);

	if (result == OsUtil::KeyNotFoundError) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptTargetServerNotFoundErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	if (result == OsUtil::DuplicateIdError) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMultipleServersFoundErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	if (result != OsUtil::Success) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptInternalErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}
int LuaScript::attach (lua_State *L) {
	return (setAgentAttached (L, true));
}
int LuaScript::detach (lua_State *L) {
	return (setAgentAttached (L, false));
}

int LuaScript::info (lua_State *L) {
	Agent *agent;
	char *server, buf[1024];
	int argc, result;
	StdString lastcontact;

	argc = lua_gettop (L);
	if (argc < 1) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	server = (char *) lua_tostring (L, 1);
	if ((! server) || (server[0] == '\0')) {
		snprintf (buf, sizeof (buf), "server %s", UiText::instance->getText (UiTextString::LuaScriptEmptyArgumentErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}

	agent = new Agent ();
	result = AgentControl::instance->findTargetAgent (StdString (server), agent);
	if (result == OsUtil::Success) {
		if (agent->isContacted ()) {
			lastcontact.assign (OsUtil::getTimestampString (agent->lastStatusTime), true);
		}
		Log::printf ("---- Server info ----\nName: %s\nID: %s\nApplication: %s\nVersion: %s\nLast contact: %s\n---- End server info ----", agent->displayName.c_str (), agent->id.c_str (), agent->applicationName.c_str (), agent->version.c_str (), lastcontact.c_str ());
	}
	delete (agent);

	if (result == OsUtil::KeyNotFoundError) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptTargetServerNotFoundErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	if (result == OsUtil::DuplicateIdError) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMultipleServersFoundErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	if (result != OsUtil::Success) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptInternalErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

int LuaScript::contact (lua_State *L) {
	char *address, buf[1024];
	int argc, port;
	StdString *hostname;

	argc = lua_gettop (L);
	if (argc < 1) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	address = (char *) lua_tostring (L, 1);
	if ((! address) || (address[0] == '\0')) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptEmptyAddressArgumentErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}

	buf[0] = '\0';
	hostname = new StdString ();
	if (! StdString (address).parseAddress (hostname, &port, SystemInterface::Constant_DefaultTcpPort1)) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptAddressParseErrorText).c_str ());
	}
	else {
		if (AgentControl::instance->isHostContacted (*hostname, port)) {
			AgentControl::instance->refreshAgentStatus (*hostname, port);
		}
		else {
			AgentControl::instance->contactAgent (*hostname, port);
			Log::info ("%s: %s", UiText::instance->getText (UiTextString::ServerUiContactingAgentMessage).c_str (), address);
		}
	}
	delete (hostname);

	if (buf[0] != '\0') {
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

int LuaScript::unlist (lua_State *L) {
	Agent *agent;
	char *server, buf[1024];
	int argc, result;

	argc = lua_gettop (L);
	if (argc < 1) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	server = (char *) lua_tostring (L, 1);
	if ((! server) || (server[0] == '\0')) {
		snprintf (buf, sizeof (buf), "server %s", UiText::instance->getText (UiTextString::LuaScriptEmptyArgumentErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}

	agent = new Agent ();
	result = AgentControl::instance->findTargetAgent (StdString (server), agent);
	if (result == OsUtil::Success) {
		Log::debug ("lua unlist; id=%s", agent->id.c_str ());
		AgentControl::instance->removeAgent (agent->id);
	}
	delete (agent);

	if (result == OsUtil::KeyNotFoundError) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptTargetServerNotFoundErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	if (result == OsUtil::DuplicateIdError) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptMultipleServersFoundErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	if (result != OsUtil::Success) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextString::LuaScriptInternalErrorText).c_str ());
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

int LuaScript::printcontrols (lua_State *L) {
	StringList names;

	App::instance->uiStack.getWidgetNames (&names);
	if (names.empty ()) {
		Log::printf ("---- No addressable controls found ----");
	}
	else {
		Log::printf ("---- Control names (%i) ----\n%s", (int) names.size (), names.join (" ").c_str ());
	}
	return (0);
}

bool LuaScript::awaitResult (LuaScript::AwaitResultFn fn, void *fnData) {
	int64_t tmax;
	bool result;

	result = false;
	tmax = OsUtil::getTime () + LuaScript::scriptTimeout;
	while (! result) {
		result = fn (fnData);
		if (result) {
			return (true);
		}
		if (LuaScript::scriptTimeout <= 0) {
			break;
		}
		SDL_Delay (LuaScript::scriptWait);
		if (App::instance->isShutdown || App::instance->isShuttingDown || (OsUtil::getTime () >= tmax)) {
			break;
		}
	}
	return (false);
}

static bool awaitResult_click (void *data) {
	return (App::instance->uiStack.clickWidget (StdString ((char *) data)));
}
int LuaScript::click (lua_State *L) {
	char *name, buf[1024];
	bool result;

	LuaScript::argvString (L, 1, &name);
	result = LuaScript::awaitResult (awaitResult_click, name);
	if (! result) {
		snprintf (buf, sizeof (buf), "click: control not found; controlName=\"%s\"", name);
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

static bool awaitResult_open (void *data) {
	return (App::instance->uiStack.openWidget (StdString ((char *) data)));
}
int LuaScript::open (lua_State *L) {
	char *name, buf[1024];
	bool result;

	LuaScript::argvString (L, 1, &name);
	result = LuaScript::awaitResult (awaitResult_open, name);
	if (! result) {
		snprintf (buf, sizeof (buf), "open: window not found; windowName=\"%s\"", name);
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

static bool awaitResult_target (void *data) {
	return (App::instance->uiStack.selectWidget (StdString ((char *) data)));
}
int LuaScript::target (lua_State *L) {
	char *name, buf[1024];
	bool result;

	LuaScript::argvString (L, 1, &name);
	result = LuaScript::awaitResult (awaitResult_target, name);
	if (! result) {
		snprintf (buf, sizeof (buf), "target: window not found; windowName=\"%s\"", name);
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

static bool awaitResult_untarget (void *data) {
	return (App::instance->uiStack.unselectWidget (StdString ((char *) data)));
}
int LuaScript::untarget (lua_State *L) {
	char *name, buf[1024];
	bool result;

	LuaScript::argvString (L, 1, &name);
	result = LuaScript::awaitResult (awaitResult_untarget, name);
	if (! result) {
		snprintf (buf, sizeof (buf), "untarget: window not found; windowName=\"%s\"", name);
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

static bool awaitResult_topmenu (void *data) {
	return (App::instance->uiStack.executeMainMenuAction ((char *) data));
}
int LuaScript::topmenu (lua_State *L) {
	char *action, buf[1024];
	bool result;

	LuaScript::argvString (L, 1, &action);
	result = LuaScript::awaitResult (awaitResult_topmenu, action);
	if (! result) {
		snprintf (buf, sizeof (buf), "topmenu: action not found; action=\"%s\"", action);
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

static bool awaitResult_textvalue (void *data) {
	char **args;

	args = (char **) data;
	return (App::instance->uiStack.setTextFieldValue (StdString (args[0]), StdString (args[1])));
}
int LuaScript::textvalue (lua_State *L) {
	char *args[2], buf[1024];
	bool result;

	LuaScript::argvString (L, 1, &(args[0]));
	LuaScript::argvString (L, 2, &(args[1]));
	result = LuaScript::awaitResult (awaitResult_textvalue, args);
	if (! result) {
		snprintf (buf, sizeof (buf), "textvalue: textfield not found; controlName=\"%s\"", args[0]);
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

int LuaScript::timeout (lua_State *L) {
	char buf[1024];
	int ms;

	LuaScript::argvInteger (L, 1, &ms);
	if (ms < 0) {
		snprintf (buf, sizeof (buf), "milliseconds must be 0 or greater (0 disables timeout)");
		return (luaL_error (L, "%s", buf));
	}
	LuaScript::scriptTimeout = ms;
	return (0);
}
