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


//Atomic more performant than a mutex
//Race Condition

//int i = 5

//load - a
//load - b
//increment - a
//storage - a - i == 6 
//increment - b i == 6
//storage - b 

enum class eFactionName;
class NetworkHandler : private NonCopyable
{
public:
	static NetworkHandler& getInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	bool isConnectedToServer() const;
	bool hasMessages();
	ServerMessage getServerMessage();
	
	
	void sendMessageToServer(const ServerMessage& message);
	bool connectToServer();
	void disconnectFromServer();
	void handleBackLog();

private:
	NetworkHandler();
	std::mutex m_mutex;
	sf::TcpSocket* m_tcpSocket;
	std::vector<ServerMessage> m_serverMessages;
	std::atomic<bool> m_connectedToServer;
	std::vector<ServerMessage> m_serverMessageBackLog;
	std::thread m_listenThread;

	void listen();
};