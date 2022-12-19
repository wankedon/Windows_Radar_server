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

//���ݶ�λ����Դɸѡ����
DFTaskData OnLinePositionTask::filterDFTaskData(DFTaskData& rawDFTaskData)
{
	DFTaskData tempDFTaskData;
	if (m_dataSource.task_runner_size() == 0)
	{//δָ������Դ��Ĭ��Ϊ���в�������
		tempDFTaskData = rawDFTaskData;
	}
	else
	{//ָ������Դ����ȡ����Դ�Ĳ�������
		for (auto nd : m_dataSource.task_runner())
		{
			auto dsId = DATACONVERT::dataSourceId(m_dataSource.task_id().value(), nd);
			if (rawDFTaskData.find(dsId) != rawDFTaskData.end())
				tempDFTaskData[dsId] = rawDFTaskData[dsId];
		}
	}
	return filterDFTaskDataByFreq(tempDFTaskData);
}

//���ݶ�λƵ��ɸѡ����
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
