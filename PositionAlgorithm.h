/**
 * @file PositionAlgorithm.h
 * @brief 策略模式调用定位算法
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-11-19
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "PositionSolver.h"

class PositionAlgorithm
{
public:
	PositionAlgorithm() = default;
	~PositionAlgorithm() = default;

public:
	virtual void inputData(PosSolver::ObserveData* stationData) = 0;
	virtual PosSolver::PS_ERROR getResult(PosSolver::TargetInform* target) = 0;

protected:
	uint32_t m_algorithmHandle;
};

class CrossPosition :public PositionAlgorithm
{
public:
	CrossPosition()
	{
		PosSolver::create(&m_algorithmHandle);
	}

	~CrossPosition()
	{
		if (m_algorithmHandle != 0)
		{
			PosSolver::destroy(m_algorithmHandle);
			m_algorithmHandle = 0;
		}
	}

public:
	virtual void inputData(PosSolver::ObserveData* stationData)
	{
		PosSolver::input(m_algorithmHandle, stationData);
	}
	virtual PosSolver::PS_ERROR getResult(PosSolver::TargetInform* target)
	{
		return PosSolver::slove(m_algorithmHandle, target);
	}
};