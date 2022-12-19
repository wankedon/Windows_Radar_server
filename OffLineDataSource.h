/**
 * @file positionDataSource.h
 * @brief 辐射源定位任务数据源
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-11-19
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "CommonDef.h"

class RadarDBAccessor;
class OffLineDataSource
{
public:
	OffLineDataSource(uint32_t taskId, NodeDevice nodeDevice,double freq, const IPv4Address& radar_db_address);
	~OffLineDataSource() = default;

public:
	std::vector<NodeDevice> getNodeDevice();
	bool isRecordEnough(int start, int count) const;
	DFTaskData getTaskData(int start, int count);

private:
	double m_freq;
	uint32_t m_taskId;
	NodeDevice m_nodeDevice;
	std::shared_ptr<RadarDBAccessor> m_dbAccessor;
};