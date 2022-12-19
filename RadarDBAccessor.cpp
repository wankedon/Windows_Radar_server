/**
 * @file RadarDBAccessor.cpp
 * @brief 辐射源测向定位数据库交互者
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-11-29
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#include "../pch.h"
#include <fmt_/format.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include "RadarDBAccessor.h"
#include "toolsFunc.h"

using namespace radarDF;
using namespace platformdal;
using namespace grpc;
using namespace std;

RadarDBAccessor::RadarDBAccessor(const IPv4Address& address)
{
	auto m_dbClientAddress = fmt::format("{}:{}", address.ip(), address.port());
	auto dbChannel = grpc::CreateChannel(m_dbClientAddress, grpc::InsecureChannelCredentials());
	m_dbClient = RadarRecLoc::NewStub(dbChannel);
}

RadarDBAccessor::~RadarDBAccessor()
{

}

//测向任务信息写进数据库
bool RadarDBAccessor::writeDFTaskInfo(DFTaskTag& taskTag)
{
	DFTaskInfoRequest tiRequest;
	if (taskTag.run_flag)
	{
		tiRequest.set_method(RequestMethod::M_POST);
	}
	else
	{
		tiRequest.set_method(RequestMethod::M_PUT);
		addQueryParam(tiRequest.mutable_query(), "taskId", to_string(taskTag.task_id.value()));
	}
	*tiRequest.mutable_body() = fillDFTaskInfo(taskTag);
	tiRequest.mutable_body()->mutable_start_time()->set_seconds(taskTag.start_time);
	tiRequest.mutable_body()->mutable_stop_time()->set_seconds(time(nullptr));
	return accessDFTaskInfo(tiRequest);
}

DFTaskInformation RadarDBAccessor::fillDFTaskInfo(DFTaskTag& taskTag)
{
	DFTaskInformation taskInfo;
	*taskInfo.mutable_account() = extractTaskAccount(taskTag.reply);//任务账号
	taskInfo.set_task_describe(taskTag.describe);//任务描述
	*taskInfo.mutable_task_params() = taskTag.params;//任务参数
	for (auto nd: taskInfo.mutable_account()->node_devices())
	{
		auto ds = taskInfo.add_data_source();
		ds->set_task_id(taskInfo.mutable_account()->task_id().value());
		*ds->mutable_node_device() = nd;
		*ds->mutable_position() = taskTag.device_position[nd.device_id().value()];
	}
	return taskInfo;
}

//TaskAccount
TaskAccount RadarDBAccessor::extractTaskAccount(const NodeReply& reply)
{
	TaskAccount account;
	for (auto header : reply.replys())
	{
		*account.mutable_task_id() = header.task_id();
		*account.add_node_devices() = header.task_runner();
	}
	*account.mutable_reply_details() = reply;
	return account;
}

bool RadarDBAccessor::accessDFTaskInfo(DFTaskInfoRequest& dbRequest)
{
	ClientContext context;
	std::unique_ptr<ClientReader<DFTaskInfoResponse>> reader(m_dbClient->accessTaskInfo(&context, dbRequest));
	DFTaskInfoResponse tiResponse;
	while (reader->Read(&tiResponse))
	{
	};
	auto status = reader->Finish();
	if (status.error_code() == Status::OK.error_code())
	{
		return tiResponse.status_code() == 0;
	}
	else
	{
		return false;
	}
}

//从数据库读测向任务信息（所有任务）
DFTaskInfo RadarDBAccessor::readDFTaskInfo()
{
	DFTaskInfoRequest tiRequest;
	ClientContext context;
	tiRequest.set_method(RequestMethod::M_GET);
	std::unique_ptr<ClientReader<DFTaskInfoResponse>> reader(m_dbClient->accessTaskInfo(&context, tiRequest));
	DFTaskInfo taskInfo;
	DFTaskInfoResponse tiResponse;
	while (reader->Read(&tiResponse))
	{
		taskInfo[tiResponse.mutable_body()->mutable_account()->task_id().value()]= tiResponse.body();
	};
	return taskInfo;
}

//测向任务数据写进数据库
bool RadarDBAccessor::writeDFTaskData(const DFItemList& itemList)
{
	DFTaskDataRequest tdRequest;
	ClientContext context;
	tdRequest.set_method(RequestMethod::M_POST);
	tdRequest.clear_body();
	for (auto tk : itemList)
	{
		*tdRequest.add_body() = tk;
	}
	std::unique_ptr<ClientReader<DFTaskDataResponse>> reader(m_dbClient->accessTaskData(&context, tdRequest));
	DFTaskDataResponse tdResponse;
	while (reader->Read(&tdResponse))
	{
	}
	return true;
}

//从数据库读测向任务数据
DFTaskData RadarDBAccessor::readDFTaskData(const QueryCondition& queryCondition)
{
	DFTaskDataRequest tdRequest;
	ClientContext context;
	auto query = tdRequest.mutable_query();
	addQueryParam(query, "taskId", to_string(queryCondition.taskId));
	auto nd = queryCondition.node_device;
	if(nd.has_node_id())
		addQueryParam(query, "nodeId", to_string(nd.node_id().value()));
	if (nd.has_device_id())
		addQueryParam(query, "deviceId", to_string(nd.device_id().value()));
	addQueryParam(query, "index_start", to_string(queryCondition.start));
	addQueryParam(query, "index_stop", to_string(queryCondition.start + queryCondition.count - 1));
	if (queryCondition.freq > 0)
	{   //如果freq为0的话，说明定位任务是从菜单项中启发的，不是基于某个测试目标
		addQueryParam(query, "start_freq", to_string(queryCondition.freq - FREQ_STEP));
		addQueryParam(query, "stop_freq", to_string(queryCondition.freq + FREQ_STEP));
	}
	tdRequest.set_method(RequestMethod::M_GET);
	std::unique_ptr<ClientReader<DFTaskDataResponse>> reader(m_dbClient->accessTaskData(&context, tdRequest));
	DFTaskData taskData;
	DFTaskDataResponse tdResponse;
	while (reader->Read(&tdResponse))
	{
		auto nodedevice = tdResponse.body().data_source().node_device();
		taskData[DATACONVERT::dataSourceId(queryCondition.taskId,nodedevice)].push_back(tdResponse.body());
	};
	return taskData;
}

//定位任务数据写进数据库
bool RadarDBAccessor::writePositionTaskData(const PositionTargetItem& targetItem)
{
	PositionTargetRequest ptRequest;
	ClientContext context;
	ptRequest.set_method(RequestMethod::M_POST);
	*ptRequest.add_body()= targetItem;
	std::unique_ptr<ClientReader<PositionTargetResponse>> reader(m_dbClient->accessTarget(&context, ptRequest));
	PositionTargetResponse ptResponse;
	while (reader->Read(&ptResponse))
	{
	}
	return true;
}

//判断测向数据记录数量是否充足
bool RadarDBAccessor::isRecordEnough(const QueryCondition& queryCondition)
{
	int enoughDataSourceCount = 0;
	auto dataSource = queryRecordCount(queryCondition);
	if (dataSource.size() == 0)
		return false;
	for (auto iter = dataSource.begin(); iter != dataSource.end(); iter++)
	{
		if (iter->second >= queryCondition.start + queryCondition.count)
			enoughDataSourceCount++;
	}
	if (dataSource.size() == 1)
		return enoughDataSourceCount > 0;
	else
		return enoughDataSourceCount > 1;
}

//查询数据库中测向数据记录数量
DataSourceTag RadarDBAccessor::queryRecordCount(const QueryCondition& queryCondition)
{
	RecordCountRequest rcRequest;
	ClientContext context;
	auto query = rcRequest.mutable_query();
	addQueryParam(query, "taskId", to_string(queryCondition.taskId));
	auto nd = queryCondition.node_device;
	if (nd.has_node_id())
		addQueryParam(query, "nodeId", to_string(nd.node_id().value()));
	if (nd.has_device_id())
		addQueryParam(query, "deviceId", to_string(nd.device_id().value()));
	addQueryParam(query, "resource", "TaskData");
	if (queryCondition.freq > 0)
	{   //如果freq为0的话，说明定位任务是从菜单项中启发的，不是基于某个测试目标
		addQueryParam(query, "start_freq", to_string(queryCondition.freq - FREQ_STEP));
		addQueryParam(query, "stop_freq", to_string(queryCondition.freq + FREQ_STEP));
	}
	RecordCountResponse rcResponse;
	m_dbClient->queryRecordCount(&context, rcRequest, &rcResponse);
	DataSourceTag dataSource;
	for (auto ct : rcResponse.record_count())
	{
		dataSource[DATACONVERT::dataSourceId(queryCondition.taskId,ct.data_source().node_device())] = ct.count();
	}
	return dataSource;
}

//添加查询参数
void RadarDBAccessor::addQueryParam(QueryRepeatedPtr* query,string key, string value)
{
	auto param = query->Add();
	param->set_key(key);
	param->set_value(value);
}

bool RadarDBAccessor::writePositionTaskInfo(PositionTaskTag& taskTag)
{
	ClientContext context;
	PositionTaskInfoRequest dbRequest;
	dbRequest.set_method(RequestMethod::M_POST);
	*dbRequest.mutable_body() = DATACONVERT::PositionTaskTagToPositionTaskInfo(taskTag);
	std::unique_ptr<ClientReader<PositionTaskInfoResponse>> reader(m_dbClient->accessPositionTaskInfo(&context, dbRequest));
	PositionTaskInfoResponse tiResponse;
	while (reader->Read(&tiResponse))
	{
	};
	auto status = reader->Finish();
	if (status.error_code() == Status::OK.error_code())
	{
		return tiResponse.status_code() == 0;
	}
	else
	{
		return false;
	}
}

PositionTaskInfo RadarDBAccessor::readPositionTaskInfo()
{
	PositionTaskInfoRequest tiRequest;
	ClientContext context;
	tiRequest.set_method(RequestMethod::M_GET);
	std::unique_ptr<ClientReader<PositionTaskInfoResponse>> reader(m_dbClient->accessPositionTaskInfo(&context, tiRequest));
	PositionTaskInfo taskInfo;
	PositionTaskInfoResponse tiResponse;
	while (reader->Read(&tiResponse))
	{
		taskInfo[tiResponse.mutable_body()->task_id()] = tiResponse.body();
	};
	return taskInfo;
}

PositionTaskData RadarDBAccessor::readPositionTaskData(const uint32_t& taskid)
{
	PositionTargetRequest tdRequest;
	ClientContext context;
	auto query = tdRequest.mutable_query();
	addQueryParam(query, "taskId", to_string(taskid));
	tdRequest.set_method(RequestMethod::M_GET);
	std::unique_ptr<ClientReader<PositionTargetResponse>> reader(m_dbClient->accessTarget(&context, tdRequest));
	PositionTaskData taskData;
	PositionTargetResponse tdResponse;
	while (reader->Read(&tdResponse))
	{
		taskData[tdResponse.body().task_id()]=tdResponse.body();
	};
	return taskData;
}