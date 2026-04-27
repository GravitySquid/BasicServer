/**
 * @file	Server.cpp
 * @brief	A basic socket server.
 *
 * Basic socket server for Windows and Linux, using platform-specific socket APIs.
 *
 * Execution example:
 * 1. Start the server (Server.exe) in a terminal - "server.exe <IP> <Port>"
 * 2. Start the client (Client.exe) in another terminal - "client.exe <IP> <Port>"
 * 
 * @author	GravitySquid
 * @date	2024/08/25
 */

#include "Server.h"
#include <sys/select.h>
#include <unistd.h>

Server::Server(std::string inIp, std::string inPort, int buffSize) : buffer(buffSize){

   	#ifdef __linux__
		int portno;
		active = false;
		ip = inIp;
		port = inPort;

		serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (serv_sockfd < 0) {
		    statusMessage = "ERROR opening socket";
		    std::cerr << statusMessage << std::endl;
			exit(1);
		}
        std::memset(&serv_addr, 0, sizeof(serv_addr));
        portno = std::stoi(port);
        // Set the address family / structure to IPv4 address & port
        serv_addr.sin_family = AF_INET;
        // Tell the server to accept connections on any available network interface (i.e. all IP addresses on the host)
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        // Converts port number from host byte order to network byte order (big-endian) - to correct format for network protocol standard
        serv_addr.sin_port = htons(portno);
        if (bind(serv_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            statusMessage = "ERROR on socket binding";
            std::cerr << statusMessage << std::endl;
			exit(1);
        }

        active = true;
	#endif

	#ifdef _WIN32
	    active = false;
		ip = inIp;
		port = inPort;

		// Convert port number to short
    	unsigned short  usPort = (unsigned short)std::stoi(port);
    	// Start WinSock API
    	WSADATA wsaData;
    	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            statusMessage = "WinSock API Startup failed.";
    		std::cerr << statusMessage << std::endl;
    		exit(1);
    	}

    	// Create a socket for server
    	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    	if (serverSocket == INVALID_SOCKET) {
            statusMessage = "Error creating socket.";
    		std::cerr << statusMessage << std::endl;
    		WSACleanup();
    		exit(1);
    	}
    	// Populate socket address struct
    	serv_addr.sin_family = AF_INET;
    	if (inet_pton(AF_INET, ip, &(serv_addr.sin_addr)) != 1) {
            statusMessage = "Error setting server address.";
    		std::cerr << statusMessage << std::endl;
    		return 1;
    	}
    	serv_addr.sin_port = htons(usPort);

    	// Bind the socket to address
    	if (bind(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
    	{
            statusMessage = "Socket Bind failed.";
    		std::cerr << statusMessage << std::endl;
    		closesocket(serverSocket);
    		WSACleanup();
    		exit(1);
    	}
     // initialise the set of client sockets
     FD_ZERO(&clientSocketSet);
     // intialise a temp set for sockets ready to read
     FD_ZERO(&read_set);
     active = true;
	#endif

	return;
}

Server::Server(std::string ip, std::string port) : Server(ip, port, DATA_BUFSIZE) {
}

void Server::Listen() {
    #ifdef _WIN32
        listen(this->serverSocket, SOMAXCONN);
    #endif
    #ifdef __linux__
        listen(this->serv_sockfd,SOMAXCONN);
    #endif
    std::string tmp = ":";
    std::string address = this->ip + tmp + this->port;
    std::cout << "Server listening on " + address + " ..." << std::endl;

	return;
}

int Server::CheckClientActivity(){

	int activity = 0;
    #ifdef _WIN32
  		// List of sockets to check = client sockets + server socket for new clients
		read_set = clientSocketSet;
		FD_SET(serverSocket, &read_set);

		// determine which sockets are ready for read
		select(0, &read_set, NULL, NULL, NULL);

		// Check if the server socket has a new client to accept
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
			activity = clientSocketSet.fd_count;
		}

		// Check if any client sockets have data to read
		for (int i = 0; i < (int) clientSocketSet.fd_count; ++i) {
			if (FD_ISSET(clientSocketSet.fd_array[i], &read_set)) {
				int bytesRead = recv(clientSocketSet.fd_array[i], buffer.data(), buffer.size(), 0);
				
				if (bytesRead > 0) {
					// Display received data and get response
					std::string msgSend = ProcessClientData(bytesRead);
					const char *message = msgSend.c_str();
					std::cout << "send >> " << message << std::endl;
					send(clientSocketSet.fd_array[i], message, (int) strlen(message), 0);
				}
				else if (bytesRead == 0) {
					// Client disconnected
					std::cout << "Client disconnected" << std::endl;
					closesocket(clientSocketSet.fd_array[i]);
					FD_CLR(clientSocketSet.fd_array[i], &clientSocketSet);
				}
			}
		}
    #endif

    #ifdef __linux__
  		// Set up file descriptor set for select
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(serv_sockfd, &read_set);
        
        // Add all client sockets to the set and compute the highest file descriptor
        int maxfd = serv_sockfd;
        for (int client : clients) {
            FD_SET(client, &read_set);
            if (client > maxfd) {
                maxfd = client;
            }
        }

		// determine which sockets are ready for read
        select(maxfd + 1, &read_set, NULL, NULL, NULL);

		// Check if server socket has new connection
		if (FD_ISSET(serv_sockfd, &read_set)) {
			socklen_t clilen = sizeof(cli_addr);
			int newsockfd = accept(serv_sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0) {
				std::cerr << "ERROR on accept" << std::endl;
				return 0;
			}
			clients.push_back(newsockfd);
			
			// Display
			std::cout << "New client connected" << std::endl;
			std::cout << "Current connections: " << clients.size() << std::endl;
		}

		// Check if any client sockets have data to read
		for (int i = 0; i < (int) clients.size(); ++i) {
			if (FD_ISSET(clients[i], &read_set)) {
				int bytesRead = recv(clients[i], buffer.data(), buffer.size(), 0);
				
				if (bytesRead > 0) {
					// Display received data
					std::string sBuff(buffer.data());
					std::string msgRecv = sBuff.substr(0, bytesRead);
					std::cout << "Data buffer received from client >>> " << msgRecv << std::endl;
					
					// Prompt for response
					std::string msgSend;
					std::cout << "\033[32m" << "ENTER RESPONSE: > " << "\033[0m";
					std::getline(std::cin, msgSend);
					
					// Send response
					const char *message = msgSend.c_str();
					std::cout << "send >> " << message << std::endl;
					send(clients[i], message, (int) strlen(message), 0);
				}
				else if (bytesRead == 0) {
					// Client disconnected
					std::cout << "Client disconnected" << std::endl;
					close(clients[i]);
					clients.erase(clients.begin() + i);
					--i;
				}
			}
		}
    #endif

	return activity;
}

std::string Server::ProcessClientData(int bytesRead) {
	if (bytesRead > 0) {
		// Display received data
		std::string sBuff(buffer.data());
		std::string msgRecv = sBuff.substr(0, bytesRead);
		std::cout << "Data buffer received from client >>> " << msgRecv << std::endl;
		
		// Prompt for response
		std::string msgSend;
		std::cout << "\033[32m" << "ENTER RESPONSE: > " << "\033[0m";
		std::getline(std::cin, msgSend);
		
		return msgSend;
	}
	return "";
}

void Server::ShutDown() {
    // Clean up
    #ifdef _WIN32
    	for (int i = 0; i < (int) clientSocketSet.fd_count; ++i)
    		closesocket(clientSocketSet.fd_array[i]);
    	closesocket(serverSocket);
    	WSACleanup();
	#endif
	#ifdef __linux__
	    for (int i = 0; i < (int) clients.size() ; ++i)
    		close(clients[i]);
		close(serv_sockfd);
	#endif
	std::cout << "Server shutdown." << std::endl;
	return;
}
