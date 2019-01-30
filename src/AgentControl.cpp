/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
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
#include <map>
#include <list>
#include <vector>
#include "SDL2/SDL.h"
#include "openssl/evp.h"
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "Util.h"
#include "StdString.h"
#include "Json.h"
#include "SystemInterface.h"
#include "HashMap.h"
#include "StringList.h"
#include "Ui.h"
#include "LinkClient.h"
#include "RecordStore.h"
#include "AgentControl.h"

const StdString AgentControl::localHostname = StdString ("127.0.0.1");
static const int COMMAND_LIST_IDLE_TIMEOUT = 60000; // ms
static const char *NAME_KEY = "a";
static const char *SECRET_KEY = "b";

AgentControl::AgentControl ()
: agentDatagramPort (63738)
, agentId ("")
, urlHostname ("")
, linkClientConnectionCount (0)
, isStarted (false)
, linkClientMutex (NULL)
, agentMapMutex (NULL)
, commandMapMutex (NULL)
, adminSecretMutex (NULL)
{
	// TODO: Possibly set an agentId value (currently empty)

	linkClientMutex = SDL_CreateMutex ();
	agentMapMutex = SDL_CreateMutex ();
	commandMapMutex = SDL_CreateMutex ();
	adminSecretMutex = SDL_CreateMutex ();
}

AgentControl::~AgentControl () {
	clearLinkClients ();
	clearCommandMap ();
	if (linkClientMutex) {
		SDL_DestroyMutex (linkClientMutex);
		linkClientMutex = NULL;
	}
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

void AgentControl::clearLinkClients () {
	std::map<StdString, LinkClient *>::iterator mi, mend;
	std::list<LinkClient *>::iterator li, lend;

	SDL_LockMutex (linkClientMutex);
	mi = linkClientMap.begin ();
	mend = linkClientMap.end ();
	while (mi != mend) {
		if (mi->second) {
			mi->second->stop ();
			delete (mi->second);
			mi->second = NULL;
		}
		++mi;
	}
	linkClientMap.clear ();

	li = linkClientCloseList.begin ();
	lend = linkClientCloseList.end ();
	while (li != lend) {
		(*li)->stop ();
		delete (*li);
		++li;
	}
	linkClientCloseList.clear ();
	linkClientConnectionCount = 0;
	SDL_UnlockMutex (linkClientMutex);
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
		return (Result::SUCCESS);
	}

	linkClientConnectionCount = 0;
	readPrefs ();

	isStarted = true;

	SDL_LockMutex (agentMapMutex);
	i = agentMap.begin ();
	end = agentMap.end ();
	while (i != end) {
		idlist.push_back (i->first);
		++i;
	}
	SDL_UnlockMutex (agentMapMutex);

	j = idlist.begin ();
	jend = idlist.end ();
	while (j != jend) {
		refreshAgentStatus (*j);
		++j;
	}

	return (Result::SUCCESS);
}

void AgentControl::stop () {
	SDL_LockMutex (adminSecretMutex);
	adminSecretList.clear ();
	SDL_UnlockMutex (adminSecretMutex);

	SDL_LockMutex (agentMapMutex);
	agentMap.clear ();
	SDL_UnlockMutex (agentMapMutex);

	clearLinkClients ();
}

void AgentControl::update (int msElapsed) {
	updateLinkClients (msElapsed);
	updateCommandMap (msElapsed);

	// TODO: Age the record store here (delete old records as appropriate)
}

void AgentControl::updateLinkClients (int msElapsed) {
	std::map<StdString, LinkClient *>::iterator mi, mend;
	std::list<LinkClient *>::iterator li, lend;
	std::map<StdString, Agent>::iterator ai, aend;
	LinkClient *client;
	HashMap urlmap;
	StdString url;
	int count;
	bool found;

	SDL_LockMutex (agentMapMutex);
	ai = agentMap.begin ();
	aend = agentMap.end ();
	while (ai != aend) {
		urlmap.insert (ai->first, ai->second.getLinkUrl ());
		++ai;
	}
	SDL_UnlockMutex (agentMapMutex);

	SDL_LockMutex (linkClientMutex);
	count = 0;
	mi = linkClientMap.begin ();
	mend = linkClientMap.end ();
	while (mi != mend) {
		client = mi->second;
		url = urlmap.find (client->agentId, "");
		if (! client->url.equals (url)) {
			client->setUrl (url);
		}

		client->update (msElapsed);
		++count;
		++mi;
	}
	linkClientConnectionCount = count;

	while (true) {
		found = false;
		li = linkClientCloseList.begin ();
		lend = linkClientCloseList.end ();
		while (li != lend) {
			client = *li;
			if (! client->isRunning) {
				found = true;
				linkClientCloseList.erase (li);
				delete (client);
				break;
			}
			++li;
		}

		if (! found) {
			break;
		}
	}

	SDL_UnlockMutex (linkClientMutex);
}

void AgentControl::updateCommandMap (int msElapsed) {
	std::map<StdString, CommandList *>::iterator mi, mend;
	StringList keys;
	StringList::iterator ki, kend;

	SDL_LockMutex (commandMapMutex);
	mi = commandMap.begin ();
	mend = commandMap.end ();
	while (mi != mend) {
		if (mi->second->isIdle (COMMAND_LIST_IDLE_TIMEOUT) && mi->second->empty ()) {
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
}

void AgentControl::connectLinkClient (const StdString &agentId) {
	StdString url;
	LinkClient *client;
	std::map<StdString, LinkClient *>::iterator pos;
	bool exists;

	exists = false;
	SDL_LockMutex (linkClientMutex);
	pos = linkClientMap.find (agentId);
	if (pos != linkClientMap.end ()) {
		exists = true;
		client = pos->second;
		++(client->usageCount);
	}
	SDL_UnlockMutex (linkClientMutex);
	if (exists) {
		return;
	}

	url = getAgentLinkUrl (agentId);
	if (url.empty ()) {
		Log::write (Log::WARNING, "Failed to connect link client, URL not found; agentId=\"%s\"", agentId.c_str ());
		return;
	}

	client = new LinkClient (url, agentId, this, AgentControl::linkClientConnect, AgentControl::linkClientDisconnect, AgentControl::linkClientCommand);
	client->usageCount = 1;
	SDL_LockMutex (linkClientMutex);
	linkClientMap.insert (std::pair<StdString, LinkClient *> (agentId, client));
	SDL_UnlockMutex (linkClientMutex);
}

void AgentControl::disconnectLinkClient (const StdString &agentId) {
	std::map<StdString, LinkClient *>::iterator pos;
	LinkClient *client;

	SDL_LockMutex (linkClientMutex);
	pos = linkClientMap.find (agentId);
	if (pos != linkClientMap.end ()) {
		client = pos->second;
		--(client->usageCount);
		if (client->usageCount <= 0) {
			linkClientMap.erase (pos);

			client->shutdown ();
			linkClientCloseList.push_back (client);
		}
	}
	SDL_UnlockMutex (linkClientMutex);
}

void AgentControl::linkClientConnect (void *agentControlPtr, LinkClient *client) {
	App::getInstance ()->handleLinkClientConnect (client->agentId);
}

void AgentControl::linkClientDisconnect (void *agentControlPtr, LinkClient *client, const StdString &errorDescription) {
	App::getInstance ()->handleLinkClientDisconnect (client->agentId, errorDescription);
}

void AgentControl::linkClientCommand (void *agentControlPtr, LinkClient *client, Json *command) {
	App *app;

	app = App::getInstance ();
	app->handleLinkClientCommand (client->agentId, command);
}

void AgentControl::writeLinkCommand (Json *command, const StdString &agentId) {
	std::map<StdString, LinkClient *>::iterator i, end;
	LinkClient *client;

	if (! command) {
		return;
	}
	SDL_LockMutex (linkClientMutex);
	end = linkClientMap.end ();
	if (agentId.empty ()) {
		i = linkClientMap.begin ();
		while (i != end) {
			client = i->second;
			if (client->isConnected) {
				client->writeCommand (command);
			}
			++i;
		}
	}
	else {
		i = linkClientMap.find (agentId);
		if (i != end) {
			client = i->second;
			if (client->isConnected) {
				client->writeCommand (command);
			}
		}
	}
	SDL_UnlockMutex (linkClientMutex);

	delete (command);
}

bool AgentControl::isLinkClientConnected (const StdString &agentId) {
	std::map<StdString, LinkClient *>::iterator i;
	LinkClient *client;
	bool result;

	result = false;
	SDL_LockMutex (linkClientMutex);
	i = linkClientMap.find (agentId);
	if (i != linkClientMap.end ()) {
		client = i->second;
		if (client->isConnected && (! client->isShutdown)) {
			result = true;
		}
	}
	SDL_UnlockMutex (linkClientMutex);

	return (result);
}

int AgentControl::getLinkClientCount () {
	std::map<StdString, LinkClient *>::iterator i, end;
	LinkClient *client;
	int count;

	count = 0;
	SDL_LockMutex (linkClientMutex);
	i = linkClientMap.begin ();
	end = linkClientMap.end ();
	while (i != end) {
		client = i->second;
		if (client->isConnected && (! client->isShutdown)) {
			++count;
		}
		++i;
	}
	SDL_UnlockMutex (linkClientMutex);

	return (count);
}

bool AgentControl::isContacted (const StdString &invokeHostname, int invokePort) {
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

bool AgentControl::isContacting (const StdString &invokeHostname, int invokePort) {
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

bool AgentControl::isUnauthorized (const StdString &invokeHostname, int invokePort) {
	CommandList *cmdlist;
	bool result;

	result = false;
	SDL_LockMutex (commandMapMutex);
	cmdlist = findCommandList (getMapKey (invokeHostname, invokePort));
	if (cmdlist && (cmdlist->lastInvokeResult == Result::ERROR_UNAUTHORIZED)) {
		result = true;
	}
	SDL_UnlockMutex (commandMapMutex);

	return (result);
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
	return (StdString::createSprintf ("%s://%s:%i%s%s", App::getInstance ()->isHttpsEnabled ? "https" : "http", hostname.c_str (), tcpPort, path.startsWith ("/") ? "" : "/", path.c_str ()));
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
		Log::write (Log::ERR, "Failed to compute string hash; err=EVP_MD_CTX_create failed");
		return (StdString (""));
	}

	len = 0;
	if (EVP_DigestInit_ex (ctx, EVP_sha256 (), NULL) != 1) {
		Log::write (Log::ERR, "Failed to compute string hash; err=EVP_DigestInit_ex failed");
	}
	else {
		if (EVP_DigestUpdate (ctx, sourceString.c_str (), sourceString.length ()) != 1) {
			Log::write (Log::ERR, "Failed to compute string hash; err=EVP_DigestUpdate failed");
		}
		else {
			if (EVP_DigestFinal_ex (ctx, digest, &len) != 1) {
				Log::write (Log::ERR, "Failed to compute string hash; err=EVP_DigestFinal_ex failed");
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
	App *app;

	app = App::getInstance ();
	invokeCommand (hostname, tcpPort, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), AgentControl::invokeGetStatusComplete, this);
}

void AgentControl::retryAgents () {
	std::map<StdString, Agent>::iterator i, end;
	StringList idlist;
	StringList::iterator j, jend;

	SDL_LockMutex (agentMapMutex);
	i = agentMap.begin ();
	end = agentMap.end ();
	while (i != end) {
		if (i->second.lastStatusTime <= 0) {
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
}

void AgentControl::refreshAgentStatus (const StdString &agentId, const StdString &queueId) {
	App *app;

	app = App::getInstance ();
	invokeCommand (agentId, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), AgentControl::invokeGetStatusComplete, this, queueId);
}

void AgentControl::refreshAgentStatus (const StdString &invokeHostname, int invokeTcpPort, const StdString &queueId) {
	App *app;
	StdString agentid;
	std::map<StdString, Agent>::iterator pos;

	app = App::getInstance ();

	SDL_LockMutex (agentMapMutex);
	pos = findAgent (invokeHostname, invokeTcpPort);
	if (pos != agentMap.end ()) {
		agentid.assign (pos->second.id);
	}
	SDL_UnlockMutex (agentMapMutex);
	if (agentid.empty ()) {
		return;
	}
	invokeCommand (agentid, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), AgentControl::invokeGetStatusComplete, this, queueId);
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
		recordStore.removeRecord (agentId);
		writePrefs ();
	}
}

int AgentControl::invokeCommand (const StdString &hostname, int tcpPort, Json *command, CommandList::InvokeCallback callback, void *callbackData, const StdString &queueId) {
	CommandList *cmdlist;
	StdString key;

	if (! command) {
		return (Result::ERROR_INVALID_PARAM);
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

	return (Result::SUCCESS);
}

int AgentControl::invokeCommand (const StdString &agentId, Json *command, CommandList::InvokeCallback callback, void *callbackData, const StdString &queueId) {
	CommandList *cmdlist;
	std::map<StdString, Agent>::iterator pos;
	StdString key, hostname;
	int port;

	if (! command) {
		return (Result::ERROR_INVALID_PARAM);
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
		return (Result::ERROR_KEY_NOT_FOUND);
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

	return (Result::SUCCESS);
}

void AgentControl::broadcastContactMessage () {
	App *app;
	Json *params, *cmd;
	StdString msg;

	app = App::getInstance ();
	params = new Json ();
	params->set ("destination", StdString::createSprintf ("udp://%s:%i", urlHostname.c_str (), app->network.datagramPort));
	cmd = app->createCommand ("ReportContact", SystemInterface::Constant_DefaultCommandType, params);
	if (! cmd) {
		return;
	}
	msg = cmd->toString ();
	delete (cmd);
	app->network.sendBroadcastDatagram (agentDatagramPort, msg.createBuffer ());
}

void AgentControl::receiveMessage (const char *messageData, int messageLength) {
	App *app;
	Json *command;
	int commandid;

	app = App::getInstance ();
	if (! app->systemInterface.parseCommand (StdString (messageData, messageLength), &command)) {
		return;
	}

	commandid = app->systemInterface.getCommandId (command);
	switch (commandid) {
		case SystemInterface::Command_AgentStatus: {
			AgentControl *agentcontrol;

			agentcontrol = &(app->agentControl);
			agentcontrol->storeAgentStatus (command);
			break;
		}
		case SystemInterface::Command_AgentContact: {
			AgentControl *agentcontrol;
			SystemInterface *interface;
			StdString agentid, hostname;
			int port;
			bool found;

			agentcontrol = &(app->agentControl);
			interface = &(app->systemInterface);
			agentid = interface->getCommandStringParam (command, "id", "");
			SDL_LockMutex (agentcontrol->agentMapMutex);
			found = (agentcontrol->agentMap.count (agentid) > 0);
			SDL_UnlockMutex (agentcontrol->agentMapMutex);

			if (! found) {
				hostname = interface->getCommandStringParam (command, "urlHostname", "");
				port = interface->getCommandNumberParam (command, "tcpPort1", 0);
				if ((! hostname.empty ()) && (port > 0)) {
					agentcontrol->contactAgent (hostname, port);
				}
			}
			break;
		}
	}
	delete (command);
}

void AgentControl::invokeGetStatusComplete (void *agentControlPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand) {
	App *app;
	AgentControl *agentcontrol;

	app = App::getInstance ();
	agentcontrol = (AgentControl *) agentControlPtr;

	if (responseCommand) {
		agentcontrol->storeAgentStatus (responseCommand, invokeHostname, invokeTcpPort);
		app->shouldSyncRecordStore = true;
	}
}

void AgentControl::storeAgentStatus (Json *agentStatusCommand, const StdString &invokeHostname, int invokeTcpPort) {
	App *app;
	SystemInterface *interface;
	int commandid;
	StdString recordid;
	Agent agent;
	std::map<StdString, Agent>::iterator pos;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	commandid = interface->getCommandId (agentStatusCommand);
	if (commandid != SystemInterface::Command_AgentStatus) {
		return;
	}

	recordid = interface->getCommandStringParam (agentStatusCommand, "id", "");
	if (recordid.empty ()) {
		return;
	}
	recordStore.addRecord (agentStatusCommand, recordid);

	SDL_LockMutex (agentMapMutex);
	pos = findAgent (recordid);
	if ((pos == agentMap.end ()) && (! invokeHostname.empty ()) && (invokeTcpPort > 0)) {
		pos = findAgent (invokeHostname, invokeTcpPort);
	}
	if (pos == agentMap.end ()) {
		pos = findAgent (recordid, true);
		if ((! invokeHostname.empty ()) && (invokeTcpPort > 0)) {
			pos->second.invokeHostname.assign (invokeHostname);
			pos->second.invokeTcpPort1 = invokeTcpPort;
		}
	}

	pos->second.id.assign (recordid);
	pos->second.lastStatusTime = Util::getTime ();
	pos->second.readCommand (agentStatusCommand);
	SDL_UnlockMutex (agentMapMutex);
	writePrefs ();
}

void AgentControl::addAdminSecret (const StdString &entryName, const StdString &entrySecret) {
	App *app;
	StringList items;
	Json *json;

	app = App::getInstance ();

	app->prefsMap.find (App::prefsServerAdminSecrets, &items);
	json = new Json ();
	json->set (NAME_KEY, entryName);
	json->set (SECRET_KEY, entrySecret);
	items.push_back (json->toString ());
	delete (json);
	app->prefsMap.insert (App::prefsServerAdminSecrets, &items);

	SDL_LockMutex (adminSecretMutex);
	adminSecretList.push_back (getStringHash (entrySecret));
	SDL_UnlockMutex (adminSecretMutex);
}

void AgentControl::removeAdminSecret (const StdString &entryName) {
	App *app;
	StringList items, writeitems;
	StringList::iterator i, end;
	StdString val;
	Json *json;
	bool shouldwrite;

	app = App::getInstance ();
	app->prefsMap.find (App::prefsServerAdminSecrets, &items);
	shouldwrite = false;
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		json = new Json ();
		if (json->parse (*i) != Result::SUCCESS) {
			shouldwrite = true;
		}
		else {
			if (entryName.equals (json->getString (NAME_KEY, ""))) {
				shouldwrite = true;
			}
			else {
				writeitems.push_back (*i);
			}
		}
		delete (json);
		++i;
	}

	if (shouldwrite) {
		app->prefsMap.insert (App::prefsServerAdminSecrets, &writeitems);
		SDL_LockMutex (adminSecretMutex);
		adminSecretList.clear ();
		i = writeitems.begin ();
		end = writeitems.end ();
		while (i != end) {
			json = new Json ();
			if (json->parse (*i) == Result::SUCCESS) {
				val = json->getString (SECRET_KEY, "");
				if (! val.empty ()) {
					adminSecretList.push_back (getStringHash (val));
				}
			}
			delete (json);
			++i;
		}
		SDL_UnlockMutex (adminSecretMutex);
	}
}

StdString AgentControl::getAdminSecret (const StdString &entryName) {
	App *app;
	StringList items;
	StringList::iterator i, end;
	StdString result;
	Json *json;

	app = App::getInstance ();
	app->prefsMap.find (App::prefsServerAdminSecrets, &items);
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		json = new Json ();
		if (json->parse (*i) == Result::SUCCESS) {
			if (entryName.equals (json->getString (NAME_KEY, ""))) {
				result.assign (json->getString (SECRET_KEY, ""));
			}
		}
		delete (json);

		if (! result.empty ()) {
			break;
		}
		++i;
	}

	return (result);
}

void AgentControl::getAdminSecretNames (StringList *destList) {
	App *app;
	StringList items;
	StringList::iterator i, end;
	StdString name;
	Json *json;

	app = App::getInstance ();
	app->prefsMap.find (App::prefsServerAdminSecrets, &items);
	destList->clear ();
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		json = new Json ();
		if (json->parse (*i) == Result::SUCCESS) {
			name = json->getString (NAME_KEY, "");
			if (! name.empty ()) {
				destList->push_back (name);
			}
		}
		delete (json);
		++i;
	}
}

bool AgentControl::setCommandAuthorization (Json *command, int secretIndex, const StdString &authToken, StdString *authPath) {
	App *app;
	EVP_MD_CTX *ctx;
	StdString secret;
	int len;
	bool result;

	if (secretIndex < 0) {
		return (false);
	}

	app = App::getInstance ();
	ctx = EVP_MD_CTX_create ();
	if (! ctx) {
		Log::write (Log::ERR, "Failed to set command authorization; err=EVP_MD_CTX_create failed");
		return (false);
	}
	if (EVP_DigestInit_ex (ctx, EVP_sha256 (), NULL) != 1) {
		EVP_MD_CTX_destroy (ctx);
		Log::write (Log::ERR, "Failed to set command authorization; err=EVP_DigestInit_ex failed");
		return (false);
	}

	result = false;
	SDL_LockMutex (adminSecretMutex);
	if (secretIndex < (int) adminSecretList.size ()) {
		secret = adminSecretList.at (secretIndex);
		len = ((int) secret.length ()) / 2;
		if (len <= 0) {
			if (authPath) {
				authPath->assign (SystemInterface::Constant_DefaultAuthorizePath);
			}
		}
		else {
			if (authPath) {
				authPath->assign (secret.substr (len));
			}
			secret.erase (len);
		}

		result = app->systemInterface.setCommandAuthorization (command, secret, authToken, AgentControl::hashUpdate, AgentControl::hashDigest, ctx);
	}
	SDL_UnlockMutex (adminSecretMutex);
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
	App *app;
	std::map<StdString, Agent>::iterator i, end;
	StringList items;

	app = App::getInstance ();
	SDL_LockMutex (agentMapMutex);
	i = agentMap.begin ();
	end = agentMap.end ();
	while (i != end) {
		if (! i->second.id.empty ()) {
			items.push_back (i->second.toPrefsJsonString ());
		}
		++i;
	}
	SDL_UnlockMutex (agentMapMutex);

	app->prefsMap.insert (App::prefsAgentStatus, &items);
}

void AgentControl::readPrefs () {
	App *app;
	StringList items;
	StringList::iterator i, end;
	StdString val;
	std::map<StdString, Agent>::iterator pos;
	Agent agent;
	Json *json;
	int result;

	app = App::getInstance ();

	app->prefsMap.find (App::prefsAgentStatus, &items);
	SDL_LockMutex (agentMapMutex);
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		result = agent.readPrefsJson (*i);
		if (result == Result::SUCCESS) {
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

	app->prefsMap.find (App::prefsServerAdminSecrets, &items);
	SDL_LockMutex (adminSecretMutex);
	adminSecretList.clear ();
	i = items.begin ();
	end = items.end ();
	while (i != end) {
		json = new Json ();
		if (json->parse (*i) == Result::SUCCESS) {
			val = json->getString (SECRET_KEY, "");
			if (! val.empty ()) {
				adminSecretList.push_back (getStringHash (val));
			}
		}
		delete (json);
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
