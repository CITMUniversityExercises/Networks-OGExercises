#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"
#include <stdlib.h> // system
#include <iostream> // cout

#pragma comment(lib, "ws2_32.lib")

SOCKET sk;
unsigned int port = 7777;


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
		sockaddr_in remoteAddr;
		remoteAddr.sin_family = AF_INET; // IPv4
		remoteAddr.sin_port = htons(port); // Port

		//remoteAddr.sin_addr.S_un.S_un_b.s_b1 = 192;
		//remoteAddr.sin_addr.S_un.S_un_b.s_b2 = 168;
		//remoteAddr.sin_addr.S_un.S_un_b.s_b3 = 1;
		//remoteAddr.sin_addr.S_un.S_un_b.s_b4 = 33;
		const char* remoteAddrStr = "127.0.0.1"; 
		inet_pton(AF_INET, remoteAddrStr, &remoteAddr.sin_addr);

		system("pause");
	}

	CleanUp();

	return 0;
}


