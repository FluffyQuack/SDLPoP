#include "../Libs/slikenet/Include/WindowsIncludes.h" //Include this instead of "windows.h" as explained in Raknet FAQ
#include "../Libs/slikenet/Include/types.h"
#include "../Libs/slikenet/Include/PacketPriority.h"
#include "Network-Main.h"
#include "Network-Intermediate.h"
#include "Network-Player.h"
#include "../Print.h"

networkPlayer_s networkPlayers[MAXCLIENTS]; //First entry in the array is always the local player
static short lastAssignedPlayerId = 0;

short Network_GetUnusedPlayerId()
{
	short prevLastAssignedPlayerId = lastAssignedPlayerId;
	while(1)
	{
		lastAssignedPlayerId++;
		if(lastAssignedPlayerId < 1)
			lastAssignedPlayerId = 1;
		if(lastAssignedPlayerId == prevLastAssignedPlayerId)
			return -1;
		bool isPlayerIdUsed = 0;
		for(int i = 0; i < MAXCLIENTS; i++)
		{
			if(networkPlayers[i].active && networkPlayers[i].networkId == lastAssignedPlayerId)
			{
				isPlayerIdUsed = 1;
				break;
			}
		}
		if(isPlayerIdUsed == 0)
			return lastAssignedPlayerId;
	}
}

void Network_ResetPlayers(bool resetAll, bool resetLocalPlayerId)
{
	memset(&networkPlayers[1], 0, sizeof(networkPlayer_s) * (MAXCLIENTS - 1));
	networkPlayers[0].active = 1;
	lastAssignedPlayerId = 0;
	if(resetAll) //If true, we also reset the local player's id and name. This should only ever be done once as we start the program.
	{
		networkPlayers[0].networkId = 0;
		networkPlayers[0].name[0] = 0;
	}

	if(resetLocalPlayerId) //This only happens when we fully terminate network connection
		networkPlayers[0].id = 0;
}

networkPlayer_s *Network_FindPlayerByNetworkId(unsigned long long networkId)
{
	for(int i = 0; i < MAXCLIENTS; i++)
	{
		if(networkPlayers[i].active && networkPlayers[i].networkId == networkId)
			return &networkPlayers[i];
	}
	return 0;
}

networkPlayer_s *Network_FindPlayerByPlayerId(short playerId)
{
	for(int i = 0; i < MAXCLIENTS; i++)
	{
		if(networkPlayers[i].active && networkPlayers[i].id == playerId)
			return &networkPlayers[i];
	}
	return 0;
}

networkPlayer_s *Network_ReturnFreePlayerSlot()
{
	for(int i = 0; i < MAXCLIENTS; i++)
	{
		if(!networkPlayers[i].active)
			return &networkPlayers[i];
	}
	return 0;
}

void Network_RemovePlayer(unsigned long long networkId)
{
	networkPlayer_s *player = Network_FindPlayerByNetworkId(networkId);
	if(player == 0)
		return;
	player->active = 0;
}

bool Network_AddPlayer(unsigned long long networkId, short playerId, char *name, unsigned char red, unsigned char green, unsigned char blue)
{
	networkPlayer_s *player = Network_ReturnFreePlayerSlot();
	if(player == 0)
		return 0;

	player->active = 1;
	player->networkId = networkId;
	player->id = playerId;
	player->red = red;
	player->green = green;
	player->blue = blue;
	strcpy_s(player->name, MAXPLAYERNAME, name);
	PrintToConsole("Added player %s with networkId %llu, playerId %i, and colours %u %u %u\n", name, networkId, playerId, red, green, blue);
	return 1;
}

void Network_UpdateGameAfterStoppingNetwork() //This is called by our when we fully terminate network code. Used by host and clients.
{
	//TODO: We should remove all entities related to other players
	//TODO: We should find entities own by us, and if they have a variable matching our player id, it should be changed to 0

	//After this is called, the network code will call Network_ResetPlayers() and reset all network player data including setting our own player id to 0
}

void Network_WeGotAcceptedIntoServer(int playerId) //We got accepted into the server and we just received our player id. Only used by clients.
{
	//TODO: If necessary, update entities owned by us to use new playerId
	networkPlayers[0].id = playerId;
	
	PrintToConsole("List of players:\n");
	for(int i = 0; i < MAXCLIENTS; i++)
	{
		if(networkPlayers[i].active)
			PrintToConsole("%s (networkId: %llu) (playerId: %i) (colour: %u %u %u)\n", networkPlayers[i].name, networkPlayers[i].networkId, networkPlayers[i].id, networkPlayers[i].red, networkPlayers[i].green, networkPlayers[i].blue);
	}

	//TODO: If necessary, send packets to server about entities owned by us so they can exist on all client game simulations
}

void Network_TemporarilySetOurPlayerIdToInvalid() //This happens as we try to join a server where we temporarily set our player id to -2 while waiting for a new id. Only used by clients.
{
	//TODO: If necessary, update entities owned by us to use temporary player id
	networkPlayers[0].id = -2;
}

void Network_NewPlayerConnected(SLNet::RakNetGUID guid) //This is called whenever a new player is added to the game. Only used by host.
{
	//TODO: If our game has data we need to sync up with our new client, we should send that data here
	//TODO: If we need to set up something in the game related to the client, we should do that here

	PrintToConsole("List of players:\n");
	for(int i = 0; i < MAXCLIENTS; i++)
	{
		if(networkPlayers[i].active)
			PrintToConsole("%s (networkId: %llu) (playerId: %i) (colour: %u %u %u)\n", networkPlayers[i].name, networkPlayers[i].networkId, networkPlayers[i].id, networkPlayers[i].red, networkPlayers[i].green, networkPlayers[i].blue);
	}
}

void Network_PlayerDisconnected(SLNet::RakNetGUID guid) //This is called whenever a new player is added to the game. Only used by host.
{
	//TODO: If anything needs to be removed from the game related to the player that just disconnected, we should do that here
}