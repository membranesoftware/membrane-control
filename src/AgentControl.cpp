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
#include "SDL2/SDL.h"
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "Util.h"
#include "Json.h"
#include "SystemInterface.h"
#include "HashMap.h"
#include "StringList.h"
#include "Ui.h"
#include "LinkClient.h"
#include "RecordStore.h"
#include "AgentControl.h"

const StdString AgentControl::localHostname = StdString ("127.0.0.1");

AgentControl::AgentControl ()
: agentDatagramPort (63738)
, agentId ("")
, urlHostname ("")
, linkClientConnectionCount (0)
, isStarted (false)
, linkClientMutex (NULL)
, commandQueueMutex (NULL)
, agentMapMutex (NULL)
{
	// TODO: Possibly set an agentId value (currently empty)

	linkClientMutex = SDL_CreateMutex ();
	commandQueueMutex = SDL_CreateMutex ();
	agentMapMutex = SDL_CreateMutex ();
}

AgentControl::~AgentControl () {
	clearLinkClients ();
	clearCommandQueues ();
	if (linkClientMutex) {
		SDL_DestroyMutex (linkClientMutex);
		linkClientMutex = NULL;
	}
	if (commandQueueMutex) {
		SDL_DestroyMutex (commandQueueMutex);
		commandQueueMutex = NULL;
	}
	if (agentMapMutex) {
		SDL_DestroyMutex (agentMapMutex);
		agentMapMutex = NULL;
	}
}

void AgentControl::destroyCommandContext (AgentControl::CommandContext *ctx) {
	delete (ctx->command);
	delete (ctx);
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

void AgentControl::clearCommandQueues () {
	std::map<StdString, AgentControl::CommandContextQueue>::iterator i, end;
	AgentControl::CommandContextQueue *q;
	AgentControl::CommandContext *ctx;

	SDL_LockMutex (commandQueueMutex);
	i = commandQueueMap.begin ();
	end = commandQueueMap.end ();
	while (i != end) {
		q = &(i->second);
		while (! q->empty ()) {
			ctx = q->front ();
			destroyCommandContext (ctx);
			q->pop ();
		}
		++i;
	}
	commandQueueMap.clear ();
	SDL_UnlockMutex (commandQueueMutex);
}

int AgentControl::start () {
	App *app;
	std::map<StdString, Agent>::iterator i, end;
	StringList idlist;
	StringList::iterator j, jend;

	app = App::getInstance ();
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
		invokeCommand (*j, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), AgentControl::invokeGetStatusComplete, this);
		++j;
	}

	return (Result::SUCCESS);
}

void AgentControl::stop () {
	agentLinkUrlMap.clear ();
	agentContactHostnameMap.clear ();
	SDL_LockMutex (agentMapMutex);
	agentMap.clear ();
	SDL_UnlockMutex (agentMapMutex);
	clearLinkClients ();
}

void AgentControl::update (int msElapsed) {
	std::map<StdString, LinkClient *>::iterator mi, mend;
	std::list<LinkClient *>::iterator li, lend;
	std::map<StdString, Agent>::iterator ai, aend;
	LinkClient *client;
	StdString url;
	int count;
	bool found;

	agentLinkUrlMap.clear ();
	SDL_LockMutex (agentMapMutex);
	ai = agentMap.begin ();
	aend = agentMap.end ();
	while (ai != aend) {
		agentLinkUrlMap.insert (ai->first, ai->second.getLinkUrl ());
		++ai;
	}
	SDL_UnlockMutex (agentMapMutex);

	SDL_LockMutex (linkClientMutex);
	count = 0;
	mi = linkClientMap.begin ();
	mend = linkClientMap.end ();
	while (mi != mend) {
		client = mi->second;
		url = agentLinkUrlMap.find (client->agentId, "");
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

	// TODO: Age the record store here (delete old records as needed)
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

void AgentControl::invokeLinkServerGetStatusComplete (void *agentControlPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	AgentControl *agentcontrol;
	App *app;
	SystemInterface *interface;
	Json serverstatus;
	StdString agentid;
	int commandid;

	agentcontrol = (AgentControl *) agentControlPtr;

	if (! responseCommand) {
		return;
	}

	app = App::getInstance ();
	interface = &(app->systemInterface);
	agentcontrol->storeAgentStatus (responseCommand);
	app->shouldSyncRecordStore = true;

	commandid = interface->getCommandId (responseCommand);
	if (commandid == SystemInterface::Command_AgentStatus) {
		if (interface->getCommandObjectParam (responseCommand, "linkServerStatus", &serverstatus)) {
			agentid = interface->getCommandStringParam (responseCommand, "id", "");
			if (! agentid.empty ()) {
				agentcontrol->connectLinkClient (agentid);
			}
		}
	}
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
}

void AgentControl::writeLinkCommand (const StdString &commandJson, const StdString &agentId) {
	std::map<StdString, LinkClient *>::iterator i, end;
	LinkClient *client;

	if (commandJson.empty () || (commandJson.find ('{') != 0)) {
		return;
	}

	SDL_LockMutex (linkClientMutex);
	end = linkClientMap.end ();
	if (agentId.empty ()) {
		i = linkClientMap.begin ();
		while (i != end) {
			client = i->second;
			if (client->isConnected) {
				client->writeCommand (commandJson);
			}
			++i;
		}
	}
	else {
		i = linkClientMap.find (agentId);
		if (i != end) {
			client = i->second;
			if (client->isConnected) {
				client->writeCommand (commandJson);
			}
		}
	}
	SDL_UnlockMutex (linkClientMutex);
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

StdString AgentControl::getAgentDisplayName (const StdString &agentId) {
	StdString s;
	std::map<StdString, Agent>::iterator pos;

	SDL_LockMutex (agentMapMutex);
	pos = agentMap.find (agentId);
	if (pos != agentMap.end ()) {
		s.assign (pos->second.displayName);
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

StdString AgentControl::getHostInvokeUrl (const StdString &hostname, int tcpPort) {
	return (StdString::createSprintf ("%s://%s:%i", App::getInstance ()->isHttpsEnabled ? "https" : "http", hostname.c_str (), tcpPort));
}

void AgentControl::contactAgent (const StdString &hostname, int tcpPort) {
	App *app;
	int64_t jobid;

	app = App::getInstance ();
	invokeCommand (hostname, tcpPort, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), AgentControl::invokeGetStatusComplete, this, &jobid);
	agentContactHostnameMap.insert (StdString::createSprintf ("%lli", (long long int) jobid), StdString::createSprintf ("%s:%i", hostname.c_str (), tcpPort));
}

void AgentControl::retryAgents () {
	App *app;
	std::map<StdString, Agent>::iterator i, end;
	StringList idlist;
	StringList::iterator j, jend;

	app = App::getInstance ();

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
		invokeCommand (*j, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), AgentControl::invokeGetStatusComplete, this, NULL, *j);
		++j;
	}
}

void AgentControl::refreshAgentStatus (const StdString &agentId, const StdString &queueId) {
	App *app;

	app = App::getInstance ();
	invokeCommand (agentId, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), AgentControl::invokeGetStatusComplete, this, NULL, queueId);
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

int AgentControl::invokeCommand (const StdString &agentId, Json *command, AgentControl::InvokeCommandCallback callback, void *callbackData, int64_t *jobId, const StdString &queueId) {
	App *app;
	AgentControl::CommandContext *ctx;
	StdString url;

	app = App::getInstance ();
	if (! command) {
		return (Result::ERROR_INVALID_PARAM);
	}

	url = getAgentInvokeUrl (agentId);
	if (url.empty ()) {
		return (Result::ERROR_KEY_NOT_FOUND);
	}
	if (url.empty ()) {
		if (command) {
			delete (command);
		}
		return (Result::ERROR_UNKNOWN_HOSTNAME);
	}
	ctx = new AgentControl::CommandContext ();
	ctx->agentId.assign (agentId);
	ctx->url.assign (url);
	ctx->command = command;
	ctx->jobId = app->getUniqueId ();
	if (jobId) {
		*jobId = ctx->jobId;
	}
	ctx->callback = callback;
	ctx->callbackData = callbackData;

	if (queueId.empty ()) {
		app->network.sendHttpPost (ctx->url, command->toString (), AgentControl::sendHttpPostComplete, ctx);
	}
	else {
		addQueueCommand (queueId, ctx);
	}

	return (Result::SUCCESS);
}

int AgentControl::invokeCommand (const StdString &hostname, int tcpPort, Json *command, AgentControl::InvokeCommandCallback callback, void *callbackData, int64_t *jobId, const StdString &queueId) {
	App *app;
	AgentControl::CommandContext *ctx;
	StdString url;

	if (! command) {
		return (Result::ERROR_INVALID_PARAM);
	}
	app = App::getInstance ();
	url.assign (getHostInvokeUrl (hostname, tcpPort));
	ctx = new AgentControl::CommandContext ();
	ctx->agentId.assign (url);
	ctx->url.assign (url);
	ctx->command = command;
	ctx->jobId = app->getUniqueId ();
	if (jobId) {
		*jobId = ctx->jobId;
	}
	ctx->callback = callback;
	ctx->callbackData = callbackData;

	if (queueId.empty ()) {
		app->network.sendHttpPost (ctx->url, command->toString (), AgentControl::sendHttpPostComplete, ctx);
	}
	else {
		addQueueCommand (queueId, ctx);
	}

	return (Result::SUCCESS);
}

void AgentControl::sendHttpPostComplete (void *contextPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	App *app;
	AgentControl::CommandContext *ctx;
	StdString resp;
	Json *responsecmd;
	int result;

	ctx = (AgentControl::CommandContext *) contextPtr;
	app = App::getInstance ();

	result = Result::SUCCESS;
	responsecmd = NULL;

	if ((statusCode != Network::HTTP_OK) || (! responseData)) {
		result = Result::ERROR_HTTP_OPERATION_FAILED;
	}

	if (result == Result::SUCCESS) {
		if (responseData->empty ()) {
			result = Result::ERROR_MALFORMED_RESPONSE;
		}
	}

	if (result == Result::SUCCESS) {
		resp.assignBuffer (responseData);
		if (! App::getInstance ()->systemInterface.parseCommand (resp, &responsecmd)) {
			result = Result::ERROR_MALFORMED_RESPONSE;
		}
	}

	if (ctx->callback) {
		ctx->callback (ctx->callbackData, ctx->jobId, result, ctx->agentId, ctx->command, responsecmd);
	}

	if (responsecmd) {
		delete (responsecmd);
	}

	if (! ctx->queueId.empty ()) {
		app->agentControl.processQueueCommand (ctx->queueId, true);
	}
	app->agentControl.destroyCommandContext (ctx);
}

void AgentControl::broadcastContactMessage () {
	App *app;
	Json *params;
	StdString cmdjson;

	app = App::getInstance ();
	params = new Json ();
	params->set ("destination", StdString::createSprintf ("udp://%s:%i", urlHostname.c_str (), app->network.datagramPort));
	cmdjson = app->createCommandJson ("ReportContact", SystemInterface::Constant_DefaultCommandType, params);
	if (cmdjson.empty ()) {
		return;
	}

	app->network.sendBroadcastDatagram (agentDatagramPort, cmdjson.createBuffer ());
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

void AgentControl::invokeGetStatusComplete (void *agentControlPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	App *app;
	StdString key, hostname, invokehostname;
	int invokeport;


	app = App::getInstance ();
	key.sprintf ("%lli", (long long int) jobId);
	hostname = app->agentControl.agentContactHostnameMap.find (key, "");
	app->agentControl.agentContactHostnameMap.remove (key);
	if (responseCommand) {
		invokeport = 0;
		if (! hostname.empty ()) {
			if (! StdString::parseAddress (hostname.c_str (), &invokehostname, &invokeport, SystemInterface::Constant_DefaultTcpPort1)) {
				invokehostname.assign ("");
				invokeport = 0;
			}
		}

		app->agentControl.storeAgentStatus (responseCommand, invokehostname, invokeport);
		app->shouldSyncRecordStore = true;
	}
}

void AgentControl::storeAgentStatus (Json *agentStatusCommand, const StdString &agentInvokeHostname, int agentInvokeTcpPort1) {
	App *app;
	SystemInterface *interface;
	int commandid;
	StdString recordid, name;
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
	pos = agentMap.find (recordid);
	if (pos == agentMap.end ()) {
		agentMap.insert (std::pair<StdString, Agent> (recordid, agent));
		pos = agentMap.find (recordid);
		pos->second.id.assign (recordid);
	}
	pos->second.lastStatusTime = Util::getTime ();
	pos->second.readCommand (agentStatusCommand);
	if (! agentInvokeHostname.empty ()) {
		pos->second.invokeHostname.assign (agentInvokeHostname);
	}
	if (agentInvokeTcpPort1 > 0) {
		pos->second.invokeTcpPort1 = agentInvokeTcpPort1;
	}
	SDL_UnlockMutex (agentMapMutex);
	writePrefs ();
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
		items.push_back (i->second.toPrefsJsonString ());
		++i;
	}
	SDL_UnlockMutex (agentMapMutex);

	app->prefsMap.insert (App::prefsAgentStatus, &items);
}

void AgentControl::readPrefs () {
	App *app;
	StringList items;
	StringList::iterator i, end;
	std::map<StdString, Agent>::iterator pos;
	Agent agent;
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
}

void AgentControl::addQueueCommand (const StdString &queueId, AgentControl::CommandContext *ctx) {
	std::map<StdString, AgentControl::CommandContextQueue>::iterator i;
	AgentControl::CommandContextQueue *q;
	bool shouldinvoke;

	shouldinvoke = false;
	SDL_LockMutex (commandQueueMutex);
	i = commandQueueMap.find (queueId);
	if (i == commandQueueMap.end ()) {
		commandQueueMap.insert (std::pair<StdString, AgentControl::CommandContextQueue> (queueId, AgentControl::CommandContextQueue ()));
		i = commandQueueMap.find (queueId);
	}
	if (i != commandQueueMap.end ()) {
		ctx->queueId.assign (queueId);
		q = &(i->second);
		if (q->size () <= 0) {
			shouldinvoke = true;
		}
		q->push (ctx);
	}
	SDL_UnlockMutex (commandQueueMutex);

	if (shouldinvoke) {
		processQueueCommand (queueId);
	}
}

void AgentControl::processQueueCommand (const StdString &queueId, bool removeTop) {
	App *app;
	std::map<StdString, AgentControl::CommandContextQueue>::iterator i;
	AgentControl::CommandContextQueue *q;
	AgentControl::CommandContext *ctx, *sendctx;

	app = App::getInstance ();
	sendctx = NULL;
	SDL_LockMutex (commandQueueMutex);
	i = commandQueueMap.find (queueId);
	if (i != commandQueueMap.end ()) {
		q = &(i->second);
		if (removeTop && (! q->empty ())) {
			q->pop ();
		}

		if (q->empty ()) {
			commandQueueMap.erase (i);
		}
		else {
			ctx = q->front ();
			if (! ctx->isActive) {
				sendctx = ctx;
				sendctx->isActive = true;
			}
		}
	}
	SDL_UnlockMutex (commandQueueMutex);

	if (sendctx) {
		app->network.sendHttpPost (sendctx->url, sendctx->command->toString (), AgentControl::sendHttpPostComplete, sendctx);
	}
}
