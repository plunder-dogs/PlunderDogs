#include "NetworkHandler.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <iostream>

NetworkHandler::NetworkHandler()
	: m_mutex(),
	m_listenThread(),
	m_tcpSocket(),
	m_serverMessages(),
	m_connectedToServer(false),
	m_serverMessageBackLog()
{}

void NetworkHandler::sendServerMessage(const ServerMessage& message)
{
	assert(m_connectedToServer);
	if (m_serverMessageBackLog.empty())
	{
		sf::Packet packetToSend;
		packetToSend << message;
		if (m_tcpSocket.send(packetToSend) != sf::Socket::Done)
		{
			std::cout << "Unable to send message to server\n";
			std::lock_guard<std::mutex> lock(m_mutex);
			m_serverMessageBackLog.push_back(message);
		}
	}
	else
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		std::cout << "Added to server client backlog\n";
		m_serverMessageBackLog.push_back(message);
	}
}

bool NetworkHandler::isConnected() const
{
	return m_connectedToServer;
}

bool NetworkHandler::hasMessages()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return !m_serverMessages.empty();
}

ServerMessage NetworkHandler::getServerMessage()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	ServerMessage serverMessage = m_serverMessages.back();
	m_serverMessages.pop_back();
	return serverMessage;
}

bool NetworkHandler::connect()
{
	assert(!m_connectedToServer);
	if (m_tcpSocket.connect("81.97.212.79", 55001) != sf::Socket::Done)
	{
		return false;
	}

	m_connectedToServer = true;
	m_listenThread = std::thread(&NetworkHandler::listen, this);
	return true;
}

void NetworkHandler::disconnect()
{
	assert(m_connectedToServer);
	m_connectedToServer = false;
	m_listenThread.join();
	m_tcpSocket.disconnect();
}

void NetworkHandler::handleBackLog()
{
	for (auto iter = m_serverMessageBackLog.begin(); iter != m_serverMessageBackLog.end();)
	{
		sf::Packet packetToSend;
		packetToSend << (*iter);
		if (m_tcpSocket.send(packetToSend) == sf::Socket::Done)
		{
			iter = m_serverMessageBackLog.erase(iter);
		}
		else
		{
			break;
		}
	}
}

void NetworkHandler::listen()
{
	while (m_connectedToServer)
	{
		sf::Packet receivedPacket;
		if (m_tcpSocket.receive(receivedPacket) == sf::Socket::Done && receivedPacket)
		{
			ServerMessage receivedServerMessage;
			receivedPacket >> receivedServerMessage;
			std::lock_guard<std::mutex> lock(m_mutex);
			m_serverMessages.push_back(receivedServerMessage);
		}
		else
		{
			std::cout << "received incorrect packet\n";
		}
	}
}