/**
 * @file PositionAlgorithm.h
 * @brief ����ģʽ���ö�λ�㷨
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-11-19
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
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