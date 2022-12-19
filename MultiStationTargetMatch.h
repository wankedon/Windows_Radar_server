#pragma once
/**
 * @file MultiStationMatch.h
 * @brief 辐射源多站数据融合
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2022-02-21
 *
 * @copyright Copyright (c) 2022  中国电子科技集团公司第四十一研究所
 *
 */
#include "CommonDef.h"
#include <tuple>
#include "node/radar/radarDF.pb.h"
using namespace radarDF;

#define THREE_MAX(f1,f2,f3) (max(f1,f2)>f3?max(f1,f2):f3)
#define THREE_MIN(f1,f2,f3) (min(f1,f2)<f3?min(f1,f2):f3)
#define Epslion 1e-8

struct BigPulseSample
{
	float rf;
	float pri;
	float pw;
	float rf_std;
	float pri_std;
	float pw_std;
};

struct numsAndTime
{
	int num;
	time_t time;
};

using StationData = std::pair<std::vector<radarDF::DFTaskDataItem>, std::vector<radarDF::DFTaskDataItem>>;
using StationBigPulseSample = std::pair<std::vector<BigPulseSample>, std::vector<BigPulseSample>>;
using MultiStationMatchResult = std::pair<DFItemList, DFItemList>;
using PulseSampleVector = std::vector<radarDF::PulseSample>;
using MatrixNor = std::vector<std::pair<uint64_t, radarDF::PulseSample>>;

class MultiStationTargetMatch
{
public:
	MultiStationTargetMatch();
	~MultiStationTargetMatch();

public:
	DFTaskData multiMatch(const DFTaskData& dfData);

private:
	std::vector<TaskMsgId> inputStationData(const DFTaskData& dfData);
	int match(std::vector<TaskMsgId>& dataSource, const DFTaskData& dfData);
	int newMatch(std::vector<TaskMsgId>& dataSource, const DFTaskData& dfData);
	StationBigPulseSample generalStationBigPulseSample();
	radarDF::PulseSample normal(const BigPulseSample& st1PulseSample, const BigPulseSample& st2PulseSample);
	void entropyWeight(PulseSampleVector& relCoeff, float weights[3], int rows);
	bool JudgeEqualofDouble(double data1, double data2);
	BigPulseSample generalBigPulseSample(const radarDF::DFTaskDataItem& item);
	PulseSampleVector constructNorMatrix(const std::vector<BigPulseSample>& station1PulseSample, const BigPulseSample& bigPulseSample);
	MatrixNor createNorMatrix(const std::vector<BigPulseSample>& station1PulseSample, const BigPulseSample& bigPulseSample);
	bool stDataHasOneItem(std::vector<std::pair<int, int>>& targetsIndex, const radarDF::PulseSample& pulseSample);
	std::tuple<float, float, float> calculateMaxMinValues(PulseSampleVector& matrixnor);
	PulseSampleVector calculateRelCoeff(const PulseSampleVector& matrixnor, const float& maxVal, const float& minVal);
	std::vector<float> calculateWeights(PulseSampleVector& relCoeff);
	int calculateDataIndex(const PulseSampleVector& matrixnor, const float& maxVal, const float& minVal);
	void fillMatchResult(const int& st1DataIndex, const int& st2DataIndex);
	PulseSampleVector generalInfij(const PulseSampleVector& f);
	std::vector<float> generaleSumBycols(PulseSampleVector& relCoeff, int rows);
	void clear();

	bool calculateMaxVal(std::vector<float>& rf_std, std::vector<int>& multiMaxIndex, const MatrixNor& norMatrix);
	int getSt1DataIndex(const std::vector<int>& multiMaxIndex, const std::vector<float>& rf_std);
	void updateSt2Pairs(const uint32_t& st2TargetId, const int& st2Num);
	bool updateSt1Target(std::map<int, int>::iterator st1Target, const int& st2Num, const uint32_t& st2TargetId);

private:
	StationData m_stData;
	MultiStationMatchResult m_matchResult;
	std::map<int, int> matchTargets;//<站1ID,站2id>
	std::map<int, numsAndTime> station2Pairs;//<站2ID,站2nums和time>
};