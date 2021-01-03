//Requires GameNetworkingSockets to compile. See the build link: https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md

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
#include "Server.h"
#include "Client.h"

Server *Server::s_pCallbackInstance = nullptr;

Client *Client::s_pCallbackInstance = nullptr;

const uint16 DEFAULT_SERVER_PORT = 25565;

void PrintUsageAndExit(int rc = 1)
{
	fflush(stderr);
	printf(
		R"usage(The information entered was invalid.
	Cmd argument usage:
    3DFourConnect.exe client SERVER_ADDR
    3DFourConnect.exe server [--port PORT]
)usage"
);
	fflush(stdout);
	exit(rc);
}

//start up options
int main(int argc, const char *argv[])
{
	bool bServer = false;
	bool bClient = false;
	bool bLocal = false;
	int nPort = DEFAULT_SERVER_PORT;
	SteamNetworkingIPAddr addrServer; addrServer.Clear();

	for (int i = 1; i < argc; ++i)
	{
		if (!bClient && !bServer)
		{
			if (!strcmp(argv[i], "client"))
			{
				bClient = true;
				continue;
			}
			if (!strcmp(argv[i], "server"))
			{
				bServer = true;
				continue;
			}
		}
		if (!strcmp(argv[i], "--port"))
		{
			++i;
			if (i >= argc)
				PrintUsageAndExit();
			nPort = atoi(argv[i]);
			if (nPort <= 0 || nPort > 65535)
				FatalError("Invalid port %d", nPort);
			continue;
		}

		// Anything else, must be server address to connect to
		if (bClient && addrServer.IsIPv6AllZeros())
		{
			if (!addrServer.ParseString(argv[i]))
				FatalError("Invalid server address '%s'", argv[i]);
			if (addrServer.m_port == 0)
				addrServer.m_port = DEFAULT_SERVER_PORT;
			continue;
		}

		PrintUsageAndExit();
	}

	//cin arguements if there is nothing in the main method arguements on launch
	if (argc == 1) {
		std::string type = "";
		std::string additionalInfo = "";

		std::cout << "No startup arguments were found. Please enter \"server\" (Server Host), \"client\" (Connect To Server), or \"local\" (Play On Same Computer)." << std::endl;
		while (type != "client" && type != "server" && type != "local") {
			std::cin >> type;

			if (type != "client" && type != "server" && type != "local") {
				std::cout << "Invalid arguements were given. Please say either \"server\" or \"client\"." << std::endl;
			}
		}

		if (type == "client") {
			bClient = true;

			std::cout << "Please enter the ip address of the server you want to connect to \"IP Address:Port\" (Port is optional)" << std::endl;
			std::cin >> additionalInfo;

			if (!addrServer.ParseString(additionalInfo.c_str()))
				FatalError("Invalid server address '%s'", additionalInfo.c_str());
			if (addrServer.m_port == 0)
				addrServer.m_port = DEFAULT_SERVER_PORT;
		}

		if (type == "server") {
			bServer = true;

			std::cout << "Please enter the port you wish to open the server on.\nType \"0\" and Port: " << nPort << " will be defaulted to.\nMake sure the port is open on your network aswell. (Port Forward)" << std::endl;
			std::cin >> additionalInfo;

			if (std::stoi(additionalInfo) != 0) {
				nPort = std::stoi(additionalInfo);
			}
		}

		if (type == "local") {
			bLocal = true;
		}
	}

	if ((bClient == bServer || (bClient && addrServer.IsIPv6AllZeros())) && bLocal == false)
		PrintUsageAndExit();

	// Create client and server sockets
	InitSteamDatagramConnectionSockets();
	LocalUserInput_Init();

	if (bLocal) {
		Local3DFourConnect game;
		game.run();
	}
	else if (bClient)
	{
		Client client;
		client.Run(addrServer);
	}
	else
	{
		Server server;
		server.Run((uint16)nPort);
	}

	ShutdownSteamDatagramConnectionSockets();

	// Ug, why is there no simple solution for portable, non-blocking console user input?
	// Just nuke the process
	//LocalUserInput_Kill();
	//NukeProcess(0);
}