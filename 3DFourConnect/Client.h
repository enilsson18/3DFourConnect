#ifndef CLIENT_H
#define CLIENT_H

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <cctype>

#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <GameNetworkingSockets/steam/steam_api.h>
#endif

#include <iostream>
#include <signal.h>

#include "Local3DFourConnect.h"

#include "Tools.h"

//prototypes
void placePieceCallback(Piece::Color color, glm::vec3 pos);

void* clientPtr;

class Client
{
public:
	void Run(const SteamNetworkingIPAddr &serverAddr)
	{
		//setup local game stuff
		clientPtr = this;

		game.gameManager.setPiecePlaceCallback(placePieceCallback);

		//disable fps counter
		game.enableFPSCounter = false;

		// Select instance to use.  For now we'll always use the default.
		m_pInterface = SteamNetworkingSockets();

		// Start connecting
		char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
		serverAddr.ToString(szAddr, sizeof(szAddr), true);
		Printf("Connecting to chat server at %s", szAddr);
		SteamNetworkingConfigValue_t opt;
		opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
		m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
		if (m_hConnection == k_HSteamNetConnection_Invalid)
			FatalError("Failed to create connection");

		while (!g_bQuit && game.run() == 1)
		{
			PollIncomingMessages();
			PollConnectionStateChanges();
			PollLocalUserInput();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void sendDataToServer(DataPacket *data) {
		m_pInterface->SendMessageToConnection(m_hConnection, data, (uint32)sizeof(*data), k_nSteamNetworkingSend_Reliable, nullptr);
	}

	//game stuff
	//convert the pieces on the board to data 1's and 2's to represent red and blue respectivley.
	//returns a datapacket with the int array converted to numbers
	DataPacket convertBoardToPacket() {
		DataPacket data;
		data.type = DataPacket::MsgType::GAME_DATA;

		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				for (int z = 0; z < 4; z++) {
					if (game.gameManager.board.data[x][y][z].type == Piece::Color::NONE) {
						data.board[x][y][z] = 0;
					}
					if (game.gameManager.board.data[x][y][z].type == Piece::Color::RED) {
						data.board[x][y][z] = 1;
					}
					if (game.gameManager.board.data[x][y][z].type == Piece::Color::BLUE) {
						data.board[x][y][z] = 2;
					}
				}
			}
		}

		return data;
	}

private:

	string filepath;

	Local3DFourConnect game;

	HSteamNetConnection m_hConnection;
	ISteamNetworkingSockets *m_pInterface;

	void PollIncomingMessages()
	{
		while (!g_bQuit)
		{
			ISteamNetworkingMessage *pIncomingMsg = nullptr;
			int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
			if (numMsgs == 0)
				break;
			if (numMsgs < 0)
				FatalError("Error checking for messages");

			// Just echo anything we get from the server
			//we trust anything coming from the server so just set the current board to whatever this is
			DataPacket *data = (DataPacket*)pIncomingMsg->m_pData;
			switch (data->type) {
				//connection info
				case DataPacket::MsgType::CONNECTION_STATUS: {
					std::cout << "recieved connection data" << std::endl;
				}
														 //attempting to place a piece
				case DataPacket::MsgType::GAME_DATA: {
					std::cout << "Recieved game data from server: " << std::endl;

					game.gameManager.board.setBoardToData(data);
				}
				default: {
					std::cout << "Recieved data of no known type" << std::endl;
				}
			}

			// We don't need this anymore.
			pIncomingMsg->Release();
		}
	}

	void PollLocalUserInput()
	{
		std::string cmd;
		while (!g_bQuit && LocalUserInput_GetNext(cmd))
		{

			// Check for known commands
			if (strcmp(cmd.c_str(), "/quit") == 0)
			{
				g_bQuit = true;
				Printf("Disconnecting from chat server");

				// Close the connection gracefully.
				// We use linger mode to ask for any remaining reliable data
				// to be flushed out.  But remember this is an application
				// protocol on UDP.  See ShutdownSteamDatagramConnectionSockets
				m_pInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);
				break;
			}

			//reset the game board
			if (strcmp(cmd.c_str(), "/clear") == 0) {
				DataPacket data;
				data.type = DataPacket::MsgType::GAME_DATA;

				for (int x = 0; x < 4; x++) {
					for (int y = 0; y < 4; y++) {
						for (int z = 0; z < 4; z++) {
							data.board[x][y][z] = 0;
						}
					}
				}

				sendDataToServer(&data);
			}

			// Anything else, just send it to the server and let them parse it
			//m_pInterface->SendMessageToConnection(m_hConnection, cmd.c_str(), (uint32)cmd.length(), k_nSteamNetworkingSend_Reliable, nullptr);
		}
	}

	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
	{
		assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

		// What's the state of the connection?
		switch (pInfo->m_info.m_eState)
		{
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			g_bQuit = true;

			// Print an appropriate message
			if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
			{
				// Note: we could distinguish between a timeout, a rejected connection,
				// or some other transport problem.
				Printf("We sought the remote host, yet our efforts were met with defeat.  (%s)", pInfo->m_info.m_szEndDebug);
			}
			else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
			{
				Printf("Alas, troubles beset us; we have lost contact with the host.  (%s)", pInfo->m_info.m_szEndDebug);
			}
			else
			{
				// NOTE: We could check the reason code for a normal disconnection
				Printf("The host hath bidden us farewell.  (%s)", pInfo->m_info.m_szEndDebug);
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			m_hConnection = k_HSteamNetConnection_Invalid;
			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
			// We will get this callback when we start connecting.
			// We can ignore this.
			break;

		case k_ESteamNetworkingConnectionState_Connected:
			Printf("Connected to server OK");
			break;

		default:
			// Silences -Wswitch
			break;
		}
	}

	static Client *s_pCallbackInstance;
	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo)
	{
		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}

	void PollConnectionStateChanges()
	{
		s_pCallbackInstance = this;
		m_pInterface->RunCallbacks();
	}
};

void placePieceCallback(Piece::Color color, glm::vec3 pos) {
	Client *client = (Client*)clientPtr;
	//if it is the clients turn

	//get the base packet
	DataPacket data = client->convertBoardToPacket();
	data.type = DataPacket::MsgType::GAME_DATA;

	//send to server
	client->sendDataToServer(&data);

	//std::cout << "sent message to server of type" << std::endl;
}

#endif