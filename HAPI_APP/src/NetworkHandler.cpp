#include "NetworkHandler.h"
#include <assert.h>
#include <iostream>
#include <string>

NetworkHandler::NetworkHandler()
{

}

NetworkHandler::~NetworkHandler()
{
	m_listeningThread.join();
}

std::vector<Message>& NetworkHandler::getServerMessages()
{
	return m_serverMessages;
}

void NetworkHandler::connect()
{
	assert(!m_connected);
	sf::Socket::Status status = m_tcpSocket.connect(sf::IpAddress::LocalHost, 55001);
	assert(status == sf::Socket::Done);
	m_connected = true;
	m_listeningThread = std::thread(&NetworkHandler::listen, this);
}

void NetworkHandler::disconnect()
{
	m_connected = false;
	m_listeningThread.join();
}

void NetworkHandler::listen()
{
	sf::Packet receivedPacket;
	while (m_connected)
	{
		receivedPacket.clear();
		if (m_tcpSocket.receive(receivedPacket) == sf::Socket::Done)
		{
			int messageType = -1;
			receivedPacket >> messageType;
			if (static_cast<eMessageType>(messageType) == eMessageType::eEstablishConnection)
			{
				int factionName = -1;
				receivedPacket >> factionName;

				std::unique_lock<std::mutex> lock(m_serverMessageMutex);
				m_serverMessages.emplace_back(static_cast<eMessageType>(messageType),
					static_cast<FactionName>(factionName));
			}
		}
	}
}