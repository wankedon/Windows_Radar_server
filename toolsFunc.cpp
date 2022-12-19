/**
 * @file DataFromConvertTools.cpp
 * @brief 数据格式转换工具
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#include "pch.h"
#include "toolsFunc.h"
#include "PositionSolver.h"

using namespace radarDF;
using namespace std;

namespace DATACONVERT
{
	//由任务Id、节点Id、设备Id组合生成数据源Id，标识一个数据源
	TaskMsgId dataSourceId(const uint32_t& taskId, const NodeDevice& nodeDevice)
	{
		TaskMsgId taskMsgId;
		taskMsgId.set_tid(taskId);
		taskMsgId.set_nid(nodeDevice.node_id().value());
		taskMsgId.set_did(nodeDevice.device_id().value());
		return taskMsgId;
	}

	//测向数据，转换为数据库格式
	DFTaskData DFTaskResultToDFTaskData(DataSourceTag& dataIndexMap, const DFTaskResult& dfResult)
	{
		DFTaskData dfTaskData;
		for (auto iter = dfResult.begin(); iter != dfResult.end(); iter++)
		{
			dfTaskData[iter->first] = DFResultListToDFTaskDataList(dataIndexMap, *iter);
		}
		return dfTaskData;
	}

	//测向数据，数据库格式转定位格式
	PosSolver::ObserveData dfTaskDataItemToObserveData(const radarDF::DFTaskDataItem& rawItem, uint64_t stationId)
	{
		auto item = rawItem.result_item();
		auto matchresult = item.match_result();
		PosSolver::ObserveData observeData;
		observeData.stationId = stationId;
		observeData.targetId = matchresult.target_id();
		observeData.pulseTime = 0;
		observeData.gpsTime = matchresult.occour_time().seconds()*1000+ matchresult.occour_time().nanos();
		observeData.doa_azi = matchresult.direction().azimuth();
		observeData.doa_ele = matchresult.direction().pitch();
		observeData.location.ln = item.device_position().longitude();
		observeData.location.lt = item.device_position().latitude();
		observeData.location.alt = item.device_position().altitude();
		auto pluseCluster = item.pulse_cluster();
		for (int i = 0; i < PosSolver::PARAM_LENGTH; i++)
		{
			auto pluseSample = pluseCluster.pulse_samples(i);
			observeData.rf[i] = pluseSample.carrier_freq();
			observeData.pw[i] = pluseSample.pulse_width();
			observeData.pri[i] = pluseSample.repeat_period();
		}
		return observeData;
	}

	//定位结果，算法格式转输出格式
	PositionResultItem targetDescribeToUIType(const PosSolver::TargetDescription& targetInfo, const DataSourceTag& stationId, const DFTaskData& dfData)
	{
		PositionResultItem resultItem;
		auto targetDescribe = resultItem.mutable_target_describe();
		fillTargetDescribe(targetDescribe, targetInfo,stationId, dfData);
		fillDFResult(resultItem, dfData);
		return resultItem;
	}

	//定位结果填充站点数据
	void fillDFResult(PositionResultItem& resultItem, const DFTaskData& dfData)
	{
		for (auto iter = dfData.begin(); iter != dfData.end(); iter++)
		{
			DFResult dfRes;
			for (auto item : iter->second)
			{
				*dfRes.add_items() = item.result_item();
				*dfRes.mutable_result_from() = item.data_source().node_device();
			}
			*resultItem.mutable_station_data()->Add() = dfRes;
		}
	}

	//定位结果，算法格式转数据库格式
	radarDF::PositionTargetItem targetDescribeToDBType(const PosSolver::TargetDescription& targetInfo, const DataSourceTag& stationId,const DFTaskData& dfData)
	{
		PositionTargetItem target;
		auto targetDescribe = target.mutable_target_describe();
		fillTargetDescribe(targetDescribe, targetInfo, stationId,dfData);
		return target;
	}

	void fillTargetDescribe(PositionTargetDescribe* targetDescribe,const PosSolver::TargetDescription& targetInfo, const DataSourceTag& stationId, const DFTaskData& dfData)
	{
		auto position = targetDescribe->mutable_target_position();
		fillTargetPosition(position, targetInfo.location);
		auto speed = targetDescribe->mutable_target_speed();
		fillTargetSpeed(speed, targetInfo.speed);
		MatchResult matchResult;
		auto dataSources = getMatchResultAndDataSource(&matchResult, targetInfo.ids, stationId, dfData);
		targetDescribe->set_target_id(matchResult.target_id()); //定位结果id为测向目标id
		*targetDescribe->mutable_target_info() = matchResult.pulse_sample();
		*targetDescribe->mutable_time() = matchResult.occour_time();
		for (auto ds : dataSources)
		{
			*targetDescribe->add_data_source() = ds;
		}
	}

	void fillTargetPosition(Position* position, PosSolver::Location location)
	{
		position->set_altitude(location.alt);
		position->set_longitude(location.ln);
		position->set_latitude(location.lt);
	}

	void fillTargetSpeed(TargetSpeed* targetSpeed, PosSolver::Speed speed)
	{
		targetSpeed->set_alt_speed(speed.altSpeed);
		targetSpeed->set_ln_speed(speed.lnSpeed);
		targetSpeed->set_lt_speed(speed.ltSpeed);
	}

	std::vector<DataSource> getMatchResultAndDataSource(MatchResult* matchResult, std::vector<PosSolver::FullId> ids, const DataSourceTag& stationId, const DFTaskData& dfData)
	{
		std::vector<DataSource> dataSource;
		for (auto id: ids)
		{
			auto dfItemList = findDFItemListByStationId(id.stationId, stationId, dfData);
			auto result = findMatchResultByTargetId(id.targetId, dfItemList);
			if (id.stationId == ids.begin()->stationId)
				*matchResult = result.second;
			dataSource.emplace_back(result.first);
		}
		return dataSource;
	}

	pair<DataSource, MatchResult> findMatchResultByTargetId(uint32_t targetId, const DFItemList& dfItemList)
	{
		pair<DataSource, MatchResult> result;
		for (auto dfItem : dfItemList)
		{
			if (dfItem.result_item().match_result().target_id() == targetId)
			{
				result.first = dfItem.data_source();
				result.second = dfItem.result_item().match_result();
				break;
			}
		}
		return result;
	}

	DFItemList findDFItemListByStationId(uint32_t sId, const DataSourceTag& stationId, const DFTaskData& dfData)
	{
		TaskMsgId taskMsgId;
		//找站号对应的数据源
		for (auto iterId = stationId.begin(); iterId != stationId.end(); iterId++)
		{
			if (iterId->second == sId)
				taskMsgId = iterId->first;
		}
		//找数据源对应的数据
		DFItemList dfItemList;
		for (auto iterData = dfData.begin(); iterData != dfData.end(); iterData++)
		{
			if (TaskMsgIdEqual(iterData->first, taskMsgId))
			{
				dfItemList = iterData->second;
			}
		}
		return dfItemList;
	}

	DFItemList DFResultListToDFTaskDataList(DataSourceTag& dataIndexMap, pair<TaskMsgId, DFResultList> dfTaskResult)
	{
		DFItemList wholeDataList;
		for (auto result : dfTaskResult.second)
		{
			auto partDataList = DFResultToDFTaskDataList(dataIndexMap, dfTaskResult.first, result);
			for (auto data : partDataList)
			{
				wholeDataList.emplace_back(data);
			}
		}
		return wholeDataList;
	}

	DFItemList DFResultToDFTaskDataList(DataSourceTag& dataIndexMap,TaskMsgId taskMsgId,const DFResult& dfResult)
	{
		auto nodeDevice = dfResult.result_from();
		DFItemList dataList;
		DFTaskDataItem dataItem;
		dataItem.mutable_data_source()->set_task_id(taskMsgId.tid());
		*dataItem.mutable_data_source()->mutable_node_device() = nodeDevice;
		for (auto item : dfResult.items())
		{
			*dataItem.mutable_data_source()->mutable_position() = item.device_position();
			dataItem.set_data_index(dataIndexMap[taskMsgId]);
			*dataItem.mutable_result_item() = item;
			dataList.emplace_back(dataItem);
			dataIndexMap[taskMsgId] += 1;
		}
		return dataList;
	}

	PositionTaskInformation PositionTaskTagToPositionTaskInfo(PositionTaskTag& taskTag)
	{
		PositionTaskInformation info;
		info.set_task_id(taskTag.task_id.value());
		info.set_task_describe(taskTag.describe);
		info.mutable_start_time()->set_seconds(taskTag.start_time);
		info.mutable_stop_time()->set_seconds(time(nullptr));
		auto datasource = taskTag.data_source;
		for (auto iter = datasource.begin(); iter != datasource.end(); iter++)
		{
			auto source = info.add_data_source();
			source->set_task_id(iter->first.tid());
			source->mutable_node_device()->mutable_node_id()->set_value(iter->first.nid());
			source->mutable_node_device()->mutable_device_id()->set_value(iter->first.did());
			*source->mutable_position() = iter->second;
		}
		return info;
	}

	bool TaskMsgIdEqual(const TaskMsgId& tm1, const TaskMsgId& tm2)
	{
		if (tm1.tid() != tm2.tid())
			return false;
		if (tm1.nid() != tm2.nid())
			return false;
		if (tm1.did() != tm2.did())
			return false;
		return true;
	}

	uint32_t ipToUint(const std::string& address)
	{
		return inet_addr(address.c_str());
	}

	struct timeval getTimeVal()
	{
		struct timeval tp;
		time_t clock;
		struct tm tm;
		SYSTEMTIME wtm;
		GetLocalTime(&wtm);
		tm.tm_year = wtm.wYear - 1900;
		tm.tm_mon = wtm.wMonth - 1;
		tm.tm_mday = wtm.wDay;
		tm.tm_hour = wtm.wHour;
		tm.tm_min = wtm.wMinute;
		tm.tm_sec = wtm.wSecond;
		tm.tm_isdst = -1;
		clock = mktime(&tm);
		tp.tv_sec = clock;
		tp.tv_usec = wtm.wMilliseconds * 1000;
		return tp;
	}

	uint64_t getCurrentMsec()
	{
		struct timeval timeval = getTimeVal();
		return (uint64_t)timeval.tv_sec * 1000 + (uint64_t)timeval.tv_usec / 1000;
	}

}

