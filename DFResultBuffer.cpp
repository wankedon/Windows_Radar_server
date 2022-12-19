/**
 * @file DFResultBuffer.cpp
 * @brief ����Դ������������
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
 *
 */
#include "../pch.h"
#include "DFResultBuffer.h"

using namespace std;
using namespace radarDF;

DFResultBuffer::DFResultBuffer()
	:m_maxSize(1024),
	m_inputTimes(0)
{
	
}

void DFResultBuffer::input(const DFResult& oneResult)
{
	std::lock_guard<std::mutex> lg(lock);
	if (buffer == nullptr)
		buffer = std::make_unique<std::list<DFResult>>();
	buffer->emplace_back(oneResult);
	if (buffer->size() > m_maxSize)
	{
		buffer->pop_front();
	}
	m_inputTimes++;

}

std::unique_ptr<std::list<DFResult>> DFResultBuffer::output()
{
	std::lock_guard<std::mutex> lg(lock);
	std::unique_ptr<std::list<DFResult>> result;
	result.swap(buffer);
	return result;
}

size_t DFResultBuffer::totalItemsCount() const
{
	std::lock_guard<std::mutex> lg(lock);
	size_t itemCount = 0;
	if (buffer)
	{
		for (auto iter = buffer->begin(); iter != buffer->end(); iter++)
		{
			itemCount += iter->items_size();
		}
	}	
	return itemCount;
}