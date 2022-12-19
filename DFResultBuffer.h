/**
 * @file DFResultBuffer.h
 * @brief ����Դ������������
 * @author װ����ҵ������� ���˶�
 * @version 0.1
 * @date 2021-12-21
 *
 * @copyright Copyright (c) 2021  �й����ӿƼ����Ź�˾����ʮһ�о���
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