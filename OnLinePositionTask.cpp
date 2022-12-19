#include "../pch.h"
#include "node/radar/radarDF.pb.h"
#include "OnLinePositionTask.h"
#include "toolsFunc.h"

using namespace std;
using namespace radarDF;

OnLinePositionTask::OnLinePositionTask(const radarDF::StartPositionRequest* request, const IPv4Address& radar_db_address)
	:PositionTask(PosType::OnLine,radar_db_address, request->task_describe()),
	m_dataSource(request->online_param().data_source()),
	m_posFreq(request->pos_freq())
{

}

void OnLinePositionTask::taskLoop()
{
	m_runFlag = true;
}

void OnLinePositionTask::inputDFTaskData(const DFTaskData& dfDataItem)
{
	DFTaskData rawDataItem = dfDataItem;
	auto inputData = PositionTask::inputDFTaskData(filterDFTaskData(rawDataItem));
	if (inputData)
		PositionTask::getPFTaskResult();
}

//根据定位数据源筛选数据
DFTaskData OnLinePositionTask::filterDFTaskData(DFTaskData& rawDFTaskData)
{
	DFTaskData tempDFTaskData;
	if (m_dataSource.task_runner_size() == 0)
	{//未指定数据源，默认为所有测向数据
		tempDFTaskData = rawDFTaskData;
	}
	else
	{//指定数据源，提取数据源的测向数据
		for (auto nd : m_dataSource.task_runner())
		{
			auto dsId = DATACONVERT::dataSourceId(m_dataSource.task_id().value(), nd);
			if (rawDFTaskData.find(dsId) != rawDFTaskData.end())
				tempDFTaskData[dsId] = rawDFTaskData[dsId];
		}
	}
	return filterDFTaskDataByFreq(tempDFTaskData);
}

//根据定位频率筛选数据
DFTaskData OnLinePositionTask::filterDFTaskDataByFreq(DFTaskData& rawDFTaskData)
{
	if (m_posFreq == 0)
		return rawDFTaskData;
	DFTaskData tempDFTaskData;
	for (auto iter = rawDFTaskData.begin(); iter != rawDFTaskData.end(); iter++)
	{
		DFItemList itemlist;
		for (auto item : iter->second)
		{
			auto itemFreq = item.result_item().pulse_cluster().freq();
			if (itemFreq > m_posFreq - FREQ_STEP && itemFreq < m_posFreq + FREQ_STEP)
				itemlist.emplace_back(item);
		}
		if (itemlist.size())
			tempDFTaskData[iter->first] = itemlist;
	}
	return tempDFTaskData;
}
