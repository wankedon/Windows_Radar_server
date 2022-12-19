/**
 * @file AlgorithmCaller.h
 * @brief ���ö�λ�㷨
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
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