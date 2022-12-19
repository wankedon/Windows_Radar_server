/**
 * @file gRPCServer_radar.cpp
 * @brief radargRPC�����װ
 * @author װ����ҵ������� ���˶� 
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022  �й����ӿƼ����Ź�˾����ʮһ�о���
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
	RPCserver->Shutdown();			//ֹͣgRPC����
	serverFuture.get();				//�ȴ��߳��˳�
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
		//��װ���������ڴ���ӷ��񴴽��Ĳ���
	}
	return nullptr;
}