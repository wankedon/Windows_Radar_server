/**
 * @file DataFromConvertTools.h
 * @brief 数据格式转换工具
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "../pch.h"
#include "PositionSolver.h"
#include "CommonDef.h"
#include <time.h>
#include <windows.h>
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")

namespace DATACONVERT
{
	TaskMsgId dataSourceId(const uint32_t& taskId, const NodeDevice& nodeDevice);
	DFTaskData DFTaskResultToDFTaskData(DataSourceTag& dataIndexMap, const DFTaskResult& dfResult);
	DFItemList DFResultListToDFTaskDataList(DataSourceTag& dataIndexMap, std::pair<TaskMsgId, DFResultList> dfTaskResult);
	DFItemList DFResultToDFTaskDataList(DataSourceTag& dataIndexMap, TaskMsgId taskMsgId, const radarDF::DFResult& dfResult);
	PosSolver::ObserveData dfTaskDataItemToObserveData(const radarDF::DFTaskDataItem& rawItem, uint64_t stationId);
	radarDF::PositionResultItem targetDescribeToUIType(const PosSolver::TargetDescription& targetInfo, const DataSourceTag& stationId, const DFTaskData& dfData);
	radarDF::PositionTargetItem targetDescribeToDBType(const PosSolver::TargetDescription& targetInfo, const DataSourceTag& stationId, const DFTaskData& dfData);
	void fillTargetDescribe(radarDF::PositionTargetDescribe* targetDescribe, const PosSolver::TargetDescription& targetInfo, const DataSourceTag& stationId, const DFTaskData& dfData);
	void fillDFResult(radarDF::PositionResultItem& resultItem, const DFTaskData& dfData);
	void fillTargetPosition(Position* position, PosSolver::Location location);
	void fillTargetSpeed(radarDF::TargetSpeed* targetSpeed, PosSolver::Speed speed);
	std::vector<radarDF::DataSource> getMatchResultAndDataSource(radarDF::MatchResult* pulseSample, std::vector<PosSolver::FullId> ids, const DataSourceTag& stationId, const DFTaskData& dfData);
	DFItemList findDFItemListByStationId(uint32_t sId, const DataSourceTag& stationId, const DFTaskData& dfData);
	std::pair<radarDF::DataSource, radarDF::MatchResult> findMatchResultByTargetId(uint32_t targetId, const DFItemList& dfItemList);
	radarDF::PositionTaskInformation PositionTaskTagToPositionTaskInfo(PositionTaskTag& taskTag);
	bool TaskMsgIdEqual(const TaskMsgId& tm1, const TaskMsgId& tm2);
	uint32_t ipToUint(const std::string& address);
	struct timeval getTimeVal();
	uint64_t getCurrentMsec();
}