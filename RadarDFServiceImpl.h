#pragma once
#include "node/radar/radarDF.grpc.pb.h"
#include "../InteractiveTaskManager.h"
#include "RadarTaskCreator.h"

enum class PositionType
{
	Unknow,
	OnLine,
	OffLine
};

class PositionTaskImpl;
class RadarDFServiceImpl : public radarDF::RadarDFService::Service
{
public:
	RadarDFServiceImpl(InteractiveTaskManager& itm, std::shared_ptr<RadarTaskCreator>& tc);
	virtual ~RadarDFServiceImpl();

public:
	::grpc::Status CreateDFTask(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::CreateTaskRequest* request, ::zb::dcts::node::TaskAccount* response) override;
	::grpc::Status UploadRadarParam(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::UploadRadarParamRequest* request, ::zb::dcts::node::NodeReply* response) override;
	::grpc::Status StartCaliberate(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::zb::dcts::node::NodeReply* response) override;
	::grpc::Status StartTimeCalib(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::StartTimeCalibRequest* request, ::zb::dcts::node::NodeReply* response) override;
	::grpc::Status StartDF(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::StartDFRequest* request, ::zb::dcts::node::NodeReply* response) override;
	::grpc::Status GetDFResult(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::grpc::ServerWriter< ::zb::dcts::node::radarDF::DFResult>* writer) override;
	::grpc::Status StopDF(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::zb::dcts::node::NodeReply* response) override;
	::grpc::Status DeleteDFTask(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::zb::dcts::node::NodeReply* response) override;
	::grpc::Status StartPosition(::grpc::ServerContext* context, const ::zb::dcts::node::radarDF::StartPositionRequest* request, ::zb::dcts::node::TaskAccount* response) override;
	::grpc::Status GetPositionResult(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::grpc::ServerWriter< ::zb::dcts::node::radarDF::PositionResult>* writer) override;
	::grpc::Status StopPosition(::grpc::ServerContext* context, const ::zb::dcts::TaskId* request, ::google::protobuf::Empty* response) override;

private:
	PositionType m_positionType;
	std::shared_ptr<RadarTaskCreator> creator;
	InteractiveTaskManager& manager;
	std::unique_ptr<PositionTaskImpl> m_positionImpl;
};