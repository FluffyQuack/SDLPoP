#pragma once

#define MAXPLAYERNAME 50

struct networkPlayer_s
{
	bool active;
	char name[MAXPLAYERNAME];
	unsigned long long networkId; //Unique identifier for this player. This is set when initializing RakNet, and should stay the same value after connecting to a client (I've got code for handling a guid change after connection, though I'm not sure if that's likely to actually ever happen)
	short id; //Another unique identifer. This is set by our netcode and is meant to be used as an identifier for gameplay packets (using this id more efficient than the above id since it's 6 bytes shorter). An id of 0 always means the server's player

	//Player colour
	unsigned char red;
	unsigned char green;
	unsigned char blue;

	//Kid data
	unsigned char frame;
	unsigned char x;
	unsigned char y;
	char direction;
	unsigned char room;
	char alive;
	unsigned short curr_seq;
	unsigned char sword;
	unsigned short current_level;
	char curr_col;
	char curr_row;
	unsigned char action;
	char fall_x;
	char fall_y;
	unsigned char repeat;
	unsigned char charid;
};

extern networkPlayer_s networkPlayers[MAXCLIENTS];

short Network_GetUnusedPlayerId();
void Network_ResetPlayers(bool resetAll, bool resetLocalPlayerId);
networkPlayer_s *Network_FindPlayerByNetworkId(unsigned long long networkId);
networkPlayer_s *Network_FindPlayerByPlayerId(short playerId);
void Network_RemovePlayer(unsigned long long networkId);
bool Network_AddPlayer(unsigned long long networkId, short playerId, char *name, unsigned char red, unsigned char green, unsigned char blue);
void Network_UpdateGameAfterStoppingNetwork();
void Network_WeGotAcceptedIntoServer(int playerId);
void Network_TemporarilySetOurPlayerIdToInvalid();
void Network_NewPlayerConnected(SLNet::RakNetGUID guid);
void Network_PlayerDisconnected(SLNet::RakNetGUID guid);