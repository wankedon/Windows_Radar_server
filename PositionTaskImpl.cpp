/**
 * @file PositionTaskImpl.cpp
 * @brief 定位任务实现
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2022-01-10
 *
 * @copyright Copyright (c) 2022  中国电子科技集团公司第四十一研究所
 *
 */
#include "../pch.h"
#include "PositionTaskImpl.h"
#include "RadarTaskCreator.h"
#include "OnLinePositionTask.h"
#include "OffLinePositionTask.h"
#include "../LocalTaskManager.h"
#include "node/radar/radarDF.pb.h"
#include "RadarDFTask.h"

using namespace std;
using namespace radarDF;

PositionTaskImpl::PositionTaskImpl(std::shared_ptr<RadarDFTask> dfTask, std::shared_ptr<RadarTaskCreator> tc)
	:m_creator(tc),
	m_manager(make_unique<LocalTaskManager>()),
	m_dfTask(dfTask)
{
}

PositionTaskImpl::~PositionTaskImpl()
{
	stopPosition(&m_response.task_id());
}

//开始定位任务
grpc::Status PositionTaskImpl::startPosition(const StartPositionRequest* request, TaskAccount* response)
{
	m_manager->abortAll();
	auto task = m_creator->createRadarPositionTask(request);
	if (task == nullptr)
		return INVALID_PARAM;
	auto status = m_manager->startTask(task, &m_response);
	if(!status.ok())
		return status;
	if (request->has_online_param())
	{
		setDFTaskDataAcquireFunc();
		*response = generateOnLineResponse(request->online_param().data_source());
	}
	if (request->has_offline_param())
	{
		*response = m_response;
	}
	return status;
}

//设置获取测向数据函数
void PositionTaskImpl::setDFTaskDataAcquireFunc()
{
	if (m_dfTask)
		m_dfTask->setDFTaskDataAcquireFunc(
			[this](const DFTaskData& dfTaskData)
			{
				inputDFTaskDataToOnLine(dfTaskData);
			}
	);
}

//输入在线定位数据
void PositionTaskImpl::inputDFTaskDataToOnLine(const DFTaskData& dfTaskData)
{
	auto task = m_manager->getSpecificTask<OnLinePositionTask>(m_response.task_id());
	if (task)
	{
		task->inputDFTaskData(dfTaskData);
	}
}

//生成在线定位响应
TaskAccount PositionTaskImpl::generateOnLineResponse(const PositionDataSource& dataSource)
{
	auto task = m_manager->getSpecificTask<OnLinePositionTask>(m_response.task_id());
	if (task == nullptr)
		return TaskAccount();
	auto account = task->getTaskAccount();
	if (dataSource.task_runner_size() == 0)
	{//未指定定位设备，默认为所有测向设备
		auto rep = m_dfTask->getDFReply();
		for (auto header : rep.replys())
		{
			*account.add_node_devices() = header.task_runner();
		}
	}
	else
	{//指定定位设备
		*account.mutable_node_devices() = dataSource.task_runner();
	}
	return account;
}

//停止定位任务
grpc::Status PositionTaskImpl::stopPosition(const TaskId* request)
{
	if (m_dfTask)
		m_dfTask->setDFTaskDataAcquireFunc();
	return m_manager->stopTask(request);
}