#include "../Libs/slikenet/Include/WindowsIncludes.h" //Include this instead of "windows.h" as explained in Raknet FAQ
#include "../Libs/slikenet/Include/peerInterface.h"
#include "../Libs/slikenet/Include/MessageIdentifiers.h"
#include "../Libs/slikenet/Include/Bitstream.h"
#include "Network-Main.h"
#include "Network-Intermediate.h"
#include "Network-Player.h"
#include "Network-Packets.h"
#include "..\Print.h"

#pragma comment(lib, "ws2_32.lib")

#if defined(_DEBUG) && !defined(_WIN64)
#pragma comment(lib, "libs\\slikenet\\Libs\\SLikeNet_LibStatic_Debug_Win32.lib")
#endif

#if defined(_DEBUG) && defined(_WIN64)
#pragma comment(lib, "libs\\slikenet\\Libs\\SLikeNet_LibStatic_Debug_x64.lib")
#endif

#if !defined(_DEBUG) && !defined(_WIN64)
#pragma comment(lib, "libs\\slikenet\\Libs\\SLikeNet_LibStatic_Retail_Win32.lib")
#endif

#if !defined(_DEBUG) && defined(_WIN64)
#pragma comment(lib, "libs\\slikenet\\Libs\\SLikeNet_LibStatic_Retail_x64.lib")
#endif

static SLNet::RakPeerInterface *peer =  0;
int network_status = NETWORKSTATUS_OFF;
int network_clientsConnectedTo = 0;
static unsigned long long network_bytesSent = 0;
static unsigned long long network_bytesReceived = 0;
char network_ip[NETWORKIP_LENGTH] = "127.0.0.1";
unsigned short network_port = 9950;
bool network_scheduleTermination = 0;

void Network_Initialize()
{
	Network_InitPacketTypeInfo();
	Network_ResetPlayers(1, 1);
	peer = SLNet::RakPeerInterface::GetInstance();
	if(peer == 0)
	{
		PrintToConsole("Warning: Failed to get instance of raknet peer interface\n");
		return;
	}
	networkPlayers[0].networkId = peer->GetMyGUID().g;
	peer->SetOccasionalPing(1);
	peer->SetTimeoutTime(60000, SLNet::UNASSIGNED_SYSTEM_ADDRESS);
	PrintToConsole("Network initialized.\n");
}

void Network_Deinitialize()
{
	if(network_status != NETWORKSTATUS_OFF)
		Network_Stop();
	if(peer == 0)
		return;
	SLNet::RakPeerInterface::DestroyInstance(peer);
	peer = 0;
}

void Network_Start(bool host)
{
	if(peer == 0)
	{
		PrintToConsole("Warning: Can't start network since network init failed.\n");
		return;
	}

	if(network_status != NETWORKSTATUS_OFF)
	{
		PrintToConsole("Warning: Network status isn't off during Network_Start()\n");
		return;
	}

	//Init
	networkPlayers[0].networkId = peer->GetMyGUID().g;
	PrintToConsole("Our GUID: %llu\n", networkPlayers[0].networkId);

	//Start hosting or joining
	if(host)
		PrintToConsole("Attempting to host server.\n");
	else
		PrintToConsole("Attempting to join server.\n");
	SLNet::SocketDescriptor socketDescriptor;
	SLNet::StartupResult success;
	if(host)
	{
		socketDescriptor.port = network_port;
		success = peer->Startup(NETWORK_MAXCLIENTS, &socketDescriptor, 1);
	}
	else //Joining server
	{
		socketDescriptor.port = 0;
		success = peer->Startup(1, &socketDescriptor, 1); //Allow connecting with only one other client
	}

	if(success != SLNet::RAKNET_STARTED && success != SLNet::RAKNET_ALREADY_STARTED)
	{
		if(success == SLNet::INVALID_SOCKET_DESCRIPTORS)
			PrintToConsole("Warning: Could not start RakNet. Invalid sock descriptors.\n");
		else if(success == SLNet::INVALID_MAX_CONNECTIONS)
			PrintToConsole("Warning: Could not start RakNet. Invalid max connections.\n");
		else if(success == SLNet::SOCKET_FAMILY_NOT_SUPPORTED)
			PrintToConsole("Warning: Could not start RakNet. Sock family not supported.\n");
		else if(success == SLNet::SOCKET_PORT_ALREADY_IN_USE)
			PrintToConsole("Warning: Could not start RakNet. Sock is already use.\n");
		else if(success == SLNet::SOCKET_FAILED_TO_BIND)
			PrintToConsole("Warning: Could not start RakNet. Sock failed to bind.\n");
		else if(success == SLNet::SOCKET_FAILED_TEST_SEND)
			PrintToConsole("Warning: Could not start RakNet. Sock failed send test.\n");
		else if(success == SLNet::PORT_CANNOT_BE_ZERO)
			PrintToConsole("Warning: Could not start RakNet. Port can't be zero.\n");
		else if(success == SLNet::FAILED_TO_CREATE_NETWORK_THREAD)
			PrintToConsole("Warning: Could not start RakNet. Failed to create network thread.\n");
		else if(success == SLNet::COULD_NOT_GENERATE_GUID)
			PrintToConsole("Warning: Could not start RakNet. Could not generate GUID.\n");
		else if(success == SLNet::COULD_NOT_GENERATE_GUID)
			PrintToConsole("Warning: Could not start RakNet. Something went wrong.\n");
		Network_Stop();
		return;
	}

	if(host)
	{
		peer->SetMaximumIncomingConnections(NETWORK_MAXCLIENTS);
		PrintToConsole("Successfully started hosting server.\n");
		network_status = NETWORKSTATUS_SERVER_HOSTING;
	}
	else //Joining server
	{
		SLNet::ConnectionAttemptResult success2 = peer->Connect(network_ip, network_port, 0, 0, 0);
		if(success2 != SLNet::CONNECTION_ATTEMPT_STARTED)
		{
			if(success2 == SLNet::INVALID_PARAMETER)
				PrintToConsole("Warning: Could not connect to server. Invalid parameters.\n");
			else if(success2 == SLNet::CANNOT_RESOLVE_DOMAIN_NAME)
				PrintToConsole("Warning: Could not connect to server. Can't resolve domain name.\n");
			else if(success2 == SLNet::ALREADY_CONNECTED_TO_ENDPOINT)
				PrintToConsole("Warning: Could not connect to server. Already connected to end point.\n");
			else if(success2 == SLNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS)
				PrintToConsole("Warning: Could not connect to server. Connectiong attempt already in progress.\n");
			else if(success2 == SLNet::SECURITY_INITIALIZATION_FAILED)
				PrintToConsole("Warning: Could not connect to server. Security initialized failed.\n");
			Network_Stop();
			return;
		}
		PrintToConsole("Successfully sent connection request. Waiting for connection request to be accepted.\n");
		network_status = NETWORKSTATUS_CLIENT_JOINING;

		//Temporarily set our new playerid to -2
		Network_TemporarilySetOurPlayerIdToInvalid();
	}

	network_bytesSent = 0;
	network_bytesReceived = 0;
	network_clientsConnectedTo = 0;
	network_scheduleTermination = 0;
}

void Network_Stop()
{
	if(peer == 0)
	{
		PrintToConsole("Warning: Raknet peer interface pointer is null during Network_Stop()\n");
		return;
	}
	if(network_status == NETWORKSTATUS_OFF)
	{
		PrintToConsole("Warning: Network status is already off during Network_Stop()\n");
		return;
	}
	PrintToConsole("Attempting to shut down network.\n");

	if(network_status == NETWORKSTATUS_CLIENT_CONNECTED) //If we're a client, then send a packet to host telling them we're leaving
		Network_SendPacket_MessageIdOnly_DefaultPacketProperties(CUSTOMPACKETID_PLAYERLEAVING);

	if(network_status != NETWORKSTATUS_OFF)
		peer->Shutdown(1000);
	PrintToConsole("Network shut down.\n");
	network_status = NETWORKSTATUS_OFF;

	//Let game know that we're fully turning off network
	Network_UpdateGameAfterStoppingNetwork();

	//Reset info about network players
	Network_ResetPlayers(0, 1);
	network_clientsConnectedTo = 0;
	network_scheduleTermination = 0;
}

void Network_SendPacket_MessageIdOnly(unsigned short messageId, SLNet::AddressOrGUID destination, bool broadcast, PacketReliability reliability, PacketPriority priority, char orderingchannel) //This is a convenient function for when we want to send a packet that contains a message ID and nothing else
{
	if(network_status == NETWORKSTATUS_OFF || network_status == NETWORKSTATUS_CLIENT_JOINING || peer == 0 || network_clientsConnectedTo == 0)
	{
		StatusUpdate("Network_SendPacket_MessageIdOnly() was called while not connected to any client.\n");
		return;
	}

	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, messageId);
	Network_SendBitStream(&bitStream, destination, broadcast, reliability, priority, orderingchannel);
}

void Network_SendPacket_MessageIdOnly_DefaultPacketProperties(unsigned short messageId, SLNet::AddressOrGUID destination, bool broadcast) //Variant of above with default packet properties
{
	if(network_status == NETWORKSTATUS_OFF || network_status == NETWORKSTATUS_CLIENT_JOINING || peer == 0 || network_clientsConnectedTo == 0)
	{
		StatusUpdate("Network_SendPacket_MessageIdOnly_DefaultPacketProperties() was called while not connected to any client.\n");
		return;
	}

	SLNet::BitStream bitStream;
	Network_Bitstream_WriteMessageId(&bitStream, messageId);
	Network_SendBitStream_DefaultPacketProperties(&bitStream, messageId, destination, broadcast);
}

void Network_SendPacket(const char *data , int packetsize, SLNet::AddressOrGUID destination, bool broadcast, PacketReliability reliability, PacketPriority priority, char orderingchannel)
{
	if(network_status == NETWORKSTATUS_OFF || network_status == NETWORKSTATUS_CLIENT_JOINING || peer == 0 || network_clientsConnectedTo == 0)
	{
		PrintToConsole("Network_SendPacket() was called while not connected to any client.\n");
		return;
	}

	/*if(!broadcast)
		PrintToConsole("Sending packet to one specific client.\n");
	else 
		PrintToConsole("Sending packet to all clients or server.\n");*/

	network_bytesSent += packetsize;
	peer->Send(data, packetsize, priority, reliability, orderingchannel, destination, broadcast);
}

void Network_SendPacket_DefaultPacketProperties(const char *data, int packetsize, unsigned short id, SLNet::AddressOrGUID destination, bool broadcast)
{
	Network_SendPacket(data, packetsize, destination, broadcast, packetTypeInfo[id].reliability, packetTypeInfo[id].priority, packetTypeInfo[id].orderingchannel);
}

void Network_SendBitStream(SLNet::BitStream *bitStream, SLNet::AddressOrGUID destination, bool broadcast, PacketReliability reliability, PacketPriority priority, char orderingchannel)
{
	if(network_status == NETWORKSTATUS_OFF || network_status == NETWORKSTATUS_CLIENT_JOINING || peer == 0 || network_clientsConnectedTo == 0)
	{
		PrintToConsole("Network_SendBitStream() was called while not connected to any client.\n");
		return;
	}

	/*if(!broadcast)
		PrintToConsole("Sending packet to one specific client.\n");
	else 
		PrintToConsole("Sending packet to all clients or server.\n");*/

	network_bytesSent += (unsigned int) bitStream->GetNumberOfBytesUsed();
	peer->Send(bitStream, priority, reliability, orderingchannel, destination, broadcast);
}

void Network_SendBitStream_DefaultPacketProperties(SLNet::BitStream *bitStream, unsigned short id, SLNet::AddressOrGUID destination, bool broadcast)
{
	Network_SendBitStream(bitStream, destination, broadcast, packetTypeInfo[id].reliability, packetTypeInfo[id].priority, packetTypeInfo[id].orderingchannel);
}

static unsigned long Network_GetTimeStampFromPacket(SLNet::Packet *p) 
{
	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
		return (unsigned long) p->data[sizeof(unsigned char)];
	return 0;
}

static unsigned char GetPacketIdentifier(SLNet::Packet *p)
{
	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	return (unsigned char) p->data[0];
}

static void UpdateGUIDAfterNewConnection(SLNet::Packet *packet, unsigned char id)
{
	//One part of RakNet documentation says that GUID is set to -1 until we establish connection.
	//Though, in practice, we always get a unique GUID after creating a RakNet peer instance.
	//Either way, just in case the GUID can change, we handle it here after getting a packet about a new connection.

	unsigned long long prevGUID = networkPlayers[0].networkId;
	networkPlayers[0].networkId = peer->GetMyGUID().g;
	if(prevGUID != networkPlayers[0].networkId)
	{
		PrintToConsole("Our GUID changed from %llu to %llu.\n", prevGUID, networkPlayers[0].networkId);
		//TODO: If we had game entities tied with our old GUID, this is where we should change it to the new GUID

		if(id == ID_NEW_INCOMING_CONNECTION) //This means that we (as host) got a new GUID after a new client connected. Which means we need to potentially tell other clients our GUID changed
		{
			Network_Send_ChangePlayerGuid(prevGUID, networkPlayers[0].networkId);
		}
	}
}

bool Network_ConnectedToAnyClient()
{
	return network_clientsConnectedTo > 0;
}

bool Network_HostingForAnyClient()
{
	return network_clientsConnectedTo > 0 && network_status == NETWORKSTATUS_SERVER_HOSTING;
}

void Network_CheckForPackets()
{
	if(peer == 0 || network_status == NETWORKSTATUS_OFF)
		return;

	if(network_scheduleTermination)
	{
		Network_Stop();
		return;
	}

	SLNet::Packet *packet = peer->Receive();
	
	while(packet)
	{
		unsigned char id = GetPacketIdentifier(packet);

		network_bytesReceived += packet->bitSize;

		switch(id)
		{
		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			network_clientsConnectedTo++;
			network_status = NETWORKSTATUS_CLIENT_CONNECTED;
			PrintToConsole("Our connection request has been accepted by the host (GUID: %llu).\n", packet->guid.g);
			UpdateGUIDAfterNewConnection(packet, id);
			Network_Send_ApplicationAndPlayerName(packet); //Send a packet with our player name, application name, and application version number
			break;
		}
		case ID_CONNECTION_ATTEMPT_FAILED:
			PrintToConsole("We failed to connect to the host.\n");
			network_scheduleTermination = 1;
			break;
		case ID_ALREADY_CONNECTED:
			PrintToConsole("We sent a connection request to a host we're already connected to.\n");
			break;
		case ID_NEW_INCOMING_CONNECTION:
		{
			network_clientsConnectedTo++;
			PrintToConsole("We've established a connection with a new client (GUID: %llu).\n", packet->guid.g);
			UpdateGUIDAfterNewConnection(packet, id);
			break;
		}
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			PrintToConsole("The server is full.\n");
			network_scheduleTermination = 1;
			break;
		case ID_DISCONNECTION_NOTIFICATION:
		case ID_CONNECTION_LOST:
			network_clientsConnectedTo--;
			if(network_status == NETWORKSTATUS_SERVER_HOSTING)
			{
				networkPlayer_s *disconnectingPlayer = Network_FindPlayerByNetworkId(packet->guid.g);
				if(disconnectingPlayer == 0) //If we didn't find the player, then this is probably someone how didn't get far enough along into the handshake process to be added to the networkPlayer array. In which case we don't need to do anything else here
					break;

				if(disconnectingPlayer->state & NETWORKPLAYERSTATE_ABOUTTOLEAVE)
					PrintToConsole("A client has manually left the session (GUID: %llu).\n", packet->guid.g);
				else if(id == ID_DISCONNECTION_NOTIFICATION)
					PrintToConsole("A client has disconnected (GUID: %llu).\n", packet->guid.g);
				else
					PrintToConsole("A client lost the connection (GUID: %llu).\n", packet->guid.g);

				//Note: To force a behaviour where we always want a player to be able to reconnect after leaving, or we want to always force a player to be fully deleted when leaving, we can choose that by altering the below if statement to always be true or false
				if(disconnectingPlayer && disconnectingPlayer->state & NETWORKPLAYERSTATE_ABOUTTOLEAVE) //If a player requested to leave (that is, they're quitting the session manually), then we fully remove them
				{
					Network_PlayerLeft(packet->guid);
					Network_RemovePlayer(packet->guid.g);
				}
				else //If they disconnected through other means, then we'll keep the player in our player array but marked as "disconnected
				{
					Network_PlayerDisconnected(packet->guid);
					Network_MarkPlayerAsDisconnected(packet->guid.g);
				}
				if(network_clientsConnectedTo)
					Network_Send_RemovePlayer(packet->guid, packet->guid.g, 1);
			}
			else 
			{
				if(id == ID_DISCONNECTION_NOTIFICATION)
					PrintToConsole("We have been disconnected from the host.\n");
				else
					PrintToConsole("We lost connection to the host.\n");
				network_scheduleTermination = 1;
			}
			break;
		case ID_CONNECTION_BANNED:
			PrintToConsole("Our connection has refused because we're banned from the server.\n");
			network_scheduleTermination = 1;
			break;
		case ID_INVALID_PASSWORD:
			PrintToConsole("Our connection was refused due to invalid password.\n");
			network_scheduleTermination = 1;
			break;
		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			PrintToConsole("Our connection was refused due incompatible protocol version.\n");
			//TODO: Show protocol version? It's the next byte in the packet
			network_scheduleTermination = 1;
			break;
		case ID_IP_RECENTLY_CONNECTED:
			PrintToConsole("Our connection was refused due to having been very recently connected.\n");
			network_scheduleTermination = 1;
			break;
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			PrintToConsole("A remote client disconnected from the server.\n");
			break;
		case ID_REMOTE_CONNECTION_LOST:
			PrintToConsole("A remote client lost connection to the server.\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			PrintToConsole("A remote client has connected to the server.\n");
			break;
		}

		if (id >= ID_USER_PACKET_ENUM) //Packet with custom ID, process it
			Network_HandleCustomPacketType(packet);

		peer->DeallocatePacket(packet);

		if(network_scheduleTermination)
		{
			Network_Stop();
			break;
		}

		//Stay in the loop as long as there are more packets.
		packet = peer->Receive();
	}	
}