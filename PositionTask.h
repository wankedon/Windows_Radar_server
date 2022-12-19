/**
 * @file PositionTask.h
 * @brief ����Դ��λ�������
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
 *
 */
#pragma once
#include "../ServerTask.h"
#include "ThreadWrap.h"
#include "../ResultBuffer.h"
#include "CommonDef.h"

enum PosType
{
	OnLine,
	OffLine
};

class PositionModule;
class RadarDBAccessor;
class MultiStationTargetMatch;
class PositionTask : public ServerTask
{
public:
	PositionTask(const PosType& posType, const IPv4Address& radar_db_address,const std::string& describe);
	virtual ~PositionTask();

public:
	virtual std::unique_ptr<TaskAccount> start() override;
	virtual void abort() override;
	std::pair<bool, std::unique_ptr<std::list<radarDF::PositionResult>>> getBuffer(uint32_t token);
	bool inputDFTaskData(const DFTaskData& dfData);
	void getPFTaskResult();
	TaskAccount getTaskAccount() { return taskAccount; }

protected:
	virtual size_t bufferedItemCount() const override;
	virtual void taskLoop() = 0;

private:
	void fillPositionTaskTag(const DFTaskData& dfData);
	bool isAborted();

protected:
	std::atomic_bool m_runFlag;

private:
	PosType m_posType;
	ThreadWrap m_thread;
	DFTaskData m_dfData;
	PositionTaskTag m_taskTag;
	ResultBuffer<radarDF::PositionResult> m_resultBuffer;
	std::unique_ptr<PositionModule> m_positionModule;
	std::unique_ptr<RadarDBAccessor> m_dbAccessor;
	std::unique_ptr<MultiStationTargetMatch> m_multiMatch;
};