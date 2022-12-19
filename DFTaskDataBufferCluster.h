/**
 * @file DFTaskDataBufferCluster.h
 * @brief 辐射源定位数据缓冲区簇
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "CommonDef.h"
#include "DFTaskDataBuffer.h"

class DFTaskDataBuffer;
class DFTaskDataBufferCluster
{
public:
	DFTaskDataBufferCluster(std::function<void(DFTaskData& dfTaskData)> acquireDFTaskData);
	~DFTaskDataBufferCluster();

public:
	void inputDFTaskData(const DFTaskData& dfTaskData);
	
private:
	void extrxctDFTaskData(const BufferResult& result);
	void eraseInActiveBuffer();

private:
	mutable std::mutex lock;
	uint64_t m_startTime;
	std::map<TaskMsgId, std::shared_ptr<DFTaskDataBuffer>, keySortFunc> m_buffers;
	std::map<uint64_t, std::vector<std::pair<TaskMsgId,DFItemList>>> m_bufferData;
	AcquireStartTimeFunc m_acquireStartTimeFunc;
	AcquireBufferResultFunc m_acquireBufferResultFunc;
	std::function<void(DFTaskData& result)> m_acquireDFTaskDataFunc;

private:
	const int TIME_SPAN_MS = 2000;
};