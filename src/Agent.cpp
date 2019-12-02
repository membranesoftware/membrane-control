/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "StdString.h"
#include "Json.h"
#include "SystemInterface.h"
#include "OsUtil.h"
#include "Agent.h"

const char *Agent::AgentIdKey = "a";
const char *Agent::InvokeHostnameKey = "b";
const char *Agent::InvokeTcpPort1Key = "c";
const char *Agent::InvokeTcpPort2Key = "d";
const char *Agent::LinkPathKey = "e";
const char *Agent::DisplayNameKey = "f";
const char *Agent::UrlHostnameKey = "g";
const char *Agent::TcpPort1Key = "h";
const char *Agent::TcpPort2Key = "i";
const char *Agent::IsAttachedKey = "j";
const char *Agent::ApplicationNameKey = "k";
const char *Agent::ServerTypeKey = "l";

Agent::Agent ()
: isAttached (false)
, lastStatusTime (0)
, invokeTcpPort1 (0)
, invokeTcpPort2 (0)
, serverType (-1)
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
	if (! applicationName.empty ()) {
		s.appendSprintf (" applicationName=\"%s\"", applicationName.c_str ());
	}
	if (! version.empty ()) {
		s.appendSprintf (" version=\"%s\"", version.c_str ());
	}
	if (! platform.empty ()) {
		s.appendSprintf (" platform=\"%s\"", platform.c_str ());
	}
	if (serverType >= 0) {
		s.appendSprintf (" serverType=%i", serverType);
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
	if (lastStatusTime > 0) {
		s.appendSprintf (" lastStatusTime=%lli", (long long int) lastStatusTime);
	}
	s.appendSprintf (" isAttached=%s", BOOL_STRING (isAttached));

	s.append (">");
	return (s);
}

void Agent::readCommand (Json *command) {
	SystemInterface *interface;
	Json serverstatus;
	StdString val;
	int commandid, i;

	interface = &(App::instance->systemInterface);
	commandid = interface->getCommandId (command);
	if (commandid != SystemInterface::CommandId_AgentStatus) {
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

	val = interface->getCommandStringParam (command, "applicationName", "");
	if (! val.empty ()) {
		applicationName.assign (val);
	}

	val = interface->getCommandStringParam (command, "version", "");
	if (! val.empty ()) {
		version.assign (val);
	}

	val = interface->getCommandStringParam (command, "platform", "");
	if (! val.empty ()) {
		platform.assign (val);
	}

	val = interface->getCommandStringParam (command, "urlHostname", "");
	if (! val.empty ()) {
		urlHostname.assign (val);
		if (invokeHostname.empty ()) {
			invokeHostname.assign (urlHostname);
		}
	}

	i = interface->getCommandNumberParam (command, "tcpPort1", 0);
	if (i > 0) {
		tcpPort1 = i;
		if (invokeTcpPort1 <= 0) {
			invokeTcpPort1 = tcpPort1;
		}
	}

	i = interface->getCommandNumberParam (command, "tcpPort2", 0);
	if (i > 0) {
		tcpPort2 = i;
	}

	if (interface->getCommandObjectParam (command, "monitorServerStatus", &serverstatus)) {
		serverType = SystemInterface::Constant_Monitor;
	}
	else if (interface->getCommandObjectParam (command, "mediaServerStatus", &serverstatus)) {
		serverType = SystemInterface::Constant_Media;
	}
	else if (interface->getCommandObjectParam (command, "cameraServerStatus", &serverstatus)) {
		serverType = SystemInterface::Constant_Camera;
	}
	else {
		serverType = -1;
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
	return (StdString::createSprintf ("%s://%s:%i", App::instance->isHttpsEnabled ? "https" : "http", hostname.c_str (), port));
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
	if (url.empty ()) {
		return (StdString (""));
	}

	url.appendSprintf ("%s/?transport=websocket", linkPath.c_str ());

	return (url);
}

StdString Agent::getApplicationNewsUrl () {
	if (version.empty () || platform.empty ()) {
		return (StdString (""));
	}
	return (App::getApplicationNewsUrl (StdString::createSprintf ("%s_%s_%s", version.c_str (), platform.c_str (), OsUtil::getEnvLanguage ("en").c_str ())));
}

StdString Agent::toPrefsJsonString () {
	Json *obj;
	StdString s;

	obj = new Json ();

	obj->set (Agent::AgentIdKey, id);
	if (! invokeHostname.empty ()) {
		obj->set (Agent::InvokeHostnameKey, invokeHostname);
	}
	if (invokeTcpPort1 > 0) {
		obj->set (Agent::InvokeTcpPort1Key, invokeTcpPort1);
	}
	if (invokeTcpPort2 > 0) {
		obj->set (Agent::InvokeTcpPort2Key, invokeTcpPort2);
	}
	if (! linkPath.empty ()) {
		obj->set (Agent::LinkPathKey, linkPath);
	}
	if (! displayName.empty ()) {
		obj->set (Agent::DisplayNameKey, displayName);
	}
	if (! applicationName.empty ()) {
		obj->set (Agent::ApplicationNameKey, applicationName);
	}
	if (serverType >= 0) {
		obj->set (Agent::ServerTypeKey, serverType);
	}
	if (! urlHostname.empty ()) {
		obj->set (Agent::UrlHostnameKey, urlHostname);
	}
	if (tcpPort1 > 0) {
		obj->set (Agent::TcpPort1Key, tcpPort1);
	}
	if (tcpPort2 > 0) {
		obj->set (Agent::TcpPort2Key, tcpPort2);
	}
	obj->set (Agent::IsAttachedKey, isAttached);

	s = obj->toString ();
	delete (obj);

	return (s);
}

int Agent::readPrefsJson (const StdString &prefsJson) {
	Json *obj;
	int result;

	obj = new Json ();
	result = Result::Success;
	if (! obj->parse (prefsJson)) {
		result = Result::JsonParseFailedError;
	}
	else {
		id = obj->getString (Agent::AgentIdKey, "");
		if (id.empty ()) {
			result = Result::MalformedDataError;
		}
	}

	if (result == Result::Success) {
		isAttached = obj->getBoolean (Agent::IsAttachedKey, false);
		invokeHostname = obj->getString (Agent::InvokeHostnameKey, "");
		invokeTcpPort1 = obj->getNumber (Agent::InvokeTcpPort1Key, (int) 0);
		invokeTcpPort2 = obj->getNumber (Agent::InvokeTcpPort2Key, (int) 0);
		linkPath = obj->getString (Agent::LinkPathKey, "");
		displayName = obj->getString (Agent::DisplayNameKey, "");
		applicationName = obj->getString (Agent::ApplicationNameKey, "");
		serverType = obj->getNumber (Agent::ServerTypeKey, (int) -1);
		urlHostname = obj->getString (Agent::UrlHostnameKey, "");
		tcpPort1 = obj->getNumber (Agent::TcpPort1Key, (int) 0);
		tcpPort2 = obj->getNumber (Agent::TcpPort2Key, (int) 0);
	}
	delete (obj);

	return (result);
}
