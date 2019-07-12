#include "NetworkHandler.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <iostream>

NetworkHandler::NetworkHandler()
{

}

void NetworkHandler::sendServerMessage(sf::Packet & packetToSend)
{
	assert(m_connectedToServer);
	if (m_tcpSocket.send(packetToSend) != sf::Socket::Done)
	{
		std::cout << "Unable to send message to server\n";
	}
}

void NetworkHandler::sendServerMessage(ServerMessage message)
{
	assert(m_connectedToServer);
	sf::Packet packetToSend;
	packetToSend << message;
	if (m_tcpSocket.send(packetToSend) != sf::Socket::Done)
	{
		std::cout << "Unable to send message to server\n";
	}
}

bool NetworkHandler::hasMessages()
{
	return !m_serverMessages.empty();
}

ServerMessage NetworkHandler::getServerMessage()
{
	ServerMessage serverMessage = m_serverMessages.back();
	m_serverMessages.pop_back();
	return serverMessage;
}

void NetworkHandler::connect()
{
	assert(!m_connectedToServer);
	sf::Socket::Status status = m_tcpSocket.connect(sf::IpAddress::LocalHost, 55001);
	m_tcpSocket.setBlocking(false);
	assert(status == sf::Socket::Done);
	m_connectedToServer = true;
}

void NetworkHandler::disconnect(FactionName localFactionName)
{
	assert(m_connectedToServer);
	m_connectedToServer = false;
	sf::Packet packetToSend;
	packetToSend << static_cast<int>(eMessageType::eDisconnect) << static_cast<int>(localFactionName);
	m_tcpSocket.send(packetToSend);
	m_tcpSocket.disconnect();
}

void NetworkHandler::listenToServer()
{
	assert(m_connectedToServer);
	sf::Packet receivedPacket;
	if (m_tcpSocket.receive(receivedPacket) == sf::Socket::Done && receivedPacket)
	{
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
	}
}