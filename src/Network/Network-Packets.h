#pragma once

//Custom id is an unsigned short with a max of around 30,000 (since parts of first byte is reserved by RakNet)
enum
{
	CUSTOMPACKETID_APPLICATIONANDPLAYERNAME, //Sent by clients just after they join a server
	CUSTOMPACKETID_ASKPLAYERTODISCONNECT_WRONGVERSION, //Sent by server to clients that don't have matching application name and version
	CUSTOMPACKETID_ASKPLAYERTODISCONNECT_CANTADDPLAYER, //Sent by server if server failed to add another network player
	CUSTOMPACKETID_ADDPLAYER, //This is sent by server to telling other clients to add a player with name and GUID
	CUSTOMPACKETID_REMOVEPLAYER, //This is sent by server telling all clients to remove a player with a specific GUID
	CUSTOMPACKETID_GIVENEWPLAYERID, //This is sent by server to a new client in order to give them their new playerId
	CUSTOMPACKETID_CHANGEPLAYERSGUID,

	//Prince of Persia packets
	CUSTOMPACKETID_POP_KIDPOS,
	CUSTOMPACKETID_POP_UPDATEPLAYERCOLOR,

	CUSTOMPACKETID_NUM
};

void Network_Bitstream_SkipMessageId(SLNet::BitStream *bitStream);
void Network_Bitstream_WriteMessageId(SLNet::BitStream *bitStream, unsigned short realId);
void Network_SendBitStream_DefaultPacketProperties(SLNet::BitStream *bitStream, SLNet::AddressOrGUID destination, bool broadcast, unsigned short id);
void Network_Send_PlayerSaysHello();
void Network_Send_ChangePlayerGuid(unsigned long long oldGuid, unsigned long long newGuid);
void Network_Send_RemovePlayer(SLNet::RakNetGUID guid, unsigned long long id, bool broadcast);
void Network_Send_ApplicationAndPlayerName(SLNet::Packet *p);
void Network_HandleCustomPacketType(SLNet::Packet *p);
void Network_InitPacketTypeInfo();