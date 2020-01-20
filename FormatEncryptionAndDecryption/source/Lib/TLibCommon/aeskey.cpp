#include "aeskey.h"
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")
EncAndDec::EncAndDec()
{
	m_operate = -1;
	m_fisrtset = false;
}
EncAndDec::~EncAndDec()
{
}
Void EncAndDec::Reset()
{
	memcpy(m_IV, m_initialIV, (sizeof(UChar)* 16));
	m_BitNum = 0;
}
Void EncAndDec::Creat(Int enc)
{
	Char input='o';
	m_operate = enc;
	printf("input userKey and Initial Vector,if use defalt set printf Y \n");
	std::cin >> input;
	if (input == 'Y'||input=='y')
	{
		UChar defalt_key[17] = "0123456789ABCDEF";
		memcpy(m_userKey, defalt_key, (sizeof(UChar)* 16));

		memset(m_initialIV, 0, (sizeof(UChar)* 16));
		memset(m_IV, 0, (sizeof(UChar)* 16));
	}
	else
	{
		printf("input your key and InitializeVector\n");
		std::cin >> m_userKey;
		std::cin >> m_initialIV;
		memcpy(m_IV, m_initialIV, (sizeof(UChar)* 16));
	}
	m_BitNum = 0;
	if (AES_ENCRYPT == enc)
	{
		if ((AES_set_encrypt_key(m_userKey, 128, &m_userEncKey)) < 0)
		{
			printf("false %s", __FUNCTION__);
			system("pause");
		}
		return;
	}
	else if (AES_DECRYPT == enc)
	{
		AES_set_decrypt_key(m_userKey, 128, &m_userDecKey);
		AES_set_encrypt_key(m_userKey, 128, &m_userEncKey);
		//AES_encrypt(m_IV, m_EIV, &m_userEncKey);
		//AES_encrypt(m_EIV, m_IV, &m_userEncKey);
		//memcpy(m_EIV, m_IV,(sizeof(UChar)*AES_BLOCK_SIZE));
		return;
	}
	else if (enc != AES_ENCRYPT&&enc != AES_DECRYPT)
		printf("error input %s",__FUNCTION__);
}
Void EncAndDec::GenerateEncIV()
{
	AES_encrypt(m_IV, m_EIV, &m_userEncKey);
	memset(m_IV, 0, (sizeof(UChar)* 16));
}
Void EncAndDec::GenerateDecIV()
{
	//AES_decrypt(m_IV, m_EIV, &m_userDecKey);
	AES_encrypt(m_IV, m_EIV, &m_userEncKey);
	memset(m_IV, 0, (sizeof(UChar)* 16));
}
Bool EncAndDec::getBitKey(UInt &key)
{
	key = 0;
	if (m_BitNum > 127 || m_BitNum < 0)
	{
		printf("error %s",__FUNCTION__);
		return false;
	}
	key = (m_EIV[m_BitNum / 8] >> (7 - m_BitNum % 8)) & 0x01;
	return true;
}
Void EncAndDec::EncOneBit(UInt &bit)
{
	UInt BitKey;
	Bool Result;
	UChar tmp_elem;
	if (bit > 1)
	{
		printf(" only one bit required %s\n",__FUNCTION__);
		system("pause");
	}
	if (m_BitNum == 0)GenerateEncIV();
	if (m_BitNum <= 127)
	{
		
		Result = getBitKey(BitKey);
		//std::cout << "enc bitkey " << BitKey << std::endl;
		if (Result == false || BitKey > 1)
		{
			printf("%s sth wrong\n", __FUNCTION__);
			system("pause");
		}
		else bit = bit^BitKey;
		tmp_elem = m_IV[m_BitNum / 8];
		tmp_elem = tmp_elem&((0x01 << (m_BitNum % 8)) ^ 0xff);
		m_IV[m_BitNum / 8] = tmp_elem ^ (bit << (7 - m_BitNum % 8));
		m_BitNum++;
		if (m_BitNum == 128)m_BitNum = 0;
	}
	else
	{
		printf("%s sth wrong\n",__FUNCTION__);
		system("pause");
	}
}
Void EncAndDec::DecOneBit(UInt &bit)
{
	UInt BitKey;
	Bool Result;
	UChar tmp_elem;
	if (bit > 1)
	{
		printf(" only one bit required %s\n", __FUNCTION__);
		system("pause");
	}
	if (m_BitNum == 0)GenerateDecIV();
	if (m_BitNum <= 127)
	{
		Result = getBitKey(BitKey);
		tmp_elem = m_IV[m_BitNum / 8];
		tmp_elem = tmp_elem&((0x01 << (m_BitNum % 8)) ^ 0xff);
		m_IV[m_BitNum / 8] = tmp_elem ^ (bit << (7 - m_BitNum % 8));
		if (Result == false || BitKey > 1)
		{
			printf("%s sth wrong\n", __FUNCTION__);
			system("pause");
		}
		else bit = bit^BitKey;
		
		m_BitNum++;
		if (m_BitNum == 128)m_BitNum = 0;
	}
	else
	{
		printf("%s sth wrong\n", __FUNCTION__);
		system("pause");
	}
}