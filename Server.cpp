#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

int main() {
    std::cout << "This is the Server...." << std::endl;

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
    // Set to localhost (IPv4 loopback address)
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) != 1) {
        std::cerr << "Error setting address to localhost." << std::endl;
        return 1;
    }
    serverAddress.sin_port = htons(27016);

    // Convert the binary IP address to a string
    char ipAddressStr[INET_ADDRSTRLEN]; // Buffer for IP address string
    inet_ntop(AF_INET, &(serverAddress.sin_addr), ipAddressStr, INET_ADDRSTRLEN);

    // Convert the port number to a string
    std::string tmp = ":";
    std::string address = ipAddressStr + tmp + std::to_string(ntohs(serverAddress.sin_port));

    // Bind the socket to an address
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
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
