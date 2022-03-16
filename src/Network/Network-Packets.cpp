#include "../Libs/slikenet/Include/WindowsIncludes.h" //Include this instead of "windows.h" as explained in Raknet FAQ
#include "../Libs/slikenet/Include/PacketPriority.h"
#include "../Libs/slikenet/Include/MessageIdentifiers.h"
#include "../Libs/slikenet/Include/Bitstream.h"
#include "Network-Main.h"
#include "Network-Intermediate.h"
#include "Network-Player.h"
#include "Network-Packets.h"
#include "Network-PrincePackets.h"
#include "..\Print.h"

enum
{
	DISCONNECTREASON_WRONGVERSION, //Client tried to join using the wrong application name or version
	DISCONNECTREASON_OUTOFPLAYERIDS, //Server failed to add player (either due to networkPlayer array being full or running out of playerIds)
	DISCONNECTREASON_DUPLICATENAME, //Client was refused to join because there's already an active player with the same name
};

static const char *applicationName = "Test";
static const char *applicationVersion = "Test";

packetTypeInfo_s packetTypeInfo[CUSTOMPACKETID_NUM];

unsigned short Network_GetCustomIdFromPacketBytes(unsigned char *bytes)
{
	//First byte is partially reserved by RakNet for internal packet types
	return ( (unsigned short) (bytes[0] - ID_USER_PACKET_ENUM) * 256) + bytes[1];
}

unsigned short Network_GetPacketIdBytesFromCustomId(unsigned short customId)
{
	unsigned short returnValue;
	((unsigned char *) &returnValue)[0] = ID_USER_PACKET_ENUM + (customId / 256);
	((unsigned char *) &returnValue)[1] = customId % 256;
	return returnValue;
}

void Network_Bitstream_SkipMessageId(SLNet::BitStream *bitStream)
{
	bitStream->IgnoreBytes(sizeof(SLNet::MessageID));
	bitStream->IgnoreBytes(sizeof(unsigned char));
}

void Network_Bitstream_WriteMessageId(SLNet::BitStream *bitStream, unsigned short realId)
{
	unsigned short messageId = Network_GetPacketIdBytesFromCustomId(realId);
	bitStream->Write( (SLNet::MessageID) ((unsigned char *) &messageId)[0]);
	bitStream->Write( (unsigned char &) ((unsigned char *) &messageId)[1]);
}

static void AddApplicationNameAndVersion(SLNet::BitStream *bitStream)
{
	//Application name
	unsigned char length = (unsigned char) strlen(applicationName);
	bitStream->Write(length); //Size of text
	bitStream->Write(applicationName, length); //Text

	//Application version
	length = (unsigned char) strlen(applicationVersion);
	bitStream->Write(length);
	bitStream->Write(applicationVersion, length);
}

void Network_Handle_ChangePlayerGuid(SLNet::Packet *p)
{
	SLNet::BitStream bitStream(p->data, p->length, false);
	Network_Bitstream_SkipMessageId(&bitStream); //Ignore message id

	unsigned long long oldId, newId;
	bitStream.Read(oldId);
	bitStream.Read(newId);
	networkPlayer_s *player = Network_FindPlayerByNetworkId(oldId);
	while(player)
	{
		player->networkId = newId;
		player = Network_FindPlayerByNetworkId(oldId);
	}
}

void Network_Send_ChangePlayerGuid(unsigned long long oldGuid, unsigned long long newGuid)
{
	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, CUSTOMPACKETID_CHANGEPLAYERSGUID);
	bitStream.Write(oldGuid);
	bitStream.Write(newGuid);
	Network_SendBitStream_DefaultPacketProperties(&bitStream, CUSTOMPACKETID_CHANGEPLAYERSGUID, SLNet::UNASSIGNED_RAKNET_GUID, 1);
}

void Network_Handle_AskPlayerToLeave(SLNet::Packet *p)
{
	SLNet::BitStream bitStream(p->data, p->length, false);
	Network_Bitstream_SkipMessageId(&bitStream); //Ignore message id

	char disconnectReason;
	bitStream.Read(disconnectReason);
	switch(disconnectReason)
	{
	case DISCONNECTREASON_WRONGVERSION:
	{
		char *str1 = 0, *str2 = 0;
		for(int i = 0; i < 2; i++)
		{
			unsigned char stringLength;
			bitStream.Read(stringLength);
			if(i == 0)
			{
				str1 = new char[stringLength + 1];
				bitStream.Read(str1, stringLength);
				str1[stringLength] = 0;
			}
			else
			{
				str2 = new char[stringLength + 1];
				bitStream.Read(str2, stringLength);
				str2[stringLength] = 0;
			}
		}

		PrintToConsole("Warning: We were refused access to the server because we've got the wrong application name or version.\nExpected: %s - %s\nOurs: %s - %s\n", str1, str2, applicationName, applicationVersion);
		delete[]str1;
		delete[]str2;
		break;
	}
	case DISCONNECTREASON_OUTOFPLAYERIDS:
	{
		PrintToConsole("Warning: We were refused access to the server because the server failed to add another network player.\n");
		break;
	}
	case DISCONNECTREASON_DUPLICATENAME:
	{
		PrintToConsole("Warning: We were refused access to the server because there's another player with our name.\n");
		break;
	}
	}

	network_scheduleTermination = 1;
}

void Network_Send_AskPlayerToLeave(SLNet::RakNetGUID guid, char disconnectReason)
{
	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, CUSTOMPACKETID_ASKPLAYERTODISCONNECT);
	bitStream.Write(disconnectReason);
	switch(disconnectReason)
	{
	case DISCONNECTREASON_WRONGVERSION:
	{
		AddApplicationNameAndVersion(&bitStream);
		break;
	}
	case DISCONNECTREASON_OUTOFPLAYERIDS:
	case DISCONNECTREASON_DUPLICATENAME:
	{
		break;
	}
	}

	Network_SendBitStream_DefaultPacketProperties(&bitStream, CUSTOMPACKETID_ASKPLAYERTODISCONNECT, guid, false);
}

void Network_Handle_LeaveNotification(SLNet::Packet *p)
{
	SLNet::BitStream bitStream(p->data, p->length, false);
	Network_Bitstream_SkipMessageId(&bitStream); //Ignore message id

	networkPlayer_s *player = Network_FindPlayerByNetworkId(p->guid.g);
	if(player)
		player->state |= NETWORKPLAYERSTATE_ABOUTTOLEAVE;
}

void Network_Send_LeaveNotification()
{
	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, CUSTOMPACKETID_PLAYERLEAVING);
	Network_SendBitStream_DefaultPacketProperties(&bitStream, CUSTOMPACKETID_PLAYERLEAVING, SLNet::UNASSIGNED_RAKNET_GUID, 1);
}

void Network_Handle_RemovePlayer(SLNet::Packet *p)
{
	SLNet::BitStream bitStream(p->data, p->length, false);
	Network_Bitstream_SkipMessageId(&bitStream); //Ignore message id

	unsigned long long id;
	bitStream.Read(id);
	Network_RemovePlayer(id);
}

void Network_Send_RemovePlayer(SLNet::RakNetGUID guid, unsigned long long id, bool broadcast)
{
	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, CUSTOMPACKETID_REMOVEPLAYER);
	bitStream.Write(id);
	Network_SendBitStream_DefaultPacketProperties(&bitStream, CUSTOMPACKETID_REMOVEPLAYER, guid, broadcast);
}

void Network_Handle_AddPlayer(SLNet::Packet *p)
{
	SLNet::BitStream bitStream(p->data, p->length, false);
	Network_Bitstream_SkipMessageId(&bitStream); //Ignore message id

	unsigned long long networkId;
	bitStream.Read(networkId);
	short playerId;
	bitStream.Read(playerId);
	unsigned char stringLength;
	bitStream.Read(stringLength);
	char *str = new char[stringLength + 1];
	bitStream.Read(str, stringLength);
	str[stringLength] = 0;
	unsigned char red, green, blue;
	bitStream.Read(red);
	bitStream.Read(green);
	bitStream.Read(blue);
	if(!Network_AddPlayer(networkId, playerId, str, red, green, blue))
		PrintToConsole("Error: Failed to add player during Network_Handle_AddPlayer()\n");
	delete[]str;
}

void Network_Send_AddPlayer(SLNet::RakNetGUID guid, unsigned long long networkId, short playerId, char *name, unsigned char red, unsigned char green, unsigned char blue, bool broadcast)
{
	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, CUSTOMPACKETID_ADDPLAYER);

	bitStream.Write(networkId);
	bitStream.Write(playerId);

	//Player name
	unsigned char length = (unsigned char) strlen(name);
	bitStream.Write(length);
	bitStream.Write(name, length);
	bitStream.Write(red);
	bitStream.Write(green);
	bitStream.Write(blue);

	Network_SendBitStream_DefaultPacketProperties(&bitStream, CUSTOMPACKETID_ADDPLAYER, guid, broadcast);
}

void Network_Handle_GiveNewPlayerTheirPlayerId(SLNet::Packet *p)
{
	SLNet::BitStream bitStream(p->data, p->length, false);
	Network_Bitstream_SkipMessageId(&bitStream); //Ignore message id

	short playerId;
	bitStream.Read(playerId);
	Network_WeGotAcceptedIntoServer(playerId);
}

void Network_Send_GiveNewPlayerTheirPlayerId(SLNet::RakNetGUID guid, short playerId)
{
	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, CUSTOMPACKETID_GIVENEWPLAYERID);

	bitStream.Write(playerId);

	Network_SendBitStream_DefaultPacketProperties(&bitStream, CUSTOMPACKETID_GIVENEWPLAYERID, guid, 0);
}

static void Network_Handle_ApplicationAndPlayerName(SLNet::Packet *p)
{
	char disconnectReason = -1;
	SLNet::BitStream bitStream(p->data, p->length, false);
	Network_Bitstream_SkipMessageId(&bitStream); //Ignore message id

	//Note: This would be a good place to check if we allow any players to connect to the game (ie, we're in the middle of gameplay and we don't players to connect, even if it's a reconnecting player)

	for(int i = 0; i < 3; i++) //In order, we're reading these strings: application name, application version, player name
	{
		unsigned char stringLength;
		bitStream.Read(stringLength);
		char *str = new char[stringLength + 1];
		bitStream.Read(str, stringLength);
		str[stringLength] = 0;
		if((i == 0 && strcmp(applicationName, str) != 0)
			|| (i == 1 && strcmp(applicationVersion, str) != 0))
		{
			delete[]str;
			disconnectReason = DISCONNECTREASON_WRONGVERSION;
			PrintToConsole("Disconnected from new client as they're running the wrong version and/or application name.\n");
			goto ask_player_to_leave;
		}
		else if(i == 2)
		{
			unsigned char red, green, blue;
			bitStream.Read(red);
			bitStream.Read(green);
			bitStream.Read(blue);

			//Check if there's an active player with the new client's name. If found, we ask the player to leave as we don't allow multiple users with the same name
			for(int j = 0; j < NETWORK_MAXCLIENTS; j++)
			{
				if(networkPlayers[j].state & NETWORKPLAYERSTATE_ACTIVE && !(networkPlayers[j].state & NETWORKPLAYERSTATE_DISCONNECTED) && strcmp(networkPlayers[j].name, str) == 0)
				{
					disconnectReason = DISCONNECTREASON_DUPLICATENAME;
					PrintToConsole("Disconnected from new client as they've got the same name as another client.\n");
					goto ask_player_to_leave;
				}
			}

			//Check if this is a player who used to be connected
			short playerId;
			networkPlayer_s *player = Network_FindDisconnectedPlayerByName(str);
			bool reconnectingPlayer = 0;
			if(player) //Player used to be connected, so use same slot and same playerId
			{
				playerId = player->id;
				player->state = NETWORKPLAYERSTATE_ACTIVE;
				player->networkId = p->guid.g;
				player->red = red;
				player->green = green;
				player->blue = blue;
				reconnectingPlayer = 1;
				PrintToConsole("Reconnected player %s with networkId %llu, playerId %i, and colours %u %u %u\n", str, p->guid.g, playerId, red, green, blue);
			}
			
			//Note: This would be a good place to check if we allow any non-reconnecting players to connect to the game (ie, we're in the middle of gameplay)

			if(!reconnectingPlayer)
				playerId = Network_GetUnusedPlayerId();

			if(playerId == -1) //This is absurdly unlikely to happen
			{
				delete[]str;
				disconnectReason = DISCONNECTREASON_OUTOFPLAYERIDS;
				PrintToConsole("Disconnected from new client because we somehow ran out of player Ids.\n");
				goto ask_player_to_leave;
			}
			if(!reconnectingPlayer)
			{
				if(!Network_AddPlayer(p->guid.g, playerId, str, red, green, blue)) //Add player
				{
					delete[]str;
					disconnectReason = DISCONNECTREASON_OUTOFPLAYERIDS;
					PrintToConsole("Disconnected from new client because networkPlayer array is full.\n");
					goto ask_player_to_leave;
				}
			}
			Network_Send_AddPlayer(p->guid, p->guid.g, playerId, str, red, green, blue, 1); //Let other clients know about this new player

			for(int j = 0; j < NETWORK_MAXCLIENTS; j++) //Let the new player know about other clients (including ourself)
			{
				if(networkPlayers[j].state & NETWORKPLAYERSTATE_ACTIVE && !(networkPlayers[j].state & NETWORKPLAYERSTATE_DISCONNECTED) && networkPlayers[j].networkId != p->guid.g)
					Network_Send_AddPlayer(p->guid, networkPlayers[j].networkId, networkPlayers[j].id, networkPlayers[j].name, networkPlayers[j].red, networkPlayers[j].green, networkPlayers[j].blue, 0);
			}

			//Send a packet to the new player with their new playerid
			Network_Send_GiveNewPlayerTheirPlayerId(p->guid, playerId);

			if(reconnectingPlayer)
				Network_PlayerReconnected(p->guid);
			else
				Network_NewPlayerConnected(p->guid);
		}

		delete[]str;
	}
	return;

ask_player_to_leave:
	Network_Send_AskPlayerToLeave(p->guid, disconnectReason);
	return;
}

void Network_Send_ApplicationAndPlayerName(SLNet::Packet *p) //This is sent by clients to the host after connection has been established
{
	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, CUSTOMPACKETID_APPLICATIONANDPLAYERNAME);

	AddApplicationNameAndVersion(&bitStream);

	//Player name
	unsigned char length = (unsigned char) strlen(networkPlayers[0].name);
	bitStream.Write(length);
	bitStream.Write(networkPlayers[0].name, length);
	bitStream.Write(networkPlayers[0].red);
	bitStream.Write(networkPlayers[0].green);
	bitStream.Write(networkPlayers[0].blue);

	Network_SendBitStream_DefaultPacketProperties(&bitStream, CUSTOMPACKETID_APPLICATIONANDPLAYERNAME, p->guid, false);
}

static unsigned char *GetPacketIdentifierPointer(SLNet::Packet *p)
{
	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
		return &p->data[sizeof(unsigned char) + sizeof(unsigned long)];

	return &p->data[0];
}

void Network_HandleCustomPacketType(SLNet::Packet *p) //Handle any packet type that isn't natively defined within RakNet
{
	unsigned short id = Network_GetCustomIdFromPacketBytes(GetPacketIdentifierPointer(p));
	if(id >= CUSTOMPACKETID_NUM)
	{
		PrintToConsole("Warning: Received unsupported custom packet id %u.", id);
		return;
	}

	if(packetTypeInfo[id].handleFunction != 0)
		(*packetTypeInfo[id].handleFunction)(p);

	if(Network_HostingForAnyClient() && packetTypeInfo[id].relay) //If we're the server, we wanna relay certain packets (so that packets sent by clients will reach remote clients)
		Network_SendPacket( (const char *) p->data, p->bitSize / 8 , p->guid, true, packetTypeInfo[id].reliability, packetTypeInfo[id].priority, packetTypeInfo[id].orderingchannel);
}

static void DefinePacketTypeInfo(unsigned short packetType, void (*handleFunction) (SLNet::Packet *), bool relay, PacketReliability reliability = RELIABLE, PacketPriority priority = MEDIUM_PRIORITY, char orderingchannel = 0)
{
	packetTypeInfo[packetType].handleFunction = handleFunction;
	packetTypeInfo[packetType].relay = relay;
	packetTypeInfo[packetType].reliability = reliability;
	packetTypeInfo[packetType].priority = priority;
	packetTypeInfo[packetType].orderingchannel = orderingchannel;
}

void Network_InitPacketTypeInfo()
{
	for(int i = 0; i < CUSTOMPACKETID_NUM; i++)
	{
		packetTypeInfo[i].handleFunction = 0;
		packetTypeInfo[i].relay = 0;
		packetTypeInfo[i].reliability = RELIABLE;
		packetTypeInfo[i].priority = MEDIUM_PRIORITY;
		packetTypeInfo[i].orderingchannel = 0;
	}

	//Define packet properties
	DefinePacketTypeInfo(CUSTOMPACKETID_APPLICATIONANDPLAYERNAME, Network_Handle_ApplicationAndPlayerName, 0, RELIABLE_ORDERED, HIGH_PRIORITY);
	DefinePacketTypeInfo(CUSTOMPACKETID_ASKPLAYERTODISCONNECT, Network_Handle_AskPlayerToLeave, 0, RELIABLE_ORDERED, HIGH_PRIORITY);
	DefinePacketTypeInfo(CUSTOMPACKETID_ADDPLAYER, Network_Handle_AddPlayer, 0, RELIABLE_ORDERED, HIGH_PRIORITY);
	DefinePacketTypeInfo(CUSTOMPACKETID_REMOVEPLAYER, Network_Handle_RemovePlayer, 0, RELIABLE_ORDERED, HIGH_PRIORITY);
	DefinePacketTypeInfo(CUSTOMPACKETID_GIVENEWPLAYERID, Network_Handle_GiveNewPlayerTheirPlayerId, 0, RELIABLE_ORDERED, HIGH_PRIORITY);
	DefinePacketTypeInfo(CUSTOMPACKETID_CHANGEPLAYERSGUID, Network_Handle_ChangePlayerGuid, 0, RELIABLE_ORDERED, HIGH_PRIORITY);
	DefinePacketTypeInfo(CUSTOMPACKETID_PLAYERLEAVING, 0, 0, RELIABLE_ORDERED, HIGH_PRIORITY);

	//Prince of Persia packets
	DefinePacketTypeInfo(CUSTOMPACKETID_POP_KIDPOS, Network_Handle_KidPosition, 1, UNRELIABLE, IMMEDIATE_PRIORITY);
	DefinePacketTypeInfo(CUSTOMPACKETID_POP_UPDATEPLAYERCOLOR, Network_Handle_UpdatePlayerColor, 1, RELIABLE, MEDIUM_PRIORITY);
}