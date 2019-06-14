#include "CircularBuffer.h"



CircularBuffer::CircularBuffer(int capacity) : 
	bufferSize(capacity), front(0), rear(0), count(0), overspill()
{
	buffer = new Message[bufferSize];
}

void CircularBuffer::deposit(Message data)
{
	std::unique_lock<std::mutex> l(lock);

	if (count == bufferSize)
	{
		overspill.emplace(data);
	}
	else
	{
		buffer[rear] = data;
		rear = (rear + 1) % bufferSize;
		++count;
	}
}

Message CircularBuffer::fetch()
{
	std::unique_lock<std::mutex> l(lock);

	Message result{ NO_MESSAGE };

	if (!overspill.empty())
	{
		result = overspill.front;
		overspill.pop;
	}
	else if (count != 0)
	{
		result = buffer[front];
		front = (front + 1) % bufferSize;
		--count;
	}

	return result;
}
