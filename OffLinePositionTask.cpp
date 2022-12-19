/**
 * @file OffLinePositionTask.cpp
 * @brief 辐射源离线定位任务，派生自PositionTask基类
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#include "../pch.h"
#include "PositionTask.h"
#include "OffLinePositionTask.h"
#include "OffLineDataSource.h"
#include "DFTaskDataBufferCluster.h"
#include "Logger.h"

using namespace std;
using namespace radarDF;

OffLinePositionTask::OffLinePositionTask(const radarDF::StartPositionRequest* request, const IPv4Address& radar_db_address)
	:PositionTask(PosType::OffLine,radar_db_address, request->task_describe())
{
	m_dfTaskDataBuffer = make_unique<DFTaskDataBufferCluster>(
		[this](DFTaskData& dfTaskData)
		{   
			offLinePosition(dfTaskData);//时间匹配后离线定位
		});
	for (auto& source : request->offline_param().data_source())
	{
		addDataSource(source, request->pos_freq(), radar_db_address);
	}
}

OffLinePositionTask::~OffLinePositionTask()
{
	
}

void OffLinePositionTask::addDataSource(const radarDF::PositionDataSource& dataSource, const double freq, const IPv4Address& radar_db_address)
{
	if (dataSource.task_runner_size() == 0)
	{//未指定定位设备，默认为所有测向设备
		m_dataSource.push_back(std::make_shared<OffLineDataSource>(dataSource.task_id().value(), NodeDevice(), freq, radar_db_address));
		for (auto nodeDev : m_dataSource.back()->getNodeDevice())
		{
			*taskAccount.add_node_devices() = nodeDev;
		}
	}
	else
	{//指定定位设备
		for (auto nodeDev : dataSource.task_runner())
		{
			m_dataSource.push_back(std::make_shared<OffLineDataSource>(dataSource.task_id().value(), nodeDev, freq, radar_db_address));
			*taskAccount.add_node_devices() = nodeDev;
		}
	}
}

void OffLinePositionTask::taskLoop()
{
	if (m_dataSource.size() == 0)
		return;
	int currentDataIndex = 0;
	m_runFlag = true;
	while (m_runFlag)
	{
		if (isDataEnough(currentDataIndex))
		{
			inputDFTaskData(currentDataIndex);
			CLOG("-Input DFTaskData To OffLinePosition");
		}
		else
		{
			CLOG("-OffLinePosition Data Not Enough");
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			break;
		}
	}
}

bool OffLinePositionTask::isDataEnough(int currentIdx) const
{
	for (auto& ds : m_dataSource)
	{
		auto isEnough = ds->isRecordEnough(currentIdx, ITEM_COUNT_PER_POSITION);
		if (!isEnough)
			return false;
	}
	return true;
}

void OffLinePositionTask::inputDFTaskData(int& currentIdx)
{
	for (uint16_t index = 0; index < m_dataSource.size(); index++)
	{
		auto dfTaskData = m_dataSource[index]->getTaskData(currentIdx, ITEM_COUNT_PER_POSITION);
		for (auto iter = dfTaskData.begin(); iter != dfTaskData.end(); iter++)
		{
			m_dfTaskData[iter->first] = iter->second;
		}
		if (m_dfTaskData.size() >= 2)
			break;
	}
	currentIdx += ITEM_COUNT_PER_POSITION;
#ifdef APPLY_TIME_MATCH 
	//时间匹配后离线定位
	m_dfTaskDataBuffer->inputDFTaskData(m_dfTaskData);
#else
	//按条数直接离线定位
	offLinePosition(m_dfTaskData);
#endif
	m_dfTaskData.clear();
}

void OffLinePositionTask::offLinePosition(const DFTaskData& dfTaskData)
{
	if (PositionTask::inputDFTaskData(dfTaskData))
		PositionTask::getPFTaskResult();
}