/**
 * @file DFTaskDataBuffer.h
 * @brief 辐射源定位数据缓冲区
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "CommonDef.h"
#include <functional>
#include <memory>
#include "node/radar/radarDF.grpc.pb.h"
#include "node/radar/RadarRecLoc.grpc.pb.h"
#include "node/nodeInternal.pb.h"

struct BufferResult
{
	TaskMsgId id;
	uint64_t startTime;
	std::list<radarDF::DFTaskDataItem> itemList;
};

enum BufferType
{
	Primary,  //主缓冲区
	Secondary //次缓冲区
};

using AcquireBufferResultFunc = std::function<void(BufferResult& result)>;
using AcquireStartTimeFunc = std::function<void(const uint64_t& startTime)>;

class DFTaskDataBuffer
{
public:
	DFTaskDataBuffer(const BufferType& type, uint64_t startTime,const std::pair<TaskMsgId, DFItemList>& dfTaskData, AcquireStartTimeFunc functime,AcquireBufferResultFunc func);
	~DFTaskDataBuffer();

public:
	void inputRawDFItemList(const DFItemList& itemlist);
	void addNewStartTime(const uint64_t& startTime);
	BufferType type() { return m_type; }
	uint64_t lastInputTime() { return m_lastInputTime;}

private:
	void updateStartTime(const radarDF::DFTaskDataItem& item);
	uint64_t getStartTime();
	int64_t itemOccourTime(const radarDF::DFTaskDataItem& item);
	std::unique_ptr<std::list<radarDF::DFTaskDataItem>> getCurrentBatchItems();
	bool isItemOfCurrentBatch(const radarDF::DFTaskDataItem& item);
	void outputCurrentBatchItems();
	void createBuffer(const radarDF::DFTaskDataItem& item);

private:
	mutable std::mutex lock;
	TaskMsgId m_id;
	BufferType m_type;
	uint64_t m_startTime;
	uint64_t m_lastInputTime;
	std::list<uint64_t> m_secondStartTimeList;
	std::unique_ptr<std::list<radarDF::DFTaskDataItem>> m_buffer;
	AcquireStartTimeFunc m_acquireStartTime;
	AcquireBufferResultFunc m_acquireBufferResultFunc;

private:
	const int TIME_SPAN_MS = 500;
};