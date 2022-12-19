/**
 * @file DFTaskDataBufferCluster.cpp
 * @brief 辐射源定位数据缓冲区簇
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022  中国电子科技集团公司第四十一研究所
 *
 */
#include "../pch.h"
#include "DFTaskDataBufferCluster.h"
#include "toolsFunc.h"

using namespace std;
using namespace radarDF;

DFTaskDataBufferCluster::DFTaskDataBufferCluster(std::function<void(DFTaskData& dfTaskData)> acquireDFTaskData)
	:m_acquireDFTaskDataFunc(acquireDFTaskData)
{
	m_acquireStartTimeFunc = [this](const uint64_t& startTime)
	{
		m_startTime = startTime;
		for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
		{
			iter->second->addNewStartTime(m_startTime);
		}
	};
	m_acquireBufferResultFunc = [this](BufferResult& result)
	{
		extrxctDFTaskData(result);
	};
}

DFTaskDataBufferCluster::~DFTaskDataBufferCluster()
{

}

void DFTaskDataBufferCluster::inputDFTaskData(const DFTaskData& dfTaskData)
{
	std::lock_guard<std::mutex> lg(lock);
	for (auto iter = dfTaskData.begin(); iter != dfTaskData.end(); iter++)
	{
		if (m_buffers.find(iter->first) == m_buffers.end())
		{
			auto bufferType = (m_buffers.size() == 0) ? BufferType::Primary : BufferType::Secondary;
			m_buffers[iter->first] = make_shared<DFTaskDataBuffer>(bufferType, m_startTime,*iter, m_acquireStartTimeFunc, m_acquireBufferResultFunc);
		}
		else
		{		
			m_buffers[iter->first]->inputRawDFItemList(iter->second);
		}
	}
	eraseInActiveBuffer();
}

//提时间匹配后DFTaskData
void DFTaskDataBufferCluster::extrxctDFTaskData(const BufferResult& result)
{
	m_bufferData[result.startTime].emplace_back(make_pair(result.id,result.itemList));
	if (m_bufferData[result.startTime].size() == m_buffers.size())
	{
		DFTaskData dfTaskData;
		for (auto td : m_bufferData[result.startTime])
		{
			dfTaskData[td.first] = td.second;
		}
		m_acquireDFTaskDataFunc(dfTaskData);
		auto iter = m_bufferData.find(result.startTime);
		m_bufferData.erase(m_bufferData.begin(), iter);
		m_bufferData.erase(iter);
	}
}

//删除不活跃缓冲区
void DFTaskDataBufferCluster::eraseInActiveBuffer()
{
	auto currentTime = DATACONVERT::getCurrentMsec();
	vector<TaskMsgId> bufferIds;
	for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
	{
		if (currentTime - iter->second->lastInputTime() > TIME_SPAN_MS)
			bufferIds.emplace_back(iter->first);
	}
	for (auto id : bufferIds)
	{
		if (m_buffers[id]->type() == BufferType::Secondary)
		{
			m_buffers.erase(id);
		}
		else
		{
			m_buffers.clear();
			break;
		}
	}
}