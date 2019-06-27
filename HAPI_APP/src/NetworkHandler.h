#pragma once

#include "Global.h"
#include <SFML/Network.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <vector>

enum class eMessageType
{
	eEstablishConnection = 0,
	eRemoteConnection
};

struct Message
{
	Message(eMessageType type, FactionName factionName)
		: type(type),
		factionName(factionName)
	{}

	eMessageType type;
	FactionName factionName;
};

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

	std::vector<Message>& getServerMessages();
	void connect();
	void disconnect();

private:
	sf::TcpSocket m_tcpSocket;
	std::thread m_listeningThread;
	bool m_connected = false;
	std::vector<Message> m_serverMessages;
	std::mutex m_serverMessageMutex;

	void listen();
};