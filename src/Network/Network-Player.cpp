#include "../Libs/slikenet/Include/WindowsIncludes.h" //Include this instead of "windows.h" as explained in Raknet FAQ
#include "../Libs/slikenet/Include/types.h"
#include "../Libs/slikenet/Include/PacketPriority.h"
#include "Network-Main.h"
#include "Network-Intermediate.h"
#include "Network-Player.h"
#include "../Print.h"

networkPlayer_s networkPlayers[NETWORK_MAXCLIENTS]; //First entry in the array is always the local player
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
		for(int i = 0; i < NETWORK_MAXCLIENTS; i++)
		{
			if(networkPlayers[i].state != NETWORKPLAYERSTATE_UNUSED && networkPlayers[i].networkId == lastAssignedPlayerId)
			{
				isPlayerIdUsed = 1;
				break;
			}
		}
		if(isPlayerIdUsed == 0)
			return lastAssignedPlayerId;
	}
}

void Network_ResetPlayers(bool resetAll, bool resetLocalPlayerId) //Called when starting program or when user turns off network
{
	memset(&networkPlayers[1], 0, sizeof(networkPlayer_s) * (NETWORK_MAXCLIENTS - 1));
	networkPlayers[0].state = NETWORKPLAYERSTATE_ACTIVE;
	lastAssignedPlayerId = 0;
	if(resetAll) //If true, we also reset the local player's id and name. This should only ever be done once as we start the program.
	{
		networkPlayers[0].networkId = 0;
		networkPlayers[0].name[0] = 0;
	}

	if(resetLocalPlayerId) //This only happens when we fully terminate network connection
		networkPlayers[0].id = 0;
}

networkPlayer_s *Network_FindDisconnectedPlayerByName(char *name)
{
	for(int i = 0; i < NETWORK_MAXCLIENTS; i++)
	{
		if(networkPlayers[i].state & NETWORKPLAYERSTATE_DISCONNECTED && strcmp(networkPlayers[i].name, name) == 0)
			return &networkPlayers[i];
	}
	return 0;
}

networkPlayer_s *Network_FindPlayerByNetworkId(unsigned long long networkId)
{
	for(int i = 0; i < NETWORK_MAXCLIENTS; i++)
	{
		if(networkPlayers[i].state & NETWORKPLAYERSTATE_ACTIVE && networkPlayers[i].networkId == networkId)
			return &networkPlayers[i];
	}
	return 0;
}

networkPlayer_s *Network_FindPlayerByPlayerId(short playerId)
{
	for(int i = 0; i < NETWORK_MAXCLIENTS; i++)
	{
		if(networkPlayers[i].state & NETWORKPLAYERSTATE_ACTIVE && networkPlayers[i].id == playerId)
			return &networkPlayers[i];
	}
	return 0;
}

int Network_FindPlayerByPlayerId_ReturnIndex(short playerId)
{
	for(int i = 0; i < NETWORK_MAXCLIENTS; i++)
	{
		if(networkPlayers[i].state & NETWORKPLAYERSTATE_ACTIVE && networkPlayers[i].id == playerId)
			return i;
	}
	return -1;
}

networkPlayer_s *Network_ReturnFreePlayerSlot()
{
	for(int i = 0; i < NETWORK_MAXCLIENTS; i++)
	{
		if(networkPlayers[i].state == NETWORKPLAYERSTATE_UNUSED)
			return &networkPlayers[i];
	}
	return 0;
}

void Network_MarkPlayerAsDisconnected(unsigned long long networkId) //Used by host to mark a player as disconnected after losing connection
{
	networkPlayer_s *player = Network_FindPlayerByNetworkId(networkId);
	if(player == 0)
		return;
	player->state |= NETWORKPLAYERSTATE_DISCONNECTED;
	player->state &= ~NETWORKPLAYERSTATE_ACTIVE;
}

void Network_RemovePlayer(unsigned long long networkId) //Used by clients as remote clients disconnect, also used by host if a client manually quits the session
{
	networkPlayer_s *player = Network_FindPlayerByNetworkId(networkId);
	if(player == 0)
		return;
	player->state = NETWORKPLAYERSTATE_UNUSED;
}

bool Network_AddPlayer(unsigned long long networkId, short playerId, char *name, unsigned char red, unsigned char green, unsigned char blue) //Used by both host and client in order to add a new player. Note: This is NOT called by host if a player is found to be re-connecting
{
	networkPlayer_s *player = Network_ReturnFreePlayerSlot();
	if(player == 0)
		return 0;

	player->state = NETWORKPLAYERSTATE_ACTIVE;
	player->networkId = networkId;
	player->id = playerId;
	player->red = red;
	player->green = green;
	player->blue = blue;
	strcpy_s(player->name, NETWORK_MAXPLAYERNAME, name);
	PrintToConsole("Added player %s with networkId %llu, playerId %i, and colours %u %u %u\n", name, networkId, playerId, red, green, blue);
	return 1;
}

void Network_UpdateGameAfterStoppingNetwork() //This is called by our when we fully terminate network code. Used by host and clients.
{
	//Note: We should remove all entities related to other players
	//Note: We should find entities own by us, and if they have a variable matching our player id, it should be changed to 0

	//After this is called, the network code will call Network_ResetPlayers() and reset all network player data including setting our own player id to 0
}

void Network_WeGotAcceptedIntoServer(int playerId) //We got accepted into the server and we just received our player id. Only used by clients.
{
	//Note: If necessary, update entities owned by us to use new playerId
	networkPlayers[0].id = playerId;
	
	PrintToConsole("List of players:\n");
	for(int i = 0; i < NETWORK_MAXCLIENTS; i++)
	{
		if(networkPlayers[i].state == NETWORKPLAYERSTATE_ACTIVE)
			PrintToConsole("%s (networkId: %llu) (playerId: %i) (colour: %u %u %u)\n", networkPlayers[i].name, networkPlayers[i].networkId, networkPlayers[i].id, networkPlayers[i].red, networkPlayers[i].green, networkPlayers[i].blue);
	}

	//Note: If necessary, send packets to server about entities owned by us so they can exist on all client game simulations
}

void Network_TemporarilySetOurPlayerIdToInvalid() //This happens as we try to join a server where we temporarily set our player id to -2 while waiting for a new id. Only used by clients.
{
	//Note: If necessary, update entities owned by us to use temporary player id
	networkPlayers[0].id = -2;
}

void Network_NewPlayerConnected(SLNet::RakNetGUID guid) //This is called whenever a new player is added to the game. Only used by host.
{
	//Note: If our game has data we need to sync up with our new client, we should send that data here
	//Note: If we need to set up something in the game related to the client, we should do that here

	PrintToConsole("List of players:\n");
	for(int i = 0; i < NETWORK_MAXCLIENTS; i++)
	{
		if(networkPlayers[i].state == NETWORKPLAYERSTATE_ACTIVE)
			PrintToConsole("%s (networkId: %llu) (playerId: %i) (colour: %u %u %u)\n", networkPlayers[i].name, networkPlayers[i].networkId, networkPlayers[i].id, networkPlayers[i].red, networkPlayers[i].green, networkPlayers[i].blue);
	}
}

void Network_PlayerReconnected(SLNet::RakNetGUID guid) //This is called whenever a player connects after having fully disconnected earlier. Only used by host.
{
	//Note: Do stuff if there's anything we need to re-sync entitites the player used to own

	PrintToConsole("List of players:\n");
	for(int i = 0; i < NETWORK_MAXCLIENTS; i++)
	{
		if(networkPlayers[i].state == NETWORKPLAYERSTATE_ACTIVE)
			PrintToConsole("%s (networkId: %llu) (playerId: %i) (colour: %u %u %u)\n", networkPlayers[i].name, networkPlayers[i].networkId, networkPlayers[i].id, networkPlayers[i].red, networkPlayers[i].green, networkPlayers[i].blue);
	}
}

void Network_PlayerLeft(SLNet::RakNetGUID guid) //This is called whenever a player requests to leave from our game. Only used by host.
{
	//Note: This means that a player has manually quit the session and we should remove all traces of the player. Ie, any entities belonging to him in the game.
}

void Network_PlayerDisconnected(SLNet::RakNetGUID guid) //This is called whenever a player disconnects from our game. Only used by host.
{
	//Note: If anything needs to be removed from the game related to the player that just disconnected, we should do that here (we should remember that we allow clients to re-connect into the same player slot, so it would be beneficial if entities can be re-used somehow). The player is flagged as a connected player during this function, but Network_MarkPlayerAsDisconnected() is called immediately afterwards
}