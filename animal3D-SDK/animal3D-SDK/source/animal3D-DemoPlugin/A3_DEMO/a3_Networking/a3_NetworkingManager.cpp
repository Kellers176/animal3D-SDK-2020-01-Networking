/*
	Copyright 2011-2020 Daniel S. Buckstein

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	animal3D SDK: Minimal 3D Animation Framework
	By Daniel S. Buckstein
	
	a3_NetworkingManager.cpp
	RakNet networking wrapper.
*/


#include "../a3_NetworkingManager.h"

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/BitStream.h"
#include "RakNet/GetTime.h"

#include "a3_Networking_EventSystem.h"
#include "a3_Networking_ObjectInfo.h"
#include "MoveInputListener.h"

#include "../a3_DemoState.h"



//-----------------------------------------------------------------------------
// networking stuff

enum a3_NetGameMessages
{
	ID_CUSTOM_MESSAGE_START = ID_USER_PACKET_ENUM,

	ID_GAME_MESSAGE_1,
	ID_ADD_INPUT_TO_GAME_OBJECT,
	ID_UPDATE_OBJECT_FOR_USER
};


#pragma pack(push, 1)

struct a3_NetGameMessageData
{
	unsigned char typeID;

	// ****TO-DO: implement game message data struct
	a3_KeyboardKey temp;

};

#pragma pack (pop)


// startup networking
a3i32 a3netStartup(a3_NetworkingManager* net, a3ui16 port_inbound, a3ui16 port_outbound, a3ui16 maxConnect_inbound, a3ui16 maxConnect_outbound)
{
	if (net && !net->peer)
	{
		RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
		if (peer)
		{
			RakNet::SocketDescriptor socket[2] = {
				RakNet::SocketDescriptor(port_inbound, 0),		// server settings
				RakNet::SocketDescriptor(),//port_outbound, 0),	// client settings
			};

			// choose startup settings based on 
			a3boolean const isServer = port_inbound && maxConnect_inbound;
			a3boolean const isClient = port_outbound && maxConnect_outbound;
			if (peer->Startup(maxConnect_inbound + maxConnect_outbound, socket + isClient, isServer + isClient) == RakNet::StartupResult::RAKNET_STARTED)
			{
				peer->SetMaximumIncomingConnections(maxConnect_inbound);
				peer->SetOccasionalPing(true);

				net->port_inbound = port_inbound;
				net->port_outbound = port_outbound;
				net->maxConnect_inbound = maxConnect_inbound;
				net->maxConnect_outbound = maxConnect_outbound;
				net->peer = peer;

				MoveInputListener* eventUpInput = new MoveInputListener(a3key_upArrow);
				MoveInputListener* eventDownInput = new MoveInputListener(a3key_downArrow);
				MoveInputListener* eventLeftInput = new MoveInputListener(a3key_leftArrow);
				MoveInputListener* eventRightInput = new MoveInputListener(a3key_rightArrow);

				eventUpInput->SetListenerKey(a3key_upArrow);
				eventDownInput->SetListenerKey(a3key_downArrow);
				eventLeftInput->SetListenerKey(a3key_leftArrow);
				eventRightInput->SetListenerKey(a3key_rightArrow);

				a3_Networking_EventSystem::Instance()->addEvent("MoveObjectUp", eventUpInput);
				a3_Networking_EventSystem::Instance()->addEvent("MoveObjectDown", eventDownInput);
				a3_Networking_EventSystem::Instance()->addEvent("MoveObjectLeft", eventLeftInput);
				a3_Networking_EventSystem::Instance()->addEvent("MoveObjectRight", eventRightInput);

				return 1;
			}
		}
	}
	return 0;
}

// shutdown networking
a3i32 a3netShutdown(a3_NetworkingManager* net)
{
	if (net && net->peer)
	{
		RakNet::RakPeerInterface* peer = (RakNet::RakPeerInterface*)net->peer;
		RakNet::RakPeerInterface::DestroyInstance(peer);
		net->peer = 0;
		return 1;
	}
	return 0;
}


// connect
a3i32 a3netConnect(a3_NetworkingManager* net, a3netAddressStr const ip)
{
	if (net && net->peer)
	{
		RakNet::RakPeerInterface* peer = (RakNet::RakPeerInterface*)net->peer;
		peer->Connect(ip, net->port_outbound, 0, 0);
		return 1;
	}
	return 0;
}

// disconnect
a3i32 a3netDisconnect(a3_NetworkingManager* net)
{
	if (net && net->peer)
	{
		RakNet::RakPeerInterface* peer = (RakNet::RakPeerInterface*)net->peer;
		a3ui16 i, j = peer->NumberOfConnections();
		for (i = 0; i < j; ++i)
			peer->CloseConnection(peer->GetSystemAddressFromIndex(i), true);
		return 1;
	}
	return 0;
}


// process inbound packets
a3i32 a3netProcessInbound(a3_NetworkingManager* net)
{
	if (net && net->peer)
	{
		RakNet::RakPeerInterface* peer = (RakNet::RakPeerInterface*)net->peer;
		RakNet::Packet* packet;
		RakNet::MessageID msg;
		a3i32 count = 0;

		MoveInputData* tempInputData;
		ObjectPosInfo* tempMoveObjInfo;

		for (packet = peer->Receive();
			packet;
			peer->DeallocatePacket(packet), packet = peer->Receive(), ++count)
		{
			RakNet::BitStream bs_in(packet->data, packet->length, false);
			bs_in.Read(msg);
			switch (msg)
			{
				// check for timestamp and process
			case ID_TIMESTAMP:
				bs_in.Read(msg);
				// ****TO-DO: handle timestamp
				RakNet::Time sendTime, recieveTime, dt;
				bs_in.Read(sendTime);
				recieveTime = RakNet::GetTime();
				dt = recieveTime - sendTime;
				printf("\n SEND TIME: %u \n", (unsigned int)sendTime);
				printf("\n RECIEVE TIME: %u \n", (unsigned int)recieveTime);
				printf("\n DIFF TIME: %u \n", (unsigned int)dt);

				// do not break; proceed to default case to process actual message contents
			default:
				switch (msg)
				{
				case ID_REMOTE_DISCONNECTION_NOTIFICATION:
					printf("Another client has disconnected.\n");
					break;
				case ID_REMOTE_CONNECTION_LOST:
					printf("Another client has lost the connection.\n");
					break;
				case ID_REMOTE_NEW_INCOMING_CONNECTION:
					printf("Another client has connected.\n");
					break;
				case ID_CONNECTION_REQUEST_ACCEPTED:
					printf("Our connection request has been accepted.\n");
					{
						// Use a BitStream to write a custom user message
						// Bitstreams are easier to use than sending casted structures, 
						//	and handle endian swapping automatically
						RakNet::BitStream bsOut[1];
						RakNet::Time sendTime;
						//indicate timestamp intent
						bsOut->Write((RakNet::MessageID)ID_TIMESTAMP);
						//get time
						sendTime = RakNet::GetTime();
						bsOut->Write(RakNet::GetTime());
						//rest of message
						bsOut->Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
						bsOut->Write("Hello world");
						peer->Send(bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
						// ****TO-DO: write timestamped message
						printf("\n SEND TIME: %u \n", (unsigned int)sendTime);



					}
					break;
				case ID_NEW_INCOMING_CONNECTION:
					printf("A connection is incoming.\n");
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					printf("The server is full.\n");
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					if (net->maxConnect_outbound) {
						printf("A client has disconnected.\n");
					}
					else {
						printf("We have been disconnected.\n");
					}
					break;
				case ID_CONNECTION_LOST:
					if (net->maxConnect_outbound) {
						printf("A client lost the connection.\n");
					}
					else {
						printf("Connection lost.\n");
					}
					break;

				case ID_GAME_MESSAGE_1:
					printf("DEBUG MESSAGE: received packet ID_GAME_MESSAGE_1.\n");
					{
						RakNet::RakString rs;
						bs_in.Read(rs);
						printf("%s\n", rs.C_String());

						// get the input


					}
					break;
				case ID_ADD_INPUT_TO_GAME_OBJECT:
					
					// server gets the input
					tempInputData = (MoveInputData*)packet->data;

					// send the input to the event system
					if (tempInputData->input == a3key_upArrow)
						a3_Networking_EventSystem::Instance()->sendEvent("MoveObjectUp");
					if (tempInputData->input == a3key_downArrow)
						a3_Networking_EventSystem::Instance()->sendEvent("MoveObjectDown");
					if (tempInputData->input == a3key_leftArrow)
						a3_Networking_EventSystem::Instance()->sendEvent("MoveObjectLeft");
					if (tempInputData->input == a3key_rightArrow)
						a3_Networking_EventSystem::Instance()->sendEvent("MoveObjectRight");

					// event system will process it and return the new pos info

					
					break;
				case ID_UPDATE_OBJECT_FOR_USER:

					// get the infor from the server for the new move info
					tempMoveObjInfo = (ObjectPosInfo*)packet->data;

					// set the object pos to pos + move input
					
					if (tempMoveObjInfo->objType == sphereSelected)
					{
						
					}
					else if (tempMoveObjInfo->objType == cylinderSelected)
					{

					}
					else if (tempMoveObjInfo->objType == torusSelected)
					{

					}
					else if (tempMoveObjInfo->objType == teapotSelected)
					{

					}

					break;
				default:
					printf("Message with identifier %i has arrived.\n", msg);
					break;
				}
				break;
			}
		}
		return count;
	}
	return 0;
}

// process outbound packets
a3i32 a3netProcessOutbound(a3_NetworkingManager* net)
{
	if (net && net->peer)
	{
		if (net->isServer)
		{
			RakNet::RakPeerInterface* peer = (RakNet::RakPeerInterface*)net->peer;

			RakNet::BitStream bsOut[1];

			/*
				unsigned char typeId;
				int objType;
				float xPos;
				float yPos;
				float zPos;
			*/

			ObjectPosInfo posData = ObjectPosInfo();
			posData.typeId = ID_UPDATE_OBJECT_FOR_USER;
			posData.objType = net->selectedSharedObject;
			posData.xPos = posData.xPos + net->moveXData;
			posData.yPos = posData.yPos + net->moveYData;
			posData.zPos = posData.zPos + net->moveZData;

			bsOut->Write((ObjectPosInfo)posData);

			for (unsigned int i = 0; i < peer->GetNumberOfAddresses(); i++)
			{
				peer->Send(bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, peer->GetSystemAddressFromIndex(i), false);
			}

			net->moveXData = 0;
			net->moveYData = 0;
			net->moveZData = 0;
		}
		else
		{
			RakNet::BitStream bsOut[1];
			
			MoveInputData moveInput = MoveInputData();
			moveInput.typeId = ID_ADD_INPUT_TO_GAME_OBJECT;
			moveInput.objType = net->selectedSharedObject;
			moveInput.input = net->inputData;

			bsOut->Write((MoveInputData)moveInput);

			net->inputData = a3key_question;
		}
	}
	return 0;
}


//-----------------------------------------------------------------------------
