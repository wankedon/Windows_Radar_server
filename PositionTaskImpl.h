/**
 * @file PositionTaskImpl.h
 * @brief 定位任务实现
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2022-01-10
 *
 * @copyright Copyright (c) 2022  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "node/radar/radarDF.grpc.pb.h"
#include "../LocalTaskManager.h"
#include "RadarTaskCreator.h"
#include "../StreamHelper.h"
#include "CommonDef.h"
#include "node/radar/radarDF.pb.h"

class RadarDFTask;
class LocalTaskManager;
class PositionTaskImpl
{
public:
	PositionTaskImpl(std::shared_ptr<RadarDFTask> dfTask, std::shared_ptr<RadarTaskCreator> tc);
	~PositionTaskImpl();

public:
	grpc::Status startPosition(const radarDF::StartPositionRequest* request, TaskAccount* response);
	template<class T>
	grpc::Status getPositionResult(::grpc::ServerContext* context, const TaskId* request, ::grpc::ServerWriter< ::zb::dcts::node::radarDF::PositionResult>* writer)
	{
		auto task = m_manager->getSpecificTask<T>(*request);
		StreamHelper<radarDF::PositionResult> helper(context, writer);
		StreamHelper<radarDF::PositionResult>::AcqFunc handler = [task](uint32_t token) {return task->getBuffer(token); };
		auto status = helper.streamOut(handler);
		if (status.error_code() == grpc::ABORTED)
		{
			m_manager->abortOne(*request);
		}
		return status;
	}
	grpc::Status stopPosition(const TaskId* request);

private:
	void setDFTaskDataAcquireFunc();
	void inputDFTaskDataToOnLine(const DFTaskData& dfTaskData);
	TaskAccount generateOnLineResponse(const radarDF::PositionDataSource& dataSource);

private:
	TaskAccount m_response;
	std::shared_ptr<RadarTaskCreator> m_creator;
	std::unique_ptr<LocalTaskManager> m_manager;
	std::shared_ptr<RadarDFTask> m_dfTask;
};