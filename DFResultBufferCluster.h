/**
 * @file DFResultBufferCluster.h
 * @brief ����Դ��������������
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
 *
 */
#pragma once
#include "CommonDef.h"

class DFResultBuffer;
class DFResultBufferCluster
{
public:
	DFResultBufferCluster(std::function<void(DFTaskResult& result)> acquireResult);
	~DFResultBufferCluster() = default;

public:
	void inputDFResult(const TaskId& taskId, const radarDF::DFResult& oneResult);
	
private:
	bool isWholeBatchByCount();
	bool isWholeBatchByTime();
	void eraseInActiveBuffer();
	std::pair<TaskMsgId, TaskMsgId> getMinMaxBufferId();
	void outPutOneBatchResult();

private:
	mutable std::mutex lock;
	std::map<TaskMsgId, std::shared_ptr<DFResultBuffer>, keySortFunc> buffers;
	std::function<void(DFTaskResult& result)> m_acquireResult;
	const int BUFFER_SIZE_DISTANCE = 20;
	const int ITEM_COUNT_PER_BUFFER = 2;
	const int TIME_SPAN_MS = 500;
	uint64_t m_initTime;
};