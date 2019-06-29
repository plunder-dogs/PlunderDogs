#pragma once

#include <mutex>
#include <queue>

struct ServerMessage
{
	//Put something in here
};

#define NO_MESSAGE ServerMessage()

class CircularBuffer
{
public:
	CircularBuffer(int capacity);

	~CircularBuffer() { delete[] buffer; }

	void deposit(ServerMessage data);

	ServerMessage fetch();

private:
	ServerMessage* buffer;
	const int bufferSize;

	int front;
	int rear;
	int count;

	std::mutex lock;

	std::queue<ServerMessage> overspill;
};
