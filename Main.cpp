
#include <iostream>
#include <string>
#include "Server.h"

int main(int argc, char* argv[]) {
	std::cout << "Server starting...." << std::endl;

	// Check optional command line arguments
	std::string ip, port;
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

	// Create Server
	Server server(ip,port);

	// Listen for any incoming connections
	server.Listen();

	// Main server loop - keep active
	while (true)
	{
		// Check for client activity and accept new connections
		server.CheckClientActivity();
	}

	// Shutdown server
	server.ShutDown();
	return 0;
}
