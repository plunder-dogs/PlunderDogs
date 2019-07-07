#include "NetworkHandler.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <iostream>

NetworkHandler::NetworkHandler()
{
	m_serverMessages.reserve(50);
}

NetworkHandler::~NetworkHandler()
{
	
}

void NetworkHandler::sendServerMessage(sf::Packet & packetToSend)
{
	if (m_tcpSocket.send(packetToSend) != sf::Socket::Done)
	{
		std::cout << "Unable to send message to server\n";
	}
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

bool NetworkHandler::isMessages()
{
	return !m_serverMessages.empty();
}

ServerMessage NetworkHandler::getServerMessage()
{
	std::lock_guard<std::mutex> lock(m_serverMessageMutex);

	ServerMessage serverMessage = m_serverMessages.back();
	m_serverMessages.pop_back();
	return serverMessage;
}

void NetworkHandler::connect()
{
	assert(!m_connected);
	sf::Socket::Status status = m_tcpSocket.connect(sf::IpAddress::LocalHost, 55001);
	assert(status == sf::Socket::Done);
	m_connected = true;
	m_listeningThread = std::thread(&NetworkHandler::listen, this);
}

void NetworkHandler::disconnect(FactionName localFactionName)
{
	sf::Packet packetToSend;
	packetToSend << static_cast<int>(eMessageType::eDisconnect) << static_cast<int>(localFactionName);
	m_tcpSocket.send(packetToSend);
	m_tcpSocket.disconnect();
	m_connected = false;
	m_listeningThread.join();
	std::cout << "Thread Ended\n";
}

void NetworkHandler::listen()
{
	sf::Packet receivedPacket;
	while (m_connected)
	{
		if (m_tcpSocket.receive(receivedPacket) == sf::Socket::Done && receivedPacket)
		{
			std::lock_guard<std::mutex> lock(m_serverMessageMutex);
			std::cout << "Received Packet\n";
			int messageType = -1;
			receivedPacket >> messageType;
			if (static_cast<eMessageType>(messageType) == eMessageType::eEstablishConnection)
			{
				int factionName = -1;
				receivedPacket >> factionName;
				m_serverMessages.emplace_back(static_cast<eMessageType>(messageType), static_cast<FactionName>(factionName));
			}
			else if (static_cast<eMessageType>(messageType) == eMessageType::eStartGame)
			{
				m_serverMessages.emplace_back(static_cast<eMessageType>(messageType));
			}
			else if (static_cast<eMessageType>(messageType) == eMessageType::eNewPlayer)
			{	
				int factionName = -1;
				receivedPacket >> factionName;
				ServerMessage message(static_cast<eMessageType>(messageType), static_cast<FactionName>(factionName));

				std::vector<eShipType> shipsToAdd;
				receivedPacket >> shipsToAdd;
				message.shipsToAdd = shipsToAdd;
				m_serverMessages.push_back(message);
			}

			receivedPacket.clear();
		}
	}
}