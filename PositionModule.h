/**
 * @file PositionModule.h
 * @brief ����Դ��λģ��
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
 *
 */
#pragma once
#include "PositionSolver.h"
#include "AlgorithmCaller.h"
#include "CommonDef.h"

class AlgorithmCaller;
class PositionModule
{
public:
	PositionModule();
	~PositionModule() = default;

public:
	void inputDFTaskData(const DFTaskData& dfData);
	std::vector<PosSolver::TargetDescription> getTargetDescription();
	DataSourceTag getStationId() { return m_stationIds; }

private:
	void inputItemToAlgorithm(const DFItemList& dfDataList, const uint32_t& stationId);

private:
	DataSourceTag m_stationIds;
	std::unique_ptr<AlgorithmCaller> m_algorithmCaller;
};