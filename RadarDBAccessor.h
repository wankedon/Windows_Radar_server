/**
 * @file RadarDBAccessor.h
 * @brief ����Դ����λ���ݿ⽻����
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-11-29
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
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