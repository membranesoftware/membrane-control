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
#include <map>
#include "SDL2/SDL.h"
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "Util.h"
#include "Json.h"
#include "SystemInterface.h"
#include "HashMap.h"
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
{
	// TODO: Possibly set an agentId value (currently empty)

	linkClientMutex = SDL_CreateMutex ();
	commandQueueMutex = SDL_CreateMutex ();
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
	HashMap::Iterator i;
	StdString id;

	app = App::getInstance ();
	if (isStarted) {
		return (Result::SUCCESS);
	}

	linkClientConnectionCount = 0;
	readPrefs ();

	isStarted = true;

	i = agentInvokeUrlMap.begin ();
	while (agentInvokeUrlMap.hasNext (&i)) {
		id = agentInvokeUrlMap.next (&i);
		invokeCommand (id, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), AgentControl::invokeAgentContactGetStatusComplete, this, NULL, StdString ("AgentControl::start"));
	}

	return (Result::SUCCESS);
}

void AgentControl::stop () {
	agentInvokeUrlMap.clear ();
	agentLinkUrlMap.clear ();
	agentDisplayNameMap.clear ();
	linkClientConnectAddressMap.clear ();
	clearLinkClients ();
}

void AgentControl::update (int msElapsed) {
	std::map<StdString, LinkClient *>::iterator mi, mend;
	std::list<LinkClient *>::iterator li, lend;
	LinkClient *client;
	StdString url;
	int count;
	bool found;

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
		// TODO: Consider periodically requesting status from the target agent, in case its linkUrl has changed and broadcasts are not enabled
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

	url = agentLinkUrlMap.find (agentId, "");
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

void AgentControl::connectLinkClientToAddress (const StdString &address) {
	App *app;
	StdString hostname;
	Json *cmd;
	int result, port;
	int64_t jobid;

	app = App::getInstance ();
	if (linkClientConnectAddressMap.exists (address)) {
		return;
	}

	Log::write (Log::DEBUG, "Connect link client to address; address=\"%s\"", address.c_str ());
	if (! StdString::parseAddress (address.c_str (), &hostname, &port)) {
		Log::write (Log::DEBUG, "Parse agent address failed; address=\"%s\"", address.c_str ());
		return;
	}

	if (port > 65535) {
		return;
	}
	if (port <= 0) {
		port = SystemInterface::Constant_DefaultTcpPort;
	}

	cmd = app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType, NULL);
	if (! cmd) {
		return;
	}

	result = invokeCommand (hostname, port, cmd, AgentControl::invokeLinkServerGetStatusComplete, this, &jobid);
	if (result != Result::SUCCESS) {
		// TODO: Show an error message here
		Log::write (Log::ERR, "Failed to execute GetStatus command; err=%i address=\"%s\"", result, address.c_str ());
		return;
	}

	// TODO: Possibly add synchronization for this operation (to account for the unlikely case in which invokeLinkServerGetStatusComplete executes before this line completes)
	linkClientConnectAddressMap.insert (address, jobid);
}

void AgentControl::invokeLinkServerGetStatusComplete (void *agentControlPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	AgentControl *agentcontrol;
	App *app;
	HashMap::Iterator i;
	SystemInterface *interface;
	Json serverstatus;
	StdString address, agentid;
	int commandid;
	int64_t addressjobid;

	agentcontrol = (AgentControl *) agentControlPtr;

	i = agentcontrol->linkClientConnectAddressMap.begin ();
	while (agentcontrol->linkClientConnectAddressMap.hasNext (&i)) {
		address = agentcontrol->linkClientConnectAddressMap.next (&i);
		addressjobid = agentcontrol->linkClientConnectAddressMap.find (address, (int64_t) 0);
		if (addressjobid == jobId) {
			agentcontrol->linkClientConnectAddressMap.remove (address);
			break;
		}
	}

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
	SystemInterface *interface;
	AgentControl *agentcontrol;
	StdString recordid;
	Json record;

	agentcontrol = (AgentControl *) agentControlPtr;
	app = App::getInstance ();
	interface = &(app->systemInterface);
	if ((interface->getCommandId (command) == SystemInterface::Command_EventRecord) && interface->getCommandObjectParam (command, "record", &record)) {
		recordid = interface->getCommandRecordId (&record);
		if (! recordid.empty ()) {
			agentcontrol->recordStore.addRecord (recordid, &record);
		}

		if (interface->getCommandId (&record) == SystemInterface::Command_AgentStatus) {
			agentcontrol->storeAgentUrls (&record);
		}
	}

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
		Log::write (Log::DEBUG, "AgentControl::writeLinkCommand discard command (not a JSON object); command=%s", commandJson.c_str ());
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

bool AgentControl::isLinkClientConnectingToAddress (const StdString &address) {
	return (linkClientConnectAddressMap.exists (address));
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
	return (agentDisplayNameMap.find (agentId, ""));
}

int AgentControl::invokeCommand (const StdString &agentId, Json *command, AgentControl::InvokeCommandCallback callback, void *callbackData, int64_t *jobId, const StdString &queueId) {
	App *app;
	AgentControl::CommandContext *ctx;
	StdString url;

	if (! command) {
		return (Result::ERROR_INVALID_PARAM);
	}
	app = App::getInstance ();
	url = agentInvokeUrlMap.find (agentId, "");
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
	url.sprintf ("http://%s:%i/", hostname.c_str (), tcpPort);
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

void AgentControl::sendHttpPostComplete (void *contextPtr, const StdString &targetUrl, int statusCode, Buffer *responseData) {
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
		resp.assignFromBuffer (responseData);
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
			SystemInterface *interface;
			StdString recordid;

			agentcontrol = &(app->agentControl);
			interface = &(app->systemInterface);
			recordid = interface->getCommandStringParam (command, "id", "");
			if (! recordid.empty ()) {
				agentcontrol->recordStore.addRecord (recordid, command);
			}
			agentcontrol->storeAgentUrls (command);

			app->handleLinkClientCommand (recordid, command);
			break;
		}
		case SystemInterface::Command_AgentContact: {
			AgentControl *agentcontrol;
			SystemInterface *interface;
			StdString agentid, oldurl, url;
			bool getstatus;

			agentcontrol = &(app->agentControl);
			interface = &(app->systemInterface);
			agentid = interface->getCommandStringParam (command, "id", "");
			if (! agentid.empty ()) {
				getstatus = false;
				oldurl = agentInvokeUrlMap.find (agentid, "");
				agentcontrol->storeAgentUrls (command);
				url = agentInvokeUrlMap.find (agentid, "");
				if (! url.empty ()) {
					if (! oldurl.equals (url)) {
						getstatus = true;
					}

					if (! getstatus) {
						agentcontrol->recordStore.lock ();
						if (! agentcontrol->recordStore.findRecord (RecordStore::matchAgentStatusSource, &agentid)) {
							getstatus = true;
						}
						agentcontrol->recordStore.unlock ();
					}
				}

				if (getstatus) {
					agentcontrol->invokeCommand (agentid, app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType), AgentControl::invokeAgentContactGetStatusComplete, agentcontrol);
				}
			}
			app->handleLinkClientCommand (agentid, command);
			break;
		}
	}
	delete (command);
}

void AgentControl::invokeAgentContactGetStatusComplete (void *agentControlPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	App *app;


	if (! responseCommand) {
		return;
	}

	app = App::getInstance ();
	app->agentControl.storeAgentStatus (responseCommand);
	app->shouldSyncRecordStore = true;
}

void AgentControl::storeAgentStatus (Json *agentStatusCommand) {
	App *app;
	SystemInterface *interface;
	int commandid;
	StdString recordid, name;

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
	recordStore.addRecord (recordid, agentStatusCommand);
	storeAgentUrls (agentStatusCommand);

	name = interface->getCommandStringParam (agentStatusCommand, "displayName", "");
	if (! name.empty ()) {
		agentDisplayNameMap.insert (recordid, name);
	}

	writePrefs ();
	app->handleLinkClientCommand (recordid, agentStatusCommand);
}

void AgentControl::storeAgentUrls (Json *command) {
	App *app;
	SystemInterface *interface;
	StdString recordid, url;
	Json serverstatus;
	int commandid;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	recordid = interface->getCommandStringParam (command, "id", "");
	if (recordid.empty ()) {
		return;
	}

	url = interface->getCommandAgentInvokeUrl (command);
	if (! url.empty ()) {
		agentInvokeUrlMap.insert (recordid, url);
	}

	commandid = interface->getCommandId (command);
	if (commandid == SystemInterface::Command_AgentStatus) {
		if (interface->getCommandObjectParam (command, "linkServerStatus", &serverstatus)) {
			url = serverstatus.getString ("linkUrl", "");
			if (! url.empty ()) {
				agentLinkUrlMap.insert (recordid, url);
			}
		}
	}
	else if (commandid == SystemInterface::Command_AgentContact) {
		url = interface->getCommandStringParam (command, "linkUrl", "");
		if (! url.empty ()) {
			agentLinkUrlMap.insert (recordid, url);
		}
	}
}

void AgentControl::writePrefs () {
	App *app;
	Json *obj;
	HashMap::Iterator i;
	StringList items;
	StdString id, val;

	app = App::getInstance ();
	i = agentInvokeUrlMap.begin ();
	while (agentInvokeUrlMap.hasNext (&i)) {
		id = agentInvokeUrlMap.next (&i);
		obj = new Json ();
		obj->set ("id", id);

		val = agentInvokeUrlMap.find (id, "");
		if (! val.empty ()) {
			obj->set ("in", val);
		}
		val = agentLinkUrlMap.find (id, "");
		if (! val.empty ()) {
			obj->set ("li", val);
		}
		val = agentDisplayNameMap.find (id, "");
		if (! val.empty ()) {
			obj->set ("di", val);
		}

		items.push_back (obj->toString ());
		delete (obj);
	}

	app->prefsMap.insert (App::prefsAgentStatus, &items);
}

void AgentControl::readPrefs () {
	App *app;
	Json *obj;
	StringList items;
	StringList::iterator i, end;
	StdString id, val;
	int result;

	app = App::getInstance ();
	app->prefsMap.find (App::prefsAgentStatus, &items);

	i = items.begin ();
	end = items.end ();
	while (i != end) {
		obj = new Json ();
		result = obj->parse (*i);
		if (result == Result::SUCCESS) {
			id = obj->getString ("id", "");
			if (! id.empty ()) {
				val = obj->getString ("in", "");
				if (! val.empty ()) {
					agentInvokeUrlMap.insert (id, val);
				}
				val = obj->getString ("li", "");
				if (! val.empty ()) {
					agentLinkUrlMap.insert (id, val);
				}
				val = obj->getString ("di", "");
				if (! val.empty ()) {
					agentDisplayNameMap.insert (id, val);
				}
			}
		}
		delete (obj);
		++i;
	}
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

int AgentControl::readSystemAgentConfiguration (HashMap *destMap, StdString *agentId) {
	StdString serverpath, filepath, runstate, id;
	Json json;
	int result;

#if PLATFORM_LINUX
	serverpath = App::getInstance ()->prefsMap.find (App::prefsServerPath, "/usr/local/membrane-server");
#endif
#if PLATFORM_MACOS
	serverpath = App::getInstance ()->prefsMap.find (App::prefsServerPath, "/Applications/Membrane Master.app");
#endif
#if PLATFORM_WINDOWS
	// TODO: Get this path from an environment variable
	serverpath = App::getInstance ()->prefsMap.find (App::prefsServerPath, "C:\\Program Files\\Membrane Server");
#endif
	if (serverpath.empty ()) {
		return (Result::ERROR_NOT_IMPLEMENTED);
	}

	filepath.assign ("");
#if PLATFORM_LINUX
	filepath.sprintf ("%s/control", serverpath.c_str ());
#endif
#if PLATFORM_MACOS
	filepath.sprintf ("%s/Contents/MacOS/run.sh", serverpath.c_str ());
#endif
#if PLATFORM_WINDOWS
	filepath.sprintf ("%s/run.js", serverpath.c_str ());
#endif
	if (filepath.empty () || (! Util::fileExists (filepath))) {
		return (Result::ERROR_APPLICATION_NOT_INSTALLED);
	}

	filepath.assign ("");
#if PLATFORM_LINUX
	filepath.sprintf ("%s/conf/systemagent.conf", serverpath.c_str ());
#endif
#if PLATFORM_MACOS
	filepath.sprintf ("%s/Contents/MacOS/conf/systemagent.conf", serverpath.c_str ());
#endif
#if PLATFORM_WINDOWS
	filepath.sprintf ("%s\\conf\\systemagent.conf", serverpath.c_str ());
#endif
	if (filepath.empty () || (! Util::fileExists (filepath))) {
		return (Result::ERROR_APPLICATION_NOT_INSTALLED);
	}

	result = destMap->read (filepath, true);
	if (result != Result::SUCCESS) {
		return (result);
	}

	filepath.assign ("");
#if PLATFORM_LINUX
	filepath.sprintf ("%s/run/systemagent/state", serverpath.c_str ());
#endif
#if PLATFORM_MACOS
	filepath.sprintf ("%s/Contents/MacOS/run/systemagent/state", serverpath.c_str ());
#endif
#if PLATFORM_WINDOWS
	filepath.sprintf ("%s\\run\\systemagent\\state", serverpath.c_str ());
#endif
	if (filepath.empty () || (! Util::fileExists (filepath))) {
		return (Result::ERROR_APPLICATION_NOT_INSTALLED);
	}

	result = Util::readFile (filepath, &runstate);
	if (result != Result::SUCCESS) {
		return (result);
	}

	result = json.parse (runstate);
	if (result != Result::SUCCESS) {
		return (result);
	}

	agentId->assign (json.getString ("agentId", ""));

	return (Result::SUCCESS);
}
