/**
 * @file OnLinePositionTask.h
 * @brief ����Դ���߶�λ����������PositionTask����
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
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