/**
 * @file PositionModule.cpp
 * @brief 辐射源定位模块
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#include "../pch.h"
#include "PositionModule.h"
#include "toolsFunc.h"
#include "Logger.h"

using namespace std;
using namespace radarDF;
using namespace PosSolver;

PositionModule::PositionModule()
	:m_algorithmCaller(make_unique<AlgorithmCaller>(PositionPolicy::DoubleStation))
{

}

void PositionModule::inputDFTaskData(const DFTaskData& dfData)
{
	for (auto iter = dfData.begin(); iter != dfData.end(); iter++)
	{
		if (m_stationIds.find(iter->first) == m_stationIds.end())
		{
			m_stationIds[iter->first] = m_stationIds.size();
		}
		inputItemToAlgorithm(iter->second, m_stationIds[iter->first]);
		//CLOG("Input Station {} 's {} DFTask Data to Position Algorithm", iter->first.did(), iter->second.size());
	}
}

void PositionModule::inputItemToAlgorithm(const DFItemList& dfDataList, const uint32_t& stationId)
{
	for (auto& item : dfDataList)
	{
		auto stationData = DATACONVERT::dfTaskDataItemToObserveData(item, stationId);
		if (stationData.doa_azi != -1 && stationData.doa_ele != -1)
			m_algorithmCaller->inputObserveData(&stationData);
	}
}

vector<TargetDescription> PositionModule::getTargetDescription()
{
	vector<TargetDescription> result(PARAM_LENGTH);
	auto targetInfo = TargetInform{ uint32_t(result.size()), result.data() };
	if (m_algorithmCaller->getResult(&targetInfo) == NONE_ERROR)
	{
		result = { result.begin(), result.begin() + targetInfo.targetNum };
	}
	else
	{
		result.clear();
	}
	return result;
}