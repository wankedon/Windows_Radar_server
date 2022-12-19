/**
 * @file RadarDirectTask.cpp
 * @brief 辐射源测向任务
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-11-29
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#include "../pch.h"
#include "RadarDFTask.h"
#include "toolsFunc.h"
#include "DFResultBufferCluster.h"
#include "RadarDBAccessor.h"
#include "DFTaskDataBufferCluster.h"
#include "Logger.h"

using namespace std;
using namespace radarDF;

RadarDFTask::RadarDFTask(const radarDF::CreateTaskRequest* request, const IPv4Address& radar_db_address)
	:InteractiveTask(RADAR_DF_TASK, DIRECTION_FINDING_SPATIAL_SPECTRUM, request->task_runner()),
	m_dfTaskDescribe(),
	m_dbAccessor(make_unique<RadarDBAccessor>(radar_db_address))
{
	m_dfTaskDataBuffer = make_unique<DFTaskDataBufferCluster>(
		[this](DFTaskData& dfTaskData)
		{
			onLinePosition(dfTaskData);//在线定位
		});
	m_dfResultBufferCluster = make_unique<DFResultBufferCluster>(
		[this](DFTaskResult& oneBatchResult)
		{
			acquireOneBatchResult(oneBatchResult);
		});
}

RadarDFTask::~RadarDFTask()
{

}

void RadarDFTask::acquireOneBatchResult(const DFTaskResult& oneBatchResult)
{
	//放入缓冲区
	//outputDFResult(oneBatchResult);
	//数据转换
	auto dfTaskData = DATACONVERT::DFTaskResultToDFTaskData(m_dataIndexMap, oneBatchResult);
	//存入数据库
	writeDFTaskDataToDB(dfTaskData);
#ifdef APPLY_TIME_MATCH
	//时间匹配后输入在线定位
	m_dfTaskDataBuffer->inputDFTaskData(dfTaskData);
#else
	//按条数直接输入在线定位
	onLinePosition(dfTaskData);
#endif
}

void RadarDFTask::onLinePosition(const DFTaskData& dfTaskData)
{
	if (m_inputDFTaskData == nullptr)
		return;
	if (dfTaskData.size() < 2)
	{
		LOG("-DFTaskData For OnLinePosition Not Enough");
		return;
	}
	m_inputDFTaskData(dfTaskData);
}

void RadarDFTask::outputDFResult(const DFTaskResult& dfResult)
{
	for (auto iter= dfResult.begin(); iter!= dfResult.end();iter++)
	{
		for (auto result : iter->second)
		{
			m_dfResultBuffer.input(std::move(result));
		}
	}
}

void RadarDFTask::writeDFTaskDataToDB(const DFTaskData& dfTaskData)
{
	for (auto iter = dfTaskData.begin(); iter != dfTaskData.end(); iter++)
	{
		for (auto item : iter->second)
		{
			m_tempData[iter->first].emplace_back(item);
		}
		if (m_tempData[iter->first].size() > BUFFER_SIZE_TO_DB)
		{
			auto startIndex = m_tempData[iter->first].front().data_index();
			auto endIndex = m_tempData[iter->first].back().data_index();
			m_dbAccessor->writeDFTaskData(m_tempData[iter->first]);
			CLOG("-Write Device{} DFTaskData {}-{} To DB", iter->first.did(), startIndex, endIndex);
			m_tempData[iter->first].clear();
		}
	}
}

void RadarDFTask::setDFTaskDataAcquireFunc(std::function<void(const DFTaskData& dfTaskData)> func)
{
	m_inputDFTaskData = func;
}

unique_ptr<NodeReply> RadarDFTask::uploadRadarParam(const radarDF::UploadRadarParams* param)
{
	m_dfTaskDescribe.params = *param;
	auto builder = MessageBuilder();
	builder.add(T_MODIFY).add(UPLOAD_RADAR_PARAM).serializeToTail(*param);
	CLOG("----UploadRadarParam");
	return sendToAll(builder);
}

unique_ptr<NodeReply> RadarDFTask::caliberate()
{
	auto builder = MessageBuilder();
	builder.add(T_MODIFY).add(CALIBERATE);
	CLOG("----Caliberate");
	return sendToAll(builder);
}

unique_ptr<NodeReply> RadarDFTask::timeCalib(const string& ntpip)
{
	auto builder = MessageBuilder();
	builder.add(T_MODIFY).add(TIME_CALIB).add(DATACONVERT::ipToUint(ntpip));
	CLOG("----Time_Calib");
	return sendToAll(builder);
}

unique_ptr<NodeReply> RadarDFTask::changeMatchPolicy(const radarDF::MatchPolicy& policy)
{
	auto builder = MessageBuilder();
	builder.add(T_MODIFY).add(CHANGE_MATCH_POLICY).add(policy);
	CLOG("----ChangeMatchPolicy");
	return sendToAll(builder);
}

unique_ptr<NodeReply> RadarDFTask::startDF(const radarDF::StartDFRequest* startDFRequest)
{
	auto builder = MessageBuilder();
	builder.add(T_MODIFY).add(START_DF).add(startDFRequest->interval());
	auto nodeReply = sendToAll(builder);
	m_dfTaskDescribe.reply = *nodeReply.get();
	m_dfTaskDescribe.task_id = taskID();
	m_dfTaskDescribe.describe = startDFRequest->task_describe();
	m_dfTaskDescribe.start_time = time(nullptr);
	m_dfTaskDescribe.run_flag = true;
	m_dbAccessor->writeDFTaskInfo(m_dfTaskDescribe);//任务开始信息存入数据库
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	LOG("----Start Direction Finding");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return nodeReply;
}

unique_ptr<NodeReply> RadarDFTask::stopDF()
{
	auto builder = MessageBuilder();
	builder.add(T_MODIFY).add(STOP_DF);
	auto nodeReply = sendToAll(builder);
	m_dfTaskDescribe.reply = *nodeReply.get();
	m_dfTaskDescribe.run_flag = false;
	m_dbAccessor->writeDFTaskInfo(m_dfTaskDescribe);//任务停止信息存入数据库
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	LOG("----Stop Direction Finding");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return nodeReply;
}

std::pair<bool, std::unique_ptr<DFResultList>> RadarDFTask::getBuffer(uint32_t token)
{
	updateVisitTime();
	auto result = bufferOutput(m_dfResultBuffer, token);
	result.first &= !isAborted();
	return result;
}

bool RadarDFTask::onTaskStreamMsg(const NodeDevice& from, MessageExtractor& extractor)
{
	DFResult result;
	if (!extractor.deserialize(result))
		return false;
	*(result.mutable_result_from()) = from;
	m_dfResultBufferCluster->inputDFResult(taskID(), result);
	extractDevicePosition(result);
	//m_resultSize[from.device_id().value()] += result.items_size();
	LOG("Receive {} DFResultItem From Device{}",result.items_size(), from.device_id().value());
	m_dfResultBuffer.input(std::move(result));
	return true;
}

void RadarDFTask::addTaskStartParam(MessageBuilder& builder)
{
	
}

size_t RadarDFTask::bufferedItemCount() const
{
	return m_dfResultBuffer.count();
}

//从侦察结果中提取设备位置
void RadarDFTask::extractDevicePosition(const DFResult& result)
{
	auto deviceid = result.result_from().device_id().value();
	for (auto& item : result.items())
	{
		if (m_dfTaskDescribe.device_position[deviceid].longitude() != 0)
			break;
		m_dfTaskDescribe.device_position[deviceid] = item.device_position();
	}
}