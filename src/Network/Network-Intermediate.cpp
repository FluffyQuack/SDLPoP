#include "../Libs/slikenet/Include/WindowsIncludes.h" //Include this instead of "windows.h" as explained in Raknet FAQ
#include "../Libs/slikenet/Include/peerInterface.h"
#include "../Libs/slikenet/Include/MessageIdentifiers.h"
#include "../Libs/slikenet/Include/Bitstream.h"
#include "Network-Main.h"
#include "Network-Intermediate.h"
#include "Network-Player.h"
#include "Network-Packets.h"
#include "..\Print.h"
#include "Network-PrincePackets.h"

typedef struct char_type_intermediate {
	unsigned char frame;
	unsigned char x;
	unsigned char y;
	char direction;
	char curr_col;
	char curr_row;
	unsigned char action;
	char fall_x;
	char fall_y;
	unsigned char room;
	unsigned char repeat;
	unsigned char charid;
	unsigned char sword;
	char alive;
	unsigned short curr_seq;
} char_type_intermediate;

void Network_Intermediate_Start(int networkStart, int port, char *ip, char *name, unsigned char colourRed, unsigned char colourGreen, unsigned char colourBlue)
{
	Network_Initialize();
	CreateConsole();
	if(ip)
		strcpy_s(network_ip, NETWORKIP_LENGTH, ip);
	if(name)
		strcpy_s(networkPlayers[0].name, MAXPLAYERNAME, name);
	networkPlayers[0].red = colourRed;
	networkPlayers[0].green = colourGreen;
	networkPlayers[0].blue = colourBlue;
	if(port != -1)
		network_port = port;
	if(networkStart)
		Network_Start(networkStart == 1);
}

void Network_Update()
{
	Network_CheckForPackets();
}

void Network_Intermediate_Stop()
{
	Network_Stop();
	Network_Deinitialize();
}

void Network_Intermediate_SendPrinceData(unsigned char frame, unsigned char x, unsigned char y, char direction, unsigned char room, char alive, unsigned short curr_seq, unsigned char sword, unsigned short current_level, char curr_col, char curr_row, unsigned char action, char fall_x, char fall_y, unsigned char repeat, unsigned char charid)
{
	if(Network_ConnectedToAnyClient())
		Network_Send_KidPosition(frame, x, y, direction, room, alive, curr_seq, sword, current_level, curr_col, curr_row, action, fall_x, fall_y, repeat, charid);
}

bool Network_Intermediate_GetPrinceData(int playerNum, unsigned char *frame, unsigned char *x, unsigned char *y, char *direction, unsigned char *room, char *alive, unsigned short *curr_seq, unsigned char *sword, unsigned short *current_level, char *curr_col, char *curr_row, unsigned char *action, char *fall_x, char *fall_y, unsigned char *repeat, unsigned char *charid)
{
	if(playerNum >= 0 && playerNum < MAXCLIENTS && Network_ConnectedToAnyClient() && networkPlayers[playerNum].active)
	{
		networkPlayer_s *player = &networkPlayers[playerNum];
		*frame = player->frame;
		*x = player->x;
		*y = player->y;
		*direction = player->direction;
		*room = player->room;
		*alive = player->alive;
		*curr_seq = player->curr_seq;
		*sword = player->sword;
		*current_level = player->current_level;
		*curr_col = player->curr_col;
		*curr_row = player->curr_row;
		*action = player->action;
		*fall_x = player->fall_x;
		*fall_y = player->fall_y;
		*repeat = player->repeat;
		*charid = player->charid;
		return 1;
	}
	return 0;
}

void Network_Intermediate_GetPlayerColours(int playerNum, unsigned char *r, unsigned char *g, unsigned char *b)
{
	if(playerNum >= 0 && playerNum < MAXCLIENTS && (playerNum == 0 || Network_ConnectedToAnyClient()) && networkPlayers[playerNum].active)
	{
		networkPlayer_s *player = &networkPlayers[playerNum];
		*r = player->red;
		*g = player->green;
		*b = player->blue;

		PrintToConsole("guid: %llu, rgb: %u %u %u\n", player->networkId, player->red, player->green, player->blue);
	}
}