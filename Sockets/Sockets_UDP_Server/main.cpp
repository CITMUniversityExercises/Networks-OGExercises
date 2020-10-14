#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"
#include <stdlib.h> // system
#include <iostream> // cout

#pragma comment(lib, "ws2_32.lib")

SOCKET sk;
unsigned int port = 7777; // server port


// --- Utilities ---

void LogError()
{
	char msg[100];
	wchar_t* s = NULL;
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s,
		0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);

	//system("pause");
	//exit(-1);
}

// -----------------

// --- Basic methods ---

bool InitializeSocketsLibrary()
{
	bool ret = true;
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	// Log and handle error
	if (iResult != NO_ERROR)
	{
		LogError();
		ret = false;
	}

	return ret;
}

void CleanUp()
{
	// --- Stop and destroy sockets ---
	shutdown(sk, SD_BOTH);
	closesocket(sk);

	// --- Release library resources ---
	int iResult = WSACleanup();

	// Log and handle error
	if (iResult != NO_ERROR)
		LogError();
}

// -----------------

int main(int argc, char** argv)
{
	bool success = InitializeSocketsLibrary();

	if (success)
	{
		// --- Create socket ---
		sk = socket(AF_INET, SOCK_DGRAM, 0);

		// --- Set address and port ---
		sockaddr_in serverAddr; // server
		serverAddr.sin_family = AF_INET; // IPv4
		serverAddr.sin_port = htons(port); // Port
		serverAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address

		// --- Make sure we can reuse IP and port ---
		int enable = 1;
		int iResult;
		iResult = setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));

		if (iResult == SOCKET_ERROR)
			LogError();

		// --- Bind socket to specific address ---
		iResult = bind(sk, (const struct sockaddr*)&serverAddr, sizeof(serverAddr));

		if (iResult != NO_ERROR)
		{
			LogError();
		}

		// --- Receive message and send notification ---
		char* buffer = new char[6];
		sockaddr_in clientAddr; // client
		int addrSize = sizeof(sockaddr_in);

		for (unsigned int i = 0; i < 5; ++i)
		{
			// --- Receive message from client ---
			recvfrom(sk, buffer, 6, 0, (struct sockaddr*)&clientAddr, &addrSize); 

			std::cout << "Server received:" << buffer << std::endl;

			// --- Send message to client ---
			sendto(sk, "World", 6, 0, (const struct sockaddr*)&clientAddr, sizeof(clientAddr));
		}

	    delete[] buffer;

		system("pause");
	}

	CleanUp();

	return 0;
}


