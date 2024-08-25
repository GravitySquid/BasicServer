/**
 * @file	Server.cpp
 * @brief	A basic socket server.
 *
 * Basic socket server for Windows, using winsock2.
 * Will take connections from multiple clients.
 *
 * @author	GravitySquid
 * @date	2024/08/25
 */

#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

#define DATA_BUFSIZE 1024

int main(int argc, char* argv[]) {
	std::cout << "Server started...." << std::endl;

	// Check optional command line arguments
	const char* ip, * port;
	if (argc == 3) // Expect IP & port number
	{
		ip = argv[1];
		port = argv[2];
	}
	else // default an IP & port number
	{
		std::cout << "Default parameters used ... " << std::endl;
		ip = "127.0.0.1"; // localhost (IPv4 loopback address)
		port = "27016";
	}
	std::cout << "Server IP ..... " << ip << std::endl;
	std::cout << "Server Port ... " << port << std::endl;

	// Convert port number to short
	unsigned short  usPort = (unsigned short)std::stoi(port);

	// Start WinSock API
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WinSock API Startup failed." << std::endl;
		return 1;
	}

	// Create a socket for server
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Error creating socket." << std::endl;
		WSACleanup();
		return 1;
	}

	// Populate socket address struct
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	if (inet_pton(AF_INET, ip, &(serverAddress.sin_addr)) != 1) {
		std::cerr << "Error setting server address." << std::endl;
		return 1;
	}
	serverAddress.sin_port = htons(usPort);

	// Bind the socket to address
	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		std::cerr << "Bind failed." << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// Listen for any incoming connections
	listen(serverSocket, SOMAXCONN);

	std::string tmp = ":";
	std::string address = ip + tmp + port;
	std::cout << "Server listening on " + address + " ..." << std::endl;

	// Set the listening socket to non-blocking mode
	//ULONG NonBlock = 1;
	//if (ioctlsocket(serverSocket, FIONBIO, &NonBlock) == SOCKET_ERROR) {
	//	printf("cCould not make server socked non-blocking - %d\n", WSAGetLastError());
	//	//return 1;
	//}

	// initialise the set of client sockets
	fd_set clientSocketSet;
	FD_ZERO(&clientSocketSet);

	char buffer[DATA_BUFSIZE];
	int bytesRead;

	fd_set read_set;
	FD_ZERO(&read_set);

	// Main server loop - keep active
	while (true)
	{
		// List of sockets to check = client sockets + server socket for new clients
		fd_set read_set = clientSocketSet;
		FD_SET(serverSocket, &read_set);

		// determine which sockets are ready for read
		select(0, &read_set, NULL, NULL, NULL);

		// Check if server socket has a new client to accept
		if (FD_ISSET(serverSocket, &read_set))
		{
			SOCKADDR_IN ClientAddr;
			int ClientAddrLen = sizeof(ClientAddr);

			SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&ClientAddr, &ClientAddrLen);
			FD_SET(clientSocket, &clientSocketSet); // add to client socket set

			// show client IP address
			char ipStr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(ClientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);

			// Convert the port number to a string
			char portString[6]; 
			_itoa_s(ntohs(ClientAddr.sin_port), portString, 10);

			// Display
			std::cout << "New client connected from " << ipStr << ":" << portString << std::endl;
			std::cout << "Current connections: " << clientSocketSet.fd_count << std::endl;
		}

		for (int i = 0; i < (int) read_set.fd_count; ++i)
		{
			// Check if a client is ready to read
			if (FD_ISSET(read_set.fd_array[i], &clientSocketSet))
			{
				bytesRead = recv(read_set.fd_array[i], buffer, sizeof(buffer), 0);
				if (bytesRead > 0)
				{
					std::string sBuff(buffer);
					std::string msgRecv = sBuff.substr(0,bytesRead);
					std::cout << "Data buffer received from client >>> " << msgRecv << std::endl;
					// Send data to the client
					std::string msgSend = "Got your message: " + msgRecv;
					const char *message = msgSend.c_str();
					std::cout << "send >> " << message << std::endl;
					send(read_set.fd_array[i], message, (int) strlen(message), 0);
				}
				else
				{
					// Client has ended connection
					// Find out the client connection details for display
					SOCKET clientSocket = read_set.fd_array[i];
					SOCKADDR_IN peerAddress;
					int peerAddressSize = sizeof(peerAddress);
					char ipStr[INET_ADDRSTRLEN];
					if (getpeername(clientSocket, reinterpret_cast<SOCKADDR*>(&peerAddress), &peerAddressSize) == 0) {
						inet_ntop(AF_INET, &(peerAddress.sin_addr), ipStr, INET_ADDRSTRLEN);
						std::cout << "Client ended connection from " << ipStr
							<< ":" << ntohs(peerAddress.sin_port) << std::endl;
					}
					else
						std::cout << "Client ended connection" << std::endl;
					
					// Clean up
					closesocket(clientSocket);

					// Remove client from the set of client sockets
					FD_CLR(clientSocket, &clientSocketSet);
					std::cout << "Current connections: " << clientSocketSet.fd_count << std::endl;
				}
			}
		}
	}

	// Clean up
	for (int i = 0; i < (int) clientSocketSet.fd_count; ++i)
	{
		closesocket(clientSocketSet.fd_array[i]);
	}
	closesocket(serverSocket);
	WSACleanup();

	std::cout << "Server shutdown." << std::endl;
	return 0;
}
