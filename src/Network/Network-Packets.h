#pragma once

//Custom id is an unsigned short with a max of around 30,000 (since parts of first byte is reserved by RakNet)
enum
{
	CUSTOMPACKETID_APPLICATIONANDPLAYERNAME, //Sent by clients just after they join a server
	CUSTOMPACKETID_ASKPLAYERTODISCONNECT, //Sent by server to client who are refused access to the server for a specified reason
	CUSTOMPACKETID_ADDPLAYER, //This is sent by server to telling other clients to add a player with name and GUID
	CUSTOMPACKETID_REMOVEPLAYER, //This is sent by server telling all clients to remove a player with a specific GUID
	CUSTOMPACKETID_GIVENEWPLAYERID, //This is sent by server to a new client in order to give them their new playerId
	CUSTOMPACKETID_CHANGEPLAYERSGUID,
	CUSTOMPACKETID_PLAYERLEAVING, //This is sent by a client to the server notifying they're shutting down network

	//Prince of Persia packets
	CUSTOMPACKETID_POP_KIDPOS,
	CUSTOMPACKETID_POP_UPDATEPLAYERCOLOR,

	CUSTOMPACKETID_NUM
};

struct packetTypeInfo_s
{
	void (*handleFunction) (SLNet::Packet *); //The function we'll call for handling this packet type
	bool relay; //If true, this packet will be relayed to all other clients if the host receives it from a client
	PacketReliability reliability;
	PacketPriority priority;
	char orderingchannel;
};

extern packetTypeInfo_s packetTypeInfo[CUSTOMPACKETID_NUM];

unsigned short Network_GetCustomIdFromPacketBytes(unsigned char *bytes);
unsigned short Network_GetPacketIdBytesFromCustomId(unsigned short customId);
void Network_Bitstream_SkipMessageId(SLNet::BitStream *bitStream);
void Network_Bitstream_WriteMessageId(SLNet::BitStream *bitStream, unsigned short realId);
void Network_Send_ChangePlayerGuid(unsigned long long oldGuid, unsigned long long newGuid);
void Network_Send_LeaveNotification();
void Network_Send_RemovePlayer(SLNet::RakNetGUID guid, unsigned long long id, bool broadcast);
void Network_Send_ApplicationAndPlayerName(SLNet::Packet *p);
void Network_HandleCustomPacketType(SLNet::Packet *p);
void Network_InitPacketTypeInfo();