#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "RingBuffer.h"
#include <math.h>
#include <memory.h>

#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Ư�̻��� : ���� ť�̱� ������ �Ҵ� ũ�Ⱑ BUFFER_SIZE ���� 1 Ŀ����
RingBuffer::RingBuffer(void)
{
	pBuffer_ = new char[BUFFER_SIZE + 1];
	front_ = rear_ = 0;
}

RingBuffer::~RingBuffer(void)
{
	delete[] pBuffer_;
}

// Return:  (int) ���� Buffer���� Enqueue�Ǿ��ִ� ũ��
int RingBuffer::GetUseSize(void)
{
	if (rear_ - front_ >= 0)
	{
		return rear_ - front_;
	}
	else
	{
		return BUFFER_SIZE - (front_ - rear_ - 1);
	}
}

// Return:(int)���� Buffer�� �߰��� Enequeue ������ ũ��
int RingBuffer::GetFreeSize(void)
{
	return BUFFER_SIZE - GetUseSize();
}

//--------------------------------------------------------------------
// ���: Ring Buffer�� Data ����
// Parameters:	IN (char *) Ring Buffer�� Data�� ���� ��� Buffer
//				IN (int) ������ ũ��
// Return:		(int) Ring Buffer�� �� ũ��(��ǻ� 0 Ȥ�� sizeToPut ���� �ϳ���)
// Ư�̻��� : Enqueue��å�� sizeToPut() > GetFreeSize()�̸� 0�� ��ȯ�Ѵ�.
//--------------------------------------------------------------------
int RingBuffer::Enqueue(IN char* pDest, IN size_t sizeToPut)
{
	size_t freeSize = GetFreeSize();	
	if (sizeToPut > freeSize)
	{
		// ��ȯ�ϴ� �ʿ����� ������ �����������.
		return 0;
	}
	size_t DirectSize = DirectEnqueueSize();

	// DirectSize�� sizeToPut���� ũ�� 
	size_t firstSize = DirectSize > sizeToPut ? sizeToPut : DirectSize;
	size_t secondSize = sizeToPut - firstSize;

	
	memcpy_s(GetWriteStartPtr(), firstSize, pDest, firstSize);
	MoveRear(firstSize);
	//memcpy_s(pBuffer_ + rear_ + 1, firstSize, pDest, firstSize);
	//rear_ = (rear_ + firstSize) % (BUFFER_SIZE + 1);
	if (secondSize == 0)
	{
		return firstSize;
	}
	memcpy_s(GetWriteStartPtr(), secondSize, pDest + firstSize, secondSize);
	MoveRear(secondSize);
	//memcpy_s(pBuffer_, secondSize, pDest + firstSize, secondSize);
	//rear_ = (rear_ + secondSize) % (BUFFER_SIZE + 1);
	return firstSize + secondSize;
}

//--------------------------------------------------------------------
// ���: Ring Buffer���� Data ����
// Parameters:	OUT (char *) Ring Buffer�� Data�� ���� ��� Buffer
//				IN (int) ������ ũ��
// Return:		(int) Ring Buffer���� ������ pDest�� ������ �������� ũ��
// Ư�̻��� : Dequeue��å�� sizeToRead > GetUseSize()�̸� Dequeue�� ����� ���������ʰ� 0�� ��ȯ�Ѵ�.
//--------------------------------------------------------------------
int RingBuffer::Dequeue(OUT char* pDest, IN size_t sizeToRead)
{
	size_t useSize = GetUseSize();
	if (sizeToRead > useSize)
	{
		// ����ִ� �����ͺ��� ���� �����͸� �������� �ϸ� �׳� ��ȯ�Ѵ�.
		return 0;
	}
	size_t directSize = DirectDequeueSize();
	size_t firstSize = directSize > sizeToRead ? sizeToRead : directSize;
	size_t secondSize = sizeToRead - firstSize;

	memcpy_s(pDest, firstSize, GetReadStartPtr(), firstSize);
	MoveFront(firstSize);
	//memcpy_s(pDest, firstSize, pBuffer_ + front_ + 1, firstSize);
	//front_ = (front_ + firstSize) % (BUFFER_SIZE + 1);
	if (secondSize <= 0)
		return firstSize;
	memcpy_s(pDest + firstSize, secondSize, GetReadStartPtr(), secondSize);
	MoveFront(secondSize);
	//memcpy_s(pDest + firstSize, secondSize, pBuffer_, secondSize);
	//front_ = (front_ + secondSize) % (BUFFER_SIZE + 1);
	return firstSize + secondSize;
}

// �ش��Լ��� Dequeue�� ���� ������ ���縸 �����ϰ� front�� ��ġ�� �ٲ��� �ʴ´�. 
int RingBuffer::Peek(IN int sizeToPeek, OUT char* pTarget)
{
	int useSize = GetUseSize();
	if (sizeToPeek > useSize)
	{
		// ����ִ� �����ͺ��� ���� �����͸� �������� �ϸ� �׳� ��ȯ�Ѵ�.
		return 0;
	}
	int directSize = DirectDequeueSize();
	int firstSize = directSize > sizeToPeek ? sizeToPeek: directSize;
	int secondSize = sizeToPeek - firstSize;

	//memcpy_s(pTarget, firstSize, GetReadStartPtr(), firstSize);
	memcpy_s(pTarget, firstSize, pBuffer_ + front_ + 1, firstSize);
	if (secondSize <= 0)
		return firstSize;
	memcpy_s(pTarget + firstSize, secondSize, pBuffer_, secondSize);
	return firstSize + secondSize;
}

void RingBuffer::ClearBuffer(void)
{
	rear_ = front_;
}


// ��� : rear_ + 1���� ť�� �Ǿ����� �̵����� �ʰ� Enqueue������ �ִ� ũ�⸦ ��ȯ��.
// �� rear+ 1���� front_ - 1������ �Ÿ� Ȥ�� rear_ + 1���� 
int RingBuffer::DirectEnqueueSize(void)
{
	if (rear_ >= front_)
	{
		return BUFFER_SIZE - rear_;
	}
	else
	{
		return front_ - rear_ - 1;
	}
}

int RingBuffer::DirectDequeueSize(void)
{
	if (rear_ >= front_)
	{
		return rear_ - front_;
	}
	else
	{
		return BUFFER_SIZE  - front_;
	}
}

// sizeToMove��ŭ rear_�� �̵���Ű�� ���� rear_�� ��ȯ��.
int RingBuffer::MoveRear(IN int sizeToMove)
{
	rear_ = (rear_ + sizeToMove) % (BUFFER_SIZE + 1);
	return rear_;
}

// sizeToMove��ŭ front_�̵���Ű�� ���� rear_�� ��ȯ��.
int RingBuffer::MoveFront(IN int sizeToMove)
{
	front_ = (front_ + sizeToMove) % (BUFFER_SIZE + 1);
	return front_;
}

// ���� front_�ε����� �˸´� �����͸� ��ȯ
char* RingBuffer::GetFrontPtr(void)
{
	return pBuffer_ + front_;
}

// ���� rear_ �ε����� �˸´� �����͸� ��ȯ
char* RingBuffer::GetRearPtr(void)
{
	return pBuffer_ + rear_;
}

char* RingBuffer::GetWriteStartPtr(void)
{
	return pBuffer_ + ((rear_ + 1) % (BUFFER_SIZE + 1));
}

char* RingBuffer::GetReadStartPtr(void)
{
	return pBuffer_ + ((front_ + 1) % (BUFFER_SIZE + 1));
}


