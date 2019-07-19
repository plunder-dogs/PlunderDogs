#pragma once

#include "Global.h"
#include <SFML/Network.hpp>
#include <vector>
#include <vector>
#include <deque>
#include <functional>

enum FactionName;
class NetworkHandler
{
public:
	static NetworkHandler& getInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	bool hasMessages();
	ServerMessage getServerMessage();
	
	void setBlocking();
	void sendServerMessage(ServerMessage message);
	void listenToServer();
	bool connect();
	void disconnect();

private:
	NetworkHandler();
	sf::TcpSocket m_tcpSocket;
	std::vector<ServerMessage> m_serverMessages;
	bool m_connectedToServer = false;
};