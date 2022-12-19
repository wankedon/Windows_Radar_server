#pragma once
#include "node/radar/radarDF.pb.h"
#include "../TaskCreator.h"

class InteractiveTask;
class ServerTask;

class RadarTaskCreator : public TaskCreator
{
public:
	RadarTaskCreator(const IPv4Address& dbServerAddress);
	~RadarTaskCreator();

public:
	std::shared_ptr<InteractiveTask> createRadarDirectTask(const radarDF::CreateTaskRequest* request);
	std::shared_ptr<ServerTask> createRadarPositionTask(const radarDF::StartPositionRequest* request);

private:
	class Impl;
	std::unique_ptr<Impl> impl;
};

