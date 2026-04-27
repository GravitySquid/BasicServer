#pragma once
#include <iostream>
#include <string>
#include <vector>

// For Windows - windsock
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // Link with Winsock library
#endif

// For Linux
#ifdef __linux__
    #include <stdio.h>
    #include <cstring>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

#define DATA_BUFSIZE 1024

class Server {
    private:
        std::string ip;
        std::string port;
       	std::vector<char> buffer;
        bool active;
        std::string statusMessage;
        struct sockaddr_in serv_addr, cli_addr;
        #ifdef _WIN32
            SOCKET serverSocket, clientSocket;
            fd_set clientSocketSet;
            fd_set read_set;
        #endif
        #ifdef __linux__
            int serv_sockfd;
            std::vector<int> clients;
            std::vector<int> clientsReadSet;
        #endif

    public:
        explicit Server(std::string ip, std::string port, int bufferSize);
        explicit Server(std::string ip, std::string port);
        void Listen();
        void ShutDown();
        int CheckClientActivity();
    
    private:
        std::string ProcessClientData(int bytesRead);
};
