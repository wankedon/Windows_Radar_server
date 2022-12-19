/**
 * @file DFTaskDataBufferCluster.h
 * @brief ����Դ��λ���ݻ�������
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022  �й����ӿƼ����Ź�˾����ʮһ�о���
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