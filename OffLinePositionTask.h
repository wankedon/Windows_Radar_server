/**
 * @file OffLinePositionTask.h
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
#include "node/radar/radarDF.pb.h"

class OffLineDataSource;
class DFTaskDataBufferCluster;
class OffLinePositionTask: public PositionTask
{
public:
	OffLinePositionTask(const radarDF::StartPositionRequest* request, const IPv4Address& radar_db_address);
	~OffLinePositionTask();

public:
	virtual void taskLoop() override;

public:
	void inputDFTaskData(int& currentIdx);
	bool isDataEnough(int currentIdx) const;

private:
	void addDataSource(const radarDF::PositionDataSource& source, const double freq, const IPv4Address& radar_db_address);
	int64_t itemOccourTime(const radarDF::DFTaskDataItem& item);
	void offLinePosition(const DFTaskData& dfTaskData);


private:
	DFTaskData m_dfTaskData;
	std::vector<std::shared_ptr<OffLineDataSource>> m_dataSource;
	static const uint32_t ITEM_COUNT_PER_POSITION = 10;
	std::unique_ptr<DFTaskDataBufferCluster> m_dfTaskDataBuffer;
};