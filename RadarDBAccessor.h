/**
 * @file RadarDBAccessor.h
 * @brief 辐射源测向定位数据库交互者
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-11-29
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "CommonDef.h"
#include "node/radar/RadarRecLoc.grpc.pb.h"
#include "node/database/dataaccesslayer.pb.h"

using QueryRepeatedPtr = google::protobuf::RepeatedPtrField<::platformdal::QueryParam>;
class RadarDBAccessor
{
public:
	RadarDBAccessor(const IPv4Address& address);
	~RadarDBAccessor();

public:
	bool writeDFTaskInfo(DFTaskTag& taskTag);
	bool writeDFTaskData(const DFItemList& itemList);
	bool writePositionTaskInfo(PositionTaskTag& taskTag);
	bool writePositionTaskData(const radarDF::PositionTargetItem& targetItem);
	DFTaskData readDFTaskData(const QueryCondition& queryCondition);
	DFTaskInfo readDFTaskInfo();
	PositionTaskInfo readPositionTaskInfo();
	PositionTaskData readPositionTaskData(const uint32_t& taskid);
	DataSourceTag queryRecordCount(const QueryCondition& queryCondition);
	bool isRecordEnough(const QueryCondition& queryCondition);

private:
	TaskAccount extractTaskAccount(const NodeReply& reply);
	radarDF::DFTaskInformation fillDFTaskInfo(DFTaskTag& taskTag);
	void addQueryParam(QueryRepeatedPtr* query, std::string key, std::string value);
	bool accessDFTaskInfo(radarDF::DFTaskInfoRequest& dbRequest);

private:
	std::unique_ptr<radarDF::RadarRecLoc::Stub> m_dbClient;
	static const uint32_t FREQ_STEP = 200;
};