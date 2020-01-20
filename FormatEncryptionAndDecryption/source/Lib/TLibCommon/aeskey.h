#pragma once
#include "TLibCommon\TypeDef.h"
#include <openssl\aes.h>
#include <iostream>
#include <stdio.h>
#include <string>
class EncAndDec
{
public:
	EncAndDec();
	~EncAndDec();
	Void Creat(Int enc);
	Bool getBitKey(UInt &key);
	Void GenerateEncIV();
	Void EncOneBit(UInt &bit);
	Void GenerateDecIV();
	Void DecOneBit(UInt &bit);
	Void Reset();
private:
	Bool   m_fisrtset;
	Int      m_operate;
	AES_KEY m_userEncKey;
	AES_KEY m_userDecKey;
	UChar m_userKey[AES_BLOCK_SIZE];
	UChar m_initialIV[AES_BLOCK_SIZE];
	UChar m_IV[AES_BLOCK_SIZE];
	UChar m_EIV[AES_BLOCK_SIZE];
	UInt m_BitNum;
};
