/**
 * @file DFResultBufferCluster.cpp
 * @brief ����Դ��������������
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
 *
 */
#include "../pch.h"
#include "DFResultBufferCluster.h"
#include "DFResultBuffer.h"
#include "toolsFunc.h"

using namespace std;
using namespace radarDF;

DFResultBufferCluster::DFResultBufferCluster(std::function<void(DFTaskResult& result)> acquireResult)
	:m_acquireResult(acquireResult),
	m_initTime(DATACONVERT::getCurrentMsec())
{

}

void DFResultBufferCluster::inputDFResult(const TaskId& taskId,const radarDF::DFResult& oneResult)
{
	std::lock_guard<std::mutex> lg(lock);
	auto dsId = DATACONVERT::dataSourceId(taskId.value(), oneResult.result_from());
	if (buffers.find(dsId) == buffers.end())
	{
		buffers[dsId] = make_shared<DFResultBuffer>();
	}
	buffers[dsId]->input(oneResult);
	eraseInActiveBuffer();
	if (isWholeBatchByTime())
		outPutOneBatchResult();
}

//ɾ������Ծ������
void DFResultBufferCluster::eraseInActiveBuffer()
{
	auto bufferId = getMinMaxBufferId();
	if (bufferId.first.tid() != 0 && bufferId.second.tid() != 0)
	{   //���������С����������������BUFFER_SIZE_DISTANCE��ʱ��Ϊ��С������ʧЧ��ɾ���û�����
		if (buffers[bufferId.second]->inputTimes() - buffers[bufferId.first]->inputTimes() >= BUFFER_SIZE_DISTANCE)
		{
			buffers.erase(bufferId.first);
		}
	}
}

//����������������С�Ļ�����Id
std::pair<TaskMsgId, TaskMsgId> DFResultBufferCluster::getMinMaxBufferId()
{
	std::pair<TaskMsgId, TaskMsgId> bufferId;
	std::pair<int, int> maxMinInputTimes(INT_MAX, 0);
	for (auto iter = buffers.begin(); iter != buffers.end(); iter++)
	{
		auto inputTimes = iter->second->inputTimes();
		if (inputTimes > maxMinInputTimes.second)
		{
			maxMinInputTimes.second = inputTimes;
			bufferId.second = iter->first;
		}
		if (inputTimes < maxMinInputTimes.first)
		{
			maxMinInputTimes.first = inputTimes;
			bufferId.first = iter->first;
		}
	}
	return bufferId;
}

//�ж��Ƿ�һ�����ε�����
bool DFResultBufferCluster::isWholeBatchByCount()
{
	int bufferSizeOfBatch = 0;
	for (auto iter = buffers.begin(); iter != buffers.end(); iter++)
	{
		if (iter->second->totalItemsCount() > ITEM_COUNT_PER_BUFFER)
			bufferSizeOfBatch++;
	}
	return bufferSizeOfBatch == buffers.size() ? true : false;
}

//�ж��Ƿ�һ�����ε�����
bool DFResultBufferCluster::isWholeBatchByTime()
{
	auto newTime = DATACONVERT::getCurrentMsec();
	if (newTime - m_initTime >= TIME_SPAN_MS)
	{
		m_initTime = newTime;
		return true;
	}
	return false;
}

//���һ�����ε�����
void DFResultBufferCluster::outPutOneBatchResult()
{
	DFTaskResult taskResult;
	for (auto iter = buffers.begin(); iter != buffers.end(); iter++)
	{
		auto ptr = iter->second->output();
		if (ptr)
			taskResult[iter->first] = *ptr;
	}
	m_acquireResult(taskResult);
}