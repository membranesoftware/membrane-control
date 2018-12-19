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
#include "Util.h"
#include "StdString.h"
#include "Json.h"
#include "SystemInterface.h"
#include "Agent.h"

Agent::Agent ()
: invokeTcpPort1 (0)
, invokeTcpPort2 (0)
, tcpPort1 (0)
, tcpPort2 (0)
{

}

Agent::~Agent () {

}

StdString Agent::toString () {
	StdString s;

	s.sprintf ("<Agent %s", id.c_str ());
	if (! invokeHostname.empty ()) {
		s.appendSprintf (" invokeHostname=\"%s\"", invokeHostname.c_str ());
	}
	if (invokeTcpPort1 > 0) {
		s.appendSprintf (" invokeTcpPort1=%i", invokeTcpPort1);
	}
	if (invokeTcpPort2 > 0) {
		s.appendSprintf (" invokeTcpPort2=%i", invokeTcpPort2);
	}
	if (! linkPath.empty ()) {
		s.appendSprintf (" linkPath=\"%s\"", linkPath.c_str ());
	}
	if (! displayName.empty ()) {
		s.appendSprintf (" displayName=\"%s\"", displayName.c_str ());
	}
	if (! urlHostname.empty ()) {
		s.appendSprintf (" urlHostname=\"%s\"", urlHostname.c_str ());
	}
	if (tcpPort1 > 0) {
		s.appendSprintf (" tcpPort1=%i", tcpPort1);
	}
	if (tcpPort2 > 0) {
		s.appendSprintf (" tcpPort2=%i", tcpPort2);
	}

	s.append (">");
	return (s);
}

void Agent::readCommand (Json *command) {
	App *app;
	SystemInterface *interface;
	StdString val;
	int commandid, i;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	commandid = interface->getCommandId (command);
	if (commandid != SystemInterface::Command_AgentStatus) {
		return;
	}

	val = interface->getCommandStringParam (command, "linkPath", "");
	if (! val.empty ()) {
		linkPath.assign (val);
	}

	val = interface->getCommandStringParam (command, "displayName", "");
	if (! val.empty ()) {
		displayName.assign (val);
	}

	val = interface->getCommandStringParam (command, "urlHostname", "");
	if (! val.empty ()) {
		urlHostname.assign (val);
	}

	i = interface->getCommandNumberParam (command, "tcpPort1", 0);
	if (i > 0) {
		tcpPort1 = i;
	}

	i = interface->getCommandNumberParam (command, "tcpPort2", 0);
	if (i > 0) {
		tcpPort2 = i;
	}
}

StdString Agent::getInvokeUrl () {
	StdString hostname;
	int port;

	if (! invokeHostname.empty ()) {
		hostname.assign (invokeHostname);
	}
	else if (! urlHostname.empty ()) {
		hostname.assign (urlHostname);
	}
	if (hostname.empty ()) {
		return (StdString (""));
	}

	port = invokeTcpPort1;
	if (port <= 0) {
		port = tcpPort1;
	}
	if (port <= 0) {
		port = SystemInterface::Constant_DefaultTcpPort1;
	}
	return (StdString::createSprintf ("%s://%s:%i", App::getInstance ()->isHttpsEnabled ? "https" : "http", hostname.c_str (), port));
}

StdString Agent::getSecondaryUrl () {
	StdString hostname;
	int port;

	if (! invokeHostname.empty ()) {
		hostname.assign (invokeHostname);
	}
	else if (! urlHostname.empty ()) {
		hostname.assign (urlHostname);
	}
	if (hostname.empty ()) {
		return (StdString (""));
	}

	port = invokeTcpPort2;
	if (port <= 0) {
		port = tcpPort2;
	}
	if (port <= 0) {
		port = SystemInterface::Constant_DefaultTcpPort2;
	}
	return (StdString::createSprintf ("http://%s:%i", hostname.c_str (), port));
}

StdString Agent::getLinkUrl () {
	StdString url;

	if (linkPath.empty ()) {
		return (StdString (""));
	}

	url = getInvokeUrl ();
	url.appendSprintf ("%s/?transport=websocket", linkPath.c_str ());

	return (url);
}

static const char *AGENT_ID_KEY = "id";
static const char *INVOKE_HOSTNAME_KEY = "ih";
static const char *INVOKE_TCP_PORT1_KEY = "i1";
static const char *INVOKE_TCP_PORT2_KEY = "i2";
static const char *LINK_PATH_KEY = "lp";
static const char *DISPLAY_NAME_KEY = "dn";
static const char *URL_HOSTNAME_KEY = "uh";
static const char *TCP_PORT1_KEY = "t1";
static const char *TCP_PORT2_KEY = "t2";

StdString Agent::toPrefsJsonString () {
	Json *obj;
	StdString s;

	obj = new Json ();

	obj->set (AGENT_ID_KEY, id);
	if (! invokeHostname.empty ()) {
		obj->set (INVOKE_HOSTNAME_KEY, invokeHostname);
	}
	if (invokeTcpPort1 > 0) {
		obj->set (INVOKE_TCP_PORT1_KEY, invokeTcpPort1);
	}
	if (invokeTcpPort2 > 0) {
		obj->set (INVOKE_TCP_PORT2_KEY, invokeTcpPort2);
	}
	if (! linkPath.empty ()) {
		obj->set (LINK_PATH_KEY, linkPath);
	}
	if (! displayName.empty ()) {
		obj->set (DISPLAY_NAME_KEY, displayName);
	}
	if (! urlHostname.empty ()) {
		obj->set (URL_HOSTNAME_KEY, urlHostname);
	}
	if (tcpPort1 > 0) {
		obj->set (TCP_PORT1_KEY, tcpPort1);
	}
	if (tcpPort2 > 0) {
		obj->set (TCP_PORT2_KEY, tcpPort2);
	}

	s = obj->toString ();
	delete (obj);

	return (s);
}

int Agent::readPrefsJson (const StdString &prefsJson) {
	Json *obj;
	int result;

	obj = new Json ();
	result = obj->parse (prefsJson);
	if (result == Result::SUCCESS) {
		id = obj->getString (AGENT_ID_KEY, "");
		if (id.empty ()) {
			result = Result::ERROR_MALFORMED_DATA;
		}
	}
	if (result == Result::SUCCESS) {
		invokeHostname = obj->getString (INVOKE_HOSTNAME_KEY, "");
		invokeTcpPort1 = obj->getNumber (INVOKE_TCP_PORT1_KEY, (int) 0);
		invokeTcpPort2 = obj->getNumber (INVOKE_TCP_PORT2_KEY, (int) 0);
		linkPath = obj->getString (LINK_PATH_KEY, "");
		displayName = obj->getString (DISPLAY_NAME_KEY, "");
		urlHostname = obj->getString (URL_HOSTNAME_KEY, "");
		tcpPort1 = obj->getNumber (TCP_PORT1_KEY, (int) 0);
		tcpPort2 = obj->getNumber (TCP_PORT2_KEY, (int) 0);
	}
	delete (obj);

	return (result);
}
