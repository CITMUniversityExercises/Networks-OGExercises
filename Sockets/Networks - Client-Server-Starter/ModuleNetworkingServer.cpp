#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	// TODO(jesus): TCP listen socket stuff

	// - Create the listenSocket
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSocket == INVALID_SOCKET)
	{
		reportError("ModuleNetworkingServer::start() - error on create socket");
		return false;
	}

	// --- Set address and port ---
	sockaddr_in serverAddr; // server
	serverAddr.sin_family = AF_INET; // IPv4
	serverAddr.sin_port = htons(port); // Port
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address

	// - Set address reuse

	// --- Make sure we can reuse IP and port ---
	int enable = 1;
	int iResult;
	iResult = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));

	if (iResult == SOCKET_ERROR)
		reportError("ModuleNetworkingServer::start() - error on setsockopt");

	// - Bind the socket to a local interface
	iResult = bind(listenSocket, (const struct sockaddr*)&serverAddr, sizeof(serverAddr));

	if (iResult != NO_ERROR)
		reportError("ModuleNetworkingServer::start() - error on bind");

	// - Enter in listen mode
	listen(listenSocket, 3);

	// - Add the listenSocket to the managed list of sockets using addSocket()
	addSocket(listenSocket);

	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, byte * data)
{
	// Set the player name of the corresponding connected socket proxy
	for (auto &connectedSocket : connectedSockets)
	{
		if (connectedSocket.socket == socket)
		{
			connectedSocket.playerName = (const char*)data;
		}
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			connectedSockets.erase(it);
			break;
		}
	}
}

