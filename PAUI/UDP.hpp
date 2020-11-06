
#ifndef UDP_HPP
#define UDP_HPP

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

class UDP
{
public:
	int publishUdpData(char* data)
	{
		WSADATA              wsaData;
		SOCKET               SendingSocket;
		SOCKADDR_IN          ReceiverAddr, SrcInfo;
		int                  Port = 9000;
		int TotalByteSent;

		// Initialize Winsock version 2.2
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			printf("Client: WSAStartup failed with error %ld\n", WSAGetLastError());
			WSACleanup();	// Clean up		
			return -1;		// Exit with error
		}

		// Create a new socket to receive datagrams on.
		SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (SendingSocket == INVALID_SOCKET)
		{
			printf("Client: Error at socket(): %ld\n", WSAGetLastError());
			WSACleanup();	// Clean up		
			return -1;		// Exit with error
		}

		ReceiverAddr.sin_family = AF_INET;
		ReceiverAddr.sin_port = htons(Port);
		ReceiverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

		TotalByteSent = sendto(SendingSocket, data, strlen(data), 0, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr));

		// When your application is finished receiving datagrams close the socket.
		if (closesocket(SendingSocket) != 0)
			printf("Client: closesocket() failed! Error code: %ld\n", WSAGetLastError());

		// When your application is finished call WSACleanup.
		if (WSACleanup() != 0)
			printf("Client: WSACleanup() failed! Error code: %ld\n", WSAGetLastError());

		// Back to the system
		return 0;
	}
};
#endif // UDP