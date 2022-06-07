#pragma once

enum
{
	NETWORKSTATUS_OFF,
	NETWORKSTATUS_SERVER_HOSTING,
	NETWORKSTATUS_CLIENT_JOINING,
	NETWORKSTATUS_CLIENT_CONNECTED
};

#define NETWORKIP_LENGTH 16
extern int network_status;
extern int network_clientsConnectedTo;
extern char network_ip[NETWORKIP_LENGTH];
extern unsigned short network_port;
extern bool network_scheduleTermination;

void Network_Initialize();
void Network_Deinitialize();
void Network_Start(bool host);
void Network_Stop();
void Network_SendPacket_MessageIdOnly(unsigned short messageId, SLNet::AddressOrGUID destination = SLNet::UNASSIGNED_RAKNET_GUID, bool broadcast = 1, PacketReliability reliability = RELIABLE, PacketPriority priority = MEDIUM_PRIORITY, char orderingchannel = 0);
void Network_SendPacket_MessageIdOnly_DefaultPacketProperties(unsigned short messageId, SLNet::AddressOrGUID destination = SLNet::UNASSIGNED_RAKNET_GUID, bool broadcast = 1);
void Network_SendPacket(const char *data , int packetsize, SLNet::AddressOrGUID destination = SLNet::UNASSIGNED_RAKNET_GUID, bool broadcast = 1, PacketReliability reliability = RELIABLE, PacketPriority priority = MEDIUM_PRIORITY, char orderingchannel = 0);
void Network_SendPacket_DefaultPacketProperties(const char *data, int packetsize, unsigned short id, SLNet::AddressOrGUID destination = SLNet::UNASSIGNED_RAKNET_GUID, bool broadcast = 1);
void Network_SendBitStream(SLNet::BitStream *bitStream, SLNet::AddressOrGUID destination = SLNet::UNASSIGNED_RAKNET_GUID, bool broadcast = 1, PacketReliability reliability = RELIABLE, PacketPriority priority = MEDIUM_PRIORITY, char orderingchannel = 0);
void Network_SendBitStream_DefaultPacketProperties(SLNet::BitStream *bitStream, unsigned short id, SLNet::AddressOrGUID destination = SLNet::UNASSIGNED_RAKNET_GUID, bool broadcast = 1);
bool Network_ConnectedToAnyClient();
bool Network_HostingForAnyClient();
void Network_CheckForPackets();
