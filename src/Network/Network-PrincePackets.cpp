#include "../Libs/slikenet/Include/WindowsIncludes.h" //Include this instead of "windows.h" as explained in Raknet FAQ
#include "../Libs/slikenet/Include/PacketPriority.h"
#include "../Libs/slikenet/Include/MessageIdentifiers.h"
#include "../Libs/slikenet/Include/Bitstream.h"
#include "Network-Main.h"
#include "Network-Intermediate.h"
#include "Network-Player.h"
#include "Network-Packets.h"
#include "..\Print.h"

void Network_Handle_KidPosition(SLNet::Packet *p)
{
	SLNet::BitStream bitStream(p->data, p->length, false);
	Network_Bitstream_SkipMessageId(&bitStream); //Ignore message id

	short id;
	bitStream.Read(id);
	networkPlayer_s *player = Network_FindPlayerByPlayerId(id);
	if(player)
	{
		bitStream.Read(player->frame);
		bitStream.Read(player->x);
		bitStream.Read(player->y);
		bitStream.Read(player->direction);
		bitStream.Read(player->room);
		bitStream.Read(player->alive);
		bitStream.Read(player->curr_seq);
		bitStream.Read(player->sword);
		bitStream.Read(player->current_level);
		bitStream.Read(player->curr_col);
		bitStream.Read(player->curr_row);
		bitStream.Read(player->action);
		bitStream.Read(player->fall_x);
		bitStream.Read(player->fall_y);
		bitStream.Read(player->repeat);
		bitStream.Read(player->charid);
	}
}

void Network_Send_KidPosition(unsigned char frame, unsigned char x, unsigned char y, char direction, unsigned char room, char alive, unsigned short curr_seq, unsigned char sword, unsigned short current_level, char curr_col, char curr_row, unsigned char action, char fall_x, char fall_y, unsigned char repeat, unsigned char charid) //Only sent by local player
{
	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, CUSTOMPACKETID_POP_KIDPOS);
	bitStream.Write(networkPlayers[0].id);
	bitStream.Write(frame);
	bitStream.Write(x);
	bitStream.Write(y);
	bitStream.Write(direction);
	bitStream.Write(room);
	bitStream.Write(alive);
	bitStream.Write(curr_seq);
	bitStream.Write(sword);
	bitStream.Write(current_level);
	bitStream.Write(curr_col);
	bitStream.Write(curr_row);
	bitStream.Write(action);
	bitStream.Write(fall_x);
	bitStream.Write(fall_y);
	bitStream.Write(repeat);
	bitStream.Write(charid);
	Network_SendBitStream_DefaultPacketProperties(&bitStream, SLNet::UNASSIGNED_RAKNET_GUID, 1, CUSTOMPACKETID_POP_KIDPOS);
}

void Network_Handle_UpdatePlayerColor(SLNet::Packet *p)
{
	SLNet::BitStream bitStream(p->data, p->length, false);
	Network_Bitstream_SkipMessageId(&bitStream); //Ignore message id

	unsigned long long networkId;
	bitStream.Read(networkId);
	networkPlayer_s *player = Network_FindPlayerByNetworkId(networkId);
	if(player)
	{
		bitStream.Read(player->red);
		bitStream.Read(player->green);
		bitStream.Read(player->blue);
		//TODO: Re-create player sprite if it was already loaded
	}
}

void Network_Send_UpdatePlayerColor(unsigned long long networkId, unsigned char red, unsigned char green, unsigned char blue) //TODO: Call this when colour is changed via an in-game menu or something like that
{
	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, CUSTOMPACKETID_POP_UPDATEPLAYERCOLOR);
	bitStream.Write(networkId);
	bitStream.Write(red);
	bitStream.Write(green);
	bitStream.Write(blue);
	Network_SendBitStream_DefaultPacketProperties(&bitStream, SLNet::UNASSIGNED_RAKNET_GUID, 1, CUSTOMPACKETID_POP_UPDATEPLAYERCOLOR);
}