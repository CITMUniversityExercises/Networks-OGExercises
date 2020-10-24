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
		sockaddr_in serverAddr; // client
		serverAddr.sin_family = AF_INET; // IPv4
		serverAddr.sin_port = htons(port); // Port
		const char* remoteAddrStr = "127.0.0.1"; 
		inet_pton(AF_INET, remoteAddrStr, &serverAddr.sin_addr);

		// --- Get CPU frequency ---
		double PCFreq = 0.0;
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		PCFreq = double(li.QuadPart) / 1000.0;

		// --- Send message to server and receive notification ---
		char* buffer = new char[6];
		//sockaddr_in serverAddr;
		int addrSize = sizeof(sockaddr_in);

		for (unsigned int i = 0; i < 5; ++i)
		{
			LARGE_INTEGER start_time;
			LARGE_INTEGER end_time;
			QueryPerformanceCounter(&start_time);

			// --- Send message to server ---
			sendto(sk, "PING", 6, 0, (const struct sockaddr*)&serverAddr, sizeof(serverAddr));

			// --- Receive message from server ---
			recvfrom(sk, buffer, 6, 0, nullptr, nullptr); // not storing anything since we already know the server's address

			QueryPerformanceCounter(&end_time);

			std::cout << "Time elapsed: " << (end_time.QuadPart - start_time.QuadPart) / PCFreq << " milliseconds" << std::endl;
			std::cout << "Client received:" << buffer << std::endl;
		}

		delete[] buffer;

		system("pause");
	}

	CleanUp();

	return 0;
}


