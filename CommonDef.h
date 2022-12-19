/**
 * @file DataFromConvertTools.cpp
 * @brief ���ݸ�ʽ����
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
 *
 */
#pragma once
#include <string>
#include "node/radar/radarDF.pb.h"
#include "node/radar/RadarRecLoc.pb.h"
#include "node/nodeInternal.pb.h"

//�Զ���Map����������
struct keySortFunc
{
	bool operator()(const TaskMsgId& tm1, const TaskMsgId& tm2) const
	{
		if (tm1.tid() != tm2.tid())
			return tm1.tid() < tm2.tid() ? true : false;
		if (tm1.nid() != tm2.nid())
			return tm1.nid() < tm2.nid() ? true : false;
		if (tm1.did() != tm2.did())
			return tm1.did() < tm2.did() ? true : false;
		return false;
	}
};

//�����������ݲ�ѯ����
struct QueryCondition
{
	uint32_t taskId;
	NodeDevice node_device;
	double freq;
	int start;
	int count;
};

//��������洢����
struct DFTaskTag
{
	TaskId task_id;
	std::string describe;
	uint64_t start_time;
	radarDF::UploadRadarParams params;
	NodeReply reply;
	std::map<uint32_t,Position> device_position;
	std::atomic_bool run_flag;
};

using DFTaskInfo = std::map<uint32_t, radarDF::DFTaskInformation>; //����������Ϣ <����Id,������Ϣ>
using DFItemList = std::list<radarDF::DFTaskDataItem>; //���ݿ�洢��ʽ�������ݼ���
using DFTaskData = std::map<TaskMsgId, DFItemList, keySortFunc>; //���ݿ�洢��ʽ�������� <����Դ,��������Դ������,key������>
using DFResultList = std::list<radarDF::DFResult>; //�����ʽ�������ݼ���
using DFTaskResult = std::map<TaskMsgId, DFResultList, keySortFunc>; //�����ʽ�������� <����Դ,��������Դ������,key������>
using DataSourceTag = std::map<TaskMsgId, uint32_t, keySortFunc>; //����Դ <����Դ,��������/վ��,key������>
using PositionTaskInfo = std::map<uint32_t, radarDF::PositionTaskInformation>; //��λ������Ϣ <����Id,������Ϣ>
using PositionTaskData = std::map<uint32_t, radarDF::PositionTargetItem>; //���ݿ�洢��ʽ��λ���� <����Id,��������>
using PositionDataSourceTag = std::map<TaskMsgId, Position, keySortFunc>; //����Դ <����Դ,����Դλ��,key������>
using DataTargetId = std::map<TaskMsgId, std::vector<uint32_t>, keySortFunc>; //����Դ <����Դ,Ŀ��id,key������>

//��������洢����
struct PositionTaskTag
{
	TaskId task_id;
	std::string describe;
	uint64_t start_time;
	PositionDataSourceTag data_source;
};