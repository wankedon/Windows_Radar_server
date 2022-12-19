/**
 * @file DataFromConvertTools.cpp
 * @brief 数据格式定义
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include <string>
#include "node/radar/radarDF.pb.h"
#include "node/radar/RadarRecLoc.pb.h"
#include "node/nodeInternal.pb.h"

//自定义Map键的排序函数
struct keySortFunc
{
	bool operator()(const TaskMsgId& tm1, const TaskMsgId& tm2) const
	{
		if (tm1.tid() != tm2.tid())
			return tm1.tid() < tm2.tid() ? true : false;
		if (tm1.nid() != tm2.nid())
			return tm1.nid() < tm2.nid() ? true : false;
		if (tm1.did() != tm2.did())
			return tm1.did() < tm2.did() ? true : false;
		return false;
	}
};

//测向任务数据查询条件
struct QueryCondition
{
	uint32_t taskId;
	NodeDevice node_device;
	double freq;
	int start;
	int count;
};

//测向任务存储描述
struct DFTaskTag
{
	TaskId task_id;
	std::string describe;
	uint64_t start_time;
	radarDF::UploadRadarParams params;
	NodeReply reply;
	std::map<uint32_t,Position> device_position;
	std::atomic_bool run_flag;
};

using DFTaskInfo = std::map<uint32_t, radarDF::DFTaskInformation>; //测向任务信息 <任务Id,任务信息>
using DFItemList = std::list<radarDF::DFTaskDataItem>; //数据库存储格式测向数据集合
using DFTaskData = std::map<TaskMsgId, DFItemList, keySortFunc>; //数据库存储格式测向数据 <数据源,来自数据源的数据,key排序函数>
using DFResultList = std::list<radarDF::DFResult>; //输出格式测向数据集合
using DFTaskResult = std::map<TaskMsgId, DFResultList, keySortFunc>; //输出格式测向数据 <数据源,来自数据源的数据,key排序函数>
using DataSourceTag = std::map<TaskMsgId, uint32_t, keySortFunc>; //数据源 <数据源,数据条数/站号,key排序函数>
using PositionTaskInfo = std::map<uint32_t, radarDF::PositionTaskInformation>; //定位任务信息 <任务Id,任务信息>
using PositionTaskData = std::map<uint32_t, radarDF::PositionTargetItem>; //数据库存储格式定位数据 <任务Id,任务数据>
using PositionDataSourceTag = std::map<TaskMsgId, Position, keySortFunc>; //数据源 <数据源,数据源位置,key排序函数>
using DataTargetId = std::map<TaskMsgId, std::vector<uint32_t>, keySortFunc>; //数据源 <数据源,目标id,key排序函数>

//测向任务存储描述
struct PositionTaskTag
{
	TaskId task_id;
	std::string describe;
	uint64_t start_time;
	PositionDataSourceTag data_source;
};