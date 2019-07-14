#include "NetworkHandler.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <iostream>

//Approximately the maximum amount of messages a client will receive
constexpr size_t APPROX_MAX_MESSAGES = 12;

NetworkHandler::NetworkHandler()
{
	m_serverMessages.reserve(APPROX_MAX_MESSAGES);
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
	assert(status == sf::Socket::Done);
	m_tcpSocket.setBlocking(false);
	m_connectedToServer = true;
}

void NetworkHandler::disconnect()
{
	assert(m_connectedToServer);
	m_connectedToServer = false;
	m_tcpSocket.disconnect();
}

void NetworkHandler::listenToServer()
{
	assert(m_connectedToServer);
	sf::Packet receivedPacket;
	if (m_tcpSocket.receive(receivedPacket) == sf::Socket::Done && receivedPacket)
	{
		ServerMessage receivedServerMessage;
		receivedPacket >> receivedServerMessage;

		m_serverMessages.push_back(receivedServerMessage);
	}
}