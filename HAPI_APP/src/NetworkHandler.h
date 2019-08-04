#pragma once

#include "Global.h"
#include "Utilities/NonCopyable.h"
#include <SFML/Network.hpp>
#include <vector>
#include <vector>
#include <deque>
#include <functional>
#include <mutex>
#include <atomic>

enum FactionName;
class NetworkHandler : private NonCopyable
{
public:
	static NetworkHandler& getInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	bool isConnected() const;
	bool hasMessages();
	ServerMessage getServerMessage();
	
	void sendServerMessage(ServerMessage message);
	bool connect();
	void disconnect();

private:
	NetworkHandler();
	std::mutex m_mutex;
	std::thread m_listenThread;
	sf::TcpSocket m_tcpSocket;
	std::vector<ServerMessage> m_serverMessages;
	std::atomic<bool> m_connectedToServer;

	void listen();
};