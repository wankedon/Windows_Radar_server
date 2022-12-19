#pragma once
#include "../gRPCServer.h"

class RadarTaskCreator;
class RadarGRPCServer : public GRPCServer
{
public:
	RadarGRPCServer(const IPv4Address& addr, const std::string& serverPath,const ServerConfig& conf);
	virtual ~RadarGRPCServer();
private:
	void launchServer(InteractiveTaskManager& itm, LocalTaskManager& ltm) override;
	IPv4Address m_dbAddress;
};

