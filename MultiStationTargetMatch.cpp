/**
 * @file MultiStationMatch.cpp
 * @brief 辐射源多站数据融合
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2022-02-21
 *
 * @copyright Copyright (c) 2022  中国电子科技集团公司第四十一研究所
 *
 */
#include "../pch.h"
#include "MultiStationTargetMatch.h"
#include "Logger.h"

using namespace std;
using namespace radarDF;

MultiStationTargetMatch::MultiStationTargetMatch()
{

}

MultiStationTargetMatch::~MultiStationTargetMatch()
{

}

DFTaskData MultiStationTargetMatch::multiMatch(const DFTaskData& dfData)
{
	DFTaskData result;
	vector<TaskMsgId> dataSource;
	if (newMatch(dataSource, dfData) >= 0)
	{
		result[dataSource[0]] = m_matchResult.first;
		result[dataSource[1]] = m_matchResult.second;
	}
	else
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		CLOG("--multiStation Match Failed");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
	clear();
	m_matchResult.first.clear();
	m_matchResult.second.clear();
	return result;
}

int MultiStationTargetMatch::newMatch(vector<TaskMsgId>& dataSource, const DFTaskData& dfData)
{
	dataSource = inputStationData(dfData);
	if (dataSource.size() == 0)
		return -1;
	int st1DataIndex = 0;
	auto bPulseSample = generalStationBigPulseSample();
	for (int st2DataIndex = 0; st2DataIndex < m_stData.second.size(); st2DataIndex++)
	{
		auto norMatrix = createNorMatrix(bPulseSample.first, bPulseSample.second[st2DataIndex]);
		vector<float> rf_std;
		vector<int> multiMaxIndex;
		if (!calculateMaxVal(rf_std, multiMaxIndex, norMatrix))
			return -1;
		st1DataIndex = getSt1DataIndex(multiMaxIndex, rf_std);
		//更新站2 id的目标num
		auto st2TargetId = m_stData.second[st2DataIndex].result_item().match_result().target_id();
		auto st2Num = m_stData.second[st2DataIndex].result_item().match_result().fuse_target_num();
		updateSt2Pairs(st2TargetId, st2Num);
		auto st1TargetId = m_stData.first[st1DataIndex].result_item().match_result().target_id();
		auto st1Target = matchTargets.find(st1TargetId);
		if (st1Target == matchTargets.end())
		{//目标对站点1id 不存在，插入目标对
			matchTargets.insert(std::make_pair(st1TargetId, st2TargetId));
		}
		else//目标对站点1 id存在
		{
			if (st1Target->second != st2TargetId)
			{
				if (!updateSt1Target(st1Target, st2Num, st2TargetId))
					return -1;
			}
		}
		fillMatchResult(st1DataIndex, st2DataIndex);
	}
	return 0;
}

bool MultiStationTargetMatch::calculateMaxVal(vector<float>& rf_std,vector<int>& multiMaxIndex, const MatrixNor& norMatrix)
{
	for (auto& m : norMatrix)
	{
		rf_std.push_back(m.second.carrier_freq());
	}
	auto maxVal = max_element(rf_std.begin(), rf_std.end());
	for (int i = 0; i < rf_std.size(); i++)
	{
		if (JudgeEqualofDouble(rf_std[i], *maxVal))
		{
			multiMaxIndex.push_back(i);
		}
	}
	if (*maxVal <= 0.001)
	{
		return false;
	}
	return true;
}

int MultiStationTargetMatch::getSt1DataIndex(const vector<int>& multiMaxIndex,const vector<float>& rf_std)
{
	int index = 0;
	if (multiMaxIndex.size() > 1)
	{//多于一个最大值，取num最大的那个索引	
		int maxNums = 0;
		index = -1;
		for (int i = 0; i < multiMaxIndex.size(); i++)
		{
			auto fusenum = m_stData.first[multiMaxIndex[i]].result_item().match_result().fuse_target_num();
			if (fusenum > maxNums)
			{
				maxNums = fusenum;
				index = multiMaxIndex[i];
			}
		}
	}
	else
	{
		index = max_element(rf_std.begin(), rf_std.end()) - rf_std.begin();
	}
	return index;
}

void MultiStationTargetMatch::updateSt2Pairs(const uint32_t& st2TargetId,const int& st2Num)
{
	auto id = station2Pairs.find(st2TargetId);
	if (id == station2Pairs.end())
	{
		numsAndTime temp{st2Num,time(nullptr)};
		station2Pairs.insert(std::make_pair(st2TargetId, temp));
	}
	else
	{
		id->second.num = st2Num;
		id->second.time = time(nullptr);//记录站2该id的更新时间
	}
}

bool MultiStationTargetMatch::updateSt1Target(std::map<int, int>::iterator st1Target,const int& st2Num,const uint32_t& st2TargetId)
{
	auto id2 = station2Pairs.find(st1Target->second);//id2有值
	if (st2Num > id2->second.num)//站2当前id的num比之前存储的num大，更新站2id，增加该idNums值
	{
		st1Target->second = st2TargetId;//更换站2的配对id
		return true;
	}
	else//站2当前的id的num值没有之前存储的匹配对id的num值大，那么判断两者的最新更新时间
	{
		if (id2->second.time < (time(nullptr) - 1)) //当原有配对值的数据超过1秒未更新
		{
			st1Target->second = st2TargetId;//改变配对
			return true;
		}
		else
		{//当原有匹配对数据更新时间较新，那站2当前的id可被认定为奇异值，这一帧匹配数据弃之不要
			return false;
		}
	}
}

int MultiStationTargetMatch::match(vector<TaskMsgId>& dataSource,const DFTaskData& dfData)
{
	dataSource = inputStationData(dfData);
	if (dataSource.size() == 0)
		return -1;
	auto bPulseSample = generalStationBigPulseSample();
	vector<pair<int, int>> targetsIndex;
	for (int st2DataIndex = 0; st2DataIndex < m_stData.second.size(); st2DataIndex++)
	{
		auto st2TargetId = m_stData.second[st2DataIndex].result_item().match_result().target_id();
		auto norMatrix = constructNorMatrix(bPulseSample.first,bPulseSample.second[st2DataIndex]);
		if ((m_stData.first.size() == 1) && (m_stData.second.size() == 1))
		{
			if (stDataHasOneItem(targetsIndex, norMatrix[0]))
				break;
			else
				return -1;
		}
		auto maxminvalues = calculateMaxMinValues(norMatrix);
		if (get<0>(maxminvalues) == -1)
			return -1;
		if (get<2>(maxminvalues) <= 0.0001)
		{
			targetsIndex.push_back({ -1,st2TargetId });
			clear();
			return -1;
		}
		else
		{
			int st1DataIndex = calculateDataIndex(norMatrix, get<0>(maxminvalues), get<1>(maxminvalues));
			targetsIndex.push_back({ st1DataIndex,st2TargetId });
			fillMatchResult(st1DataIndex, st2DataIndex);
		}
	}
	clear();
	return 0;
}

std::vector<TaskMsgId> MultiStationTargetMatch::inputStationData(const DFTaskData& dfData)
{
	std::vector<TaskMsgId> dataSource;
	if (dfData.begin()->second.size() == 0
		|| dfData.rbegin()->second.size() == 0
		)
		return dataSource;
	for (auto item : dfData.begin()->second)
	{
		m_stData.first.emplace_back(item);
	}
	dataSource.emplace_back(dfData.begin()->first);
	for (auto item : dfData.rbegin()->second)
	{
		m_stData.second.emplace_back(item);
	}
	dataSource.emplace_back(dfData.rbegin()->first);
	return dataSource;
}

StationBigPulseSample MultiStationTargetMatch::generalStationBigPulseSample()
{
	StationBigPulseSample stBigPulseSample;
	for (auto& item : m_stData.first)
	{
		stBigPulseSample.first.push_back(generalBigPulseSample(item));
	}
	for (auto& item : m_stData.second)
	{
		stBigPulseSample.second.push_back(generalBigPulseSample(item));
	}
	return stBigPulseSample;
}

BigPulseSample MultiStationTargetMatch::generalBigPulseSample(const DFTaskDataItem& item)
{
	BigPulseSample bigPulseSample;
	auto pulseSample = item.result_item().match_result().pulse_sample();
	bigPulseSample.rf = pulseSample.carrier_freq();
	bigPulseSample.pri = pulseSample.repeat_period();
	bigPulseSample.pw = pulseSample.pulse_width();
	auto stdpulseSample = item.result_item().match_result().std_pulse_sample();
	bigPulseSample.rf_std = stdpulseSample.carrier_freq();
	bigPulseSample.pri_std = stdpulseSample.repeat_period();
	bigPulseSample.pw_std = stdpulseSample.pulse_width();
	return bigPulseSample;
}

PulseSampleVector MultiStationTargetMatch::constructNorMatrix(const std::vector<BigPulseSample>& station1PulseSample,const BigPulseSample& bigPulseSample)
{
	PulseSampleVector norMatrix;
	for (int i = 0; i < m_stData.first.size(); i++)
	{
		norMatrix.push_back(normal(station1PulseSample[i], bigPulseSample));
	}
	return norMatrix;
}

MatrixNor MultiStationTargetMatch::createNorMatrix(const std::vector<BigPulseSample>& st1PulseSample, const BigPulseSample& pulseSampleOfSt2)
{
	MatrixNor norMatrix;
	for (int i = 0; i < m_stData.first.size(); i++)
	{
		auto time = m_stData.first[i].result_item().match_result().occour_time();
		auto timeValue = time.seconds() * 1000 + time.nanos();
		norMatrix.push_back({ timeValue,normal(st1PulseSample[i], pulseSampleOfSt2) });
	}
	return norMatrix;
}

PulseSample MultiStationTargetMatch::normal(const BigPulseSample& st1PulseSample, const BigPulseSample& st2PulseSample)
{
	std::vector<float> matchMatrix(sizeof(BigPulseSample) / sizeof(float));
	memcpy(matchMatrix.data(), &st1PulseSample, sizeof(BigPulseSample));
	std::vector<float> relationVector(sizeof(BigPulseSample) / sizeof(float));
	memcpy(relationVector.data(), &st2PulseSample, sizeof(BigPulseSample));
	std::vector<float> threshodPrecision =
	{
		2,
		(float)(relationVector[1] * 0.05),
		(float)(relationVector[2] * 0.08)
	};
	std::vector<float> matchMatrixNor(3);
	for (int i = 0; i < 3; i++)
	{
		auto delt = matchMatrix[i] - relationVector[i];
		if (abs(delt) < threshodPrecision[i])
		{
			matchMatrixNor[i] = 1.0;
		}
		else if (matchMatrix[i] <= (relationVector[i] - threshodPrecision[i]))
		{
			matchMatrixNor[i] = exp(-pow((delt + threshodPrecision[i]), 2) / 2 / pow(THREE_MAX(matchMatrix[i + 3], relationVector[i + 3], threshodPrecision[i]), 2));
		}
		else if (matchMatrix[i] >= (relationVector[i] + threshodPrecision[i]))
		{
			matchMatrixNor[i] = exp(-pow((delt - threshodPrecision[i]), 2) / 2 / pow(THREE_MAX(matchMatrix[i + 3], relationVector[i + 3], threshodPrecision[i]), 2));
		}
	}
	PulseSample stdData;
	stdData.set_carrier_freq(matchMatrixNor[0]);
	stdData.set_repeat_period(matchMatrixNor[1]);
	stdData.set_pulse_width(matchMatrixNor[2]);
	return stdData;
}

bool MultiStationTargetMatch::stDataHasOneItem(vector<pair<int, int>>& targetsIndex, const PulseSample& pulseSample)
{
	if (pulseSample.carrier_freq() <= 0.0001)
	{
		clear();
		return false;
	}
	else
	{
		auto targetId1 = m_stData.first[0].result_item().match_result().target_id();
		auto targetId2 = m_stData.second[0].result_item().match_result().target_id();
		targetsIndex.push_back({ targetId1,targetId2 });
		fillMatchResult(0, 0);
		return true;
	}
}

tuple<float,float,float> MultiStationTargetMatch::calculateMaxMinValues(PulseSampleVector& matrixnor)
{
	vector<float> maxValues(matrixnor.size());
	vector<float> minValues(matrixnor.size());
	for (int i = 0; i < matrixnor.size(); i++)
	{
		matrixnor[i].set_carrier_freq(fabs(matrixnor[i].carrier_freq() - 1.0));
		matrixnor[i].set_repeat_period(fabs(matrixnor[i].repeat_period() - 1.0));
		matrixnor[i].set_pulse_width(fabs(matrixnor[i].pulse_width() - 1.0));
		maxValues[i] = THREE_MAX(matrixnor[i].carrier_freq(), matrixnor[i].repeat_period(), matrixnor[i].pulse_width());
		minValues[i] = THREE_MIN(matrixnor[i].carrier_freq(), matrixnor[i].repeat_period(), matrixnor[i].pulse_width());
	}
	if (maxValues.size() == 0)	//只关注rf列数值的最小值 改
	{
		clear();
		return make_tuple(-1, -1, -1);
	}
	auto maxVal = max_element(maxValues.begin(), maxValues.end());
	auto minVal = min_element(minValues.begin(), minValues.end());
	auto maxFromMin = max_element(minValues.begin(), minValues.end());
	return make_tuple(*maxVal, *minVal, *maxFromMin);
}

int MultiStationTargetMatch::calculateDataIndex(const PulseSampleVector& matrixnor, const float& maxVal, const float& minVal)
{
	vector<float> p(matrixnor.size());
	auto relCoeff = calculateRelCoeff(matrixnor, maxVal, minVal);
	auto weights = calculateWeights(relCoeff);
	for (int i = 0; i < m_stData.first.size(); i++)
	{
		p[i] = weights[0] * relCoeff[i].carrier_freq() + weights[1] * relCoeff[i].repeat_period() + weights[2] * relCoeff[i].pulse_width();
	}
	return max_element(p.begin(), p.end()) - p.begin();
}

PulseSampleVector MultiStationTargetMatch::calculateRelCoeff(const PulseSampleVector& matrixnor, const float& maxVal, const float& minVal)
{
	float defCoeff = 0.5;
	vector<PulseSample> relCoeff(matrixnor.size());
	for (int i = 0; i < relCoeff.size(); i++)
	{
		relCoeff[i].set_carrier_freq((minVal + defCoeff * (maxVal)) / (matrixnor[i].carrier_freq() + defCoeff * (maxVal)));
		relCoeff[i].set_repeat_period((minVal + defCoeff * (maxVal)) / (matrixnor[i].repeat_period() + defCoeff * (maxVal)));
		relCoeff[i].set_pulse_width((minVal + defCoeff * (maxVal)) / (matrixnor[i].pulse_width() + defCoeff * (maxVal)));
	}
	return relCoeff;
}

vector<float> MultiStationTargetMatch::calculateWeights(PulseSampleVector& relCoeff)
{
	vector<float> weight(3);
	float weights[3];
	float alpha = 1.0;
	vector<float> weights_basic = { 0.8,0.1,0.1 };
	entropyWeight(relCoeff, weights, m_stData.first.size());
	for (int i = 0; i < 3; i++)
	{
		weights[i] = alpha * weights_basic[i] + (1 - alpha) * weights[i];
	}
	weights[0] = 1;//20220214 只判断rf值
	weights[1] = 0;//20220214 只判断rf值
	weights[2] = 0;//20220214 只判断rf值
	weight[0] = weights[0];
	weight[1] = weights[1];
	weight[2] = weights[2];
	return weight;
}

//熵权法求取描述矩阵权重 
void MultiStationTargetMatch::entropyWeight(PulseSampleVector& relCoeff, float weights[3], int rows)
{
	auto sumBycols = generaleSumBycols(relCoeff, rows);
	PulseSampleVector f(rows);
	for (int i = 0; i < rows; i++)
	{
		f[i].set_carrier_freq(relCoeff[i].carrier_freq() / sumBycols[0]);
		f[i].set_repeat_period(relCoeff[i].repeat_period() / sumBycols[1]);
		f[i].set_pulse_width(relCoeff[i].pulse_width() / sumBycols[2]);
	}
	auto infIj = generalInfij(f);
	vector<float> H(3);
	for (int i = 0; i < rows; i++)
	{
		H[0] = H[0] - f[i].carrier_freq() * infIj[i].carrier_freq() / log(rows);
		H[1] = H[1] - f[i].repeat_period() * infIj[i].repeat_period() / log(rows);
		H[2] = H[2] - f[i].pulse_width() * infIj[i].pulse_width() / log(rows);
	}
	float sumH = H[0] + H[1] + H[2];
	if (JudgeEqualofDouble(sumH, 3.0))
	{
		memset(weights, 0.33, sizeof(weights));
	}
	else
	{
		weights[0] = (1 - H[0]) / (3 - sumH);
		weights[1] = (1 - H[1]) / (3 - sumH);
		weights[2] = (1 - H[2]) / (3 - sumH);
	}
}

vector<float> MultiStationTargetMatch::generaleSumBycols(PulseSampleVector& relCoeff, int rows)
{
	vector<float> sumBycols(3);
	for (int i = 0; i < rows; i++)
	{
		if (JudgeEqualofDouble(relCoeff[i].carrier_freq(), 0))
		{
			relCoeff[i].set_carrier_freq(0.001);
		}
		if (JudgeEqualofDouble(relCoeff[i].repeat_period(), 0))
		{
			relCoeff[i].set_repeat_period(0.001);
		}
		if (JudgeEqualofDouble(relCoeff[i].pulse_width(), 0))
		{
			relCoeff[i].set_pulse_width(0.001);
		}
		sumBycols[0] += relCoeff[i].carrier_freq();
		sumBycols[1] += relCoeff[i].repeat_period();
		sumBycols[2] += relCoeff[i].pulse_width();
	}
	return sumBycols;
}

PulseSampleVector MultiStationTargetMatch::generalInfij(const PulseSampleVector& f)
{
	PulseSampleVector Infij(f.size());
	for (int i = 0; i < Infij.size(); i++)
	{
		if (JudgeEqualofDouble(f[i].carrier_freq(), 0))
		{
			Infij[i].set_carrier_freq(0);
		}
		else
		{
			Infij[i].set_carrier_freq(log(f[i].carrier_freq()));
		}
		if (JudgeEqualofDouble(f[i].repeat_period(), 0))
		{
			Infij[i].set_repeat_period(0);
		}
		else
		{
			Infij[i].set_repeat_period(log(f[i].repeat_period()));
		}
		if (JudgeEqualofDouble(f[i].pulse_width(), 0))
		{
			Infij[i].set_pulse_width(0);
		}
		else
		{
			Infij[i].set_pulse_width(log(f[i].pulse_width()));
		}
	}
	return Infij;
}

bool MultiStationTargetMatch::JudgeEqualofDouble(double data1, double data2)
{
	if (fabs(data1 - data2) < Epslion)
		return true;
	return false;
}

void MultiStationTargetMatch::fillMatchResult(const int& st1DataIndex, const int& st2DataIndex)
{
	auto targetId = m_stData.first[st1DataIndex].result_item().match_result().target_id();
	auto item = m_stData.second[st2DataIndex].mutable_result_item();
	item->set_target_id(targetId);
	item->mutable_match_result()->set_target_id(targetId);
	m_matchResult.first.push_back(m_stData.first[st1DataIndex]);
	m_matchResult.second.push_back(m_stData.second[st2DataIndex]);
}

void MultiStationTargetMatch::clear()
{
	m_stData.first.clear();
	m_stData.second.clear();
}