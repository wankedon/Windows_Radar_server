/**
 * @file PositionTask.cpp
 * @brief 辐射源定位任务基类
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#include "../pch.h"
#include "PositionModule.h"
#include "PositionTask.h"
#include "RadarDBAccessor.h"
#include "MultiStationTargetMatch.h"
#include "toolsFunc.h"
#include "Logger.h"

using namespace std;;
using namespace radarDF;

PositionTask::PositionTask(const PosType& posType,const IPv4Address& radar_db_address, const std::string& describe)
	:ServerTask(RADAR_DF_TASK, DIRECTION_FINDING_SPATIAL_SPECTRUM),
	m_positionModule(make_unique<PositionModule>()),
	m_dbAccessor(make_unique<RadarDBAccessor>(radar_db_address)),
	m_multiMatch(make_unique<MultiStationTargetMatch>()),
	m_posType(posType)
{
	m_taskTag.describe = describe;
}

PositionTask::~PositionTask()
{
	abort();
}

std::pair<bool, std::unique_ptr<list<PositionResult>>> PositionTask::getBuffer(uint32_t token)
{
	updateVisitTime();
	auto result = bufferOutput(m_resultBuffer, token);
	result.first &= !isAborted();
	return result;
}

std::unique_ptr<TaskAccount> PositionTask::start()
{
	if (m_thread.joinable())
	{
		return  false;
	}
	function<void()> loopFunc = [this]() {taskLoop(); };
	function<void()> stopFunc = [this]() {m_runFlag = false; };
	m_thread.start(loopFunc, &stopFunc);
	m_taskTag.task_id = taskID();
	m_taskTag.start_time = time(nullptr);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN);
	LOG("--Start Position");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return make_unique<TaskAccount>(taskAccount);
}

bool PositionTask::inputDFTaskData(const DFTaskData& dfData)
{
	if (dfData.size() == 0)
		return false;
	fillPositionTaskTag(dfData);
	m_dfData = dfData;
	auto matchData = m_multiMatch->multiMatch(dfData);
	if (matchData.size())
	{
		m_positionModule->inputDFTaskData(matchData);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
		LOG("-Input {} MatchData To Position", matchData.begin()->second.size());
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		return true;
	}
	else 
	{
		return false;
	}
}

void PositionTask::fillPositionTaskTag(const DFTaskData& dfData)
{
	for (auto iter = dfData.begin(); iter != dfData.end(); iter++)
	{
		if (m_taskTag.data_source.find(iter->first) != m_taskTag.data_source.end())
			break;
		for (auto& item : iter->second)
		{
			if (m_taskTag.data_source.find(iter->first) != m_taskTag.data_source.end())
				break;
			m_taskTag.data_source[iter->first] = item.result_item().device_position();
		}
	}
}

void PositionTask::getPFTaskResult()
{
	auto targetDescribe = m_positionModule->getTargetDescription();
	auto stationId = m_positionModule->getStationId();
	PositionResult posResult;
	for (auto& td : targetDescribe)
	{
		auto targetItem = DATACONVERT::targetDescribeToDBType(td, stationId, m_dfData);
		targetItem.set_task_id(taskID().value());
		if (m_posType == PosType::OnLine)
			m_dbAccessor->writePositionTaskData(targetItem);
		*posResult.add_items() = DATACONVERT::targetDescribeToUIType(td, stationId, m_dfData);
	}
	m_resultBuffer.input(std::move(posResult));
}

void PositionTask::abort()
{
	m_thread.stop();
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN);
	LOG("--Stop Position");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	if (m_posType == PosType::OnLine)
		m_dbAccessor->writePositionTaskInfo(m_taskTag);
}

size_t PositionTask::bufferedItemCount() const
{
	return m_resultBuffer.count();
}

bool PositionTask::isAborted()
{
	return m_runFlag == false;
}