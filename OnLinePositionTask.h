/**
 * @file OnLinePositionTask.h
 * @brief 辐射源在线定位任务，派生自PositionTask基类
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "PositionTask.h"
#include "CommonDef.h"

class OnLinePositionTask : public PositionTask
{
public:
	OnLinePositionTask(const radarDF::StartPositionRequest* request, const IPv4Address& radar_db_address);
	~OnLinePositionTask() = default;

public:
	virtual void taskLoop() override; 
	void inputDFTaskData(const DFTaskData& dfDataItem);

private:
	DFTaskData filterDFTaskData(DFTaskData& rawDFTaskData);
	DFTaskData filterDFTaskDataByFreq(DFTaskData& rawDFTaskData);

private:
	radarDF::PositionDataSource m_dataSource;
	double m_posFreq;
	static const uint32_t FREQ_STEP = 200;

};