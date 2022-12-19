#include "../pch.h"
#include "RadarTaskCreator.h"
#include "RadarDFTask.h"
#include "OffLinePositionTask.h"
#include "OnLinePositionTask.h"

using namespace std;

class RadarTaskCreator::Impl
{
public:
	Impl(const IPv4Address& dataBaseServerAddress)
		:radar_db_address(dataBaseServerAddress)
	{
	}
	~Impl() = default;

public:
	std::shared_ptr<InteractiveTask> createRadarDirectTask(const radarDF::CreateTaskRequest* request)
	{
		return make_shared<RadarDFTask>(request, radar_db_address);
	}

	shared_ptr<ServerTask> createRadarPositionTask(const radarDF::StartPositionRequest* request)
	{
		if (request->has_offline_param())
		{
			return std::dynamic_pointer_cast<ServerTask>(make_shared<OffLinePositionTask>(request, radar_db_address));
		}
		else if (request->has_online_param())
		{
			return std::dynamic_pointer_cast<ServerTask>(make_shared<OnLinePositionTask>(request, radar_db_address));
		}
		else
		{
			return nullptr;
		}
	}

private:
	IPv4Address radar_db_address; //辐射源数据库服务端地址
};

RadarTaskCreator::RadarTaskCreator(const IPv4Address& dataBaseServerAddress)
	:TaskCreator(RADAR_DF_TASK),
	impl(make_unique<RadarTaskCreator::Impl>(dataBaseServerAddress))
{

}

RadarTaskCreator::~RadarTaskCreator() = default;

std::shared_ptr<InteractiveTask> RadarTaskCreator::createRadarDirectTask(const radarDF::CreateTaskRequest* request)
{
	return impl->createRadarDirectTask(request);
}

std::shared_ptr<ServerTask> RadarTaskCreator::createRadarPositionTask(const radarDF::StartPositionRequest* request)
{
	return impl->createRadarPositionTask(request);
}