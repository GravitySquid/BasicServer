#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

int main(int argc, char* argv[]) {
    std::cout << "Server started...." << std::endl;

    const char* ip, * port;
    if (argc == 2) // Expect IP & port number
    {
        ip = argv[1];
        port = argv[2];
    }
    else // defaults
    {
        std::cout << "Default parameters used ... " << std::endl;
        ip = "127.0.0.1"; // localhost (IPv4 loopback address)
        port = "27016";
    }
    std::cout << "IP to listen to ..... " << ip << std::endl;
    std::cout << "Port to listen to ... " << port << std::endl;

    // Convert port
    unsigned short  usPort = (unsigned short)std::stoi(port);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }
    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &(serverAddress.sin_addr)) != 1) {
        std::cerr << "Error setting address to localhost." << std::endl;
        return 1;
    }
    serverAddress.sin_port = htons(usPort);

    // Convert the binary IP address to a string
    char ipAddressStr[INET_ADDRSTRLEN]; // Buffer for IP address string
    inet_ntop(AF_INET, &(serverAddress.sin_addr), ipAddressStr, INET_ADDRSTRLEN);

    // Convert the port number to a string
    std::string tmp = ":";
    std::string address = ip + tmp + port;

    // Bind the socket to an address
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) 
    {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server listening on port " + address + " ..." << std::endl;

    // Accept client connections
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    char buffer[1024];
    char bufferCopy[1024];
    int bytesRead;

    // Receive data from client and echo it back
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        strncpy_s(bufferCopy, buffer, bytesRead);
        std::cout << "Data buffer received from client >>> " << bufferCopy << std::endl;
        send(clientSocket, buffer, bytesRead, 0);
    }

    // Clean up
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    std::cout << "Connection closed." << std::endl;
    return 0;
}
