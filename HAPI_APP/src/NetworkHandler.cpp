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

void NetworkHandler::sendServerMessage(ServerMessage message)
{
	sf::Packet packetToSend;
	packetToSend << message;
	if (m_tcpSocket.send(packetToSend) != sf::Socket::Done)
	{
		std::cout << "Unable to send message to server\n";
	}
}

std::vector<ServerMessage>& NetworkHandler::getServerMessages()
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
			else if (static_cast<eMessageType>(messageType) == eMessageType::eStartGame)
			{
				std::unique_lock<std::mutex> lock(m_serverMessageMutex);
				m_serverMessages.emplace_back(static_cast<eMessageType>(messageType));
			}
			else if (static_cast<eMessageType>(messageType) == eMessageType::eNewRemoteConnection)
			{
				std::unique_lock<std::mutex> lock(m_serverMessageMutex);
				m_serverMessages.emplace_back(static_cast<eMessageType>(messageType));
				std::vector<eShipType> shipsToAdd;
				receivedPacket >> shipsToAdd;
				m_serverMessages.back().shipsToAdd = shipsToAdd;
			}
		}
	}
}