/**
 * @file RadarDirectTask.h
 * @brief 辐射源测向任务
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-11-29
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "../InteractiveTask.h"
#include "node/radar/radarDF.grpc.pb.h"
#include "node/radar/radarDFInternal.pb.h"
#include "../ResultBuffer.h"
#include "../radar/CommonDef.h"

class RadarDBAccessor;
class DFResultBufferCluster;
class DFTaskDataBufferCluster;
class RadarDFTask : public InteractiveTask
{
public:
	RadarDFTask(const radarDF::CreateTaskRequest* request, const IPv4Address& radar_db_address);
	~RadarDFTask();

public:
	std::unique_ptr<NodeReply> uploadRadarParam(const radarDF::UploadRadarParams* param);
	std::unique_ptr<NodeReply> caliberate();
	std::unique_ptr<NodeReply> timeCalib(const std::string& ntpip);
	std::unique_ptr<NodeReply> changeMatchPolicy(const radarDF::MatchPolicy& policy);
	std::unique_ptr<NodeReply> startDF(const radarDF::StartDFRequest* startDFRequest);
	std::unique_ptr<NodeReply> stopDF();
	std::pair<bool, std::unique_ptr<DFResultList>> RadarDFTask::getBuffer(uint32_t token);
	void setDFTaskDataAcquireFunc(std::function<void(const DFTaskData& dfTaskData)> func = nullptr);
	NodeReply getDFReply() { return m_dfTaskDescribe.reply; };

protected:
	bool onTaskStreamMsg(const NodeDevice& from, MessageExtractor& extractor) override;
	void addTaskStartParam(MessageBuilder& builder) override;
	size_t bufferedItemCount() const override;

private:
	void acquireOneBatchResult(const DFTaskResult& oneBatchResult);
	void extractDevicePosition(const radarDF::DFResult& result);
	void outputDFResult(const DFTaskResult& oneBatchResult);
	void writeDFTaskDataToDB(const DFTaskData& dfTaskData);
	void onLinePosition(const DFTaskData& dfTaskData);

private:
	DataSourceTag m_dataIndexMap;
	DFTaskTag m_dfTaskDescribe;
	ResultBuffer<radarDF::DFResult> m_dfResultBuffer;
	DFTaskData m_tempData;
	//std::map<uint32_t, uint32_t> m_resultSize;
	std::unique_ptr<RadarDBAccessor> m_dbAccessor;
	std::unique_ptr<DFResultBufferCluster> m_dfResultBufferCluster;
	std::function<void(const DFTaskData& dfTaskData)> m_inputDFTaskData;
	std::unique_ptr<DFTaskDataBufferCluster> m_dfTaskDataBuffer;

private:
	const int BUFFER_SIZE_TO_DB = 20;
};