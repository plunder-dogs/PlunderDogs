#pragma once

#include "Global.h"
#include <SFML/Network.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <vector>
#include <deque>
#include <functional>
#include <atomic>

enum FactionName;
class NetworkHandler
{
public:
	NetworkHandler();
	~NetworkHandler();

	void sendServerMessage(sf::Packet& packetToSend);
	void sendServerMessage(ServerMessage message);
	bool isMessages();
	ServerMessage getServerMessage();
	void connect();
	void disconnect(FactionName localFactionName);

private:
	sf::TcpSocket m_tcpSocket;
	std::thread m_listeningThread;
	std::atomic<bool> m_connected = false;
	std::vector<ServerMessage> m_serverMessages;
	std::mutex m_serverMessageMutex;

	void listen();
};