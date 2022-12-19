/**
 * @file gRPCServer_radar.cpp
 * @brief radargRPC服务封装
 * @author 装备事业部软件组 王克东 
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022  中国电子科技集团公司第四十一研究所
 * 
 */

#include "../pch.h"
#include "../gRPCServer.h"
#include "Logger.h"
#include "../InteractiveTaskManager.h"

using namespace std;

GRPCServer::GRPCServer(const IPv4Address& addr, const std::string& path)
	:address(addr),
	serverPath(path)
{
	
}

GRPCServer::~GRPCServer()
{
	stop();
}

void GRPCServer::start(InteractiveTaskManager& itm, LocalTaskManager& ltm)
{
	if (serverFuture.valid())
		return;
	if (RPCserver != nullptr)
		return;
	serverFuture = async([this, &itm, &ltm] {launchServer(itm, ltm); });
}

void GRPCServer::stop()
{
	if (!serverFuture.valid())
		return;
	if (RPCserver == nullptr)
		return;
	RPCserver->Shutdown();			//停止gRPC服务
	serverFuture.get();				//等待线程退出
	RPCserver.reset();
}

#include "RadarGRPCServer.h"
std::shared_ptr<GRPCServer> createGRPCServer(const std::string& serverName, const ServerConfig& conf, const std::string& exePath)
{
	auto iter = conf.services().find(serverName);
	if (iter != conf.services().end())
	{
		if(iter->first == "RADAR_DF_API")
		{
			return make_shared<RadarGRPCServer>(iter->second, exePath, conf);
		}
		//各装备负责人在此添加服务创建的操作
	}
	return nullptr;
}