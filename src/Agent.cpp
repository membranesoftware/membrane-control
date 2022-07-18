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
#include "App.h"
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

void Agent::copyDisplayData (const Agent &other) {
	id.assign (other.id);
	isAttached = other.isAttached;
	lastStatusTime = other.lastStatusTime;
	invokeHostname.assign (other.invokeHostname);
	invokeTcpPort1 = other.invokeTcpPort1;
	invokeTcpPort2 = other.invokeTcpPort2;
	serverType = other.serverType;
	displayName.assign (other.displayName);
	applicationName.assign (other.applicationName);
	urlHostname.assign (other.urlHostname);
	tcpPort1 = other.tcpPort1;
	tcpPort2 = other.tcpPort2;
	version.assign (other.version);
	platform.assign (other.platform);
}

void Agent::readCommand (Json *command) {
	Json serverstatus;
	StdString val;
	int commandid, i;

	commandid = SystemInterface::instance->getCommandId (command);
	if (commandid != SystemInterface::CommandId_AgentStatus) {
		return;
	}

	val = SystemInterface::instance->getCommandStringParam (command, "linkPath", "");
	if (! val.empty ()) {
		linkPath.assign (val);
	}

	val = SystemInterface::instance->getCommandStringParam (command, "displayName", "");
	if (! val.empty ()) {
		displayName.assign (val);
	}

	val = SystemInterface::instance->getCommandStringParam (command, "applicationName", "");
	if (! val.empty ()) {
		applicationName.assign (val);
	}

	val = SystemInterface::instance->getCommandStringParam (command, "version", "");
	if (! val.empty ()) {
		version.assign (val);
	}

	val = SystemInterface::instance->getCommandStringParam (command, "platform", "");
	if (! val.empty ()) {
		platform.assign (val);
	}

	val = SystemInterface::instance->getCommandStringParam (command, "urlHostname", "");
	if (! val.empty ()) {
		urlHostname.assign (val);
		if (invokeHostname.empty ()) {
			invokeHostname.assign (urlHostname);
		}
	}

	i = SystemInterface::instance->getCommandNumberParam (command, "tcpPort1", 0);
	if (i > 0) {
		tcpPort1 = i;
		if (invokeTcpPort1 <= 0) {
			invokeTcpPort1 = tcpPort1;
		}
	}

	i = SystemInterface::instance->getCommandNumberParam (command, "tcpPort2", 0);
	if (i > 0) {
		tcpPort2 = i;
	}

	if (SystemInterface::instance->getCommandObjectParam (command, "monitorServerStatus", &serverstatus)) {
		serverType = Agent::Monitor;
	}
	else if (SystemInterface::instance->getCommandObjectParam (command, "mediaServerStatus", &serverstatus)) {
		serverType = Agent::Media;
	}
	else if (SystemInterface::instance->getCommandObjectParam (command, "cameraServerStatus", &serverstatus)) {
		serverType = Agent::Camera;
	}
	else {
		serverType = -1;
	}
}

bool Agent::isContacted () {
	return (isAttached && (lastStatusTime > 0));
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

	url.appendSprintf ("%s/?transport=websocket&EIO=3", linkPath.c_str ());
	return (url);
}

StdString Agent::getApplicationNewsUrl () {
	if (version.empty () || platform.empty ()) {
		return (StdString (""));
	}
	return (App::getApplicationNewsUrl (StdString::createSprintf ("%s_%s_%s", version.c_str (), platform.c_str (), OsUtil::getEnvLanguage ("en").c_str ())));
}

Json *Agent::createState () {
	Json *state;

	state = new Json ();
	state->set (Agent::AgentIdKey, id);
	if (! invokeHostname.empty ()) {
		state->set (Agent::InvokeHostnameKey, invokeHostname);
	}
	if (invokeTcpPort1 > 0) {
		state->set (Agent::InvokeTcpPort1Key, invokeTcpPort1);
	}
	if (invokeTcpPort2 > 0) {
		state->set (Agent::InvokeTcpPort2Key, invokeTcpPort2);
	}
	if (! linkPath.empty ()) {
		state->set (Agent::LinkPathKey, linkPath);
	}
	if (! displayName.empty ()) {
		state->set (Agent::DisplayNameKey, displayName);
	}
	if (! applicationName.empty ()) {
		state->set (Agent::ApplicationNameKey, applicationName);
	}
	if (serverType >= 0) {
		state->set (Agent::ServerTypeKey, serverType);
	}
	if (! urlHostname.empty ()) {
		state->set (Agent::UrlHostnameKey, urlHostname);
	}
	if (tcpPort1 > 0) {
		state->set (Agent::TcpPort1Key, tcpPort1);
	}
	if (tcpPort2 > 0) {
		state->set (Agent::TcpPort2Key, tcpPort2);
	}
	state->set (Agent::IsAttachedKey, isAttached);

	return (state);
}

int Agent::readState (Json *state) {
	id = state->getString (Agent::AgentIdKey, "");
	if (id.empty ()) {
		return (OsUtil::MalformedDataError);
	}

	isAttached = state->getBoolean (Agent::IsAttachedKey, false);
	invokeHostname = state->getString (Agent::InvokeHostnameKey, "");
	invokeTcpPort1 = state->getNumber (Agent::InvokeTcpPort1Key, (int) 0);
	invokeTcpPort2 = state->getNumber (Agent::InvokeTcpPort2Key, (int) 0);
	linkPath = state->getString (Agent::LinkPathKey, "");
	displayName = state->getString (Agent::DisplayNameKey, "");
	applicationName = state->getString (Agent::ApplicationNameKey, "");
	serverType = state->getNumber (Agent::ServerTypeKey, (int) -1);
	urlHostname = state->getString (Agent::UrlHostnameKey, "");
	tcpPort1 = state->getNumber (Agent::TcpPort1Key, (int) 0);
	tcpPort2 = state->getNumber (Agent::TcpPort2Key, (int) 0);

	return (OsUtil::Success);
}

StdString Agent::getCommandAgentName (Json *command) {
	StdString name;

	if (SystemInterface::instance->getCommandId (command) != SystemInterface::CommandId_AgentStatus) {
		return (StdString (""));
	}
	name = SystemInterface::instance->getCommandStringParam (command, "displayName", "");
	if (! name.empty ()) {
		return (name);
	}
	name = SystemInterface::instance->getCommandStringParam (command, "urlHostname", "");
	if (! name.empty ()) {
		return (name);
	}
	name = SystemInterface::instance->getCommandAgentId (command);
	if (! name.empty ()) {
		return (name);
	}
	return (StdString (""));
}

StdString Agent::getCommandAgentAddress (Json *command) {
	StdString hostname;
	int port;

	if (SystemInterface::instance->getCommandId (command) != SystemInterface::CommandId_AgentStatus) {
		return (StdString (""));
	}
	hostname = SystemInterface::instance->getCommandStringParam (command, "urlHostname", "");
	if (hostname.empty ()) {
		return (StdString (""));
	}
	port = SystemInterface::instance->getCommandNumberParam (command, "tcpPort1", SystemInterface::Constant_DefaultTcpPort1);
	if ((port < 0) || (port > 65535)) {
		return (StdString (""));
	}
	return (StdString::createSprintf ("%s:%i", hostname.c_str (), port));
}

bool Agent::compareAscending (const Agent &a, const Agent &b) {
	return (a.displayName.lowercased ().compare (b.displayName.lowercased ()) < 0);
}
