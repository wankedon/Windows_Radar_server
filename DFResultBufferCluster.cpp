/**
 * @file DFResultBufferCluster.cpp
 * @brief 辐射源测向结果缓冲区簇
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
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

//删除不活跃缓冲区
void DFResultBufferCluster::eraseInActiveBuffer()
{
	auto bufferId = getMinMaxBufferId();
	if (bufferId.first.tid() != 0 && bufferId.second.tid() != 0)
	{   //当最大与最小缓冲区输入次数相差BUFFER_SIZE_DISTANCE次时认为最小缓冲区失效，删除该缓冲区
		if (buffers[bufferId.second]->inputTimes() - buffers[bufferId.first]->inputTimes() >= BUFFER_SIZE_DISTANCE)
		{
			buffers.erase(bufferId.first);
		}
	}
}

//获得输入次数最大和最小的缓冲区Id
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

//判断是否够一个批次的数据
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

//判断是否够一个批次的数据
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

//输出一个批次的数据
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