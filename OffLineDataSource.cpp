/**
 * @file positionDataSource.cpp
 * @brief ����Դ��λ��������Դ
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-11-19
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
 *
 */
#include "../pch.h"
#include "OffLineDataSource.h"
#include "RadarDBAccessor.h"

OffLineDataSource::OffLineDataSource(uint32_t taskId, NodeDevice nodeDevice,double freq,const IPv4Address& radar_db_address)
	:m_taskId(taskId),
	m_nodeDevice(nodeDevice),
	m_freq(freq),
	m_dbAccessor(std::make_shared<RadarDBAccessor>(radar_db_address))
{

}

std::vector<NodeDevice> OffLineDataSource::getNodeDevice()
{
	std::vector<NodeDevice> nodeDevice;
	auto taskinfo = m_dbAccessor->readDFTaskInfo();
	if (taskinfo.size())
	{
		for (auto dataSource:taskinfo[m_taskId].data_source())
		{
			nodeDevice.emplace_back(dataSource.node_device());
		}
	}
	return nodeDevice;
}

DFTaskData OffLineDataSource::getTaskData(int start, int count)
{
	if (!isRecordEnough(start, count))
		return DFTaskData();
	return m_dbAccessor->readDFTaskData(QueryCondition{ m_taskId ,m_nodeDevice,m_freq, start, count });
}

bool OffLineDataSource::isRecordEnough(int start, int count) const
{
	return m_dbAccessor->isRecordEnough(QueryCondition{ m_taskId ,m_nodeDevice,m_freq, start, count });
}

