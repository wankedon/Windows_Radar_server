#include "../pch.h"
#include "RadarDFServiceImpl.h"
#include "RadarTaskCreator.h"
#include "RadarDFTask.h"
#include "OffLinePositionTask.h"
#include "OnLinePositionTask.h"
#include "PositionTaskImpl.h"
#include "CustomStatus.h"
#include "../StreamHelper.h"
#include "PositionTaskImpl.h"
#include "Logger.h"

using namespace std;
using namespace radarDF;

RadarDFServiceImpl::RadarDFServiceImpl(InteractiveTaskManager& itm, std::shared_ptr<RadarTaskCreator>& tc)
	:manager(itm),
	creator(tc),
	m_positionType(PositionType::Unknow)
{

}

RadarDFServiceImpl::~RadarDFServiceImpl()
{
	m_positionImpl.reset();
}

::grpc::Status RadarDFServiceImpl::CreateDFTask(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::CreateTaskRequest* request, ::zb::dcts::node::TaskAccount* response)
{
	auto task = creator->createRadarDirectTask(request);
	if (task)
	{
		CLOG("------Create DFTask");
		return manager.startTask(task, response);
	}
	else
	{
		return INVALID_PARAM;
	}
}

::grpc::Status RadarDFServiceImpl::UploadRadarParam(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::UploadRadarParamRequest* request, ::zb::dcts::node::NodeReply* response)
{
	auto handler = [request](std::shared_ptr<RadarDFTask> task) {return task->uploadRadarParam(&request->radar_params()); };
	return manager.modifyTask<RadarDFTask>(request->task_id(), response, handler);
}

::grpc::Status RadarDFServiceImpl::StartCaliberate(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::zb::dcts::node::NodeReply* response)
{
	auto handler = [request](std::shared_ptr<RadarDFTask> task) {return task->caliberate(); };
	return manager.modifyTask<RadarDFTask>(*request, response, handler);
}

::grpc::Status RadarDFServiceImpl::StartTimeCalib(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::StartTimeCalibRequest* request, ::zb::dcts::node::NodeReply* response)
{
	auto handler = [request](std::shared_ptr<RadarDFTask> task) {return task->timeCalib(request->ntp_address()); };
	return manager.modifyTask<RadarDFTask>(request->task_id(), response, handler);
}

::grpc::Status RadarDFServiceImpl::StartDF(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::StartDFRequest* request, ::zb::dcts::node::NodeReply* response)
{
	auto handler = [request](std::shared_ptr<RadarDFTask> task) {return task->startDF(request); };
	return manager.modifyTask<RadarDFTask>(request->task_id(), response, handler);
}

::grpc::Status RadarDFServiceImpl::GetDFResult(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::grpc::ServerWriter< ::zb::dcts::node::radarDF::DFResult>* writer)
{
	auto task = manager.getSpecificTask<RadarDFTask>(*request);
	StreamHelper<radarDF::DFResult> helper(context, writer);
	StreamHelper<radarDF::DFResult>::AcqFunc handler = [task](uint32_t token) {return task->getBuffer(token); };
	auto status = helper.streamOut(handler);
	if (status.error_code() == grpc::ABORTED)
	{
		manager.abortOne(*request);
	}
	return status;
}

::grpc::Status RadarDFServiceImpl::StopDF(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::zb::dcts::node::NodeReply* response)
{
	m_positionImpl.reset();
	auto handler = [request](std::shared_ptr<RadarDFTask> task) {return task->stopDF(); };
	return manager.modifyTask<RadarDFTask>(*request, response, handler);
}

::grpc::Status RadarDFServiceImpl::DeleteDFTask(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::zb::dcts::node::NodeReply* response)
{
	CLOG("------Delete DFTask");
	m_positionImpl.reset();
	return manager.stopTask(request, response);
}

::grpc::Status RadarDFServiceImpl::StartPosition(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::StartPositionRequest* request, ::zb::dcts::node::TaskAccount* response)
{
	m_positionImpl.reset();
	std::shared_ptr<RadarDFTask> dfTask = nullptr;
	if (request->has_online_param())
	{
		dfTask = manager.getSpecificTask<RadarDFTask>(request->online_param().data_source().task_id());
		if (dfTask == nullptr)
			return TASK_NOT_FOUND;
		m_positionType = PositionType::OnLine;
	}
	else if (request->has_offline_param())
	{
		m_positionType = PositionType::OffLine;
	}
	if (m_positionImpl == nullptr)
		m_positionImpl = make_unique<PositionTaskImpl>(dfTask, creator);
	return m_positionImpl->startPosition(request, response);
}

::grpc::Status RadarDFServiceImpl::GetPositionResult(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::grpc::ServerWriter< ::zb::dcts::node::radarDF::PositionResult>* writer)
{
	if (m_positionType == PositionType::OffLine)
	{
		return m_positionImpl->getPositionResult<OffLinePositionTask>(context, request, writer);
	}
	else if (m_positionType == PositionType::OnLine)
	{
		return m_positionImpl->getPositionResult<OnLinePositionTask>(context, request, writer);
	}
	else
	{
		return TASK_NOT_FOUND;
	}
}

::grpc::Status RadarDFServiceImpl::StopPosition(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::google::protobuf::Empty* response)
{
	return m_positionImpl->stopPosition(request);
}