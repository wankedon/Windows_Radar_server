/**
 * @file AlgorithmCaller.cpp
 * @brief ���ö�λ�㷨
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
 *
 */
#include "../pch.h"
#include "PositionAlgorithm.h"
#include "AlgorithmCaller.h"

using namespace PosSolver;

AlgorithmCaller::AlgorithmCaller(PositionPolicy policy)
{
	changePolicy(policy);
}

AlgorithmCaller::~AlgorithmCaller()
{

}

void AlgorithmCaller::changePolicy(PositionPolicy policy)
{
	switch (policy)
	{
	case PositionPolicy::DoubleStation:
		m_algorithm = std::make_shared<CrossPosition>();
		break;
	default:
		break;
	}
}

void AlgorithmCaller::inputObserveData(ObserveData* stationData)
{
	m_algorithm->inputData(stationData);
}

PS_ERROR AlgorithmCaller::getResult(TargetInform* target)
{
	return m_algorithm->getResult(target);
}