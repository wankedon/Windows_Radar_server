#include "../pch.h"
#include "../InteractiveTaskManager.h"
#include "../LocalTaskManager.h"
#include "RadarGRPCServer.h"
#include "RadarTaskCreator.h"
#include "RadarDFServiceImpl.h"
#include "logger.h"
#include <grpcpp/server_builder.h>

using namespace std;

RadarGRPCServer::RadarGRPCServer(const IPv4Address& addr, const std::string& serverPath,const ServerConfig& conf)
	:GRPCServer(addr, serverPath)
{
	auto iter = conf.services().find("RADAR_DF_DB");
	if (iter != conf.services().end())
		m_dbAddress = iter->second;
}

RadarGRPCServer::~RadarGRPCServer() = default;

void RadarGRPCServer::launchServer(InteractiveTaskManager& itm, LocalTaskManager& ltm)
{
	auto creator = make_shared<RadarTaskCreator>(m_dbAddress);
	std::string server_address(address.ip() + ":" + std::to_string(address.port()));
	RadarDFServiceImpl radarService(itm, creator);
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials())
		.RegisterService(&radarService);
	RPCserver = builder.BuildAndStart();
	LOG("radar grpc server started");
	RPCserver->Wait();
	LOG("radar grpc server has shutdown");
}