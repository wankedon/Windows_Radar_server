/**
 * @file positionDataSource.h
 * @brief ����Դ��λ��������Դ
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-11-19
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
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