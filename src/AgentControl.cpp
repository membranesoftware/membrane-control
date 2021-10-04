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
#include <map>
#include <vector>
#include <list>
#include "SDL2/SDL.h"
#include "openssl/evp.h"
#include "App.h"
#include "Log.h"
#include "Network.h"
#include "OsUtil.h"
#include "StdString.h"
#include "Json.h"
#include "SystemInterface.h"
#include "HashMap.h"
#include "StringList.h"
#include "Ui.h"
#include "LinkClient.h"
#include "CommandList.h"
#include "CommandListener.h"
#include "RecordStore.h"
#include "AgentControl.h"

AgentControl *AgentControl::instance = NULL;

const char *AgentControl::AgentStatusKey = "AgentStatus";
const char *AgentControl::AgentServerNameKey = "AgentServerName";
const char *AgentControl::ServerAdminSecretsKey = "ServerAdminSecrets";
const char *AgentControl::NameKey = "a";
const char *AgentControl::SecretKey = "b";
const char *AgentControl::AuthorizePathKey = "c";
const char *AgentControl::AuthorizeSecretKey = "d";

const int AgentControl::CommandListIdleTimeout = 60000; // ms

AgentControl::AgentControl ()
: agentDatagramPort (SystemInterface::Constant_DefaultUdpPort)
, urlHostname ("")
, agentServerName ("")
, isStarted (false)
, linkClient (this, AgentControl::linkClientConnect, AgentControl::linkClientDisconnect, AgentControl::linkClientCommand)
, agentMapMutex (NULL)
, commandMapMutex (NULL)
, adminSecretMutex (NULL)
{
	agentMapMutex = SDL_CreateMutex ();
	commandMapMutex = SDL_CreateMutex ();
	adminSecretMutex = SDL_CreateMutex ();
}

AgentControl::~AgentControl () {
	clearCommandMap ();
	if (agentMapMutex) {
		SDL_DestroyMutex (agentMapMutex);
		agentMapMutex = NULL;
	}
	if (commandMapMutex) {
		SDL_DestroyMutex (commandMapMutex);
		commandMapMutex = NULL;
	}
	if (adminSecretMutex) {
		SDL_DestroyMutex (adminSecretMutex);
		adminSecretMutex = NULL;
	}
}

void AgentControl::clearCommandMap () {
	std::map<StdString, CommandList *>::iterator i, end;

	SDL_LockMutex (commandMapMutex);
	i = commandMap.begin ();
	end = commandMap.end ();
	while (i != end) {
		if (i->second) {
			delete (i->second);
			i->second = NULL;
		}
		++i;
	}
	commandMap.clear ();
	SDL_UnlockMutex (commandMapMutex);
}

int AgentControl::start () {
	std::map<StdString, Agent>::iterator i, end;
	StringList idlist;
	StringList::iterator j, jend;

	if (isStarted) {
		return (OsUtil::Result::Success);
	}
	readPrefs ();
	linkClient.start ();

	isStarted = true;

	CommandListener::instance->subscribe (SystemInterface::CommandId_AgentStatus, CommandListener::CommandCallbackContext (AgentControl::receiveAgentStatus, this));
	CommandListener::instance->subscribe (SystemInterface::CommandId_TaskItem, CommandListener::CommandCallbackContext (AgentControl::receiveTaskItem, this));

	SDL_LockMutex (agentMapMutex);
	i = agentMap.begin ();
	end = agentMap.end ();
	while (i != end) {
		if (i->second.isAttached) {
			idlist.push_back (i->first);
		}
		++i;
	}
	SDL_UnlockMutex (agentMapMutex);

	j = idlist.begin ();
	jend = idlist.end ();
	while (j != jend) {
		refreshAgentStatus (*j);
		++j;
	}

	return (OsUtil::Result::Success);
}

void AgentControl::stop () {
	CommandListener::instance->unsubscribe (SystemInterface::CommandId_AgentStatus, CommandListener::CommandCallbackContext (AgentControl::receiveAgentStatus, this));
	CommandListener::instance->unsubscribe (SystemInterface::CommandId_TaskItem, CommandListener::CommandCallbackContext (AgentControl::receiveTaskItem, this));
	linkClient.clear ();

	SDL_LockMutex (adminSecretMutex);
	adminSecretList.clear ();
	SDL_UnlockMutex (adminSecretMutex);

	SDL_LockMutex (agentMapMutex);
	agentMap.clear ();
	agentUpdateUrlMap.clear ();
	SDL_UnlockMutex (agentMapMutex);
}

void AgentControl::update (int msElapsed) {
	std::map<StdString, CommandList *>::iterator mi, mend;
	StringList keys;
	StringList::iterator ki, kend;

	SDL_LockMutex (commandMapMutex);
	mi = commandMap.begin ();
	mend = commandMap.end ();
	while (mi != mend) {
		if (mi->second->isIdle (AgentControl::CommandListIdleTimeout) && mi->second->empty ()) {
			keys.push_back (mi->first);
		}
		++mi;
	}
	if (! keys.empty ()) {
		ki = keys.begin ();
		kend = keys.end ();
		while (ki != kend) {
			mi = commandMap.find (*ki);
			if (mi != commandMap.end ()) {
				if (mi->second) {
					delete (mi->second);
					mi->second = NULL;
				}
				commandMap.erase (mi);
			}
			++ki;
		}
	}
	SDL_UnlockMutex (commandMapMutex);

	linkClient.update (msElapsed);
}

void AgentControl::connectLinkClient (const StdString &agentId) {
	StdString url;

	url = getAgentLinkUrl (agentId);
	if (url.empty ()) {
		Log::warning ("Failed to connect link client, URL not found; agentId=\"%s\"", agentId.c_str ());
		return;
	}

	linkClient.connect (agentId, url);
}

void AgentControl::getAgents (std::list<Agent> *destList) {
	std::map<StdString, Agent>::iterator i, end;
	Agent agent;

	destList->clear ();
	SDL_LockMutex (agentMapMutex);
	i = agentMap.begin ();
	end = agentMap.end ();
	while (i != end) {
		agent.copyDisplayData (i->second);
		destList->push_back (agent);
		++i;
	}
	SDL_UnlockMutex (agentMapMutex);
}

void AgentControl::getAgentIds (StringList *destList) {
	std::map<StdString, Agent>::iterator i, end;

	destList->clear ();
	SDL_LockMutex (agentMapMutex);
	i = agentMap.begin ();
	end = agentMap.end ();
	while (i != end) {
		destList->push_back (i->first);
		++i;
	}
	SDL_UnlockMutex (agentMapMutex);
}

int AgentControl::findTargetAgent (const StdString &searchKey, Agent *destAgent) {
	std::map<StdString, Agent>::iterator i, end, pos;
	StdString key;
	int result;

	result = OsUtil::Result::Success;
	key.assign (searchKey.lowercased ());

	SDL_LockMutex (agentMapMutex);
	end = agentMap.end ();
	pos = end;
	if (key.isUuid ()) {
		pos = agentMap.find (key);
		if (pos == end) {
			result = OsUtil::Result::KeyNotFoundError;
		}
	}
	else {
		i = agentMap.begin ();
		while (i != end) {
			if (key.equals (i->second.displayName.lowercased ())) {
				if (pos != end) {
					result = OsUtil::Result::DuplicateIdError;
					break;
				}
				pos = i;
			}
			++i;
		}
		if (pos == end) {
			result = OsUtil::Result::KeyNotFoundError;
		}
	}

	if (result == OsUtil::Result::Success) {
		if ((pos != end) && destAgent) {
			destAgent->copyDisplayData (pos->second);
		}
	}
	SDL_UnlockMutex (agentMapMutex);

	return (result);
}

void AgentControl::setAgentAttached (const StdString &agentId, bool attached) {
	std::map<StdString, Agent>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		if (attached != pos->second.isAttached) {
			found = true;
			pos->second.isAttached = attached;
		}
	}
	SDL_UnlockMutex (agentMapMutex);

	if (found) {
		if (attached) {
			refreshAgentStatus (agentId);
		}
		else {
			RecordStore::instance->removeRecord (agentId);
		}
		writePrefs ();
	}
}

bool AgentControl::isAgentAttached (const StdString &agentId) {
	std::map<StdString, Agent>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		result = pos->second.isAttached;
	}
	SDL_UnlockMutex (agentMapMutex);

	return (result);
}

bool AgentControl::isAgentContacting (const StdString &agentId) {
	std::map<StdString, Agent>::iterator pos;
	StdString hostname;
	int port;

	port = 0;
	SDL_LockMutex (agentMapMutex);
	pos = findAgent (agentId);
	if (pos != agentMap.end ()) {
		hostname.assign (pos->second.invokeHostname);
		port = pos->second.invokeTcpPort1;
	}
	SDL_UnlockMutex (agentMapMutex);
	if (hostname.empty () || (port <= 0)) {
		return (false);
	}

	return (isHostContacting (hostname, port));
}

bool AgentControl::isAgentContacted (const StdString &agentId) {
	std::map<StdString, Agent>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		result = pos->second.isContacted ();
	}
	SDL_UnlockMutex (agentMapMutex);

	return (result);
}

bool AgentControl::isAgentUnauthorized (const StdString &agentId) {
	std::map<StdString, Agent>::iterator pos;
	StdString hostname;
	int port;

	port = 0;
	SDL_LockMutex (agentMapMutex);
	pos = findAgent (agentId);
	if (pos != agentMap.end ()) {
		hostname.assign (pos->second.invokeHostname);
		port = pos->second.invokeTcpPort1;
	}
	SDL_UnlockMutex (agentMapMutex);
	if (hostname.empty () || (port <= 0)) {
		return (false);
	}

	return (isHostUnauthorized (hostname, port));
}

bool AgentControl::isAgentAuthorized (const StdString &agentId) {
	std::map<StdString, Agent>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (agentMapMutex);
	pos = findAgent (agentId);
	if (pos != agentMap.end ()) {
		if ((pos->second.lastStatusTime > 0) && (! pos->second.authorizeSecret.empty ())) {
			result = true;
		}
	}
	SDL_UnlockMutex (agentMapMutex);

	return (result);
}

void AgentControl::disconnectLinkClient (const StdString &agentId) {
	linkClient.disconnect (agentId);
}

bool AgentControl::isLinkClientConnected (const StdString &agentId) {
	return (linkClient.isConnected (agentId));
}

void AgentControl::linkClientConnect (void *agentControlPtr, const StdString &agentId) {
	App::instance->handleLinkClientConnect (agentId);
}

void AgentControl::linkClientDisconnect (void *agentControlPtr, const StdString &agentId, const StdString &errorDescription) {
	App::instance->handleLinkClientDisconnect (agentId, errorDescription);
}

void AgentControl::linkClientCommand (void *agentControlPtr, const StdString &agentId, Json *command) {
	CommandListener::instance->emit (agentId, command);
}

void AgentControl::receiveAgentStatus (void *agentControlPtr, const StdString &agentId, Json *command) {
	AgentControl::instance->storeAgentStatus (command);
	App::instance->shouldSyncRecordStore = true;
}

void AgentControl::receiveTaskItem (void *agentControlPtr, const StdString &agentId, Json *command) {
	RecordStore::instance->addRecord (command);
	App::instance->shouldSyncRecordStore = true;
}

void AgentControl::writeLinkCommand (Json *command, const StdString &agentId) {
	linkClient.writeCommand (command, agentId);
}

bool AgentControl::isHostContacted (const StdString &invokeHostname, int invokePort) {
	std::map<StdString, Agent>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (agentMapMutex);
	pos = findAgent (invokeHostname, invokePort);
	if (pos != agentMap.end ()) {
		if ((! pos->second.id.empty ()) && (pos->second.lastStatusTime > 0)) {
			result = true;
		}
	}
	SDL_UnlockMutex (agentMapMutex);

	return (result);
}

bool AgentControl::isHostContacting (const StdString &invokeHostname, int invokePort) {
	CommandList *cmdlist;
	bool result;

	result = false;
	SDL_LockMutex (commandMapMutex);
	cmdlist = findCommandList (getMapKey (invokeHostname, invokePort));
	if (cmdlist && (! cmdlist->empty ())) {
		result = true;
	}
	SDL_UnlockMutex (commandMapMutex);

	return (result);
}

bool AgentControl::isHostUnauthorized (const StdString &invokeHostname, int invokePort) {
	CommandList *cmdlist;
	bool result;

	result = false;
	SDL_LockMutex (commandMapMutex);
	cmdlist = findCommandList (getMapKey (invokeHostname, invokePort));
	if (cmdlist && (cmdlist->lastInvokeResult == OsUtil::Result::UnauthorizedError)) {
		result = true;
	}
	SDL_UnlockMutex (commandMapMutex);

	return (result);
}

StdString AgentControl::getAgentHostAddress (const StdString &agentId) {
	StdString s;
	std::map<StdString, Agent>::iterator pos;

	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		s.assign (pos->second.invokeHostname);
		if (pos->second.invokeTcpPort1 != SystemInterface::Constant_DefaultTcpPort1) {
			s.appendSprintf (":%i", pos->second.invokeTcpPort1);
		}
	}
	SDL_UnlockMutex (agentMapMutex);

	return (s);
}

StdString AgentControl::getAgentDisplayName (const StdString &agentId) {
	StdString s;
	std::map<StdString, Agent>::iterator pos;

	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		if (! pos->second.displayName.empty ()) {
			s.assign (pos->second.displayName);
		}
		else if (! pos->second.invokeHostname.empty ()) {
			s.assign (pos->second.invokeHostname);
		}
	}
	SDL_UnlockMutex (agentMapMutex);

	return (s);
}

StdString AgentControl::getAgentApplicationName (const StdString &agentId) {
	StdString s;
	std::map<StdString, Agent>::iterator pos;

	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		s.assign (pos->second.applicationName);
	}
	SDL_UnlockMutex (agentMapMutex);

	return (s);
}

int AgentControl::getAgentServerType (const StdString &agentId) {
	int type;
	std::map<StdString, Agent>::iterator pos;

	type = -1;
	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		type = pos->second.serverType;
	}
	SDL_UnlockMutex (agentMapMutex);

	return (type);
}

StdString AgentControl::getAgentInvokeUrl (const StdString &agentId, Json *command, const StdString &path) {
	std::map<StdString, Agent>::iterator pos;
	StdString url;

	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		url.assign (pos->second.getInvokeUrl ());
	}
	SDL_UnlockMutex (agentMapMutex);

	if (url.empty ()) {
		if (command) {
			delete (command);
		}
		return (StdString (""));
	}

	if (! path.startsWith ("/")) {
		url.append ("/");
	}
	url.append (path);
	if (command) {
		url.appendSprintf ("?%s=%s", SystemInterface::Constant_UrlQueryParameter, command->toString ().urlEncoded ().c_str ());
		delete (command);
	}
	return (url);
}

StdString AgentControl::getAgentSecondaryUrl (const StdString &agentId, Json *command, const StdString &path) {
	std::map<StdString, Agent>::iterator pos;
	StdString url;

	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		url.assign (pos->second.getSecondaryUrl ());
	}
	SDL_UnlockMutex (agentMapMutex);

	if (url.empty ()) {
		if (command) {
			delete (command);
		}
		return (StdString (""));
	}

	url.append (path);
	if (command) {
		url.appendSprintf ("?%s=%s", SystemInterface::Constant_UrlQueryParameter, command->toString ().urlEncoded ().c_str ());
		delete (command);
	}
	return (url);
}

StdString AgentControl::getAgentLinkUrl (const StdString &agentId) {
	StdString s;
	std::map<StdString, Agent>::iterator pos;

	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		s.assign (pos->second.getLinkUrl ());
	}
	SDL_UnlockMutex (agentMapMutex);

	return (s);
}

StdString AgentControl::getHostInvokeUrl (const StdString &hostname, int tcpPort, const StdString &path) {
	return (StdString::createSprintf ("%s://%s:%i%s%s", App::instance->isHttpsEnabled ? "https" : "http", hostname.c_str (), tcpPort, path.startsWith ("/") ? "" : "/", path.c_str ()));
}

StdString AgentControl::getMapKey (const StdString &hostname, int tcpPort) {
	return (StdString::createSprintf ("%s:%i", hostname.c_str (), tcpPort));
}

StdString AgentControl::getStringHash (const StdString &sourceString) {
	EVP_MD_CTX *ctx;
	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int len;

	ctx = EVP_MD_CTX_create ();
	if (! ctx) {
		Log::err ("Failed to compute string hash; err=EVP_MD_CTX_create failed");
		return (StdString (""));
	}

	len = 0;
	if (EVP_DigestInit_ex (ctx, EVP_sha256 (), NULL) != 1) {
		Log::err ("Failed to compute string hash; err=EVP_DigestInit_ex failed");
	}
	else {
		if (EVP_DigestUpdate (ctx, sourceString.c_str (), sourceString.length ()) != 1) {
			Log::err ("Failed to compute string hash; err=EVP_DigestUpdate failed");
		}
		else {
			if (EVP_DigestFinal_ex (ctx, digest, &len) != 1) {
				Log::err ("Failed to compute string hash; err=EVP_DigestFinal_ex failed");
			}
		}
	}
	EVP_MD_CTX_destroy (ctx);

	if (len <= 0) {
		return (StdString (""));
	}
	return (StdString::createHex (digest, len));
}

void AgentControl::contactAgent (const StdString &hostname, int tcpPort) {
	invokeCommand (hostname, tcpPort, App::instance->createCommand (SystemInterface::Command_GetStatus), AgentControl::invokeGetStatusComplete, this);
}

bool AgentControl::isAgentInvoking (const StdString &agentId) {
	CommandList *cmdlist;
	std::map<StdString, Agent>::iterator pos;
	StdString key, hostname;
	int port;
	bool result;

	port = 0;
	SDL_LockMutex (agentMapMutex);
	pos = findAgent (agentId);
	if (pos != agentMap.end ()) {
		hostname.assign (pos->second.invokeHostname);
		port = pos->second.invokeTcpPort1;
	}
	SDL_UnlockMutex (agentMapMutex);
	if (hostname.empty () || (port <= 0)) {
		return (false);
	}

	key.assign (getMapKey (hostname, port));
	result = false;
	SDL_LockMutex (commandMapMutex);
	cmdlist = findCommandList (key);
	if (cmdlist && (! cmdlist->empty ())) {
		result = true;
	}
	SDL_UnlockMutex (commandMapMutex);

	return (result);
}

void AgentControl::refreshAgentStatus (const StdString &agentId, const StdString &queueId) {
	invokeCommand (agentId, App::instance->createCommand (SystemInterface::Command_GetStatus), AgentControl::invokeGetStatusComplete, this, queueId);
}

void AgentControl::refreshAgentStatus (const StdString &invokeHostname, int invokeTcpPort, const StdString &queueId) {
	StdString agentid;
	std::map<StdString, Agent>::iterator pos;

	SDL_LockMutex (agentMapMutex);
	pos = findAgent (invokeHostname, invokeTcpPort);
	if (pos != agentMap.end ()) {
		agentid.assign (pos->second.id);
	}
	SDL_UnlockMutex (agentMapMutex);
	if (agentid.empty ()) {
		return;
	}
	invokeCommand (agentid, App::instance->createCommand (SystemInterface::Command_GetStatus), AgentControl::invokeGetStatusComplete, this, queueId);
}

void AgentControl::refreshAgentStatus (StringList *agentIdList, const StdString &queueId) {
	StringList::iterator i, end;

	i = agentIdList->begin ();
	end = agentIdList->end ();
	while (i != end) {
		refreshAgentStatus ((*i), queueId);
		++i;
	}
}

void AgentControl::checkAgentUpdates (const StdString &agentId) {
	std::map<StdString, Agent>::iterator pos;
	StdString newsurl;
	bool shouldrequest;

	shouldrequest = false;
	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		newsurl = pos->second.getApplicationNewsUrl ();
		if ((! newsurl.empty ()) && (! agentUpdateUrlMap.exists (newsurl))) {
			shouldrequest = true;
			agentUpdateUrlMap.insert (newsurl, "");
		}
	}
	SDL_UnlockMutex (agentMapMutex);

	if (shouldrequest && (! newsurl.empty ())) {
		Network::instance->sendHttpGet (newsurl, Network::HttpRequestCallbackContext (AgentControl::getApplicationNewsComplete, this));
	}
}

void AgentControl::getApplicationNewsComplete (void *agentControlPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	StdString resp;
	Json *cmd, *item;
	StdString url;
	int i, count;

	if (! responseData) {
		return;
	}
	cmd = NULL;
	resp.assignBuffer (responseData);
	if (SystemInterface::instance->parseCommand (resp, &cmd)) {
		if (SystemInterface::instance->getCommandId (cmd) == SystemInterface::CommandId_ApplicationNews) {
			count = SystemInterface::instance->getCommandArrayLength (cmd, "items");
			for (i = 0; i < count; ++i) {
				item = new Json ();
				if (SystemInterface::instance->getCommandObjectArrayItem (cmd, "items", i, item)) {
					url = item->getString ("actionTarget", "");
					if (App::isUpdateUrl (url)) {
						SDL_LockMutex (AgentControl::instance->agentMapMutex);
						AgentControl::instance->agentUpdateUrlMap.insert (targetUrl, url);
						SDL_UnlockMutex (AgentControl::instance->agentMapMutex);
						App::instance->shouldSyncRecordStore = true;
					}
				}
				delete (item);
			}
		}
		delete (cmd);
	}
}

StdString AgentControl::getAgentUpdateUrl (const StdString &agentId) {
	std::map<StdString, Agent>::iterator pos;
	StdString newsurl, updateurl;

	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		newsurl = pos->second.getApplicationNewsUrl ();
		if (! newsurl.empty ()) {
			updateurl = agentUpdateUrlMap.find (newsurl, "");
		}
	}
	SDL_UnlockMutex (agentMapMutex);

	return (updateurl);
}

void AgentControl::removeAgent (const StdString &agentId) {
	std::map<StdString, Agent>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		found = true;
		agentMap.erase (pos);
	}
	SDL_UnlockMutex (agentMapMutex);

	if (found) {
		RecordStore::instance->removeRecord (agentId);
		writePrefs ();
	}
}

int AgentControl::invokeCommand (const StdString &hostname, int tcpPort, Json *command, CommandList::InvokeCallback callback, void *callbackData, const StdString &queueId) {
	CommandList *cmdlist;
	StdString key;

	if (! command) {
		return (OsUtil::Result::InvalidParamError);
	}

	if (! queueId.empty ()) {
		key.assign (queueId);
	}
	else {
		key.assign (getMapKey (hostname, tcpPort));
	}
	SDL_LockMutex (commandMapMutex);
	cmdlist = findCommandList (key, true);
	cmdlist->addCommand (hostname, tcpPort, command, callback, callbackData);
	SDL_UnlockMutex (commandMapMutex);

	return (OsUtil::Result::Success);
}

int AgentControl::invokeCommand (const StdString &agentId, Json *command, CommandList::InvokeCallback callback, void *callbackData, const StdString &queueId) {
	CommandList *cmdlist;
	std::map<StdString, Agent>::iterator pos;
	StdString key, hostname;
	int port;

	if (! command) {
		return (OsUtil::Result::InvalidParamError);
	}

	port = 0;
	SDL_LockMutex (agentMapMutex);
	pos = findAgent (agentId);
	if (pos != agentMap.end ()) {
		hostname.assign (pos->second.invokeHostname);
		port = pos->second.invokeTcpPort1;
	}
	SDL_UnlockMutex (agentMapMutex);

	if (hostname.empty () || (port <= 0)) {
		delete (command);
		return (OsUtil::Result::KeyNotFoundError);
	}

	if (! queueId.empty ()) {
		key.assign (queueId);
	}
	else {
		key.assign (getMapKey (hostname, port));
	}
	SDL_LockMutex (commandMapMutex);
	cmdlist = findCommandList (key, true);
	cmdlist->addCommand (hostname, port, command, callback, callbackData);
	SDL_UnlockMutex (commandMapMutex);

	return (OsUtil::Result::Success);
}

int AgentControl::invokeCommand (StringList *agentIdList, Json *command, CommandList::InvokeCallback callback, void *callbackData, const StdString &queueId) {
	StringList::iterator i, end, last;
	int count, result;

	if (! command) {
		return (0);
	}
	if (agentIdList->empty ()) {
		delete (command);
		return (0);
	}

	count = 0;
	i = agentIdList->begin ();
	end = agentIdList->end ();
	last = end;
	--last;
	while (i != end) {
		result = invokeCommand (*i, (i != last) ? command->copy () : command, callback, callbackData, queueId);
		if (result != OsUtil::Result::Success) {
			Log::debug ("Failed to invoke command; err=%i agentId=\"%s\"", result, (*i).c_str ());
		}
		else {
			++count;
		}
		++i;
	}

	return (count);
}

void AgentControl::broadcastContactMessage () {
	Json *params, *cmd;
	StdString msg;

	params = new Json ();
	params->set ("destination", StdString::createSprintf ("udp://%s:%i", urlHostname.c_str (), Network::instance->datagramPort));
	cmd = App::instance->createCommand (SystemInterface::Command_ReportContact, params);
	if (! cmd) {
		return;
	}
	msg = cmd->toString ();
	delete (cmd);
	Network::instance->sendBroadcastDatagram (agentDatagramPort, msg.createBuffer ());
}

void AgentControl::receiveMessage (const char *messageData, int messageLength, const char *sourceAddress, int sourcePort) {
	Json *command;
	int commandid;

	if (! SystemInterface::instance->parseCommand (StdString (messageData, messageLength), &command)) {
		return;
	}

	commandid = SystemInterface::instance->getCommandId (command);
	switch (commandid) {
		case SystemInterface::CommandId_AgentStatus: {
			AgentControl::instance->storeAgentStatus (command);
			break;
		}
		case SystemInterface::CommandId_AgentContact: {
			StdString agentid, hostname;
			int port;
			bool found;

			agentid = SystemInterface::instance->getCommandStringParam (command, "id", "");
			SDL_LockMutex (AgentControl::instance->agentMapMutex);
			found = (AgentControl::instance->agentMap.count (agentid) > 0);
			SDL_UnlockMutex (AgentControl::instance->agentMapMutex);

			if (! found) {
				hostname.assign (sourceAddress);
				if (hostname.empty ()) {
					hostname = SystemInterface::instance->getCommandStringParam (command, "urlHostname", "");
				}

				port = SystemInterface::instance->getCommandNumberParam (command, "tcpPort1", 0);
				if ((! hostname.empty ()) && (port > 0)) {
					AgentControl::instance->contactAgent (hostname, port);
				}
			}
			break;
		}
	}
	delete (command);
}

void AgentControl::invokeGetStatusComplete (void *agentControlPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	std::map<StdString, Agent>::iterator pos;

	if (responseCommand && (SystemInterface::instance->getCommandId (responseCommand) == SystemInterface::CommandId_AgentStatus)) {
		AgentControl::instance->storeAgentStatus (responseCommand, invokeHostname, invokeTcpPort);
	}
	else {
		SDL_LockMutex (AgentControl::instance->agentMapMutex);
		pos = AgentControl::instance->findAgent (invokeHostname, invokeTcpPort);
		if (pos != AgentControl::instance->agentMap.end ()) {
			pos->second.lastStatusTime = 0;
		}
		SDL_UnlockMutex (AgentControl::instance->agentMapMutex);
	}
	App::instance->shouldSyncRecordStore = true;
}

void AgentControl::storeAgentStatus (Json *agentStatusCommand, const StdString &invokeHostname, int invokeTcpPort) {
	int commandid;
	StdString recordid, removeid, linkurl1, linkurl2;
	Agent agent;
	std::map<StdString, Agent>::iterator pos;

	commandid = SystemInterface::instance->getCommandId (agentStatusCommand);
	if (commandid != SystemInterface::CommandId_AgentStatus) {
		return;
	}

	recordid = SystemInterface::instance->getCommandStringParam (agentStatusCommand, "id", "");
	if (recordid.empty ()) {
		return;
	}
	RecordStore::instance->addRecord (agentStatusCommand, recordid);

	removeid.assign ("");
	SDL_LockMutex (agentMapMutex);
	pos = findAgent (recordid);
	if ((pos == agentMap.end ()) && (! invokeHostname.empty ()) && (invokeTcpPort > 0)) {
		pos = findAgent (invokeHostname, invokeTcpPort);
		if ((pos != agentMap.end ()) && (! recordid.equals (pos->first))) {
			removeid.assign (pos->first);
			pos = agentMap.end ();
		}
	}
	if (pos == agentMap.end ()) {
		pos = findAgent (recordid, true);
		if ((! invokeHostname.empty ()) && (invokeTcpPort > 0)) {
			pos->second.invokeHostname.assign (invokeHostname);
			pos->second.invokeTcpPort1 = invokeTcpPort;
		}
	}

	linkurl1.assign (pos->second.getLinkUrl ());
	pos->second.id.assign (recordid);
	pos->second.lastStatusTime = OsUtil::getTime ();
	pos->second.isAttached = true;
	pos->second.readCommand (agentStatusCommand);
	linkurl2.assign (pos->second.getLinkUrl ());
	SDL_UnlockMutex (agentMapMutex);

	if (! removeid.empty ()) {
		removeAgent (removeid);
	}
	if (! linkurl1.equals (linkurl2)) {
		linkClient.setLinkUrl (recordid, linkurl2);
	}

	writePrefs ();
}

void AgentControl::addAdminSecret (const StdString &entryName, const StdString &entrySecret) {
	HashMap *prefs;
	JsonList items;
	Json *json;
	AgentControl::AdminSecret entry;

	prefs = App::instance->lockPrefs ();
	prefs->find (AgentControl::ServerAdminSecretsKey, &items);
	json = new Json ();
	json->set (AgentControl::NameKey, entryName);
	json->set (AgentControl::SecretKey, entrySecret);
	items.push_back (json);
	prefs->insert (AgentControl::ServerAdminSecretsKey, &items);
	App::instance->unlockPrefs ();

	entry.name.assign (entryName);
	getAdminSecretAuthorizationValues (getStringHash (entrySecret), &(entry.authorizePath), &(entry.authorizeSecret));
	SDL_LockMutex (adminSecretMutex);
	adminSecretList.push_back (entry);
	SDL_UnlockMutex (adminSecretMutex);
}

void AgentControl::removeAdminSecret (int secretIndex) {
	HashMap *prefs;
	JsonList items;
	JsonList::iterator i, end;
	AgentControl::AdminSecret entry;
	Json *json;
	bool found;
	int listindex;

	if (secretIndex < 0) {
		return;
	}
	prefs = App::instance->lockPrefs ();
	prefs->find (AgentControl::ServerAdminSecretsKey, &items);
	App::instance->unlockPrefs ();
	if (secretIndex >= (int) items.size ()) {
		return;
	}

	found = false;
	listindex = 0;
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		json = *i;
		if (listindex == secretIndex) {
			found = true;
			delete (json);
			items.erase (i);
			break;
		}
		++listindex;
		++i;
	}
	if (! found) {
		return;
	}

	SDL_LockMutex (adminSecretMutex);
	adminSecretList.clear ();
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		json = *i;
		if (readAdminSecret (&entry, json)) {
			adminSecretList.push_back (entry);
		}
		++i;
	}
	SDL_UnlockMutex (adminSecretMutex);

	prefs = App::instance->lockPrefs ();
	prefs->insert (AgentControl::ServerAdminSecretsKey, &items);
	App::instance->unlockPrefs ();
}

StdString AgentControl::getAdminSecretValue (int secretIndex) {
	HashMap *prefs;
	JsonList items;
	JsonList::iterator i, end;
	StdString result;
	Json *json;

	if (secretIndex < 0) {
		return (StdString (""));
	}
	prefs = App::instance->lockPrefs ();
	prefs->find (AgentControl::ServerAdminSecretsKey, &items);
	App::instance->unlockPrefs ();
	if (secretIndex >= (int) items.size ()) {
		return (StdString (""));
	}

	json = items.at (secretIndex);
	if (json) {
		result.assign (json->getString (AgentControl::SecretKey, ""));
	}
	return (result);
}

void AgentControl::getAdminSecretNames (StringList *destList) {
	std::vector<AgentControl::AdminSecret>::iterator i, end;

	destList->clear ();
	SDL_LockMutex (adminSecretMutex);
	i = adminSecretList.begin ();
	end = adminSecretList.end ();
	while (i != end) {
		destList->push_back (i->name);
		++i;
	}
	SDL_UnlockMutex (adminSecretMutex);
}

bool AgentControl::getAgentAuthorization (const StdString &agentId, StdString *authorizePath, StdString *authorizeSecret, StdString *authorizeToken) {
	std::map<StdString, Agent>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (agentMapMutex);
	pos = findAgent (agentId);
	if (pos != agentMap.end ()) {
		result = true;
		if (authorizePath) {
			if (pos->second.authorizePath.empty ()) {
				authorizePath->assign (SystemInterface::Constant_DefaultAuthorizePath);
			}
			else {
				authorizePath->assign (pos->second.authorizePath);
			}
		}
		if (authorizeSecret) {
			authorizeSecret->assign (pos->second.authorizeSecret);
		}
		if (authorizeToken) {
			authorizeToken->assign (pos->second.authorizeToken);
		}
	}
	SDL_UnlockMutex (agentMapMutex);

	if (! result) {
		if (authorizePath) {
			authorizePath->assign (SystemInterface::Constant_DefaultAuthorizePath);
		}
		if (authorizeSecret) {
			authorizeSecret->assign ("");
		}
		if (authorizeToken) {
			authorizeToken->assign ("");
		}
	}
	return (result);
}

void AgentControl::setAgentAuthorization (const StdString &agentId, int secretIndex, const StdString &authorizeToken) {
	std::map<StdString, Agent>::iterator pos;
	AgentControl::AdminSecret entry;

	if (secretIndex >= 0) {
		SDL_LockMutex (adminSecretMutex);
		if (secretIndex < (int) adminSecretList.size ()) {
			entry = adminSecretList.at (secretIndex);
		}
		SDL_UnlockMutex (adminSecretMutex);
	}

	SDL_LockMutex (agentMapMutex);
	pos = findAgent (agentId);
	if (pos != agentMap.end ()) {
		if (entry.authorizeSecret.empty ()) {
			pos->second.authorizePath.assign ("");
			pos->second.authorizeSecret.assign ("");
			pos->second.authorizeToken.assign ("");
		}
		else {
			pos->second.authorizePath.assign (entry.authorizePath);
			pos->second.authorizeSecret.assign (entry.authorizeSecret);
			pos->second.authorizeToken.assign (authorizeToken);
		}
	}
	SDL_UnlockMutex (agentMapMutex);
}

void AgentControl::setHostAuthorization (const StdString &hostname, int tcpPort, int secretIndex, const StdString &authorizeToken) {
	std::map<StdString, Agent>::iterator pos;
	AgentControl::AdminSecret entry;

	if (secretIndex >= 0) {
		SDL_LockMutex (adminSecretMutex);
		if (secretIndex < (int) adminSecretList.size ()) {
			entry = adminSecretList.at (secretIndex);
		}
		SDL_UnlockMutex (adminSecretMutex);
	}

	SDL_LockMutex (agentMapMutex);
	pos = findAgent (hostname, tcpPort);
	if (pos != agentMap.end ()) {
		if (entry.authorizeSecret.empty ()) {
			pos->second.authorizePath.assign ("");
			pos->second.authorizeSecret.assign ("");
			pos->second.authorizeToken.assign ("");
		}
		else {
			pos->second.authorizePath.assign (entry.authorizePath);
			pos->second.authorizeSecret.assign (entry.authorizeSecret);
			pos->second.authorizeToken.assign (authorizeToken);
		}
	}
	SDL_UnlockMutex (agentMapMutex);
}

bool AgentControl::setCommandAuthorization (Json *command, int secretIndex, const StdString &authorizeToken, StdString *authorizePath) {
	EVP_MD_CTX *ctx;
	AgentControl::AdminSecret entry;
	bool result;

	if (secretIndex < 0) {
		return (false);
	}
	SDL_LockMutex (adminSecretMutex);
	if (secretIndex < (int) adminSecretList.size ()) {
		entry = adminSecretList.at (secretIndex);
	}
	SDL_UnlockMutex (adminSecretMutex);
	if (entry.authorizeSecret.empty ()) {
		return (false);
	}

	ctx = EVP_MD_CTX_create ();
	if (! ctx) {
		Log::err ("Failed to set command authorization; err=EVP_MD_CTX_create failed");
		return (false);
	}
	if (EVP_DigestInit_ex (ctx, EVP_sha256 (), NULL) != 1) {
		EVP_MD_CTX_destroy (ctx);
		Log::err ("Failed to set command authorization; err=EVP_DigestInit_ex failed");
		return (false);
	}

	if (authorizePath) {
		authorizePath->assign (entry.authorizePath);
	}
	result = SystemInterface::instance->setCommandAuthorization (command, entry.authorizeSecret, authorizeToken, AgentControl::hashUpdate, AgentControl::hashDigest, ctx);
	EVP_MD_CTX_destroy (ctx);

	return (result);
}

void AgentControl::hashUpdate (void *contextPtr, unsigned char *data, int dataLength) {
	EVP_DigestUpdate ((EVP_MD_CTX *) contextPtr, data, dataLength);
}

StdString AgentControl::hashDigest (void *contextPtr) {
	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int len;
	StdString s;

	if (EVP_DigestFinal_ex ((EVP_MD_CTX *) contextPtr, digest, &len) == 1) {
		s.assignHex (digest, len);
	}

	return (s);
}

void AgentControl::writePrefs () {
	HashMap *prefs;
	std::map<StdString, Agent>::iterator i, end;
	JsonList items;

	SDL_LockMutex (agentMapMutex);
	i = agentMap.begin ();
	end = agentMap.end ();
	while (i != end) {
		if (! i->second.id.empty ()) {
			items.push_back (i->second.createState ());
		}
		++i;
	}
	SDL_UnlockMutex (agentMapMutex);

	prefs = App::instance->lockPrefs ();
	prefs->insert (AgentControl::AgentStatusKey, &items);
	App::instance->unlockPrefs ();
}

void AgentControl::readPrefs () {
	HashMap *prefs;
	JsonList items;
	JsonList::iterator i, end;
	std::map<StdString, Agent>::iterator pos;
	Agent agent;
	AgentControl::AdminSecret entry;
	int result;

	prefs = App::instance->lockPrefs ();
	agentServerName = prefs->find (AgentControl::AgentServerNameKey, "");

	prefs->find (AgentControl::AgentStatusKey, &items);

	// TODO: Remove this operation (when transition from the legacy key is no longer needed)
	App::transferJsonListPrefs (prefs, "AgentStatus", &items);

	App::instance->unlockPrefs ();

	SDL_LockMutex (agentMapMutex);
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		result = agent.readState (*i);
		if (result == OsUtil::Result::Success) {
			pos = agentMap.find (agent.id);
			if (pos == agentMap.end ()) {
				agentMap.insert (std::pair<StdString, Agent> (agent.id, agent));
			}
			else {
				pos->second = agent;
			}
		}
		++i;
	}
	SDL_UnlockMutex (agentMapMutex);

	prefs = App::instance->lockPrefs ();
	prefs->find (AgentControl::ServerAdminSecretsKey, &items);

	// TODO: Remove this operation (when transition from the legacy key is no longer needed)
	if (App::transferJsonListPrefs (prefs, "ServerAdminSecrets", &items)) {
		prefs->insert (AgentControl::ServerAdminSecretsKey, &items);
	}

	App::instance->unlockPrefs ();

	SDL_LockMutex (adminSecretMutex);
	adminSecretList.clear ();
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		if (readAdminSecret (&entry, *i)) {
			adminSecretList.push_back (entry);
		}
		++i;
	}
	SDL_UnlockMutex (adminSecretMutex);
}

CommandList *AgentControl::findCommandList (const StdString &listName, bool createNew) {
	std::map<StdString, CommandList *>::iterator pos;
	CommandList *cmd;

	pos = commandMap.find (listName);
	if (pos != commandMap.end ()) {
		return (pos->second);
	}
	if (! createNew) {
		return (NULL);
	}

	cmd = new CommandList (listName);
	commandMap.insert (std::pair<StdString, CommandList *> (listName, cmd));
	return (cmd);
}

std::map<StdString, Agent>::iterator AgentControl::findAgent (const StdString &agentId, bool createNew) {
	std::map<StdString, Agent>::iterator pos;
	Agent item;

	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		return (pos);
	}
	if (! createNew) {
		return (agentMap.end ());
	}

	item.id.assign (agentId);
	agentMap.insert (std::pair<StdString, Agent> (agentId, item));
	return (agentMap.find (agentId));
}

std::map<StdString, Agent>::iterator AgentControl::findAgent (const StdString &invokeHostname, int invokePort) {
	std::map<StdString, Agent>::iterator i, end;

	i = agentMap.begin ();
	end = agentMap.end ();
	while (i != end) {
		if (i->second.invokeHostname.equals (invokeHostname) && (i->second.invokeTcpPort1 == invokePort)) {
			return (i);
		}
		if (i->second.urlHostname.equals (invokeHostname) && (i->second.tcpPort1 == invokePort)) {
			return (i);
		}
		++i;
	}

	return (end);
}

bool AgentControl::readAdminSecret (AgentControl::AdminSecret *entry, Json *prefsItem) {
	StdString path, secret;
	bool result;

	result = false;
	entry->name = prefsItem->getString (AgentControl::NameKey, "-");
	secret = prefsItem->getString (AgentControl::SecretKey, "");
	if (! secret.empty ()) {
		getAdminSecretAuthorizationValues (getStringHash (secret), &(entry->authorizePath), &(entry->authorizeSecret));
		result = true;
	}
	else {
		path = prefsItem->getString (AgentControl::AuthorizePathKey, "");
		secret = prefsItem->getString (AgentControl::AuthorizeSecretKey, "");
		if ((! path.empty ()) && (! secret.empty ())) {
			entry->authorizePath.assign (path);
			entry->authorizeSecret.assign (secret);
			result = true;
		}
	}

	return (result);
}

void AgentControl::getAdminSecretAuthorizationValues (const StdString &adminSecret, StdString *authorizePath, StdString *authorizeSecret) {
	StdString path, secret;
	int len;

	secret.assign (adminSecret);
	len = ((int) secret.length ()) / 2;
	if (len <= 0) {
		path.assign (SystemInterface::Constant_DefaultAuthorizePath);
	}
	else {
		path.assign (secret.substr (len));
		secret.erase (len);
	}

	if (authorizePath) {
		authorizePath->assign (path);
	}
	if (authorizeSecret) {
		authorizeSecret->assign (secret);
	}
}
