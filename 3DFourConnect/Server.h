#ifndef SERVER_H
#define SERVER_H

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

#include "Tools.h"

#include "Local3DFourConnect.h"

class Server {
public:
	void Run(uint16 nPort)
	{
		//init game stuff
		game = GameManager();

		// Select instance to use.  For now we'll always use the default.
		// But we could use SteamGameServerNetworkingSockets() on Steam.
		m_pInterface = SteamNetworkingSockets();

		// Start listening
		SteamNetworkingIPAddr serverLocalAddr;
		serverLocalAddr.Clear();
		serverLocalAddr.m_port = nPort;
		SteamNetworkingConfigValue_t opt;
		opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
		m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
		if (m_hListenSock == k_HSteamListenSocket_Invalid)
			std::cout << "Failed to listen on port " << nPort << std::endl;
		m_hPollGroup = m_pInterface->CreatePollGroup();
		if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
			std::cout << "Failed to listen on port " << nPort << std::endl;
		std::cout << "Server listening on port " << nPort << std::endl;

		while (!g_bQuit)
		{
			PollIncomingMessages();
			PollConnectionStateChanges();
			PollLocalUserInput();

			game.update();

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		// Close all the connections
		std::cout << "Closing connections..." << std::endl;
		for (auto it : m_mapClients)
		{
			// Send them one more goodbye message.  Note that we also have the
			// connection close reason as a place to send final data. However,
			// that's usually best left for more diagnostic/debug text not actual
			// protocol strings.
			SendStringToClient(it.first, "Server is shutting down.  Goodbye.");

			// Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
			// to flush this out and close gracefully.
			m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
		}
		m_mapClients.clear();

		m_pInterface->CloseListenSocket(m_hListenSock);
		m_hListenSock = k_HSteamListenSocket_Invalid;

		m_pInterface->DestroyPollGroup(m_hPollGroup);
		m_hPollGroup = k_HSteamNetPollGroup_Invalid;
	}
private:

	//Game vars
	GameManager game;

	//Networking vars
	HSteamListenSocket m_hListenSock;
	HSteamNetPollGroup m_hPollGroup;
	ISteamNetworkingSockets *m_pInterface;

	struct Client_t
	{
		std::string m_sNick;
	};

	std::map< HSteamNetConnection, Client_t > m_mapClients;

	void SendStringToClient(HSteamNetConnection conn, const char *str)
	{
		//m_pInterface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
	}

	void SendDataToClient(HSteamNetConnection conn, DataPacket *data) {
		m_pInterface->SendMessageToConnection(conn, data, (uint32)sizeof(*data), k_nSteamNetworkingSend_Reliable, nullptr);
	}

	void SendStringToAllClients(const char *str, HSteamNetConnection except = k_HSteamNetConnection_Invalid)
	{
		for (auto &c : m_mapClients)
		{
			if (c.first != except)
				SendStringToClient(c.first, str);
		}
	}

	void SendDataToAllClients(DataPacket *data, HSteamNetConnection except = k_HSteamNetConnection_Invalid)
	{
		for (auto &c : m_mapClients)
		{
			if (c.first != except)
				SendDataToClient(c.first, data);
		}
	}

	void PollIncomingMessages()
	{
		char temp[1024];

		while (!g_bQuit)
		{
			ISteamNetworkingMessage *pIncomingMsg = nullptr;
			int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMsg, 1);
			if (numMsgs == 0)
				break;
			if (numMsgs < 0)
				std::cout << "Error checking for messages" << std::endl;
			assert(numMsgs == 1 && pIncomingMsg);
			auto itClient = m_mapClients.find(pIncomingMsg->m_conn);
			assert(itClient != m_mapClients.end());

			//parse data sent
			DataPacket *data = (DataPacket*)pIncomingMsg->m_pData;
			switch (data->type) {
				//connection info
				case DataPacket::MsgType::CONNECTION_STATUS: {
					std::cout << "recieved connection data" << std::endl;
				}
				//attempting to place a piece
				case DataPacket::MsgType::GAME_DATA: {
					//if the current turn is set to the player that made the move
					std::cout << "Recieved game data from client: " << std::endl;

					//send the updated board to all the users
					SendDataToAllClients(data);
				}
				default: {
					//std::cout << "Recieved data of no known type" << std::endl;
				}
			}
			//std::cout << "Recieved game data from client: " << std::endl;

			// We don't need this anymore.
			pIncomingMsg->Release();

			// Check for known commands.  None of this example code is secure or robust.
			// Don't write a real server like this, please.

			// Assume it's just a ordinary chat message, dispatch to everybody else
			//sprintf_s(temp, "%s: %s", itClient->second.m_sNick.c_str(), cmd);
			//SendStringToAllClients(temp, itClient->first);
		}
	}

	void PollLocalUserInput()
	{
		std::string cmd;
		while (!g_bQuit && LocalUserInput_GetNext(cmd))
		{
			if (strcmp(cmd.c_str(), "/quit") == 0)
			{
				g_bQuit = true;
				std::cout << "Shutting down server" << std::endl;
				break;
			}
			if (strcmp(cmd.c_str(), "/test") == 0)
			{
				DataPacket data;
				data.msg = "hello";
				std::cout << "Sent Test Messages" << std::endl;

				for (auto &c : m_mapClients)
				{
					SendDataToClient(c.first, &data);
				}
				break;
			}

			// That's the only command we support
			std::cout << "The server only knows one command: '/quit'" << std::endl;
		}
	}

	void SetClientNick(HSteamNetConnection hConn, const char *nick)
	{

		// Remember their nick
		m_mapClients[hConn].m_sNick = nick;

		// Set the connection name, too, which is useful for debugging
		m_pInterface->SetConnectionName(hConn, nick);
	}

	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
	{
		char temp[1024];

		// What's the state of the connection?
		switch (pInfo->m_info.m_eState)
		{
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			// Ignore if they were not previously connected.  (If they disconnected
			// before we accepted the connection.)
			if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
			{

				// Locate the client.  Note that it should have been found, because this
				// is the only codepath where we remove clients (except on shutdown),
				// and connection change callbacks are dispatched in queue order.
				auto itClient = m_mapClients.find(pInfo->m_hConn);
				assert(itClient != m_mapClients.end());

				// Select appropriate log messages
				const char *pszDebugLogAction;
				if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				{
					pszDebugLogAction = "problem detected locally";
					sprintf_s(temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.m_sNick.c_str(), pInfo->m_info.m_szEndDebug);
				}
				else
				{
					// Note that here we could check the reason code to see if
					// it was a "usual" connection or an "unusual" one.
					pszDebugLogAction = "closed by peer";
					sprintf_s(temp, "%s hath departed", itClient->second.m_sNick.c_str());
				}

				// Spew something to our own log.  Note that because we put their nick
				// as the connection description, it will show up, along with their
				// transport-specific data (e.g. their IP address)
				std::cout << "Connection " << pInfo->m_info.m_szConnectionDescription << pszDebugLogAction << ", reason " << pInfo->m_info.m_eEndReason << ": " << pInfo->m_info.m_szEndDebug << std::endl;

				m_mapClients.erase(itClient);

				// Send a message so everybody else knows what happened
				SendStringToAllClients(temp);
			}
			else
			{
				assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
		{
			// This must be a new connection
			assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

			std::cout << "Connection request from " << pInfo->m_info.m_szConnectionDescription << std::endl;

			// A client is attempting to connect
			// Try to accept the connection.
			if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
			{
				// This could fail.  If the remote host tried to connect, but then
				// disconnected, the connection may already be half closed.  Just
				// destroy whatever we have on our side.
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				std::cout << "Can't accept connection.  (It was already closed?)" << std::endl;
				break;
			}

			// Assign the poll group
			if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup))
			{
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				std::cout << "Failed to set poll group?" << std::endl;
				break;
			}

			// give a name based on the number of players connected to the server
			std::string nick = "Player " + std::to_string(m_mapClients.size());

			// Also send them a list of everybody who is already connected
			if (m_mapClients.empty())
			{
				SendStringToClient(pInfo->m_hConn, "Thou art utterly alone.");
			}
			else
			{
				sprintf_s(temp, "%d companions greet you:", (int)m_mapClients.size());
				for (auto &c : m_mapClients)
					SendStringToClient(pInfo->m_hConn, c.second.m_sNick.c_str());
			}

			// Let everybody else know who they are for now
			DataPacket data;
			data.type = DataPacket::MsgType::CONNECTION_STATUS;
			data.msg = (nick + " has joined the match");
			SendDataToAllClients(&data, pInfo->m_hConn);

			// Add them to the client list, using std::map wacky syntax
			m_mapClients[pInfo->m_hConn];
			SetClientNick(pInfo->m_hConn, nick.c_str());
			break;
		}

		case k_ESteamNetworkingConnectionState_Connected:
			// We will get a callback immediately after accepting the connection.
			// Since we are the server, we can ignore this, it's not news to us.
			break;

		default:
			// Silences -Wswitch
			break;
		}
	}

	static Server *s_pCallbackInstance;

	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo)
	{
		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}

	void PollConnectionStateChanges()
	{
		s_pCallbackInstance = this;
		m_pInterface->RunCallbacks();
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
					if (game.board.data[x][y][z].type == Piece::Color::NONE) {
						data.board[x][y][z] = 0;
					}
					if (game.board.data[x][y][z].type == Piece::Color::RED) {
						data.board[x][y][z] = 1;
					}
					if (game.board.data[x][y][z].type == Piece::Color::BLUE) {
						data.board[x][y][z] = 2;
					}
				}
			}
		}

		return data;
	}
};

#endif
