/**
 * @file DFTaskDataBuffer.cpp
 * @brief ����Դ��λ���ݻ�����
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022  �й����ӿƼ����Ź�˾����ʮһ�о���
 *
 */
#include "../pch.h"
#include "DFTaskDataBuffer.h"
#include "toolsFunc.h"
#include <stdlib.h>

using namespace std;
using namespace radarDF;

DFTaskDataBuffer::DFTaskDataBuffer(const BufferType& type, uint64_t startTime,const std::pair<TaskMsgId, DFItemList>& dfTaskData,AcquireStartTimeFunc acquireTime,AcquireBufferResultFunc acquireResult)
	:m_id(dfTaskData.first),
	m_type(type),
	m_startTime(0),
	m_lastInputTime(0),
	m_acquireStartTime(acquireTime),
	m_acquireBufferResultFunc(acquireResult)
{
	addNewStartTime(startTime);
	inputRawDFItemList(dfTaskData.second);
}

DFTaskDataBuffer::~DFTaskDataBuffer()
{

}

//����DFItemList
void DFTaskDataBuffer::inputRawDFItemList(const DFItemList& itemList)
{
	std::lock_guard<std::mutex> lg(lock);
	for (auto item : itemList)
	{
		createBuffer(item);
		if (isItemOfCurrentBatch(item))
		{
			m_buffer->emplace_back(item);
		}
		else
		{
			outputCurrentBatchItems();
			createBuffer(item);
			m_buffer->emplace_back(item);
		}
	}
	m_lastInputTime = DATACONVERT::getCurrentMsec();
}

//����buffer
void DFTaskDataBuffer::createBuffer(const radarDF::DFTaskDataItem& item)
{
	if (m_buffer == nullptr)
		m_buffer = make_unique<std::list<radarDF::DFTaskDataItem>>();
	if (m_buffer->size() == 0)
		updateStartTime(item);
}

//��Ŀ��ȡʱ��
int64_t DFTaskDataBuffer::itemOccourTime(const radarDF::DFTaskDataItem& item)
{
	auto occourTime = item.result_item().match_result().occour_time();
	return occourTime.seconds() * 1000 + occourTime.nanos();
}

//���������������ο�ʼʱ��
void DFTaskDataBuffer::updateStartTime(const radarDF::DFTaskDataItem& item)
{
	if (m_type == BufferType::Primary)
	{
		m_startTime = itemOccourTime(item);
		m_acquireStartTime(m_startTime);
	}
	if (m_type == BufferType::Secondary)
	{
		m_startTime = getStartTime();
	}
}

//��ȡ�λ��������ο�ʼʱ��
uint64_t DFTaskDataBuffer::getStartTime()
{
	uint64_t startTime;
	if (m_secondStartTimeList.size())
	{
		startTime = m_secondStartTimeList.front();
		m_secondStartTimeList.pop_front();
	}
	else
	{
		startTime = m_startTime;
	}
	return startTime;
}

//��Ӵλ��������ο�ʼʱ��
void DFTaskDataBuffer::addNewStartTime(const uint64_t& startTime)
{
	if (m_type != BufferType::Secondary)
		return;
	if (m_secondStartTimeList.size() == 0)
	{
		m_secondStartTimeList.push_back(startTime);
	}
	else
	{
		if (m_secondStartTimeList.back() != startTime)
			m_secondStartTimeList.push_back(startTime);
	}
}

//�ж���Ŀ�Ƿ����ڵ�ǰ����
bool DFTaskDataBuffer::isItemOfCurrentBatch(const radarDF::DFTaskDataItem& item)
{
	if (fabs(itemOccourTime(item) - m_startTime) < TIME_SPAN_MS)
		return true;
	else
		return false;
}

//�����ǰ��������
void DFTaskDataBuffer::outputCurrentBatchItems()
{
	auto items = *getCurrentBatchItems();
	if (items.size())
		m_acquireBufferResultFunc(BufferResult{ m_id,m_startTime, items });
}

//��ȡ��ǰ��������
std::unique_ptr<std::list<radarDF::DFTaskDataItem>> DFTaskDataBuffer::getCurrentBatchItems()
{
	std::unique_ptr<list<radarDF::DFTaskDataItem>> result;
	result.swap(m_buffer);
	return result;
}