/**
 * @file AlgorithmCaller.h
 * @brief 调用定位算法
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once

enum class PositionPolicy
{
	DoubleStation
};

class PositionAlgorithm;
class AlgorithmCaller
{
public:
	AlgorithmCaller(PositionPolicy policy);
	~AlgorithmCaller();

public:
	void changePolicy(PositionPolicy policy);
	void inputObserveData(PosSolver::ObserveData* stationData);
	PosSolver::PS_ERROR getResult(PosSolver::TargetInform* target);

private:
	std::shared_ptr<PositionAlgorithm> m_algorithm;
};