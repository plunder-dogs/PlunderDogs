#pragma once

#include <mutex>
#include <queue>

struct Message
{
	//Put something in here
};

#define NO_MESSAGE Message()

class CircularBuffer
{
private:
	Message* buffer;
	const int bufferSize;

	int front;
	int rear;
	int count;

	std::mutex lock;

	std::queue<Message> overspill;

public:
	CircularBuffer(int capacity);

	~CircularBuffer() { delete[] buffer; }

	void deposit(Message data);

	Message fetch();
};
