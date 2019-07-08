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
	NetworkHandler();
	static NetworkHandler& getInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	bool hasMessages();
	ServerMessage getServerMessage();
	
	void sendServerMessage(sf::Packet& packetToSend);
	void sendServerMessage(ServerMessage message);
	void listenToServer();
	void connect();
	void disconnect(FactionName localFactionName);

private:
	sf::TcpSocket m_tcpSocket;
	std::vector<ServerMessage> m_serverMessages;
	bool m_connectedToServer = false;
};