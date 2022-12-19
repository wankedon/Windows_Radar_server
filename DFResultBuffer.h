/**
 * @file DFResultBuffer.h
 * @brief 辐射源测向结果缓冲区
 * @author 装备事业部软件组 王克东
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  中国电子科技集团公司第四十一研究所
 *
 */
#pragma once
#include "CommonDef.h"

class DFResultBuffer
{
public:	
	DFResultBuffer();
	~DFResultBuffer()=default;

public:
	void input(const radarDF::DFResult& oneItem);
	std::unique_ptr<DFResultList> output();
	size_t totalItemsCount() const;
	int inputTimes() {return m_inputTimes;}

private:
	int m_inputTimes;
	const int m_maxSize;
	mutable std::mutex lock;
	std::unique_ptr<DFResultList> buffer;
};