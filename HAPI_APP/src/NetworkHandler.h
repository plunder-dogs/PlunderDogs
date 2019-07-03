#pragma once

#include "Global.h"
#include <SFML/Network.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <vector>

class NetworkHandler
{
public:
	NetworkHandler();
	~NetworkHandler();

	static NetworkHandler& getInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	void sendServerMessage(sf::Packet& packetToSend);
	void sendServerMessage(ServerMessage message);
	std::vector<ServerMessage>& getServerMessages();
	void connect();
	void disconnect();

private:
	sf::TcpSocket m_tcpSocket;
	std::thread m_listeningThread;
	bool m_connected = false;
	std::vector<ServerMessage> m_serverMessages;
	std::mutex m_serverMessageMutex;

	void listen();
};