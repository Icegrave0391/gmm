#pragma once

#include "DataClass.h"
#include <cmath>
#include <cstdlib>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <string>
#include <io.h>
  
#define showParseSmallCTU 
#define TEST 0
//#define AreaNum 0
#define distance 30
#define YCOLOR 1
using namespace std;
ofstream fout;

const char *Property[5] = { "UnDealed", "BackGround", "Object", "Intra", "NotSure" };
//t; colors
Pel Colors[10][2] = { { 0, 0 }, { 0, 128 }, { 128, 0 }, { 0, 255 }, { 255, 0 }, { 128, 255 }, { 255, 128 }, { 255, 255 } };
Pel Mvd_color[4][2] = { { 80, 160 }, { 255, 140 }, { 240, 240 }, { 110, 254 } };
Pel Merge_color[5][2] = { { 128, 128 }, { 90, 90 }, { 128, 10 }, { 0, 0 } };
//t:                      BG_color/B&W   MergeWhitNotSuer/green  MovingObj/red    Intra/blue
Pel BlockColor[4][2] = { { 128, 128 }, { 60, 60 }, { 110, 254 }, { 255, 0 } };


//the following colors, the 1 - 7 should be most widely used 
//					        	0: PU level red .	 1:   CU num <16>   2:	CU num <5,10>  3: CU <2,4>		4:0 				5				6							
UChar ObjectColor[17][3] = { { 202, 85, 74 }, { 105, 212, 234 }, { 178, 171, 0 }, { 225, 0, 148 }, { 149, 43, 21 }, { 29, 255, 107 }, { 58, 94, 86 },
//7					8				9				10					11					12: ankle PU     13
{ 76, 84, 255 }, { 240, 64, 138 }, { 165, 106, 191 }, { 104, 213, 53 }, { 187, 21, 85 }, { 217, 148, 64 }, { 142, 191, 117 }, { 180, 170, 181 },
{ 255, 128, 128 }, { 0, 128, 128 } };
//Pel CtuColor[14][3] = {}

static Float ratioUpperBound = 1.4f;
static Float ratioLowerBound = 0.6f;
//////////////////////////////////////
//

Bool minORmax(Int priorVal, Int midVal, Int nextVal)
{
	if (midVal > nextVal && midVal > priorVal)
		return true;
	if (midVal < nextVal && midVal < priorVal)
		return true;
	return false;
}

Void TestIfInputNull(PuUnit* curPu)
{
	if (curPu == NULL)
	{
		printf("error input PU\n");
		return;
	}
}
/******************************************************************************************
get different position  Indexes of a Prediciton Unit :leftbottom ,lefttop, righttop,center
*******************************************************************************************/
UInt getLefTopIndex(PuUnit *curPu)
{
	TestIfInputNull(curPu);
	return g_auiZscanToRaster[curPu->getAbsPartInd()];
}

UInt getTopRigIndex(PuUnit* curPu)
{
	TestIfInputNull(curPu);
	return g_auiZscanToRaster[curPu->getAbsPartInd()] + curPu->getPuWidth()/MinCUSize - 1;
}

UInt getLefBotIndex(PuUnit *curPu)
{
	TestIfInputNull(curPu);
	return g_auiZscanToRaster[curPu->getAbsPartInd()] + (curPu->getPuHeight() / 4 - 1)*(curPu->getCtu()->getCtuWid() / 4);
}

UInt getRigBotIndex(PuUnit *curPu)
{
	UInt LefBotIdx = getLefBotIndex(curPu);
	return LefBotIdx + (curPu->getPuWidth() / 4) - 1;
}

UInt getCenterIndex(PuUnit *curPu)
{

	UInt LefTopIdx = getLefTopIndex(curPu);
	UInt PuWid = curPu->getPuWidth();
	UInt PuHig = curPu->getPuHeight();
	UInt tmpIdx;
	tmpIdx = LefTopIdx + (curPu->getCtu()->getCtuWid() / 4)*(PuHig / 8) + PuWid / 8;
	return tmpIdx;
}

/***************************************************
these function are to get neighbor PUs of current PU
these PUs are not consider z-scan order
****************************************************/
//
Void getNegBorLB(PuUnit *curPu, PuUnit *&corpu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrX() == 0 || (curPu->getPuAbsAddrY() + curPu->getPuHeight()) >= curPu->getCtu()->getPic()->getPicHig())
	{
		corpu = NULL;
		return;
	}
	else
	{
		UInt LefBottomIdx;
		CtuData* TmpCtu;
		UInt CtuWid, CtuHig;
		UInt RowNum, ColNum;
		Bool PuExit = false;
		CtuHig = curPu->getCtu()->getCtuHeight();
		CtuWid = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		LefBottomIdx = getLefBotIndex(curPu);
		if ((LefBottomIdx / ColNum == RowNum - 1)&&LefBottomIdx%ColNum==0)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - 1 + curPu->getCtu()->getPic()->getPicWid() / CtuWid);
			corpu = TmpCtu->getPu(ColNum - 1);
			PuExit = true;
			return;
		}
		if ((LefBottomIdx / ColNum == RowNum - 1) && LefBottomIdx%ColNum != 0)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() + curPu->getCtu()->getPic()->getPicWid() / CtuWid);
			corpu = TmpCtu->getPu(LefBottomIdx%ColNum - 1);
			PuExit = true;
			return;
		}
		if (LefBottomIdx / ColNum != RowNum - 1)
		{
			if (LefBottomIdx%ColNum == 0)
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - 1);
				corpu = TmpCtu->getPu(LefBottomIdx + 2 * ColNum - 1);
				PuExit = true;
			}
			else if (LefBottomIdx%ColNum != 0)
			{
				corpu = curPu->getCtu()->getPu(LefBottomIdx + ColNum - 1);
				PuExit = true;
			}
		}
		else
		{
			printf("any other condition?/n");
			int test = 0;
		}
		TestIfInputNull(corpu);
	}
}

Void getNegBorL(PuUnit* curPu, PuUnit *&LeftPu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrX() <= 0)
	{
		LeftPu = NULL;
		return;
	}
	else
	{
		UInt LefBotIdx;
		CtuData *TmpCtu;
		UInt CtuWid, CtuHig;
		UInt RowNum, ColNum;
		Bool PuExit = false;
		CtuHig = curPu->getCtu()->getCtuHeight();
		CtuWid = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		LefBotIdx = getLefBotIndex(curPu);
		if (LefBotIdx %ColNum == 0)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - 1);
			LeftPu = TmpCtu->getPu(LefBotIdx + ColNum - 1);
			PuExit = true;
		}
		else
		{
			LeftPu = curPu->getCtu()->getPu(LefBotIdx - 1);
			PuExit = true;
		}
		TestIfInputNull(LeftPu);
	}
}

Void getNegBorLT(PuUnit* curPu, PuUnit *&LefTopPu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrX() == 0 || curPu->getPuAbsAddrY() == 0)
	{
		LefTopPu = NULL;
		return;
	}
	else
	{
		UInt LefTopIdx;
		CtuData *TmpCtu;
		UInt ctuWid, ctuHig, RowNum, ColNum;
		Bool ExitPu = false;
		ctuHig = curPu->getCtu()->getCtuHeight();
		ctuWid = curPu->getCtu()->getCtuWid();
		RowNum = ctuHig / MinCUSize;
		ColNum = ctuWid / MinCUSize;
		LefTopIdx = getLefTopIndex(curPu);
		if (LefTopIdx / ColNum == 0)
		{
			if (LefTopIdx%ColNum == 0)
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - curPu->getCtu()->getPic()->getPicWid() / ctuWid - 1);
				LefTopPu = TmpCtu->getPu(ColNum*RowNum - 1);
				ExitPu = true;
			}
			else
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - curPu->getCtu()->getPic()->getPicWid() / ctuWid);
				LefTopPu = TmpCtu->getPu(ColNum*(RowNum - 1) + LefTopIdx%ColNum - 1);
				ExitPu = true;
			}
		}
		else if (LefTopIdx%ColNum == 0 && LefTopIdx / ColNum != 0)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - 1);
			LefTopPu = TmpCtu->getPu(LefTopIdx - 1);
			ExitPu = true;
		}
		else if (LefTopIdx / ColNum != 0 && LefTopIdx%ColNum != 0)
		{
			LefTopPu = curPu->getCtu()->getPu(LefTopIdx - ColNum - 1);
			ExitPu = true;
		}
		TestIfInputNull(LefTopPu);
	}
}

Void getNegBorUp(PuUnit *curPu, PuUnit *&UpPu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrY() <= 0)
	{
		UpPu = NULL;
		return;
	}
	else
	{
		UInt RigTopIdx;
		CtuData *TmpCtu;
		UInt CtuWid, CtuHig;
		UInt RowNum, ColNum;
		Bool ExitPu = false;
		CtuHig = curPu->getCtu()->getCtuHeight();
		CtuWid = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		RigTopIdx = getTopRigIndex(curPu);
		if (RigTopIdx / ColNum == 0)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - curPu->getCtu()->getPic()->getPicWid() / CtuWid);
			UpPu = TmpCtu->getPu(ColNum*(RowNum - 1) + RigTopIdx%ColNum);
			ExitPu = true;
		}
		else
		{
			UpPu = curPu->getCtu()->getPu(RigTopIdx - ColNum);
			ExitPu = true;
		}
		TestIfInputNull(UpPu);
	}
}

Void getNegRT(PuUnit *curPu, PuUnit *&RTPu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrY() == 0 || (curPu->getPuAbsAddrX() + curPu->getPuWidth()) == curPu->getCtu()->getPic()->getPicWid())
	{
		RTPu = NULL;
		return;
	}
	else
	{
		UInt RigTopIdx;
		CtuData * TmpCtu;
		UInt CtuWid, CtuHig, RowNum, ColNum;
		Bool ExitPu = false;
		CtuWid = curPu->getCtu()->getCtuHeight();
		CtuHig = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		RigTopIdx = getTopRigIndex(curPu);
		if ((RigTopIdx%ColNum == ColNum - 1)&&RigTopIdx/ColNum!=0)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() + 1);
			RTPu = TmpCtu->getPu(RigTopIdx - 2 * ColNum + 1);
		}
		if (RigTopIdx / ColNum == 0)
		{
			if (RigTopIdx == ColNum - 1)
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() + 1 - curPu->getCtu()->getPic()->getPicWid() / CtuWid);
				RTPu = TmpCtu->getPu((RowNum - 1)*ColNum);
			}
			else
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - curPu->getCtu()->getPic()->getPicWid() / CtuWid);
				RTPu = TmpCtu->getPu((RowNum - 1)*ColNum + RigTopIdx + 1);
			}
			ExitPu = true;
		}
		if ((RigTopIdx / ColNum != 0) && (RigTopIdx%ColNum!=ColNum-1))
		{
			RTPu = curPu->getCtu()->getPu(RigTopIdx - ColNum + 1);
			ExitPu = true;
		}
		TestIfInputNull(RTPu);
	}
}

Void getNegRB(PuUnit *curPu, PuUnit *&RBPu)
{
	TestIfInputNull(curPu);
	if (((curPu->getPuAbsAddrX() + curPu->getPuWidth()) == curPu->getCtu()->getPic()->getPicWid()) || ((curPu->getPuAbsAddrY() + curPu->getPuHeight()) == curPu->getCtu()->getPic()->getPicHig()))
	{
		RBPu = NULL;
		return;
	}
	else
	{
		UInt RBIdx = getRigBotIndex(curPu);
		CtuData * TmpCtu;
		UInt CtuWid, CtuHig, RowNum, ColNum;
		Bool ExitPu = false;
		CtuWid = curPu->getCtu()->getCtuHeight();
		CtuHig = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		if (RBIdx / ColNum == RowNum - 1 && RBIdx%ColNum == ColNum - 1)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() + 1 + curPu->getCtu()->getPic()->getPicWid() / CtuWid);
			RBPu = TmpCtu->getPu(0);
			ExitPu = true;
		}
		if (RBIdx / ColNum != RowNum - 1 && RBIdx%ColNum == ColNum - 1)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() + 1);
			RBPu = TmpCtu->getPu(RBIdx + 1);
			ExitPu = true;
		}
		if (RBIdx / ColNum == RowNum - 1 && RBIdx%ColNum != ColNum - 1)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() + curPu->getCtu()->getPic()->getPicWid() / CtuWid);
			RBPu = TmpCtu->getPu(RBIdx%ColNum + 1);
			ExitPu = true;
		}
		if (RBIdx / ColNum != RowNum - 1 && RBIdx%ColNum != ColNum - 1)
		{
			RBPu = curPu->getCtu()->getPu(RBIdx + ColNum + 1);
			ExitPu = true;
		}
		TestIfInputNull(RBPu);
	}
}

Void getNegR(PuUnit *curPu, PuUnit *&RPu)
{
	TestIfInputNull(curPu);
	if ((curPu->getPuAbsAddrX() + curPu->getPuWidth()) == curPu->getCtu()->getPic()->getPicWid())
	{
		RPu = NULL;
		return;
	}
	else
	{
		UInt RTIdx = getTopRigIndex(curPu);
		CtuData * TmpCtu;
		UInt CtuWid, CtuHig, RowNum, ColNum;
		Bool ExitPu = false;
		CtuWid = curPu->getCtu()->getCtuHeight();
		CtuHig = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		if (RTIdx%ColNum == ColNum - 1)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() + 1);
			RPu = TmpCtu->getPu(RTIdx - ColNum + 1);
			ExitPu = true;
		}
		if ( RTIdx%ColNum != ColNum - 1)
		{
			RPu = curPu->getCtu()->getPu(RTIdx  + 1);
			ExitPu = true;
		}
		TestIfInputNull(RPu);
	}
}

Void getNegBot(PuUnit *curPu, PuUnit *&Dpu)
{
	TestIfInputNull(curPu);
	if ((curPu->getPuAbsAddrY() + curPu->getPuHeight() )== curPu->getCtu()->getPic()->getPicHig())
	{
		Dpu = NULL;
		return;
	}
	
	else
	{
		UInt LBIdx = getLefBotIndex(curPu);
		CtuData * TmpCtu;
		UInt CtuWid, CtuHig, RowNum, ColNum;
		Bool ExitPu = false;
		CtuWid = curPu->getCtu()->getCtuHeight();
		CtuHig = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		if (LBIdx/ColNum == ColNum - 1)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() + curPu->getCtu()->getPic()->getPicWid() / CtuWid);
			Dpu = TmpCtu->getPu(LBIdx%ColNum);
			ExitPu = true;
		}
		if (LBIdx/ColNum != ColNum - 1)
		{
			Dpu = curPu->getCtu()->getPu(LBIdx + ColNum);
			ExitPu = true;
		}
		TestIfInputNull(Dpu);
	}
}
/********************************************************************
the follow functions are get neighbor PUs 
alse take z-scan order into consideration
**********************************************************************/
//get neigborhood PUs 
Void getLeftBottomPu(PuUnit *curPu, PuUnit *&LBpu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrX() == 0 || (curPu->getPuAbsAddrY() + curPu->getPuHeight()) >= curPu->getCtu()->getPic()->getPicHig())
	{
		LBpu = NULL;
		return;
	}
	else
	{
		UInt LefBottomIdx;
		CtuData* TmpCtu;
		UInt CtuWid, CtuHig;
		UInt RowNum, ColNum;
		Bool PuExit = false;
		CtuHig = curPu->getCtu()->getCtuHeight();
		CtuWid = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;

		LefBottomIdx = getLefBotIndex(curPu);
		if (LefBottomIdx / ColNum == RowNum - 1)
		{
			LBpu = NULL;
			PuExit = false;
			return;
		}
		else if (LefBottomIdx/ColNum!=RowNum-1)
		{
			if (LefBottomIdx%ColNum == 0)
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - 1);
				LBpu = TmpCtu->getPu(LefBottomIdx + 2 * ColNum - 1);
				PuExit = true;
			}
			else 
				LBpu = curPu->getCtu()->getPu(LefBottomIdx + ColNum - 1);
			PuExit = true;
			if (LBpu == NULL || LBpu->getAbsPartInd() > curPu->getAbsPartInd())
			{
				PuExit = false;
				LBpu = NULL;
			}
		}
		if (PuExit)
			TestIfInputNull(LBpu);
	}
}

Void getLeftPu(PuUnit* curPu, PuUnit *&LeftPu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrX() <= 0)
	{
		LeftPu = NULL;
		return;
	}
	else
	{
		UInt LefBotIdx;
		CtuData *TmpCtu;
		UInt CtuWid, CtuHig;
		UInt RowNum, ColNum;
		Bool PuExit = false;
		CtuHig = curPu->getCtu()->getCtuHeight();
		CtuWid = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		LefBotIdx = getLefBotIndex(curPu);
		if (LefBotIdx %ColNum == 0)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - 1);
			LeftPu = TmpCtu->getPu(LefBotIdx + ColNum - 1);
			PuExit = true;
		}
		else
		{
			LeftPu = curPu->getCtu()->getPu(LefBotIdx - 1);
			PuExit = true;
		}
		if (PuExit)TestIfInputNull(LeftPu);
	}
}

Void getLefTopPu(PuUnit* curPu, PuUnit *&LefTopPu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrX() == 0 || curPu->getPuAbsAddrY() == 0)
	{
		LefTopPu = NULL;
		return;
	}
	else
	{
		UInt LefTopIdx;
		CtuData *TmpCtu;
		UInt ctuWid, ctuHig, RowNum, ColNum;
		Bool ExitPu = false;
		ctuHig = curPu->getCtu()->getCtuHeight();
		ctuWid = curPu->getCtu()->getCtuWid();
		RowNum = ctuHig / MinCUSize;
		ColNum = ctuWid / MinCUSize;
		LefTopIdx = getLefTopIndex(curPu);
		if (LefTopIdx / ColNum == 0)
		{
			if (LefTopIdx%ColNum == 0)
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - curPu->getCtu()->getPic()->getPicWid() / ctuWid - 1);
				LefTopPu = TmpCtu->getPu(ColNum*RowNum - 1);
				ExitPu = true;
			}
			else
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - curPu->getCtu()->getPic()->getPicWid() / ctuWid);
				LefTopPu = TmpCtu->getPu(ColNum*(RowNum - 1) + LefTopIdx%ColNum - 1);
				ExitPu = true;
			}
		}
		else if (LefTopIdx%ColNum == 0 && LefTopIdx / ColNum != 0)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - 1);
			LefTopPu = TmpCtu->getPu(LefTopIdx - 1);
			ExitPu = true;
		}
		else if (LefTopIdx / ColNum != 0 && LefTopIdx%ColNum != 0)
		{
			LefTopPu = curPu->getCtu()->getPu(LefTopIdx - ColNum - 1);
			ExitPu = true;
		}
		if (ExitPu)TestIfInputNull(LefTopPu);
	}
}

Void getUpPu(PuUnit *curPu, PuUnit *&UpPu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrY() <= 0)
	{
		UpPu = NULL;
		return;
	}
	else
	{
		UInt RigTopIdx;
		CtuData *TmpCtu;
		UInt CtuWid, CtuHig;
		UInt RowNum, ColNum;
		Bool ExitPu = false;
		CtuHig = curPu->getCtu()->getCtuHeight();
		CtuWid = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		RigTopIdx = getTopRigIndex(curPu);
		if (RigTopIdx / ColNum == 0)
		{
			TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - curPu->getCtu()->getPic()->getPicWid() / CtuWid);
			UpPu = TmpCtu->getPu(ColNum*(RowNum - 1) + RigTopIdx%ColNum);
			ExitPu = true;
		}
		else
		{
			UpPu = curPu->getCtu()->getPu(RigTopIdx - ColNum);
			ExitPu = true;
		}
		if (ExitPu)TestIfInputNull(UpPu);
	}
}

Void getRigTopPu(PuUnit *curPu, PuUnit *&RTPu)
{
	TestIfInputNull(curPu);
	if (curPu->getPuAbsAddrY() == 0 || (curPu->getPuAbsAddrX() + curPu->getPuWidth()) == curPu->getCtu()->getPic()->getPicWid())
	{
		RTPu = NULL;
		return;
	}
	else
	{
		UInt RigTopIdx;
		CtuData * TmpCtu;
		UInt CtuWid, CtuHig, RowNum, ColNum;
		Bool ExitPu = false;
		CtuWid = curPu->getCtu()->getCtuHeight();
		CtuHig = curPu->getCtu()->getCtuWid();
		RowNum = CtuHig / MinCUSize;
		ColNum = CtuWid / MinCUSize;
		RigTopIdx = getTopRigIndex(curPu);
		if (RigTopIdx / ColNum == 0)
		{
			if (RigTopIdx == ColNum - 1)
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() + 1 - curPu->getCtu()->getPic()->getPicWid() / CtuWid);
				RTPu = TmpCtu->getPu((RowNum - 1)*ColNum);
			}
			else
			{
				TmpCtu = curPu->getCtu()->getPic()->getCtu(curPu->getCtu()->getCtuAddr() - curPu->getCtu()->getPic()->getPicWid() / CtuWid);
				RTPu = TmpCtu->getPu((RowNum - 1)*ColNum + RigTopIdx + 1);
			}
			ExitPu = true;
		}
		else if (RigTopIdx / ColNum != 0 && RigTopIdx%ColNum == ColNum - 1)
		{
			RTPu = NULL;
			ExitPu = false;
		}
		else if (RigTopIdx / ColNum != 0 && RigTopIdx%ColNum != ColNum - 1)
		{
			RTPu = curPu->getCtu()->getPu(RigTopIdx - ColNum + 1);
			if (curPu->getAbsPartInd() < RTPu->getAbsPartInd())
			{
				RTPu = NULL;
				ExitPu = false;
			}
			else ExitPu = true;
		}
		if (ExitPu)TestIfInputNull(RTPu);
	}
}

//get pu in corresponding positon of next frame
Void getCorLBPu(PuUnit *curPu, PuUnit *&CorLBPu, UInt PicNum)
{
	UInt PuLBIdx = getLefBotIndex(curPu);
	PicData* curPic = curPu->getCtu()->getPic();
	PicData* nextPic;
	if (PicNum == 1)	nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum());
	else if (PicNum == 2)nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum() + 1);
	if (nextPic == NULL)
		CorLBPu = NULL;
	else
		CorLBPu = nextPic->getCtu(curPu->getCtu()->getCtuAddr())->getPu(PuLBIdx);
}

Void getCorLTPu(PuUnit *curPu, PuUnit *&CorLTPu, UInt PicNum)
{
	UInt PuLTIdx = getLefTopIndex(curPu);
	PicData *curPic = curPu->getCtu()->getPic();
	PicData *nextPic;
	if (PicNum == 1)	nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum());
	else if (PicNum == 2)nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum() + 1);
	if (nextPic == NULL)
		CorLTPu = NULL;
	else CorLTPu = nextPic->getCtu(curPu->getCtu()->getCtuAddr())->getPu(PuLTIdx);
}

Void getCorRTtPu(PuUnit *curPu, PuUnit *&CorRTPu, UInt PicNum)
{
	UInt PuRTIdx = getTopRigIndex(curPu);
	PicData* curPic = curPu->getCtu()->getPic();
	PicData* nextPic;
	if (PicNum == 1)	nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum());
	else if (PicNum == 2)nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum() + 1);
	if (nextPic == NULL)
		CorRTPu = NULL;
	else
		CorRTPu = nextPic->getCtu(curPu->getCtu()->getCtuAddr())->getPu(PuRTIdx);
}

Void getCorRBPu(PuUnit *curPu, PuUnit *&CorRBPu, UInt PicNum)
{
	UInt PuRBIdx = getRigBotIndex(curPu);
	PicData* curPic = curPu->getCtu()->getPic();
	PicData* nextPic;
	if (PicNum == 1)	nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum());
	else if (PicNum == 2)nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum() + 1);
	if (nextPic == NULL)
		CorRBPu = NULL;
	else
		CorRBPu = nextPic->getCtu(curPu->getCtu()->getCtuAddr())->getPu(PuRBIdx);
}

Void getCorCentPu(PuUnit *curPu, PuUnit *&CorPu,UInt PicNum)
{
	UInt PuCenterIdx = getCenterIndex(curPu);
	PicData* curPic = curPu->getCtu()->getPic();
	PicData* nextPic;
	if (PicNum == 0)	nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum());
	else if (PicNum == 1)nextPic = curPic->getCurVideoData()->getPic(curPic->getPicNum() + 1);
	if (nextPic == NULL)
		CorPu = NULL;
	else
		CorPu = nextPic->getCtu(curPu->getCtu()->getCtuAddr())->getPu(PuCenterIdx);
}

//test if mvp exit zero
Void stepOne(PuUnit* curPu)
{
	if (curPu->getPpu()->getZeroMv() == true && curPu->getSecPredPu()->getZeroMv() == true)
	{
		if (curPu->getTmpFlag() == true)
		{
			curPu->setStatus(curPu->getTmppu()->getStatus());
			curPu->setFinal(curPu->getTmppu());
			curPu->setSortBasis(BY_REFER_TEMPERAL);
			return;
		}
		else
		{
			curPu->setStatus(STA_BG);
			curPu->setFinal(NULL);
			curPu->setSortBasis(BY_ZERO_MV);
			return;
		}
	}
	else if (curPu->getPpu()->getZeroMv() != curPu->getSecPredPu()->getZeroMv())
	{
		curPu->setStatus(curPu->getSecPredPu()->getStatus());
		curPu->setFinal(curPu->getSecPredPu());
		curPu->setSortBasis(BY_REFER_SPATIAL);
		return;
	}
	else curPu->setStatus(STA_NOTSURE);
}

///setp two
//find the same smvp ie. scale =4096
Void stepTwo(PuUnit* curPu)
{
	if (curPu->getSpaMvpDir(0) == curPu->getSpaMvpDir(1))
	{
		if (curPu->getPpu() == curPu->getSecPredPu())
		{
			curPu->setSmvpNum(1);
			if (curPu->getTmpFlag())
			{
				curPu->setStatus(curPu->getTmppu()->getStatus());
				curPu->setFinal(curPu->getTmppu());
				curPu->setSortBasis(BY_REFER_TEMPERAL);
				return;
			}
			else
			{
				curPu->setStatus(STA_BG);
				curPu->setFinal(NULL);
				curPu->setSortBasis(BY_ZERO_MV);
				return;
			}
		}
		else
		{
			printf("will this happen???\n");
			int a = 0;
		}
	}
	else return;
}

//the function should be changed
Void PreDealPu(PuUnit* curPu)
{
	Bool RefPuMergFlag1, RefPuMergeFlag2;
	UInt x, y;
	curPu->getPuAbsAddrXY(x, y);
	if (x == 144 && y == 392 && curPu->getCtu()->getPic()->getPicNum() == 18)
	{
		Int a = 1;
	}
	curPu->setFinal(NULL);
	curPu->getPpu()->getMergFlag(RefPuMergFlag1);
	curPu->getSecPredPu()->getMergFlag(RefPuMergeFlag2);
	if (RefPuMergFlag1 == RefPuMergeFlag2 && RefPuMergFlag1== true)
	{
		if ((curPu->getPpu()->getBaseMergPu() == curPu->getSecPredPu()->getBaseMergPu())&&curPu->getPpu()->getBaseMergPu()!=NULL)
		{
			if (curPu->getTmpFlag() == true)
			{
				curPu->setFinal(curPu->getTmppu());
				curPu->setStatus(curPu->getTmppu()->getStatus());
				curPu->setSortBasis(BY_REFER_TEMPERAL);
				return;
			}
			else
			{
				curPu->setStatus(STA_BG);
				curPu->setSortBasis(BY_ZERO_MV);
				return;
			}
		}
		else
		{
			curPu->setStatus(STA_NOTSURE);
			return;
		}
	}
	else if (RefPuMergFlag1 == true && RefPuMergeFlag2 == false)
	{
		if (curPu->getPpu()->getBaseMergPu() == curPu->getSecPredPu())
		{
			if (curPu->getTmpFlag() == true)
			{
				curPu->setFinal(curPu->getTmppu());
				curPu->setStatus(curPu->getTmppu()->getStatus());
				curPu->setSortBasis(BY_REFER_TEMPERAL);
				return;
			}
			else
			{
				curPu->setStatus(STA_BG);
				curPu->setFinal(NULL);
				curPu->setSortBasis(BY_ZERO_MV);
				return;
			}
		}
		else
		{
			curPu->setStatus(STA_NOTSURE);
			return;
		}
	}
	else if (RefPuMergFlag1 == false && RefPuMergeFlag2 == true)
	{
		if (curPu->getSecPredPu()->getBaseMergPu() == curPu->getPpu())
		{
			if (curPu->getTmpFlag() == true)
			{
				curPu->setFinal(curPu->getTmppu());
				curPu->setStatus(curPu->getTmppu()->getStatus());
				curPu->setSortBasis(BY_REFER_TEMPERAL);
				return;
			}
			else
			{
				curPu->setStatus(STA_BG);
				curPu->setFinal(NULL);
				curPu->setSortBasis(BY_ZERO_MV);
				return;
			}
		}
			else
			{
				curPu->setStatus(STA_NOTSURE);
				return;
			}
	}
	else
	{
		curPu->setStatus(STA_NOTSURE);
		return;
	}
}
/////////////////////////a new function 

Status TraceCurPu(PuUnit* curPu)
{
	Status FirstSta, SecSta, TmpSta;
	FirstSta = curPu->getPpu()->getStatus();
	SecSta = curPu->getSecPredPu()->getStatus();
	if (curPu->getTmpFlag() == true)
		TmpSta = curPu->getTmppu()->getStatus();
	if (FirstSta != STA_NOTSURE&&SecSta != STA_NOTSURE)
	{
		if (FirstSta != SecSta)
		{
			curPu->setStatus(SecSta);
			curPu->setFinal(curPu->getSecPredPu());
			curPu->setSortBasis(BY_REFER_SPATIAL);
			return SecSta;
		}
		else if (FirstSta == SecSta)
		{
			if (curPu->getTmpFlag() == true)
			{
				if (TmpSta == FirstSta)
				{
					curPu->setStatus(FirstSta);
					curPu->setSortBasis(BY_REFER_SPATIAL);
					curPu->setFinal(curPu->getPpu());
					return FirstSta;
				}
				else
				{
					curPu->setStatus(STA_NOTSURE);
					return STA_NOTSURE;
				}
			}
			else
			{
				if (FirstSta == STA_BG)
				{
					curPu->setStatus(STA_BG);
					return STA_BG;
				}
				else
				{
					curPu->setStatus(STA_NOTSURE);
					return STA_NOTSURE;
				}
			}
		}
	}
	return STA_NOTSURE;
}

//when mv prediction is using advanced motion prediction
/*
Void dealAmvp(PuUnit* pPu, UInt mvdx, UInt mvdy)
{
	UInt x, y;
	pPu->getPuAbsAddrXY(x, y);
	if (x == 1248 && y == 440 && pPu->getCtu()->getPic()->getPicNum() == 3)
		Int testPoint = 0;
	if (mvdx > Threshold || mvdy > Threshold)
	{
		pPu->setStatus(STA_MOVOBJ);
		pPu->setSortBasis(BY_BIG_MVD);
		if (pPu->getTmpFlag() && pPu->getSmvpNum() >= 1 && pPu->getMvPInd() == 1)
		{
			if (pPu->getSpaMvpDir(0) == MD_ABOVE || pPu->getSpaMvpDir(0) == MD_ABOVE_RIGHT || pPu->getSpaMvpDir(0) == MD_ABOVE_LEFT)
			{
				pPu->setSmvpNum(1);
				pPu->getTmppu()->addPusReferCurr(pPu);
				if ((mvdx < Threshold || mvdy < Threshold) && pPu->getTmppu()->getStatus() == STA_MOVOBJ)
				{
					pPu->setSortBasis(BY_REFER_TEMPERAL);
					pPu->setFinal(pPu->getTmppu());
				}
			}
		}
		return;
	}
	
	// mvp index is 0
	else if (pPu->getMvPInd() == 0)
	{
		if (pPu->getSmvpNum() != 0)
		{
			pPu->setSortBasis(BY_REFER_SPATIAL);
			pPu->setStatus(pPu->getPpu()->getStatus());
			pPu->setFinal(pPu->getPpu());
			return;
		}
		else if (pPu->getSmvpNum() == 0 && pPu->getTmpFlag() == true)
		{
			pPu->setSortBasis(BY_REFER_TEMPERAL);
			pPu->setStatus(pPu->getTmppu()->getStatus());
			return;
		}
		else if (pPu->getSmvpNum() == 0 && pPu->getTmpFlag() == false)
		{
			pPu->setSortBasis(BY_ZERO_MV);
			pPu->setStatus(STA_BG);
			return;
		}
	}

	//mvp index is 1
	else if (pPu->getMvPInd() == 1)
	{
		//no spatial mvp exists
		if (pPu->getSmvpNum() == 0)
		{
			if (pPu->getTmpFlag() == true)pPu->setSortBasis(BY_REFER_TEMPERAL);
			else pPu->setSortBasis(BY_ZERO_MV);
			pPu->setStatus(STA_BG);
			return;
		}
		//only on spatial mvp exist;
		else if (pPu->getSmvpNum() == 1)
		{
			if (pPu->getTmpFlag() == true)
			{
				pPu->setStatus(pPu->getTmppu()->getStatus());
				pPu->setSortBasis(BY_REFER_TEMPERAL);
				return;
			}
			else
			{
				pPu->setSortBasis(BY_ZERO_MV);
				pPu->setStatus(STA_BG);
				return;
			}
		}
		else if (pPu->getSmvpNum() == 2)
		{
			pPu->dealTwoSmvp();
		}
	}
}
*/
Void DealMergeBlock(PuUnit* ppu)
{
	UInt x, y;
	Bool MergeWithZeroMv;
	MVP_DIR cur_mvDir;
	ppu->getPuAbsAddrXY(x, y);
	cur_mvDir= ppu->getSpaMvpDir(0);
	MergeWithZeroMv = ppu->getMergeWithZeroMv();
	if (ppu->getMergIndex() == 4)
		Int testPoint = 0;
	if (MergeWithZeroMv || cur_mvDir == MD_REF_ZERO)
	{
		ppu->setStatus(STA_BG);
		ppu->setSortBasis(BY_ZERO_MV);
	}
	else
	{
		PuUnit* PredPu;
		PuUnit* BasePu;
		Status Pred_status;
		if (ppu->getTmpFlag())PredPu = ppu->getTmppu();
		else PredPu = ppu->getPpu();
		if (PredPu->getMergeWithZeroMv() == true)
		{
			ppu->setsMvpMergeFlag(true);
			ppu->setStatus(STA_BG);
			BasePu = PredPu->getBaseMergPu();
			ppu->setBaseMergPu(BasePu);
			ppu->setSortBasis(BY_ZERO_MV);
		}
		else 
		{
			if (PredPu->getMergflag())
				BasePu = PredPu->getBaseMergPu();
			else BasePu = PredPu;
			ppu->setBaseMergPu(BasePu);
			Pred_status = PredPu->getStatus();
			ppu->setStatus(Pred_status);
			if (Pred_status == STA_MOVOBJ)
			{
				if (ppu->getTmpFlag() == true)
					ppu->setSortBasis(BY_REFER_TEMPERAL);
				else
					ppu->setSortBasis(BY_REFER_SPATIAL);
			}
		}
	}
}
/**********************************************************
here is try to determin which merge candidate is the right one 
as when index is 0 or 4 ,
the upper method is not right ,as when index is 4,
you can not decide whether temporal candidate is equal to before 
so do not know whether the right MVP is zero or temporal one
there is no need to determine which one 
when index is between the scope of 1 to 3 
first of all , is try to reduce the redundant candidate number .
at this step, one should be 
************************************************************/
Void MergeCandidateDecision(PuUnit* pcPU)
{
	UInt candIdx = pcPU->getMergIndex();
	if (candIdx == 0)
	{
		if (pcPU->getDirAndPu().size() > 0)
		{
			pcPU->setMvpIsSure(true);
			pcPU->setStatus(pcPU->getPpu()->getStatus());
			pcPU->setStatusIsSure(pcPU->getStatusIsSure());
			if (pcPU->getBaseMergPu() == pcPU->getPpu())
			{
				pcPU->setMvpIsSure(true);
			}
		}
		else
		{
			pcPU->setMvpIsSure(true);
			pcPU->setZeroMv(true);
			pcPU->setStatus(STA_BG);
		}
		return;
	}
	else if (candIdx == 4)
	{
		if (pcPU->getTmpFlag())
		{
			pcPU->setMvpIsSure(true);
			pcPU->setStatus(pcPU->getTmppu()->getStatus());
			pcPU->setStatusIsSure(pcPU->getTmppu()->getStatusIsSure());
		}
		else
		{
			pcPU->setMvpIsSure(true);
			pcPU->setZeroMv(true);
			pcPU->setStatus(STA_BG);
		}
		return;
	}
	else if (candIdx >= 1 && candIdx < 4)
	{
		//decrease mvp number as far as possible 
	}
}
Void MergeCondition(PuUnit* pcPu)
{
	UInt mergeIndex = pcPu->getMergIndex();
	if (mergeIndex == 0)
	{
		pcPu->setMergeCandIsSure(true);
		if (pcPu->getPpu() != NULL)
		{
			pcPu->setStatus(pcPu->getStatus());
			pcPu->setStatusIsSure(pcPu->getPpu()->getStatusIsSure());
			return;
		}
		else
		{
			pcPu->setStatus(STA_BG);
			pcPu->setStatusIsSure(false);
			///////////////////////////////////////////for debug
			if (pcPu->getMergeWithZeroMv() == false)
			{
				printf("will this happen");
				int testpoint = 0;
			}
		}

	}
}
Void setMvdColor(PuUnit* ppu, UInt mvdx, UInt mvdy, Pel &coloru, Pel &colorv)
{
	UInt ref_colInd = 0;
	if (mvdx <= 3 && mvdy <= 3)
	{
		if (ppu->getMvPInd() == 1 && ppu->getSmvpNum() == 2 && ppu->getMvpIsSure() == false)
		{
			coloru = 0;
			colorv = 128;
		}
		else 
		{
			coloru = Mvd_color[0][0];
			colorv = Mvd_color[0][1];
		}
	}
	else if ((mvdx > 3 || mvdy > 3) && (mvdx <= 7 && mvdy <= 7))
	{
		if (ppu->getMvPInd() == 1 && ppu->getSmvpNum() == 2 && ppu->getMvpIsSure()==false)
		{
			coloru = 0;
			colorv = 128;
		}
		else
		{
			coloru = Mvd_color[1][0];
			colorv = Mvd_color[1][1];
		}
	}
	else if ((mvdx>7 || mvdy>7) && (mvdx <= 15 && mvdy <= 15)){
		coloru = Mvd_color[2][0];
		colorv = Mvd_color[2][1];
	}
	else if ((mvdx > 15 || mvdy > 15)){
		coloru = Mvd_color[3][0];
		colorv = Mvd_color[3][1];
	}
}
//PredictionUnit construct
PuUnit::PuUnit(UInt num) :m_uPartIdx(num),
m_coordx(0),
m_coordy(0),
m_PuWidth(0),
m_PuHeigh(0),
m_uScanIndex(0),
m_uMergeIndex(0),
m_uRefFrmIndex(0),
m_uAMVPIndex(0),
m_PuAddInPicX(0),
m_PuAddInPicY(0),
m_uScale(0),
m_uSmvpNum(0),
m_iHorMV(-1),
m_iVerMV(-1),
m_status(STA_UNSORTED),
m_ePreStatus(STA_UNSORTED),
m_SortBasis(NOTSORTED),
m_bTravel(false),
m_skipFlag(false),
m_mergFlag(false),
m_SmvpEqual(false),
m_bZeroCand(false),
m_bTempFlag(false),
m_bStaSettled(false),
m_bSuspectFlag(false),
m_SmvpMerged(false),
m_MvpIsSure(false),
m_bMvEqlZero(false),
m_bMvUeqlZero(false),
m_statueChanged(false),
m_MergeWihtZeroMv(false),
m_bMrgCandIsSure(true),
m_bMrgStatusIsSure(true),
m_bMrgBaseIsSure(true),
m_bA0Avail(false),
m_bB0Avail(false),
m_bB1Avail(false),
m_bA1Avail(false),
m_bB2Avail(false)
{
	m_auMvd[0] = 0;
	m_auMvd[1] = 0;
	m_sMvpDir[0] = m_sMvpDir[1] = MD_NONE;
	m_tMvpDir = MD_NONE;
	m_ePredMode = MODE_NONE;
	m_curCtu = NULL;
	m_curCb = NULL;
	m_pcRefPu = NULL;
	m_pcSecRefPu = NULL;
	m_pcTempPu = NULL;
	m_pcAmvpRefPu = NULL;
	m_pcMergBasePu = NULL;
	m_pLTPu = NULL;
	m_pLBPu = NULL;
	m_pRTPu = NULL;
	m_pRBPu = NULL;
	m_pcPuMergeCurr = NULL;
	m_vPusReferCurr.clear();
	m_mMergeCandidatePus.clear();
}

//get information from reference for current PU
//zero_mvp_flag   
Void PuUnit::getInfoFromRefPu(PuUnit* refPu, Bool tmpFlag)
{
	Bool mergeFlag;
	//candidate is not zero 
	m_bZeroCand = false;
	//not refer temporal candidate 
	m_bTempFlag = tmpFlag;
	//set base merge PU
	refPu->getMergFlag(mergeFlag);
	if (mergeFlag)
	{
		if (refPu->getMergeWithZeroMv())
		{
			m_bMvEqlZero = true;
			m_MergeWihtZeroMv = true;
		}
		else 
			m_bMvEqlZero = false;

		if (refPu->getBaseMergPu() != NULL)
			m_pcMergBasePu = refPu->getBaseMergPu();
		else 
			m_pcMergBasePu = refPu;//
	}
	else
	{
		m_pcMergBasePu = refPu;
	}
}

//
Void PuUnit::PredChang(){
	if (m_bStaSettled)return;
	PicData* pPic = m_curCtu->getPic();
	PuUnit *PuChan1,*PuChan2,*TmPu;//t: puchan1 point to curPU.
	UInt PicNum = pPic->getPicNum();
	FILE* handle;
	Char name[60];
	Bool puflag=false;
	//Bool SkipFlag;
	UInt chan = 0, corx, cory;
	Bool mergFlag;
	Bool SettledFlag;
	sprintf(name, "FramLog%d\\frm%d_CtuAddr%d_corx%d_cory%d.txt", PicNum,PicNum, m_curCtu->getCtuAddr(), m_coordx, m_coordy);
	handle=fopen(name,"wt");
	if (handle == NULL)
	{
		auto val = errno;
		char* mesg = strerror(errno);
		system("pause");
	}
	PuChan1 = this;
	
	while (PuChan1){
		Bool MergeWithZeroMV = PuChan1->getMergeWithZeroMv();
		Bool ZeroMV = PuChan1->getZeroCand();
		Bool TmpFlag = PuChan1->getTmpFlag();
		Status PuStatus;
		Int PropertyNum = 4;
		PuChan1->getMergFlag(mergFlag);
		PuChan2 = PuChan1->getPpu();
		if (mergFlag)
		{
			PuChan1->setSetFlag(false);
			//PuChan1->setStatus(NOTSURE);
			if (ZeroMV){
				chan++;
				PuChan1->setStatus(STA_BG);
				PuChan1->setSetFlag(true);
				PicNum = PuChan1->getCtu()->getPic()->getPicNum();
				PuChan1->getXY(corx, cory);
				PropertyNum = STA_BG;
				fprintf(handle, "ChanNum%d_RefFrame%d_CorX%d_CorY%d determined by ZeroMVP,Property is %s", chan, PicNum, corx, cory, Property[PropertyNum]);
				fclose(handle);
				return;
			}
			else if (TmpFlag)
			{
				chan++;
				TmPu = PuChan1->getTmppu();
				if (TmPu)
				{
					SettledFlag = TmPu->getSetFlag();
					if (SettledFlag){
						PuStatus = TmPu->getStatus();
						PuChan1->setStatus(PuStatus);
						PropertyNum = PuStatus;
						PuChan1->setSetFlag(true);
					}
					TmPu->getXY(corx, cory);
					PicNum = TmPu->getCtu()->getPic()->getPicNum();
					PuChan1->setSetFlag(false);
					PuChan1->setStatus(STA_NOTSURE);
					fprintf(handle, "ChanNum%d_RefFrame%d_CorX%d_CorY%d determined by TmpPu,Property is %s", chan, PicNum, corx, cory, Property[PropertyNum]);
				}
				else {
					PuChan1->setStatus(STA_NOTSURE);
					//system("pause");
					PuChan1->setSetFlag(true);
					fprintf(handle, "ChanNum%d_RefFrame%d_CorX%d_CorY%d  can`t determined by TmpPu,Property is %s", chan, PicNum, corx, cory, Property[PropertyNum]);
				}
				fclose(handle);
				return;
			}
			else if (PuChan2){
				SettledFlag = PuChan2->getSetFlag();
				if (SettledFlag){
					PuStatus = PuChan2->getStatus();
					PuChan1->setStatus(PuStatus);
					PropertyNum = PuStatus;
					PuChan1->setSetFlag(true);
				}
				else {
					PuStatus = PuChan2->getStatus();
					PuChan1->setStatus(PuStatus);
					PropertyNum = PuStatus;
					PuChan1->setSetFlag(false);
				}
				PuChan2->getXY(corx, cory);
				PicNum = PuChan2->getCtu()->getPic()->getPicNum();
				fprintf(handle, "ChanNum%d_RefFrame%d_CorX%d_CorY%d determined by SpatialPu,Property is %s", chan, PicNum, corx, cory, Property[PropertyNum]);
				fclose(handle);
				return;
			}
			else {
				printf("system error ");
				system("pause");
			}
		}
		/////////////////////////////////////
		else{
			//PuUnit *PuChan1,*PuChan2,*TmPu;//t: puchan1 point to curPU.
			if (((m_auMvd[0] > 4) && (m_auMvd[1] > 3)) || ((m_auMvd[1] > 4)) && (m_auMvd[0] > 3))
			{
				m_status = STA_MOVOBJ;
				m_bStaSettled = true;
				fclose(handle);
				return;
			}
			if (m_uAMVPIndex == 0)
			{
				if (m_pcRefPu != NULL){
					SettledFlag = m_pcRefPu->getSetFlag();
					if (SettledFlag){
						m_bStaSettled = true;
						m_status = m_pcRefPu->getStatus();
						if ((((m_auMvd[0] > 4) && (m_auMvd[1] > 2)) || ((m_auMvd[1] > 4)) && (m_auMvd[0] > 2)) && m_status == STA_BG)
							m_status = STA_MOVOBJ;
						fclose(handle);
						return;
					}
				}
				else if (m_bTempFlag)
				{
					if (m_pcRefPu != NULL){

						SettledFlag = m_pcRefPu->getSetFlag();
						if (SettledFlag){
							m_bStaSettled = true;
							m_status = m_pcRefPu->getStatus();
							if ((((m_auMvd[0] > 4) && (m_auMvd[1] > 2)) || ((m_auMvd[1] > 4)) && (m_auMvd[0] > 2)) && m_status == STA_BG)
								m_status = STA_MOVOBJ;
							fclose(handle);
							return;
						}
					}
					else {
						m_status = STA_MOVOBJ;
						m_bStaSettled = true;
						fclose(handle);
						return;
					}
				}
				else {
					m_status = STA_BG;
					m_bStaSettled = true; 
					fclose(handle);
					return;
				}
			}

			else if (m_uAMVPIndex == 1)
			{
				if (m_pcSecRefPu == NULL&&m_bTempFlag == true)
				{
					if (m_pcRefPu != NULL)
					{
						SettledFlag = m_pcRefPu->getSetFlag();
						if (SettledFlag)
						{
							m_bStaSettled = true;
							m_status = m_pcRefPu->getStatus();
							if ((((m_auMvd[0] > 4) && (m_auMvd[1] > 2)) || ((m_auMvd[1] > 4)) && (m_auMvd[0] > 2)) && m_status == STA_BG)
								m_status = STA_MOVOBJ;
							fclose(handle);
							return;
						}
					}
					else {
						m_status = STA_MOVOBJ;
						m_bStaSettled = true;
						fclose(handle);
						return;
					}
				}
				else if (m_bZeroCand)
				{
					m_status = STA_BG;
					m_bStaSettled = true;
					fclose(handle);
					return;
				}
				else
				{
					m_bStaSettled = false;
					m_status = STA_NOTSURE;
					fclose(handle);
					return;
				}
			}

			break;
		}

	}
	fclose(handle);
}

////////////////////////////////////////////////////////////////
Void PuUnit::dealTwoSmvp()
{
	Bool MvpIsSure = false;
	Bool twoMvpEqual = false;
	//condition: spatial mvps maybe the same when only on reference frame
	if (m_pcRefPu == m_pcSecRefPu && m_uScale == 4096)
	{
		MvpIsSure = true;
		twoMvpEqual = true;
	}
	//two spatial mvp have different status
	if (MvpIsSure == false)
	{
		if (m_pcRefPu->getStatus() != m_pcSecRefPu->getStatus())
		{
			twoMvpEqual = false;
			MvpIsSure = true;
		}
	}
	//two spatial mvps may have relation
	if (MvpIsSure == false)
	{
		if (m_pcRefPu->getPuAbsAddrY() <= m_PuAddInPicY && m_pcSecRefPu->getPuAbsAddrX() <= m_PuAddInPicX)
		{
			//two of them are AMVP model
			if (m_pcRefPu->getMergflag() == false && m_pcSecRefPu->getMergflag() == false)
			{
				twoMvpEqual = false;
				MvpIsSure = true;
			}
			//two of them are Merge model
			else if (m_pcRefPu->getMergflag() == true && m_pcSecRefPu->getMergflag() == true)
			{
				if (m_pcRefPu->getBaseMergPu() == m_pcSecRefPu->getBaseMergPu() && m_pcRefPu->getBaseMergPu() != NULL)
				{
					twoMvpEqual = true;
					MvpIsSure = true;
				}
			}
			//
			else 
			{
				//one of them merge with another
				if ((m_pcRefPu->getMergflag() == true && m_pcRefPu->getPpu() == m_pcSecRefPu) ||
					(m_pcSecRefPu->getMergflag() == true && m_pcSecRefPu->getPpu() == m_pcRefPu))
				{
					twoMvpEqual = true;
					MvpIsSure = true;
				}
				//
				else if (MvpIsSure == false &&
					((m_pcSecRefPu->getCtu() != m_curCtu&&m_pcRefPu->getMergflag() == false)
					|| (m_pcSecRefPu->getCtu() == m_curCtu&&m_pcSecRefPu->getMergflag() == false)))
				{
					twoMvpEqual = false;
					MvpIsSure = true;
				}
			}
		}
		else if (m_pcRefPu->getPuAbsAddrY() <= m_PuAddInPicY&&m_pcSecRefPu->getPuAbsAddrY()>m_PuAddInPicX)
		{
			Int testPoint = 0;
		}
	}
	if (MvpIsSure==false)
	{
		if (m_SmvpEqual == true)
			twoMvpEqual = true;
		MvpIsSure = true;
	}
	if (MvpIsSure)
	{
		if (twoMvpEqual)
		{
			if (m_bTempFlag)
			{
				m_SortBasis = BY_REFER_TEMPERAL;
				m_status = m_pcTempPu->getStatus();
			}
			else
			{
				m_SortBasis = BY_ZERO_MV;
				m_status = STA_BG;
			}
		}
		else
		{
			m_SortBasis = BY_REFER_SPATIAL;
			m_pcAmvpRefPu = m_pcSecRefPu;
			m_status = m_pcSecRefPu->getStatus();
		}
	}
}


//left and above
Bool PuUnit::relationLeftAndAbove(Bool &equalFlag)
{
	PuUnit* pcLeftPU;
	PuUnit* pcAbovePU;
	std::map<PuUnit*, Bool> routePU;
	pcLeftPU = m_mMergeCandidatePus[MD_LEFT];
	pcAbovePU = m_mMergeCandidatePus[MD_ABOVE];
	if (pcLeftPU == NULL || pcAbovePU == NULL)
	{
		printf("error to find neighbor PUs of above and left \n");
		system("pause");
		return false;
	}
	// first of all if two of the candidate status is sure and not same ,their mv should not equal.

	//they merge with the same or merge with the other 
	if (pcLeftPU->getMergflag() && pcAbovePU->getMergflag())
	{
		if (pcLeftPU->MergeBaseIsSure() && pcAbovePU->MergeBaseIsSure() && pcAbovePU->getBaseMergPu() == pcLeftPU->getBaseMergPu())
		{
			equalFlag = true;
			return true;
		}
	}
	if (pcLeftPU->getMergflag() == false && pcAbovePU->getMergflag())
	{
		if (pcAbovePU->MergeBaseIsSure() && pcAbovePU->getBaseMergPu() == pcLeftPU)
		{
			equalFlag = true;
			return true;
		}
	}
	if (pcAbovePU->getMergflag() == false && pcLeftPU->getMergflag())
	{
		if (pcLeftPU->MergeBaseIsSure() && pcLeftPU->getBaseMergPu() == pcAbovePU)
		{
			equalFlag = true;
			return true;
		}
	}
	
	if (pcAbovePU->getMergflag() == false && pcLeftPU->getMergflag())
	{
		if (pcAbovePU->getMvpIsSure() && !pcAbovePU->getZeroCand() && (pcAbovePU->getFinal()->getMergflag()))
		{
			if (pcLeftPU->MergeBaseIsSure() && pcAbovePU->getFinal()->MergeBaseIsSure())
			{
				if (pcLeftPU->getBaseMergPu() == pcAbovePU->getFinal()->getBaseMergPu())
				{
					equalFlag = false;
					return true;
				}
			}
		}
	}
	
	if (pcAbovePU->getMergflag() && pcLeftPU->getMergflag())
	{
		if (pcLeftPU->getMvpIsSure() && pcLeftPU->getFinal()->getMergflag())
		{
			if (pcLeftPU->getFinal()->getBaseMergPu() == pcAbovePU->getBaseMergPu())
			{
				equalFlag = false;
				return true;
			}
		}
	}
	//first left upedeg is above current edge 
	if (pcLeftPU->getPuAbsAddrY() < m_PuAddInPicY)
	{
		//they are neighbors 
		if (pcAbovePU->getPuAbsAddrX() == m_PuAddInPicX)
		{
			if (pcAbovePU->getMergflag())
			{
				if (pcAbovePU->getMergIndex() != 0)
					equalFlag = false;
				else
					equalFlag = true;
			}
			else
				equalFlag = false;
			return true;
		}
		//there are PUs between them 
		else if (pcAbovePU->getPuAbsAddrX() > m_PuAddInPicX)
		{
			PuUnit* tempLeftPu;
			getLeftPu(pcAbovePU, tempLeftPu);
			if (tempLeftPu == NULL || tempLeftPu->getPredMode() == MODE_INTRA)
			{
				return false;
			}
			else
			{
				//only one pu between theam and it  is amvp 
				if (tempLeftPu->getPuAbsAddrX() == m_PuAddInPicX)
				{
					if (tempLeftPu->getMergflag() == false)
					{
						if (pcAbovePU->getMergflag() && pcAbovePU->getMergIndex() == 0 && pcAbovePU->getB1Flag() == true)
						{
							equalFlag = false;
							return true;
						}
					}
					else
					{
						if (pcAbovePU->getMergflag() == false && tempLeftPu->getMergIndex() == 0)
						{
							equalFlag = false;
							return true;
						}
					}
				}
				//not only one 
				else
				{
					//above neighbor pu is equal to left 
					if (pcAbovePU->getMergflag() == false)
					{
						if (tempLeftPu->getMergflag() && tempLeftPu->getBaseMergPu() == pcLeftPU && tempLeftPu->MergeBaseIsSure())
						{
							equalFlag = false;
							return true;
						}
						else return false;
					}
					// above is equal to the right pu of left 
					if (pcLeftPU->getMergflag() == false && pcAbovePU->getMergflag() && pcAbovePU->MergeBaseIsSure())
					{
						PuUnit* basePu = pcAbovePU->getBaseMergPu();
						if (basePu == NULL && pcLeftPU->getStatus() != STA_BG && pcLeftPU->getStatusIsSure())
						{
							equalFlag = false;
							return true;
						}
						if (basePu != NULL )
						{
							getLeftPu(basePu, tempLeftPu);
							if (tempLeftPu == pcLeftPU)
							{
								equalFlag = false;
								return true;
							}
						}
					}
				}
			}
		}
	}
	//above pu is on the edge of curretn PU 
	//in this condition, above PU is the above and above right PU of left PU
	if (pcAbovePU->getPuAbsAddrX() < m_PuAddInPicX)
	{
		if (pcLeftPU->getPuAbsAddrY() == m_PuAddInPicY)
		{
			if (pcLeftPU->getMergflag() == false)
			{
				equalFlag = false;
				return true;
			}
			else
			{
				if (pcAbovePU->getMergflag())
				{
					if (pcAbovePU->MergeBaseIsSure() && pcLeftPU->MergeBaseIsSure())
					{
						if (pcAbovePU->getBaseMergPu() == pcLeftPU->getBaseMergPu())
						{
							equalFlag = true;
							return true;
						}
					}
				}
				else
				{
					if (pcLeftPU->getMergeCandIsSure() && pcLeftPU->getPpu() != pcAbovePU)
					{
						equalFlag = false;
						return true;
					}
					if (pcLeftPU->getBaseMergPu() == pcAbovePU && pcLeftPU->MergeBaseIsSure())
					{
						equalFlag = true;
						return true;
					}
					if (pcLeftPU->getMergIndex() == 0 && pcLeftPU->getSpaMvpDir(0) == MD_LEFT)
					{
						if (pcLeftPU->getPpu()->getPuAbsAddrY() == m_PuAddInPicY)
						{
							equalFlag = false;
							return true;
						}
					}
				}
			}
		}
		else
		{
			PuUnit* abovePu;
			getUpPu(pcLeftPU, abovePu);
			if (abovePu == NULL || abovePu->getPredMode() == MODE_INTRA)
			{
				return false;
			}
			else
			{
				if (abovePu->getMergflag())
				{
					if (abovePu->getMergeCandIsSure() && abovePu->getPpu() == pcAbovePU)
					{
						if (pcLeftPU->getMergflag() == false)
						{
							equalFlag = false;
							return true;
						}
					}
				}
				else
				{
					if (pcLeftPU->getMergeCandIsSure() && pcLeftPU->getPpu() == abovePu)
					{
						equalFlag = false;
						return true;
					}
				}
			}
		}
	}
	else if (pcLeftPU->getPuAbsAddrY() == m_PuAddInPicY && pcAbovePU->getPuAbsAddrX() == m_PuAddInPicX)
	{
		if (pcLeftPU->getMergflag() == false && pcAbovePU->getMergflag() == false)
		{
			equalFlag = false;
			return true;
		}
		else
		{
			if (pcLeftPU->getCb()->getCtu() != pcAbovePU->getCb()->getCtu())
			{
				if (pcLeftPU->getMergflag() == false)
				{
					equalFlag = false;
					return true;
				}
				else
				{
					if (pcLeftPU->getMergeCandIsSure())
					{
						if (pcLeftPU->getPpu() == pcAbovePU)
						{
							equalFlag = true;
							return true;
						}
					}
					if (pcLeftPU->MergeBaseIsSure() && pcAbovePU->MergeBaseIsSure())
					{
						if (pcLeftPU->getBaseMergPu() == pcAbovePU->getBaseMergPu())
						{
							equalFlag = true;
							return true;
						}
					}
				}
			}
			if (pcLeftPU->getCb()->getCtu() == pcAbovePU->getCb()->getCtu())
			{
				if (pcLeftPU->getAbsPartInd() < pcAbovePU->getAbsPartInd())
				{
					if (pcAbovePU->getMergflag() == false)
					{
						equalFlag = false;
						return true;
					}
					else
					{
						if (pcAbovePU->getMergeCandIsSure() && pcAbovePU->getPpu() == pcLeftPU)
						{
							equalFlag = true;
							return true;
						}
					}
				}
				if (pcLeftPU->getAbsPartInd() > pcAbovePU->getAbsPartInd())
				{
					if (pcLeftPU->getMergflag() == false)
					{
						equalFlag = false;
						return true;
					}
					else
					{
						if (pcAbovePU->getMergeCandIsSure() && pcLeftPU->getPpu() == pcAbovePU)
						{
							equalFlag = false;
							return true;
						}
					}
				}
			}
		}
	}
	return false;
	/////////////////////////////////////////////
}
//below left and left 
Bool PuUnit::relationLBAndLeft(Bool &equalFlag)
{
	PuUnit* pcLeft;
	PuUnit* pcLeftBot;
	pcLeft = m_mMergeCandidatePus[MD_LEFT];
	pcLeftBot = m_mMergeCandidatePus[MD_BELOW_LEFT];
	if (pcLeft == NULL || pcLeftBot == NULL)
	{
		printf("error to find neighbor PUs of above and above right\n ");
		system("pause");
	}
	//first of all ,the two candidate maybe only one 
	if (pcLeft == pcLeftBot)
	{
		equalFlag = true;
		return true;
	}
	//if they come from the same CB
	if (pcLeft->getCb() == pcLeftBot->getCb())
	{
		if (pcLeft->getMergflag() && pcLeftBot->getMergflag())
		{
			if (pcLeft->getBaseMergPu() == pcLeftBot->getBaseMergPu())
				equalFlag = true;
			else
				equalFlag = false;
		}
		else
			equalFlag = false;
		return true;
	}
	// if they have two different status 
//	if (pcLeft->getStatus() != pcLeftBot->getStatus() && pcLeft->getStatusUnsure() == false && pcLeftBot->getStatusUnsure() == false)
//	{
//		equalFlag = false;
//		return true;
//	}
	else
	{
		//if bottom is AMVP model so they must have different mv 
		if (pcLeftBot->getMergflag() == false)
		{
			equalFlag = false;
			return true;
		}
		else
		{
			if (pcLeftBot->getPpu() == pcLeft && pcLeftBot->getMergeCandIsSure() == true)
			{
				equalFlag = true;
				return true;
			}

			/*
			//if left is merge mode and leftbottom maybe  its left bottom  
			//acutally this will not happen for the scan order
			if (pcLeft->getMergflag() == false && pcLeft->getPuAbsAddrX() > pcLeftBot->getPuAbsAddrX())
			{
				equalFlag = false;
				return true;
			}
			*/

			if (pcLeft->getMergflag() == true)
			{
				if (pcLeft->getBaseMergPu() == pcLeftBot->getBaseMergPu() && pcLeft->MergeBaseIsSure() && pcLeftBot->MergeBaseIsSure())
				{
					equalFlag = true;
					return true;
				}
				else
					return false;
			}
		}
	}
	return false;
}
//lfet and above left
Bool PuUnit::relationLeftAndLA(Bool &equalFlag)
{
	PuUnit* pcLeft;
	PuUnit* pcLeftAbove;
	pcLeft = m_mMergeCandidatePus[MD_LEFT];
	pcLeftAbove = m_mMergeCandidatePus[MD_ABOVE_LEFT];
	if (pcLeft == NULL || pcLeftAbove == NULL)
	{
		printf("error to find neighbor PU left and left above");
		system("pause");
	}
	//left and above left is the same pu 
	if (pcLeft == pcLeftAbove)
	{
		equalFlag = true;
		return true;
	}
	//they are in the same CB 
	if (pcLeft->getCb() == pcLeftAbove->getCb())
	{
		if (pcLeft->getMergflag() && pcLeftAbove->getMergflag())
		{
			if (pcLeft->getBaseMergPu() == pcLeftAbove->getBaseMergPu())
				equalFlag = true;
			else
				equalFlag = false;
			return true;
		}
		equalFlag = false;
		return true;
	}
//	if (pcLeft->getStatus() != pcLeftAbove->getStatus() && pcLeft->getStatusUnsure() == false && pcLeftAbove->getStatusUnsure() == false)
//	{
//		equalFlag = false;
//		return true;
//	}

	//left pu is not a neighbor of left above PU
	//there PUs between them 
	if ((pcLeftAbove->getPuAbsAddrY() + pcLeftAbove->getPuHeight()) < pcLeft->getPuAbsAddrY())
	{
		PuUnit* upPu;
		getUpPu(pcLeft, upPu);
		if (upPu == NULL || (upPu != NULL && upPu->getMergflag() == false))
			return false;
		else
		{
			if (upPu->MergeBaseIsSure())
			{
				//above left is merge mode
				if (pcLeftAbove->getMergflag())
				{
					if (pcLeftAbove->MergeBaseIsSure() && pcLeftAbove->getBaseMergPu() == upPu->getBaseMergPu())
					{
						if (pcLeft->getMergflag() == false)
						{
							equalFlag = false;
							return true;
						}
						else
						{
							if (pcLeft->MergeBaseIsSure())
							{
								if (pcLeft->getBaseMergPu() == upPu->getBaseMergPu())
								{
									equalFlag = true;
									return true;
								}
								else
									return false;
							}
						}
					}
				}
				else
				{
					if (pcLeft->MergeBaseIsSure() && pcLeft->getPpu() == upPu)
					{
						equalFlag = false;
						return true;
					}
					else
						return false;
				}
			}
		}
		return false;
	}
	// left PU is a neighbor of left above PU
	else
	{
		if (pcLeft->getMergflag() == false)
		{
			equalFlag = false;
			return true;
		}
		else
		{
			if (pcLeft->getMergeCandIsSure() == true)
			{
				if (pcLeft->getPpu() == pcLeftAbove)
				{
					equalFlag = true;
					return true;
				}
				if (pcLeft->MergeBaseIsSure() && pcLeftAbove->getMergflag() == true && pcLeftAbove->MergeBaseIsSure())
				{
					if (pcLeft->getBaseMergPu() == pcLeftAbove->getBaseMergPu())
					{
						equalFlag = true;
						return true;
					}
				}
			}
		}
	}
	return false;
}

Void PuUnit::searchMergeCandidate()
{

	//index is 0 start
	//in this condition, it is sure which candidate is used 
	if (m_uMergeIndex == 0)
	{
		m_bMrgCandIsSure = true;
		//if no spatial candidate exist 
		if (m_mMergeCandidatePus.empty())
		{
			if (m_bTempFlag == true)
			{
				m_status = m_pcTempPu->getStatus();
				m_bMrgStatusIsSure = m_pcTempPu->getStatusIsSure();
				m_pcRefPu = m_pcTempPu;
				getInfoFromRefPu(m_pcTempPu, true);
				if (m_pcTempPu->getMergflag() == true)
				{
					m_pcMergBasePu = m_pcTempPu->getBaseMergPu();
					m_bMrgBaseIsSure = m_pcTempPu->MergeBaseIsSure();
				}
				else
				{
					m_pcMergBasePu = m_pcTempPu;
					m_bMrgBaseIsSure = true;
				}
			}
			else
			{
				m_status = STA_BG;
				m_pcRefPu = NULL;
				m_bMrgStatusIsSure = true;
				m_bMvEqlZero = true;
				m_bMrgBaseIsSure = true;
				m_MergeWihtZeroMv = true;
			}
		}
		//spatial candidates exist 
		else
		{
			m_pcRefPu = m_mMergeCandidatePus.begin()->second;
			m_sMvpDir[0] = m_mMergeCandidatePus.begin()->first;
			m_status = m_pcRefPu->getStatus();
			m_bMrgStatusIsSure = m_pcRefPu->getStatusIsSure();
			if (m_pcRefPu->getMergflag() == true)
			{
				m_pcMergBasePu = m_pcRefPu->getBaseMergPu();
				m_bMrgBaseIsSure = m_pcRefPu->MergeBaseIsSure();
			}
			else
			{
				m_pcMergBasePu = m_pcRefPu;
				m_bMrgBaseIsSure = true;
			}
		}
		return;
	}

	//index is 4
	//in this condition, the candidate must be temporal or zero MV 
	if (m_uMergeIndex == 4)
	{
		m_bMrgCandIsSure = true;
		if (m_bTempFlag == true)
		{
			m_status = m_pcTempPu->getStatus();
			m_bMrgStatusIsSure = m_pcTempPu->getStatusIsSure();
			m_pcRefPu = m_pcTempPu;
			getInfoFromRefPu(m_pcTempPu, true);
			if (m_pcTempPu->getMergflag() == true)
			{
				m_pcMergBasePu = m_pcTempPu->getBaseMergPu();
				m_bMrgBaseIsSure = m_pcTempPu->MergeBaseIsSure();
			}
			else
			{
				m_pcMergBasePu = m_pcTempPu;
				m_bMrgBaseIsSure = true;
			}
		}
		else
		{
			m_status = STA_BG;
			m_pcRefPu = NULL;
			m_bMrgStatusIsSure = true;
			m_bMvEqlZero = true;
			m_bMrgBaseIsSure = true;
			m_MergeWihtZeroMv = true;
		}
		return;
	}
	
	//no enough spatial candidate 
	if (m_mMergeCandidatePus.size()<m_uMergeIndex)
	{
		m_bMrgCandIsSure = true;
		if (m_bTempFlag == true)
		{
			m_status = m_pcTempPu->getStatus();
			m_bMrgStatusIsSure = m_pcTempPu->getStatusIsSure();
			m_pcRefPu = m_pcTempPu;
			if (m_pcTempPu->getMergflag() == true)
			{
				m_pcMergBasePu = m_pcTempPu->getBaseMergPu();
				m_bMrgBaseIsSure = m_pcTempPu->MergeBaseIsSure();
			}
			else
			{
				m_pcMergBasePu = m_pcTempPu;
				m_bMrgBaseIsSure = true;
			}
		}
		else
		{
			m_status = STA_BG;
			m_pcRefPu = NULL;
			m_bMrgStatusIsSure = true;
			m_bMvEqlZero = true;
			m_bMrgBaseIsSure = true;
			m_MergeWihtZeroMv = true;
		}
		return;
	}

	//
	if (m_uMergeIndex > 0 && m_uMergeIndex < 4)
	{
		PuUnit *leftPU, *abovePU, *aboveRigPU, *belowLefPU, *aboveLefPu, *pRefPu = NULL;
		Bool relateA1AndB1 = false, relateB1AndB0 = false, relateA1AndA0 = false, relateB1AndB2 = false, relateA1AndB2 = false;
		Bool leftEqlAbove = false, aboveEqlAboveRig = false, leftEqlBelowLef = false, aboveEqlAboveLef = false, leftEqlAboveLef = false;
		UInt diffNum = 0, possibleIdx = (UInt)m_mMergeCandidatePus.size();
		MVP_DIR pRefDir = MD_NONE;
		Bool realIndexExist = true, stopFlag = false;
		Bool fillFlag = true;
		std::map<MVP_DIR, PuUnit*, RankPuByDir> candPuBuffer;

		//left candidate is available 
		if (m_bA1Avail)
		{
			leftPU = m_mMergeCandidatePus[MD_LEFT];
			candPuBuffer[MD_LEFT] = leftPU;
			++diffNum;

		}
		//above candidate is available
		//compare left and above 
		if (m_bB1Avail)
		{
			abovePU = m_mMergeCandidatePus[MD_ABOVE];
			//left candidate exists 
			if (m_bA1Avail)
			{
				relateA1AndB1 = relationLeftAndAbove(leftEqlAbove);
				if (relateA1AndB1 == true)
				{
					if (leftEqlAbove == false)
					{
						if (fillFlag)
							candPuBuffer[MD_ABOVE] = abovePU;
						if (diffNum == m_uMergeIndex)
						{
							pRefPu = abovePU;
							pRefDir = MD_ABOVE;
							stopFlag = true;
						}
						++diffNum;
					}
					else
						--possibleIdx;
				}
				else
				{
					if (fillFlag)
						candPuBuffer[MD_ABOVE] = abovePU;
					realIndexExist = false;
				}
			}
			//left candidate not exist 
			else
			{
				if (fillFlag)
					candPuBuffer[MD_ABOVE] = abovePU;
				++diffNum;
			}
		}
		//above right is available
		//compare above and above right
		if (m_bB0Avail)
		{
			aboveRigPU = m_mMergeCandidatePus[MD_ABOVE_RIGHT];
			//above exist 
			if (m_bB1Avail)
			{
				relateB1AndB0 = relationAboveAndAR(aboveEqlAboveRig);
				//relation exist 
				if (relateB1AndB0 == true)
				{
					if (aboveEqlAboveRig == false)
					{
						if (fillFlag)
							candPuBuffer[MD_ABOVE_RIGHT] = aboveRigPU;
						if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
						{
							pRefPu = aboveRigPU;
							pRefDir = MD_ABOVE_RIGHT;
							stopFlag = true;
						}
						++diffNum;
					}
					else
						--possibleIdx;
				}
				//not exist
				else
				{
					candPuBuffer[MD_ABOVE_RIGHT] = aboveRigPU;
					realIndexExist = false;
				}
			}
			//above not exist 
			else
			{
				if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
				{
					pRefPu = aboveRigPU;
					pRefDir = MD_ABOVE_RIGHT;
					stopFlag = true;
				}
				candPuBuffer[MD_ABOVE_RIGHT] = aboveRigPU;
				++diffNum;
			}
		}

		//below left is available
		//compare left and below left 
		if (m_bA0Avail)
		{
			belowLefPU = m_mMergeCandidatePus[MD_BELOW_LEFT];
			//left is available 
			if (m_bA1Avail)
			{
				relateA1AndA0 = relationLBAndLeft(leftEqlBelowLef);
				if (relateA1AndA0)
				{
					if (leftEqlBelowLef == false)
					{
						candPuBuffer[MD_BELOW_LEFT] = belowLefPU;
						if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
						{
							pRefPu = belowLefPU;
							pRefDir = MD_BELOW_LEFT;
							stopFlag = true;
						}
						++diffNum;
					}
					else
						--possibleIdx;
				}
				else
				{
					candPuBuffer[MD_BELOW_LEFT] = belowLefPU;
					realIndexExist = false;
				}
			}
			//left is not available
			else
			{
				if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
				{
					pRefPu = belowLefPU;
					pRefDir = MD_BELOW_LEFT;
					stopFlag = true;
				}
				candPuBuffer[MD_BELOW_LEFT] = belowLefPU;
				++diffNum;
			}
		}
		//above left is available
		//compare with left and above 
		if (m_bB2Avail)
		{
			aboveLefPu = m_mMergeCandidatePus[MD_ABOVE_LEFT];
			if (m_bA1Avail)
				relateA1AndB2 = relationLeftAndLA(leftEqlAboveLef);
			if (m_bB1Avail)
				relateB1AndB2 = relationAboveAndLA(aboveEqlAboveLef);
			if (m_bA1Avail && m_bB1Avail)
				relateA1AndB1 = relationLeftAndAbove(leftEqlAbove);
			//three conditions 
			//first : left exists ,above not exist 
			if (m_bA1Avail && m_bB1Avail == false)
			{
				if (relateA1AndB2)
				{
					if (leftEqlAboveLef == false)
					{
						candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
						if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
						{
							pRefPu = aboveLefPu;
							pRefDir = MD_ABOVE_LEFT;
							stopFlag = true;
						}
						++diffNum;
					}
					else
						--possibleIdx;
				}
				else
				{
					realIndexExist = false;
					candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
				}
			}
			//second :left not exist , above exists 
			if (m_bA1Avail == false && m_bB1Avail)
			{
				if (relateB1AndB2)
				{
					if (aboveEqlAboveLef == false)
					{
						candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
						if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
						{
							pRefPu = aboveLefPu;
							pRefDir = MD_ABOVE_LEFT;
							stopFlag = true;
						}
						++diffNum;
					}
					else
						--possibleIdx;
				}
				else
				{
					realIndexExist = false;
					candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
				}
			}
			//third :both left and above area exist 
			if (m_bA1Avail && m_bB1Avail)
			{
				//left  and above exist relation 
				if (relateA1AndB1)
				{
					//left and above  are equal 
					if (leftEqlAbove)
					{
						//if above left have relation with both left and above 
						if (relateA1AndB2 && relateB1AndB2)
						{
							//if both left and above equal above left ,but they are not equal ,
							//there must something wrong 
							if (leftEqlAboveLef && !aboveEqlAboveLef)
							{
								printf("\n there must somethin be wrong in relation B1 && A1!\n");
								printf("%d \n%s \n", __LINE__, __FILE__);
								system("pause");
							}
							else
							{
								//if above left is not equal to one of left or above
								if (leftEqlAboveLef == false || aboveEqlAboveLef == false)
								{
									candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
									if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
									{
										pRefPu = aboveLefPu;
										pRefDir = MD_ABOVE_LEFT;
										stopFlag = true;
									}
									++diffNum;
								}
							}
						}
						//one of them have relation with B2
						//left have relation with above left 
						else if (relateA1AndB2 && !relateB1AndB2)
						{
							if (leftEqlAboveLef == false)
							{
								candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
								if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
								{
									pRefPu = aboveLefPu;
									pRefDir = MD_ABOVE_LEFT;
									stopFlag = true;
								}
								++diffNum;
							}
							else
								--possibleIdx;
						}
						//above have relation with above left 
						else if (relateB1AndB2 && !relateA1AndB2)
						{
							if (aboveEqlAboveLef == false)
							{
								candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
								if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
								{
									pRefPu = aboveLefPu;
									pRefDir = MD_ABOVE_LEFT;
									stopFlag = true;
								}
								++diffNum;
							}
							else
								--possibleIdx;
						}
						//they have no relation 
						else if (!relateA1AndB2 && !relateB1AndB2)
						{
							realIndexExist = false;
							candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
						}
					}

					//they are not equal 
					else
					{
						//they have relation 
						if (relateA1AndB2 && relateB1AndB2)
						{
							// above left equal with both left and above 
							if (leftEqlBelowLef && aboveEqlAboveLef)
							{
								printf("\n there must somethin be wrong in relation B1 && A1!\n");
								printf("%d\n %s \n", __LINE__, __FILE__);
								system("pause");
							}
							//above left not equal either one of above and left 
							else if (!leftEqlAboveLef && !aboveEqlAboveLef)
							{
								candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
								if (diffNum == m_uMergeIndex && realIndexExist == true && stopFlag == false)
								{
									pRefPu = aboveLefPu;
									pRefDir = MD_ABOVE_LEFT;
									stopFlag = true;
								}
								++diffNum;
							}
						}
						else if (relateA1AndB2 && !relateB1AndB2)
						{
							if (leftEqlAboveLef == false)
							{
								realIndexExist = false;
								candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
								++diffNum;
							}
						}

						else if (relateB1AndB2 && !relateA1AndB2)
						{
							if (aboveEqlAboveLef == false)
							{
								realIndexExist = false;
								candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
								++diffNum;
							}
						}
						else if (!relateA1AndB2 && !relateB1AndB2)
						{
							realIndexExist = false;
							candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
						}
					}
				}
				//left and above have no relation 
				else
				{
					if (relateA1AndB2 && relateB1AndB2)
					{
						if (leftEqlAboveLef && aboveEqlAboveLef)
						{
							printf("\n there must somethin be wrong in relation B1 && A1!\n");
							printf("%d\n %s \n", __LINE__, __FILE__);
							system("pause");
						}
						else if (!leftEqlAboveLef || !aboveEqlAboveLef)
						{
							realIndexExist = false;
							candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
							++diffNum;
						}
					}
					else if (relateA1AndB2 && !relateB1AndB2)
					{
						if (leftEqlAboveLef == false)
						{
							realIndexExist = false;
							candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
							++diffNum;
						}
					}
					else if (!relateA1AndB2 &&relateB1AndB2)
					{
						if (aboveEqlAboveLef == false){
							realIndexExist = false;
							candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
							++diffNum;
						}
					}
					else if (!relateA1AndB2 && !relateB1AndB2)
					{
						realIndexExist = false;
						candPuBuffer[MD_ABOVE_LEFT] = aboveLefPu;
					}
				}
			}
		}
		///////////////
		//condition that the right candidate could be located 
		if (realIndexExist == true)
		{
			//located at spatial candidate 
			if (stopFlag == true)
			{
				m_pcRefPu = pRefPu;									//set the right reference candidate 
				m_bMrgCandIsSure = true;							//set candidate flag 
				m_sMvpDir[0] = pRefDir;
				m_status = pRefPu->getStatus();						//get status 
				m_bMrgStatusIsSure = pRefPu->getStatusIsSure();		//if current status is sure 
				getInfoFromRefPu(pRefPu, false);					//
				if (m_pcRefPu->getMergflag())
					m_bMrgBaseIsSure = m_pcRefPu->MergeBaseIsSure();
				else
					m_bMrgBaseIsSure = true;
			}
			//temporal or zero MV
			else
			{
				if (m_bTempFlag == true)
				{
					m_pcRefPu = m_pcTempPu;
					m_status = m_pcTempPu->getStatus();
					m_bMrgStatusIsSure = m_pcTempPu->getStatusIsSure();
					m_bMrgCandIsSure = true;
					getInfoFromRefPu(m_pcTempPu, true);
					if (m_pcTempPu->getMergflag())
						m_bMrgBaseIsSure = m_pcTempPu->MergeBaseIsSure();
					else
						m_bMrgBaseIsSure = true;
				}
				else
				{
					m_status = STA_BG;
					m_pcRefPu = NULL;
					m_bMrgStatusIsSure = true;
					m_bMrgCandIsSure = true;
					m_MergeWihtZeroMv = true;
					m_bMvEqlZero = true;
					m_bMrgBaseIsSure = true;
				}
			}
		}
		//set varialbes of current pu 
		else
		{

		}

	}
}


//above and above right
Bool PuUnit::relationAboveAndAR(Bool &equalFlag)
{
	PuUnit *pcAbove;
	PuUnit *pcAboveRight;
	pcAbove = m_mMergeCandidatePus[MD_ABOVE];
	pcAboveRight = m_mMergeCandidatePus[MD_ABOVE_RIGHT];
	//one of them not exist
	if (pcAbove == NULL || pcAboveRight == NULL)
	{
		printf("error to find neighbor PUs of above and above right\n ");
		system("pause");
	}
	//they are the same one 
	if (pcAbove == pcAboveRight)
	{
		equalFlag = true;
		return true;
	}
	//above right is AMVP mode
	if (pcAboveRight->getMergflag() == false)
	{
		equalFlag = false;
		return true;
	}
	else
	{
		if (pcAbove->getCb() == pcAboveRight->getCb())
		{
			if (pcAbove->getMergflag() == false)
			{
				equalFlag = false;
				return true;
			}
			if (pcAbove->getMergflag() && pcAboveRight->getMergflag())
			{
				if (pcAbove->getBaseMergPu() == pcAboveRight->getBaseMergPu())
					equalFlag = true;
				else
					equalFlag = false;
			}
			else 
				equalFlag = false;
			return true;
		}
		else
		{

			if (pcAboveRight->getMergIndex() != 0)
				equalFlag = false;
			else
				equalFlag = true;
			return true;
		}
	}
}
// above and above left
Bool PuUnit::relationAboveAndLA(Bool &equalFlag)
{
	PuUnit* pcAbove;
	PuUnit* pcAboveLeft;
	pcAbove = m_mMergeCandidatePus[MD_ABOVE];
	pcAboveLeft = m_mMergeCandidatePus[MD_ABOVE_LEFT];
	if (pcAbove == NULL || pcAboveLeft == NULL)
	{
		printf("error to find above or above left PU\n");
		system("pause");
		return false;
	}
	//they may the same pu 
	if (pcAbove == pcAboveLeft)
	{
		equalFlag = true;
		return true;
	}
	//they may in the same CB 
	if (pcAbove->getCb() == pcAboveLeft->getCb())
	{
		if (pcAbove->getMergflag() && pcAboveLeft->getMergflag())
		{
			if (pcAbove->getBaseMergPu() == pcAboveLeft->getBaseMergPu())
				equalFlag = true;
			else
				equalFlag = false;
		}
		else equalFlag = false;
		return true;
	}
//	if (pcAbove->getStatus() != pcAboveLeft->getStatus() && 
//		pcAbove->getStatusUnsure() == false && pcAboveLeft->getStatusUnsure() == false)
//	{
//		equalFlag = false;
//		return true;
//	}
	if (pcAbove->getPuAbsAddrX() == m_PuAddInPicX)
	{
		if (pcAbove->getMergflag() == false)
		{
			equalFlag = false;
			return true;
		}
		else
		{
			if (pcAbove->getMergIndex() != 0)
			{
				equalFlag = false;
				return true;
			}
		}
	}
	else
	{
		//above is merge mode
		if (pcAbove->getMergflag() == true)
		{
			if (pcAboveLeft->getMergflag() == false)
			{
				if (pcAbove->getBaseMergPu() == pcAboveLeft && pcAbove->MergeBaseIsSure()==true)
				{
					equalFlag = true;
					return true;
				}
				else 
					return false;
			}
			if (pcAboveLeft->getMergflag() == true)
			{
				if (pcAbove->MergeBaseIsSure() && pcAboveLeft->MergeBaseIsSure())
				{
					if (pcAbove->getBaseMergPu() == pcAboveLeft->getBaseMergPu())
					{
						equalFlag = true;
						return true;
					}
					else
						return false;
				}
			}
		}
		//above is AMVP mode
		else
		{
			PuUnit* leftPU;
			getLeftPu(pcAbove, leftPU);
			if (leftPU == NULL || (leftPU != NULL && leftPU->getMergflag() == false))
				return false;
			if (leftPU == pcAboveLeft)
			{
				equalFlag = false;
				return false;
			}
			else
			{
				//there is only one PU between them 
				if (leftPU->getPuAbsAddrX() == m_PuAddInPicX)
				{
					if (leftPU->getMergIndex() == 0)
					{
						equalFlag = false;
						return true;
					}
					else
					{
						equalFlag = false;
						return true;
					}
				}
				//there exist other pus 
				else
				{
					if (pcAboveLeft->getMergflag() == true)
					{
						if (leftPU->MergeBaseIsSure() == true && pcAboveLeft->MergeBaseIsSure() == true)
						{
							if (leftPU->getBaseMergPu() == pcAboveLeft->getBaseMergPu())
							{
								equalFlag = false;
								return true;
							}
							else
								return false;
						}
					}
					else
					{
						if (leftPU->getBaseMergPu() == pcAboveLeft && leftPU->MergeBaseIsSure())
						{
							equalFlag = false;
							return true;
						}
						else
							return false;
					}
				}

			}
		}
	}
	return false;
}
//A0 & B1
Bool PuUnit::relationBLAndAbove(Bool &equalFlag)
{
	PuUnit *pcBL, *pcAbove;
	pcBL = m_mMergeCandidatePus[MD_BELOW_LEFT];
	pcAbove = m_mMergeCandidatePus[MD_ABOVE];
	if (pcBL == NULL || pcAbove == NULL)
	{
		printf(" error to load below or above neighbor PUs");
		system("pause");
	}
	if (pcBL->getPuAbsAddrY() == (m_PuAddInPicY + m_PuHeigh))
	{
		PuUnit *leftPu;
		if (pcBL->getMergflag() && pcAbove->getMergflag())
		{
			if (pcAbove->MergeBaseIsSure() && pcBL->MergeBaseIsSure())
			{
				if (pcAbove->getBaseMergPu() == pcBL->getBaseMergPu())
				{
					equalFlag = true;
					return true;
				}
			}
		}
		getLeftPu(this, leftPu);
		if (leftPu != NULL && leftPu->getPredMode() != MODE_INTRA)
		{
			Bool aboveAndLeft = false, leftAndBL = false;
			Bool leftEqlAbove, leftEqlBL;
			m_mMergeCandidatePus[MD_LEFT] = leftPu;
			aboveAndLeft = relationLeftAndAbove(leftEqlAbove);
			leftAndBL = relationLBAndLeft(leftEqlBL);
			if (aboveAndLeft&&leftAndBL)
			{
				if (leftEqlAbove && leftEqlBL)
				{
					equalFlag = true;
					return true;
				}
				if (leftEqlAbove == !leftEqlBL)
				{
					equalFlag = false;
					return true;
				}
			}
			else return false;
		}
		else return false;
	}
	else if (pcBL->getPuAbsAddrY() < (m_PuAddInPicY + m_PuHeigh))
	{
		Bool relationship = false;
		m_mMergeCandidatePus[MD_LEFT] = pcBL;
		relationship = relationLeftAndAbove(equalFlag);
		if (relationship == true)
			return true;
		else return false;
	}
	return false;
}
//A0 & B0
Bool PuUnit::relationBLAndAR(Bool &equalFlag)
{
	PuUnit *pcBL, *pcAR;
	pcBL = m_mMergeCandidatePus[MD_BELOW_LEFT];
	pcAR = m_mMergeCandidatePus[MD_ABOVE_RIGHT];
	if (pcBL == NULL || pcAR == NULL)
	{
		printf("error to load belowleft or above right PU");
		system("pause");
	}

	if (pcBL->getPuAbsAddrY() == (m_PuAddInPicY + m_PuHeigh))
	{
		if (pcAR->getPuAbsAddrX() == (m_PuAddInPicX + m_PuWidth))
		{
			PuUnit* leftPu;
			getLeftPu(this, leftPu);
			if (leftPu != NULL && leftPu->getPredMode() != MODE_INTRA)
			{
				Bool leftAndBL, leftEqlBL;
				m_mMergeCandidatePus[MD_LEFT] = leftPu;
				leftAndBL = relationLBAndLeft(leftEqlBL);
				if (leftAndBL == false)
				{
					return false;
				}
				else
				{
					Bool leftAndAR, leftEqlAR;
					leftAndAR = relationLeftAndAR(leftEqlAR);
					if (leftAndAR == false)
						return false;
					else
					{
						if (leftEqlBL && leftEqlAR)
						{
							equalFlag = true;
							return true;
						}
						else if (leftEqlAR == !leftEqlBL)
						{
							equalFlag = false;
							return true;
						}
						else return false;
					}
				}
			}
			else
			{
				getLeftPu(pcAR, leftPu);
				if (leftPu != NULL && leftPu->getPredMode() != MODE_INTRA)
				{
					Bool BLAndAbove, BLEqlAbove;
					Bool aboveAndAR, aboveEqlAR;
					m_mMergeCandidatePus[MD_ABOVE] = leftPu;
					BLAndAbove = relationBLAndAbove(BLEqlAbove);
					aboveAndAR = relationAboveAndAR(aboveEqlAR);
					if (BLAndAbove == false || aboveAndAR == false)
						return false;
					else
					{
						if (BLEqlAbove && aboveEqlAR)
						{
							equalFlag = true;
							return true;
						}
						if (BLEqlAbove == !aboveEqlAR)
						{
							equalFlag = false;
							return true;
						}
						else
							return false;
					}
				}
				else
					return false;
			}
		}
		if (pcAR->getPuAbsAddrX() < (m_PuAddInPicX + m_PuWidth))
		{
			Bool aboveAndBL;
			m_mMergeCandidatePus[MD_ABOVE] = pcAR;
			aboveAndBL = relationBLAndAbove(equalFlag);
			if (aboveAndBL)
			{
				return true;
			}
			else return false;
		}
		else return false;
	}
	else
	{
		m_mMergeCandidatePus[MD_LEFT] = pcBL;
		Bool relationLeftandAR;
		relationLeftandAR = relationLeftAndAR(equalFlag);
		if (relationLeftandAR == false)
			return false;
		else
			return true;
	}
	return false;
}
//A0 & B2
Bool PuUnit::relationBLAndAL(Bool &equalFlag)
{
	PuUnit *pcBL, *pcAL;
	pcBL = m_mMergeCandidatePus[MD_BELOW_LEFT];
	pcAL = m_mMergeCandidatePus[MD_ABOVE_LEFT];
	if (pcBL == NULL || pcAL == NULL)
	{
		printf("error to find below left or above left PU");
		system("pause");
	}
	if (pcBL->getPuAbsAddrY() == (m_PuAddInPicY + m_PuHeigh))
	{
		PuUnit* leftPu;
		getLeftPu(this, leftPu);
		if (leftPu == NULL || leftPu->getPredMode() == MODE_INTRA)
		{
			if (pcBL->getMergflag() && pcAL->getMergflag())
			{
				if (pcBL->MergeBaseIsSure() && pcAL->MergeBaseIsSure() && pcAL->getBaseMergPu() == pcBL->getBaseMergPu())
				{
					equalFlag = true;
					return true;
				}
				else
					return false;
			}
			else return false;
		}
		else
		{
			Bool BLAndLeft, LeftAndAL;
			Bool BLEqlLeft, LeftEqlAL;
			m_mMergeCandidatePus[MD_LEFT] = leftPu;
			BLAndLeft = relationLBAndLeft(BLEqlLeft);
			LeftAndAL = relationLeftAndLA(LeftEqlAL);
			if (BLAndLeft && LeftAndAL)
			{
				if (BLEqlLeft && LeftEqlAL)
				{
					equalFlag = true;
					return true;
				}
				if (BLEqlLeft == !LeftEqlAL)
				{
					equalFlag = false;
					return true;
				}
				else 
					return false;
			}
			else 
				return false;
		}
	}
	else
	{
		m_mMergeCandidatePus[MD_LEFT] = pcBL;
		Bool relationship = false;
		relationship = relationLeftAndLA(equalFlag);
		if (relationship)
			return true;
		else
			return false;
	}
}
//A1 & B0
Bool PuUnit::relationLeftAndAR(Bool &equalFlag)
{
	PuUnit *pcLeft, *pcAR;
	pcLeft = m_mMergeCandidatePus[MD_LEFT];
	pcAR = m_mMergeCandidatePus[MD_ABOVE_RIGHT];
	if (pcLeft == NULL || pcAR == NULL)
	{
		printf("error to load left or above right PU\n");
		system("pause");
	}
	if (pcAR->getPuAbsAddrX() == (m_PuAddInPicX + m_PuWidth))
	{
		PuUnit* leftPu;
		getLeftPu(pcAR, leftPu);
		if (leftPu == NULL || leftPu->getPredMode() == MODE_INTRA)
		{
			if (pcAR->getMergflag() && pcLeft->getMergflag())
			{
				if (pcAR->MergeBaseIsSure() && pcLeft->MergeBaseIsSure() && pcAR->getBaseMergPu() == pcLeft->getBaseMergPu())
				{
					equalFlag = true;
					return true;
				}
				else
					return false;
			}
			else
				return false;
		}
		else
		{
			Bool leftAndAbove, aboveAndAR;
			Bool leftEqlAbove, aboveEqlAR;
			m_mMergeCandidatePus[MD_ABOVE] = leftPu;
			leftAndAbove = relationLeftAndAbove(leftEqlAbove);
			aboveAndAR = relationAboveAndAR(aboveEqlAR);
			if (leftAndAbove && aboveAndAR)
			{
				if (leftEqlAbove && aboveEqlAR)
				{
					equalFlag = true;
					return true;
				}
				if (leftEqlAbove == !aboveEqlAR)
				{
					equalFlag = false;
					return true;
				}
				else
					return false;
			}
			else
				return false;
		}
	}
	else
	{
		m_mMergeCandidatePus[MD_ABOVE] = pcAR;
		Bool leftAndAbove;
		leftAndAbove = relationLeftAndAbove(equalFlag);
		if (leftAndAbove)
			return true;
		else
			return false;
	}
}
//B0 & B2
Bool PuUnit::relationALAndAR(Bool &equalFlag)
{
	PuUnit *pcAL, *pcAR;
	pcAL = m_mMergeCandidatePus[MD_ABOVE_LEFT];
	pcAR = m_mMergeCandidatePus[MD_ABOVE_RIGHT];
	if (pcAL == NULL || pcAR == NULL)
	{
		printf("error to load above left or above right PU\n");
		system("pause");
		return false;
	}
	if (pcAR->getPuAbsAddrX() == (m_PuAddInPicX + m_PuWidth))
	{
		PuUnit* leftPu;
		getLeftPu(pcAR, leftPu);
		if (leftPu == NULL || leftPu->getPredMode() == MODE_INTRA)
		{
			if (pcAL->getMergflag() && pcAR->getMergflag())
			{
				if (pcAR->MergeBaseIsSure() && pcAL->MergeBaseIsSure())
				{
					if (pcAR->getBaseMergPu() == pcAL->getBaseMergPu())
					{
						equalFlag = true;
						return true;
					}
					else return false;
				}
				else return false;
			}
			if (pcAR->MergeBaseIsSure() && pcAR->getBaseMergPu() == pcAL)
			{
				equalFlag = true;
				return true;
			}
			else return false;
		}
		else
		{
			Bool AboveAndAR, aboveAndAL;
			Bool aboveEqlAR, aboveEqlAL;
			AboveAndAR = relationAboveAndAR(aboveEqlAR);
			aboveAndAL = relationAboveAndLA(aboveEqlAL);
			if (aboveAndAL && AboveAndAR)
			{
				if (aboveEqlAL && aboveEqlAR)
				{
					equalFlag = true;
					return true;
				}
				else if (aboveEqlAL == !aboveEqlAR)
				{
					equalFlag = false;
					return true;
				}
				else return false;
			}
			else
				return false;
		}
	}
	else
	{
		m_mMergeCandidatePus[MD_ABOVE] = pcAR;
		Bool relationship = relationAboveAndLA(equalFlag);
		if (relationship)
			return true;
		else
			return false;
	}
	return false;
}

PuUnit::PuUnit()
{
}

PuUnit::~PuUnit()
{
}
Void PuUnit::destroy()
{
}

CUData::CUData() :m_cuDepth(0)
, m_PredInf(MODE_NONE)
, m_CBWidth(0)
, m_CBheight(0)
, m_PartSize(SIZE_NONE)
, m_puNums(0)
{
	m_coord[0] = 0;
	m_coord[1] = 0;
}
CUData::~CUData()
{
	UInt puNumber = (UInt)m_PuUnits.size();
	for (UInt i = 0; i < puNumber; i++)
	{
		m_PuUnits[i]->destroy();
		delete m_PuUnits[i];
	}
}
CUData::CUData(UInt wid, UInt hig, UInt x, UInt y, UInt dep) :m_CBWidth(wid),
m_iCBSizeInBits(0),
m_CBheight(hig), 
m_cuDepth(dep), 
m_PartSize(SIZE_NONE),
m_puNums(0)
{
	m_coord[0] = x;
	m_coord[1] = y;
	m_PredInf = MODE_NONE;
	m_curCtu = NULL;
}
Void CUData::GoPUs()
{
	PuUnit* pPuUnit;
	UInt num = 0;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			pPuUnit->PredChang();
			num++;
		}
}

Void CUData::setCBAndCUColor(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv, Pel* Ycom, Pel* UVcom, UInt colorIndex)
{
	UInt num = 0, x = 0, y = 0, colorIdx = colorIndex;
	UInt Xstep, Ystep, pux;
	Pel *pYRow, *pURow, *pVRow;
	UInt xWay = m_coord[0];
	UInt yWay = m_coord[1];
	PuUnit* pPuUnit;
	Int Smvp_num = 0;
	Pel colory = 0, coloru = 0, colorv = 0;
	Pel *UVcomVal = UVcom;
	Pel *YcomVal = Ycom;
	auto beg = m_PuUnits.begin();
	if (m_PredInf == MODE_INTRA)
	{
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
		{
	//			colorIdx = 0;
		}
		colory = ObjectColor[colorIdx][0];
		coloru = ObjectColor[colorIdx][1];
		colorv = ObjectColor[colorIdx][2];

		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = coloru;
		for (UInt i = 0; i < Ystep; i++)
		{
			pURow = Elemu[i] + pux;
			memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
		}

		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = colorv;
		for (UInt i = 0; i < Ystep; i++)
		{
			pVRow = Elemv[i] + pux;
			memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
		}
		if (coloru == 128 && colorv == 128)
		{
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				//memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
				memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
			}
		}
		else
		{
			for (UInt i = 0; i < m_CBWidth; i++)
				YcomVal[i] = colory;
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
				//memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
			}
		}
		return;
	}
	switch (m_PartSize)
	{
		//t: this model is for a skip pu
	case SIZE_NONE:
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
		{
//			colorIdx = 0;
		}
		colory = ObjectColor[colorIdx][0];
		coloru = ObjectColor[colorIdx][1];
		colorv = ObjectColor[colorIdx][2];
		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = coloru;
		for (UInt i = 0; i < Ystep; i++)
		{
			pURow = Elemu[i] + pux;
			memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
		}

		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = colorv;
		for (UInt i = 0; i < Ystep; i++)
		{
			pVRow = Elemv[i] + pux;
			memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
		}
		if (coloru == 128 && colorv == 128)
		{
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				//	memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
				memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
			}
		}
		else
		{
			for (Int i = 0; i < m_CBWidth; i++)
				YcomVal[i] = colory;
			for (int j = 0; j < Ystep * 2; j++)
			{
				pYRow = Elemy[j] + 2 * pux;
				memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
			}
		}
		break;
	case SIZE_2Nx2N:
		UInt testX, testY;
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		pPuUnit->getPuAbsAddrXY(testX, testY);
		/*for test */
		if (pPuUnit->getPuAbsAddrX() == 0 && pPuUnit->getPuAbsAddrY() == 0)
			Int testPoint = 0;
		/**/
		if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
		{
//			colorIdx = 0;
		}
		colory = ObjectColor[colorIdx][0];
		coloru = ObjectColor[colorIdx][1];
		colorv = ObjectColor[colorIdx][2];
		if (pPuUnit->getPuAbsAddrX() == 0 && pPuUnit->getPuAbsAddrY() == 0)
			Int testPoint = 0;
		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = coloru;
		for (UInt i = 0; i < Ystep; i++)
		{
			pURow = Elemu[i] + pux;
			memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
		}

		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = colorv;
		for (UInt i = 0; i < Ystep; i++)
		{
			pVRow = Elemv[i] + pux;
			memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
		}
		if (coloru == 128 && colorv == 128)
		{
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
			}
		}
		else
		{
			for (UInt i = 0; i < m_CBWidth; i++)
				YcomVal[i] = colory;
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
			}
		}

		break;
	case SIZE_2NxN:
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 2;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];

			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
//				colorIdx = 0;
			}
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++)
			{
				pURow = Elemu[i + num*Ystep] + pux;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++)
			{
				pVRow = Elemv[i + num*Ystep] + pux;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
				}
			}
			else
			{
				for (UInt i = 0; i < m_CBWidth; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
				}
			}
			num++;
		}
		break;

	case SIZE_Nx2N:
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];

			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
//				colorIdx = 0;
			}
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++){
				pURow = Elemu[i] + pux + Xstep*num;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++){
				pVRow = Elemv[i] + pux + Xstep*num;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++){
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memset(pYRow, 0, sizeof(Pel)*Xstep * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < Xstep * 2; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++){
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_NxN:
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 2;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
	//			colorIdx = 0;
			}
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++)
			{
				pURow = Elemu[i + (num > 1 ? 1 : 0)*Ystep] + pux + Xstep*(num & 0X01);
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++)
			{
				pVRow = Elemv[i + (num > 1 ? 1 : 0)*Ystep] + pux + Xstep*(num & 0x01);
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i + 2 * (num>1 ? 1 : 0)*Ystep] + 2 * pux + 2 * Xstep*(num & 0x01);
					//	memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
					memset(pYRow, 0, sizeof(Pel)*Xstep * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < Xstep * 2; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i + 2 * (num>1 ? 1 : 0)*Ystep] + 2 * pux + 2 * Xstep*(num & 0x01);
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_2NxnU:
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 3;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
	//			colorIdx = 0;
			}
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
			y = Ystep*(1 + 2 * (num > 0 ? 1 : 0));
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < y; i++){
				pURow = Elemu[i + num*Ystep] + pux;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < y; i++){
				pVRow = Elemv[i + num*Ystep] + pux;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < y * 2; i++)
				{
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memset(pYRow, 0, sizeof(Pel)*Xstep * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < Xstep * 2; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < y * 2; i++)
				{
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_2NxnD:
		Xstep = m_CBWidth >> 1;
		Ystep = (3 * (m_CBheight >> 2)) >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
//				colorIdx = 0;
			}
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
			y = (num == 1 ? (m_CBheight >> 3) : Ystep);
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < y; i++)
			{
				pURow = Elemu[i + num*Ystep] + pux;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < y; i++)
			{
				pVRow = Elemv[i + num*Ystep] + pux;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < y * 2; i++)
				{
					pYRow = Elemy[i + 2 * num*Ystep] + 2 * pux;
					memset(pYRow, 0, sizeof(Pel)*Xstep * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < 2 * Xstep; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < y * 2; i++)
				{
					pYRow = Elemy[i + 2 * num*Ystep] + 2 * pux;
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_nLx2N:

		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
//				colorIdx = 0;
			}
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
			x = (num == 0) ? Xstep : (3 * Xstep);
			for (int i = 0; i < x; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++)
			{
				pURow = Elemu[i] + pux + Xstep*num;
				memcpy(pURow, UVcomVal, sizeof(Pel)*x);
			}

			for (int i = 0; i < x; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++)
			{
				pVRow = Elemv[i] + pux + Xstep*num;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*x);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memset(pYRow, 0, sizeof(Pel)*x * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < x * 2; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memcpy(pYRow, YcomVal, sizeof(Pel)*x * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_nRx2N:
		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
//				colorIdx = 0;
			}
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
			x = (num == 1) ? Xstep : (3 * Xstep);
			for (int i = 0; i < x; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++){
				pURow = Elemu[i] + pux + num * 3 * Xstep;
				memcpy(pURow, UVcomVal, sizeof(Pel)*x);
			}

			for (int i = 0; i < x; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++){
				pVRow = Elemv[i] + pux + num * 3 * Xstep;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*x);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep * 3 * num;
					memset(pYRow, 0, sizeof(Pel)* 2 * x);
				}
			}
			else
			{
				for (UInt i = 0; i < 2 * x; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep * 3 * num;
					memcpy(pYRow, YcomVal, sizeof(Pel)* 2 * x);
				}
			}
			num++;
		}
		break;
	default:
		printf("error size");
		break;
	}
}
/*set Y U V componet color for current CB */
Void CUData::setCBcolor(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv, Pel* Ycom, Pel* UVcom, UInt colorIndex)
{
	UInt num = 0, x = 0, y = 0, colorIdx = 13;
	UInt Xstep, Ystep, pux;
	Pel *pYRow, *pURow, *pVRow;
	UInt xWay = m_coord[0];
	UInt yWay = m_coord[1];
	Int Smvp_num = 0;
	Pel colory = 0, coloru = 0, colorv = 0;
	Pel *UVcomVal = UVcom;
	Pel *YcomVal = Ycom;
	Xstep = m_CBWidth >> 1;
	Ystep = m_CBheight >> 1;
	pux = xWay >> 1;
	//set color index 
	coloru = colorv = 128;
	if (m_iCBSizeInBits > 10)
		colorIdx = 2;
	if (m_cuDepth >= 2)
		colorIdx = 1;
	if (m_iCBSizeInBits > 10 && m_cuDepth >= 2)
		colorIdx = 0;
	//set color to each array 
	colory = ObjectColor[colorIndex][0];
	coloru = ObjectColor[colorIndex][1];
	colorv = ObjectColor[colorIndex][2];
	//u component value set 
	for (int i = 0; i < Xstep; i++)
		UVcomVal[i] = coloru;
	for (UInt i = 0; i < Ystep; i++)
	{
		pURow = Elemu[i] + pux;
		memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
	}
	//v component value set 
	for (int i = 0; i < Xstep; i++)
		UVcomVal[i] = colorv;
	for (UInt i = 0; i < Ystep; i++)
	{
		pVRow = Elemv[i] + pux;
		memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
	}
	//y component value set 
	if (coloru == 128 && colorv == 128)
	{
		for (UInt i = 0; i < Ystep * 2; i++)
		{
			pYRow = Elemy[i] + 2 * pux;
			memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
		}
	}
	// other condition
	else
	{
		for (UInt i = 0; i < m_CBWidth; i++)
			YcomVal[i] = colory;
		for (UInt i = 0; i < Ystep * 2; i++)
		{
			pYRow = Elemy[i] + 2 * pux;
//			memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
		}
	}
	return;
}
//set Y,U,V component value
Void CUData::setYUVcompValue(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv, Pel* Ycom, Pel* UVcom)
{
	UInt num = 0, x = 0, y = 0, colorIdx = 0;
	UInt Xstep, Ystep, pux;
	Pel *pYRow, *pURow, *pVRow;
	UInt xWay = m_coord[0];
	UInt yWay = m_coord[1];
	PuUnit* pPuUnit;
	Int Smvp_num = 0;
	Pel colory = 0, coloru = 0, colorv = 0;
	Pel *UVcomVal = UVcom;
	Pel *YcomVal = Ycom;
	auto beg = m_PuUnits.begin();
	if (m_PredInf == MODE_INTRA)
	{
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
		{
			if (pPuUnit->getStatusIsSure())
				colorIdx = 0;
			else
				colorIdx = 12;
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
		}

		colory = ObjectColor[7][0];
		coloru = ObjectColor[7][1];
		colorv = ObjectColor[7][2];
		if (pPuUnit->getStatus() == STA_BG)
			coloru = colorv = 128;
		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = coloru;
		for (UInt i = 0; i < Ystep; i++)
		{
			pURow = Elemu[i] + pux;
			memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
		}

		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = colorv;
		for (UInt i = 0; i < Ystep; i++)
		{
			pVRow = Elemv[i] + pux;
			memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
		}
		if (coloru == 128 && colorv == 128)
		{
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				//memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
				memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
			}
		}
		else
		{
			for (UInt i = 0; i < m_CBWidth; i++)
				YcomVal[i] = colory;
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
				//memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
			}
		}
		return;
	}
	switch (m_PartSize)
	{
		//t: this model is for a skip pu
	case SIZE_NONE:
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		if (pPuUnit->getStatus() == STA_BG)
		{
			if (pPuUnit->getStatusIsSure())
				colorIdx = 13;
			else
				colorIdx = 13;
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
		}
		if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
		{
			if (pPuUnit->getStatusIsSure())
				colorIdx = 0;
			else
				colorIdx = 12;
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
		}
		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = coloru;
		for (UInt i = 0; i < Ystep; i++)
		{
			pURow = Elemu[i] + pux;
			memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
		}

		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = colorv;
		for (UInt i = 0; i < Ystep; i++)
		{
			pVRow = Elemv[i] + pux;
			memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
		}
		if (coloru == 128 && colorv == 128)
		{
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				//	memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
				memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
			}
		}
		else
		{
			for (Int i = 0; i < m_CBWidth; i++)
				YcomVal[i] = colory;
			for (int j = 0; j < Ystep * 2; j++)
			{
				pYRow = Elemy[j] + 2 * pux;
				memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
			}
		}
		break;
	case SIZE_2Nx2N:
		UInt testX, testY;
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		pPuUnit->getPuAbsAddrXY(testX, testY);
		/*for test */
		if (pPuUnit->getPuAbsAddrX() == 0 && pPuUnit->getPuAbsAddrY() == 0)
			Int testPoint = 0;
		/**/
		if (pPuUnit->getStatus() == STA_BG)
		{
			if (pPuUnit->getStatusIsSure())
				colorIdx = 13;
			else
				colorIdx = 13;
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
		}
		if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
		{
			if (pPuUnit->getStatusIsSure())
				colorIdx = 0;
			else
				colorIdx = 12;
			colory = ObjectColor[colorIdx][0];
			coloru = ObjectColor[colorIdx][1];
			colorv = ObjectColor[colorIdx][2];
		}
		if (pPuUnit->getPuAbsAddrX() == 0 && pPuUnit->getPuAbsAddrY() == 0)
			Int testPoint = 0;
		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = coloru;
		for (UInt i = 0; i < Ystep; i++)
		{
			pURow = Elemu[i] + pux;
			memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
		}

		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = colorv;
		for (UInt i = 0; i < Ystep; i++)
		{
			pVRow = Elemv[i] + pux;
			memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
		}
		if (coloru == 128 && colorv == 128)
		{
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
			}
		}
		else
		{
			for (UInt i = 0; i < m_CBWidth; i++)
				YcomVal[i] = colory;
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
			}
		}

		break;
	case SIZE_2NxN:
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 2;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_BG)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 13;
				else
					colorIdx = 13;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 0;
				else
					colorIdx = 12;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++)
			{
				pURow = Elemu[i + num*Ystep] + pux;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++)
			{
				pVRow = Elemv[i + num*Ystep] + pux;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memset(pYRow, 0, sizeof(Pel)*m_CBWidth);
				}
			}
			else
			{
				for (UInt i = 0; i < m_CBWidth; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
				}
			}
			num++;
		}
		break;

	case SIZE_Nx2N:
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_BG)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 13;
				else
					colorIdx = 13;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 0;
				else
					colorIdx = 12;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++){
				pURow = Elemu[i] + pux + Xstep*num;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++){
				pVRow = Elemv[i] + pux + Xstep*num;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++){
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memset(pYRow, 0, sizeof(Pel)*Xstep * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < Xstep * 2; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++){
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_NxN:
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 2;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_BG)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 13;
				else
					colorIdx = 13;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 0;
				else
					colorIdx = 12;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++)
			{
				pURow = Elemu[i + (num > 1 ? 1 : 0)*Ystep] + pux + Xstep*(num & 0X01);
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++)
			{
				pVRow = Elemv[i + (num > 1 ? 1 : 0)*Ystep] + pux + Xstep*(num & 0x01);
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i + 2 * (num>1 ? 1 : 0)*Ystep] + 2 * pux + 2 * Xstep*(num & 0x01);
					//	memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
					memset(pYRow, 0, sizeof(Pel)*Xstep * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < Xstep * 2; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i + 2 * (num>1 ? 1 : 0)*Ystep] + 2 * pux + 2 * Xstep*(num & 0x01);
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_2NxnU:
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 3;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_BG)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 13;
				else
					colorIdx = 13;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 0;
				else
					colorIdx = 12;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			y = Ystep*(1 + 2 * (num > 0 ? 1 : 0));
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < y; i++){
				pURow = Elemu[i + num*Ystep] + pux;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < y; i++){
				pVRow = Elemv[i + num*Ystep] + pux;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < y * 2; i++)
				{
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memset(pYRow, 0, sizeof(Pel)*Xstep * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < Xstep * 2; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < y * 2; i++)
				{
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_2NxnD:
		Xstep = m_CBWidth >> 1;
		Ystep = (3 * (m_CBheight >> 2)) >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_BG)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 13;
				else
					colorIdx = 13;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 0;
				else
					colorIdx = 12;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			y = (num == 1 ? (m_CBheight >> 3) : Ystep);
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < y; i++)
			{
				pURow = Elemu[i + num*Ystep] + pux;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			}

			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < y; i++)
			{
				pVRow = Elemv[i + num*Ystep] + pux;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < y * 2; i++)
				{
					pYRow = Elemy[i + 2 * num*Ystep] + 2 * pux;
					memset(pYRow, 0, sizeof(Pel)*Xstep * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < 2 * Xstep; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < y * 2; i++)
				{
					pYRow = Elemy[i + 2 * num*Ystep] + 2 * pux;
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_nLx2N:

		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_BG)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 13;
				else
					colorIdx = 13;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 0;
				else
					colorIdx = 12;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			x = (num == 0) ? Xstep : (3 * Xstep);
			for (int i = 0; i < x; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++)
			{
				pURow = Elemu[i] + pux + Xstep*num;
				memcpy(pURow, UVcomVal, sizeof(Pel)*x);
			}

			for (int i = 0; i < x; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++)
			{
				pVRow = Elemv[i] + pux + Xstep*num;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*x);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memset(pYRow, 0, sizeof(Pel)*x * 2);
				}
			}
			else
			{
				for (UInt i = 0; i < x * 2; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memcpy(pYRow, YcomVal, sizeof(Pel)*x * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_nRx2N:
		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getStatus() == STA_BG)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 13;
				else
					colorIdx = 13;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			if (pPuUnit->getStatus() == STA_MOVOBJ || pPuUnit->getStatus() == STA_POSSIBLE_OBJ)
			{
				if (pPuUnit->getStatusIsSure())
					colorIdx = 0;
				else
					colorIdx = 12;
				colory = ObjectColor[colorIdx][0];
				coloru = ObjectColor[colorIdx][1];
				colorv = ObjectColor[colorIdx][2];
			}
			x = (num == 1) ? Xstep : (3 * Xstep);
			for (int i = 0; i < x; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++){
				pURow = Elemu[i] + pux + num * 3 * Xstep;
				memcpy(pURow, UVcomVal, sizeof(Pel)*x);
			}

			for (int i = 0; i < x; i++)
				UVcomVal[i] = colorv;
			for (UInt i = 0; i < Ystep; i++){
				pVRow = Elemv[i] + pux + num * 3 * Xstep;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*x);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep * 3 * num;
					memset(pYRow, 0, sizeof(Pel)* 2 * x);
				}
			}
			else
			{
				for (UInt i = 0; i < 2 * x; i++)
					YcomVal[i] = colory;
				for (UInt i = 0; i < Ystep * 2; i++)
				{
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep * 3 * num;
					memcpy(pYRow, YcomVal, sizeof(Pel)* 2 * x);
				}
			}
			num++;
		}
		break;
	default:
		printf("error size");
		break;
	}
}

//set U, V component value 
Void CUData::setUVComponent(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv, Pel* Ycom, Pel* UVcom)
{
	UInt num = 0, x = 0, y = 0;
	UInt Xstep, Ystep;
	Pel *pYRow, *pURow, *pVRow;
	UInt xWay = m_coord[0];
	UInt yWay = m_coord[1];
	UInt pux;
	PuUnit* pPuUnit;
	Int Smvp_num = 0;
	Pel colory = 0, coloru = 0, colorv = 0, testColor = 25;
	Pel *UVcomVal = UVcom;//new Pel[m_CBWidth]();
	Pel *YcomVal = Ycom;//new Pel[m_CBWidth]();
	memset(UVcom, 0, sizeof(Pel)*m_CBWidth);
	memset(Ycom, 0, sizeof(Pel)*m_CBWidth);
	auto beg = m_PuUnits.begin();
	if (m_PredInf == MODE_INTRA)
	{
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;

		////////////////////////////////////////////////////////////
		coloru = colorv == 0;

		if (//pPuUnit->getStatusIsSure() == true && 
			pPuUnit->getStatus() == STA_BG)
		{
			coloru = colorv = 128;
		}
		if (//pPuUnit->getStatusIsSure() == true && 
			pPuUnit->getStatus() == STA_MOVOBJ  )
		{
			coloru = colorv = 250;
		}
		////////////////////////////////////////////////////////////////////////////////
		if (pPuUnit->getStatus() == STA_BG)
			coloru = colorv = 128;
#ifdef Obj
		if (pPuUnit->getStatusIsSure() == false)
			coloru = colorv = 0;
#endif 
//		if (pPuUnit->getStatus() == STA_MOVOBJ)
//			//coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
//			coloru = colorv = (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
		//coloru = colorv = (40 * ((pPuUnit->getCandArea()->getObject()[0]->getObjectNum()) % 3)) % 200 + 10;

		/////////////////////////////////////////////////////////////////

		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = coloru;
		for (UInt i = 0; i < Ystep; i++)
		{
			pURow = Elemu[i] + pux;
			memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			pVRow = Elemv[i] + pux;
			memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
		}
		if (coloru == 128 && colorv == 128)
		{
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
			}
		}
		return;
	}
	switch (m_PartSize)
	{
	case SIZE_NONE://t: this model is for a skip pu
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		if (pPuUnit->getPuAbsAddrX() == 1248 && pPuUnit->getPuAbsAddrY() == 448)
			Int testPoint = 0;
		////////////////////////////////////////////////////////////
		coloru = colorv == 0;

		if (//pPuUnit->getStatusIsSure() == true && 
			pPuUnit->getStatus() == STA_BG)
		{
			coloru = colorv = 128;
		}
		if (//pPuUnit->getStatusIsSure() == true && 
			pPuUnit->getStatus() == STA_MOVOBJ)
		{
			coloru = colorv == 250;
		}
		////////////////////////////////////////////////////////////////////////////////
		if (pPuUnit->getStatus() == STA_BG)
			coloru = colorv = 128;
#ifdef Obj
		if (pPuUnit->getStatusIsSure() == false)
			coloru = colorv = 0;
#endif 
//		if (pPuUnit->getStatus() == STA_MOVOBJ)
//		{
			//			coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
//				coloru = colorv = (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
//		}

		///////////////////////////////////////////////////////////////
		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = coloru;
		for (UInt i = 0; i < Ystep; i++)
		{
			pURow = Elemu[i] + pux;
			memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			pVRow = Elemv[i] + pux;
			memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
		}
		if (coloru == 128 && colorv == 128)
		{
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
			}
		}
		break;
	case SIZE_2Nx2N:
		UInt testX, testY;
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		pPuUnit->getPuAbsAddrXY(testX, testY);
		if (testX == 616 && testY == 56)
			Int testPoint = 0;
		////////////////////////////////////////////////////////////
		coloru = colorv == 0;

		if (//pPuUnit->getStatusIsSure() == true && 
			pPuUnit->getStatus() == STA_BG)
		{
			coloru = colorv = 128;
		}
		if (//pPuUnit->getStatusIsSure() == true && 
			pPuUnit->getStatus() == STA_MOVOBJ)
		{
			coloru = colorv = 250;
		}
		////////////////////////////////////////////////////////////////////////////////

		if (pPuUnit->getStatus() == STA_BG)coloru = colorv = 128;
#ifdef Obj
		if (pPuUnit->getStatusIsSure() == false)
			coloru = colorv = 0;
#endif 
//		if (pPuUnit->getStatus() == STA_MOVOBJ)
//		{
//			coloru = colorv = 250; (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
			//			coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
			//coloru = colorv = (40 * ((pPuUnit->getCandArea()->getObject()[0]->getObjectNum()) % 5)) % 200 + 10;
//		}

		///////////////////////////////////


		if (pPuUnit->getPuAbsAddrX() == 240 && pPuUnit->getPuAbsAddrY() == 448)
			Int testPoint = 0;
		for (int i = 0; i < Xstep; i++)
			UVcomVal[i] = coloru;
		for (UInt i = 0; i < Ystep; i++)
		{
			pURow = Elemu[i] + pux;
			memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
			pVRow = Elemv[i] + pux;
			memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
		}
		if (coloru == 128 && colorv == 128)
		{
			for (UInt i = 0; i < Ystep * 2; i++)
			{
				pYRow = Elemy[i] + 2 * pux;
				memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
			}
		}
		break;
	case SIZE_2NxN:
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 2;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];

			////////////////////////////////////////////////////////////
			coloru = colorv == 0;

			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_BG)
			{
				coloru = colorv = 128;
			}
			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_MOVOBJ)
			{
				coloru = colorv = 250;
			}
			////////////////////////////////////////////////////////////////////////////////
			if (pPuUnit->getStatus() == STA_BG)coloru = colorv = 128;
#ifdef Obj
			if (pPuUnit->getStatusIsSure() == false)
				coloru = colorv = 0;
#endif 
//			if (pPuUnit->getStatus() == STA_MOVOBJ)
				//			coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
//				coloru = colorv = (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
				//coloru = colorv = (40 * ((pPuUnit->getCandArea()->getObject()[0]->getObjectNum()) % 5)) % 200 + 10;

			////////////////////////////////////////////////////////////
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++)
			{
				pURow = Elemu[i + num*Ystep] + pux;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
				pVRow = Elemv[i + num*Ystep] + pux;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++){
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memcpy(pYRow, YcomVal, sizeof(Pel)*m_CBWidth);
				}
			}
			num++;
		}
		break;

	case SIZE_Nx2N:
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];

			////////////////////////////////////////////////////////////
			coloru = colorv == 0;

			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_BG)
			{
				coloru = colorv = 128;
			}
			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_MOVOBJ)
			{
				coloru = colorv = 250;
			}
			////////////////////////////////////////////////////////////////////////////////
			if (pPuUnit->getStatus() == STA_BG)coloru = colorv = 128;
#ifdef Obj
			if (pPuUnit->getStatusIsSure() == false)
				coloru = colorv = 0;
#endif 
//			if (pPuUnit->getStatus() == STA_MOVOBJ)
//			{
				//				coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
//				coloru = colorv = (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
				//coloru = colorv = (40 * ((pPuUnit->getCandArea()->getObject()[0]->getObjectNum()) % 5)) % 200 + 10;
//			}

			////////////////////////////////////////////////////////////////////////
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++){
				pURow = Elemu[i] + pux + Xstep*num;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
				pVRow = Elemv[i] + pux + Xstep*num;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++){
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_NxN:
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 2;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];

			////////////////////////////////////////////////////////////
			coloru = colorv == 0;

			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_BG)
			{
				coloru = colorv = 128;
			}
			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_MOVOBJ)
			{
				coloru = colorv = 250;
			}
			////////////////////////////////////////////////////////////////////////////////
			if (pPuUnit->getStatus() == STA_BG)coloru = colorv = 128;
#ifdef Obj
			if (pPuUnit->getStatusIsSure() == false)
				coloru = colorv = 0;
#endif 
//			if (pPuUnit->getStatus() == STA_MOVOBJ)
//			{
				//				coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
//				coloru = colorv = (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
				//	coloru = colorv = (40 * ((pPuUnit->getCandArea()->getObject()[0]->getObjectNum()) % 5)) % 200 + 10;
//			}

			//////////////////////////////////////////////////////////////
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++)
			{
				pURow = Elemu[i + (num > 1 ? 1 : 0)*Ystep] + pux + Xstep*(num & 0X01);
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
				pVRow = Elemv[i + (num > 1 ? 1 : 0)*Ystep] + pux + Xstep*(num & 0x01);
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++){
					pYRow = Elemy[i + 2 * (num>1 ? 1 : 0)*Ystep] + 2 * pux + 2 * Xstep*(num & 0x01);
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_2NxnU:
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 3;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];

			////////////////////////////////////////////////////////////
			coloru = colorv == 0;

			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_BG)
			{
				coloru = colorv = 128;
			}
			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_MOVOBJ)
			{
				coloru = colorv = 250;
			}
			////////////////////////////////////////////////////////////////////////////////
			if (pPuUnit->getStatus() == STA_BG)coloru = colorv = 128;
#ifdef Obj
			if (pPuUnit->getStatusIsSure() == false)
				coloru = colorv = 0;
#endif 
//			if (pPuUnit->getStatus() == STA_MOVOBJ)
//			{
				//				coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
//				coloru = colorv = (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
				//	coloru = colorv = (40 * ((pPuUnit->getCandArea()->getObject()[0]->getObjectNum()) % 5)) % 200 + 10;
//			}

			////////////////////////////////////////////////////////////////
			y = Ystep*(1 + 2 * (num > 0 ? 1 : 0));
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < y; i++){
				pURow = Elemu[i + num*Ystep] + pux;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
				pVRow = Elemv[i + num*Ystep] + pux;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < y * 2; i++){
					pYRow = Elemy[i + num*Ystep * 2] + 2 * pux;
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
					//	pPuUnit->add_Ycom(pRow + pux * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_2NxnD:
		Xstep = m_CBWidth >> 1;
		Ystep = (3 * (m_CBheight >> 2)) >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];

			////////////////////////////////////////////////////////////
			coloru = colorv == 0;

			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_BG)
			{
				coloru = colorv = 128;
			}
			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_MOVOBJ)
			{
				coloru = colorv = 250;
			}

			////////////////////////////////////////////////////////////////////////////////
			if (pPuUnit->getStatus() == STA_BG)coloru = colorv = 128;
#ifdef Obj
			if (pPuUnit->getStatusIsSure() == false)
				coloru = colorv = 0;
#endif 
//			if (pPuUnit->getStatus() == STA_MOVOBJ)
//			{
				//				coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
//				coloru = colorv = (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
				//	coloru = colorv = (40 * ((pPuUnit->getCandArea()->getObject()[0]->getObjectNum()) % 5)) % 200 + 10;
//			}

			//////////////////////////////////////////////////////////////////////
			y = (num == 1 ? (m_CBheight >> 3) : Ystep);
			for (int i = 0; i < Xstep; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < y; i++)
			{
				pURow = Elemu[i + num*Ystep] + pux;
				memcpy(pURow, UVcomVal, sizeof(Pel)*Xstep);
				pVRow = Elemv[i + num*Ystep] + pux;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*Xstep);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < y * 2; i++){
					pYRow = Elemy[i + 2 * num*Ystep] + 2 * pux;
					memcpy(pYRow, YcomVal, sizeof(Pel)*Xstep * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_nLx2N:

		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];

			////////////////////////////////////////////////////////////			
			coloru = colorv == 0;

			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_BG)
			{
				coloru = colorv = 128;
			}
			if (//pPuUnit->getStatusIsSure() == true &&
				pPuUnit->getStatus() == STA_MOVOBJ)
			{
				coloru = colorv = 250;
			}
			////////////////////////////////////////////////////////////////////////////////
			if (pPuUnit->getStatus() == STA_BG)
				coloru = colorv = 128;
#ifdef Obj
			if (pPuUnit->getStatusIsSure() == false)
				coloru = colorv = 0;
#endif 
//			if (pPuUnit->getStatus() == STA_MOVOBJ)
//			{
				//				coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
//				coloru = colorv = (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
				//	coloru = colorv = (40 * ((pPuUnit->getCandArea()->getObject()[0]->getObjectNum()) % 5)) % 200 + 10;
//			}

			////////////////////////////////////////////////////////////
			x = (num == 0) ? Xstep : (3 * Xstep);
			for (int i = 0; i < x; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++)
			{
				pURow = Elemu[i] + pux + Xstep*num;
				memcpy(pURow, UVcomVal, sizeof(Pel)*x);
				pVRow = Elemv[i] + pux + Xstep*num;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*x);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++){
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep*num;
					memcpy(pYRow, YcomVal, sizeof(Pel)*x * 2);
				}
			}
			num++;
		}
		break;
	case SIZE_nRx2N:
		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];

			////////////////////////////////////////////////////////////
			coloru = colorv == 0;

			if (//pPuUnit->getStatusIsSure() == true &&
				pPuUnit->getStatus() == STA_BG)
			{
				coloru = colorv = 128;
			}
			if (//pPuUnit->getStatusIsSure() == true && 
				pPuUnit->getStatus() == STA_MOVOBJ)
			{
				coloru = colorv = 250;
			}
			////////////////////////////////////////////////////////////////////////////////
			if (pPuUnit->getStatus() == STA_BG)coloru = colorv = 128;
#ifdef Obj
			if (pPuUnit->getStatusIsSure() == false)
				coloru = colorv = 0;
#endif 
//			if (pPuUnit->getStatus() == STA_MOVOBJ)
//			{
				//				coloru = colorv = (testColor*((pPuUnit->getCurSubArea()->getCurAea()->getAreaNum() % 10) + 1)) % 200 + 10;
//				coloru = colorv = (testColor * ((pPuUnit->getCurSubArea()->getSubAreaNum() % 10) + 1)) % 200 + 40;
				//	coloru = colorv = (40 * ((pPuUnit->getCandArea()->getObject()[0]->getObjectNum()) % 5)) % 200 + 10;
//			}

			////////////////////////////////////////////////////////
			x = (num == 1) ? Xstep : (3 * Xstep);
			for (int i = 0; i < x; i++)
				UVcomVal[i] = coloru;
			for (UInt i = 0; i < Ystep; i++){
				pURow = Elemu[i] + pux + num * 3 * Xstep;
				memcpy(pURow, UVcomVal, sizeof(Pel)*x);
				pVRow = Elemv[i] + pux + num * 3 * Xstep;
				memcpy(pVRow, UVcomVal, sizeof(Pel)*x);
			}
			if (coloru == 128 && colorv == 128)
			{
				for (UInt i = 0; i < Ystep * 2; i++){
					pYRow = Elemy[i] + 2 * pux + 2 * Xstep * 3 * num;
					memcpy(pYRow, YcomVal, sizeof(Pel)* 2 * x);
				}
			}
			num++;
		}
		break;
	default:
		printf("error size");
		break;
	}
//	delete[]UVcomVal;
//	delete[]YcomVal;
}
/*
Void CUData::Classify()
{
	UInt num = 0;
	PuUnit* pPuUnit;
	Int Smvp_num = 0;
	UInt mvdx, mvdy;
	Bool MergFlag = false;
	auto beg = m_PuUnits.begin();
	if (m_PredInf == MODE_INTRA)
	{
		pPuUnit = m_PuUnits[num];
		pPuUnit->setStatus(STA_INTRA);

		return;
	}
	switch (m_PartSize)
	{
	case SIZE_NONE:
		
		pPuUnit = m_PuUnits[num];
		if (pPuUnit->getPuAbsAddrX() == 1232 && pPuUnit->getPuAbsAddrY() == 432)
			Int testPoint = 0;
		pPuUnit->getMergFlag(MergFlag);

		if (MergFlag)
			DealMergeBlock(pPuUnit);
		else
		{
			pPuUnit->getMvd(mvdx, mvdy);
			dealAmvp(pPuUnit, mvdx, mvdy);
		}

		break;
	case SIZE_2Nx2N:
		pPuUnit = m_PuUnits[num];
		pPuUnit->getMergFlag(MergFlag);
		if (pPuUnit->getPuAbsAddrX() == 1248 && pPuUnit->getPuAbsAddrY() == 448)
			Int testPoint = 0;
		if (MergFlag)DealMergeBlock(pPuUnit);
		else
		{
			pPuUnit->getMvd(mvdx, mvdy);
			dealAmvp(pPuUnit, mvdx, mvdy);
		}
		break;
	case SIZE_2NxN:
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			
			pPuUnit->getMergFlag(MergFlag);
			if (MergFlag)DealMergeBlock(pPuUnit);
			else
			{
				pPuUnit->getMvd(mvdx, mvdy);
				dealAmvp(pPuUnit, mvdx, mvdy);
			}
	
			num++;
		}
		break;

	case SIZE_Nx2N:
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			pPuUnit->getMergFlag(MergFlag);
			if (MergFlag)DealMergeBlock(pPuUnit);
			else
			{
				pPuUnit->getMvd(mvdx, mvdy);
				dealAmvp(pPuUnit, mvdx, mvdy);
			}

			num++;
		}
		break;
	case SIZE_NxN:
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			pPuUnit->getMergFlag(MergFlag);
			if (MergFlag)DealMergeBlock(pPuUnit);
			else
			{
				pPuUnit->getMvd(mvdx, mvdy);
				dealAmvp(pPuUnit, mvdx, mvdy);
			}

			num++;
		}
		break;
	case SIZE_2NxnU:
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			
			pPuUnit->getMergFlag(MergFlag);
			if (MergFlag)DealMergeBlock(pPuUnit);
			else
			{
				pPuUnit->getMvd(mvdx, mvdy);
				dealAmvp(pPuUnit, mvdx, mvdy);
			}

			num++;
		}
		break;
	case SIZE_2NxnD:
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			
			pPuUnit->getMergFlag(MergFlag);
			if (MergFlag)DealMergeBlock(pPuUnit);
			else
			{
				pPuUnit->getMvd(mvdx, mvdy);
				dealAmvp(pPuUnit, mvdx, mvdy);
			}

			num++;
		}
		break;
	case SIZE_nLx2N:
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			pPuUnit->getMergFlag(MergFlag);
			if (MergFlag)DealMergeBlock(pPuUnit);
			else
			{
				pPuUnit->getMvd(mvdx, mvdy);
				dealAmvp(pPuUnit, mvdx, mvdy);
			}
	
			num++;
		}
		break;
	case SIZE_nRx2N:
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			if (pPuUnit->getPuAbsAddrX() == 24 && pPuUnit->getPuAbsAddrY() == 672)
				Int testPoint = 0;
			pPuUnit->getMergFlag(MergFlag);
			if (MergFlag)DealMergeBlock(pPuUnit);
			else
			{
				pPuUnit->getMvd(mvdx, mvdy);
				dealAmvp(pPuUnit, mvdx, mvdy);
			}
	
			num++;
		}
		break;
	default:
		printf("error size");
		break;
	}
}*/
/////////////////

//t: set differet clolor to differet block ,not used now
Void CUData::AsigElem(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv)
{
	UInt num = 0, x = 0, y = 0;
	UInt Xstep,Ystep;

	Pel* pRow;
	UInt xWay = m_coord[0];
	UInt yWay = m_coord[1];
	UInt pux;
	PuUnit* pPuUnit;
	Pel colory = 0, coloru = 255, colorv = 255;
	Status tmp_status;
	auto beg = m_PuUnits.begin();
	switch (m_PartSize)
	{
	case SIZE_NONE:
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		tmp_status = pPuUnit->getStatus();
		if (tmp_status == STA_BG)coloru = colorv = 128;
		else  if (tmp_status == STA_MOVOBJ)coloru = colorv = 255;
		else  if (tmp_status == STA_NOTSURE)coloru = colorv = 0;
		for (UInt i = 0; i < Ystep; i++){
			pRow = Elemu[i];
			for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = coloru;
		}
		for (UInt i = 0; i < Ystep; i++){
			pRow = Elemv[i];
			for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = colorv;
		}
		break;
	case SIZE_2Nx2N:
		pPuUnit = m_PuUnits[num];
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		tmp_status = pPuUnit->getStatus();
		if (tmp_status == STA_BG)coloru = colorv = 128;
		if (tmp_status == STA_MOVOBJ)coloru = colorv = 255;
		if (tmp_status == STA_NOTSURE)coloru = colorv = 0;
		
		for (UInt i = 0; i < Ystep; i++){
			pRow = Elemu[i];
			for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = coloru;
		}
		for (UInt i = 0; i < Ystep; i++){
			pRow = Elemv[i];
			for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = colorv;
		}
		break;
	case SIZE_2NxN:
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 2;
		pux = xWay >> 1;
		while (num < m_puNums){
			pPuUnit = m_PuUnits[num];
			tmp_status = pPuUnit->getStatus();
			if (tmp_status == STA_BG)coloru = colorv = 128;
			if (tmp_status == STA_MOVOBJ)coloru = colorv = 255;
			if (tmp_status == STA_NOTSURE)coloru = colorv = 0;
			
			for (UInt i = 0; i < Ystep; i++){
				pRow = Elemu[i + num*Ystep];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = coloru;
			}

			for (UInt i = 0; i < Ystep; i++){
				pRow = Elemv[i + num*Ystep];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = colorv;
			}
			num++;
		}
		break;

	case SIZE_Nx2N:
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums){
			pPuUnit = m_PuUnits[num];
			tmp_status = pPuUnit->getStatus();
			if (tmp_status == STA_BG)coloru = colorv = 128;
			if (tmp_status == STA_MOVOBJ)coloru = colorv = 255;
			if (tmp_status == STA_NOTSURE)coloru = colorv = 0;

			for (UInt i = 0; i < Ystep; i++){
				pRow = Elemu[i];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + Xstep*num + j] = coloru;
			}
			for (UInt i = 0; i < Ystep; i++){
				pRow = Elemv[i];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + Xstep*num + j] = colorv;
			}
			num++;
		}
		break;
	case SIZE_NxN:
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 2;
		pux = xWay >> 1;
		while (num < m_puNums){
			pPuUnit = m_PuUnits[num];
			tmp_status = pPuUnit->getStatus();
			if (tmp_status == STA_BG)coloru = colorv = 128;
			if (tmp_status == STA_MOVOBJ)coloru = colorv = 255;
			if (tmp_status == STA_NOTSURE)coloru = colorv = 0;
			y = (num & 0x02) ? 1 : 0;
			x = (num & 0x01) ? 1 : 0;

			for (UInt i = 0; i < Ystep; i++){
				pRow = Elemu[i + y*Ystep];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + j + x*Xstep] = coloru;
			}
			for (UInt i = 0; i < Ystep; i++){
				pRow = Elemv[i + y*Ystep];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + j + x*Xstep] = colorv;
			}
			num++;
		}
		break;
	case SIZE_2NxnU:
		Xstep = m_CBWidth >> 1;
		Ystep = m_CBheight >> 3;
		pux = xWay >> 1;
		while (num < m_puNums){
			pPuUnit = m_PuUnits[num];
			tmp_status = pPuUnit->getStatus();
			if (tmp_status == STA_BG)coloru = colorv = 128;
			if (tmp_status == STA_MOVOBJ)coloru = colorv = 255;
			if (tmp_status == STA_NOTSURE)coloru = colorv = 0;
			y = Ystep*(1 + 2 * num);

			for (UInt i = 0; i < y; i++){
				pRow = Elemu[i + num*Ystep];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = coloru;
			}
			for (UInt i = 0; i < y; i++){
				pRow = Elemv[i + num*Ystep];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = colorv;
			}
			num++;
		}
		break;
	case SIZE_2NxnD:
		Xstep = m_CBWidth >> 1;
		Ystep = (3 * (m_CBheight >> 2)) >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			tmp_status = pPuUnit->getStatus();
			if (tmp_status == STA_BG)coloru = colorv = 128;
			if (tmp_status == STA_MOVOBJ)coloru = colorv = 255;
			if (tmp_status == STA_NOTSURE)coloru = colorv = 0;
			y = (num == 1 ? (m_CBheight >> 3) : Ystep);

			for (UInt i = 0; i < y; i++)
			{
				pRow = Elemu[i + num*Ystep];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = coloru;
			}

			for (UInt i = 0; i < y; i++)
			{
				pRow = Elemv[i + num*Ystep];
				for (UInt j = 0; j < Xstep; j++)pRow[pux + j] = colorv;
			}
			num++;
		}
		break;
	case SIZE_nLx2N:
		
		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			tmp_status = pPuUnit->getStatus();
			if (tmp_status == STA_BG)coloru = colorv = 128;
			if (tmp_status == STA_MOVOBJ)coloru = colorv = 255;
			if (tmp_status == STA_NOTSURE)coloru = colorv = 0;
			if (num == 0){ x = Xstep; }
			if (num == 1){ x = 3 * Xstep; }

			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = Elemu[i];
				for (UInt j = 0; j < x; j++)pRow[pux + j + Xstep*num] = coloru;
			}

			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = Elemv[i];
				for (UInt j = 0; j < x; j++)pRow[pux + j + Xstep*num] = colorv;
			}

			num++;
		}
		break;
	case SIZE_nRx2N:
		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 1;
		pux = xWay >> 1;
		while (num < m_puNums)
		{
			pPuUnit = m_PuUnits[num];
			tmp_status = pPuUnit->getStatus();
			if (tmp_status == STA_BG)coloru = colorv = 128;
			if (tmp_status == STA_MOVOBJ)coloru = colorv = 255;
			if (tmp_status == STA_NOTSURE)coloru = colorv = 0;
			if (num == 1){ x = Xstep; }
			if (num == 0){ x = 3 * Xstep; }

			for (UInt i = 0; i < Ystep; i++){
				pRow = Elemu[i];
				for (UInt j = 0; j < x; j++)pRow[pux + j + num * 3 * Xstep] = coloru;
			}
			for (UInt i = 0; i < Ystep; i++){
				pRow = Elemv[i];
				for (UInt j = 0; j < x; j++)pRow[pux + j + num * 3 * Xstep] = colorv;
			}
			num++;
		}
		break;
	default:
		printf("error size");
		break;
	}
}


//t: add a new PU to a CB
Void CUData::addNewPart(PuUnit* NewPu)
{
	UInt num = 0, x = 0, y = 0;
	UInt Xstep, Ystep;
	UInt absX, absY;
	PuUnit** pRow;
	UInt xWay = m_coord[0];
	UInt yWay = m_coord[1];
	UInt pux, puy;
	x = xWay >> 2;
	y = yWay >> 2;
	m_curCtu->getAbsXY(absX, absY);
	m_PuUnits.push_back(NewPu);
	NewPu->setCurCtu(m_curCtu);
	try 
	{
		if (m_PredInf == MODE_NONE)
			throw "prediction model not getted yet\n";
	}
	catch (string str)
	{
		std::cout << str << endl;
	}
	NewPu->getNum(num);
	if (m_PredInf == MODE_INTRA)
	{
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 2;
		NewPu->setXY(xWay, yWay);
		NewPu->setPuAbsAddrXY(xWay + absX, yWay + absY);
		NewPu->setPuHeight(m_CBheight);
		NewPu->setPuWidth(m_CBWidth);
		for (UInt i = 0; i < Ystep; i++)
		{
			pRow = m_curCtu->getPup(y + i);
			for (UInt j = 0; j < Xstep; j++)pRow[x + j] = NewPu;
		}
	}

	else if (m_PredInf ==MODE_INTER)
	{
		switch (m_PartSize)
		{
		case SIZE_2Nx2N:
			NewPu->setXY(xWay, yWay);
			NewPu->setPuAbsAddrXY(xWay + absX, yWay + absY);
			Xstep = m_CBWidth >> 2;
			Ystep = m_CBheight >> 2;
			NewPu->setPuHeight(m_CBheight);
			NewPu->setPuWidth(m_CBWidth);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(y + i);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j] = NewPu;
			}

			break;
		case SIZE_2NxN:
			Xstep = m_CBWidth >> 2;
			Ystep = m_CBheight >> 3;
			pux = xWay;
			puy = yWay + num*(m_CBheight >> 1);
			NewPu->setXY(pux, puy);
			NewPu->setPuAbsAddrXY(pux + absX, puy + absY);
			NewPu->setPuHeight(m_CBheight >> 1);
			NewPu->setPuWidth(m_CBWidth);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(y + i + num*Ystep);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j] = NewPu;
			}
			break;
		case SIZE_Nx2N:
			Xstep = m_CBWidth >> 3;
			Ystep = m_CBheight >> 2;
			pux = xWay + num*(m_CBWidth >> 1);
			puy = yWay;
			NewPu->setXY(pux, puy);
			NewPu->setPuAbsAddrXY(pux + absX, puy + absY);
			NewPu->setPuHeight(m_CBheight);
			NewPu->setPuWidth(m_CBWidth >> 1);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(y + i);
				for (UInt j = 0; j < Xstep; j++)pRow[x + Xstep*num + j] = NewPu;
			}
			break;
		case SIZE_NxN:
			Xstep = m_CBWidth >> 3;
			Ystep = m_CBheight >> 3;
			pux = xWay + ((num & 0x01) ? 1 : 0)*(m_CBWidth >> 1);
			puy = yWay + ((num & 0x02) ? 1 : 0)*(m_CBheight >> 1);
			NewPu->setXY(pux, puy);
			NewPu->setPuAbsAddrXY(pux + absX, puy + absY);
			NewPu->setPuHeight(m_CBheight >> 1);
			NewPu->setPuWidth(m_CBWidth >> 1);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(i + ((num & 0x02) ? 1 : 0)*Ystep + y);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j + ((num & 0x01) ? 1 : 0)*Xstep] = NewPu;
			}
			break;
		case SIZE_2NxnU:
			Xstep = m_CBWidth >> 2;
			pux = xWay;
			NewPu->setPuWidth(m_CBWidth);
			if (num == 0)
			{
				y = yWay >> 2;
				Ystep = m_CBheight >> 4;
				puy = yWay;
				NewPu->setPuHeight(m_CBheight >> 2);
			}
			else
			{
				y = (yWay + (m_CBheight >> 2)) >> 2;
				Ystep = 3 * (m_CBheight >> 4);
				puy = yWay + (m_CBheight >> 2);
				NewPu->setPuHeight(3 * (m_CBheight >> 2));
			}
			NewPu->setXY(pux, puy);
			NewPu->setPuAbsAddrXY(pux + absX, puy + absY);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(y + i);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j] = NewPu;
			}
			break;
		case SIZE_2NxnD:
			pux = xWay;
			Xstep = m_CBWidth >> 2;
			NewPu->setPuWidth(m_CBWidth);
			if (num == 0)
			{
				y = yWay >> 2;
				puy = yWay;
				Ystep = 3 * (m_CBheight >> 4);
				NewPu->setPuHeight(3 * (m_CBheight >> 2));
			}
			else
			{
				y = (yWay + m_CBheight - (m_CBheight >> 2)) >> 2;
				puy = yWay + 3 * (m_CBheight >> 2);
				Ystep = m_CBheight >> 4;
				NewPu->setPuHeight(m_CBheight >> 2);
			}
			NewPu->setXY(pux, puy);
			NewPu->setPuAbsAddrXY(pux + absX, puy + absY);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(i + y);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j] = NewPu;
			}
			break;
		case SIZE_nLx2N:
			puy = yWay;
			Ystep = m_CBheight >> 2;
			NewPu->setPuHeight(m_CBheight);
			if (num == 0)
			{
				x = xWay >> 2;
				pux = xWay;
				Xstep = m_CBheight >> 4;
				NewPu->setPuWidth(m_CBWidth >> 2);
			}
			else {
				x = (xWay + (m_CBWidth >> 2)) >> 2;
				pux = xWay + (m_CBWidth >> 2);
				Xstep = 3 * (m_CBWidth >> 4);
				NewPu->setPuWidth(3 * (m_CBWidth >> 2));
			}
			NewPu->setXY(pux, puy);
			NewPu->setPuAbsAddrXY(pux + absX, puy + absY);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(y + i);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j] = NewPu;
			}
			break;
		case SIZE_nRx2N:
			Ystep = m_CBheight >> 2;
			puy = yWay;
			NewPu->setPuHeight(m_CBheight);
			if (num == 0)
			{
				pux = xWay;
				Xstep = 3 * (m_CBWidth >> 4);
				x = xWay >> 2;
				NewPu->setPuWidth(3 * (m_CBWidth >> 2));
			}
			else {
				pux = xWay + m_CBWidth - (m_CBWidth >> 2);
				Xstep = m_CBWidth >> 4;
				x = pux >> 2;
				NewPu->setPuWidth(m_CBWidth >> 2);
			}
			NewPu->setXY(pux, puy);
			NewPu->setPuAbsAddrXY(pux + absX, puy + absY);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(y + i);
				for (UInt j = 0; j < Xstep; j++)
					pRow[x + j] = NewPu;
			}
			break;
		case SIZE_NONE:
			NewPu->setXY(xWay, yWay);
			NewPu->setPuAbsAddrXY(xWay + absX, yWay + absY);
			Xstep = m_CBWidth >> 2;
			Ystep = m_CBheight >> 2;
			NewPu->setPuWidth(m_CBWidth);
			NewPu->setPuHeight(m_CBheight);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(y + i);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j] = NewPu;
			}
			break;
		default:
			printf("error size");
			break;
		}
	}
}

//t: want to set Value to PU_pointers of a LCU
Void CUData::setMinCUs()
{
	UInt num = 0, x = 0, y = 0;
	UInt Xstep, Ystep;
	PuUnit** pRow;
	UInt xWay = m_coord[0];
	UInt yWay = m_coord[1];
	UInt pux, puy;
	PuUnit* pPuUnit;
	x = xWay >> 2;
	y = yWay >> 2;
	switch (m_PartSize)
	{
	case SIZE_2Nx2N:
		pPuUnit = m_PuUnits[num];
		pPuUnit->setXY(xWay,yWay);
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 2;
		for (UInt i = 0; i < Ystep; i++)
		{
			pRow = m_curCtu->getPup(y+i);
			for (UInt j = 0; j < Xstep; j++)pRow[x + j] = pPuUnit;
		}

		break;
	case SIZE_2NxN:
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 3;
		while (num < m_puNums){
			pux = xWay;
			puy = yWay + num*(m_CBheight >> 1);
			pPuUnit = m_PuUnits[num];
			pPuUnit->setXY(pux,puy);
			for (UInt i = 0; i < Ystep; i++){
				pRow = m_curCtu->getPup(y + i + num*Ystep);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j] = pPuUnit;
			}
			num++;
		}
		break;
	case SIZE_Nx2N:
		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 2;
		while (num < m_puNums){
			pPuUnit = m_PuUnits[num];
			pux = xWay + num*(m_CBWidth >> 1);
			puy = yWay;
			pPuUnit->setXY(pux,puy);
			for (UInt i = 0; i < Ystep; i++){
				pRow = m_curCtu->getPup(y+i);
				for (UInt j = 0; j < Xstep; j++)pRow[x + Xstep*num + j] = pPuUnit;
			}
			num++;
		}
		break;
	case SIZE_NxN:
		Xstep = m_CBWidth >> 3;
		Ystep = m_CBheight >> 3;
		while (num < m_puNums){
			pPuUnit = m_PuUnits[num];
			pux = xWay + (num & 0x01)*(m_CBWidth >> 1);
			puy = yWay + (num & 0x02)*(m_CBheight >> 1);
			pPuUnit->setXY(pux,puy);
			for (UInt i = 0; i < Ystep; i++){
				pRow = m_curCtu->getPup(i + (num & 0x02)*Ystep + y);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j + (num & 0x01)*Xstep] = pPuUnit;
			}
			num++;
		}
		break;
	case SIZE_2NxnU:
		Xstep = m_CBWidth>>2;
		pux = xWay;
		while (num < m_puNums){
			pPuUnit = m_PuUnits[num];
			
			if (num == 0){
				y = yWay >> 2;
				Ystep = m_CBheight >> 4;
				puy = yWay;
			}
			else {
				y = (yWay +( m_CBheight >> 2)) >> 2;
				Ystep = 3 * (m_CBheight >> 4);
				puy = yWay + (m_CBheight >> 2);
			}
			pPuUnit->setXY(pux, puy);
			for (UInt i = 0; i < Ystep; i++){
				pRow = m_curCtu->getPup(y + i);
					for (UInt j = 0; j < Xstep; j++)pRow[x + j] = pPuUnit;
				}
			num++;
		}
		break;
	case SIZE_2NxnD:
		pux = xWay;
		Xstep = m_CBWidth >> 2;
		while (num < m_puNums){
			pPuUnit = m_PuUnits[num];
			if (num == 0){
				y = yWay >> 2;
				puy = yWay;
				Ystep = 3 * (m_CBheight >> 4);
			}
			else {
				y = (yWay + m_CBheight - (m_CBheight >> 2)) >> 2;
				puy = yWay + 3 * (m_CBheight >> 2);
				Ystep = m_CBheight >> 4;
			}
			for (UInt i = 0; i < Ystep; i++){
				pRow = m_curCtu->getPup(i + y);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j] = pPuUnit;
			}
			num++;
		}
		break;
	case SIZE_nLx2N:
		puy = yWay;
		Ystep = m_CBheight >> 2;
		while (num < m_puNums){
			pPuUnit = m_PuUnits[num];
			if (num == 0){
				x = xWay >> 2;
				pux = xWay;
				Xstep = m_CBheight >> 4;
			}
			else {
				x = (xWay +(m_CBWidth >> 2)) >> 2;
				pux = xWay + (m_CBWidth >> 2);
				Xstep = 3 * (m_CBWidth >> 4);
			}
			pPuUnit->setXY(pux,puy);
			for (UInt i = 0; i < Ystep; i++){
				pRow = m_curCtu->getPup(y + i);
				for (UInt j = 0; j < Xstep; j++)pRow[x + j ] = pPuUnit;
			}
			num++;
		}
		break;
	case SIZE_nRx2N:
		Ystep = m_CBheight >> 2;
		puy = yWay;
		while (num<m_puNums){
			pPuUnit = m_PuUnits[num];
			if (num == 0){
				pux = xWay;
				Xstep = 3 * (m_CBWidth >> 4);
				x = xWay >> 2;
			}
			else {
				pux = xWay + m_CBWidth - (m_CBWidth >> 2);
				Xstep = m_CBWidth >> 4;
				x = pux >> 2;
			}
			pPuUnit->setXY(pux,puy);
			for (UInt i = 0; i < Ystep; i++)
			{
				pRow = m_curCtu->getPup(y + i);
					for (UInt j = 0; j < Xstep; j++)
						pRow[x + j] = pPuUnit;
			}
			num++;
		}
		break;
	case SIZE_NONE:
		pPuUnit = m_PuUnits[num];
		pPuUnit->setXY(xWay, yWay);
		Xstep = m_CBWidth >> 2;
		Ystep = m_CBheight >> 2;
		for (UInt i = 0; i < Ystep; i++)
		{
			pRow = m_curCtu->getPup(y + i);
			for (UInt j = 0; j < Xstep; j++)pRow[x + j] = pPuUnit;
		}
		break;
	default:
		printf("error size");
		break;
	}
}



CtuData::CtuData(UInt ctuaddr, UInt depth, UInt ctuwid, UInt ctuhig) :m_CtuAddr(ctuaddr)
, m_CtuWidth(ctuwid)
, m_CtuHeight(ctuhig)
, m_depth(depth)
, m_uiSKipPortion(0)
, m_fIntraPortion(0)
, m_uiBitsNum(0)
, m_uiResidBitsNum(0)
, m_bMovingFlag(false)
, m_eCtuStatus(STA_UNSORTED)
, m_eInitialSta(STA_UNSORTED)
, m_bBigMvdFlag(false)
, m_bTraveled(false)
, m_bBorderCtu(false)
, m_bEdgeCtu(true)
, m_bCentCtu(false)
{
	UInt xmin, ymin;
	m_uvWidth = ctuwid >> 1;
	m_uvHeight = ctuhig >> 1;
	m_SliType = UNKNOW;
	m_pPic = NULL;
	m_pCurObj = NULL;
	xmin = ctuwid >> 2;
	ymin = ctuhig >> 2;
	for (UInt i = 0; i < ymin; i++)
	{
		PuUnit **puRow = new PuUnit*[xmin]();
		m_puPointers.push_back(puRow);
	}
}

Void CtuData::createYuvBuf(Bool outputFlag)
{
	for (UInt i = 0; i < m_CtuHeight; i++)
	{
		Pel *OrgRow = new Pel[m_CtuWidth]();
		m_OrgY.push_back(OrgRow);
	}
	for (UInt i = 0; i < m_uvHeight; i++)
	{
		Pel *Row = new Pel[m_uvWidth]();
		memset(Row, 128, sizeof(Pel)*m_uvWidth);
		m_OrgU.push_back(Row);
	}
	for (UInt i = 0; i < m_uvHeight; i++)
	{
		Pel *OrgRow = new Pel[m_uvWidth]();
		memset(OrgRow, 128, sizeof(Pel)*m_uvWidth);
		m_OrgV.push_back(OrgRow);
	}
}

Void CtuData::copyYuvBuf(CtuData* pCtu)
{
	for (UInt i = 0; i < m_CtuHeight; i++)
	{
		m_OrgY.push_back(pCtu->getOrgY(i));
	}
	for (UInt i = 0; i < m_uvHeight; i++)
	{
		m_OrgU.push_back(pCtu->getOrgU(i));
		m_OrgV.push_back(pCtu->getOrgV(i));
	}
}
Void CtuData::freeYuvBuf()
{
	for (UInt i = 0; i < m_CtuWidth; i++)
		delete[]m_OrgY[i];
	for (UInt i = 0; i < m_uvWidth; i++)
	{
		delete[]m_OrgU[i];
		delete[]m_OrgV[i];
	}
}

//CTU dat constructor
CtuData::CtuData() :m_CtuAddr(0)
, m_CtuWidth(0)
, m_CtuHeight(0)
, m_depth(0)
, m_uiBitsNum(0)
, m_uiResidBitsNum(0)
, m_uiSKipPortion(0)
, m_bMovingFlag(false)
, m_eCtuStatus(STA_UNSORTED)
, m_bBigMvdFlag(false)
, m_bTraveled(false)
, m_bBorderCtu(false)
, m_bEdgeCtu(true)
, m_bCentCtu(false)
{
	m_pPic = NULL;
	m_pCurObj = NULL;
}

CtuData::~CtuData()
{
	PuUnit **puRow;
	m_pPic = NULL;
	for (UInt i = 0; i < m_CtuHeight >> 2; i++)
	{
		puRow = m_puPointers[i];
		memset(puRow, 0, sizeof(PuUnit*)*(m_CtuWidth >> 2));
		delete[]puRow;
	}

	UInt cbNum = (UInt)m_CbData.size();
	for (UInt i = 0; i < cbNum; i++)
		delete m_CbData[i];
	m_OrgY.clear();
	m_OrgU.clear();
	m_OrgV.clear();
}
//
Bool CtuData::intraCtu()
{
	if (m_CbData.size() == 1)
	{
		if (m_CbData[0]->getPreMode() == MODE_INTRA)
			return true;
	}
	return false;
}

void CtuData::setOrderOfSmallCtu(UInt order)
{
	if (order > 3)
	{
		cout << "the order of small CTU in a CTU should be range from 0 to 3" << endl;
		exit(0);
	}
	oderOfSmallCtu = order;
}

UInt CtuData::getMaxDepth()
{
	UInt maxDepth = 0;
	UInt tempDepth = 0;
	auto beg = m_CbData.begin();
	while (beg != m_CbData.end())
	{
		tempDepth = (*beg)->getDepth();
		if (tempDepth > maxDepth)
			maxDepth = tempDepth;
		beg++;
	}
	return maxDepth;
}

//void CtuData::totalCuInSmallCtu(std::vector<UInt>& totalNum)	//partition each CTU into four small CTUs											
//{
//	UInt ctuHeight = this->getCtuHeight();
//	UInt ctuAddress = this->getCtuAddr();
//	if (getMaxDepth() < 1 || ctuHeight!=64||ctuAddress>220)
//	{
//		for (int i = 0; i < 4;i++)
//		totalNum.push_back(0);
//		return;
//	}
//
//	int totArea = 0;//the total area of 1/4 CTU,which is 1024
//	CUData* currCU;
//	UInt depth;
//	int area;
//	auto beg = m_CbData.begin();
//	UInt num = 0;
//	while (beg != m_CbData.end())
//	{
//		currCU = *beg;
//		depth = currCU->getDepth();
//		area = pow(2, 6 - depth) * pow(2, 6 - depth);
//		totArea += area;
//		num++;
//		if (totArea == 1024)
//		{
//			totalNum.push_back(num);
//			num = 0;
//			totArea = 0;
//			
//		}
//		beg++;
//	}
//}


//void CtuData::totalCuInSmallCtu(UInt& totalNum, UInt order)
//{
//	vector<UInt> temp;
//	totalCuInSmallCtu(temp);
//	switch (order)
//	{	case 0:
//		totalNum = temp[0]; break;
//		case 1:
//		totalNum = temp[1]; break;
//		case 2:
//		totalNum = temp[2]; break;
//		case 3:
//		totalNum = temp[3]; break;
//	}
//	return;
//
//}


//void CtuData::partInSurroundSmallCtu(UInt order, vector<UInt>& totalNum,UInt& atPicEdge)	//partition in 8 surround small CTUs
//{
//	if (this == NULL)
//	{
//		cout << "the CTU point is NULL, return" << endl;
//		exit(0);
//	}
//	if (order>3)
//	{
//		cout << "order is smaller than zero or larger than 3" << endl;
//		exit(0);
//	}
//
//	CtuData* tempCtu;
//	UInt tempTotal;
//	switch (order)
//	{
//	case 0:
//		tempCtu = getLeftNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 1);
//			totalNum.push_back(tempTotal);
//		}
//		totalCuInSmallCtu(tempTotal, 1);
//		totalNum.push_back(tempTotal);
//		tempCtu = getUpNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 2);
//			totalNum.push_back(tempTotal);
//		}
//		totalCuInSmallCtu(tempTotal, 2);
//		totalNum.push_back(tempTotal);
//
//		tempCtu = getLeftTopNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 3);
//			totalNum.push_back(tempTotal);
//		}
//		tempCtu = getUpNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 3);
//			totalNum.push_back(tempTotal);
//		}
//
//		tempCtu = getLeftNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 3);
//			totalNum.push_back(tempTotal);
//		}
//		totalCuInSmallCtu(tempTotal, 3);
//		totalNum.push_back(tempTotal);
//
//		break;
//	case 1:
//		totalCuInSmallCtu(tempTotal, 0);
//		totalNum.push_back(tempTotal);
//		tempCtu = getRigNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 0);
//			totalNum.push_back(tempTotal);
//		}
//
//		tempCtu = getUpNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 3);
//			totalNum.push_back(tempTotal);
//		}
//		totalCuInSmallCtu(tempTotal, 3);
//		totalNum.push_back(tempTotal);
//
//		tempCtu = getUpNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 2);
//			totalNum.push_back(tempTotal);
//		}
//
//		tempCtu = getRightTopNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 2);
//			totalNum.push_back(tempTotal);
//		}
//		totalCuInSmallCtu(tempTotal, 2);
//		totalNum.push_back(tempTotal);
//		tempCtu = getRigNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 2);
//			totalNum.push_back(tempTotal);
//		}
//		break;
//	case 2:
//		tempCtu = getLeftNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 3);
//			totalNum.push_back(tempTotal);
//		}
//		totalCuInSmallCtu(tempTotal, 3);
//		totalNum.push_back(tempTotal);
//		totalCuInSmallCtu(tempTotal, 0);
//		totalNum.push_back(tempTotal);
//
//		tempCtu = getDownNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 0);
//			totalNum.push_back(tempTotal);
//		}
//
//		tempCtu = getLeftNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 1);
//			totalNum.push_back(tempTotal);
//		}
//		totalCuInSmallCtu(tempTotal, 1);
//		totalNum.push_back(tempTotal);
//
//		tempCtu = getBottomLeftNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 1);
//			totalNum.push_back(tempTotal);
//		}
//
//		tempCtu = getDownNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 1);
//			totalNum.push_back(tempTotal);
//		}
//		break;
//	case 3:
//		totalCuInSmallCtu(tempTotal, 2);
//		totalNum.push_back(tempTotal);
//		tempCtu = getRigNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 2);
//			totalNum.push_back(tempTotal);
//		}
//
//		totalCuInSmallCtu(tempTotal, 1);
//		totalNum.push_back(tempTotal);
//
//		tempCtu = getDownNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 2);
//			totalNum.push_back(tempTotal);
//		}
//		totalCuInSmallCtu(tempTotal, 0);
//		totalNum.push_back(tempTotal);
//
//		tempCtu = getRigNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 0);
//			totalNum.push_back(tempTotal);
//		}
//
//		tempCtu = getDownNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 0);
//			totalNum.push_back(tempTotal);
//		}
//
//		tempCtu = getBotRightNeighbor();
//		if (tempCtu == NULL)
//		{
//			atPicEdge = 1;
//			return;
//		}
//		else
//		{
//			tempCtu->totalCuInSmallCtu(tempTotal, 0);
//			totalNum.push_back(tempTotal);
//		}
//		break;
//	}
//	//cout << "the order is " << order << endl;
//	//cout << "the numbers are " << endl;
//	//for (int i = 0; i < 9; i++)
//		//cout << totalNum[i + 2] << " ";
//		//cout << endl;
//
//}


//	//get neighbor CTUS
CtuData* CtuData::getLeftNeighbor()
{
	UInt ctuInRow = m_pPic->getCtuInRow();

	if (m_CtuAddr%ctuInRow != 0)
	{
		return  m_pPic->getCtu(m_CtuAddr - 1);//left 
	}
	else return NULL;
}
CtuData* CtuData::getLeftTopNeighbor()
{
	UInt ctuInRow = m_pPic->getCtuInRow();

	if (m_CtuAddr%ctuInRow != 0 && m_CtuAddr / ctuInRow > 0)
	{
		return  m_pPic->getCtu(m_CtuAddr - 1 - ctuInRow);//left 
	}
	else return NULL;
}
CtuData* CtuData::getBottomLeftNeighbor()
{
	UInt ctuInRow = m_pPic->getCtuInRow();
	UInt ctuInCol = m_pPic->getCtuInCol();

	if ((m_CtuAddr / ctuInRow) < (ctuInCol - 1) && m_CtuAddr%ctuInRow != 0)//down 
	{
		return  m_pPic->getCtu(m_CtuAddr + ctuInRow - 1);
	}
	else return NULL;
}
CtuData* CtuData::getUpNeighbor()
{
	UInt ctuInRow = m_pPic->getCtuInRow();
	if (m_CtuAddr / ctuInRow > 0)//up
	{
		return m_pPic->getCtu(m_CtuAddr - ctuInRow);
	}
	return NULL;
}

CtuData* CtuData::getRightTopNeighbor()
{
	UInt ctuInRow = m_pPic->getCtuInRow();
	if ((m_CtuAddr%ctuInRow)<(ctuInRow - 1) && (m_CtuAddr / ctuInRow > 0))//up
	{
		return  m_pPic->getCtu(m_CtuAddr - ctuInRow + 1);
	}
	return NULL;
}
//
CtuData* CtuData::getRigNeighbor()
{
	UInt ctuInRow = m_pPic->getCtuInRow();
	if ((m_CtuAddr%ctuInRow) < (ctuInRow - 1))//up
	{
		return  m_pPic->getCtu(m_CtuAddr + 1);
	}
	return NULL;
}

CtuData* CtuData::getDownNeighbor()
{
	UInt ctuInRow = m_pPic->getCtuInRow();
	UInt ctuInCol = m_pPic->getCtuInCol();

	if ((m_CtuAddr / ctuInRow) < (ctuInCol - 1))//down 
	{
		return  m_pPic->getCtu(m_CtuAddr + ctuInRow);
	}
	else return NULL;
}
CtuData* CtuData::getBotRightNeighbor()
{
	UInt ctuInRow = m_pPic->getCtuInRow();
	UInt ctuInCol = m_pPic->getCtuInCol();
	if ((m_CtuAddr / ctuInRow) < (ctuInCol - 1) && (m_CtuAddr%ctuInRow) < (ctuInRow - 1))
	{
		return m_pPic->getCtu(m_CtuAddr + ctuInRow + 1);
	}
	else
		return NULL;
}

CtuData* CtuData::getNeighbourCTU(UInt num)
{

	//5 3 6
	//1 0 2
	//7 4 8
	switch (num)
	{
	case 0:
		return this; break;
	case 1:
		return getLeftNeighbor(); break;
	case 2:
		return getRigNeighbor(); break;
	case 3:
		return getUpNeighbor(); break;
	case 4:
		return getDownNeighbor(); break;
	case 5:
		return getLeftTopNeighbor(); break;
	case 6:
		return getRightTopNeighbor(); break;
	case 7:
		return getBottomLeftNeighbor(); break;
	case 8:
		return  getBotRightNeighbor(); break;
	default:
		cout << "the number is smaller than 0 or larger than 9" << endl;
		exit(0);
		break;
	}
}

//
Void CtuData::possibleMotion()
{
	CtuData *surroundCtu;
	CtuData *corCtu;
	PicData* pPic;
	Bool stopFlag = false, intraFlag = intraCtu();
	Float intraPortion = getIntraPortion();
	UInt ctuInRow = m_pPic->getPicWid() / m_CtuWidth + ((m_pPic->getPicWid() % m_CtuWidth) ? 1 : 0);
	UInt ctuInCol = m_pPic->getPicHig() / m_CtuHeight + ((m_pPic->getPicHig() % m_CtuHeight) ? 1 : 0);
	UInt neighborNum = 0;
	if (m_eCtuStatus == STA_POSSIBLE_OBJ && m_bMovingFlag == true)
	{
		if (m_CtuAddr%ctuInRow != 0)
		{
			surroundCtu = m_pPic->getCtu(m_CtuAddr - 1);//left 
			if (surroundCtu->getCtuSta() == STA_MOVOBJ || surroundCtu->intraCtu() == true)
			{
				neighborNum++;
			}
		}

		if (neighborNum < 2 && (m_CtuAddr%ctuInRow) != (ctuInRow - 1))//right
		{
			surroundCtu = m_pPic->getCtu(m_CtuAddr + 1);
			if (surroundCtu->getCtuSta() == STA_MOVOBJ || surroundCtu->intraCtu() == true)
				neighborNum++;
		}

		if (neighborNum < 2 && (m_CtuAddr / ctuInRow > 0))//up
		{
			surroundCtu = m_pPic->getCtu(m_CtuAddr - ctuInRow);
			if (surroundCtu->getCtuSta() == STA_MOVOBJ || surroundCtu->intraCtu() == true)
				neighborNum++;
		}

		if (neighborNum < 2 && ((m_CtuAddr / ctuInRow) < (ctuInCol - 1)))//down 
		{
			surroundCtu = m_pPic->getCtu(m_CtuAddr + ctuInRow);
			if (surroundCtu->getCtuSta() == STA_MOVOBJ || surroundCtu->intraCtu() == true)
				neighborNum++;
		}

		if (neighborNum == 0)
		{
			m_eCtuStatus = STA_POSSIBLE_BG;
			return;
		}
		else
		{
			UInt corNum = 0;
			pPic = m_pPic->getCurVideoData()->getPic(m_pPic->getPicNum() - 2);//pre 
			corCtu = pPic->getCtu(m_CtuAddr);
			if (corCtu->getCtuSta() == STA_MOVOBJ || corCtu->intraCtu() == true)
			{
				if (m_uiResidBitsNum > 0 || m_CbData.size() > 1 || neighborNum > 1)
					m_eCtuStatus = STA_MOVOBJ;
				else corNum++;
			}
			if (m_eCtuStatus != STA_MOVOBJ)
			{
				pPic = m_pPic->getCurVideoData()->getPic(m_pPic->getPicNum());
				if (pPic == NULL)
					return;
				else
				{
					corCtu = pPic->getCtu(m_CtuAddr);//after 
					if (corCtu->getCtuSta() == STA_MOVOBJ || corCtu->intraCtu() == true)
					{
						if (m_uiResidBitsNum > 0 || m_CbData.size() > 1 || neighborNum > 1 || corNum > 0)
							m_eCtuStatus = STA_MOVOBJ;
					}
					return;
				}
			}
		}
	}
	if ((intraPortion >= 0.6 || intraFlag == true) && m_eCtuStatus != STA_MOVOBJ)
	{
		if (m_CtuAddr%ctuInRow != 0)
		{
			surroundCtu = m_pPic->getCtu(m_CtuAddr - 1);//left 
			if (surroundCtu->getCtuSta() == STA_MOVOBJ && surroundCtu->intraCtu() != true)
				stopFlag = true;
		}

		if (stopFlag == false && (m_CtuAddr%ctuInRow) != (ctuInRow - 1))//right
		{
			surroundCtu = m_pPic->getCtu(m_CtuAddr + 1);
			if (surroundCtu->getCtuSta() == STA_MOVOBJ && surroundCtu->intraCtu() != true)
				stopFlag = true;
		}

		if (stopFlag == false && (m_CtuAddr / ctuInRow > 0))//up
		{
			surroundCtu = m_pPic->getCtu(m_CtuAddr - ctuInRow);
			if (surroundCtu->getCtuSta() == STA_MOVOBJ && surroundCtu->intraCtu() != true)
				stopFlag = true;
		}

		if (stopFlag == false && ((m_CtuAddr / ctuInRow) < (ctuInCol - 1)))//down 
		{
			surroundCtu = m_pPic->getCtu(m_CtuAddr + ctuInRow);
			if (surroundCtu->getCtuSta() == STA_MOVOBJ && surroundCtu->intraCtu() != true)
				stopFlag = true;
		}

		if (stopFlag == false)
		{
			m_eCtuStatus = STA_POSSIBLE_BG;
			return;
		}
		else
		{
			pPic = m_pPic->getCurVideoData()->getPic(m_pPic->getPicNum() - 2);//pre 
			corCtu = pPic->getCtu(m_CtuAddr);
			if (corCtu->getCtuSta() == STA_MOVOBJ && corCtu->intraCtu() != true)
				m_eCtuStatus = STA_MOVOBJ;
			else
			{
				pPic = m_pPic->getCurVideoData()->getPic(m_pPic->getPicNum());
				if (pPic == NULL)
					return;
				else
				{
					corCtu = pPic->getCtu(m_CtuAddr);//after 
					if (corCtu->getCtuSta() == STA_MOVOBJ && corCtu->intraCtu() != true)
						m_eCtuStatus = STA_MOVOBJ;
					return;
				}
			}
		}
	}
}
//check if current ctu is on the edge of current candidate
Void CtuData::checkPosition()
{
	UInt ctuAddr = m_CtuAddr;
	UInt neightborNum = 0, surNum = 0;
	UInt ctuInRow = m_pPic->getCtuInRow(), cutInCol = m_pPic->getCtuInCol();
	CtuData* pSurroundCtu;
	m_bEdgeCtu = true;
	m_bCentCtu = false;
	pSurroundCtu = getLeftNeighbor();//left 
	if (pSurroundCtu != NULL &&pSurroundCtu->getCtuSta() == STA_MOVOBJ)
	{
		++surNum;
		++neightborNum;
	}

	pSurroundCtu = getLeftTopNeighbor();
	if (pSurroundCtu != NULL &&pSurroundCtu->getCtuSta() == STA_MOVOBJ)
	{
		++neightborNum;
	}

	pSurroundCtu = getBottomLeftNeighbor();
	if (pSurroundCtu != NULL &&pSurroundCtu->getCtuSta() == STA_MOVOBJ)
	{
		++neightborNum;
	}

	pSurroundCtu = getUpNeighbor();
	if (pSurroundCtu != NULL &&pSurroundCtu->getCtuSta() == STA_MOVOBJ)
	{
		++surNum;
		++neightborNum;
	}

	pSurroundCtu = getRightTopNeighbor();
	if (pSurroundCtu != NULL &&pSurroundCtu->getCtuSta() == STA_MOVOBJ)
	{
		++neightborNum;
	}

	pSurroundCtu = getRigNeighbor();
	if (pSurroundCtu != NULL &&pSurroundCtu->getCtuSta() == STA_MOVOBJ)
	{
		++surNum;
		++neightborNum;
	}

	pSurroundCtu = getDownNeighbor();
	if (pSurroundCtu != NULL &&pSurroundCtu->getCtuSta() == STA_MOVOBJ)
	{
		++surNum;
		++neightborNum;
	}

	pSurroundCtu = getBotRightNeighbor();
	if (pSurroundCtu != NULL &&pSurroundCtu->getCtuSta() == STA_MOVOBJ)
	{
		++neightborNum;
	}
	if (surNum == 4)
		m_bCentCtu = true;
	if (neightborNum == 8)
		m_bEdgeCtu = false;
}
//



Void CtuData::classifyCtuStatus()
{
	if (m_bMovingFlag)
	{
		if (m_bBigMvdFlag)
		{
			if (m_CbData.size() == 1 && m_uiResidBitsNum < 30)
			{
				m_eInitialSta = m_eCtuStatus = STA_POSSIBLE_OBJ;
			}
			else
			{
				if (m_uiResidBitsNum>20)
					m_eInitialSta = m_eCtuStatus = STA_MOVOBJ;
				else
					m_eInitialSta = m_eCtuStatus = STA_POSSIBLE_OBJ;
			}
		}
		else
		{
			if (m_CbData.size() >= 7)
			{
				if (m_uiResidBitsNum > 10)
					m_eInitialSta = m_eCtuStatus = STA_MOVOBJ;
				else
					m_eInitialSta = m_eCtuStatus = STA_POSSIBLE_OBJ;
			}
			else
			{
				if (m_CbData.size() == 1)
				{
					if (m_uiResidBitsNum > 30)
						m_eInitialSta = m_eCtuStatus = STA_MOVOBJ;
					else
						m_eInitialSta = m_eCtuStatus = STA_POSSIBLE_OBJ;
				}
				else
				{
					if (m_uiResidBitsNum > 20)
						m_eInitialSta = m_eCtuStatus = STA_MOVOBJ;
					else
						m_eInitialSta = m_eCtuStatus = STA_POSSIBLE_OBJ;
				}
			}
		}
	}
	else
	{
		if (m_CbData.size() >= 7 && m_uiResidBitsNum > 60)
		{
			m_eInitialSta = m_eCtuStatus = STA_POSSIBLE_OBJ;
		}
		else
		{
			if (m_uiResidBitsNum > 40)
				m_eInitialSta = m_eCtuStatus = STA_POSSIBLE_OBJ;
			else
				m_eInitialSta = m_eCtuStatus = STA_BG;
		}
	}
}



//
CtuData* CtuData::getCorCtu(Bool subSeqFlag)
{
	PicData* pCorPic;
	UInt picNum = m_pPic->getPicNum();
	VideoData* pCurVideo = m_pPic->getCurVideoData();
	if (subSeqFlag == true)
		pCorPic = pCurVideo->getPic(picNum);
	else
		pCorPic = pCurVideo->getPic(picNum - 2);
	if (pCorPic != NULL)
		return pCorPic->getCtu(m_CtuAddr);
	else
		return NULL;
}
//
Void CtuData::reDealCtu()
{
	PicData *pPrePic, *pNextPic;
	CtuData *corCtu, *leftNeighbor, *rightNeighbor, *upNeighbor, *bottomNeighbor;
	Bool intraFlag = intraCtu();
	auto CuNum = m_CbData.size();
	UInt surroundMovingCtuNum = 0;
	leftNeighbor = getLeftNeighbor();
	rightNeighbor = getRigNeighbor();
	upNeighbor = getUpNeighbor();
	bottomNeighbor = getDownNeighbor();
	pPrePic = m_pPic->getCurVideoData()->getPic(m_pPic->getPicNum() - 2);//pre
	pNextPic = m_pPic->getCurVideoData()->getPic(m_pPic->getPicNum());

	if (leftNeighbor != NULL && leftNeighbor->getCtuSta() == STA_MOVOBJ && leftNeighbor->getBorderFlag() == false)
		++surroundMovingCtuNum;
	if (upNeighbor != NULL && upNeighbor->getCtuSta() == STA_MOVOBJ && upNeighbor->getBorderFlag() == false)
		++surroundMovingCtuNum;
	if (rightNeighbor != NULL && rightNeighbor->getCtuSta() == STA_MOVOBJ && rightNeighbor->getBorderFlag() == false)
		++surroundMovingCtuNum;
	if (bottomNeighbor != NULL && bottomNeighbor->getCtuSta() == STA_MOVOBJ && bottomNeighbor->getBorderFlag() == false)
		++surroundMovingCtuNum;
	if (m_eCtuStatus == STA_MOVOBJ && surroundMovingCtuNum == 4)
	{
		if (pNextPic != NULL)
		{
			corCtu = pNextPic->getCtu(m_CtuAddr);
			if (corCtu->getCtuSta() != STA_MOVOBJ)
				corCtu->setCtuSta(STA_MOVOBJ);
		}
	}
	if (m_eCtuStatus!=STA_MOVOBJ)
	{
		if (surroundMovingCtuNum == 4)
			m_eCtuStatus = STA_MOVOBJ;
		if (surroundMovingCtuNum == 3 && m_eCtuStatus == STA_POSSIBLE_OBJ)
		{
			m_eCtuStatus = STA_MOVOBJ;
		}
	}
}

//search neighbor ctus to merge them together as an object candidate
/*
parameters:
ObjectCandidate: if a neighbor ctus belong to another candidate, merge the two candidates as one
recoverFlag:   if this flag is true, this means the function is used in the step to reset background ctus
				as part of moving object
			if this flag is false, this means the function used in the step that classify ctus only with 
			cus and bits number
*/
Void CtuData::travelNeighborCtu(ObjectCandidate* pObject, Bool recoverFlag)
{
	CtuData*  pNeighborCtu=NULL;
	UInt neighborNum = 0;
	m_bTraveled = true;
	while (neighborNum < 8)
	{
		if (neighborNum == 0)
			pNeighborCtu = getLeftTopNeighbor();
		if (neighborNum == 1)
			pNeighborCtu = getLeftNeighbor();
		if (neighborNum == 2)
			pNeighborCtu = getBottomLeftNeighbor();
		if (neighborNum == 3)
			pNeighborCtu = getUpNeighbor();
		if (neighborNum == 4)
			pNeighborCtu = getRightTopNeighbor();
		if (neighborNum == 5)
			pNeighborCtu = getRigNeighbor();
		if (neighborNum == 6)
			pNeighborCtu = getDownNeighbor();
		if (neighborNum == 7)
			pNeighborCtu = getBotRightNeighbor();
		if (pNeighborCtu != NULL && pNeighborCtu->getCtuSta() == STA_MOVOBJ)
		{
			if (recoverFlag == false)
			{
				if (pNeighborCtu->getObject() != pObject)
				{
					pNeighborCtu->setObject(pObject);
					pObject->addCtuElement(pNeighborCtu);
					pNeighborCtu->travelNeighborCtu(pObject, false);
				}
				else
				{
					if (pNeighborCtu->getTraveledFlag() == false)
						pNeighborCtu->travelNeighborCtu(pObject, false);
				}
			}
			else
			{
				if (pObject != NULL)
				{
					if (pNeighborCtu->getObject() != NULL
						&&pNeighborCtu->getObject()->getMovingOjbect() != NULL
						&&pNeighborCtu->getObject()->getMovingOjbect()->getMovObjectNum() != 0)
					{
						if (pNeighborCtu->getObject()->getMovingOjbect()->getMovObjectNum() == pObject->getMovingOjbect()->getMovObjectNum())
						{
							m_pCurObj = pNeighborCtu->getObject();
							m_pCurObj->addCtuElement(this);
							break;
						}
					}
				}
				else
				{
					if (pNeighborCtu->getObject() != m_pCurObj)
					{
						if (pNeighborCtu->getObject()->getMovingOjbect() != NULL
							&&pNeighborCtu->getObject()->getMovingOjbect()->getMovObjectNum() != 0
							&& pNeighborCtu->getObject()->getMovingOjbect()->getMovObjectNum() == m_pCurObj->getMovingOjbect()->getMovObjectNum())
							pNeighborCtu->getObject()->mergeCandidate(m_pCurObj);
					}
				}
			}
		}
		++neighborNum;
	}
}
//
Void CtuData::ShowDif(UInt &color_num, Pel* Ycom, Pel* UVCom,CtuData *pCtu)
{
	if (m_CbData.empty())return;
	else
	{
		UInt lx, ly, rx, ry;
		CUData *curCB;
		Pel *yRow, *uRow, *vRow;
		PredMode Pre = MODE_NONE;
		UInt colorIndex = 14, CBNum = (UInt)m_CbData.size();
		Bool SkipFlag = false;
		if (m_eCtuStatus == STA_INTRA)
			colorIndex = 12;
		if (m_eCtuStatus == STA_MOVOBJ)
		{
//			if (CBNum<7 && m_uiBitsNum>50)
				colorIndex = 0;
//			else if (CBNum >= 7 && m_uiBitsNum > 40)
//				colorIndex = 1;
		}
		if (m_eCtuStatus == STA_POSSIBLE_OBJ)
		{
			if (m_bMovingFlag == true)
				colorIndex = 1;
			else
				colorIndex = 3;
		}
		if (m_eCtuStatus == STA_POSSIBLE_BG)
		{
			if (m_bMovingFlag == true)
				colorIndex = 4;
			else
				colorIndex = 5;
		}

		auto beg = m_CbData.begin();
		auto end = m_CbData.end();
//		if (colorIndex == 0 || colorIndex == 1 || colorIndex == 2 || colorIndex == 3 || colorIndex == 12 || colorIndex == 4 || colorIndex == 5)
		if (m_eCtuStatus == STA_MOVOBJ)
		{
//			if (m_pCurObj != NULL)
//				colorIndex = m_pCurObj->getObjNum() % 13;
			if (m_pCurObj->getMovingOjbect() != NULL && m_pCurObj->getMovingOjbect()->getMovObjectNum() != 0)
			{
				colorIndex = m_pCurObj->getMovingOjbect()->getMovObjectNum() % 13;
				//if (m_pCurObj->getNextMatch() == true && m_pCurObj->getPriorMatch() == true && m_pCurObj->getUsefulFlag() == true)
				if (m_pCurObj->getUsefulFlag() == true)
					//if (m_pCurObj->getGroupNum() != 0)
					colorIndex = (m_pCurObj->getGroupNum() + 3) % 12;
			}
			else
				SkipFlag = true;
//			if (m_bEdgeCtu == true)
//				colorIndex = 0;
			while (beg != m_CbData.end() && !SkipFlag)
			{
				std::vector<Pel*> Elemy;
				std::vector<Pel*> Elemu;
				std::vector<Pel*> Elemv;
				curCB = *beg;
				Pre = curCB->getPreMode();
				lx = curCB->getx();
				ly = curCB->gety();
				rx = lx + curCB->getCBWidth();
				ry = ly + curCB->getCBHeight();
				{
					Int mid = (ly + ry) / 2;
					for (UInt i = ly; i < ry; i++){
						yRow = pCtu->getOrgY(i);
						Elemy.push_back(yRow);
					}
					for (UInt i = ly >> 1; i < ry >> 1; i++){
						uRow = pCtu->getOrgU(i);
						Elemu.push_back(uRow);
					}
					for (UInt i = ly >> 1; i < ry >> 1; i++){
						vRow = pCtu->getOrgV(i);
						Elemv.push_back(vRow);
					}
					curCB->setCBAndCUColor(Elemy, Elemu, Elemv, Ycom, UVCom, colorIndex);
				}
				beg++;
			}
		}
	}
}
/*
Void CtuData::Classify()
{
	if (m_CbData.empty())
		return;
	else 
	{
		CUData *curCB;
		auto beg = m_CbData.begin();
		auto end = m_CbData.end();
		while (beg != m_CbData.end())
		{
			curCB = *beg;
				curCB->Classify();
			beg++;
		}
	}
}
*/
Void CtuData::GoPuChain(){
	if (m_CbData.empty())return;
	else 
	{

		CUData *curCB;
		PredMode Pre = MODE_NONE;
		auto beg = m_CbData.begin();
		auto end = m_CbData.end();
		while (beg != m_CbData.end())
		{
			curCB = *beg;
			Pre = curCB->getPreMode();
			if (Pre == MODE_INTRA)
			{
				break;
			}
			else {
				curCB->GoPUs();
			}
			beg++;
		}
	}
}


Void CtuData::addOrgYUV(UInt corx, UInt cory, TComYuv* pCom)
{
	UInt y, wid, hig;
	Pel *pDstY;
	Pel *pSrcY;
	wid = pCom->getWidth();
	hig = pCom->getHeight();
	pSrcY = pCom->getLumaAddr(0, wid);
	for (y = cory; y < (cory + hig); y++)
	{
		pDstY = m_OrgY[y];
		pDstY += corx;	
		::memcpy(pDstY, pSrcY, sizeof(Pel)*wid);
		pSrcY += wid;
	}
}
/////////////////////////////


//ObjectCandidate::ObjectCandidate() :m_uiCurObjNum(0)
//, m_uiPicNum(0)
//, m_uiGroupNum(0)
//, m_uiGroupOrder(0)
//, m_bShow(false)
//, m_bTraveled(false)
//, m_bMatchWithPrior(false)
//, m_bMatchWithNext(false)
//, m_bUseFul(true)
//, m_bPotentialHead(false)
//, m_bPotentialTail(false)
//, m_bTrueLeftEdge(false)
//, m_bTrueUpEdge(false)
//, m_bTrueRigEdge(false)
//, m_bTrueBotEdge(false)
//, m_uiCentroidX(0)
//, m_uiCentroidY(0)
//, m_uiLeftEdge(0)
//, m_uiUpEdge(0)
//, m_uiRightEdge(0)
//, m_uiBottomEdge(0)
//, m_enuInheritHorDir(NO_DIRECTION)
//, m_enuInheritVerDir(NO_DIRECTION)
//{
//	m_pcCurPic = nullptr;
//	m_pcMovingObject = nullptr;
//	m_pcNextOjbect = nullptr;
//	m_pcPriorOjbect = nullptr;
//	m_pcPriorMatchCand = nullptr;
//	m_pcNextMatchCand = nullptr;
//	//m_pcCoreSize = nullptr;
//	m_pcSubseqEnd = nullptr;
//	motionLeftAndRight = 0;
//	motionTopAndBottom = 0;
//	deltaX = 0, deltaY = 0; 
//	predX = 0; predY = 0;
//	flagX = 0; flagY = 0;
//	leftRight = false;
//	upDown = false;
//	connectHeadFlag=false;
//	connectTailFlag=false;
//}

ObjectCandidate::ObjectCandidate(PicData* currPic) :m_uiCurObjNum(0)
, m_uiPicNum(0)
, m_uiGroupNum(0)
, m_uiGroupOrder(0)
, m_bShow(false)
, m_bTraveled(false)
, m_bMatchWithPrior(false)
, m_bMatchWithNext(false)
, m_bUseFul(true)
, m_bPotentialHead(false)
, m_bPotentialTail(false)
, m_bTrueLeftEdge(false)
, m_bTrueUpEdge(false)
, m_bTrueRigEdge(false)
, m_bTrueBotEdge(false)
, m_uiCentroidX(0)
, m_uiCentroidY(0)
, m_uiLeftEdge(0)
, m_uiUpEdge(0)
, m_uiRightEdge(0)
, m_uiBottomEdge(0)
, m_enuInheritHorDir(NO_DIRECTION)
, m_enuInheritVerDir(NO_DIRECTION)
{
	m_uiPicNum = currPic->getPicNum();
	m_pcCurPic = currPic;
	currPic->addNewObj(this);
	m_pcMovingObject = nullptr;
	m_pcNextOjbect = nullptr;
	m_pcPriorOjbect = nullptr;
	m_pcPriorMatchCand = nullptr;
	m_pcNextMatchCand = nullptr;
	//m_pcCoreSize = nullptr;
	m_pcSubseqEnd = nullptr;
	motionLeftAndRight = 0;
	motionTopAndBottom = 0;
	deltaX = 0, deltaY = 0;
	predX = 0; predY = 0;
	flagX = 0; flagY = 0;
	leftRight = false;
	upDown = false;
	connectHeadFlag = false;
	connectTailFlag = false;

}

ObjectCandidate::~ObjectCandidate()
{

}
//
Void ObjectCandidate::eraseCtuElement(CtuData* pCtu)
{
	auto beg = m_mCtuElement.begin();
	while (beg != m_mCtuElement.end())
	{
		if (beg->first == pCtu)
		{
			m_mCtuElement.erase(beg);
			return;
		}
		beg++;
	}
}

Void	ObjectCandidate::setMovingObject(MovingObject* pMovObj)
{
	if (m_pcMovingObject!=NULL)
		m_pcMovingObject->eraseCandidate(this);
	
	m_pcMovingObject = pMovObj;
	pMovObj->addCandidate(this->getPicNum(), this);
}

Void ObjectCandidate::clearCtuElements()
{ map<CtuData*, Bool, ctuAddrIncCompare>emptyCtu; m_mCtuElement.clear(); m_mCtuElement.swap(emptyCtu); }

Void ObjectCandidate::setCtuElements(map<CtuData*, Bool, ctuAddrIncCompare> tempCtu)
{ map<CtuData*, Bool, ctuAddrIncCompare>emptyCtu; m_mCtuElement.clear(); m_mCtuElement.swap(emptyCtu); m_mCtuElement.swap(tempCtu); }

Void ObjectCandidate::setPxlElement()
{
	//cout << "beginssssss" << endl;
	UInt ctuSize = m_pcCurPic->getCtuWid();
	UInt ctuInRow = m_pcCurPic->getCtuInRow(), ctuInCol = m_pcCurPic->getCtuInCol();
	UInt totalPicNum = m_pcCurPic->getCurVideoData()->getPicNumInVideo();
	UInt leftEdge, rightEdge, upEdge, bottomEdge, curCtuAddr;// , corCtuAddr;
	UInt verIndex, horIndex, verLength, horLength, horCoord, verCoord;
	UInt movObjNum = m_pcMovingObject->getMovObjectNum();
	//cout << "first dddd" << endl;
	getObjEdge(leftEdge, upEdge, rightEdge, bottomEdge, true);
	//cout << "m_mCtuElement.size() is " << m_mCtuElement.size() << endl;
	//cout << "leftEdge upEdge rightEdge  bottomEdge is " << leftEdge << "  " << upEdge << "  " << rightEdge << "  " << bottomEdge << endl;
	//cout << "second dddd" << endl;
	verLength = bottomEdge - upEdge;
	horLength = rightEdge - leftEdge;
	verIndex = upEdge / ctuSize;
	horIndex = leftEdge / ctuSize;
	std::vector<std::vector<std::pair<Int, Bool>>> tempElement;
	m_vPxlElement.clear();
	tempElement.swap(m_vPxlElement);
	std::vector<std::vector<Int>>				emptyCtuAddr;
	m_vCtuAddr.clear();
	m_vCtuAddr.swap(emptyCtuAddr);
	

	for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		std::vector<std::pair<Int, Bool>> pxlRow;
		for (UInt horIdx = 0; horIdx < horLength; horIdx++)
		{
			std::pair<Int, Bool> pxlEle(-1, false);
			pxlRow.push_back(pxlEle);
		}
		m_vPxlElement.push_back(pxlRow);
	}
	
	for (UInt verIndex = 0; verIndex < verLength / ctuSize; verIndex++)
	{
		std::vector<Int> addrData;
		for (UInt colIndex = 0; colIndex < horLength / ctuSize; colIndex++)
			addrData.push_back(-1);
		m_vCtuAddr.push_back(addrData);
	}
	
	auto beg = m_mCtuElement.begin();
	while (beg != m_mCtuElement.end())
	{
		curCtuAddr = beg->first->getCtuAddr();
		
		horCoord = (curCtuAddr%ctuInRow - horIndex);				//vertical   coordiante in ctu level
		verCoord = (curCtuAddr / ctuInRow - verIndex);				//horizontal coordiante in  ctu level 
		//UInt x=0, y = 0;
		//beg->first->getAbsXY(x,y);
		//horCoord = x / ctuSize - horIndex;
		//verCoord = y / ctuSize - verIndex;
		m_vCtuAddr[verCoord][horCoord] = curCtuAddr;
		horCoord = horCoord*ctuSize;
		verCoord = verCoord*ctuSize;
		for (UInt verIdx = 0; verIdx < ctuSize; verIdx++)
		{
			for (UInt horIdx = 0; horIdx < ctuSize; horIdx++)
			{
				m_vPxlElement[verCoord + verIdx][horCoord + horIdx].first = movObjNum;
				m_vPxlElement[verCoord + verIdx][horCoord + horIdx].second = true;
			}
		}

		++beg;
	}
	
	//for (UInt rowIdx = 0; (rowIdx*ctuSize + m_uiUpEdge) < m_uiBottomEdge; rowIdx++)
	//{
	//	std::vector<Int> rowData;
	//	
	//	for (UInt colIdx = 0; (colIdx*ctuSize + m_uiLeftEdge) < m_uiRightEdge; colIdx++)
	//	{
	//		if (m_vPxlElement[rowIdx*ctuSize][colIdx*ctuSize].first != -1)
	//			rowData.push_back(1);
	//		else
	//			rowData.push_back(-1);
	//	}
	//	//m_vCtuData.push_back(rowData);
	//}

	//cout << "endsssss" << endl;
}
//
Void ObjectCandidate::delBigDelta()
{
	/******************************************
	UInt ctuSize = m_pcCurPic->getCtuHig();
	auto beg = m_vDeltaXY.begin();
	while (beg != m_vDeltaXY.end())
	{
		if (abs(beg->first) > ctuSize || abs(beg->second) > ctuSize)
			beg = m_vDeltaXY.erase(beg);
		else
			++beg;
	}
	if (m_vDeltaXY.size() == 0)
		m_bBigDelta = true;
	****************************************/
}
Bool ObjectCandidate::checkMatchWithGroup(ObjectCandidate* pCorCand, UInt groupSize)//,Int deltaX,Int deltaY)
{
	/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (m_mMotionInf.size() < groupSize)
	{
	return false;
	}
	else
	{
	if (m_uiPicNum == 304)
	Int debugPoint = 0;
	if (m_uiPicNum == 115 && pCorCand->getPicNum() == 113)
	Int debugPoint = 0;
	Bool soloMaxMv = false;
	UInt checkNum = 0, maxMvNum = 0, checkedCandNUm = 0, headPicNum, tailPicNum;
	ObjectCandidate *curCand;
	Int curDeltaX, curDeltaY;
	Float maxProbability = 0;
	std::pair<Int, Int> curMotionVector;
	std::vector<motionInf> curMotionInf;
	std::deque<ObjectCandidate*> check_group;
	std::map<std::pair<Int, Int>, std::vector<ObjectCandidate*>> mvAndCand;
	std::map<std::pair<Int, Int>, Float> mvAndProbability;
	auto rBegIndex = m_mMotionInf.rbegin();

	//search the first match candidate in the group
	while (rBegIndex != m_mMotionInf.rend())
	{
	tailPicNum = rBegIndex->first->getPicNum();
	if (tailPicNum == m_uiLatestMatchedPicNumInorder)
	break;//get the first one that matched with current
	++rBegIndex;
	++checkedCandNUm;
	}
	//no enough candidae
	if ((m_mMotionInf.size() - checkedCandNUm) < groupSize)
	return false;
	auto headCheckCand = rBegIndex;//the first matched in the check group
	auto endCheckCand = rBegIndex;//the last matched in the check group
	//check if the candidates are neighbor
	while (checkNum <= groupSize && rBegIndex != m_mMotionInf.rend())
	{
	if (rBegIndex->second.size() > 0)
	{
	check_group.push_front(rBegIndex->first);
	++checkNum;
	endCheckCand = rBegIndex;
	}
	++rBegIndex;
	}
	//the last one in the group
	headPicNum = endCheckCand->first->getPicNum();
	//they are not neighbors or not enough candidate for judgement
	if ((tailPicNum - headPicNum) > (groupSize + 1) || checkNum < (groupSize - 1))
	return false;
	//first turn
	//check if current group of frames contain the same MV
	endCheckCand = rBegIndex;
	rBegIndex = headCheckCand;//the first matched candidate
	while (rBegIndex != endCheckCand)
	{
	if (rBegIndex->second.size() > 0)
	{
	curCand = rBegIndex->first;
	curMotionInf = rBegIndex->second;
	auto motionfIndex = curMotionInf.begin();
	while (motionfIndex != curMotionInf.end())
	{
	curDeltaX = motionfIndex->deltaX;
	curDeltaY = motionfIndex->deltaY;
	curMotionVector = std::make_pair(curDeltaX, curDeltaY);
	if (mvAndCand.find(curMotionVector) == mvAndCand.end())
	{
	std::vector<ObjectCandidate*> curObjElem;
	curObjElem.push_back(curCand);
	mvAndCand[curMotionVector] = curObjElem;
	mvAndProbability[curMotionVector] = motionfIndex->probaVal;
	}
	else
	{
	mvAndCand[curMotionVector].push_back(curCand);
	mvAndProbability[curMotionVector] += motionfIndex->probaVal;
	}
	++motionfIndex;
	}
	}
	++rBegIndex;
	}

	maxMvNum = 0;
	auto mvIndex = mvAndCand.begin();
	auto maxMvIndex = mvAndCand.begin();
	while (mvIndex != mvAndCand.end())
	{
	if (mvIndex->second.size() == maxMvNum)
	{
	soloMaxMv = false;
	}
	if (mvIndex->second.size() > maxMvNum)
	{
	maxMvNum = (UInt)mvIndex->second.size();
	maxMvIndex = mvIndex;
	soloMaxMv = true;
	}
	++mvIndex;
	}
	//second turn
	//if no mv occured more than two times
	//extend the group
	//check if the candidates are neighbors

	if (maxMvNum <= (groupSize - 2) && rBegIndex != m_mMotionInf.rend())
	{
	//no enough candidate for compare
	//if (m_mMotionInf.size() < (CONSISTAN_GROUP_NUMBER + 1))
	tailPicNum = endCheckCand->first->getPicNum();
	endCheckCand = m_mMotionInf.rend();
	while (rBegIndex != m_mMotionInf.rend())
	{
	if (rBegIndex->second.size() > 0)
	{
	endCheckCand = rBegIndex;
	break;
	}
	++rBegIndex;
	}
	if (endCheckCand == m_mMotionInf.rend())
	return false;
	headPicNum = endCheckCand->first->getPicNum();
	//they are not neighbors
	if ((tailPicNum - headPicNum) > 2)
	return false;
	//	mvAndCand.clear();
	//	checkNum = CONSISTAN_GROUP_NUMBER + 1;
	//	rBegIndex = headCheckCand;//m_mMotionInf.rbegin();
	{
	curCand = endCheckCand->first;
	curMotionInf = endCheckCand->second;
	auto motionfIndex = curMotionInf.begin();
	while (motionfIndex != curMotionInf.end())
	{
	curDeltaX = motionfIndex->deltaX;
	curDeltaY = motionfIndex->deltaY;
	curMotionVector = std::make_pair(curDeltaX, curDeltaY);
	if (mvAndCand.find(curMotionVector) == mvAndCand.end())
	{
	std::vector<ObjectCandidate*> curObjElem;
	curObjElem.push_back(curCand);
	mvAndCand[curMotionVector] = curObjElem;
	mvAndProbability[curMotionVector] = motionfIndex->probaVal;

	}
	else
	{
	mvAndCand[curMotionVector].push_back(curCand);
	mvAndProbability[curMotionVector] += motionfIndex->probaVal;
	}
	++motionfIndex;
	}
	}


	maxMvNum = 0;
	mvIndex = mvAndCand.begin();
	maxMvIndex = mvAndCand.begin();
	while (mvIndex != mvAndCand.end())
	{
	if (mvIndex->second.size() == maxMvNum)
	soloMaxMv = false;
	if (mvIndex->second.size() > maxMvNum)
	{
	maxMvNum = (UInt)mvIndex->second.size();
	maxMvIndex = mvIndex;
	soloMaxMv = true;
	}
	++mvIndex;
	}
	}
	if (maxMvNum > (groupSize - 2))
	{
	std::map<ObjectCandidate*, Bool> curGroupMap;
	std::vector<ObjectCandidate*> curGroupVector;
	std::map<ObjectCandidate*, std::vector<std::pair<Int, Int>>> &objMatchPair = m_pcMovingObject->getCandAndMatchGroups();
	auto mvProbIndex = mvAndProbability.begin();
	mvIndex = mvAndCand.begin();
	soloMaxMv = false;
	maxProbability = 0;
	std::map<std::pair<Int, Int>, Float> curMvAndProb;
	std::vector<std::pair<Int, Int>> curMvs;
	while (mvIndex != mvAndCand.end() && mvProbIndex != mvAndProbability.end())
	{
	if (mvIndex->second.size() == maxMvNum)
	{
	std::pair<Int, Int> curMv = std::make_pair(mvIndex->first.first, mvIndex->first.second);
	if (mvProbIndex->second > maxProbability)
	{
	curGroupMap.clear();
	curMvAndProb.clear();
	soloMaxMv = true;
	maxProbability = mvProbIndex->second;
	curMvAndProb[curMv] = maxProbability;
	}
	if (mvProbIndex->second == maxProbability)
	{
	curMvAndProb[curMv] = maxProbability;
	soloMaxMv = false;
	}

	auto curGroupBeg = mvIndex->second.begin();
	while (curGroupBeg != mvIndex->second.end())
	{
	curGroupMap[*curGroupBeg] = false;
	++curGroupBeg;
	}
	//will be deleted for test if probability will be setted as 00
	if (mvProbIndex->second == 0)
	Int strangePoint = 0;
	}
	++mvProbIndex;
	++mvIndex;
	}
	if (curGroupMap.size() > 0)
	{
	curGroupVector.clear();
	auto curMapBeg = curGroupMap.begin();
	while (curMapBeg != curGroupMap.end())
	{
	curGroupVector.push_back(curMapBeg->first);
	++curMapBeg;
	}
	}
	curGroupMap.clear();
	m_aMapGroups.insert({ curGroupVector, curMvAndProb });
	curMvs.clear();
	objMatchPair[this] = curMvs;
	return true;
	}
	return false;
	}
	return false;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	////
	return false;
}
Void ObjectCandidate::removeSubMatchGroups(std::vector<ObjectCandidate*> subGroup)
{
	if (m_aMapGroups.find(subGroup) != m_aMapGroups.end())
	{
		auto mapGroupBeg = m_aMapGroups.begin();
		while (mapGroupBeg != m_aMapGroups.end())
		{
			if (mapGroupBeg->first == subGroup)
			{
				m_aMapGroups.erase(mapGroupBeg++);
			}
			else
				++mapGroupBeg;
		}
	}
}
//merge current candidate with another candidate 
Void ObjectCandidate::mergeCandidate(ObjectCandidate* pCorObj)
{
	auto beg = m_mCtuElement.begin();
	while (beg != m_mCtuElement.end())
	{
		beg->first->setObject(pCorObj);
		pCorObj->addCtuElement(beg->first);
		++beg;
	}
	m_mCtuElement.clear();
	m_uiCurObjNum = 0;
	m_pcMovingObject = NULL;
}
//


Void ObjectCandidate::getObjCentroid(UInt &centroidX, UInt &centroidY, Bool reCalculate)
{
	if (m_mCtuElement.size() == 0)
	{
		cout << "the object's ctu number is zero, getting object centroid fails, exit" << endl;
		exit(0);
	}
	if (m_uiCentroidX > 0 && m_uiCentroidY > 0 && !reCalculate)
	{
		centroidX = m_uiCentroidX;
		centroidY = m_uiCentroidY;
	}

	else
	{
		CtuData* curCtu = NULL;
		UInt rightEdge = m_pcCurPic->getPicWid();
		UInt bottomEdge = m_pcCurPic->getPicHig();
		UInt ctuSize = m_pcCurPic->getCtuWid();
		UInt ctuArea = ctuSize*ctuSize;
		UInt sumOfWeightX = 0;
		UInt sumOfWeightY = 0;
		UInt sumOfCtu = (UInt)m_mCtuElement.size();
		UInt yPos = 0;
		m_uiCentroidX = 0;
		m_uiCentroidY = 0;

		auto begCtu = m_mCtuElement.begin();
		while (begCtu != m_mCtuElement.end())
		{
			curCtu = begCtu->first;
			yPos = curCtu->getAbsy();
			
			m_uiCentroidX += ctuArea*(curCtu->getAbsx() + ctuSize / 2);
			m_uiCentroidY += ctuArea*(curCtu->getAbsy() + ctuSize / 2);
			sumOfWeightX += ctuArea;
			sumOfWeightY += ctuArea;
		
	
			++begCtu;
		}

		if (sumOfWeightX != 0)
		{
			m_uiCentroidX = UInt(m_uiCentroidX / sumOfWeightX);
			m_uiCentroidY = UInt(m_uiCentroidY / sumOfWeightY);
		}
		if (m_uiCentroidX > rightEdge)m_uiCentroidX = rightEdge;
		if (m_uiCentroidY > bottomEdge) m_uiCentroidY = bottomEdge;
		centroidX = m_uiCentroidX;
		centroidY = m_uiCentroidY;
	}
}

Bool ObjectCandidate::checkCandAtObjectHeadsOrTails(Bool head)
{
	MovingObject *pObject = this->getMovingOjbect();

	vector<UInt> picVectors;
	UInt picNum = 0;

	UInt headPicNum = pObject->getHeadPointer()->getCurPic()->getPicNum();
	UInt tailPicNum = pObject->getTailPointer()->getCurPic()->getPicNum();
	VideoData* pCurVideo = pObject->getHeadPointer()->getCurPic()->getCurVideoData();
	PicData* pCurPic;
	while (headPicNum <= tailPicNum)
	{
		pCurPic = pCurVideo->getPic(headPicNum - 1);

		//ObjectCandidate* pCurObjCand;
		auto beg = pCurPic->getObjects().begin();
		while (beg != pCurPic->getObjects().end())
		{
			if ((*beg)->getMovingOjbect() == pObject)
			{
				picNum = (*beg)->getPicNum();
				auto same = find(picVectors.begin(), picVectors.end(), picNum);
				if (same==picVectors.end())
				picVectors.push_back(picNum);
			}
			++beg;
		}
		++headPicNum;
	}


	sort(picVectors.begin(), picVectors.end());

	UInt currNum = this->getPicNum();

	UInt length = picVectors.size();

	

	if (head == true)
	{
		if (picVectors.size() <= 3)
			return true;

		else if (picVectors.size() > 3 && picVectors.size() <= 8)
		{
			if (currNum <= picVectors[1])
				return true;
			else return false;
		}
		else
		if (picVectors.size() > 8)
		{

			if (currNum <= picVectors[2])
				return true;
			else return false;
		}
		else
		{
			cout << "there is a bug in checkCandAtObjectHeadsOrTails" << endl;
			exit(0);
			return 0;
		}
	}
	else
	{
		if (picVectors.size() <= 3)
			return true;
		else
		if (picVectors.size() > 3 && picVectors.size() <= 8)
		{
			if (currNum >= picVectors[length-2])
				return true;
			else 
				return false;
		}
		else
		if (picVectors.size() > 8)
		{

			if (currNum >= picVectors[length-3])
				return true;
			else 
				return false;
		}
		else
		{
			cout << "there is a bug in checkCandAtObjectHeadsOrTails" << endl;
			exit(0);
			return 0;
		}
	}


}

/*************************************************

***********************************************************/
UInt ObjectCandidate::getEdgeWithDir(UInt edgeDir)
{
	switch (edgeDir)
	{
	case 1:
		return m_uiLeftEdge;
		break;
	case 2:
		return m_uiRightEdge;
		break;
	case 3:
		return m_uiUpEdge;
		break;
	case 4:
		return m_uiBottomEdge;
		break;
	default:
		printf("error direction \n");
		return 65536;
		break;
	}
}
//get currente edge
Void ObjectCandidate::getObjEdge(UInt &leftEdge, UInt &upEdge, UInt &rightEdge, UInt &botEdge, Bool reCalculate)
{
	if (m_mCtuElement.size() == 0)
	{
		cout << "the object's ctu number is zero, getting object edge fails, exit" << endl;
		exit(0);
	}
	if (m_uiLeftEdge != 0 && m_uiUpEdge != 0 && m_uiRightEdge != 0 && m_uiBottomEdge != 0 && !reCalculate)
	{
		leftEdge = m_uiLeftEdge;
		upEdge = m_uiUpEdge;
		rightEdge = m_uiRightEdge;
		botEdge = m_uiBottomEdge;
	}
	else
	{
		CtuData* curCtu = NULL;
		
		UInt ctuSize = m_pcCurPic->getCtuWid(), ctuCoordX = 0, ctuCoordY = 0;
		m_uiLeftEdge = m_pcCurPic->getPicWid();
		m_uiUpEdge = m_pcCurPic->getPicHig();
		m_uiRightEdge = m_uiBottomEdge = 0;

		//PicData*	currPic = getCurPic();
		//UInt ctuInRow = currPic->getCtuInRow();
		auto begCtu = m_mCtuElement.begin();
		while (begCtu != m_mCtuElement.end())
		{
			curCtu = begCtu->first;
			curCtu->getAbsXY(ctuCoordX, ctuCoordY);
			//UInt address=curCtu->getCtuAddr();
			//ctuCoordX = (address %ctuInRow)*ctuSize;
			//ctuCoordY = (address /ctuInRow)*ctuSize;
			m_uiUpEdge = ctuCoordY < m_uiUpEdge ? ctuCoordY : m_uiUpEdge;
			m_uiLeftEdge = ctuCoordX < m_uiLeftEdge ? ctuCoordX : m_uiLeftEdge;
			m_uiRightEdge = (ctuCoordX + ctuSize) > m_uiRightEdge ? (ctuCoordX + ctuSize) : m_uiRightEdge;
			m_uiBottomEdge = (ctuCoordY + ctuSize) > m_uiBottomEdge ? (ctuCoordY + ctuSize) : m_uiBottomEdge;
			++begCtu;
		}
		leftEdge = m_uiLeftEdge;
		upEdge = m_uiUpEdge;
		rightEdge = m_uiRightEdge;
		botEdge = m_uiBottomEdge;

	}


}
Bool ObjectCandidate::atTheBorder(UInt &leftDir, UInt &upDir, UInt &rigDir, UInt &botDir)
{
	UInt picWidth = m_pcCurPic->getPicWid();
	UInt picHeigt = m_pcCurPic->getPicHig();
	Bool edegExit = false;
	getObjEdge(m_uiLeftEdge, m_uiUpEdge, m_uiRightEdge, m_uiBottomEdge, false);
	leftDir = m_uiLeftEdge == 0 ? 1 : 0;
	upDir = m_uiUpEdge == 0 ? 2 : 0;
	rigDir = m_uiRightEdge == picWidth ? 3 : 0;
	botDir = m_uiBottomEdge == picHeigt ? 4 : 0;
	return (leftDir + upDir + rigDir + botDir) > 0 ? true : false;
}
//
Bool ObjectCandidate::bigGapWithNeighbors(ObjectCandidate *pPriorCand, ObjectCandidate *pNextCand, UInt checkDir)
{
	UInt ctuSize;
	Int curEdge, priorEdge, nextEdge;
	ctuSize = m_pcCurPic->getCtuHig();

	curEdge = (Int)this->getEdgeWithDir(checkDir);
	nextEdge = (Int)pNextCand->getEdgeWithDir(checkDir);
	if (abs(curEdge - nextEdge) > ctuSize)
	{
		if (pPriorCand == this)
			return true;
		else
		{
			priorEdge = pPriorCand->getEdgeWithDir(checkDir);
			return minORmax(priorEdge, curEdge, nextEdge);
		}
	}
	else
		return false;
}
Bool ObjectCandidate::priorAndNextHaveSomeBoundry(ObjectCandidate* pPrior, ObjectCandidate *pNext, UInt checkDir)
{
	UInt ctuSize;////////////////////
	Int curEdge, priorBoundry, nextBoundry;
	if (pNext == NULL)
		return false;
	ctuSize = m_pcCurPic->getCtuHig();/////////////////////////////////////////////////////////
	curEdge = (Int)getEdgeWithDir(checkDir);
	nextBoundry = (Int)pNext->getEdgeWithDir(checkDir);
	priorBoundry = (Int)pPrior->getEdgeWithDir(checkDir);
	if (nextBoundry == priorBoundry)
		return true;
	else
		return false;
}

Bool ObjectCandidate::calculateBoundingBoxOverlaps(ObjectCandidate* pCorObject, Float &ratio1, Float &ratio2)
{
	Bool answer = false;

	UInt leftEdge1, upEdge1, rightEdge1, bottomEdge1 = 0;
	UInt leftEdge2, upEdge2, rightEdge2, bottomEdge2 = 0;
	UInt Width1, Height1 ,area1= 0;
	UInt Width2, Height2 ,area2= 0;


	

	ratio1 = 0, ratio2 = 0; 

	this->getObjEdge(leftEdge1, upEdge1, rightEdge1, bottomEdge1,false);
	Width1 = rightEdge1 - leftEdge1;
	Height1 = bottomEdge1 - upEdge1;
	pCorObject->getObjEdge(leftEdge2, upEdge2, rightEdge2, bottomEdge2, false);
	Width2 = rightEdge2 - leftEdge2;
	Height2 = bottomEdge2 - upEdge2;

	UInt overLapleftEdge, overLapUpEdge, overLapRightEdge, overLapBottomEdge = 0;
	UInt overLapWidth, overLapHeight = 0;
	UInt overLapArea = 0;

	overLapleftEdge = leftEdge1 > leftEdge2 ? leftEdge1 : leftEdge2;
	overLapUpEdge = upEdge1 > upEdge2 ? upEdge1 : upEdge2;
	overLapRightEdge = rightEdge1 < rightEdge2 ? rightEdge1 : rightEdge2;
	overLapBottomEdge = bottomEdge1 < bottomEdge2 ? bottomEdge1 : bottomEdge2;
	

	if (overLapleftEdge >= overLapRightEdge || overLapUpEdge >= overLapBottomEdge)
		answer= false;
	else
	{
		overLapWidth = overLapRightEdge - overLapleftEdge;
		overLapHeight = overLapBottomEdge - overLapUpEdge;
		overLapArea = overLapWidth*overLapHeight;

		area1 = Width1*Height1;
		area2 = Width2*Height2;
		ratio1 = Float(overLapArea) / Float(area1);
		ratio2 = Float(overLapArea) / Float(area2);

		if ((ratio1 >= 0.6 && ratio2 >= 0.1) || (ratio1 >= 0.1 && ratio2 >= 0.6))
			answer=true;
		else
			answer= false;


	}

	if (answer == true)
		return true;
	else
	{
		std::map<CtuData*, Bool, ctuAddrIncCompare> obj1CtuData = this->getCtuElements();
		std::map<CtuData*, Bool, ctuAddrIncCompare> obj2CtuData = pCorObject->getCtuElements();

		map<UInt, Bool> ctuAddress1, ctuAddress2;
		auto beg = obj1CtuData.begin();
		while (beg != obj1CtuData.end())
		{
			ctuAddress1.insert(make_pair(beg->first->getCtuAddr(), true));
			beg++;
		}

		beg = obj2CtuData.begin();
		while (beg != obj2CtuData.end())
		{
			ctuAddress2.insert(make_pair(beg->first->getCtuAddr(), true));
			beg++;
		}

		UInt totalCtuNum1 = obj1CtuData.size();
		UInt totalCtuNum2 = obj2CtuData.size();

		if (totalCtuNum1 == 0 || totalCtuNum2 == 0)
		{
			cout << "ctu1 number or ctu2 number is zero, error, exit" << endl;
			exit(0);
		}

		UInt pubCtuNum1 = 0, pubCtuNum2 = 0;

		auto begAddr = ctuAddress1.begin();
		while (begAddr != ctuAddress1.end())
		{
			if (ctuAddress2.find(begAddr->first) != ctuAddress2.end())
			{
				pubCtuNum1++;
				pubCtuNum2++;
			}
			begAddr++;
		}

		UInt privateNum1 = totalCtuNum1 - pubCtuNum1;
		UInt privateNum2 = totalCtuNum2 - pubCtuNum2;

		Float ctuRatio1 = Float(pubCtuNum1) / Float(totalCtuNum1);
		Float ctuRatio2 = Float(pubCtuNum2) / Float(totalCtuNum2);

		if (ctuRatio1 >= 0.7 || ctuRatio2 >= 0.7 || (pubCtuNum1>privateNum1) || (pubCtuNum2>privateNum2) )
			answer = true;

		return answer;

	}



}


Bool ObjectCandidate::addCtuAccordingToBoundingBox(UInt leftEdge_Curr, UInt upEdge_Curr, UInt rightEdge_Curr, UInt botEdge_Curr, CtuData* ctu, ObjectCandidate* pSmallCand, UInt leftEdge_Large, UInt  upEdge_Large, UInt rightEdge_Large, UInt  botEdge_Large)
{
	Bool match = false;
	UInt leftEdge = 0, upEdge = 0, rightEdge = 0, botEdge = 0;
	pSmallCand->getObjEdge(leftEdge, upEdge, rightEdge, botEdge, true);
	

	if (leftEdge < leftEdge_Large)
		leftEdge = leftEdge_Large;
	if (upEdge < upEdge_Large)
		upEdge = upEdge_Large;
	if (rightEdge > rightEdge_Large)
		rightEdge = rightEdge_Large;
	if (botEdge  > botEdge_Large)
		botEdge = botEdge_Large;

	Float leftRatio = Float(leftEdge - leftEdge_Large) / Float(rightEdge_Large - leftEdge_Large);
	Float rightRatio = Float(rightEdge - leftEdge_Large) / Float(rightEdge_Large - leftEdge_Large);
	Float upRatio = Float(upEdge - upEdge_Large) / Float(botEdge_Large - upEdge_Large);
	Float botRatio = Float(botEdge - upEdge_Large) / Float(botEdge_Large - upEdge_Large);

	UInt x=0, y=0;
	ctu->getAbsXY(x,y);
	//cout << "pLargeCand pic num is " << pLargeCand->getPicNum() << endl;
	//cout << "pLargeCand ctu size is " << pLargeCand->getCtuElements().size() << endl;


	/*cout << "candidate ctu size is " << pSmallCand->getCtuElements().size() << endl;
	cout << "candidate pic num is " << pSmallCand->getPicNum() << endl;
	cout << "the current pic num is " << this->getPicNum() << endl;
	cout << "large left, up, right, bottom is " << leftEdge_Large << "  " << upEdge_Large << "  " << rightEdge_Large << "   " << botEdge_Large << endl;
	cout << "candidate left, up, right, bottom is "<<leftEdge <<"  "<< upEdge<<"  " << rightEdge<<"   " << botEdge << endl;
	cout << " leftRatio rightRatio  upRatio  botRatio " << leftRatio<<"  " << rightRatio <<"  "<< upRatio<<"  " << botRatio << endl;*/
	//UInt leftEdge_Curr = 0, upEdge_Curr = 0, rightEdge_Curr = 0, botEdge_Curr = 0;
	//this->getObjEdge(leftEdge_Curr, upEdge_Curr, rightEdge_Curr, botEdge_Curr, true);
	if ((x - leftEdge_Curr) >= (leftRatio*Float(rightEdge_Curr - leftEdge_Curr)) && (x - leftEdge_Curr) <= (rightRatio*Float(rightEdge_Curr - leftEdge_Curr)))
	if ((y - upEdge_Curr) >= (upRatio*Float(botEdge_Curr - upEdge_Curr)) && (y - upEdge_Curr) <= (botRatio*Float(botEdge_Curr - upEdge_Curr)))
		match = true;
	//cout << "match is " << match << endl;
	//cout << endl;
	//int aa; cin >> aa;
	return match;


}





//calculate pading pixel with corresponding 
Void ObjectCandidate::calculateOverlaps(ObjectCandidate* pCorObject, Float &padingPixel, Bool fractionFlag)
{
	//cout << "begin calculate over" << endl;
	CtuData* curCtu;
	Float padPixel = 0;
	UInt tempCentroidX, tempCentroidY;
	UInt curLeftX, curUpY, curRightX, curBottomY;
	UInt corLeftX, corUpY, corRightX, corBottomY;
	UInt leftBound, rightBound, upBound, bottomBound;
	Int deltaX, deltaY, startX, startY;
	UInt pixelVal = 0, curCtuNum = 0;
	UInt ctuSize = m_pcCurPic->getCtuWid();
	UInt ctuArea = ctuSize* ctuSize;
	UInt tempPicRadius, newRad;
	Pxl** tempPic;
	getObjCentroid(tempCentroidX, tempCentroidY, false);
	pCorObject->getObjCentroid(tempCentroidX, tempCentroidY, false);
	getObjEdge(curLeftX, curUpY, curRightX, curBottomY, false);
	pCorObject->getObjEdge(corLeftX, corUpY, corRightX, corBottomY, false);


	leftBound = curLeftX < corLeftX ? curLeftX : corLeftX;
	upBound = curUpY < corUpY ? curUpY : corUpY;
	rightBound = curRightX>corRightX ? curRightX : corRightX;
	bottomBound = curBottomY>corBottomY ? curBottomY : corBottomY;

	newRad = (m_uiCentroidX - leftBound) > (rightBound - m_uiCentroidX) ? (m_uiCentroidX - leftBound) : (rightBound - m_uiCentroidX);
	newRad = (m_uiCentroidY - upBound) > newRad ? (m_uiCentroidY - upBound) : newRad;
	newRad = (bottomBound - m_uiCentroidY) > newRad ? (bottomBound - m_uiCentroidY) : newRad;

	deltaX = abs((Int)m_uiCentroidX - (Int)tempCentroidX);
	deltaY = abs((Int)m_uiCentroidY - (Int)tempCentroidY);
	deltaX = deltaX > deltaY ? deltaX : deltaY;
	newRad += deltaX;
	//forgot what this statement for 
	newRad = ctuSize * (newRad / ctuSize + 1);
	tempPicRadius = newRad;
	tempPic = new Pxl*[2 * tempPicRadius]();
	for (UInt i = 0; i < tempPicRadius * 2; i++)
		tempPic[i] = new Pxl[tempPicRadius * 2]();
	deltaX = tempPicRadius - (Int)m_uiCentroidX;
	deltaY = tempPicRadius - (Int)m_uiCentroidY;
	if (fractionFlag == true)
	{
		deltaX = tempPicRadius - (Int)tempCentroidX;
		deltaY = tempPicRadius - (Int)tempCentroidY;
	}
	auto begCtu = m_mCtuElement.begin();
	while (begCtu != m_mCtuElement.end())
	{
		curCtu = begCtu->first;
		startX = curCtu->getAbsx();
		startY = curCtu->getAbsy();
		startX = (Int)startX + deltaX;
		startY = (Int)startY + deltaY;
		for (Int row = startY;row < startY + ctuSize; row++)
		{
			for (Int col = startX; col < startX + ctuSize; col++)
			{
				tempPic[row][col] += 0x01;
			}
		}
		++begCtu;
	}

	deltaX = tempPicRadius - (Int)tempCentroidX;
	deltaY = tempPicRadius - (Int)tempCentroidY;

	std::map<CtuData*, Bool, ctuAddrIncCompare> corCtuElem = pCorObject->getCtuElemRef();
	auto begCorCtu = corCtuElem.begin();
	while (begCorCtu != corCtuElem.end())
	{
		curCtu = begCorCtu->first;
		startX = curCtu->getAbsx();
		startY = curCtu->getAbsy();
		startX = (Int)startX + deltaX;
		startY = (Int)startY + deltaY;
		for (Int row = startY; row < startY + ctuSize; row++)
		{
			for (Int col = startX; col < startX + ctuSize; col++)
			{
//				tempPic[row][col] += 0x02;
				if (tempPic[row][col] == 0x01)
					++padPixel;
			}
		}
		++begCorCtu;
	}
	padingPixel = padPixel;
	for (UInt i = 0; i < tempPicRadius * 2; i++)
	{
		delete[]tempPic[i];
		tempPic[i] = NULL;
	}
	delete[]tempPic;
	tempPic = NULL;

	//cout << "end calculate over" << endl;
}
//calculate overlapping area with one boundry coincide with another candidate
Bool ObjectCandidate::calMatchWithDirection(ObjectCandidate* pCorObj)//, UInt direction)
{
	UInt leftEdge, upEdge, rigEdge, botEdge;
	Int deltaX, deltaY, corStartX, corEndY, curStartX;// , curEndY;
	Float maxPadingPxl = 0, sizeRatio, ctuSize, curSize, corSize;
	std::vector<std::vector<std::pair<Int, Bool>>>& pxlElement = pCorObj->getPxlElement();
	ctuSize = (Float)m_pcCurPic->getCtuHig();
	ctuSize = ctuSize*ctuSize;
	curSize = ctuSize*m_mCtuElement.size();
	corSize = pCorObj->getCtuElements().size()*ctuSize;
	sizeRatio = curSize > corSize ? corSize / curSize : curSize / corSize;
	pCorObj->getObjEdge(leftEdge, upEdge, rigEdge, botEdge, false);
	deltaX = m_uiRightEdge - rigEdge;
	deltaY = m_uiUpEdge - upEdge;
	if (((Int)leftEdge + deltaX) < (Int)m_uiLeftEdge)
	{
		corStartX = (rigEdge - leftEdge) - (m_uiRightEdge - m_uiLeftEdge);
		curStartX = 0;
	}
	else
	{
		corStartX = 0;
		curStartX = m_uiRightEdge - m_uiLeftEdge - (rigEdge - leftEdge);
	}
	if (((Int)botEdge + deltaY) < (Int)m_uiBottomEdge)
		corEndY = botEdge - upEdge;
	else
		corEndY = m_uiBottomEdge - m_uiUpEdge;
	for (UInt verIdx = 0; verIdx < corEndY; verIdx++)
	{
		for (UInt horIdx = 0; (curStartX + horIdx) < (m_uiRightEdge - m_uiLeftEdge); horIdx++)
		if (m_vPxlElement[verIdx][curStartX + horIdx].first != -1 && pxlElement[verIdx][corStartX + horIdx].first != -1)
		{
			++maxPadingPxl;
		}
	}
	if (maxPadingPxl / corSize >= 0.65 && maxPadingPxl / curSize >= 0.65 && sizeRatio>0.65)
	{
		std::ofstream deltaFile;
		std::string name = "obj";//objNumDelt
		UInt objNum = m_pcMovingObject->getMovObjectNum();
		UInt groupNum = m_uiGroupNum;
		Char ObjNumBuf[12], groupNumBuf[12];
		std::vector<std::pair<Int, Int>> tempDelta;
		_itoa(groupNum, groupNumBuf, 10);
		_itoa(objNum, ObjNumBuf, 10);
		m_pcMovingObject->addCandidatePair(this, pCorObj);
		name.append(ObjNumBuf);
		name = name + ObjNumBuf + ".txt";
		deltaFile.open(name, ios::out|ios::app);
		deltaFile << "frame number frome  :" << m_pcCurPic->getPicNum() << '\t' << pCorObj->getCurPic()->getPicNum() << endl;
		deltaFile << deltaX << ' ' << deltaY << endl;
		printf("obj: %d . frame number: %d to %d\n ", m_pcMovingObject->getMovObjectNum(), m_pcCurPic->getPicNum(), pCorObj->getCurPic()->getPicNum());
		printf("deltax = %d , deltaY = %d\n", deltaX, deltaY);
		deltaFile.close();
		return true;
	}
	else
		return false;
}
/*--parameters 
pObj:        corresponding object candidate
padPixel :   to record max overlapping  pixels
shifitFlag:  to move the corresponding candidate to find the position with max padding pixels
matchFlag:   means the two candidates mathes well (no less than 70 percent overlapping area  and sizeratio between 0.7 - 1.3)
*/
Bool ObjectCandidate::calculateMaxOverlaps(ObjectCandidate* pObj, Float &padPixel,Bool shiftFlag,Bool &matchFlag)
{
	Float padingPixel = 0, maxPadPixel = 0, sizeRatio = 0;
	Int deltaX, deltaY, coordX, coordY;
	UInt pixVal = 0, ctuArea, tempPicRadius;
	UInt tempLeftX, tempUpY, tempRightX, tempBottomY;
	UInt leftBoundry, upBoundry, rigBoundry, botBoundry;
	UInt tempCentroidX, tempCentroidY, ctuSize = m_pcCurPic->getCtuWid();
	Pxl** tempPic;
	CtuData* curCtu;
	Int maxShift, offsetVal = 0;
	UInt curVol, corVol, minVol;
	ctuArea = ctuSize* ctuSize;
	if (shiftFlag)
		maxShift = 2 * ctuSize;
	else
		maxShift = ctuSize;
	curVol = ((UInt)m_mCtuElement.size())*ctuArea;
	corVol = ((UInt)pObj->getCtuElements().size())*ctuArea;
	minVol = curVol < corVol ? curVol : corVol;
	sizeRatio = curVol < corVol ? (Float)curVol / corVol : (Float)corVol / curVol;
	getObjCentroid(tempCentroidX, tempCentroidY, false);
	pObj->getObjCentroid(tempCentroidX, tempCentroidY, false);
	getObjEdge(tempLeftX, tempUpY, tempRightX, tempBottomY, false);
	pObj->getObjEdge(tempLeftX, tempUpY, tempRightX, tempBottomY, false);
	
	leftBoundry = m_uiLeftEdge < tempLeftX ? m_uiLeftEdge : tempLeftX;
	upBoundry = m_uiUpEdge < tempUpY ? m_uiUpEdge : tempUpY;
	rigBoundry = m_uiRightEdge>tempRightX ? m_uiRightEdge : tempRightX;
	botBoundry = m_uiBottomEdge>tempBottomY ? m_uiBottomEdge : tempBottomY;

	tempPicRadius = (m_uiCentroidX - leftBoundry) > (rigBoundry - m_uiCentroidX) ? (m_uiCentroidX - leftBoundry) : (rigBoundry - m_uiCentroidX);
	tempPicRadius = (m_uiCentroidY - upBoundry) > tempPicRadius ? (m_uiCentroidY - upBoundry) : tempPicRadius;
	tempPicRadius = (botBoundry - m_uiCentroidY) > tempPicRadius ? (botBoundry - m_uiCentroidY) : tempPicRadius;
	deltaX = abs((Int)m_uiCentroidX - (Int)tempCentroidX);
	deltaY = abs((Int)m_uiCentroidY - (Int)tempCentroidY);
	deltaX = max(deltaX, deltaY);
	tempPicRadius += deltaX;
	tempPicRadius += maxShift;
	tempPic = new Pxl*[2 * tempPicRadius]();
	for (UInt i = 0; i < 2 * tempPicRadius; i++)
		tempPic[i] = new Pxl[tempPicRadius * 2]();
	deltaX = tempPicRadius - (Int)m_uiCentroidX;
	deltaY = tempPicRadius - (Int)m_uiCentroidY;

	auto curCtuBeg = m_mCtuElement.begin();
	while (curCtuBeg != m_mCtuElement.end())
	{
		curCtu = curCtuBeg->first;
		coordX = curCtu->getAbsx();
		coordY = curCtu->getAbsy();
		coordX = (Int)coordX + deltaX;
		coordY = (Int)coordY + deltaY;
		for (Int row = coordY; row < coordY + ctuSize; row++)
		{
			for (Int col = coordX; col < coordX + ctuSize; col++)
			{
				tempPic[row][col] = 0x01;
			}
		}
		++curCtuBeg;
	}
	std::map<CtuData*, Bool, ctuAddrIncCompare> corCtuElem = pObj->getCtuElemRef();
	offsetVal = -(maxShift / 2);
	while (offsetVal < maxShift / 2)
	{
		Int turnX = -(maxShift / 2);
		Int turnY = offsetVal;
		while (turnX < maxShift / 2)
		{
			padingPixel = 0;
			deltaX = tempPicRadius + turnX - (Int)tempCentroidX;
			deltaY = tempPicRadius + turnY - (Int)tempCentroidY;
			auto begCorCtu = corCtuElem.begin();
			while (begCorCtu != corCtuElem.end())
			{
				curCtu = begCorCtu->first;
				coordX = curCtu->getAbsx();
				coordY = curCtu->getAbsy();
				coordX = (Int)coordX + deltaX;
				coordY = (Int)coordY + deltaY;
				for (Int row = coordY; row < coordY + ctuSize; row++)
				{
					for (Int col = coordX; col < coordX + ctuSize; col++)
					{
						if (tempPic[row][col] == 0x01)
							++padingPixel;
					}
				}
				++begCorCtu;
			}
			maxPadPixel = padingPixel > maxPadPixel ? padingPixel : maxPadPixel;
			if (maxPadPixel == minVol)
				break;
			turnX += 2;
		}
		if (maxPadPixel == minVol)
			break;
		offsetVal += 2;
	}
	padPixel = maxPadPixel;
	if (shiftFlag == true)
	{
		if (maxPadPixel / corVol >= 0.6 && maxPadPixel / curVol >= 0.6 && sizeRatio >= 0.6)
			matchFlag = true;
		else
			matchFlag = false;
	}

	for (UInt i = 0; i < tempPicRadius * 2; i++)
	{
		delete[]tempPic[i];
		tempPic[i] = NULL;
	}
	delete[]tempPic;
	tempPic = NULL;

	if (maxPadPixel / corVol >= 0.5 && maxPadPixel / curVol >= 0.5)
		return true;
	else
		return false;
}
//
Void ObjectCandidate::searchMaxCommonArea(ObjectCandidate* pCorObj, Bool &matchFlag)
{
	Float curPadPxl, sizeRatio, maxPadPxl = 0;
	UInt ctuSize = m_pcCurPic->getCtuWid();
	UInt verLen, horLen, curVol, corVol, ctuArea;
	UInt startCorX, startCorY, endCorX, endCorY, picCorX, picCorY;
	UInt tempCentroicX, tempCentroidY, tempLeft, tempRight, tempUp, tempBottom;
	UInt verShift, horShift, curHeight, curWidth, corHeight, corWidth;
	Int corPxlIdx, curPxlIdx, leftMost, rightMost, upMost, bottMost, picLeft, picUp;
	Int deltaX, deltaY;
	std::vector<std::vector<std::pair<Int, Bool>>>& corPxlElement = pCorObj->getPxlElement();
	std::vector<std::pair<Int, Int>> deltaXY;
	ctuArea = ctuSize*ctuSize;
	curVol = ctuArea*(UInt)m_mCtuElement.size();
	corVol = ctuArea*(UInt)pCorObj->getCtuElements().size();
	sizeRatio = curVol > corVol ? (Float)corVol / curVol : (Float)curVol / corVol;
	getObjCentroid(tempCentroicX, tempCentroidY, false);
	getObjEdge(tempLeft, tempUp, tempRight, tempBottom, false);
	pCorObj->getObjCentroid(tempCentroicX, tempCentroidY, false);
	pCorObj->getObjEdge(tempLeft, tempUp, tempRight, tempBottom, false);
	corHeight = tempBottom - tempUp;
	corWidth = tempRight - tempLeft;
	curHeight = m_uiBottomEdge - m_uiUpEdge;
	curWidth = m_uiRightEdge - m_uiLeftEdge;

	verLen = corHeight + curHeight;
	horLen = corWidth + curWidth;

	verShift = ctuSize;
	horShift = ctuSize;
	deltaX = m_uiLeftEdge - tempRight;
	deltaY = m_uiUpEdge - tempBottom;
	while (verShift < verLen)
	{
		//vertical shift 
		if (verShift < corHeight)
		{
			startCorY = corHeight - verShift;
			picCorY = 0;
			if (verShift < curHeight)
				endCorY = corHeight;
			else
				endCorY = startCorY + curHeight;
		}
		else
		{
			startCorY = 0;
			picCorY = verShift - corHeight;
			if (verShift < curHeight)
				endCorY = corHeight;
			else
				endCorY = corHeight - (verShift - curHeight);
		}
		//horizontal shift 
		horShift = ctuSize;

		while (horShift < horLen)
		{
			curPadPxl = 0;
			if (horShift < corWidth)
			{
				startCorX = corWidth - horShift;
				picCorX = 0;
				if (horShift < curWidth)
					endCorX = corWidth;
				else
					endCorX = startCorX + curWidth;
			}
			else
			{
				startCorX = 0;
				picCorX = horShift - corWidth;
				if (horShift < curWidth)
					endCorX = corWidth;
				else
					endCorX = corWidth - (horShift - curWidth);
			}

			for (UInt verIdx = startCorY; verIdx < endCorY; verIdx++)
			{
				for (UInt horIdx = startCorX; horIdx < endCorX; horIdx++)
				{
					corPxlIdx = corPxlElement[verIdx][horIdx].first;
					if (corPxlIdx != -1)
					{
						curPxlIdx = m_vPxlElement[picCorY + verIdx - startCorY][picCorX + horIdx - startCorX].first;
						if (curPxlIdx != -1)
							++curPadPxl;
					}
				}
			}

			if (curPadPxl >= maxPadPxl)
			{
				maxPadPxl = curPadPxl;//max padding pixels number

				leftMost = startCorX;//coordinates of common area 
				rightMost = endCorX;
				upMost = startCorY;
				bottMost = endCorY;

				picLeft = picCorX;	//
				picUp = picCorY;

					//deltaXY.clear();
				deltaX = horShift;
				deltaY = verShift;
				deltaX += m_uiLeftEdge - tempRight;
				deltaY += m_uiUpEdge - tempBottom;
				deltaXY.push_back(std::make_pair(deltaX, deltaY));		
			}
			horShift += ctuSize;
		}
		//
		verShift += ctuSize;
	}

	if (maxPadPxl / curVol >= 0.5001 && maxPadPxl / corVol >= 0.5001 && sizeRatio >= 0.5001)
	{
		//not needed 
		/*--to show common area --*/
		/*
		for (UInt verIdx = upMost; verIdx < bottMost; verIdx++)
		{
			for (UInt horIdx = leftMost; horIdx < rightMost; horIdx++)
			{
				corPxlIdx = corPxlElement[verIdx][horIdx].first;
				if (corPxlIdx != -1)
				{
					curPxlIdx = m_vPxlElement[picUp + verIdx - upMost][picLeft + horIdx - leftMost].first;
					if (curPxlIdx != -1)
						corPxlElement[verIdx][horIdx].first = 0;
				}
			}
		}

		for (UInt verIdx = upMost; verIdx < bottMost; verIdx++)
		{
			for (UInt horIdx = leftMost; horIdx < rightMost; horIdx++)
			{
				curPxlIdx = m_vPxlElement[picUp + verIdx - upMost][picLeft + horIdx - leftMost].first;
				if (curPxlIdx != -1)
				{
					corPxlIdx = corPxlElement[verIdx][horIdx].first;
					if (corPxlIdx = -1)
						m_vPxlElement[picUp + verIdx - upMost][picLeft + horIdx - leftMost].first = 0;
				}
			}
		}
		*/
		matchFlag = true;
	}
	else
	{
		matchFlag = false;

	}
	if (m_uiGroupNum != pCorObj->getGroupNum())
	{
		if (maxPadPxl / corVol >= 0.6 && maxPadPxl / curVol >= 0.60 && sizeRatio >= 0.60)
			matchFlag = true;
	}
}
//
Bool ObjectCandidate::checkFrontierRealtaion(ObjectCandidate* pCorCand, MotionDirection curDir, Int &deltaEdge)
{
	Int curFrontier, corFrontier;
	switch (curDir)
	{
	case MOVE_LEFT:
		curFrontier = (Int)m_uiLeftEdge;
		corFrontier = (Int)pCorCand->getLeftEdge();
		if (corFrontier > curFrontier)
			return false;
		break;
	case MOVE_RIGHT:
		curFrontier = (Int)m_uiRightEdge;
		corFrontier = (Int)pCorCand->getRightEdge();
		if (corFrontier < curFrontier)
			return false;
		break;
	case MOVE_UP:
		curFrontier = (Int)m_uiUpEdge;
		corFrontier = (Int)pCorCand->getUpEdge();
		if (corFrontier > corFrontier)
			return false;
		break;
	case MOVE_DOWN:
		curFrontier = (Int)m_uiBottomEdge;
		corFrontier = (Int)pCorCand->getBottomEdge();
		if (corFrontier < curFrontier)
			return false;
	default:
		return false;
		break;
	}
	deltaEdge = corFrontier - curFrontier;
	return true;
}
//
Void ObjectCandidate::matchWithCorrespongdingCand(ObjectCandidate* pCorCand, Bool &matchFlag)
{
	if (m_mDeltaInf.find(pCorCand) != m_mDeltaInf.end())
	{
		matchFlag = true;
		return;
	}
	Float curPadPxl, sizeRatio, maxPadPxl = 0, sizeMatchLevel;
	UInt ctuSize = m_pcCurPic->getCtuWid();
	UInt verLen, horLen, curVol, corVol, ctuArea;
	UInt startCorX, startCorY, endCorX, endCorY, picCorX, picCorY;
	UInt  tempLeft, tempRight, tempUp, tempBottom;
	UInt verShift, horShift, curHeight, curWidth, corHeight, corWidth;
	Int deltaX, deltaY, curPicNum, corPicNum, corPxlIdx, curPxlIdx;
	UInt dirLeft, dirRight, dirUp, dirDown;
	Bool curAtBoard = false, corAtBoard = false, overlapFlag;
	std::map<ObjectCandidate*, std::vector<deltaInfo>, candInPicOrder>	 &corDeltaInfo = pCorCand->getDeltaInf();

	std::vector<std::vector<std::pair<Int, Bool>>>& corPxlElement = pCorCand->getPxlElement();
	std::vector<deltaInfo> vDeltaInf;
	std::vector<std::pair<Int, Int>> deltaXY;
	curPicNum = m_uiPicNum;
	corPicNum = pCorCand->getPicNum();
	ctuArea = ctuSize*ctuSize;
	curVol = ctuArea*(UInt)m_mCtuElement.size();
	corVol = ctuArea*(UInt)pCorCand->getCtuElements().size();
	sizeRatio = curVol > corVol ? (Float)corVol / curVol : (Float)curVol / corVol;
	getObjEdge(tempLeft, tempUp, tempRight, tempBottom, false);
	pCorCand->getObjEdge(tempLeft, tempUp, tempRight, tempBottom, false);
	corHeight = tempBottom - tempUp;
	corWidth = tempRight - tempLeft;
	curHeight = m_uiBottomEdge - m_uiUpEdge;
	curWidth = m_uiRightEdge - m_uiLeftEdge;

	verLen = corHeight + curHeight;
	horLen = corWidth + curWidth;

	verShift = ctuSize;
	horShift = ctuSize;
	deltaX = m_uiLeftEdge - tempRight;
	deltaY = m_uiUpEdge - tempBottom;
	while (verShift < verLen)
	{
		//vertical shift 
		if (verShift < corHeight)
		{
			startCorY = corHeight - verShift;
			picCorY = 0;
			if (verShift < curHeight)
				endCorY = corHeight;
			else
				endCorY = startCorY + curHeight;
		}
		else
		{
			startCorY = 0;
			picCorY = verShift - corHeight;
			if (verShift < curHeight)
				endCorY = corHeight;
			else
				endCorY = corHeight - (verShift - curHeight);
		}

		//horizontal shift 
		horShift = ctuSize;
		while (horShift < horLen)
		{
			curPadPxl = 0;
			if (horShift < corWidth)
			{
				startCorX = corWidth - horShift;
				picCorX = 0;
				if (horShift < curWidth)
					endCorX = corWidth;
				else
					endCorX = startCorX + curWidth;
			}
			else
			{
				startCorX = 0;
				picCorX = horShift - corWidth;
				if (horShift < curWidth)
					endCorX = corWidth;
				else
					endCorX = corWidth - (horShift - curWidth);
			}

			for (UInt verIdx = startCorY; verIdx < endCorY; verIdx += ctuSize)
			{
				for (UInt horIdx = startCorX; horIdx < endCorX; horIdx += ctuSize)
				{
					corPxlIdx = corPxlElement[verIdx][horIdx].first;
					if (corPxlIdx != -1)
					{
						curPxlIdx = m_vPxlElement[picCorY + verIdx - startCorY][picCorX + horIdx - startCorX].first;
						if (curPxlIdx != -1)
							curPadPxl += ctuArea;
					}
				}
			}

			if (curPadPxl >= maxPadPxl)
			{
				deltaX = horShift;
				deltaY = verShift;
				deltaX += m_uiLeftEdge - tempRight;
				deltaY += m_uiUpEdge - tempBottom;
				if (!(abs(curPicNum - corPicNum) == 1 && (abs(deltaX) > ctuSize || abs(deltaY) > ctuSize)))
				{
					if (curPadPxl > maxPadPxl)
					{
						deltaXY.clear();
					}
					maxPadPxl = curPadPxl;//save max padding pixels number
	
					deltaXY.push_back(std::make_pair(deltaX, deltaY));
				}
			}
			horShift += ctuSize;
		}
		//
		verShift += ctuSize;
	}
	curAtBoard = atTheBorder(dirLeft, dirUp, dirRight, dirDown);
	corAtBoard = atTheBorder(dirLeft, dirUp, dirRight, dirDown);
	if ((curAtBoard || corAtBoard) || (curVol <= ctuArea * 2) || (corVol <= ctuArea * 2))
	{
		overlapFlag = (maxPadPxl / corVol >= 0.5f) || (maxPadPxl / curVol >= 0.5f);
		sizeMatchLevel = 0.5f;
	}
	else
	{
		overlapFlag = (maxPadPxl / corVol >= 0.5f) && (maxPadPxl / curVol >= 0.5f);
		sizeMatchLevel = 0.6f;
	}
	//save motion for general match 
	if (overlapFlag && sizeRatio >= sizeMatchLevel)
	{
		auto beg = deltaXY.begin();
		while (beg != deltaXY.end())
		{
			deltaInfo curDeltaInf;// = new motionInf();
			curDeltaInf.deltaX = beg->first;
			curDeltaInf.deltaY = beg->second;
			vDeltaInf.push_back(curDeltaInf);
			++beg;
		}
		m_mDeltaInf[pCorCand] = vDeltaInf;
		corDeltaInfo[this] = vDeltaInf;
		matchFlag = true;
	}
	else
	{
		matchFlag = false;
		vDeltaInf.clear();
	}
}
//
Void ObjectCandidate::checkFrontierMatch(ObjectCandidate* pCorCand, Bool &motionExist, ObjectCandidate* pCheckCand, Bool verifyFlag)
{
	typedef std::pair<Int, Int> motionVector;
	typedef std::map<ObjectCandidate*, std::vector<deltaInfo>, candInPicOrder> deltaInfo_type;

	UInt curUpEdgeSize, curLeftEdgeSize, curRigEdgeSize, curBotEdgeSize;
	UInt curSubUpEdge, curSubLeftEdge, curSubRigEdge, curSubBotEdge;
	UInt corUpEdgeSize, corLeftEdgeSize, corRigEdgeSize, corBotEdgeSize;
	UInt corSubUpEdge, corSubLeftEdge, corSubRigEdge, corSubBotEdge;

	Float bodyOverlapSize, upOverlapSize, leftOverlapSize, rigOverlapSize, botOverlapSize;
	Float subUpOverlap, subLeftOverlap, subRigOverlap, subBotOverlap;
	std::vector<std::vector<std::pair<Int, Bool>>>& corPxlElement = pCorCand->getPxlElement();
	UInt verLen, horLen, ctuSize, stepSize = 0;
	UInt startCorX, startCorY, endCorX, endCorY, picCorX, picCorY;
	UInt orgCorLeftSide, orgCorUpEdge, orgCorRightEdge, orgCorBotEdge;
	Int verShift, horShift, curHeight, curWidth, corHeight, corWidth;
	Int  curPxlVal, corPxlVal;
	Float curRatio, corRatio, cursubRatio, corSubRatio;
	motionVector curMv;
	std::map<motionVector, std::vector<Float>> mvAndEdgeMatchRatio;
	deltaInfo_type &corDeltaInfo = pCorCand->getDeltaInf();
	MotionDirection horDir, verDir;//;
	//	std::vector<std::pair<UInt, UInt>> corXY1, picXY1;
	stepSize = verifyFlag == true ? 3 : 2;
	getObjEdge(orgCorLeftSide, orgCorUpEdge, orgCorRightEdge, orgCorBotEdge, false);
	pCorCand->getObjEdge(orgCorLeftSide, orgCorUpEdge, orgCorRightEdge, orgCorBotEdge, false);
	ctuSize = m_pcCurPic->getCtuWid();
	corHeight = orgCorBotEdge - orgCorUpEdge;
	corWidth = orgCorRightEdge - orgCorLeftSide;
	curHeight = m_uiBottomEdge - m_uiUpEdge;
	curWidth = m_uiRightEdge - m_uiLeftEdge;
	
	verLen = corHeight + curHeight;
	horLen = corWidth + curWidth;
	auto corDeltaInfoIndex = corDeltaInfo[this].begin();
	auto deltaInfIndex = m_mDeltaInf[pCorCand].begin();
	std::pair<MotionDirection, MotionDirection> curMotionDir, subHorDir, subVerDir;
	
	std::map<std::pair<MotionDirection, MotionDirection>, ObjectCandidate*> MotionDirections;
	while (deltaInfIndex != m_mDeltaInf[pCorCand].end())
	{
		//remove delta with edge difference more than two ctusize
		//move to left 
		if (deltaInfIndex->deltaX < 0 && m_uiLeftEdge != 0 && orgCorLeftSide != 0)
		{
			if (abs((Int)m_uiLeftEdge - ((Int)orgCorLeftSide + deltaInfIndex->deltaX)) > ctuSize)
			{
				deltaInfIndex = m_mDeltaInf[pCorCand].erase(deltaInfIndex);
				corDeltaInfoIndex = corDeltaInfo[this].erase(corDeltaInfoIndex);
			}
			if (deltaInfIndex == m_mDeltaInf[pCorCand].end())
				break;
		}
		//move to right
		if (deltaInfIndex->deltaX > 0 && m_uiRightEdge != m_pcCurPic->getPicWid() && orgCorRightEdge != m_pcCurPic->getPicWid())
		{
			if (abs((Int)m_uiRightEdge - ((Int)orgCorRightEdge + deltaInfIndex->deltaX)) > ctuSize)
			{
				deltaInfIndex = m_mDeltaInf[pCorCand].erase(deltaInfIndex);
				corDeltaInfoIndex = corDeltaInfo[this].erase(corDeltaInfoIndex);
			}
			if (deltaInfIndex == m_mDeltaInf[pCorCand].end())
				break;
		}
		if (deltaInfIndex->deltaY < 0 && m_uiUpEdge != 0 && orgCorUpEdge != 0)
		{
			if (abs((Int)m_uiUpEdge - ((Int)orgCorUpEdge + deltaInfIndex->deltaY)) > ctuSize)
			{
				deltaInfIndex = m_mDeltaInf[pCorCand].erase(deltaInfIndex);
				corDeltaInfoIndex = corDeltaInfo[this].erase(corDeltaInfoIndex);
			}
			if (deltaInfIndex == m_mDeltaInf[pCorCand].end())
				break;
		}
		if (deltaInfIndex->deltaY > 0 && m_uiBottomEdge != m_pcCurPic->getPicHig() && orgCorBotEdge != m_pcCurPic->getPicHig())
		{
			if (abs((Int)m_uiBottomEdge - ((Int)orgCorBotEdge + deltaInfIndex->deltaY)) > ctuSize)
			{
				deltaInfIndex = m_mDeltaInf[pCorCand].erase(deltaInfIndex);
				corDeltaInfoIndex = corDeltaInfo[this].erase(corDeltaInfoIndex);
			}
			if (deltaInfIndex == m_mDeltaInf[pCorCand].end())
				break;
		}

		curSubUpEdge = curSubRigEdge = curSubLeftEdge = curSubBotEdge = 0;
		corSubBotEdge = corSubLeftEdge = corSubRigEdge = corSubUpEdge = 0;
		subBotOverlap = subLeftOverlap = subRigOverlap = subUpOverlap = 0;

		curUpEdgeSize = curLeftEdgeSize = curRigEdgeSize = curBotEdgeSize = 0;
		corUpEdgeSize = corLeftEdgeSize = corRigEdgeSize = corBotEdgeSize = 0;

		bodyOverlapSize = upOverlapSize = leftOverlapSize = rigOverlapSize = botOverlapSize = 0;
		//calculate overlaping ratio 
		horShift = deltaInfIndex->deltaX - (m_uiLeftEdge - orgCorRightEdge);
		verShift = deltaInfIndex->deltaY - (m_uiUpEdge - orgCorBotEdge);
		if (verShift < 0 || horShift < 0)
			int detbupoint = 0;
		//calculate vertical start index
		if (verShift < corHeight)//
		{
			//first line for calculation 
			startCorY = corHeight - verShift;
			picCorY = 0;
			//last line for calculation
			if (verShift < curHeight)
				endCorY = corHeight;
			else
				endCorY = startCorY + curHeight;
		}
		else
		{
			startCorY = 0;
			picCorY = verShift - corHeight;
			if (verShift < curHeight)
				endCorY = corHeight;
			else
				endCorY = corHeight - (verShift - curHeight);
		}
		//calculate horizontal start index 
		//first and last row 
		if (horShift < corWidth)
		{
			startCorX = corWidth - horShift;
			picCorX = 0;
			if (horShift < curWidth)
				endCorX = corWidth;
			else
				endCorX = startCorX + curWidth;
		}
		else
		{
			startCorX = 0;
			picCorX = horShift - corWidth;
			if (horShift < curWidth)
				endCorX = corWidth;
			else
				endCorX = corWidth - (horShift - curWidth);
		}

		//startCorY and startCorX for the corresponding Candidate
		//PicCorX,picCorY for current 
		for (UInt verIdx = startCorY; verIdx < endCorY; verIdx += ctuSize)
		{
			for (UInt horIdx = startCorX; horIdx < endCorX; horIdx += ctuSize)
			{
				corPxlVal = corPxlElement[verIdx][horIdx].first;
				curPxlVal = m_vPxlElement[picCorY + verIdx - startCorY][picCorX + horIdx - startCorX].first;

				if (curPxlVal != -1 && corPxlVal != -1)
				{
					bodyOverlapSize += 1;
				}
				if (curPxlVal != -1)
				{
					//boundries 
					if ((picCorY + verIdx - startCorY) == 0)//first row 
						++curUpEdgeSize;
					if ((picCorY + verIdx - startCorY) == (curHeight - 32))//last row
						++curBotEdgeSize;
					if ((picCorX + horIdx - startCorX) == 0)//first collum
						++curLeftEdgeSize;
					if ((picCorX + horIdx - startCorX) == (curWidth - 32))//last colum
						++curRigEdgeSize;
					//sub-boundries
					if ((picCorY + verIdx - startCorY) == 32)//sub -up
						++curSubUpEdge;
					if ((picCorY + verIdx - startCorY) == (curHeight - 64))//
						++curSubBotEdge;
					if ((picCorX + horIdx - startCorX) == 32)
						++curSubLeftEdge;
					if ((picCorX + horIdx - startCorX) == (curWidth - 64))
						++curSubRigEdge;
				}
				if (corPxlVal != -1)
				{
					if (verIdx == 0)//upside
					{
						++corUpEdgeSize;
						if ((picCorY + verIdx - startCorY) == 0)
							++upOverlapSize;
					}
					if (verIdx == (corHeight - 32))//bottom side
					{
						++corBotEdgeSize;
						if ((picCorY + verIdx - startCorY) == (curHeight - 32))
							++botOverlapSize;
					}
					if (horIdx == 0)//left side 
					{
						++corLeftEdgeSize;
						if ((picCorX + horIdx - startCorX) == 0)
							++leftOverlapSize;
					}
					if (horIdx == corWidth - 32)//right
					{
						++corRigEdgeSize;
						if ((picCorX + horIdx - startCorX) == (curWidth - 32))
							++rigOverlapSize;
					}
					//sub boundries 
					if (verIdx == 32)//upside
					{
						++corSubUpEdge;
						if ((picCorY + verIdx - startCorY) == 32)
							++subUpOverlap;
					}
					if (verIdx == (corHeight - 64))//bottom side
					{
						++corSubBotEdge;
						if ((picCorY + verIdx - startCorY) == (curWidth - 64))
							++subBotOverlap;
					}
					if (horIdx == 0)//left side 
					{
						++corSubLeftEdge;
						if ((picCorX + horIdx - startCorX) == 32)
							++subLeftOverlap;
					}
					if (horIdx == corWidth - 64)
					{
						++corSubRigEdge;
						if ((picCorX + horIdx - startCorX) == (curWidth - 64))
							++subRigOverlap;
					}
				}
			}
		}
		curMv.first = deltaInfIndex->deltaX;
		curMv.second = deltaInfIndex->deltaY;
		curMotionDir.first = curMotionDir.second = NO_DIRECTION;
		//for edge match judegment
		if (deltaInfIndex->deltaX == 0)
			horDir = NO_DIRECTION;
		//move to left 
		else if (deltaInfIndex->deltaX < 0)
		{
			horDir = MOVE_LEFT;
			curRatio = curLeftEdgeSize == 0 ? -1 : (leftOverlapSize / curLeftEdgeSize);
			corRatio = corLeftEdgeSize == 0 ? -1 : (leftOverlapSize / corLeftEdgeSize);
			cursubRatio = curSubLeftEdge == 0 ? -1 : (curSubLeftEdge / subLeftOverlap);
			corSubRatio = corSubLeftEdge == 0 ? -1 : (corSubLeftEdge / subLeftOverlap);
		}
		//move to right
		else if (deltaInfIndex->deltaX > 0)
		{
			horDir = MOVE_RIGHT;
			curRatio = curRigEdgeSize == 0 ? -1 : (rigOverlapSize / curRigEdgeSize);
			corRatio = corRigEdgeSize == 0 ? -1 : (rigOverlapSize / corRigEdgeSize);
			cursubRatio = curSubRigEdge == 0 ? -1 : (curSubRigEdge / subRigOverlap);
			corSubRatio = corSubRigEdge == 0 ? -1 : (corSubRigEdge / subRigOverlap);
		}
		//HORIZONTAL MOTION
		if (deltaInfIndex->deltaX != 0)
		{
			deltaInfIndex->xWayRatio = curRatio;
			corDeltaInfoIndex->xWayRatio = corRatio;
			if (curRatio >= 0 && corRatio >= 0)
				curMotionDir.first = horDir;
			else
			{
				if (corSubRatio >= 0.5&&cursubRatio >= 0.5)
					curMotionDir.first = horDir;
			}
			if (curMotionDir.first != NO_DIRECTION)
			{
				if (horDir == MOVE_LEFT)
				{
					if (abs((Int)pCorCand->getLeftEdge() - (Int)m_uiLeftEdge) > ctuSize * stepSize)
					{
						motionExist = false;
						return;
					}
				}
				else
				{
					if (abs((Int)pCorCand->getRightEdge() - (Int)m_uiRightEdge) > ctuSize * stepSize)
					{
						motionExist = false;
						return;
					}
				}
			}
		}
		if (deltaInfIndex->deltaY == 0)
			verDir = NO_DIRECTION;
		else if (deltaInfIndex->deltaY < 0)
		{
			verDir = MOVE_UP;
			curRatio = curUpEdgeSize == 0 ? -1 : (upOverlapSize / curUpEdgeSize);
			corRatio = corUpEdgeSize == 0 ? -1 : (upOverlapSize / corUpEdgeSize);
			cursubRatio = curSubUpEdge == 0 ? -1 : (subUpOverlap / curSubUpEdge);
			corSubRatio = corSubUpEdge == 0 ? -1 : (subUpOverlap / corSubUpEdge);
		}
		else if (deltaInfIndex->deltaY > 0)
		{
			verDir = MOVE_DOWN;
			curRatio = curBotEdgeSize == 0 ? -1 : (botOverlapSize / curBotEdgeSize);
			corRatio = corBotEdgeSize == 0 ? -1 : (botOverlapSize / corBotEdgeSize);
			cursubRatio = curSubBotEdge == 0 ? -1 : (curSubBotEdge / subBotOverlap);
			corSubRatio = corSubBotEdge == 0 ? -1 : (corSubBotEdge / subBotOverlap);
		}
		//VERTICAL MOTION
		if (deltaInfIndex->deltaY != 0)
		{
			deltaInfIndex->yWayRatio = curRatio;
			corDeltaInfoIndex->yWayRatio = corRatio;
			if (curRatio >= 0 && corRatio >= 0)
				curMotionDir.second = verDir;
			else
			{
				if (corSubRatio >= 0.5&&cursubRatio >= 0.5)
					curMotionDir.second = verDir;
			}
			if (curMotionDir.second != NO_DIRECTION)
			{
				if (verDir == MOVE_DOWN)
				{
					if (abs((Int)pCorCand->getBottomEdge() - (Int)m_uiBottomEdge) > ctuSize*stepSize)
					{
						motionExist = false;
						return;
					}
				}
				else
				{
					if (abs((Int)pCorCand->getUpEdge() - (Int)m_uiUpEdge) > ctuSize * stepSize)
					{
						motionExist = false;
						return;
					}
				}
			}
		}
		MotionDirections[curMotionDir] = pCorCand;
		++deltaInfIndex;
		++corDeltaInfoIndex;
	}
	if (verifyFlag)
	{
		std::vector<std::pair<MotionDirection, MotionDirection>> &checkMotionDir = pCheckCand->getMotionDirectionInfo();
		auto motDirBeg = MotionDirections.begin();
		while (motDirBeg != MotionDirections.end())
		{
			if (motDirBeg->first.first != NO_DIRECTION || motDirBeg->first.second != NO_DIRECTION)
			{
				curMotionDir.first = motDirBeg->first.first;
				curMotionDir.second = motDirBeg->first.second;
				auto findResult = find(checkMotionDir.begin(), checkMotionDir.end(), curMotionDir);
				if (findResult != checkMotionDir.end())
				{
					motionExist = true;
					break;
				}
				if (motDirBeg->first.first != NO_DIRECTION&&motDirBeg->first.second != NO_DIRECTION)
				{
					subHorDir.first = curMotionDir.first;
					subVerDir.first = subHorDir.second = NO_DIRECTION;
					subVerDir.second = curMotionDir.second;
					auto subHorResult = find(checkMotionDir.begin(), checkMotionDir.end(), subHorDir);
					auto subVerResult = find(checkMotionDir.begin(), checkMotionDir.end(), subVerDir);
					if (subHorResult != checkMotionDir.end() || subVerResult != checkMotionDir.end())
					{
						motionExist = true;
						break;
					}
				}
			}
			++motDirBeg;
		}
		if (motionExist)
		{
			auto motionBeg = MotionDirections.begin();
			while (motionBeg != MotionDirections.end())
			{
				curMotionDir.first = motDirBeg->first.first;
				curMotionDir.second = motDirBeg->first.second;
				auto findResult = find(checkMotionDir.begin(), checkMotionDir.end(), curMotionDir);
				if (findResult == checkMotionDir.end())
					checkMotionDir.push_back(curMotionDir);
				++motionBeg;
			}
			subHorDir.first = subHorDir.second = subVerDir.first = subVerDir.second = NO_DIRECTION;
			auto checkMotDirIndex = checkMotionDir.begin();
			while (checkMotDirIndex != checkMotionDir.end())
			{
				if (checkMotDirIndex->first == MOVE_LEFT)
					subHorDir.first = MOVE_LEFT;
				if (checkMotDirIndex->first == MOVE_RIGHT)
					subHorDir.second = MOVE_RIGHT;
				if (checkMotDirIndex->second == MOVE_UP)
					subVerDir.first = MOVE_UP;
				if (checkMotDirIndex->second == MOVE_DOWN)
					subVerDir.second = MOVE_DOWN;
				++checkMotDirIndex;
			}
			checkMotionDir.clear();
			if (subHorDir.first != NO_DIRECTION)
			{
				curMotionDir.first = MOVE_LEFT;
				curMotionDir.second = NO_DIRECTION;
				checkMotionDir.push_back(curMotionDir);
			}
			if (subHorDir.second != NO_DIRECTION)
			{
				curMotionDir.first = MOVE_RIGHT;
				curMotionDir.second = NO_DIRECTION;
				checkMotionDir.push_back(curMotionDir);
			}
			if (subVerDir.first != NO_DIRECTION)
			{
				curMotionDir.first = NO_DIRECTION;
				curMotionDir.second = MOVE_UP;
				checkMotionDir.push_back(curMotionDir);
			}
			if (subVerDir.second != NO_DIRECTION)
			{
				curMotionDir.first = NO_DIRECTION;
				curMotionDir.second = MOVE_DOWN;
				checkMotionDir.push_back(curMotionDir);
			}
		}
		else
			motionExist = false;
	}
	else	
	{
		auto motDirBeg = MotionDirections.begin();
	
		while (motDirBeg != MotionDirections.end())
		{
			if (motDirBeg->first.first != NO_DIRECTION || motDirBeg->first.second != NO_DIRECTION)
			{
				curMotionDir.first = motDirBeg->first.first;
				curMotionDir.second = motDirBeg->first.second;
				m_mMotionDirInfo.push_back(curMotionDir);
	
			}
			++motDirBeg;
		}
		if (m_mMotionDirInfo.size() > 0)
			motionExist = true;
		else
			motionExist = false;
	}
}

Bool ObjectCandidate::checkAnotherDir(ObjectCandidate *pCorCand,ObjectCandidate* pCurCand)
{
	typedef std::pair<Int, Int> motionVector;
	typedef std::map<ObjectCandidate*, std::vector<deltaInfo>, candInPicOrder> deltaInfo_type;

	UInt curUpEdgeSize, curLeftEdgeSize, curRigEdgeSize, curBotEdgeSize;
	UInt curSubUpEdge, curSubLeftEdge, curSubRigEdge, curSubBotEdge;
	UInt corUpEdgeSize, corLeftEdgeSize, corRigEdgeSize, corBotEdgeSize;
	UInt corSubUpEdge, corSubLeftEdge, corSubRigEdge, corSubBotEdge;

	Float bodyOverlapSize, upOverlapSize, leftOverlapSize, rigOverlapSize, botOverlapSize;
	Float subUpOverlap, subLeftOverlap, subRigOverlap, subBotOverlap;
	std::vector<std::vector<std::pair<Int, Bool>>>& corPxlElement = pCorCand->getPxlElement();
	UInt verLen, horLen, ctuSize, stepSize = 0;
	UInt startCorX, startCorY, endCorX, endCorY, picCorX, picCorY;
	UInt orgCorLeftSide, orgCorUpEdge, orgCorRightEdge, orgCorBotEdge;
	Int verShift, horShift, curHeight, curWidth, corHeight, corWidth;
	Int  curPxlVal, corPxlVal;
	Float curRatio, corRatio, cursubRatio, corSubRatio;
	motionVector curMv;
	std::map<motionVector, std::vector<Float>> mvAndEdgeMatchRatio;
	deltaInfo_type &corDeltaInfo = pCorCand->getDeltaInf();
	MotionDirection horDir, verDir;//;
	//	std::vector<std::pair<UInt, UInt>> corXY1, picXY1;
	stepSize = 2;
	getObjEdge(orgCorLeftSide, orgCorUpEdge, orgCorRightEdge, orgCorBotEdge, false);
	pCorCand->getObjEdge(orgCorLeftSide, orgCorUpEdge, orgCorRightEdge, orgCorBotEdge, false);
	ctuSize = m_pcCurPic->getCtuWid();
	corHeight = orgCorBotEdge - orgCorUpEdge;
	corWidth = orgCorRightEdge - orgCorLeftSide;
	curHeight = m_uiBottomEdge - m_uiUpEdge;
	curWidth = m_uiRightEdge - m_uiLeftEdge;

	verLen = corHeight + curHeight;
	horLen = corWidth + curWidth;
	auto corDeltaInfoIndex = corDeltaInfo[this].begin();
	auto deltaInfIndex = m_mDeltaInf[pCorCand].begin();
	std::pair<MotionDirection, MotionDirection> curMotionDir, subHorDir, subVerDir;

	std::map<std::pair<MotionDirection, MotionDirection>, ObjectCandidate*> MotionDirections;
	std::vector<std::pair<MotionDirection, MotionDirection>> &corMotionDirInfo = pCurCand->getMotionDirectionInfo();
	while (deltaInfIndex != m_mDeltaInf[pCorCand].end())
	{
		//remove delta with edge difference more than two ctusize
		//move to left 
		if (deltaInfIndex->deltaX < 0 && m_uiLeftEdge != 0 && orgCorLeftSide != 0)
		{
			if (abs((Int)m_uiLeftEdge - ((Int)orgCorLeftSide + deltaInfIndex->deltaX)) > ctuSize)
			{
				deltaInfIndex = m_mDeltaInf[pCorCand].erase(deltaInfIndex);
				corDeltaInfoIndex = corDeltaInfo[this].erase(corDeltaInfoIndex);
			}
			if (deltaInfIndex == m_mDeltaInf[pCorCand].end())
				break;
		}
		//move to right
		if (deltaInfIndex->deltaX > 0 && m_uiRightEdge != m_pcCurPic->getPicWid() && orgCorRightEdge != m_pcCurPic->getPicWid())
		{
			if (abs((Int)m_uiRightEdge - ((Int)orgCorRightEdge + deltaInfIndex->deltaX)) > ctuSize)
			{
				deltaInfIndex = m_mDeltaInf[pCorCand].erase(deltaInfIndex);
				corDeltaInfoIndex = corDeltaInfo[this].erase(corDeltaInfoIndex);
			}
			if (deltaInfIndex == m_mDeltaInf[pCorCand].end())
				break;
		}
		if (deltaInfIndex->deltaY < 0 && m_uiUpEdge != 0 && orgCorUpEdge != 0)
		{
			if (abs((Int)m_uiUpEdge - ((Int)orgCorUpEdge + deltaInfIndex->deltaY)) > ctuSize)
			{
				deltaInfIndex = m_mDeltaInf[pCorCand].erase(deltaInfIndex);
				corDeltaInfoIndex = corDeltaInfo[this].erase(corDeltaInfoIndex);
			}
			if (deltaInfIndex == m_mDeltaInf[pCorCand].end())
				break;
		}
		if (deltaInfIndex->deltaY > 0 && m_uiBottomEdge != m_pcCurPic->getPicHig() && orgCorBotEdge != m_pcCurPic->getPicHig())
		{
			if (abs((Int)m_uiBottomEdge - ((Int)orgCorBotEdge + deltaInfIndex->deltaY)) > ctuSize)
			{
				deltaInfIndex = m_mDeltaInf[pCorCand].erase(deltaInfIndex);
				corDeltaInfoIndex = corDeltaInfo[this].erase(corDeltaInfoIndex);
			}
			if (deltaInfIndex == m_mDeltaInf[pCorCand].end())
				break;
		}

		curSubUpEdge = curSubRigEdge = curSubLeftEdge = curSubBotEdge = 0;
		corSubBotEdge = corSubLeftEdge = corSubRigEdge = corSubUpEdge = 0;
		subBotOverlap = subLeftOverlap = subRigOverlap = subUpOverlap = 0;

		curUpEdgeSize = curLeftEdgeSize = curRigEdgeSize = curBotEdgeSize = 0;
		corUpEdgeSize = corLeftEdgeSize = corRigEdgeSize = corBotEdgeSize = 0;

		bodyOverlapSize = upOverlapSize = leftOverlapSize = rigOverlapSize = botOverlapSize = 0;
		//calculate overlaping ratio 
		horShift = deltaInfIndex->deltaX - (m_uiLeftEdge - orgCorRightEdge);
		verShift = deltaInfIndex->deltaY - (m_uiUpEdge - orgCorBotEdge);
		if (verShift < 0 || horShift < 0)
			int detbupoint = 0;
		//calculate vertical start index
		if (verShift < corHeight)//
		{
			//first line for calculation 
			startCorY = corHeight - verShift;
			picCorY = 0;
			//last line for calculation
			if (verShift < curHeight)
				endCorY = corHeight;
			else
				endCorY = startCorY + curHeight;
		}
		else
		{
			startCorY = 0;
			picCorY = verShift - corHeight;
			if (verShift < curHeight)
				endCorY = corHeight;
			else
				endCorY = corHeight - (verShift - curHeight);
		}
		//calculate horizontal start index 
		//first and last row 
		if (horShift < corWidth)
		{
			startCorX = corWidth - horShift;
			picCorX = 0;
			if (horShift < curWidth)
				endCorX = corWidth;
			else
				endCorX = startCorX + curWidth;
		}
		else
		{
			startCorX = 0;
			picCorX = horShift - corWidth;
			if (horShift < curWidth)
				endCorX = corWidth;
			else
				endCorX = corWidth - (horShift - curWidth);
		}

		//startCorY and startCorX for the corresponding Candidate
		//PicCorX,picCorY for current 
		for (UInt verIdx = startCorY; verIdx < endCorY; verIdx += ctuSize)
		{
			for (UInt horIdx = startCorX; horIdx < endCorX; horIdx += ctuSize)
			{
				corPxlVal = corPxlElement[verIdx][horIdx].first;
				curPxlVal = m_vPxlElement[picCorY + verIdx - startCorY][picCorX + horIdx - startCorX].first;

				if (curPxlVal != -1 && corPxlVal != -1)
				{
					bodyOverlapSize += 1;
				}
				if (curPxlVal != -1)
				{
					//boundries 
					if ((picCorY + verIdx - startCorY) == 0)//first row 
						++curUpEdgeSize;
					if ((picCorY + verIdx - startCorY) == (curHeight - 32))//last row
						++curBotEdgeSize;
					if ((picCorX + horIdx - startCorX) == 0)//first collum
						++curLeftEdgeSize;
					if ((picCorX + horIdx - startCorX) == (curWidth - 32))//last colum
						++curRigEdgeSize;
					//sub-boundries
					if ((picCorY + verIdx - startCorY) == 32)//sub -up
						++curSubUpEdge;
					if ((picCorY + verIdx - startCorY) == (curHeight - 64))//
						++curSubBotEdge;
					if ((picCorX + horIdx - startCorX) == 32)
						++curSubLeftEdge;
					if ((picCorX + horIdx - startCorX) == (curWidth - 64))
						++curSubRigEdge;
				}
				if (corPxlVal != -1)
				{
					if (verIdx == 0)//upside
					{
						++corUpEdgeSize;
						if ((picCorY + verIdx - startCorY) == 0)
							++upOverlapSize;
					}
					if (verIdx == (corHeight - 32))//bottom side
					{
						++corBotEdgeSize;
						if ((picCorY + verIdx - startCorY) == (curHeight - 32))
							++botOverlapSize;
					}
					if (horIdx == 0)//left side 
					{
						++corLeftEdgeSize;
						if ((picCorX + horIdx - startCorX) == 0)
							++leftOverlapSize;
					}
					if (horIdx == corWidth - 32)//right
					{
						++corRigEdgeSize;
						if ((picCorX + horIdx - startCorX) == (curWidth - 32))
							++rigOverlapSize;
					}
					//sub boundries 
					if (verIdx == 32)//upside
					{
						++corSubUpEdge;
						if ((picCorY + verIdx - startCorY) == 32)
							++subUpOverlap;
					}
					if (verIdx == (corHeight - 64))//bottom side
					{
						++corSubBotEdge;
						if ((picCorY + verIdx - startCorY) == (curWidth - 64))
							++subBotOverlap;
					}
					if (horIdx == 0)//left side 
					{
						++corSubLeftEdge;
						if ((picCorX + horIdx - startCorX) == 32)
							++subLeftOverlap;
					}
					if (horIdx == corWidth - 64)
					{
						++corSubRigEdge;
						if ((picCorX + horIdx - startCorX) == (curWidth - 64))
							++subRigOverlap;
					}
				}
			}
		}
		curMv.first = deltaInfIndex->deltaX;
		curMv.second = deltaInfIndex->deltaY;
		curMotionDir.first = curMotionDir.second = NO_DIRECTION;
		//for edge match judegment
		if (deltaInfIndex->deltaX == 0)
			horDir = NO_DIRECTION;
		//move to left 
		else if (deltaInfIndex->deltaX < 0)
		{
			horDir = MOVE_LEFT;
			curRatio = curLeftEdgeSize == 0 ? -1 : (leftOverlapSize / curLeftEdgeSize);
			corRatio = corLeftEdgeSize == 0 ? -1 : (leftOverlapSize / corLeftEdgeSize);
			cursubRatio = curSubLeftEdge == 0 ? -1 : (curSubLeftEdge / subLeftOverlap);
			corSubRatio = corSubLeftEdge == 0 ? -1 : (corSubLeftEdge / subLeftOverlap);
		}
		//move to right
		else if (deltaInfIndex->deltaX > 0)
		{
			horDir = MOVE_RIGHT;
			curRatio = curRigEdgeSize == 0 ? -1 : (rigOverlapSize / curRigEdgeSize);
			corRatio = corRigEdgeSize == 0 ? -1 : (rigOverlapSize / corRigEdgeSize);
			cursubRatio = curSubRigEdge == 0 ? -1 : (curSubRigEdge / subRigOverlap);
			corSubRatio = corSubRigEdge == 0 ? -1 : (corSubRigEdge / subRigOverlap);
		}
		//HORIZONTAL MOTION
		if (deltaInfIndex->deltaX != 0)
		{
			deltaInfIndex->xWayRatio = curRatio;
			corDeltaInfoIndex->xWayRatio = corRatio;
			if (curRatio >= 0 && corRatio >= 0)
				curMotionDir.first = horDir;
			else
			{
				if (corSubRatio >= 0.5&&cursubRatio >= 0.5)
					curMotionDir.first = horDir;
			}
		}
		if (deltaInfIndex->deltaY == 0)
			verDir = NO_DIRECTION;
		else if (deltaInfIndex->deltaY < 0)
		{
			verDir = MOVE_UP;
			curRatio = curUpEdgeSize == 0 ? -1 : (upOverlapSize / curUpEdgeSize);
			corRatio = corUpEdgeSize == 0 ? -1 : (upOverlapSize / corUpEdgeSize);
			cursubRatio = curSubUpEdge == 0 ? -1 : (subUpOverlap / curSubUpEdge);
			corSubRatio = corSubUpEdge == 0 ? -1 : (subUpOverlap / corSubUpEdge);
		}
		else if (deltaInfIndex->deltaY > 0)
		{
			verDir = MOVE_DOWN;
			curRatio = curBotEdgeSize == 0 ? -1 : (botOverlapSize / curBotEdgeSize);
			corRatio = corBotEdgeSize == 0 ? -1 : (botOverlapSize / corBotEdgeSize);
			cursubRatio = curSubBotEdge == 0 ? -1 : (curSubBotEdge / subBotOverlap);
			corSubRatio = corSubBotEdge == 0 ? -1 : (corSubBotEdge / subBotOverlap);
		}
		//VERTICAL MOTION
		if (deltaInfIndex->deltaY != 0)
		{
			deltaInfIndex->yWayRatio = curRatio;
			corDeltaInfoIndex->yWayRatio = corRatio;
			if (curRatio >= 0 && corRatio >= 0)
				curMotionDir.second = verDir;
			else
			{
				if (corSubRatio >= 0.5&&cursubRatio >= 0.5)
					curMotionDir.second = verDir;
			}
		}
		MotionDirections[curMotionDir] = pCorCand;
		++deltaInfIndex;
		++corDeltaInfoIndex;
	}
	{
		curMotionDir.first = corMotionDirInfo.begin()->first;
		curMotionDir.second = corMotionDirInfo.begin()->second;
		auto motDirBeg = MotionDirections.begin();
		while (motDirBeg != MotionDirections.end())
		{
			if (motDirBeg->first.first != NO_DIRECTION && motDirBeg->first.second != NO_DIRECTION)
			{
				if (motDirBeg->first.first == curMotionDir.first)
				{
					curMotionDir.first = NO_DIRECTION;
					curMotionDir.second = motDirBeg->first.second;
					corMotionDirInfo.push_back(curMotionDir);
					return true;
				}
				if (motDirBeg->first.second == curMotionDir.second)
				{
					curMotionDir.first = motDirBeg->first.first;
					curMotionDir.second = NO_DIRECTION;
					corMotionDirInfo.push_back(curMotionDir);
					return true;
				}
			}
			++motDirBeg;
		}
	}
	return false;
}
//
Void ObjectCandidate::matchWithCorCandGeneralElder(ObjectCandidate* pCorCand, Bool &corMatched, Bool orderFlag)
{
}


//
//verDir: if verDir is 0, means move to left 
//		: if verDir is 1, means move to right
//horDir: if horDir is 0, means move to up
//		: if horDir is 1, means mvoe to bottom
Void ObjectCandidate::edgeMatch(ObjectCandidate* corCand, UInt verDir, UInt horDir, Bool &verMatch, Bool &horMatch)
{
	UInt ctuSize = m_pcCurPic->getCtuWid();
	UInt corLeftEdge = 0, corUpEdge = 0, corRigEdge = 0, corBotEdge = 0;
	corCand->getObjEdge(corLeftEdge, corUpEdge, corRigEdge, corBotEdge, false);
	std::vector<std::vector<std::pair<Int, Bool>>>& corPxlElement = corCand->getPxlElement();
	std::vector<std::vector<Int>> curCtuData;
	std::vector<std::vector<Int>> corCtuData;
	//for each row 
	for (UInt row = 0;(m_uiUpEdge+ row*ctuSize) < m_uiBottomEdge; row++)
	{
		//for each col
		std::vector<Int> rowData;
		for (UInt col = 0;( col*ctuSize+ m_uiLeftEdge)< m_uiRightEdge; col++)
		{
			if (m_vPxlElement[row*ctuSize][col*ctuSize].first != -1)
				rowData.push_back(1);
			else
				rowData.push_back(0);
		}
		curCtuData.push_back(rowData);
	}
	for (UInt row = 0; (corUpEdge + row*ctuSize) < corBotEdge; row++)
	{
		std::vector<Int> rowData;
		for (UInt col = 0; (col*ctuSize + corLeftEdge) < corRigEdge; col++)
		{
			if (corPxlElement[row*ctuSize][col*ctuSize].first != -1)
				rowData.push_back(1);
			else
				rowData.push_back(0);
		}
		corCtuData.push_back(rowData);
	}
}
//
Void ObjectCandidate::calEdgeShift(ObjectCandidate* corCand, UInt &horShift, UInt &verShift)
{
	UInt rowIdx = 0, colIdx = 0;
	UInt ctuSize = m_pcCurPic->getCtuHig();
	std::vector<std::vector<Int>> curCtuData;
	std::vector<std::vector<Int>> corCtuData;
	UInt corLeftEdge = 0, corUpEdge = 0, corRightEdge = 0, corBottEdge = 0;
	std::vector<std::vector<std::pair<Int, Bool>>>& corPxlElement = corCand->getPxlElement();
	corCand->getObjEdge(corLeftEdge, corUpEdge, corRightEdge, corBottEdge, false);
	for (rowIdx = 0; (rowIdx*ctuSize + m_uiUpEdge) < m_uiBottomEdge; rowIdx++)
	{
		std::vector<Int> rowData;
		for (colIdx = 0; (colIdx*ctuSize + m_uiLeftEdge) < m_uiRightEdge; ++colIdx)
		{
			if (m_vPxlElement[rowIdx*ctuSize][colIdx*ctuSize].first != -1)
				rowData.push_back(1);
			else
				rowData.push_back(0);
		}
	}
	for (rowIdx = 0; (rowIdx*ctuSize + corUpEdge) < corBottEdge; rowIdx++)
	{
		std::vector<Int> rowData;
		for (colIdx = 0; (colIdx*ctuSize + corLeftEdge) < corRightEdge; colIdx++)
		{
			if (corPxlElement[rowIdx*ctuSize][colIdx*ctuSize].first != -1)
				rowData.push_back(1);
			else
				rowData.push_back(0);
		}
	}
	UInt verDir = 0, horDir = 0;
	if (corLeftEdge < m_uiLeftEdge)
	{
		//move to left 
		//corleft < curLeft and corRig<=curRig
		if (corRightEdge <= m_uiRightEdge)
		{
			//
			if ((m_uiLeftEdge - corLeftEdge)>ctuSize)
			{

			}
		}

	}
}
//not complete
//orgHorDir: means if there is
Void ObjectCandidate::detectShiftDir(ObjectCandidate* corCand, UInt orgHorDir, UInt orgVerDir, UInt &curHorDir, UInt &curVerDir)
{
	UInt ctuSize = 0;
	//horDir: horizontal direction 
	//horDir =1: move to left; horDir =2: move to right; horDir =3: both edge extended 
	UInt horDir = 0, verDir = 0;
	std::vector<std::vector<Int>> curCtuData;
	std::vector<std::vector<Int>> corCtuData;
	UInt corLeftEdge = 0, corUpEdge = 0, corRigEdge = 0, corBotEdge = 0;
	std::vector<std::vector<std::pair<Int, Bool>>>& corPxlElement = corCand->getPxlElement();

	corCand->getObjEdge(corLeftEdge, corUpEdge, corRigEdge, corBotEdge, false);
	for (UInt rowIdx = 0; (rowIdx*ctuSize + m_uiUpEdge) < m_uiBottomEdge; rowIdx++)
	{
		std::vector<Int> rowData;
		for (UInt colIdx = 0; (m_uiLeftEdge + colIdx*ctuSize) < m_uiRightEdge; colIdx++)
		{
			if (m_vPxlElement[rowIdx*ctuSize][colIdx*ctuSize].first != -1)
				rowData.push_back(1);
			else
				rowData.push_back(-1);
		}
		curCtuData.push_back(rowData);
	}
	for (UInt rowIdx = 0; (rowIdx*ctuSize + m_uiUpEdge) < m_uiBottomEdge; rowIdx++)
	{
		std::vector<Int> rowData;
		for (UInt colIdx = 0; (colIdx*ctuSize + m_uiLeftEdge) < m_uiRightEdge; colIdx++)
		{
			if (corPxlElement[rowIdx*ctuSize][colIdx*ctuSize].first != -1)
				rowData.push_back(1);
			else
				rowData.push_back(-1);
		}
		corCtuData.push_back(rowData);
	}
	//horizontal motion exist 
	if (orgHorDir != 0)
	{
		//left 
//		if (orgHorDir == 1)
//		{
//			if (corLeftEdge < m_uiLeftEdge)
//				horDir = 1;
//			if (corLeftEdge == )
//		}
		//right
//		if (orgHorDir == 2)
//		{
//
//		}
	}
	else
	{
		//move to left 
		//horDir =1;
		UInt leftShiftVal = 0;
		if (corLeftEdge < m_uiLeftEdge)
		{
			if (m_uiLeftEdge - corLeftEdge>ctuSize)
				leftShiftVal = 0;
			else
			{
				leftShiftVal = 1;
				horDir = 1;
			}
		}
		//move to right
		//horDir =2;
		UInt rightShiftVal = 0;
		if (corRigEdge > m_uiRightEdge)
		{
			if (corRigEdge - m_uiRightEdge > ctuSize)
				rightShiftVal = 0;
			else
			{
				rightShiftVal = 1;
				//only right edge extended
				if (horDir == 0)
					horDir = 2;
				else
				{
					if (orgHorDir == 2)
						horDir = 2;
					if (orgHorDir == 0)
						horDir = 3;
					//right edge changed
					if (orgHorDir == 1)
					{
						if (horDir == 1 || (m_uiLeftEdge));
						else;
					}
				}
			}
		}
	}
	//move to up
	//verDir =1
	UInt upShiftVal = 0;
	if (corUpEdge < m_uiUpEdge)
	{
		if (m_uiUpEdge - corUpEdge>ctuSize)
			upShiftVal = 0;
		else
		{
			upShiftVal = 1;
			verDir = 1;
		}
	}

	//move to bottom 
	//verDir =2
	UInt botShiftVal = 0;
	if (m_uiBottomEdge < corBotEdge)
	{
		if (corBotEdge - m_uiBottomEdge>ctuSize)
			botShiftVal = 0;
		else
		{
			botShiftVal = 1;
			if (verDir == 0)
				verDir = 2;
			else
			{
				if (orgVerDir == 0)
					verDir = 3;
				if (orgVerDir == 2)
					verDir = 2;
				if (orgVerDir == 1)
				{

				}
			}
		}
	}
}


void ObjectCandidate::getObjectPos(UInt& xPos, UInt& yPos, int flag)
{
	map<CtuData*, Bool, ctuAddrIncCompare>	m_mCtuElement;
	CtuData *ctu = NULL;
	int x = 0, y = 0;
	vector<UInt> allBeginX;
	vector<UInt> allBeginY;
	m_mCtuElement = this->getCtuElements();
	auto it = m_mCtuElement.begin();
	allBeginX.clear();
	allBeginY.clear();
	int beginX = 0, beginY = 0;
	while (it != m_mCtuElement.end())
	{
		ctu = (*it).first;
		x = ctu->getAbsx(); y = ctu->getAbsy();
		allBeginX.push_back(x);  allBeginY.push_back(y);
		it++;
	}
	if (allBeginX.size() != 0)
	{
		int mid = allBeginX.size() / 2;
		if (flag == 1)
		{
			xPos = allBeginX[mid]; yPos = allBeginY[mid];
		}
		else
		{
			xPos = allBeginX[0]; yPos = allBeginY[0];
		}
	}
	else
	{
		cout << "cannot get object's position" << endl;
		int debug; cin >> debug;
	}
}

void ObjectCandidate::getObjectCentroid(UInt& xPos, UInt& yPos)
{
	map<CtuData*, Bool, ctuAddrIncCompare>	m_mCtuElement;
	CtuData *ctu = NULL;
	UInt x = 0, y = 0;

	m_mCtuElement = this->getCtuElements();
	auto it = m_mCtuElement.begin();
	
	UInt totalNum = 0;
	UInt totalX = 0, totalY = 0;
	while (it != m_mCtuElement.end())
	{
		ctu = (*it).first;
		x = ctu->getAbsx(); y = ctu->getAbsy();
		it++;
		totalX += x; totalY += y;
		totalNum++;
	}
	if (totalNum != 0)
	{
		xPos = totalX /totalNum;  yPos = totalY / totalNum;
	}
	else
	{
		cout << "cannot get object's centroid" << endl;
		int debug; cin >> debug;
		exit(0);
	}
}

bool ObjectCandidate::inObject(UInt x, UInt y)
{
	map<CtuData*, Bool, ctuAddrIncCompare>	m_mCtuElement;
	m_mCtuElement = this->getCtuElements();
	auto it = m_mCtuElement.begin();
	CtuData *ctu = NULL;

	while (it != m_mCtuElement.end())
	{
		ctu = (*it).first;

		if (x >= ctu->getAbsx() && x < (ctu->getAbsx() + 32) && y >= ctu->getAbsy() && y <(ctu->getAbsy() + 32))
		{

			return true;
		}
		it++;

	}
	return false;

}

bool ObjectCandidate::findSegmentCtu()
{
	//map<CtuData*, Bool, ctuAddrIncCompare>	m_mCtuElement;
	//CtuData *ctu = NULL;
	//CtuData *ctu2 = NULL;
	//int x = 0, y = 0;
	//int x2 = 0, y2 = 0;
	//vector<int> object1X;
	//vector<int> object1Y;
	//vector<int> object2X;
	//vector<int> object2Y;

	//m_mCtuElement = this->getCtuElements();
	//auto it = m_mCtuElement.begin();
	//auto itNew = m_mCtuElement.begin();

	//while (it != m_mCtuElement.end())
	//{
	//	ctu = (*it).first;
	//	x = ctu->getAbsx(); y = ctu->getAbsy();
	//	itNew = it;
	//	itNew++;
	//	while (itNew != m_mCtuElement.end())
	//	{
	//		ctu2 = (*itNew).first;
	//		x2 = ctu2->getAbsx(); y2 = ctu2->getAbsy();
	//		if ((x - x2) <= 32 && (y - y2) <= 32)
	//		{
	//		}
	//		itNew++;
	//	}
	//	it++;
	//}
	return false;
}

ObjectCandidate* ObjectCandidate::findCorCand(Int frameInterval, Bool &findFlag)
{
	ObjectCandidate* corCand=NULL;
	Int PicNum = (Int)m_pcCurPic->getPicNum();

	UInt ctuSize = m_pcCurPic->getCtuWid(), ctuInRow = m_pcCurPic->getCtuInRow(), ctuInCol = m_pcCurPic->getCtuInCol();//
	UInt leftEdge, upEdge, rightEdge, bottomEdge, curCtuAddr, corCtuAddr;//
	UInt verIndex, horIndex, verLength, horLength, verCoord, horCoord;//
	std::vector<std::vector<Int>> searchArea;//

	UInt totalPicNum = m_pcCurPic->getCurVideoData()->getPicNumInVideo();
	PicData* pPic;
	CtuData *corCtu;
	ObjectCandidate* corObject;
	std::map<ObjectCandidate*, Bool> traveledObjectCand;
	Bool  matchFlag = false, matchFlag2 = false;
	Bool addFlag = false;
	PicNum += frameInterval;
	if (PicNum < 2 || PicNum >= totalPicNum)
	{
		findFlag = false;
		return corCand;
	}
	pPic = m_pcCurPic->getCurVideoData()->getPic(PicNum);

	/////////////////////
	getObjEdge(leftEdge, upEdge, rightEdge, bottomEdge, false);
	verLength = (bottomEdge - upEdge) / ctuSize;
	horLength = (rightEdge - leftEdge) / ctuSize;
	verIndex = upEdge / ctuSize;
	horIndex = leftEdge / ctuSize;
	for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		std::vector<Int> rowIdx;
		for (UInt horIdx = 0; horIdx < horLength; horIdx++)
			rowIdx.push_back(-2);
		searchArea.push_back(rowIdx);
	}
	auto iterBeg = m_mCtuElement.begin();
	while (iterBeg != m_mCtuElement.end())
	{
		curCtuAddr = iterBeg->first->getCtuAddr();
		horCoord = curCtuAddr%ctuInRow;
		verCoord = curCtuAddr / ctuInRow;
		searchArea[verCoord - verIndex][horCoord - horIndex] = curCtuAddr;
		++iterBeg;
	}

	for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		UInt horIdx = 0, rhorIdx = 0;
		for (horIdx = 0; horIdx < horLength; horIdx++)
		{
			if (searchArea[verIdx][horIdx] != -2)
				break;
			else
				searchArea[verIdx][horIdx] = -1;
		}
		for (rhorIdx = horLength - 1; rhorIdx>horIdx; rhorIdx--)
		{
			if (searchArea[verIdx][rhorIdx] != -2)
				break;
			else
				searchArea[verIdx][rhorIdx] = -1;
		}
	}
	for (UInt horIdx = 0; horIdx < horLength; horIdx++)
	{
		UInt verIdx = 0, rverIdx = 0;
		for (verIdx = 0; verIdx < verLength; verIdx++)
		{
			if (searchArea[verIdx][horIdx] >= 0)
				break;
			else
				searchArea[verIdx][horIdx] = -1;
		}
		for (rverIdx = verLength - 1; rverIdx>verIdx; rverIdx--)
		{
			if (searchArea[rverIdx][horIdx] >= 0)
				break;
			else
				searchArea[rverIdx][horIdx] = -1;
		}
	}


	for (UInt verIdx = 0; verIdx < verLength && !findFlag; verIdx++)
	//for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		for (UInt horIdx = 0; horIdx < horLength && !findFlag; horIdx++)
		//for (UInt horIdx = 0; horIdx < horLength; horIdx++)
		{
			if (searchArea[verIdx][horIdx] != -1)
			{
				if (searchArea[verIdx][horIdx] >= 0)
					corCtuAddr = searchArea[verIdx][horIdx];
				else
				{
					corCtuAddr = verIndex*ctuInRow + horIndex + verIdx*ctuInRow + horIdx;
					searchArea[verIdx][horIdx] = corCtuAddr;
				}
				corCtu = pPic->getCtu(corCtuAddr);
				if (corCtu->getCtuSta() == STA_MOVOBJ && corCtu->getObject() != NULL && corCtu->getObject()->getMovingOjbect()->getMovObjectNum() != 0 )
				{
					corObject = corCtu->getObject();
					if (traveledObjectCand.find(corObject) == traveledObjectCand.end() && corObject->getMovingOjbect() != m_pcMovingObject)
					{
						//matchFlag = compareWithCorObject(corObject, false);
						Float ratio1 = 0, ratio2 = 0;
						//matchFlag = calculateBoundingBoxOverlaps(corObject,ratio1,ratio2);
						matchFlag = true;
						if (matchFlag) 
						{
							findFlag = true;
							traveledObjectCand[corObject] = true;
							corCand=corObject;
						}
						//break;
					}
				}
			}
		}
	}
	return corCand;
}



Void ObjectCandidate::splitCtusIntoLeftRight(std::map<CtuData*, Bool, ctuAddrIncCompare>& leftCtu, std::map<CtuData*, Bool, ctuAddrIncCompare>& rightCtu, Float leftRightRatio)
{
	std::map<CtuData*, Bool, ctuAddrIncCompare> currCtu = m_mCtuElement;
	
	if (currCtu.size() <= 1)
	{
		cout << "the ctus cannot be splitted, error" << endl;
		exit(0);
	}

	//cout << ""
	UInt leftNum = Float(currCtu.size())*leftRightRatio;
	UInt  mostLeftAbsX = this->getCurPic()->getPicWid();
	UInt mostTopAbsY = this->getCurPic()->getPicHig();
	UInt i = 1;
	while (i <= leftNum)
	{
		mostLeftAbsX = this->getCurPic()->getPicWid();
		mostTopAbsY = this->getCurPic()->getPicHig();
		auto tempCtu = currCtu.begin();
		auto beg = currCtu.begin();
		while (beg != currCtu.end())
		{
			if (beg->first->getAbsx() <= mostLeftAbsX && beg->first->getAbsx() <= mostTopAbsY)
			{
				mostLeftAbsX = beg->first->getAbsx();
				mostTopAbsY = beg->first->getAbsy();
				tempCtu = beg;
			}
			beg++;
		}
		
		//cout << "begin inserting ctu" << endl;
		leftCtu.insert(*tempCtu);
		//cout << "222 inserting ctu" << endl;
		tempCtu = currCtu.erase(tempCtu);
		//cout << "333 inserting ctu" << endl;
		//cout << "i is " << i << endl;
		i++;

	}
	//cout << "777 inserting ctu" << endl;
	rightCtu.swap(currCtu);
	//cout << "888 inserting ctu" << endl;

}

UInt ObjectCandidate::findPubCtu(ObjectCandidate* pObject)
{
	std::map<CtuData*, Bool, ctuAddrIncCompare> ctu1 = this->getCtuElements();
	std::map<CtuData*, Bool, ctuAddrIncCompare> ctu2 = pObject->getCtuElements();
	std::map<UInt, CtuData*> ctuAddress1;
	std::map<UInt, CtuData*> ctuAddress2;
	std::map<CtuData*, Bool, ctuAddrIncCompare> pubCtus;
	// find public Ctus

	auto beg = ctu1.begin();
	while (beg != ctu1.end())
	{
		ctuAddress1.insert(make_pair(beg->first->getCtuAddr(), beg->first));
		beg++;
	}

	beg = ctu2.begin();
	while (beg != ctu2.end())
	{
		ctuAddress2.insert(make_pair(beg->first->getCtuAddr(), beg->first));
		beg++;
	}

	UInt pubArea = 0;
	auto begAddr = ctuAddress1.begin();
	while (begAddr != ctuAddress1.end())
	{
		if (ctuAddress2.find(begAddr->first) != ctuAddress2.end())
			pubArea++;
		begAddr++;
	}


	/*beg = ctu1.begin();
	while (beg != ctu1.end())
	{
		if (ctu2.find(beg->first) != ctu2.end())
			pubCtus.insert(*beg);
			beg++;
	}*/

	return pubArea;



}

//std::map<CtuData*, Bool, ctuAddrIncCompare> ObjectCandidate::findPubCtu(ObjectCandidate* cand1)
//{
//
//	
//
//}

//
//fucntion not used now 
/****************************************************************************************
Void ObjectCandidate::calcuDirWithCorCand(ObjectCandidate* pCorCand)
{
	UInt corLeftEdge = 0, corUpEdge = 0, corRigEdge = 0, corBotEdge = 0;
	UInt diffEdgeNum = 0;
	MotionDirection curDir = REMAIN_STATIONARY;
	std::vector<motionInfo*> curMotionInfo;
	Bool movLeft = false, movRig = false, movUp = false, movDown = false;
	getObjEdge(corLeftEdge, corUpEdge, corRigEdge, corBotEdge, true);
	pCorCand->getObjEdge(corLeftEdge, corUpEdge, corRigEdge, corBotEdge, true);
	curMotionInfo.clear();
	if (corLeftEdge < m_uiLeftEdge)
	{
		movLeft = true;
		++diffEdgeNum;
		curDir = MOVE_LEFT;
	}
	if (corRigEdge > m_uiRightEdge)
	{
		movRig = true;
		++diffEdgeNum;
		curDir = MOVE_RIGHT;
	}
	if (corUpEdge < m_uiUpEdge)
	{
		movUp = true;
		++diffEdgeNum;
		curDir = MOVE_UP;
	}
	if (corBotEdge > m_uiBottomEdge)
	{
		movDown = true;
		++diffEdgeNum;
		curDir = MOVE_DOWN;
	}
	if (diffEdgeNum == 0)
		pCorCand->addMotionInfo(pCorCand, curMotionInfo);
	if (diffEdgeNum == 1)
		oneDirCompare(pCorCand,curDir);
	if (diffEdgeNum == 2)
	{
		if (movLeft == true && movRig == true)
		{
			oneDirCompare(pCorCand, MOVE_LEFT);
			oneDirCompare(pCorCand, MOVE_RIGHT);
		}
		else if (movUp == true && movDown == true)
		{
			oneDirCompare(pCorCand, MOVE_UP);
			oneDirCompare(pCorCand, MOVE_DOWN);
		}
		else
			twoDirCompare(pCorCand);
	}
	if (diffEdgeNum == 3)
	{
		if (movLeft == false)
			threeDirCompare(pCorCand, MOVE_LEFT);
		else if (movRig == false)
			threeDirCompare(pCorCand, MOVE_RIGHT);
		else if (movUp == false)
			threeDirCompare(pCorCand, MOVE_UP);
		else if (movDown == false)
			threeDirCompare(pCorCand, MOVE_DOWN);
	}
	if (diffEdgeNum == 4)
	{
			threeDirCompare(pCorCand, MOVE_LEFT);
			threeDirCompare(pCorCand, MOVE_RIGHT);
			threeDirCompare(pCorCand, MOVE_UP);
			threeDirCompare(pCorCand, MOVE_DOWN);
	}
}
****************************************************************************************************/
//
//function not need now
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//function not needed

//
//calculate delta with corresponding candidate
//corCand: corresponding candidate 
//priorDir: if motion direction exist 
//if current candidate suitable  to compare with corresponding candidate return true  
//else return false and reset scan candidate 
Bool ObjectCandidate::calDeltaWithCorCand(ObjectCandidate* corCand, UInt priorHorDir, UInt priorVerDir, UInt &curHorDir, UInt &curVerDir)
{
	return false;
	
}
//calculate delta x and delta y with more than one frame intervals 
Void ObjectCandidate::calDeltaXY(std::vector<ObjectCandidate*>& candidates)
{
	if (candidates.size() < 2)
		return;
	UInt tempLeftEdge, tempUpEdge, tempRigEdge, tempBotEdge;
	ObjectCandidate* tempCand;
	auto beg = candidates.begin();
	++beg;
	while (beg != candidates.end())
	{
		tempCand = *beg;
		tempCand->getObjEdge(tempLeftEdge, tempUpEdge, tempRigEdge, tempBotEdge, false);
		if (((tempLeftEdge > m_uiLeftEdge) && (tempRigEdge > m_uiRightEdge))
			|| ((tempLeftEdge < m_uiLeftEdge) && (tempRigEdge < m_uiRightEdge))
			|| ((tempUpEdge<m_uiUpEdge) && (tempBotEdge<m_uiBottomEdge)
			|| (tempUpEdge>m_uiUpEdge&&tempBotEdge>m_uiBottomEdge)))
		{

		}
		else
			++tempCand;
	}
}
//
//search the common area of a group candidates 
/*parameter:
  candidates: candidates fo current group
*/

//compare current objcet with corresponding object
Bool ObjectCandidate::compareWithCorObject(ObjectCandidate* pCorObject, Bool fillCandFlag)
{
	//return false;
	//return true;
	// &ratio1, Float ratio2
	//this->calculateBoundingBoxOverlaps();
	Float sizeRatio = 0, curPadingPixel = 0, corPadingPixel = 0, padPixel = 0;
	UInt ctuSize = m_pcCurPic->getCtuWid();
	UInt ctuArea = ctuSize*ctuSize;
	UInt corVol = ((UInt)(pCorObject->getCtuElements().size()))*ctuArea;
	UInt curVol = (UInt)(m_mCtuElement.size())*ctuArea;
	Bool matchFlag = false, shiftFlag = false, unuseflag = false;
	sizeRatio = (Float)corVol / curVol;

	UInt ctuSize1 = pCorObject->getCtuElements().size();
	UInt ctuSize2 = m_mCtuElement.size();


	//it is useful to first calculate the bounding box overlapping of the two objects,
	//if they are highly overlapped, we can make a conclusion directly.
	/*Float ratio1, ratio2 = 0;
	Bool overlap = false;
	calculateBoundingBoxOverlaps(pCorObject,ratio1,ratio2);*/

	//the following calculating their overlapping bounding box

	if (sizeRatio > 0.6 && sizeRatio < 1.4)
		shiftFlag = true;//it has been proved that there is no bug here


	if (!fillCandFlag && (sizeRatio<0.2 || sizeRatio>5))
		return false;

	if (ctuSize1 >= 8 && ctuSize2 >= 8 && (sizeRatio<=0.5 || sizeRatio>=2))
		return false;

	if (ctuSize1 >= 4 && ctuSize2 >= 4 && (sizeRatio <= 0.33 || sizeRatio >= 3))
		return false;

	if (sizeRatio < 0.3)
	{
		//if next object is small 
		pCorObject->calculateOverlaps(this, corPadingPixel, true);
		if (corPadingPixel / corVol >= 0.5f)
		{
			if (fillCandFlag)
			{
				pCorObject->addPriorCand(this, true);
				m_mNextCandidates[pCorObject] = false;
			}
			return true;
		}
		return false;
	}
	else if (sizeRatio > 3)
	{
		calculateOverlaps(pCorObject, padPixel, true);
		if (padPixel / curVol >= 0.5f)
		{
			if (fillCandFlag)
			{
				m_mNextCandidates[pCorObject] = true;
				pCorObject->addPriorCand(this, false);
			}
			return true;
		}
		return false;
	}
	else
	{
		if (corVol > curVol)
			matchFlag = pCorObject->calculateMaxOverlaps(this, padPixel, shiftFlag, unuseflag);
		else
			matchFlag = calculateMaxOverlaps(pCorObject, padPixel, shiftFlag, unuseflag);
		if (shiftFlag && fillCandFlag)
		{
			if (padPixel / corVol >= 0.7f&& padPixel / curVol >= 0.7f)
			{
				m_bMatchWithNext = true;
				pCorObject->setPriorMatch(true);
			}
		}
		if (padPixel / corVol >= 0.5f && padPixel / curVol >= 0.5f)
		{
			if (fillCandFlag)
			{
				m_mNextCandidates[pCorObject] = true;
				pCorObject->addPriorCand(this, true);
			}
			return true;
		}
		else
		{
			if (corVol > curVol)
			{
				if (matchFlag || padPixel / curVol >= 0.5)
				{
					if (fillCandFlag)
					{
						m_mNextCandidates[pCorObject] = true;
						pCorObject->addPriorCand(this, true);
					}
					return true;
				}
			}
			else
			{
				if (matchFlag || padPixel / corVol >= 0.5)
				{
					if (fillCandFlag)
					{
						m_mNextCandidates[pCorObject] = true;
						pCorObject->addPriorCand(this, true);
					}
					return true;
				}
			}
		}
		return false;
	}
}

Bool ObjectCandidate::findCorObject(MovingObject * pObject, ObjectCandidate* pBigObject,ObjectCandidate*& returnCand)
{
	UInt picNum = this->getPicNum();

	UInt headPicNum = pObject->getHeadPointer()->getCurPic()->getPicNum();
	UInt tailPicNum = pObject->getTailPointer()->getCurPic()->getPicNum();
	VideoData* pCurVideo = pObject->getHeadPointer()->getCurPic()->getCurVideoData();
	PicData* pCurPic=NULL;

	if (picNum<headPicNum || picNum>tailPicNum)
		return false;

	pCurPic = pCurVideo->getPic(picNum-1);

	ObjectCandidate* pCurObjCand = NULL;
	auto beg = pCurPic->getObjects().begin();
	while (beg != pCurPic->getObjects().end())
	{
		if ((*beg)->getMovingOjbect() == pObject)
		{
			pCurObjCand = *beg;
			break;
		}
		++beg;
	}

	if (pCurObjCand == NULL)
	{
		returnCand = NULL;
		return false;
	}
	else
	{
		Bool match = false;
		Float ratio1, ratio2 = 0;
		match=pCurObjCand->calculateBoundingBoxOverlaps(pBigObject, ratio1, ratio2);
		if (match == true)
		{
			returnCand = pCurObjCand;
			return true;
		}
		
		else 
		{
			returnCand = NULL;
			return false;
		}

	}


}

Bool ObjectCandidate::checkObjectsOverlapFrames(ObjectCandidate* pCorObject)
{
	MovingObject* pObject1 = this->getMovingOjbect();
	MovingObject* pObject2 = pCorObject->getMovingOjbect();
	
	Bool answer=pObject1->checkObjectsOverlapFrames(pObject2);

	return answer;

}


//compare current objcet with corresponding object
Bool ObjectCandidate::judgeMatch(ObjectCandidate* pCorObject)
{
	Float sizeRatio = 0;
	UInt ctuSize = m_pcCurPic->getCtuWid();
	UInt ctuArea = ctuSize*ctuSize;

	UInt x, y = 0;
	UInt currLeft = 0, currRight = 0, currTop = 0, currBottom = 0;
	UInt corLeft = 0, corRight = 0, corTop = 0, corBottom = 0;
	auto ctu = m_mCtuElement.begin();
	while (ctu != m_mCtuElement.end())
	{
	
		(*ctu).first->getAbsXY(x, y);
		if (ctu == m_mCtuElement.begin())
		{
			currLeft = currRight = x;
			currTop = currBottom = y;
		}
		else
		{
			if (x <currLeft)
				currLeft = x;
			if (x >currRight)
				currRight = x;
			if (y < currTop)
				currTop = y;
			if (x >currBottom)
				currBottom = x;

		}
		ctu++;
	}

	ctu = pCorObject->m_mCtuElement.begin();
	while (ctu != pCorObject->m_mCtuElement.end())
	{

		(*ctu).first->getAbsXY(x, y);
		if (ctu == pCorObject->m_mCtuElement.begin())
		{
			corLeft = corRight = x;
			corTop = corBottom = y;
		}
		else
		{
			if (x <corLeft)
				corLeft = x;
			if (x >corRight)
				corRight = x;
			if (y < corTop)
				corTop = y;
			if (x >corBottom)
				corBottom = x;

		}
		ctu++;
	}

	UInt currVol = (UInt)(currRight - currLeft)*(currBottom - currTop)*ctuArea;
	UInt corVol = (UInt)(corRight - corLeft)*(corBottom - corTop)*ctuArea;

	sizeRatio = (Float)corVol / currVol;

	if (sizeRatio >= 0.5 && sizeRatio <= 2)
		return true;
	else
	return false;
	
}



Void ObjectCandidate::chainCorObject(Int frameInterval, Bool forward)
{
	UInt PicNum = m_pcCurPic->getPicNum(), ctuSize = m_pcCurPic->getCtuWid();
	//cout << "now the page is " << PicNum << endl;
	UInt ctuInRow = m_pcCurPic->getCtuInRow(), ctuInCol = m_pcCurPic->getCtuInCol();
	UInt totalPicNum = m_pcCurPic->getCurVideoData()->getPicNumInVideo();
	UInt leftEdge, rightEdge, upEdge, bottomEdge, curCtuAddr, corCtuAddr;
	UInt verIndex, horIndex, verLength, horLength, horCoord, verCoord;
	PicData* pPic;
	CtuData *corCtu;
	ObjectCandidate* corObject;
	std::map<ObjectCandidate*, Bool> traveledObjectCand;
	std::vector<std::vector<Int>> searchArea;	//search area within current object candidate boundaries
	std::vector<std::vector<Int>> extendedArea;	//move current search area to different direction  
	Bool stopFlag = false, matchFlag = false;
	Bool addFlag = false;
	PicNum += frameInterval;

	if (PicNum <= 2 || PicNum >= totalPicNum)
	{
		return;
	}

	getObjEdge(leftEdge, upEdge, rightEdge, bottomEdge, false);
	verLength = (bottomEdge - upEdge) / ctuSize;
	horLength = (rightEdge - leftEdge) / ctuSize;
	verIndex = upEdge / ctuSize;
	horIndex = leftEdge / ctuSize;

	
	for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		std::vector<Int> rowIdx;
		for (UInt horIdx = 0; horIdx < horLength; horIdx++)
			rowIdx.push_back(-2);
		searchArea.push_back(rowIdx);
	}

	auto beg = m_mCtuElement.begin();
	while (beg != m_mCtuElement.end())
	{
		curCtuAddr = beg->first->getCtuAddr();
		horCoord = curCtuAddr%ctuInRow;				//vertical   coordiante in ctu level
		verCoord = curCtuAddr / ctuInRow;			//horizontal coordiante in  ctu level 
		searchArea[verCoord - verIndex][horCoord - horIndex] = curCtuAddr;
		
		++beg;
	}

	for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		for (UInt horIdx = 0;horIdx<horLength; horIdx++)
		{
			if (searchArea[verIdx][horIdx] != -2)
				break;
			else
				searchArea[verIdx][horIdx] = -1;
		}
		for (UInt horIdx = horLength - 1; horIndex >= 0; horIndex--)
		{
			if (searchArea[verIdx][horIdx] != -2)
				break;
			else
				searchArea[verIdx][horIdx] = -1;
		}
	}

	


	for (UInt horIdx = 0; horIdx < horLength; horIdx++)
	{
		for (UInt verIdx = 0; verIdx < verLength; verIdx++)
		{
			if (searchArea[verIdx][horIdx] >= 0)
				break;
			else
				searchArea[verIdx][horIdx] = -1;
		}
		for (UInt verIdx = verLength - 1; verIdx >= 0; verIdx--)
		{
			if (searchArea[verIdx][horIdx] >= 0)
				break;
			else
				searchArea[verIdx][horIdx] = -1;
		}
	}

	
	pPic = m_pcCurPic->getCurVideoData()->getPic(PicNum);

	


	//search object candidate in corresponding area.
	for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		for (UInt horIdx = 0; horIdx < horLength; horIdx++)
		{
			if (searchArea[verIdx][horIdx] != -1)
			{
				if (searchArea[verIdx][horIdx] >= 0)
					corCtuAddr = searchArea[verIdx][horIdx];
				else
				{
					corCtuAddr = verIndex*ctuInRow + horIndex + verIdx*ctuInRow + horIdx;
					searchArea[verIdx][horIdx] = corCtuAddr;
				}
				corCtu = pPic->getCtu(corCtuAddr);
				if (corCtu->getCtuSta() == STA_MOVOBJ && corCtu->getObject() != NULL)
				{
					corObject = corCtu->getObject();
					if (traveledObjectCand.find(corObject) == traveledObjectCand.end())
					{
						traveledObjectCand[corObject] = true;
						compareWithCorObject(corObject, true);

						//UInt picNum = m_pcCurPic->getPicNum();
						//if (picNum == 150 )
						//{
						//	UInt adress = corCtu->getCtuAddr();
						//	
						//	cout << "fffffirst connect" << endl;
						//	cout << "the edge is " << leftEdge << "  " << upEdge << "  " << rightEdge << "   " << bottomEdge << endl;
						//	cout << "the ctu adress is " << adress << endl;
						//	cout << "verIndex is " << verIndex << endl;
						//	cout << "verIdx is " << verIdx << endl;
						//	cout << "horIdx is " << horIdx << endl;
						//	cout << "now is " << picNum << " , exrer,find" << endl;
						//	//if (frameInterval < 0)
						//	//cout << "prior" << endl;
						//	//else
						//	//cout << "sub" << endl;
						//	cout << "cennected pic is " << corObject->getPicNum() << endl;
						//	UInt x, y = 0;
						//	this->getObjCentroid(x, y, true);
						//	cout << "the curr x,y is " << x << " " << y << endl;
						//	corObject->getObjCentroid(x, y, true);
						//	cout << "the corr x,y is " << x << " " << y << endl;
						//	std::map<CtuData*, Bool, ctuAddrIncCompare> ctuData=corObject->getCtuElements();
						//	auto beg = ctuData.begin();
						//	while (beg != ctuData.end())
						//	{
						//		UInt adress = 0;
						//		adress=beg->first->getCtuAddr();
						//		cout << adress << " ";
						//		beg++;
						//	}
						//	cout << endl;

						//	int tt; cin >> tt;
						//}
					}
				}
			}
		}
	}


	//extend search 
	//if no corresponding object candidate found in current search area,
	//then move current search area to each surround direction:
	//top left ; up ; top right ; left  ; right  ; bottom left ; bottom ; bottom  right 
	//0 1 2 
	//3   4
	//5 6 7
	if (m_mNextCandidates.size() == 0)
	{
		Int extendDir = 0;
		Bool stopSearch = false;
		while (extendDir < 8 && stopSearch == false)
		{
			extendedArea = searchArea;
			for (UInt verIdx = 0; verIdx < verLength; verIdx++)
			{
				for (UInt horIdx = 0; horIdx < horLength; horIdx++)
				{
					if (searchArea[verIdx][horIdx] >= 0)
					{
						corCtuAddr = searchArea[verIdx][horIdx];
						//move left 
						if ((extendDir == 0 || extendDir == 3 || extendDir == 5) && (corCtuAddr%ctuInRow>0))
							extendedArea[verIdx][horIdx] -= 1;
						//move up 
						if ((extendDir == 2 || extendDir == 4 || extendDir == 7) && (corCtuAddr%ctuInRow<(ctuInRow - 1)))
							extendedArea[verIdx][horIdx] += 1;
						//move right 
						if ((extendDir == 0 || extendDir == 1 || extendDir == 2) && (corCtuAddr / ctuInRow > 0))
							extendedArea[verIdx][horIdx] -= ctuInRow;
						//move down
						if ((extendDir == 5 || extendDir == 6 || extendDir == 7) && (corCtuAddr / ctuInRow < (ctuInCol - 1)))
							extendedArea[verIdx][horIdx] += ctuInRow;
					}
				}
			}
			for (UInt verIdx = 0; verIdx < verLength; verIdx++)
			{
				for (UInt horIdx = 0; horIdx < horLength; horIdx++)
				{
					if (extendedArea[verIdx][horIdx] != -1)
					{
						if (extendedArea[verIdx][horIdx] >= 0)
							corCtuAddr = extendedArea[verIdx][horIdx];
						else
						{
							//corCtuAddr = verIndex + verIdx*ctuInRow + horIdx;
							corCtuAddr = verIndex*ctuInRow + horIndex + verIdx*ctuInRow + horIdx;
							extendedArea[verIdx][horIdx] = corCtuAddr;
						}
						corCtu = pPic->getCtu(corCtuAddr);
						if (corCtu->getCtuSta() == STA_MOVOBJ && corCtu->getObject() != NULL)
						{
							corObject = corCtu->getObject();
							if (traveledObjectCand.find(corObject) == traveledObjectCand.end())
							{
								traveledObjectCand[corObject] = true;
								stopSearch = compareWithCorObject(corObject, true);
							}
						}
					}
				}
			}
			++extendDir;
		}
		//if (traveledObjectCand.size() == 1 && m_mNextCandidates.size() == 0)
		//{
		//	Float curVol, corVol;
		//	UInt ctuArea = ctuSize* ctuSize;
		//	Float padPxl = 0, sizeRatio = 0;
		//	Bool matflag;
		//	ObjectCandidate* pTempObjCand = traveledObjectCand.begin()->first;
		//	curVol = ((Float)m_mCtuElement.size())*ctuArea;
		//	corVol = ((Float)pTempObjCand->getCtuElements().size())*ctuArea;
		//	sizeRatio = curVol > corVol ? corVol / curVol : curVol / corVol;
		//	if (sizeRatio > 0.2)
		//	{
		//		calculateMaxOverlaps(pTempObjCand, padPxl, true, matflag);
		//		if (((Float)padPxl / curVol >= 0.3 && (Float)padPxl / corVol >= 0.3) || (Float)padPxl / curVol >= 0.5 || (Float)padPxl / corVol >= 0.5)
		//		{
		//			//UInt PicNum = m_pcCurPic->getPicNum();
		//			//if (PicNum == 150 || PicNum == 151)
		//			//{
		//			//	cout << "second connect" << endl;
		//			//	cout << "now is " << PicNum << " , exrer,not find yet" << endl;
		//			//	//if (frameInterval < 0)
		//			//	//cout << "prior" << endl;
		//			//	//else
		//			//	//cout << "sub" << endl;
		//			//	cout << "pTempObjCand pic is " << pTempObjCand->getPicNum() << endl;
		//			//	UInt x, y = 0;
		//			//	this->getObjCentroid(x, y, true);
		//			//	cout << "the curr x,y is " << x << " " << y << endl;
		//			//	pTempObjCand->getObjCentroid(x, y, true);
		//			//	cout << "the pTempObjCand x,y is " << x << " " << y << endl;
		//			//	int tt; cin >> tt;
		//			//}
		//			m_mNextCandidates[pTempObjCand] = false;
		//			pTempObjCand->addPriorCand(this, false);
		//		}
		//	}
		//}
	}
	//search 
}


//
Void ObjectCandidate::endExtention(Int frameInterval, Bool &findFlag, Bool ShortStep, UInt extendTurn, Bool specialSearch)
{
	Int PicNum = (Int)m_pcCurPic->getPicNum();

	UInt ctuSize = m_pcCurPic->getCtuWid(), ctuInRow = m_pcCurPic->getCtuInRow(), ctuInCol = m_pcCurPic->getCtuInCol();//
	UInt leftEdge, upEdge, rightEdge, bottomEdge, curCtuAddr, corCtuAddr;//
	UInt verIndex, horIndex, verLength, horLength, verCoord, horCoord;//
	std::vector<std::vector<Int>> searchArea;//
	std::vector<std::vector<Int>> shiftSearchArea;//

	UInt totalPicNum = m_pcCurPic->getCurVideoData()->getPicNumInVideo();
	PicData* pPic;
	CtuData *corCtu;
	ObjectCandidate* corObject;
	std::map<ObjectCandidate*, Bool> traveledObjectCand;
	std::map<MovingObject*, Bool> traveledMovingObject;
	Bool  matchFlag = false, matchFlag2 = false;
	Bool addFlag = false;
	PicNum += frameInterval;
	if (PicNum < 2 || PicNum >= totalPicNum)
	{
		findFlag = false;
		return;
	}
	pPic = m_pcCurPic->getCurVideoData()->getPic(PicNum);

	/////////////////////

	cout << "finish end11" << endl;
	getObjEdge(leftEdge, upEdge, rightEdge, bottomEdge, false);
	verLength = (bottomEdge - upEdge) / ctuSize;
	horLength = (rightEdge - leftEdge) / ctuSize;
	verIndex = upEdge / ctuSize;
	horIndex = leftEdge / ctuSize;
	for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		std::vector<Int> rowIdx;
		for (UInt horIdx = 0; horIdx < horLength; horIdx++)
			rowIdx.push_back(-2);
		searchArea.push_back(rowIdx);
	}
	auto iterBeg = m_mCtuElement.begin();
	while (iterBeg != m_mCtuElement.end())
	{
		curCtuAddr = iterBeg->first->getCtuAddr();
		horCoord = curCtuAddr%ctuInRow;
		verCoord = curCtuAddr / ctuInRow;
		searchArea[verCoord - verIndex][horCoord - horIndex] = curCtuAddr;
		++iterBeg;
	}

	cout << "finish end22" << endl;

	for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		UInt horIdx = 0, rhorIdx = 0;
		for (horIdx = 0; horIdx < horLength; horIdx++)
		{
			if (searchArea[verIdx][horIdx] != -2)
				break;
			else
				searchArea[verIdx][horIdx] = -1;
		}
		for (rhorIdx = horLength - 1; rhorIdx>horIdx; rhorIdx--)
		{
			if (searchArea[verIdx][rhorIdx] != -2)
				break;
			else
				searchArea[verIdx][rhorIdx] = -1;
		}
	}
	for (UInt horIdx = 0; horIdx < horLength; horIdx++)
	{
		UInt verIdx = 0, rverIdx = 0;
		for (verIdx = 0; verIdx < verLength; verIdx++)
		{
			if (searchArea[verIdx][horIdx] >= 0)
				break;
			else
				searchArea[verIdx][horIdx] = -1;
		}
		for (rverIdx = verLength - 1; rverIdx>verIdx; rverIdx--)
		{
			if (searchArea[rverIdx][horIdx] >= 0)
				break;
			else
				searchArea[rverIdx][horIdx] = -1;
		}
	}

	cout << "finish end33" << endl;
	//for (UInt verIdx = 0; verIdx < verLength && !findFlag; verIdx++)
	for (UInt verIdx = 0; verIdx < verLength; verIdx++)
	{
		//for (UInt horIdx = 0; horIdx < horLength && !findFlag; horIdx++)
		for (UInt horIdx = 0; horIdx < horLength; horIdx++)
		{
			if (searchArea[verIdx][horIdx] != -1)
			{
				if (searchArea[verIdx][horIdx] >= 0)
					corCtuAddr = searchArea[verIdx][horIdx];
				else// this else should not be executed 
				{
					corCtuAddr = verIndex*ctuInRow + horIndex + verIdx*ctuInRow + horIdx;
					//corCtuAddr = verIndex + verIdx*ctuInRow + horIdx;
					searchArea[verIdx][horIdx] = corCtuAddr;
				}
				corCtu = pPic->getCtu(corCtuAddr);
				if (corCtu->getCtuSta() == STA_MOVOBJ && corCtu->getObject() != NULL && corCtu->getObject()->getMovingOjbect()->getMovObjectNum() != 0)
				{
					corObject = corCtu->getObject();
					
					//traveledMovingObject
					if (traveledObjectCand.find(corObject) == traveledObjectCand.end())
					{
						matchFlag = compareWithCorObject(corObject, false);
						/*if (matchFlag == false && extendTurn == 4)
						{
							Float ratio1, ratio2;
							matchFlag = calculateBoundingBoxOverlaps(corObject, ratio1, ratio2);
						}*/

						matchFlag2 = checkObjectsOverlapFrames(corObject);

						Float ratio1 = 0, ratio2 = 0;
						Bool match1 = 0;
						match1 = this->calculateBoundingBoxOverlaps(corObject, ratio1, ratio2);

						//if ((matchFlag && matchFlag2 && corObject->getMovingOjbect() != m_pcMovingObject) || (matchFlag && extendTurn == 4))
						if ((matchFlag && matchFlag2 && corObject->getMovingOjbect() != m_pcMovingObject) || extendTurn == 4)
						{
							if (extendTurn == 4)
							{
								ObjectCandidate * returnCand = NULL;
								Bool findObject = findCorObject(corCtu->getObject()->getMovingOjbect(), corObject, returnCand);
								if (findObject)
								{
									addNeighbourCand(returnCand, true);

								}
							}

							Bool matchFlag3 = true;

							if (extendTurn == 3)
							{
								UInt leftDir = 0, upDir = 0, rigDir = 0, botDir = 0;
								Bool atBorder1 = false, atBorder2 = false;
								atBorder1 = this->atTheBorder(leftDir, upDir, rigDir, botDir);
								atBorder2 = corObject->atTheBorder(leftDir, upDir, rigDir, botDir);
								if (atBorder1 == true || atBorder2 == true)
									matchFlag3 = false;
								if (frameInterval < 0)
								{
									if (!corObject->checkCandAtObjectHeadsOrTails(false))
										matchFlag3 = false;
								}
								else
								{
									if (!corObject->checkCandAtObjectHeadsOrTails(true))
										matchFlag3 = false;
								}

							}

							if (matchFlag3 == true)
							{
								findFlag = true;
								traveledObjectCand[corObject] = true;

								if (frameInterval < 0)
								{
									//if (corObject->getMovingOjbect()->getTailPointer()->getCurPic()->getPicNum() >= m_pcCurPic->getPicNum())
									if (corObject->checkCandAtObjectHeadsOrTails(false))
										m_pcMovingObject->addPriorMovObj(corObject->getMovingOjbect(), true);
									else
									{
										m_pcMovingObject->addPriorMovObj(corObject->getMovingOjbect(), false);
									}
								}
								else
								{
									//if (corObject->getMovingOjbect()->getHeadPointer()->getCurPic()->getPicNum() <= m_pcCurPic->getPicNum())
									if (corObject->checkCandAtObjectHeadsOrTails(true))
										m_pcMovingObject->addSubSeqMovObj(corObject->getMovingOjbect(), true);

									else
									{
										m_pcMovingObject->addSubSeqMovObj(corObject->getMovingOjbect(), false);
									}
								}
							}

						}
						//break;
					}
				}
			}
		}
	}

	cout << "finish end55" << endl;
	if ((extendTurn <= 2) && (traveledObjectCand.size() == 0 || !findFlag))
	{
		Int extendDir = 0;
		Bool stopSearch = false;

		while (extendDir < 8 && stopSearch == false)
		{
			std::vector<std::vector<Int>> tempArea;
			shiftSearchArea.clear();
			tempArea.swap(shiftSearchArea);
			tempArea.clear();
			shiftSearchArea = searchArea;
			for (UInt verIdx = 0; verIdx < verLength; verIdx++)
			{
				for (UInt horIdx = 0; horIdx < horLength; horIdx++)
				{
					if (searchArea[verIdx][horIdx] >= 0)
					{
						corCtuAddr = searchArea[verIdx][horIdx];
						//move left 
						if ((extendDir == 0 || extendDir == 3 || extendDir == 5) && (corCtuAddr%ctuInRow>0))
							shiftSearchArea[verIdx][horIdx] -= 1;
						//move right 
						if ((extendDir == 2 || extendDir == 4 || extendDir == 7) && (corCtuAddr%ctuInRow<(ctuInRow - 1)))
							shiftSearchArea[verIdx][horIdx] += 1;
						//move up, up left, up right 
						if ((extendDir == 0 || extendDir == 1 || extendDir == 2) && (corCtuAddr / ctuInRow > 0))
							shiftSearchArea[verIdx][horIdx] -= ctuInRow;
						//move down, down left ,down right
						if ((extendDir == 5 || extendDir == 6 || extendDir == 7) && (corCtuAddr / ctuInRow < (ctuInCol - 1)))
							shiftSearchArea[verIdx][horIdx] += ctuInRow;
					}
				}
			}
			for (UInt verIdx = 0; verIdx < verLength &&!findFlag; verIdx++)
			{
				for (UInt horIdx = 0; horIdx < horLength && !findFlag; horIdx++)
				{
					if (shiftSearchArea[verIdx][horIdx] != -1)
					{
						if (shiftSearchArea[verIdx][horIdx] >= 0)
							corCtuAddr = shiftSearchArea[verIdx][horIdx];
						else
						{
							corCtuAddr = verIndex*ctuInRow + horIndex + verIdx*ctuInRow + horIdx;
							//corCtuAddr = verIndex + verIdx*ctuInRow + horIdx;
							shiftSearchArea[verIdx][horIdx] = corCtuAddr;
						}
						corCtu = pPic->getCtu(corCtuAddr);
						if (corCtu->getCtuSta() == STA_MOVOBJ && corCtu->getObject() != NULL && corCtu->getObject()->getMovingOjbect()->getMovObjectNum() != 0)
						{
							corObject = corCtu->getObject();
							if (traveledObjectCand.find(corObject) == traveledObjectCand.end())
							{

								matchFlag = compareWithCorObject(corObject, false);


								matchFlag2 = checkObjectsOverlapFrames(corObject);
								if (matchFlag && matchFlag2 && corObject->getMovingOjbect() != m_pcMovingObject)
								{
									stopSearch = true;
									findFlag = true;
									traveledObjectCand[corObject] = true;


									if (frameInterval < 0)
									{
										//if (corObject->getMovingOjbect()->getTailPointer()->getCurPic()->getPicNum() >= m_pcCurPic->getPicNum())
										if (corObject->checkCandAtObjectHeadsOrTails(false))
											m_pcMovingObject->addPriorMovObj(corObject->getMovingOjbect(), true);
										else
											m_pcMovingObject->addPriorMovObj(corObject->getMovingOjbect(), false);
									}
									else
									{
										//if (corObject->getMovingOjbect()->getHeadPointer()->getCurPic()->getPicNum() <= m_pcCurPic->getPicNum())
										if (corObject->checkCandAtObjectHeadsOrTails(true))
											m_pcMovingObject->addSubSeqMovObj(corObject->getMovingOjbect(), true);

										else
											m_pcMovingObject->addSubSeqMovObj(corObject->getMovingOjbect(), false);
									}


								}
								break;
							}
						}
					}
				}
			}
			++extendDir;
		}

		//cout << "finish end55middle" << endl;
		//if (traveledObjectCand.size() == 1)
		//{
		//	Bool matflag;
		//	Float curVol, corVol;
		//	UInt ctuArea = ctuSize* ctuSize;
		//	Float padPxl = 0, sizeRatio;
		//	ObjectCandidate* pTempObjCand = traveledObjectCand.begin()->first;
		//	curVol = ((Float)m_mCtuElement.size())*ctuArea;
		//	corVol = ((Float)pTempObjCand->getCtuElements().size())*ctuArea;
		//	sizeRatio = curVol < corVol ? curVol / corVol : corVol / curVol;
		//	if (sizeRatio>0.2)
		//	{
		//		calculateMaxOverlaps(pTempObjCand, padPxl, true, matflag);
		//		matchFlag2 = checkObjectsOverlapFrames(pTempObjCand);

		//		//if ((padPxl / curVol >= 0.25 && padPxl / corVol >= 0.25 || padPxl / curVol >= 0.5 || padPxl / corVol >= 0.5)  && matchFlag2)
		//		if ((padPxl / curVol >= 0.25 && padPxl / corVol >= 0.25) && matchFlag2)
		//		{

		//			findFlag = true;

		//			if (frameInterval < 0)
		//			{
		//				//if (corObject->getMovingOjbect()->getTailPointer()->getCurPic()->getPicNum() >= m_pcCurPic->getPicNum())
		//				if (pTempObjCand->checkCandAtObjectHeadsOrTails(false))
		//					m_pcMovingObject->addPriorMovObj(pTempObjCand->getMovingOjbect(), true);
		//				else
		//					m_pcMovingObject->addPriorMovObj(pTempObjCand->getMovingOjbect(), false);
		//			}
		//			else
		//			{
		//				//if (corObject->getMovingOjbect()->getHeadPointer()->getCurPic()->getPicNum() <= m_pcCurPic->getPicNum())
		//				if (pTempObjCand->checkCandAtObjectHeadsOrTails(true))
		//					m_pcMovingObject->addSubSeqMovObj(pTempObjCand->getMovingOjbect(), true);

		//				else
		//					m_pcMovingObject->addSubSeqMovObj(pTempObjCand->getMovingOjbect(), false);
		//			}
		//		}
		//	}

		//}
	}
	cout << "finish end66" << endl;
}
//
Void ObjectCandidate::checkEdgeCtu()
{
	auto beg = m_mCtuElement.begin();
	while (beg != m_mCtuElement.end())
	{
		beg->first->checkPosition();
		++beg;
	}
}
//
//
//if a ctu in current candidate is surrounded by ctus that belong to current candidate,
//and its corresponding ctu in prior or next frame is classified as background 
//and the corresponding one has neighbor ctus belong to current object, 
//reset the corresponding one as part of current object
Void ObjectCandidate::recoverCtu()
{
	CtuData* pCorCtu;
	auto beg = m_mCtuElement.begin();
	cout << "qqqqq" << endl;
	while (beg != m_mCtuElement.end())
	{
		if (beg->first->getEdgeFlag() == false || beg->first->getCentFlag() == true)
		{
			//prior subsequence frame exist corresponding ctu
			pCorCtu = beg->first->getCorCtu(true);
			if (pCorCtu != NULL&& pCorCtu->getCtuSta() != STA_MOVOBJ)
			{
				pCorCtu->setCtuSta(STA_MOVOBJ);
				pCorCtu->travelNeighborCtu(this, true);
				pCorCtu->travelNeighborCtu(NULL, true);
				if (pCorCtu->getObject() != NULL)
					pCorCtu->getObject()->checkEdgeCtu();
				else
					pCorCtu->setCtuSta(STA_POSSIBLE_OBJ);
			}
			//prior frame exist corresponding ctu
			pCorCtu = beg->first->getCorCtu(false);
			if (pCorCtu != NULL&& pCorCtu->getCtuSta() != STA_MOVOBJ && pCorCtu->getObject() == NULL)
			{
				
				pCorCtu->setCtuSta(STA_MOVOBJ);
				pCorCtu->travelNeighborCtu(this, true);
				pCorCtu->travelNeighborCtu(NULL, true);
				if (pCorCtu->getObject() != NULL)
					pCorCtu->getObject()->checkEdgeCtu();
				else
					pCorCtu->setCtuSta(STA_POSSIBLE_OBJ);
			}
		}
		++beg;
	}

	cout << "middlee" << endl;
	beg = m_mCtuElement.begin();
	while (beg != m_mCtuElement.end())
	{
		if (m_pcCurPic->getPicNum() == m_pcCurPic->getCurVideoData()->getPicNumInVideo())
			break;
		pCorCtu = beg->first->getCorCtu(true);
		if (pCorCtu != NULL && pCorCtu->getCtuSta() != STA_MOVOBJ)
		{
			pCorCtu->checkPosition();
			if (pCorCtu->getCentFlag() == true || pCorCtu->getEdgeFlag() == false)
			{
				pCorCtu->setCtuSta(STA_MOVOBJ);
				pCorCtu->travelNeighborCtu(this, true);
				pCorCtu->travelNeighborCtu(NULL, true);
				if (pCorCtu->getObject() != NULL)
					pCorCtu->getObject()->checkEdgeCtu();
				else
					pCorCtu->setCtuSta(STA_POSSIBLE_OBJ);
			}
		}
		pCorCtu = beg->first->getCorCtu(false);
		if (pCorCtu != NULL && pCorCtu->getCtuSta() != STA_MOVOBJ)
		{
			pCorCtu->checkPosition();
			if (pCorCtu->getCentFlag() == true || pCorCtu->getEdgeFlag() == false)
			{
				pCorCtu->setCtuSta(STA_MOVOBJ);
				pCorCtu->travelNeighborCtu(this, true);
				pCorCtu->travelNeighborCtu(NULL, true);
				if (pCorCtu->getObject() != NULL)
					pCorCtu->getObject()->checkEdgeCtu();
				else
					pCorCtu->setCtuSta(STA_POSSIBLE_OBJ);
			}
		}
		++beg;
	}
	
}
//
Void ObjectCandidate::dealObjcetChain()
{
	UInt MovingObjNum = 0;
	Bool newMovingObject = false;
	
	if (m_mPriorCandidates.size() == 0)
		newMovingObject = true;
	if (m_mPriorCandidates.size() > 1)
		newMovingObject = true;
	if (m_mPriorCandidates.size() == 1 && m_mPriorCandidates.begin()->first->getNextCand().size() > 1)
		newMovingObject = true;
	if (m_mPriorCandidates.size() == 1 && m_mPriorCandidates.begin()->first->getNextCand().size() == 1)
		newMovingObject = false;

	//UInt picNum = this->getPicNum();
	//UInt centroidX = 0l, centroidY = 0;
	//if (picNum == 666)
	//{
	//	this->getObjCentroid(centroidX, centroidY,true);
	//	cout << "x,y is " << centroidX << "  " << centroidY << endl;
	//	cout << "pic num is " << picNum << endl;
	//	cout << "PriorCandidates size is "<<m_mPriorCandidates.size() << endl;
	//	if (m_mPriorCandidates.size() >= 2)
	//	{

	//		auto beg = m_mPriorCandidates.begin();

	//		cout << "their pic nums are " << beg->first->getPicNum() << endl;
	//		beg->first->getObjCentroid(centroidX, centroidY, true);
	//		cout << "x,y is " << centroidX << "  " << centroidY << endl;
	//		beg++;
	//		cout << "their pic nums are " << beg->first->getPicNum() << endl;
	//		beg->first->getObjCentroid(centroidX, centroidY, true);
	//		cout << "x,y is " << centroidX << "  " << centroidY << endl;
	//	}

	//	cout << "prior next is  " << m_mPriorCandidates.begin()->first->getNextCand().size() << endl;

	//	cout << "new object is " << newMovingObject << endl; 
	//	//int ss; cin >> ss;
	//}
	if (newMovingObject)
	{
		MovingObjNum = m_pcCurPic->getCurVideoData()->getInitialNum();
		m_pcMovingObject = new MovingObject(MovingObjNum);
		m_pcMovingObject->addCandidate(this->getPicNum(), this);

		m_pcMovingObject->setSeqLength(1);
		m_pcMovingObject->resetHeadPointer();
		m_pcMovingObject->resetTailPointer();
		//m_pcMovingObject->setHeadPointer(this);
		//m_pcMovingObject->setTailPointer(this);
		m_pcMovingObject->setMovObjectNum(MovingObjNum);
		
		
		m_pcCurPic->getCurVideoData()->increaseInitialNum();
		m_pcCurPic->getCurVideoData()->addMovObjToVector(m_pcMovingObject);
		//cout << "the moving object is " << MovingObjNum << endl;
		//m_pcMovingObject->setMovObjectNum((UInt)m_pcCurPic->getCurVideoData()->getTempMovObj().size());
	}
	if (newMovingObject == false)
	{
		m_pcMovingObject = m_mPriorCandidates.begin()->first->getMovingOjbect();
		//m_pcMovingObject->setTailPointer(this);
		m_pcMovingObject->addCandidate(this->getPicNum(), this);
		m_pcMovingObject->resetTailPointer();
		m_pcMovingObject->inreaseLength(1);
		
	}
}

//deal with the condition that one object corresponding to more than one object in next frame
//or many objects correspond to one object in next frame
Bool ObjectCandidate::dealOneToMany()
{
	UInt longSubSeq = 0;
	/*Bool sameObj = true;
	Bool noiseExst = false;
	MovingObject *pNextMoving = NULL;
	auto beg = m_mNextCandidates.begin();
	while (beg != m_mNextCandidates.end())
	{
		if (pNextMoving == NULL)
			pNextMoving = beg->first->getMovingOjbect();
		else
		{
			if (beg->first->getMovingOjbect() != pNextMoving)
			 {
				sameObj = false;
				break;
			}
		}
		++beg;
	}*/

	map<MovingObject*, Bool> pObjMap;
	auto beg = m_mNextCandidates.begin();
	while (beg != m_mNextCandidates.end())
	{
		pObjMap.insert(make_pair(beg->first->getMovingOjbect(), beg->second));
		++beg;
	}

	auto begMap = pObjMap.begin();
	while (begMap != pObjMap.end())
	{
		if (begMap->first->getSeqLength() > 5)
			longSubSeq++;
		//if (beg->second == true)
			//noiseExst = true;
		begMap++;
	}


	//if (sameObj == false)
	//{
		/*beg = m_mNextCandidates.begin();
		while (beg != m_mNextCandidates.end())
		{
			if (beg->first->getMovingOjbect()->getSeqLength() > 3)
				longSubSeq++;
			if (beg->second == true)
				noiseExst = true;
			beg++;
		}*/
		//if (noiseExst == true)
		//{
		//	beg = m_mNextCandidates.begin();
		//	while (beg != m_mNextCandidates.end())
		//	{
		//		//if (beg->second == false && beg->first->getNextCand().size() ==0)
		//		//{
		//			//beg->first->getMovingOjbect()->setMovObjectNum(0);
		//			//m_mNextCandidates.erase(beg++);
		//		//}
		//		//else
		//			beg++;
		//	}
		//}
	//}
	if (longSubSeq > 1)
			return false;

	else	
		return true;
}

//deal with condition that more than one object in prior frame correspond to current object
Bool ObjectCandidate::dealManyToOne()
{
	//MovingObject *pFirTempMovObj = NULL, *pSecTempMovObj = NULL;
	//vector<MovingObject*> pObjVec;
	map<MovingObject*, Bool> pObjMap;
	UInt longSubSeqNum = 0;
	auto beg = m_mPriorCandidates.begin();
	while (beg != m_mPriorCandidates.end())
	{
		//pObjVec.push_back(beg->first->getMovingOjbect());
		pObjMap.insert(make_pair(beg->first->getMovingOjbect(),beg->second));
		beg++;
	}
	
	/* beg = m_mPriorCandidates.begin();
	while (beg != m_mPriorCandidates.end())
	{
		auto beg2 = beg;
		beg2++;

		while (beg2 != m_mPriorCandidates.end())
		{
			if (beg->first->getMovingOjbect() == beg2->first->getMovingOjbect())
			{
				cout << "there is an unexpected " << endl;
				int aa; cin >> aa;
				beg2=m_mPriorCandidates.erase(beg2);
				
				continue;
			}
			beg2++;
		}
		beg++;
	}*/

	auto begMap = pObjMap.begin();
	auto begMap2 = pObjMap.begin();
	while (begMap != pObjMap.end())
	{
		auto begMap2 = begMap;
		begMap2++;

		while (begMap2 != pObjMap.end())
		{
			if (begMap->first == begMap2->first)
			{
				cout << "there is an unexpected " << endl;
				int aa; cin >> aa;
				begMap2 = pObjMap.erase(begMap2);

				continue;
			}
			begMap2++;
		}
		begMap++;
	}

	begMap = pObjMap.begin();
	while (begMap != pObjMap.end())
	{
		if (begMap->first->getSeqLength() > 5)
			++longSubSeqNum;
		begMap++;
	}
	
	//while (beg != m_mPriorCandidates.end())
	//{
	//	if (beg->first->getMovingOjbect()->getSeqLength() > 3)
	//		++longSubSeqNum;
	//	//if (pFirTempMovObj == NULL || pSecTempMovObj == NULL)
	//	//{
	//	//	if (pFirTempMovObj == NULL)
	//	//		pFirTempMovObj = beg->first->getMovingOjbect();
	//	//	if (pSecTempMovObj == NULL)
	//	//	{
	//	//		if (beg->first->getMovingOjbect()->getPriorMovObj().size() > 0)
	//	//			pSecTempMovObj = beg->first->getMovingOjbect()->getPriorMovObj().begin()->first;
	//	//	}
	//	//	/*if (beg->first->getMovingOjbect()->getSeqLength() > 3)
	//	//		++longSubSeqNum;*/
	//	//	++beg;
	//	//	continue;
	//	//}
	///*	else
	//	{
	//		if (beg->first->getMovingOjbect() != pFirTempMovObj)
	//		{
	//			if (beg->first->getMovingOjbect()->getPriorMovObj().size() > 0 && pSecTempMovObj != NULL)
	//			{
	//				if (beg->first->getMovingOjbect()->getPriorMovObj().begin()->first != pSecTempMovObj)
	//					return false;
	//					
	//			}
	//		}
	//	}*/
	//	beg++;
	//}
	if (longSubSeqNum < 2)
		return true;
	else
		return false;
}

Void ObjectCandidate::seperate()
{
	//MovingObject *pFirTempMovObj = NULL, *pSecTempMovObj = NULL;
	//UInt longSubSeqNum = 0;
	//auto beg = m_mPriorCandidates.begin();
	//while (beg != m_mPriorCandidates.end())
	//{
	//	if (pFirTempMovObj == NULL || pSecTempMovObj == NULL)
	//	{
	//		if (pFirTempMovObj == NULL)
	//			pFirTempMovObj = beg->first->getMovingOjbect();
	//		if (pSecTempMovObj == NULL)
	//		{
	//			if (beg->first->getMovingOjbect()->getPriorMovObj().size() > 0)
	//				pSecTempMovObj = beg->first->getMovingOjbect()->getPriorMovObj().begin()->first;
	//		}
	//		if (beg->first->getMovingOjbect()->getSeqLength() > 3)
	//			++longSubSeqNum;
	//		++beg;
	//		continue;
	//	}
	//	else
	//	{
	//		if (beg->first->getMovingOjbect() != pFirTempMovObj)
	//		{
	//			if (beg->first->getMovingOjbect()->getPriorMovObj().size() > 0 && pSecTempMovObj != NULL)
	//			{
	//				if (beg->first->getMovingOjbect()->getPriorMovObj().begin()->first != pSecTempMovObj)
	//					return false;
	//			}
	//		}
	//	}
	//	beg++;
	//}
	//if (longSubSeqNum < 2)
	//	return true;
	//else
	//	return false;

}


//
Void ObjectCandidate::assignMovObjToPriorCandidate()
{
	ObjectCandidate *pObjCand = NULL;
	Bool match = false;
	UInt i = 1;
	auto beg = m_mPriorCandidates.begin();
	while (beg != m_mPriorCandidates.end())
	{
		pObjCand = beg->first;
		/*Float ratio1, ratio2;
		Bool match=this->calculateBoundingBoxOverlaps(pObjCand, ratio1, ratio2);
		if (!match)
		{
			beg++;
			continue;
		}
		MovingObject* currObj = this->getMovingOjbect();
		MovingObject* priorObj = pObjCand->getMovingOjbect();
		if (!(currObj->getSeqLength() <= 8 || priorObj->getSeqLength() <= 8))
		{
			beg++;
			continue;
		}*/
		match=checkObjectsOverlapFrames(pObjCand);

		if (beg->first->getMovingOjbect() != m_pcMovingObject && match)
			beg->first->getMovingOjbect()->resetElement(m_pcMovingObject);
	
		if (beg->first->getMovingOjbect()->getPriorMovObj().size() > 0)
			beg->first->getMovingOjbect()->assignMovObjToPrior(m_pcMovingObject);
		++beg;
		cout << "i is " << i << endl;
	}
	cout << "begin reseting lengthEEEEE" << endl;
	//if(beg->first==NULL)
		//cout<<t
	//beg->first->getMovingOjbect()->resetObjLength();
}
//
Void ObjectCandidate::assignMovObjToSubseqCandidate()
{
	ObjectCandidate *pObjCand = NULL;
	Bool match = false;
	auto beg = m_mNextCandidates.begin();

	while (beg != m_mNextCandidates.end())
	{
		/*pObjCand = beg->first;
		Float ratio1, ratio2;
		Bool match = this->calculateBoundingBoxOverlaps(pObjCand, ratio1, ratio2);
		if (!match)
		{
			beg++;
			continue;
		}
		MovingObject* currObj = this->getMovingOjbect();
		MovingObject* nextObj = pObjCand->getMovingOjbect();
		if (!(currObj->getSeqLength() <= 8 || nextObj->getSeqLength() <= 8))
		{
			beg++;
			continue;
		}*/
		pObjCand = beg->first;
		match = checkObjectsOverlapFrames(pObjCand);
		if (beg->first->getMovingOjbect() != m_pcMovingObject && match)
			beg->first->getMovingOjbect()->resetElement(m_pcMovingObject);
		cout << "debug point 22222" << endl;
		beg++;
	}
}
//
Void ObjectCandidate::mergePriorObject()
{
	Bool sameMovObject = true;
	MovingObject* tempMovingObject = NULL;
	if (m_mPriorCandidates.size() > 1)
	{
		auto beg = m_mPriorCandidates.begin();
		while (beg != m_mPriorCandidates.end())
		{
			if (beg->first->getMovingOjbect()->getSeqLength() <= 4 )
			{
				if (beg->first->getMovingOjbect() != m_pcMovingObject)
					beg->first->getMovingOjbect()->resetElement(m_pcMovingObject);
			}
			++beg;
		}
	}
}
//
Void ObjectCandidate::mergeTailObject()
{
	Bool sameMovObject = true;
	MovingObject* tempMovingObject = NULL;
	if (m_mNextCandidates.size() > 1)
	{
		auto beg = m_mNextCandidates.begin();
		while (beg != m_mNextCandidates.end())
		{
			if (beg->first->getMovingOjbect()->getSeqLength() <= 4)
			{
				if (beg->first->getMovingOjbect() != m_pcMovingObject)
					beg->first->getMovingOjbect()->resetElement(m_pcMovingObject);
			}
			++beg;
		}

	}
}
//
Bool ObjectCandidate::removable(Bool forward, Bool strictFlag)
{
	PicData* pCorPic;
	CtuData* corCtu;
	Bool  possibleMotion = false;
	UInt curPicNum = m_pcCurPic->getPicNum();
	auto beg = m_mCtuElement.begin();
	while (beg != m_mCtuElement.end())
	{
		if (curPicNum > 2 && forward == true)
		{
			pCorPic = m_pcCurPic->getCurVideoData()->getPic(curPicNum - 2);
			corCtu = pCorPic->getCtu(beg->first->getCtuAddr());
			if (corCtu->getCtuSta() == STA_MOVOBJ )
				possibleMotion = true;
			if (strictFlag && corCtu->getCtuSta() == STA_POSSIBLE_OBJ)
				possibleMotion = true;
		}
		if ((curPicNum < m_pcCurPic->getCurVideoData()->getPicNumInVideo() - 1) && forward == false)
		{
			pCorPic = m_pcCurPic->getCurVideoData()->getPic(curPicNum);
			corCtu = pCorPic->getCtu(beg->first->getCtuAddr());
			if (corCtu->getCtuSta() == STA_MOVOBJ)
				possibleMotion = true;
			if (strictFlag && corCtu->getCtuSta() == STA_POSSIBLE_OBJ)
				possibleMotion = true;
		}
		beg++;
	}
	if (possibleMotion)
		return false;
	else
		return true;
}
/*moving object class*/
//MovingObject::MovingObject() :m_uiMovObjectNum(0)
//, m_uiSeqLength(0)
//, m_uiGroups(0)
//, m_colorNum(0)
//, m_colorFlag(false)
//, m_bIncreased(false)
//, m_bRealHead(true)
//, m_bRealTail(true)
//, m_bSubSeq(false)
//, m_bHeadExtended(false)
//, m_bTailExtended(false)
//{
//	m_pcHeadPointer = NULL;
//	m_pcTailPointer = NULL;
//	m_pcBegUseful = NULL;
//	m_pcRBegUseful = NULL;
//	//m_pcRelateMovObj = NULL;
//	m_pcRelateMovObj = this;
//}

MovingObject::MovingObject(UInt objNum) :m_uiSeqLength(0)
, m_uiGroups(0)
, m_colorNum(0)
, m_colorFlag(false)
, m_bIncreased(false)
, m_bRealHead(true)
, m_bRealTail(true)
, m_bSubSeq(false)
, m_bHeadExtended(false)
, m_bTailExtended(false)
{
	m_uiMovObjectNum = objNum;
	m_pcHeadPointer = NULL;
	m_pcTailPointer = NULL;
	m_pcBegUseful = NULL;
	m_pcRBegUseful = NULL;
	//m_pcRelateMovObj = NULL;
	m_pcRelateMovObj = this;
}

MovingObject::~MovingObject()
{
	m_pcHeadPointer = NULL;
	m_pcTailPointer = NULL;
}

Void MovingObject::clearElement()
{
	m_pcHeadPointer = NULL;
	m_pcTailPointer = NULL;
	m_pcBegUseful = NULL;
	m_pcRBegUseful = NULL;
	m_uiSeqLength = 0;
}
//Void MovingObject::setCandPxlElement()
//{
//	UInt picIdx, candIdx;
//	PicData* pCurPic;
//	ObjectCandidate* pCurObjCand;
//	VideoData* pCurVideo = m_pcHeadPointer->getCurPic()->getCurVideoData();
//	auto beg = m_mPicIndex.begin();
//	while (beg != m_mPicIndex.end())
//	{
//		picIdx = beg->first;
//		candIdx = beg->second;
//		pCurPic = pCurVideo->getPic(picIdx - 1);
//		pCurObjCand = pCurPic->getObjectWithIdx(candIdx - 1);
//		pCurObjCand->setPxlElement();
//		++beg;
//	}
//}
Void MovingObject::removeNoise()
{
	if (m_uiSeqLength <= 2)
	{
		if (m_pcHeadPointer->removable(true, false) && m_pcTailPointer->removable(false, false))
			m_uiMovObjectNum = 0;
	}
}
//
Void MovingObject::travelCand()
{
	UInt headPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
	UInt tailPicNum = m_pcTailPointer->getCurPic()->getPicNum();
	VideoData* pCurVideo = m_pcHeadPointer->getCurPic()->getCurVideoData();
	PicData* pCurPic;
	while (headPicNum <= tailPicNum)
	{
		pCurPic = pCurVideo->getPic(headPicNum - 1);
		pCurPic->setEdgeCtuCandFlag(this);
		++headPicNum;
	}
	
	headPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
	while (headPicNum <= tailPicNum)
	{
		pCurPic = pCurVideo->getPic(headPicNum - 1);
		pCurPic->recoverCandidate(this);
		++headPicNum;
	}
	


	headPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
	while (headPicNum <= tailPicNum)
	{
		pCurPic = pCurVideo->getPic(tailPicNum - 1);
		pCurPic->recoverCandidate(this);
		--tailPicNum;
	}
	
}
//
Void MovingObject::setUseFulCand()
{
	UInt curCandNum = 0;
	ObjectCandidate *pCurCand = nullptr, *pPriorCand = nullptr, *pNextCand = nullptr;
	UInt curPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
	UInt endPicNum = m_pcTailPointer->getCurPic()->getPicNum();
	VideoData* pCurVideo = m_pcHeadPointer->getCurPic()->getCurVideoData();
	PicData* pCurPic;
	std::vector<ObjectCandidate*> usefulCands;
	//set candidate useful flag: only one candidate in a frame and not contact with boundary
	UInt ObjectNum = this->getMovObjectNum();
	//cout << "setting useful, the number is " << ObjectNum << endl;
	while (curPicNum <= endPicNum)
	{
		pCurPic = pCurVideo->getPic(curPicNum - 1);
		pCurPic->setCandidateUsefulFlag(this);
		++curPicNum;
	}
}
//
//Void MovingObject::resetUsefulCand()
//{
//	PicData* tempPic = nullptr;
//	UInt tempPicNum = 0, tempObjNum = 0;
//	ObjectCandidate *tempCand = nullptr, *pFirCand = nullptr, *pSecCand = nullptr;
//	VideoData* curVideo = m_pcHeadPointer->getCurPic()->getCurVideoData();
//	std::vector<ObjectCandidate*> usefulCand;
//	Bool matchFlag = false;
//	auto picIdxBeg = m_mPicIndex.begin();
//	while (picIdxBeg != m_mPicIndex.end())
//	{
//		tempPicNum = picIdxBeg->first;
//		tempObjNum = picIdxBeg->second;
//		tempPic = curVideo->getPic(tempPicNum - 1);
//		tempCand = tempPic->getObjectWithIdx(tempObjNum - 1);
//		if (tempCand->getUsefulFlag() && tempCand->getGroupNum() != 0)
//		{
//			tempCand->resetRelationWithCorCand();
//			usefulCand.push_back(tempCand);
//		}
//		++picIdxBeg;
//	}
//	auto usefulCandBeg = usefulCand.begin();
//	while (usefulCandBeg != usefulCand.end())
//	{
//		matchFlag = false;
//		pFirCand = *usefulCandBeg;
//		++usefulCandBeg;
//		if (usefulCandBeg != usefulCand.end())
//		{
//			pSecCand = *usefulCandBeg;
//			pFirCand->searchMaxCommonArea(pSecCand, matchFlag);
//			if (matchFlag == true)
//			{
//				pFirCand->setNextMatch(true);
//				pSecCand->setPriorMatch(true);
//			}
//			if (matchFlag == false && pFirCand->getPriorMatch() == false)
//				pFirCand->setUsefulFlag(false);
//		}
//		else
//		{
//			if (pFirCand->getPriorMatch() == false)
//				pFirCand->setUsefulFlag(false);
//		}
//	}
//}
//
Void MovingObject::fullfillEmpty()
{
	UInt headPicNum, tailPicNum, tempPicNum;
	UInt curPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
	UInt endPicNum = m_pcTailPointer->getCurPic()->getPicNum();
	UInt curObjNum = 1;
	ObjectCandidate* pTempCand;
	std::vector<ObjectCandidate*> useFullCand;
	VideoData* pCurVideo = m_pcHeadPointer->getCurPic()->getCurVideoData();
	PicData* pCurPic;
	std::vector<UInt> picElement;
	tempPicNum = headPicNum = m_pcHeadPointer->getPicNum();
	tailPicNum = m_pcHeadPointer->getPicNum();
	while (tempPicNum <= tailPicNum)
	{
		picElement.push_back(tempPicNum);
		++tempPicNum;
	}
	auto picEleIndex = picElement.begin();
	auto picIdxBeg = m_mPicIndex.begin();
	while (picIdxBeg != m_mPicIndex.end())
	{
		curPicNum = picIdxBeg->first;
		curObjNum = picIdxBeg->second;
		pCurPic = pCurVideo->getPic(curPicNum - 1);
		pTempCand = pCurPic->getObjectWithIdx(curObjNum - 1);
		if (pTempCand->getUsefulFlag())
		{
			useFullCand.push_back(pTempCand);
			auto findResult = find(picElement.begin(), picElement.end(), curPicNum);
			if (findResult != picElement.end())
				picElement.erase(findResult);
			++picIdxBeg;
		}
		else
		{
			pTempCand->setObjNum(0);
			picIdxBeg = m_mPicIndex.erase(picIdxBeg);
		}
	}
	PicData *curPic;
	ObjectCandidate *pPriorCand, *pCurCand, *pNextCand;
	auto usefulBeg = useFullCand.begin();
	auto nextIndex = useFullCand.begin();
	auto curIndex = useFullCand.begin();
	
	picEleIndex = picElement.begin();
	while (picEleIndex != picElement.end())
	{
		curPicNum = *picEleIndex;
		curPic = pCurVideo->getPic(curPicNum - 1);
		ObjectCandidate* addCand = new ObjectCandidate(curPic);
		curPic->addNewObj(addCand);
		addCand->setMovingObject(this);
		addCand->setUsefulFlag(false);
		addCand->setGroupNum(0);
		m_mPicIndex.insert(std::make_pair(curPicNum, addCand->getObjNum()));
		++picEleIndex;
	}
	useFullCand.clear();
	picIdxBeg = m_mPicIndex.begin();
	while (picIdxBeg != m_mPicIndex.end())
	{
		curPicNum = picIdxBeg->first;
		curObjNum = picIdxBeg->second;
		pCurPic = pCurVideo->getPic(curPicNum - 1);
		pTempCand = pCurPic->getObjectWithIdx(curObjNum - 1);
		useFullCand.push_back(pTempCand);
		++picIdxBeg;
	}
	usefulBeg = useFullCand.begin();
	pCurCand = *usefulBeg;
	if (pCurCand->getUsefulFlag() == false)
	{
		++usefulBeg;
		pNextCand = *usefulBeg;
		while (usefulBeg != useFullCand.end())
		{
			pNextCand = *usefulBeg;
			if (pNextCand->getUsefulFlag())
				break;
		}
		curIndex = useFullCand.begin();
		std::vector<std::vector<std::pair<Int, Bool>>>&padPxl = pNextCand->getPxlElement();
		while (curIndex != usefulBeg)
		{
			pCurCand = *curIndex;
			std::vector<std::vector<std::pair<Int, Bool>>>&curPxl = pCurCand->getPxlElement();
			curPxl.clear();
			auto padIndex = padPxl.begin();
			while (padIndex != padPxl.end())
			{
				curPxl.push_back(*padIndex);
				++padIndex;
			}
			++curIndex;
		}
	}
	UInt changeStep = 0, curStep;
	auto priorIndex = usefulBeg;
	pCurCand = *usefulBeg;
	while (usefulBeg != useFullCand.end())
	{
		
		if (pCurCand->getUsefulFlag() == false)
		{
			curIndex = usefulBeg;
			while (usefulBeg != useFullCand.end())
			{
				pNextCand = *usefulBeg;
				if (pNextCand->getObjNum() != 0)
					break;
				++usefulBeg;
			}
			pPriorCand = *priorIndex;
			if (usefulBeg != useFullCand.end())
			{
				pNextCand = *usefulBeg;
				changeStep = (pNextCand->getPicNum() - pPriorCand->getPicNum()) / 2;
			}
			else
				changeStep = (UInt)useFullCand.size();
			pNextCand = *usefulBeg;
			curStep = 0;
			pPriorCand = *priorIndex;
			curIndex = priorIndex;
			++curIndex;
			std::vector<std::vector<std::pair<Int, Bool>>>&padPrioPxl = pPriorCand->getPxlElement();
			while (curIndex != usefulBeg)
			{
				if (curStep < changeStep)
					pCurCand = *curIndex;
				else
					pCurCand = pNextCand;
				std::vector<std::vector<std::pair<Int, Bool>>>&padPxl = pCurCand->getPxlElement();
				std::vector<std::vector<std::pair<Int, Bool>>>&curPxl = pCurCand->getPxlElement();
				curPxl.clear();
				auto padIndex = padPxl.begin();
				while (padIndex != padPxl.end())
				{
					curPxl.push_back(*padIndex);
					++padIndex;
				}
				++curIndex;
				++curStep;
			}
		}
		if (usefulBeg != useFullCand.end())
		{
			priorIndex = usefulBeg;
			++usefulBeg;
		}
	}
}
//
Void MovingObject::setCandGroup()
{
	UInt curPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
	UInt endPicNum = m_pcTailPointer->getCurPic()->getPicNum();
	UInt curObjNum = 1, curGroupNum = 1, lastGroupNum = 0, stride = 0, nextGroupNum = 0;
	ObjectCandidate* pTempCand;
	ObjectCandidate *pFirCand, *pSecCand;// , *pThirdCand;
	std::vector<ObjectCandidate*> useFullCand;
	std::vector<ObjectCandidate*> groupCand;
	std::map<UInt, UInt>	tempGroupAndLength;
	VideoData* pCurVideo = m_pcHeadPointer->getCurPic()->getCurVideoData();
	PicData* pCurPic;
	Float padPxl;
	Bool matchFlag = false;
	//collect useful candidates 
	auto picIdxBeg = m_mPicIndex.begin();
	while (picIdxBeg != m_mPicIndex.end())
	{
		curPicNum = picIdxBeg->first;
		curObjNum = picIdxBeg->second;
		pCurPic = pCurVideo->getPic(curPicNum - 1);
		pTempCand = pCurPic->getObjectWithIdx(curObjNum - 1);
		if (pTempCand->getUsefulFlag())
			useFullCand.push_back(pTempCand);
		++picIdxBeg;
	}

	//divide them into different groups
	auto usefulCandBeg = useFullCand.begin();
	while (usefulCandBeg != useFullCand.end())
	 {
		pFirCand = *usefulCandBeg;
		++usefulCandBeg;
		if (usefulCandBeg != useFullCand.end())
		{
			pSecCand = *usefulCandBeg;
			pFirCand->calculateMaxOverlaps(pSecCand, padPxl, true, matchFlag);
			//pFirCand->searchMaxCommonArea(pSecCand, matchFlag);
			if (matchFlag == true)
			{
				pFirCand->setNextMatchCand(pSecCand);
				pSecCand->setPriorMatchCand(pFirCand);
				pFirCand->setNextMatch(true);
				pSecCand->setPriorMatch(true);
				if (pFirCand->getGroupNum() == 0)
				{
					//here increase the group number for set color index not equal to default value( i.e. moving object number)
					if (curGroupNum == m_uiMovObjectNum)
						curGroupNum++;
					pFirCand->setGroupNum(curGroupNum);
					pSecCand->setGroupNum(curGroupNum);
					pFirCand->setGroupOrder(1);
					pSecCand->setGroupOrder(2);
					m_mGroupAndLength[curGroupNum] = 2;
					++curGroupNum;
				}
				else
				{
					pSecCand->setGroupNum(pFirCand->getGroupNum());
					pSecCand->setGroupOrder(pFirCand->getGroupOrder() + 1);
					m_mGroupAndLength[pFirCand->getGroupNum()] = pFirCand->getGroupOrder() + 1;
				}
			}
			else
			{
				if (pFirCand->getGroupNum() == 0)
				{
					if (curGroupNum == m_uiMovObjectNum)
						curGroupNum++;
					pFirCand->setGroupNum(curGroupNum);
					pFirCand->setGroupOrder(1);
					m_mGroupAndLength[curGroupNum] = 1;
					++curGroupNum;
				}
			}
		}
		else
		{
			if (pFirCand->getGroupNum() == 0)
			{
				if (curGroupNum == m_uiMovObjectNum)
					curGroupNum++;
				pFirCand->setGroupNum(curGroupNum);
				pFirCand->setGroupOrder(1);
				m_mGroupAndLength[curGroupNum] = 1;
				++curGroupNum;
			}
			break;
		}
	}
	//
	//merge groups 
	
	usefulCandBeg = useFullCand.begin();

	while (usefulCandBeg != useFullCand.end())
	{
		pFirCand = *usefulCandBeg;
		if (pFirCand->getGroupNum() == 0)
		{
			++usefulCandBeg;
			continue;
		}
		else
			++usefulCandBeg;
		groupCand.clear();
		groupCand.push_back(pFirCand);
		//collect a group of candidates 
		while (usefulCandBeg != useFullCand.end())
		{
			pSecCand = *usefulCandBeg;

			if (pSecCand->getGroupNum() == 0)
			{
				++usefulCandBeg;
				continue;
			}
			if (pFirCand->getGroupNum() == pSecCand->getGroupNum())
			{
				pFirCand = pSecCand;
				groupCand.push_back(pSecCand);
				++usefulCandBeg;
			}
			else
				break;
		}
		//extend current 
		if (groupCand.size() > 1 && usefulCandBeg!=useFullCand.end())
		{
			nextGroupNum = pSecCand->getGroupNum();
			if (m_mGroupAndLength[nextGroupNum] <= 2)
				++usefulCandBeg;
			while (usefulCandBeg != useFullCand.end())
			{
				pSecCand = *usefulCandBeg;
				//extend to next group
				if (usefulCandBeg != useFullCand.end())
				{
					pSecCand = *usefulCandBeg;
					auto rBeg = groupCand.rbegin();
					pFirCand = *rBeg;
					if (pFirCand->getNextMatch() && (pFirCand->getNextMatchCand() == pSecCand))
						matchFlag = true;
					else
						pFirCand->calculateMaxOverlaps(pSecCand, padPxl, true, matchFlag);
					if (matchFlag)
					{
						pSecCand->setGroupNum(pFirCand->getGroupNum());
						pSecCand->setGroupOrder(pFirCand->getGroupOrder() + 1);
						m_mGroupAndLength[pFirCand->getGroupNum()] = pFirCand->getGroupOrder() + 1;
						groupCand.push_back(pSecCand);
						++usefulCandBeg;//search next
					}
					else
					{
						nextGroupNum = pSecCand->getGroupNum();
						if (m_mGroupAndLength[nextGroupNum] <= 2)
							++usefulCandBeg;
						else
							break;
					}
				}
			}
		}
	}

	m_mGroupAndLength.clear();
	usefulCandBeg = useFullCand.begin();
	while (usefulCandBeg != useFullCand.end())
	{
		pFirCand = *usefulCandBeg;
		curGroupNum = pFirCand->getGroupNum();
		if (m_mGroupAndLength.find(curGroupNum) == m_mGroupAndLength.end())
		{
			m_mGroupAndLength[curGroupNum] = 1;
			pFirCand->setGroupOrder(1);
		}
		else
		{
			m_mGroupAndLength[curGroupNum] = m_mGroupAndLength[curGroupNum] + 1;
			pFirCand->setGroupOrder(m_mGroupAndLength[curGroupNum]);
		}
		++usefulCandBeg;
	}
	usefulCandBeg = useFullCand.begin();
	while (usefulCandBeg != useFullCand.end())
	{
		pFirCand = *usefulCandBeg;
		curGroupNum = pFirCand->getGroupNum();
		if (m_mGroupAndLength[curGroupNum] == 1)
			pFirCand->setGroupOrder(0);
		++usefulCandBeg;
	}
}

//
Void MovingObject::copyPriorMovObj(MovingObject *pDst)
{
	ObjectCandidate* head1 = this->getHeadPointer();
	ObjectCandidate* head2 = pDst->getHeadPointer();

	if (head1->getPicNum() < head2->getPicNum())
	{
		pDst->setHeadExtentionFlag(this->getHeadExtentionFlag());
		pDst->clearPriorMovObj();
	}
	else
	{
		this->clearPriorMovObj();
	}

	MovingObject* pTempMovObj;
	Bool			flag;
	auto beg = m_mPriorMovingObject.begin();
	while (beg != m_mPriorMovingObject.end())
	{
		pTempMovObj = beg->first;
		while (pTempMovObj->getMovObjectNum() == 0)
			pTempMovObj = pTempMovObj->getRelateMovObj();
		flag = beg->second;
		//if (pTempMovObj->getTailPointer()->getPicNum()<pDst->getHeadPointer()->getPicNum())
		if (pTempMovObj != pDst)
		pDst->addPriorMovObj(pTempMovObj, flag);
		beg++;
	}
	pDst->resetPriorAndSubObjs();

}
//
Void MovingObject::copySubSeqMovObj(MovingObject *pDst)
{
	ObjectCandidate* tail1 = this->getHeadPointer();
	ObjectCandidate* tail2 = pDst->getHeadPointer();

	if (tail1->getPicNum() > tail2->getPicNum())
	{
		pDst->setTailExtentionFlag(this->getTailExtentionFlag());
		pDst->clearPriorMovObj();
	}
	else
		this->clearPriorMovObj();


	MovingObject* pTempMovObj;
	Bool flag;
	auto beg = m_mSubSeqMovingObject.begin();
	while (beg != m_mSubSeqMovingObject.end())
	{
		pTempMovObj = beg->first;
		while (pTempMovObj->getMovObjectNum() == 0)
			pTempMovObj = pTempMovObj->getRelateMovObj();
		flag = beg->second;
		//if (pTempMovObj->getHeadPointer()->getPicNum()>pDst->getTailPointer()->getPicNum())
		if (pTempMovObj != pDst)
		pDst->addSubSeqMovObj(pTempMovObj, flag);
		beg++;
	}
	pDst->resetPriorAndSubObjs();
}
//
Void MovingObject::assignMovObjToPrior(MovingObject* pSrc)
{

	while (pSrc->getMovObjectNum() == 0)
		pSrc = pSrc->getRelateMovObj();
	auto beg = m_mPriorMovingObject.begin();

	Bool match = false;
	while (beg != m_mPriorMovingObject.end())
	{
		cout << "beggg" << endl;
		MovingObject *tempObj = beg->first;
		cout << "tempObj is " << tempObj << endl;
		if (tempObj == NULL)
			cout << "this is a NULL one" << endl;
		while (tempObj->getMovObjectNum() == 0)
		{
			cout << "tempObj middle is " << tempObj << endl;
			tempObj = tempObj->getRelateMovObj();
		}
		cout << "tempObj related is " << tempObj << endl;
		match = tempObj ->checkObjectsOverlapFrames(pSrc);
		//if (beg->first != pSrc && match)
		if (tempObj != pSrc && match)
		{
			tempObj->resetElement(pSrc);
			beg = m_mPriorMovingObject.begin();
			continue;
		}
		cout << "debug point w33" << endl;
		++beg;
	}
}
//
Void MovingObject::assignMovObjToSubseq(MovingObject* pSrc)
{
	
	while (pSrc->getMovObjectNum() == 0)
		pSrc = pSrc->getRelateMovObj();

	Bool match = false;
	auto beg = m_mSubSeqMovingObject.begin();
	while (beg != m_mSubSeqMovingObject.end())
	{
		MovingObject *tempObj = beg->first;
		while (tempObj->getMovObjectNum() == 0)
			tempObj = tempObj->getRelateMovObj();
		match = tempObj->checkObjectsOverlapFrames(pSrc);
		//if (beg->first != pSrc && match)
		if (tempObj != pSrc && match)
		{
			tempObj->resetElement(pSrc);
			beg = m_mPriorMovingObject.begin();
			continue;
		}
		cout << "Debug point 777" << endl;
		++beg;
	}
}

Void MovingObject::resetElement(MovingObject* pDstMovingObj)
{

	if (m_uiMovObjectNum == 0) //it is not reasonale to merge a number 0 object with another object
		return;

	Bool flag = false;
	flag = checkObjectsOverlapFrames(pDstMovingObj);
	if (flag == false)
		return;

	this->copyPriorMovObj(pDstMovingObj);

	this->copySubSeqMovObj(pDstMovingObj);

	pDstMovingObj->resetPriorAndSubObjs();

	PicData* curPic;
	UInt picNum, newMovNum;
	
	curPic = m_pcHeadPointer->getCurPic();
	//m_pcHeadPointer->getCurPic()->getCurVideoData()->getMovObjMap();
	
	
	//cout << "pDstMovingObj cand size is " << pDstMovingObj->getObjectCandidates().size() << endl;
	if (this != pDstMovingObj)
	while (curPic->getPicNum() <= m_pcTailPointer->getCurPic()->getPicNum())
	{
		
		//	cout << "this and that are the same" << endl;
		//cout << "pDstMovingObj cand size is " << pDstMovingObj->getObjectCandidates().size() << endl;
		picNum = curPic->getPicNum();
		curPic->resetMovingObj(this, pDstMovingObj);//move element from first to second 
		curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(picNum);
		if (curPic == NULL)
			break;
		
	}

	//if (m_pcHeadPointer->getCurPic()->getPicNum() < pDstMovingObj->getHeadPointer()->getCurPic()->getPicNum())
		//pDstMovingObj->setHeadPointer(m_pcHeadPointer);

	//cout << "After, pDstMovingObj cand size is " << pDstMovingObj->getObjectCandidates().size() << endl;
	pDstMovingObj->resetHeadPointer();
	//this->resetHeadPointer();
	
	pDstMovingObj->resetTailPointer();
	//this->resetTailPointer();
	
	pDstMovingObj->resetObjLength();
	this->resetObjLength();


	m_uiMovObjectNum = 0;//m_uiMovObjectNum is the moving object number 

	newMovNum =pDstMovingObj->getHeadPointer()->getCurPic()->getCurVideoData()->getInitialNum();
	pDstMovingObj->getHeadPointer()->getCurPic()->getCurVideoData()->increaseInitialNum();
	
	pDstMovingObj->setMovObjectNum(newMovNum);

	/*auto beg = m_mPriorMovingObject.begin();
	while (beg != m_mPriorMovingObject.end())
	{
		if (beg->first == pDstMovingObj)
		{
			beg = m_mPriorMovingObject.erase(beg);
			continue;
		}
		beg++;
	}
	auto beg = m_mSubSeqMovingObject.begin();
	while (beg != m_mSubSeqMovingObject.end())
	{
		if (beg->first == pDstMovingObj)
		{
			beg = m_mPriorMovingObject.erase(beg);
			continue;
		}
		beg++;
	}*/
	m_pcRelateMovObj = pDstMovingObj;
		
	

}

std::map<MovingObject*, Bool> MovingObject::getConnectedObjects(Bool headExtend)
{
	//this->resetPriorAndSubObjs();
	std::map<MovingObject*, Bool> objects;
	std::map<MovingObject*, Bool> tempObjects;
	if (headExtend == true)
		objects = this->getPriorMovObj();
	else
		objects = this->getSubSeqMovObj();


	UInt longSequence = 0;
	
	if (objects.size() == 1)
		return objects;
	else
	{
		auto beg = objects.begin();
		while (beg != objects.end())
		{
			if (beg->first->getSeqLength() > 5)
			{
				Bool objectFlag = beg->second;
				tempObjects[beg->first] = objectFlag;
				longSequence++;
			}
			beg++;
		}
	}


	if (longSequence > 1)
		return tempObjects;
	else
	{
		std::map<MovingObject*, Bool> results;
		return results;
	}





}

Void MovingObject::setUsefulFlag(Bool flag)
{
	PicData* curPic;
	UInt picNum;
	curPic = m_pcHeadPointer->getCurPic();
	
	while (curPic->getPicNum() <= m_pcTailPointer->getCurPic()->getPicNum())
	{
		picNum = curPic->getPicNum();
		curPic->resetUsefulFlag(this, flag);
		curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(picNum);
	}
}
//
Bool MovingObject::extendChainConstrained(Bool forwardFlag, Bool ShortStep, UInt extendTurn , Bool specialSearch)
{
	MovingObject* pMovingObj = NULL;
	ObjectCandidate *oldHeader, *oldTail;
	UInt curPicNum, totalPicNum;
	Int frameInterval = 0, frameStride = 0;
	Bool findFlag = false;
	totalPicNum = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPicNumInVideo();
	oldHeader = m_pcHeadPointer;
	oldTail = m_pcTailPointer;
	//search prior frames 
	if (forwardFlag && //m_pcHeadPointer->getPriorCand().size() == 0)//	
		m_mPriorMovingObject.size() == 0)
	{
		cout << "begin forward" << endl;
		//frameInterval = ShortStep ? 1 : FRAME_STRIDE;
		if (extendTurn == 3)
		{
			frameInterval = 2 * FRAME_STRIDE;
			frameStride = 6 * FRAME_STRIDE;
		}
		else
		{
			frameInterval = 1;
			frameStride = ShortStep ? FRAME_STRIDE : 2 * FRAME_STRIDE;
		}
		curPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
		/* for debug */
	
		m_bHeadExtended = false;

		while (((curPicNum - frameInterval - 1) > 0) && frameInterval <= frameStride)
		{
			m_pcHeadPointer->endExtention(0 - frameInterval - 1, findFlag, ShortStep, extendTurn, specialSearch);
			if (findFlag)
			{
				m_bHeadExtended = true;
				//if (extendTurn != 3)
				break;
			}
			++frameInterval;
			//curPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
		}

		cout << "end forward" << endl;
	}
	//search subsequent frames 
	else if (!forwardFlag && //m_pcTailPointer->getNextCand().size() == 0)
		m_mSubSeqMovingObject.size() == 0)
	{
		cout << "begin backward" << endl;
		//frameInterval = ShortStep ? 0 : FRAME_STRIDE;
		if (extendTurn == 3)
		{
			frameInterval = 2*FRAME_STRIDE;
			frameStride = 6 * FRAME_STRIDE;
		}
		else
		{
			frameInterval = 0;
			frameStride = ShortStep ? FRAME_STRIDE : 2 * FRAME_STRIDE;
		}
		curPicNum = m_pcTailPointer->getCurPic()->getPicNum();
	
		m_bTailExtended = false;
		while ((curPicNum + frameInterval) < totalPicNum && frameInterval <= frameStride)
		{
			m_pcTailPointer->endExtention(frameInterval, findFlag, ShortStep, extendTurn, specialSearch);
			
				if (findFlag)
				{
					m_bTailExtended = true;
					//if (extendTurn != 3)
					break;
				}
			
			
			++frameInterval;
			//curPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
		}
		cout << "end backward" << endl;
	}
	//the code below no use
	if (oldHeader != m_pcHeadPointer || oldTail != m_pcTailPointer)
	{
		cout << "here is new not equal old, exit " << endl; 
		exit(0);
		return true;
	}
	else
		return false;
}

//deal with condition that more than one object in prior frame correspond to current object
Void MovingObject::dealMergeConstrained(UInt Exturn)
{
	cout << "ffeefef" << endl;
	//if (m_pcHeadPointer->getPriorCand().size() > 1 && m_uiSeqLength > 3)
	if (m_pcHeadPointer == NULL)
		cout << "head is NULL " << endl;
	if (m_pcHeadPointer->getPriorCand().size() > 1)
	{
		Bool sameMovingObject;
		cout << "begin seada" << endl;
		sameMovingObject = m_pcHeadPointer->dealManyToOne();
		cout << "finsh wewqq" << endl;
		if (Exturn == 1 && m_pcHeadPointer->getPicNum()==196)
			{
				cout << "same is " << sameMovingObject << endl;
				cout << "MergeConstrained,(this is the question)the object's head pic is " << m_pcHeadPointer->getPicNum() << endl;
				cout << "the object's tail pic is " << m_pcTailPointer->getPicNum()<<endl;
				cout << "the object's priors are the following" << endl;
				cout << endl;
				std::map<ObjectCandidate*, Bool> priorCand = m_pcHeadPointer->getPriorCand();
				auto beg = priorCand.begin();
				while (beg != priorCand.end())
				{
					cout << "the pic num of prior is  " << beg->first->getPicNum()<<endl;
					cout << "the object num is  " << beg->first->getMovingOjbect()->getMovObjectNum() << endl;
					cout << "the object's head is " << beg->first->getMovingOjbect()->getHeadPointer()->getPicNum()<< endl;
					cout << "the object's tail is " << beg->first->getMovingOjbect()->getTailPointer()->getPicNum() << endl;
					cout << endl;
					beg++;
				}
				//int yu; cin >> yu;
				
			}
			if (sameMovingObject == true)
			{
				m_pcHeadPointer->assignMovObjToPriorCandidate();
				cout << "finish assingingggee" << endl;
			}
	}
	
	

}

//deal with the condition that one object corresponding to more than one object in next frame
//or many objects correspond to one object in next frame
Void MovingObject::dealSplitConstrained(UInt Exturn)
{
	cout << "beginning dealing splitting" << endl;
	//if (m_pcTailPointer->getNextCand().size() > 1 && m_uiSeqLength > 3)
	if (m_pcTailPointer->getNextCand().size() > 1)
	{
		Bool sameMovingObject;
		cout << "gfrthj" << endl;
		sameMovingObject = m_pcTailPointer->dealOneToMany();
		
		if (sameMovingObject == true)
		{
			//if (Exturn == 2)
			//{
			//	cout << "SplitConstrained,the object's head pic is " << m_pcHeadPointer->getPicNum() << endl;
			//	cout << "(this is the question) the object's tail pic is " << m_pcTailPointer->getPicNum() << endl;
			//	cout << "the object's nexts are the following" << endl;

			//	std::map<ObjectCandidate*, Bool> nextCand = m_pcTailPointer->getNextCand();
			//	auto beg = nextCand.begin();
			//	while (beg != nextCand.end())
			//	{
			//		cout << "the pic num of next is  " << beg->first->getPicNum() << endl;
			//		beg++;
			//	}
			//	//int yu; cin >> yu;
			//}
			cout << "hjkiu" << endl;
			m_pcTailPointer->assignMovObjToSubseqCandidate();
			cout << "zaqjp" << endl;
		}

	}
}



Bool MovingObject::ObjectDeletable(UInt extendTurn)
{
	UInt motionStride = m_pcTailPointer->getCurPic()->getPicNum() - m_pcHeadPointer->getCurPic()->getPicNum();
	if (extendTurn != 2)
	{
		return false;
	}
	else if (extendTurn == 2)
	{
		//return false;

		if (motionStride < 8 || m_uiSeqLength < 8)
		{
			return true;
		}
		else
			return false;
	}

	

	/*else
	{
		cout << "there is a bug when doing ObjectDelet,exit" << endl;
		exit(0);
		return 0;
	}*/
}

Void MovingObject::releaseChainOutOfData(UInt extendTurn)
{
	m_mPriorMovingObject.clear();
	m_mSubSeqMovingObject.clear();
	m_bHeadExtended = false;
	m_bTailExtended = false;

	//this->getHeadPointer();

}
//
Void MovingObject::releaseErrorConnection()
{
}


//
Void MovingObject::dealConstrainedExtention(Bool ShortStep, Bool head)
{
	
	if (head)
	{
		if (m_mPriorMovingObject.size() > 1)
		{
			m_bRealHead = false;
			//return;
		}
		//head extention 
		//else
		//{
			std::map<MovingObject*, Bool>	connectedMovingObject;

			connectedMovingObject = this->getConnectedObjects(true);

			//if (m_mPriorMovingObject.size() == 1 && m_pcHeadPointer->getPriorCand().size() <= 1)
			if (connectedMovingObject.size() != 0)
			{
				Bool stopFlag = false, constrainFlag = true;
				MovingObject* pTempMovObj = connectedMovingObject.begin()->first;
				Bool corFlag = connectedMovingObject.begin()->second;
				//if (ShortStep == false && m_mPriorMovingObject.size() == 1)
				//if (m_mPriorMovingObject.size() == 1)
				//constrainFlag = true;
				//else
				//constrainFlag = false;
				while (pTempMovObj->getSubSeqMovObj().size() <= 1 && constrainFlag && stopFlag == false && corFlag)
				//while (pTempMovObj->getSubSeqMovObj().size() <= 1 && constrainFlag && stopFlag == false)
				{
					/*UInt picNum = m_pcHeadPointer->getPicNum();
					UInt x, y = 0; m_pcHeadPointer->getObjCentroid(x, y, true);
					if (picNum == 358)
					{
					cout << "head extension, the pic is " << picNum << endl;
					cout << "the x,y is " << x << " " << y << endl;
					cout << " m_mPriorMovingObject.size() is " << m_mPriorMovingObject.size() << endl;
					cout << "m_pcHeadPointer->getPriorCand() is " << m_pcHeadPointer->getPriorCand().size() << endl;

					int rr; cin >> rr;
					}*/


					//if (pTempMovObj->getTailPointer()->getNextCand().size() > 1 && ShortStep)
					//	break;

					while (pTempMovObj->getMovObjectNum() == 0)
						//if (pTempMovObj->getMovObjectNum() == 0)
					{
						pTempMovObj = pTempMovObj->getRelateMovObj();
						std::cout << "this loop set a related object to a moving object " << endl;
					}
					std::cout << "finish setting" << endl;

					Bool matchFlag2 = checkObjectsOverlapFrames(pTempMovObj);
					if (matchFlag2 == false)
					{
						break;
					}

					

					pTempMovObj->setTailExtentionFlag(false);

					pTempMovObj->resetElement(this);
					
					//m_mPriorMovingObject.clear();

					//pTempMovObj->copyPriorMovObj(this);

					cout << "finish copyprior" << endl;

					m_pcHeadPointer->setHeadFlag(true);
					if (pTempMovObj->getHeadExtentionFlag())
					{
						m_bHeadExtended = true;

						connectedMovingObject = this->getConnectedObjects(true);
						if (connectedMovingObject.size() != 0)
						{
							pTempMovObj = connectedMovingObject.begin()->first;
							stopFlag = false;
							corFlag = connectedMovingObject.begin()->second;
						}
						else
							stopFlag = true;
					}
					else
					{
						stopFlag = true;//m_bHeadExtended = false;
					}
				}

			}
		//}
	}

	else
	{
		if (m_mSubSeqMovingObject.size() > 1)
		{
			m_bRealTail = false;
			//return;
		}
		//tail extention
		//else
		//{


			//if (m_mSubSeqMovingObject.size() == 1 && m_pcTailPointer->getNextCand().size() <= 1)

			std::map<MovingObject*, Bool>	connectedMovingObject;

			connectedMovingObject = this->getConnectedObjects(false);

			if (connectedMovingObject.size() == 1)
			{
				MovingObject* pTempMovObj = connectedMovingObject.begin()->first;
				Bool corFlag = connectedMovingObject.begin()->second;
				Bool stopFlag = false;

				while (pTempMovObj->getPriorMovObj().size() <= 1 && corFlag == true && !stopFlag)
				//while (pTempMovObj->getPriorMovObj().size() <= 1 && !stopFlag)
				{
					//if (pTempMovObj->getHeadPointer()->getPriorCand().size() > 1)
						//break;
					
					/*UInt picNum=m_pcTailPointer->getPicNum();
					UInt x, y = 0; m_pcTailPointer->getObjCentroid(x, y,true);
					if (picNum == 357)
					{
					cout << "tai extension, the pic is " << picNum << endl;
					cout << "the x,y is " << x << " " << y << endl;
					cout << "m_mSubSeqMovingObject.size() is " << m_mSubSeqMovingObject.size() << endl;
					cout << "m_pcTailPointer->getNextCand() is " << m_pcTailPointer->getNextCand().size() << endl;

					int rr; cin >> rr;
					}*/

					while (pTempMovObj->getMovObjectNum() == 0)
						//if (pTempMovObj->getMovObjectNum() == 0)
					{
						pTempMovObj = pTempMovObj->getRelateMovObj();
						cout << "this loop set a related object to a moving object " << endl;
					}
					cout << "finish setting" << endl;

					Bool matchFlag2 = checkObjectsOverlapFrames(pTempMovObj);
					if (matchFlag2 == false)
					{
						//cout << "matching2 false,ready to break" << endl;
						/*int gg; cin >> gg;*/
						break;
					}
					//cout << "matching2 true2" << endl;
					/*cout << "matchFlag3333" << endl;
					int rr; cin >> rr;*/

					cout << "finsh checking, begin setting" << endl;
					pTempMovObj->setHeadExtentionFlag(false);
					
					pTempMovObj->resetElement(this);
					cout << "finfff ffffff" << endl;
					//m_mSubSeqMovingObject.clear();
					
					//pTempMovObj->copySubSeqMovObj(this);

					cout << "finish copySubsequence" << endl;

					if (pTempMovObj->getTailExtentionFlag())
					{
						m_bTailExtended = true;

						connectedMovingObject = this->getConnectedObjects(false);

						if (connectedMovingObject.size() != 0)
						{
							pTempMovObj = connectedMovingObject.begin()->first;
							corFlag = connectedMovingObject.begin()->second;
							stopFlag = false;
						}
						else
							stopFlag = true;
					}
					else
						stopFlag = true;
				}
			}

	}
}



Bool	MovingObject::splitOneObjectIntoTwo(MovingObject* subSeqObj1, MovingObject*subSeqObj2, MovingObject* &priorObj2, UInt& splitBegPic, UInt& splitEndPic)
{
	MovingObject* priorObj1 = this;
	Bool splitFinish = true;
	if (m_uiMovObjectNum == 0) //it is not reasonale to split a number 0 object
		return false;

	std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> subSeqObj1CtuVec;
	std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> subSeqObj2CtuVec;

	subSeqObj1CtuVec = subSeqObj1->getObjCtus(true);
	subSeqObj2CtuVec = subSeqObj2->getObjCtus(true);
	
	UInt nearestPriorSplitPicNum = 0;
	Bool priorObj2Exist = false;
	ObjectCandidate* nearestSplitCand = NULL;
	MovingObject* neighMovingObj = NULL;
	priorObj2 = NULL;
	Bool findPriorSplit = false;

	nearestSplitCand = findNearestPriorSplitCand(neighMovingObj);
	if (nearestSplitCand == NULL)
		findPriorSplit;
	else
		findPriorSplit = true;

	if (findPriorSplit == true)
	{
		nearestPriorSplitPicNum = nearestSplitCand->getPicNum();
	}
	else
		nearestPriorSplitPicNum = 0;

	cout << "finish find Nearest Prior Split Candidate " << endl;
	
	PicData* curPic = NULL;
	UInt picNum = 0;

	cout << " nearestPriorSplitPicNum is " << nearestPriorSplitPicNum << endl;
	
	//Cand should be the head pointer of the subsequent two objects
	if (nearestPriorSplitPicNum == 0)
	{
		splitFinish = true;
	}
	else
	{
		splitFinish = true;
		splitBegPic = nearestPriorSplitPicNum;

		if (neighMovingObj!=NULL)
			priorObj2 = neighMovingObj;
	}

	Bool needToSplit = false;
	if (nearestPriorSplitPicNum != 0)
		needToSplit = true;

	Bool match1 = false, match2 = false;
	Float ratio1 = 0, ratio2 = 0;
	
	if (needToSplit)
	{
		curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(nearestPriorSplitPicNum - 1);
		splitEndPic = m_pcTailPointer->getCurPic()->getPicNum();

		match1 = this->getTailPointer()->calculateBoundingBoxOverlaps(subSeqObj1->getHeadPointer(), ratio1, ratio2);
		match2 = this->getTailPointer()->calculateBoundingBoxOverlaps(subSeqObj2->getHeadPointer(), ratio1, ratio2);

		cout << "first, match1 match2 are " << match1 << "  " << match2 << endl;
		if (match1&&match2)
		{
			while (curPic->getPicNum() <= m_pcTailPointer->getCurPic()->getPicNum())
			{
				picNum = curPic->getPicNum();
				curPic->splitMovingObj(this, subSeqObj1, subSeqObj2);//split the crrent object into one and two 

				curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(picNum);
				if (curPic == NULL)
					break;
			}

			std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> priorObj1CtuVec=this->getObjCtus(false);
			std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> priorObj2CtuVec;
			if (priorObj2!=NULL)
			priorObj2CtuVec = priorObj2->getObjCtus(false);
			
			Bool ACisSame = false;
			std::vector<Float> A_velocity, B_velocity, C_velocity, D_velocity;

			/*Bool equal =( priorObj2 == this);
			cout << "priorObj2==this is " << equal << endl;*/
			if (priorObj2 != NULL)
			{
				ACisSame = compareObjects(priorObj1CtuVec, A_velocity, priorObj2CtuVec, B_velocity, subSeqObj1CtuVec, C_velocity, subSeqObj2CtuVec, D_velocity);

				if (ACisSame)
				{
					priorObj1->resetElement(subSeqObj1);
					priorObj2->resetElement(subSeqObj2);
				}
				else
				{
					priorObj1->resetElement(subSeqObj2);
					priorObj2->resetElement(subSeqObj1);
				}
			}
			else
			{
				ACisSame = compareObjects3objects(priorObj1CtuVec, A_velocity, subSeqObj1CtuVec, C_velocity, subSeqObj2CtuVec, D_velocity);

				if (ACisSame)
				{
					priorObj1->resetElement(subSeqObj1);
					
				}
				else
				{
					priorObj1->resetElement(subSeqObj2);
				}
			

			}

			
		}
		else if (match1)
			this->resetElement(subSeqObj1);
		else if (match2)
			this->resetElement(subSeqObj2);
	}
	else
	{
		match1 = this->getTailPointer()->calculateBoundingBoxOverlaps(subSeqObj1->getHeadPointer(), ratio1, ratio2);
		match2 = this->getTailPointer()->calculateBoundingBoxOverlaps(subSeqObj2->getHeadPointer(), ratio1, ratio2);

		if (match1&&match2)
		{
			ObjectCandidate * pObj1 = subSeqObj1->getHeadPointer();
			ObjectCandidate * pObj2 = subSeqObj2->getHeadPointer();
			if (pObj1->getCtuElements().size()>pObj2->getCtuElements().size())
				this->resetElement(subSeqObj1);
			else
				this->resetElement(subSeqObj2);
		}
		else if (match1)
			this->resetElement(subSeqObj1);
		else if (match2)
			this->resetElement(subSeqObj2);
	}

	//reset the head and the tail pointer
	this->resetObjLength();
	UInt seqLength = this->getSeqLength();
	if (seqLength != 0)
	{
		this->resetHeadPointer();
		this->resetTailPointer();	
	}
	else
	{
		m_uiMovObjectNum = 0;
		setRelateMovObj(subSeqObj1);
	}

	subSeqObj1->resetHeadPointer();
	subSeqObj1->resetTailPointer();
	subSeqObj2->resetHeadPointer();
	subSeqObj2->resetTailPointer();

	return splitFinish;
}


Bool	MovingObject::mergeTwobjectsIntoOne(MovingObject*pSubLargeObject, UInt& mergeBegPic, UInt& mergeEndPic)
{
	this->resetTailPointer();
	m_pcTailPointer = this->getTailPointer();

	UInt picNum = m_pcTailPointer->getCurPic()->getPicNum();
	PicData* curPic = m_pcTailPointer->getCurPic()->getCurVideoData()->getPic(picNum);
	ObjectCandidate* pLargeCand=pSubLargeObject->findCandBeforeOrAfterPicNum(picNum,true);

	if (pLargeCand == NULL)
		return false;
	UInt leftEdge_Large = 0, upEdge_Large = 0, rightEdge_Large = 0, botEdge_Large = 0;
	pLargeCand->getObjEdge(leftEdge_Large, upEdge_Large, rightEdge_Large, botEdge_Large,true);

	UInt pubArea=m_pcTailPointer->findPubCtu(pLargeCand);
	cout << "pubCtu size is " << pubArea << endl;
	mergeBegPic = 0, mergeEndPic = 0;
	Bool split = false;
	cout << " m_pcTailPointer pic num is " << m_pcTailPointer->getPicNum() << "  " << m_pcTailPointer->getCtuElements().size() << endl;

	UInt newPicNum = 1;
	while (curPic->getPicNum() <= pSubLargeObject->getTailPointer()->getPicNum())
	{
		cout << "hello splitting" << endl;
		picNum = curPic->getPicNum();
		//split=curPic->splitObj1AccordingToCand(pSubLargeObject, this, m_pcTailPointer);
		split = curPic->splitObj1AccordingToBoundingBox(pSubLargeObject, this, m_pcTailPointer,leftEdge_Large,  upEdge_Large,  rightEdge_Large,  botEdge_Large);
		if (newPicNum > 30)
			break;
		
		if (split == true)
		{
			cout << "tail, split is true" << endl;
			//int rr; cin >> rr;
			newPicNum++;
			if (mergeBegPic == 0)
				mergeBegPic = picNum;
			mergeEndPic = picNum;
		}
		
		curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(picNum);
		if (curPic == NULL)
			break;
	}
	int ru; cin >> ru;
	this->resetHeadPointer();
	this->resetTailPointer();
	pSubLargeObject->resetHeadPointer();
	pSubLargeObject->resetTailPointer();
	return true;

}


Bool MovingObject::compareObjects(MovingObject* objectA, std::vector<Float> A_velocity, MovingObject* objectB,  std::vector<Float> B_velocity,
	MovingObject* objectC, std::vector<Float> C_velocity, MovingObject* objectD, std::vector<Float> D_velocity, MovingObject* objectE )
{

	//cout << "debug point1" << endl;
	//UInt sizeE = objectE.size();
	Bool AisEmpty = false, BisEmpty = false, CisEmpty = false, DisEmpty = false;
	if (objectA == NULL || objectB == NULL)
	{
		if (objectA == NULL && objectB != NULL)
		{
			objectA = objectB;
			AisEmpty = true;
		}
		else
		if (objectB == NULL && objectA != NULL)
		{
			objectB = objectA;
			BisEmpty = true;
		}
		else
		{
			cout << "it is not right for objectA and objectB be NULL at the same time, exit" << endl;
			exit(0);
			return false;
		}
	}

	//cout << "debug point2" << endl;

	if (objectC == NULL || objectD == NULL)
	{
		if (objectC == NULL && objectD != NULL)
		{
			objectC = objectD;
			CisEmpty = true;
		}
		else
		if (objectD == NULL && objectC != NULL)
		{
			objectD = objectC;
			DisEmpty = true;
		}
		else
		//if (objectC == NULL && objectD == NULL)
		{
			cout << "it is not right for objectC and objectD be NULL at the same time, exit" << endl;
			exit(0);
			return false;
		}
	}

	cout << "AisEmpty  " << AisEmpty << "  BisEmpty " << BisEmpty << "  CisEmpty " << CisEmpty << " DisEmpty " << DisEmpty << endl;
	cout << "A, B, C, D->getMovObjectNum() is " << objectA->getMovObjectNum() << "  " << objectB->getMovObjectNum() << "  " << objectC->getMovObjectNum()
		<<" "<< objectD->getMovObjectNum() << endl;
	cout << "A, B, C, D candidate size is " << objectA->getObjectCandidates().size() << "  " << objectB->getObjectCandidates().size() << "  " << objectC->getObjectCandidates().size() << "  "
		<< objectD->getObjectCandidates().size() << endl;
	if (objectA->getMovObjectNum() == 0 || objectB->getMovObjectNum() == 0 || objectC->getMovObjectNum() == 0 || objectD->getMovObjectNum() == 0)
	{
		cout << "the object num is zero, exit" << endl;
		exit(0);
	}
	
	//cout << "the head of A is " << objectA->getHeadPointer()->getPicNum() << endl;
	//cout << "the tail of A is " << objectA->getTailPointer()->getPicNum() << endl;

	

	std::vector<ObjectCandidate*> candA;
	std::vector<ObjectCandidate*> candB;
	std::vector<ObjectCandidate*> candC;
	std::vector<ObjectCandidate*> candD;
	std::vector<ObjectCandidate*> tempCand;

	std::vector<UInt> areaA;
	std::vector<UInt> areaB;
	std::vector<UInt> areaC;
	std::vector<UInt> areaD;
	std::vector<UInt> tempArea;

	ObjectCandidate* headPoint = NULL;
	ObjectCandidate* tailPoint = NULL;
	//get candA,B,C,D from the moving objects
	

	UInt currPicNum = 0;
	UInt headPicNum = 0;
	UInt tailPicNum = 0;
	VideoData* pCurVideo = this->getHeadPointer()->getCurPic()->getCurVideoData();
	PicData* pCurPic=NULL;
	MovingObject* tempObject = NULL;


	//the following is for debug
	//****************************************************************************
	//pCurPic = pCurVideo->getPic(objectA->getTailPointer()->getPicNum() - 1);
	//UInt totalArea = 0;

	////ObjectCandidate* pCurObjCand;
	//std::vector<ObjectCandidate*> objects(pCurPic->getObjects());
	//auto beg = objects.begin();
	//while (beg != objects.end())
	//{
	//	if ((*beg)->getMovingOjbect()->getMovObjectNum() == objectA->getMovObjectNum())
	//	{
	//		tempCand.push_back(*beg);
	//		totalArea += (*beg)->getCtuElements().size();
	//	}
	//	++beg;
	//}
	////cout << "AisEmpty  " << AisEmpty << "  BisEmpty " << BisEmpty << "  CisEmpty " << CisEmpty << " DisEmpty " << DisEmpty << endl;
	////cout << "the totalArea of A is " << totalArea << endl;
	//************************************************************************************

	UInt maxPicNum = 5;

	UInt turn = 1;
	UInt i = 1;
	while (turn <= 4)
	{
		if (turn == 1)
		{
			tempObject = objectA;
		}
		if (turn == 2)
		{
			tempObject = objectB;
		}
		if (turn == 3)
		{
			tempObject = objectC;
		}

		if (turn == 4)
		{
			tempObject = objectD;
		}

		headPoint = tempObject->getHeadPointer();
		tailPoint = tempObject->getTailPointer();

		headPicNum = headPoint->getCurPic()->getPicNum();
		tailPicNum = tailPoint->getCurPic()->getPicNum();

		UInt totalArea = 0;
		tempCand.clear();

		UInt endPicNum = 0;
		if (turn <= 2)
		{
			currPicNum = tailPicNum;
			i = 1;
			while (currPicNum >= headPicNum)
			{
				// it is only useful the most recently candidates. Thus, 5 was set here.
				//The other candidates are not useful
				if (i > maxPicNum)
					break;

				pCurPic = pCurVideo->getPic(currPicNum - 1);
				totalArea = 0;

				//ObjectCandidate* pCurObjCand;
				auto beg = pCurPic->getObjects().begin();
				while (beg != pCurPic->getObjects().end())
				{
					if ((*beg)->getMovingOjbect()->getMovObjectNum() == tempObject->getMovObjectNum())
					{
						tempCand.push_back(*beg);
						totalArea += (*beg)->getCtuElements().size();
					}
					++beg;
				}

				/*cout << "turn is " << turn << endl;
				cout << "the pic is " << currPicNum << endl;
				cout << "the total Area is " << totalArea << endl;*/
				tempArea.push_back(totalArea);

				++i;
				--currPicNum;
			}
		}
		else
		{

			currPicNum = headPicNum;
			i = 1;
			while (currPicNum <= tailPicNum)
			{
				// it is only useful the most recently candidates. Thus, 5 was set here.
				//The other candidates are not useful
				if (i > maxPicNum)
					break;

				pCurPic = pCurVideo->getPic(currPicNum - 1);
				totalArea = 0;

				//ObjectCandidate* pCurObjCand;
				auto beg = pCurPic->getObjects().begin();
				while (beg != pCurPic->getObjects().end())
				{
					if ((*beg)->getMovingOjbect()->getMovObjectNum() == tempObject->getMovObjectNum())
					{
						tempCand.push_back(*beg);
						//cout << "iin  turn is " << turn<<endl;
						//cout << "iin the pic is " << (*beg)->getPicNum() << endl;
						totalArea += (*beg)->getCtuElements().size();
					}
					++beg;
				}
				/*cout << "turn is " << turn << endl;
				cout << "the pic is " << currPicNum << endl;
				cout << "the total Area is " << totalArea << endl;*/
				tempArea.push_back(totalArea);

				++i;
				++currPicNum;
			}
		}

		if (turn == 1)
		{
			areaA.swap(tempArea);
			candA.swap(tempCand);
		}
		if (turn == 2)
		{
			areaB.swap(tempArea);
			candB.swap(tempCand);
		}
		if (turn == 3)
		{
			areaC.swap(tempArea);
			candC.swap(tempCand);
		}

		if (turn == 4)
		{
			areaD.swap(tempArea);
			candD.swap(tempCand);
		}
		

		turn++;
	}

	//cout << "debug point44" << endl;
	Float averageCtuSizeA = 0, averageCtuSizeB = 0, averageCtuSizeC = 0, averageCtuSizeD = 0;

	Float tempaverageCtuSize = 0;
	 turn = 1;
	while (turn <= 4)
	{
		if (turn == 1)
		{
			tempArea.clear();
			tempArea.assign(areaA.begin(), areaA.end());
		}
		else
		if (turn == 2)
		{
			tempArea.clear();
			tempArea.assign(areaB.begin(), areaB.end());
		}
		else if (turn == 3)
		{
			tempArea.clear();
			tempArea.assign(areaC.begin(), areaC.end());
		}
		else if (turn == 4)
		{
			tempArea.clear();
			tempArea.assign(areaD.begin(), areaD.end());
		}
		else
		{
			cout << "there is a bug in compareObjects, exit." << endl;
			exit(0);
		}
		tempaverageCtuSize = 0;
		auto beg = tempArea.begin();
		while (beg != tempArea.end())
		{
			tempaverageCtuSize += (*beg);
			beg++;
		}
		tempaverageCtuSize = Float(tempaverageCtuSize) / Float(tempArea.size());

		if (turn == 1)
			averageCtuSizeA = tempaverageCtuSize;
		if (turn == 2)
			averageCtuSizeB = tempaverageCtuSize;
		if (turn == 3)
			averageCtuSizeC = tempaverageCtuSize;
		if (turn == 4)
			averageCtuSizeD = tempaverageCtuSize;

		//cout << "turn is "<<turn << endl;
		//cout << "tempaverageCtuSize is  " << tempaverageCtuSize << endl;
		turn++;
	}

	//cout << "debug point55" << endl;
	Float absDeltaAC = abs(averageCtuSizeA - averageCtuSizeC);
	Float absDeltaAD = abs(averageCtuSizeA - averageCtuSizeD);
	Float minDeltaA = absDeltaAC <= absDeltaAD ? absDeltaAC : absDeltaAD;

	Float absDeltaBC = abs(averageCtuSizeB - averageCtuSizeC);
	Float absDeltaBD = abs(averageCtuSizeB - averageCtuSizeD);
	Float minDeltaB = absDeltaBC <= absDeltaBD ? absDeltaBC : absDeltaBD;

	if (AisEmpty)
	{
		if (minDeltaB == absDeltaBD)
			return true;
		else
			return false;
	}

	if (BisEmpty)
	{
		if (minDeltaA == absDeltaAC)
			return true;
		else
			return false;
	}

	if (CisEmpty)
	{
		if (absDeltaBD <= absDeltaAD)
			return true;
		else
			return false;
	}

	if (DisEmpty)
	{
		if (absDeltaAC <= absDeltaBC)
			return true;
		else
			return false;
	}


	//int df; cin >> df;
	if ((minDeltaA == absDeltaAC && minDeltaB == absDeltaBD) || (minDeltaA == absDeltaAD && minDeltaB == absDeltaBC))
	{
		if (minDeltaA == absDeltaAC && minDeltaB == absDeltaBD)
			return true;
		else
			return false;

	}
	else
		return true;

}



Bool MovingObject::compareObjects(std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> priorObj1, std::vector<Float> A_velocity, std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> priorObj2, std::vector<Float> B_velocity,
	std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> subSeqObj1, std::vector<Float> C_velocity, std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>>subSeqObj2, std::vector<Float> D_velocity)
{

	cout << "A, B, C, D size is " << priorObj1.size() << "  " << priorObj2.size() << "  " << subSeqObj1.size()
		<< " " << subSeqObj2.size() << endl;
	
	if (priorObj1.size() == 0 || priorObj2.size() == 0 || subSeqObj1.size() == 0 || subSeqObj2.size() == 0)
	{
		cout << "the object candidate size is zero, exit" << endl;
		exit(0);
	}

	UInt totalCtuSizeA = 0, totalCtuSizeB = 0, totalCtuSizeC = 0, totalCtuSizeD = 0;

	auto beg = priorObj1.begin();
	while (beg != priorObj1.end())
	{
		totalCtuSizeA = totalCtuSizeA + (*beg).size();
		beg++;
	}

	beg = priorObj2.begin();
	while (beg != priorObj2.end())
	{
		totalCtuSizeB = totalCtuSizeB + (*beg).size();
		beg++;
	}

	beg = subSeqObj1.begin();
	while (beg != subSeqObj1.end())
	{
		totalCtuSizeC = totalCtuSizeC + (*beg).size();
		beg++;
	}

	beg = subSeqObj2.begin();
	while (beg != subSeqObj2.end())
	{
		totalCtuSizeD = totalCtuSizeD + (*beg).size();
		beg++;
	}

	

	//cout << "debug point44" << endl;
	Float averageCtuSizeA = 0, averageCtuSizeB = 0, averageCtuSizeC = 0, averageCtuSizeD = 0;

	averageCtuSizeA = Float(totalCtuSizeA) / Float(priorObj1.size());
	averageCtuSizeB = Float(totalCtuSizeB) / Float(priorObj2.size());
	averageCtuSizeC = Float(totalCtuSizeC) / Float(subSeqObj1.size());
	averageCtuSizeD = Float(totalCtuSizeD) / Float(subSeqObj2.size());

	cout << "total size is " << totalCtuSizeA << "  " << totalCtuSizeB << "  " << totalCtuSizeC << "  " << totalCtuSizeD << endl;
	cout << "average size is " << averageCtuSizeA << "  " << averageCtuSizeB << "  " << averageCtuSizeC << "  " << averageCtuSizeD << endl;
	//int rt; cin >> rt;

	
	//Float absDeltaAC = abs(averageCtuSizeA - averageCtuSizeC);
	//Float absDeltaAD = abs(averageCtuSizeA - averageCtuSizeD);
	//Float minDeltaA = absDeltaAC <= absDeltaAD ? absDeltaAC : absDeltaAD;

	//Float absDeltaBC = abs(averageCtuSizeB - averageCtuSizeC);
	//Float absDeltaBD = abs(averageCtuSizeB - averageCtuSizeD);
	//Float minDeltaB = absDeltaBC <= absDeltaBD ? absDeltaBC : absDeltaBD;

	if (averageCtuSizeA > averageCtuSizeB)
	{
		if (averageCtuSizeC > averageCtuSizeD)
			return true;
		else
			return false;
	}
	else
	{
		if (averageCtuSizeC <= averageCtuSizeD)
			return true;
		else
			return false;
	}

}

Bool MovingObject::compareObjects3objects(std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> priorObj1, std::vector<Float> A_velocity, std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> subSeqObj1, std::vector<Float> C_velocity, std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>>subSeqObj2, std::vector<Float> D_velocity)
{
	cout << "A, C, D size is " << priorObj1.size() <<  "  " << subSeqObj1.size()
		<< " " << subSeqObj2.size() << endl;

	if (priorObj1.size() == 0 || subSeqObj1.size() == 0 || subSeqObj2.size() == 0)
	{
		cout << "the object candidate size is zero, exit" << endl;
		exit(0);
	}

	UInt totalCtuSizeA = 0, totalCtuSizeC = 0, totalCtuSizeD = 0;

	auto beg = priorObj1.begin();
	while (beg != priorObj1.end())
	{
		totalCtuSizeA = totalCtuSizeA + (*beg).size();
		beg++;
	}



	beg = subSeqObj1.begin();
	while (beg != subSeqObj1.end())
	{
		totalCtuSizeC = totalCtuSizeC + (*beg).size();
		beg++;
	}

	beg = subSeqObj2.begin();
	while (beg != subSeqObj2.end())
	{
		totalCtuSizeD = totalCtuSizeD + (*beg).size();
		beg++;
	}



	//cout << "debug point44" << endl;
	Float averageCtuSizeA = 0, averageCtuSizeC = 0, averageCtuSizeD = 0;

	averageCtuSizeA = Float(totalCtuSizeA) / Float(priorObj1.size());
	averageCtuSizeC = Float(totalCtuSizeC) / Float(subSeqObj1.size());
	averageCtuSizeD = Float(totalCtuSizeD) / Float(subSeqObj2.size());

	cout << "total size is " << totalCtuSizeA <<  "  " << totalCtuSizeC << "  " << totalCtuSizeD << endl;
	cout << "average size is " << averageCtuSizeA << "  " << averageCtuSizeC << "  " << averageCtuSizeD << endl;

	Float absAC = abs(averageCtuSizeA - averageCtuSizeC);
	Float absAD = abs(averageCtuSizeA - averageCtuSizeD);
	
	if (absAC < absAD)
		return true;
	else
		return false;


}


Void MovingObject::addCandidate(UInt picNum, ObjectCandidate* pObject)
{

	pair<multimap<UInt, ObjectCandidate*>::iterator, multimap<UInt, ObjectCandidate*>::iterator> position;
	position = objectCandidates.equal_range(picNum);
	while (position.first != position.second)
	{
		if (position.first->second == pObject)
			return;

		position.first++;
	}

	objectCandidates.insert(make_pair(picNum, pObject));
}



Void MovingObject::eraseCandidate(ObjectCandidate* candidate)	
{
	auto beg = objectCandidates.begin();

	std::multimap<UInt, ObjectCandidate*> emptyCandMap;
	while (beg != objectCandidates.end())
	{
		if (beg->second == candidate)
		{
			objectCandidates.erase(beg);
			emptyCandMap.swap(objectCandidates);
			objectCandidates.swap(emptyCandMap);
			break;
		}
		beg++;
	}
	//objectCandidates.erase(candidate);
}

Void MovingObject::resetHeadPointer()
{
	if (objectCandidates.size() == 0)
	{
		//m_pcHeadPointer = NULL;
		//return;
		cout << "the object's candidates size is 0, set headPointer failed, exit " << endl;
		exit(0);
	}
	auto beg = objectCandidates.begin();
	if (beg->second == NULL)
	{
		cout << "when reseting head, there is a null, exit(0)" << endl;
		exit(0);
	}
	
	m_pcHeadPointer=
		beg->second;
}

Void MovingObject::resetTailPointer()
{

	if (objectCandidates.size() == 0)
	{
		//m_pcTailPointer = NULL;
		//return;
		cout << "the object's candidates size is 0, set tailPointer failed, exit " << endl;
		exit(0);
	}
	auto beg = objectCandidates.end();
	
	beg--;
	m_pcTailPointer = beg->second;
}

Void MovingObject::resetObjLength()
{
	std::map<UInt, Bool> length;
	
	auto beg = objectCandidates.begin();
	while (beg != objectCandidates.end())
	{
		
		length.insert(make_pair(beg->first,true));
		beg++;
	}
	
	m_uiSeqLength = length.size();


}


ObjectCandidate* MovingObject::findCandBeforeOrAfterPicNum(UInt num, Bool forward)
{
	ObjectCandidate* answer = NULL;

	if (forward == true)
	{
		auto beg = objectCandidates.lower_bound(num);

		if (beg == objectCandidates.end())
			return NULL;
		while (beg->first <= num)
		{
			beg++;
			if (beg == objectCandidates.end())
				return NULL;
		}

		answer = beg->second;
		return answer;
	}

	else
	{
		auto beg = objectCandidates.find(num);

		if (beg == objectCandidates.end())
			return NULL;
		while (beg->first >= num)
		{
			if (beg == objectCandidates.begin())
				return NULL;
			beg--;

		}

		answer = beg->second;
		return answer;
	}

}


ObjectCandidate* MovingObject::findCandAtPicNum(UInt num)
{

	ObjectCandidate* answer = NULL;

		auto beg = objectCandidates.find(num);

		if (beg == objectCandidates.end())
			return NULL;
		else
		{
			answer = beg->second;
			return answer;
		}

}

Void MovingObject::resetPriorAndSubObjs()
{
	////cout << "begin resetinggg" << endl;
	//this->resetHeadPointer();
	//this->resetTailPointer();
	//ObjectCandidate *head = this->getHeadPointer();
	//ObjectCandidate *tail = this->getTailPointer();

	//MovingObject* tempMovObj = NULL;
	//auto beg = m_mPriorMovingObject.begin();
	//Bool flag = false;
	//while (beg != m_mPriorMovingObject.end())
	//{
	//	if (beg->first->getMovObjectNum() == 0)
	//	{
	//		tempMovObj = beg->first;
	//		
	//		while (tempMovObj->getMovObjectNum() == 0)
	//		{
	//			//if (tempMovObj->getRelateMovObj() == NULL)
	//				//cout << "there is a NULL" << endl;
	//			tempMovObj = tempMovObj->getRelateMovObj();
	//		}
	//		flag = beg->second;
	//		beg = m_mPriorMovingObject.erase(beg);
	//		m_mPriorMovingObject.insert(make_pair(tempMovObj, flag));
	//		beg = m_mPriorMovingObject.begin();
	//		continue;
	//	}
	//	beg++;
	//}
	////cout << "second setting" << endl;

	//beg = m_mSubSeqMovingObject.begin();
	//while (beg != m_mSubSeqMovingObject.end())
	//{
	//	if (beg->first->getMovObjectNum() == 0)
	//	{
	//		tempMovObj = beg->first;
	//		while (tempMovObj->getMovObjectNum() == 0)
	//			tempMovObj = tempMovObj->getRelateMovObj();
	//		flag = beg->second;
	//		beg = m_mSubSeqMovingObject.erase(beg);
	//		m_mSubSeqMovingObject.insert(make_pair(tempMovObj, flag));
	//		beg = m_mSubSeqMovingObject.begin();
	//		continue;
	//	}
	//	beg++;
	//}

	/*beg = m_mPriorMovingObject.begin();
	while (beg != m_mPriorMovingObject.end())
	{
		if (beg->first->getTailPointer()->getPicNum() >= head->getPicNum())
		{
			beg = m_mPriorMovingObject.erase(beg);
			continue;
		}
		beg++;
	}

	beg = m_mSubSeqMovingObject.begin();
	while (beg != m_mSubSeqMovingObject.end())
	{
		if (beg->first->getHeadPointer()->getPicNum() <= tail->getPicNum())
		{
			beg = m_mSubSeqMovingObject.erase(beg);
			continue;
		}
		beg++;
	}*/

	//cout << "Finisheeeee" << endl;
}

std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> MovingObject::getObjCtus(Bool head)
{

	std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> tempCtuVec;
	std::multimap<UInt, ObjectCandidate*> tempCands = getObjectCandidates();
	std::map<CtuData*, Bool, ctuAddrIncCompare> tempCtu;
	ObjectCandidate* pTempCand = NULL;
	UInt nextPicNum = 0;
	UInt currPicNum = 0;

	//head means get the 5 frames from head
	if (head)
	{

		Bool nextPicIsNew = false;
		auto begCand = tempCands.begin();
		auto begCand2 = begCand;
		UInt totalPicNum = 1;
		while (begCand != tempCands.end())
		{
			if (totalPicNum > 5)
				break;
			pTempCand = begCand->second;

			std::map<CtuData*, Bool, ctuAddrIncCompare> tempCtu2 = pTempCand->getCtuElements();
			auto beg2 = tempCtu2.begin();
			while (beg2 != tempCtu2.end())
			{
				tempCtu.insert(*beg2);
				beg2++;
			}

			currPicNum = begCand->second->getPicNum();
			begCand2 = begCand;
			begCand2++;
			if (begCand2 != tempCands.end())
			{
				nextPicNum = begCand2->second->getPicNum();
				if (currPicNum == nextPicNum)
					nextPicIsNew = false;
				else
					nextPicIsNew = true;
			}
			else
			{
				nextPicIsNew = true;
			}

			if (nextPicIsNew == true)
			{
				totalPicNum++;
				tempCtuVec.push_back(tempCtu);
				tempCtu.clear();
			}

			begCand++;
		}
	}
	else
	{
		
			Bool nextPicIsNew = false;
			auto begCand = tempCands.end();
			begCand--;
			auto begCand2 = begCand;
			UInt totalPicNum = 1;
			while (begCand != tempCands.begin())
			{
				if (totalPicNum > 5)
					break;
				pTempCand = begCand->second;

				std::map<CtuData*, Bool, ctuAddrIncCompare> tempCtu2 = pTempCand->getCtuElements();
				auto beg2 = tempCtu2.begin();
				while (beg2 != tempCtu2.end())
				{
					tempCtu.insert(*beg2);
					beg2++;
				}

				currPicNum = begCand->second->getPicNum();
				begCand2 = begCand;
				begCand2--;
				if (begCand != tempCands.begin())
				{
					nextPicNum = begCand2->second->getPicNum();
					if (currPicNum == nextPicNum)
						nextPicIsNew = false;
					else
						nextPicIsNew = true;
				}
				else
				{
					nextPicIsNew = true;
				}

				if (nextPicIsNew == true)
				{
					totalPicNum++;
					tempCtuVec.push_back(tempCtu);
					tempCtu.clear();
				}

				begCand--;
			}
	}

	return  tempCtuVec;

}



Bool MovingObject::objVelocityCompare(MovingObject *  pObj, Bool thisTailToPObjHead)
{
	if (thisTailToPObjHead)
	{
		this->resetObjLength();
		//this->


	}
	return true;

}

//
Void MovingObject::extendUnconstrianed()
{
	////////////////////////////////////
}

Void MovingObject::detectMovObjeTrajectory()
{

	string dirName = "./traDatas/";
	

	int aa = _mkdir(dirName.c_str());

	
	UInt ObjectNum = this->getMovObjectNum();
	//cout << "detecting the trajectory of obect  " << ObjectNum << endl;
	///////////////////////////////////////////////////////////////////
	Bool unUsefulCand = false;
	VideoData *pCurVideo = m_pcHeadPointer->getCurPic()->getCurVideoData();
	UInt curPicNum, curObjCandNum, curCheckDir = 1;
	PicData* pCurPic = nullptr;
	std::vector<ObjectCandidate*> usefulCands;
	ObjectCandidate *pCurCand = nullptr, *pCheckCand = nullptr, *pNodeCand = nullptr, *pPriorCand = nullptr, *pNextCand = nullptr;
	
	std::pair<Bool, Int> curStep;
	std::vector<std::pair<Bool, Int>>  curDirAndStep;
	curStep.first = false;
	curStep.second = 0;
	curDirAndStep.clear();
	for (Int dirNum = 0; dirNum < 4; dirNum++)
		curDirAndStep.push_back(curStep);

	//collect all useful candidates 
	auto picIndex = m_mPicIndex.begin();
	auto nextPicIndex = m_mPicIndex.begin();
	while (picIndex != m_mPicIndex.end())
	{
		nextPicIndex = picIndex;
		++nextPicIndex;
		if (nextPicIndex != m_mPicIndex.end() && nextPicIndex->first == picIndex->first)
		{
			curPicNum = picIndex->first;
			curObjCandNum = picIndex->second;
			pCurPic = pCurVideo->getPic(curPicNum - 1);
			pCurCand = pCurPic->getObjectWithIdx(curObjCandNum - 1);

			while (nextPicIndex != m_mPicIndex.end() && nextPicIndex->first == picIndex->first)
				++nextPicIndex;
			if (nextPicIndex == m_mPicIndex.end())
				break;
			else
			{
				picIndex = nextPicIndex;
				continue;
			}
		}
		curPicNum = picIndex->first;
		curObjCandNum = picIndex->second;
		pCurPic = pCurVideo->getPic(curPicNum - 1);
		pCurCand = pCurPic->getObjectWithIdx(curObjCandNum - 1);

		usefulCands.push_back(pCurCand);
	
		m_mDirAndMotion[pCurCand] = curDirAndStep;
		++picIndex;
	}

	

	cout << "the size of useful is " << usefulCands.size() << endl;

	vector<UInt> picNum1, picNum2;

	getCandCentroid(usefulCands, 1, picNum1);
	writeCandEdge(usefulCands);

	cout << "finish detecting" << endl;
	getObjectTraUpdated(usefulCands, 1, picNum1);
	getObjectTraUpdated(usefulCands, 2, picNum2);

	cout << "bug222 " << endl;

}


Void MovingObject::segment()
{
	cout << "segmenting " << endl;
	///////////////////////////////////////////////////////////////////
	Bool unUsefulCand = false;
	VideoData *pCurVideo = m_pcHeadPointer->getCurPic()->getCurVideoData();
	UInt curPicNum, curObjCandNum, curCheckDir = 1;
	PicData* pCurPic = nullptr;
	std::vector<ObjectCandidate*> usefulCands;
	ObjectCandidate *pCurCand = nullptr;

	//collect all useful candidates 
	auto picIndex = m_mPicIndex.begin();
	auto nextPicIndex = m_mPicIndex.begin();
	while (picIndex != m_mPicIndex.end())
	{
		nextPicIndex = picIndex;
		++nextPicIndex;
		if (nextPicIndex != m_mPicIndex.end() && nextPicIndex->first == picIndex->first)
		{
			curPicNum = picIndex->first;
			curObjCandNum = picIndex->second;
			pCurPic = pCurVideo->getPic(curPicNum - 1);
			pCurCand = pCurPic->getObjectWithIdx(curObjCandNum - 1);

			while (nextPicIndex != m_mPicIndex.end() && nextPicIndex->first == picIndex->first)
				++nextPicIndex;
			if (nextPicIndex == m_mPicIndex.end())
				break;
			else
			{
				picIndex = nextPicIndex;
				continue;
			}
		}
		curPicNum = picIndex->first;
		curObjCandNum = picIndex->second;
		pCurPic = pCurVideo->getPic(curPicNum - 1);
		pCurCand = pCurPic->getObjectWithIdx(curObjCandNum - 1);

		usefulCands.push_back(pCurCand);
		++picIndex;
	}

	vector<UInt> picNum1, picNum2;
	objSeg(usefulCands, 1, picNum1);
}

Void MovingObject::addMotionStepInfo(UInt dirIdx, Int startPicIdx, Int endPicIdx, Int motionDir, UInt len)
{
	std::pair<std::pair<Int, Int>, std::pair<Int, Int>> dirMotionInfo;
	std::pair<std::pair<Int, Int>, std::pair<Bool, Bool>> dirMotionState;

	std::pair<Int, Int> stageStartAndEndIndex;
	std::pair<Int, Int>  stageMotionAndLength;

	std::pair<Bool, Bool>   stageAndstate;

	stageStartAndEndIndex.first = startPicIdx;
	stageStartAndEndIndex.second = endPicIdx;

	stageMotionAndLength.first = motionDir;
	stageMotionAndLength.second = len;

	stageAndstate.first = true;
	stageAndstate.second = true;

	dirMotionInfo.first = stageStartAndEndIndex;
	dirMotionInfo.second = stageMotionAndLength;

	dirMotionState.first = stageStartAndEndIndex;
	dirMotionState.second = stageAndstate;

	m_mMotionStageAndLength[dirIdx - 1].push_back(dirMotionInfo);
	m_mMotionStageAndState[dirIdx - 1].push_back(dirMotionState);
}
//
Void MovingObject::checkMotionDirection(std::vector<ObjectCandidate*> &usefulCands, std::vector<ObjectCandidate*>::iterator &travelIndex, UInt checkDir)
{
	Bool stopMergeFlag = false, turnFlag = true, settledFlag = false;
	ObjectCandidate *pCurCand, *pCheckCand;
	UInt checkStride = 0, routeStride = 0, ctuSize, stepLen = 0;
	UInt firRouteLen = 0, secRouteLen = 0, firRouteFaultLen = 0, secRouteFaultLen = 0;
	Int curStepEdge, checkEdge, edgeDelta, changSign, begPicIdx, endPicIdx;
	std::map<ObjectCandidate*, Int, candInOrder> checkMotRoute;
	changSign = 0;
	ctuSize = m_pcHeadPointer->getCurPic()->getCtuWid();
	while (turnFlag)
	{
		if (travelIndex == usefulCands.end())
			Int debugppp = 0;
		auto firStepIdx = travelIndex;
		auto secStepIdx = travelIndex;
		if ((*firStepIdx)->getPicNum() == (*usefulCands.rbegin())->getPicNum())
			return;
		if (changSign != 0)
			turnFlag = false;
		checkStride = 0;
		pCurCand = *travelIndex;//
		checkMotRoute.clear();
		settledFlag = stopMergeFlag = false;
		firRouteLen = firRouteFaultLen = secRouteLen = secRouteFaultLen = 0;
		curStepEdge = (Int)pCurCand->getEdgeWithDir(checkDir);

		confirmBoundryUniform(usefulCands, travelIndex, checkDir, checkStride);//check first route 

		secStepIdx = firStepIdx;
		curStepEdge = (*firStepIdx)->getEdgeWithDir(checkDir);
		while (secStepIdx != travelIndex)//
		{
			pCurCand = *secStepIdx;
			if (pCurCand->getEdgeWithDir(checkDir) != curStepEdge)
				++firRouteFaultLen;
			checkMotRoute[pCurCand] = curStepEdge;
			++secStepIdx;
			++firRouteLen;
		}

		if (travelIndex == usefulCands.end())//
		{
			if (firRouteFaultLen / (Float)firRouteLen > 0.35)
			{
				travelIndex = firStepIdx++;//
				return;
			}
			else
				settledFlag = true;
		}
		if (travelIndex != usefulCands.end())//
		{
			pCheckCand = *travelIndex;////////////////////////////
			changSign = pCurCand->getEdgeWithDir(checkDir) < pCheckCand->getEdgeWithDir(checkDir) ? 1 : -1;//////////////////////////

			if (pCurCand->getEdgeWithDir(checkDir) == pCheckCand->getEdgeWithDir(checkDir))
				stopMergeFlag = true;

			checkStride = checkStride > 1 ? checkStride : 2;
			routeStride = stopMergeFlag ? checkStride : checkStride * 2;
			confirmBoundryUniform(usefulCands, travelIndex, checkDir, routeStride);///////////////////////
			pCheckCand = *secStepIdx;
			curStepEdge = pCheckCand->getEdgeWithDir(checkDir);
			while (secStepIdx != usefulCands.end() && secStepIdx != travelIndex)//////////////////////////////
			{
				if ((*secStepIdx)->getEdgeWithDir(checkDir) != pCheckCand->getEdgeWithDir(checkDir))
					++secRouteFaultLen;
				checkMotRoute[(*secStepIdx)] = curStepEdge;
				++secRouteLen;
				++secStepIdx;
			}
			if ((secRouteLen == 0 && travelIndex != usefulCands.end()) || (secRouteFaultLen / (Float)secRouteLen > 0.35f))///////////////////////////////
				settledFlag = false;
			else
				settledFlag = true;
			if (settledFlag == false && travelIndex == usefulCands.end())
			{
				settledFlag = true;
				changSign = 0;
			}
		}
		//find each change point
		if (settledFlag)
		{
			turnFlag = false;
			Int stepDelta = 0;
			Int stepSize = 0;

			auto routeIndex = checkMotRoute.begin();
			pCurCand = routeIndex->first;
			begPicIdx = pCurCand->getPicNum();
			curStepEdge = pCurCand->getEdgeWithDir(checkDir);
			m_mDirAndMotion[routeIndex->first][checkDir - 1].first = true;
			auto curDirIndex = m_mDirAndMotion.find(routeIndex->first);
			if (curDirIndex != m_mDirAndMotion.end() && curDirIndex != m_mDirAndMotion.begin())
			{
				--curDirIndex;
				while (curDirIndex->second[checkDir - 1].first == false && curDirIndex != m_mDirAndMotion.begin())
					--curDirIndex;
				if (curDirIndex == m_mDirAndMotion.begin())
				{
					while (curDirIndex->second[checkDir - 1].first == false)
						++curDirIndex;
				}
				stepSize = curDirIndex->second[checkDir - 1].second;
				if (curDirIndex->first->getEdgeWithDir(checkDir) != routeIndex->first->getEdgeWithDir(checkDir))
				{
					stepDelta = routeIndex->first->getEdgeWithDir(checkDir) - curDirIndex->first->getEdgeWithDir(checkDir);;
					stepSize += stepDelta;
				}
			}
			else
				stepSize = m_mDirAndMotion[routeIndex->first][checkDir - 1].second;
			stepDelta = stepDelta == 0 ? 0 : stepDelta > 0 ? 1 : -1;
			
			while (routeIndex != checkMotRoute.end())
			{
				checkEdge = routeIndex->second;
				if ((curStepEdge < checkEdge && changSign == 1) || (curStepEdge > checkEdge && changSign == -1))
				{
					addMotionStepInfo(checkDir, begPicIdx, endPicIdx, stepDelta, firRouteLen);
					begPicIdx = routeIndex->first->getPicNum();
					edgeDelta = checkEdge - curStepEdge;
					stepSize += edgeDelta;
					curStepEdge  = checkEdge;
				}
				m_mDirAndMotion[routeIndex->first][checkDir - 1].first = true;
				
				m_mDirAndMotion[routeIndex->first][checkDir - 1].second = stepSize;
				endPicIdx = routeIndex->first->getPicNum();
				++routeIndex;
			}
			if (changSign != 0)
				addMotionStepInfo(checkDir, begPicIdx, endPicIdx, changSign, secRouteLen);
			else
				addMotionStepInfo(checkDir, begPicIdx, endPicIdx, stepDelta, firRouteLen);

			travelIndex = secStepIdx;
		}
	}

	if (settledFlag == false)
		++travelIndex;
	//test end ======================================//
	if (travelIndex == usefulCands.end())
		Int debugPoint = 0;
	if (travelIndex == usefulCands.end() || (*travelIndex)->getPicNum() == (*usefulCands.rbegin())->getPicNum())
	{
		Bool testResult = false;
		Int changeTurnTimes = 0, objNumIdx = 0;
		std::string midResult("midleResult");
		std::string  turnTimeName;
		Char objDstIdx[3];
		Char turnDstIdx[3];
		Char dirDstIdx[2];
		Char* destName;
		FILE *midRes;
		objNumIdx = m_uiMovObjectNum;
	//	midRes = fopen(midResult, "w+");
		Bool loopFlag = true;
//		if (usefulCands.size() == 202 && (checkDir == 4 || checkDir == 3))
			testResult = true;
		if (testResult)
		{
			//_itoa(objeIndex, str, 10);
			_itoa(objNumIdx, objDstIdx, 10);
			_itoa(changeTurnTimes, turnDstIdx, 10);
			_itoa(checkDir, dirDstIdx, 10);
			turnTimeName = objDstIdx + midResult + dirDstIdx + turnDstIdx + ".txt";
			destName = _strdup(turnTimeName.c_str());
			midRes = fopen(destName, "w+");
			setBoundryMotionValue(checkDir);
			auto dirBeg = m_mDirAndMotion.begin();
			printf("===========================new result=================================== \n");
			while (dirBeg != m_mDirAndMotion.end())
			{
				//if (dirBeg->second[checkDir - 1].first == true)
				//	fprintf(midRes, "%d \n", dirBeg->second[checkDir - 1].second);

				++dirBeg;
			}
			++changeTurnTimes;
			fclose(midRes);
		}
		setMotionStageState(checkDir, false);
		while (loopFlag)
		{
			stopMergeFlag = true;
			while (stopMergeFlag)
			{
				stopMergeFlag = mergeShortMotion(checkDir);//stopMergeFlag = removeShotMotion(checkDir);
				if (stopMergeFlag && testResult)
				{
					//_itoa(objeIndex, str, 10);
					_itoa(changeTurnTimes, turnDstIdx, 10);
					_itoa(checkDir, dirDstIdx, 10);
					turnTimeName = objDstIdx + midResult + dirDstIdx + turnDstIdx + ".txt";
					destName = _strdup(turnTimeName.c_str());
					midRes = fopen(destName, "w+");
					setBoundryMotionValue(checkDir);
					auto dirBeg = m_mDirAndMotion.begin();
					printf("===========================new result=================================== \n");
					while (dirBeg != m_mDirAndMotion.end())
					{
						//if (dirBeg->second[checkDir - 1].first == true)
						//	fprintf(midRes, "%d \n", dirBeg->second[checkDir - 1].second);

						++dirBeg;
					}
					++changeTurnTimes;
					fclose(midRes);
				}
			}
			loopFlag = false;
			stopMergeFlag = true;
			while (stopMergeFlag)
			{
				stopMergeFlag = removeBoundryShortMotion(checkDir);//mergeShortMotion(checkDir);
				if (stopMergeFlag && testResult)
				{
					setBoundryMotionValue(checkDir);
					_itoa(changeTurnTimes, turnDstIdx, 10);
					_itoa(checkDir, dirDstIdx, 10);

					turnTimeName = objDstIdx + midResult + dirDstIdx + turnDstIdx + ".txt";
					destName = _strdup(turnTimeName.c_str());
					midRes = fopen(destName, "w+");
					//midRes = fopen(turnTimeName.c_str(), "w+");
					auto dirBeg = m_mDirAndMotion.begin();
					printf("===========================new result=================================== \n");
					while (dirBeg != m_mDirAndMotion.end())
					{
						if (dirBeg->second[checkDir - 1].first == true)
							fprintf(midRes, "%d \n", dirBeg->second[checkDir - 1].second);

						++dirBeg;
					}
					fclose(midRes);
					++changeTurnTimes;
				}
			}
			loopFlag = false;
			stopMergeFlag = true;
			while (stopMergeFlag)
			{
				stopMergeFlag = removeBoundryShortMotion(checkDir);//mergeShortMotion(checkDir);
				if (stopMergeFlag && testResult)
				{
					setBoundryMotionValue(checkDir);
					_itoa(changeTurnTimes, turnDstIdx, 10);
					_itoa(checkDir, dirDstIdx, 10);

					turnTimeName = objDstIdx + midResult + dirDstIdx + turnDstIdx + ".txt";
					destName = _strdup(turnTimeName.c_str());
					midRes = fopen(destName, "w+");
					//midRes = fopen(turnTimeName.c_str(), "w+");
					auto dirBeg = m_mDirAndMotion.begin();
					printf("===========================new result=================================== \n");
					while (dirBeg != m_mDirAndMotion.end())
					{
						if (dirBeg->second[checkDir - 1].first == true)
							fprintf(midRes, "%d \n", dirBeg->second[checkDir - 1].second);

						++dirBeg;
					}
					fclose(midRes);
					++changeTurnTimes;
				}
				if (stopMergeFlag)
					loopFlag = true;
			}
		}
		auto testIdx = m_mMotionStageAndLength.begin();
		auto testDirMotion = m_mDirAndMotion;
		setBoundryMotionValue(checkDir);
		//test for result ===============================//
		auto dirBeg = m_mDirAndMotion.begin();
		printf("===========================new result=================================== \n");
		while (dirBeg != m_mDirAndMotion.end())
		{
			if (dirBeg->second[checkDir - 1].first == true)
				printf("%d \n", dirBeg->second[checkDir - 1].second);
			++dirBeg;
		}
		//test end ======================================//
		Int breakPoint = 0;
	}
	//test end ======================================//
	if (travelIndex == usefulCands.end())
		Int debugPoint = 0;
}


//Void MovingObject::markOneFourthCTUInFrames(std::vector<ObjectCandidate*> &usefulCands, vector<ObjectCandidate*>::iterator travelIndex)
//{
//	PicData* picOfThisCand = (*travelIndex)->getCurPic();
//	
//	UInt ObjectNum = this->getMovObjectNum();
//	char number[10];
//	sprintf(number, "%d", ObjectNum);
//	string outfileName = number;
//	outfileName += "newMarkedCTUs.txt";
//
//	vector<UInt> partitionInCtu;
//	for (UInt i = 0; i < 4; i++)
//		partitionInCtu.push_back(0);
//	vector<vector<UInt>> addressCTUpartition;
//	for (UInt i = 0; i < 220; i++)
//		addressCTUpartition.push_back(partitionInCtu);
//
//	if (travelIndex == usefulCands.begin())
//	fout.open(outfileName);
//
//	std::map<CtuData*, Bool, ctuAddrIncCompare> currCtuElement;
//	CtuData* pCtu,*pCurrCtu = NULL;
//	UInt depth = 0;
//	vector<CUData*> CU;
//	vector<UInt> numOfCuInSmallCtu;
//	vector<UInt> addressOrderPartition;
//	vector<UInt> ctuAddressFlag;//if the CTU has been checked, its address will be put into this vector
//	vector<vector<UInt>> currCtuAndPar;
//	bool checkedFlag = false;
//	UInt atPicEdge=0;
//	currCtuElement = (*travelIndex)->getCtuElements();
//	auto beg = currCtuElement.begin();
//	while (beg != currCtuElement.end())
//	{
//		pCtu = beg->first;
//		UInt newAddr = pCtu->getCtuAddr();
//		if (newAddr<220)
//		for (UInt k = 0; k < 4; k++)
//		addressCTUpartition[newAddr][k]=1;
//		for (UInt i = 0; i < 5; i++)
//		{
//			pCurrCtu = pCtu->getNeighbourCTU(i);
//			if (pCurrCtu == NULL)
//				continue;
//			UInt currAddr = pCurrCtu->getCtuAddr();
//			auto address = ctuAddressFlag.begin();
//			checkedFlag = false;
//			while (address != ctuAddressFlag.end())
//			{
//				if (currAddr == (*address))
//				{
//					checkedFlag = true;
//					break;
//				}
//				address++;
//			}
//			//if (checkedFlag == true)
//				//continue;
//			ctuAddressFlag.push_back(pCurrCtu->getCtuAddr());
//			depth = pCurrCtu->getMaxDepth();
//			if (depth >= 2)
//			{
//				numOfCuInSmallCtu.clear();
//				CU = pCurrCtu->getCB();
//				pCurrCtu->totalCuInSmallCtu(numOfCuInSmallCtu);
//				UInt picNum = (*travelIndex)->getPicNum();
//				UInt ctuAddr = pCurrCtu->getCtuAddr();
//				for (UInt j = 0; j < 4; j++)
//				{
//					if (numOfCuInSmallCtu[j]>2)
//					{
//						//fout << ctuAddr << " " << j << " " << numOfCuInSmallCtu[j]<<" ";
//						addressOrderPartition.clear();
//						addressOrderPartition.push_back(ctuAddr);
//						addressOrderPartition.push_back(j);
//						addressOrderPartition.push_back(numOfCuInSmallCtu[j]);
//						atPicEdge = 0;
//						pCurrCtu->partInSurroundSmallCtu(j, addressOrderPartition,atPicEdge);
//						if (atPicEdge == 0)
//						{
//							currCtuAndPar.push_back(addressOrderPartition);
//							addressCTUpartition[ctuAddr][j] = numOfCuInSmallCtu[j];
//							CtuData* pointerCtu = new CtuData;
//							UInt a, b;
//							pCurrCtu->getAbsXY(a,b);
//					
//							pointerCtu->setAbsXY(a,b);
//							pointerCtu->setOrderOfSmallCtu(j);
//							pointerCtu->setPartInSmallCtu(numOfCuInSmallCtu[j]);
//							picOfThisCand->addSmallCtu(pointerCtu);
//						}
//					}
//				}
//			}
//		}
//		beg++;
//	}
//	(*travelIndex)->setCurrSCtuAndPart(currCtuAndPar);
//	
//		fout << endl;
//		fout << "this is the object " << ObjectNum << endl;
//		fout << "the number of frames is " << (*travelIndex)->getPicNum() << ",  the same as in the YUV file" << endl;
//
//		UInt smallCtu = 0;
//		for (int i = 0; i < 11; i++)
//		{		
//			if (i % 4 == 0)
//			{
//				fout << endl;
//			}
//			for (int j = 0; j < 20; j++)
//			{
//				if (j % 4 == 0)
//					fout << " " ;
//				smallCtu = addressCTUpartition[j + 20 * i][0];
//				if (smallCtu != 0)
//				{
//					if (smallCtu != 1)
//						fout << "X" << " ";
//#ifdef showParseSmallCTU
//					else
//						fout << "1" << " ";
//#endif
//				}
//				else fout << "0 ";
//
//				smallCtu = addressCTUpartition[j + 20 * i][1];
//				if (smallCtu != 0)
//				{
//					if (smallCtu != 1)
//						fout << "X" << " ";
//#ifdef showParseSmallCTU
//					else
//						fout << "1" << " ";
//#endif
//				}
//				else fout << "0 ";
//			
//
//	
//			}
//			fout << endl;
//			for (int j = 0; j < 20; j++)
//			{
//				if (j % 4 == 0)
//					fout << " " ;
//
//				smallCtu = addressCTUpartition[j + 20 * i][2];
//				if (smallCtu != 0)
//				{
//					if (smallCtu != 1)
//						fout << "X" << " ";
//#ifdef showParseSmallCTU
//					else
//						fout << "1" << " ";
//#endif
//				}
//				else fout << "0 ";
//		
//				smallCtu = addressCTUpartition[j + 20 * i][3];
//				if (smallCtu != 0)
//				{
//					if (smallCtu != 1)
//						fout << "X" << " ";
//#ifdef showParseSmallCTU
//					else
//						fout << "1" << " ";
//#endif
//				}
//				else fout << "0 ";
//			}
//			fout << endl;
//		}
//}


Void MovingObject::getObjectTra(std::vector<ObjectCandidate*> &usefulCands,int direction)
{

}


Void MovingObject::getCandCentroid(std::vector<ObjectCandidate*> &usefulCands, int direction, vector<UInt>& picNum)
{
	string textName = "tra.txt";

	UInt ObjectNum = this->getMovObjectNum();

	UInt longDisPicNum = 0;
	char number[10];
	sprintf(number, "%d", ObjectNum);
	string outfileName = "./traDatas/";
	outfileName += number;
	ofstream fout;

	outfileName += textName;
	fout.open(outfileName);
	if (!fout){
		cout << "file open failed.\n";
		exit(1);//program exit
	}
	cout << "detecting the ObjectCentroid of obect  " << number << endl;


	auto beginIndex = usefulCands.begin();
	auto currIndex = beginIndex;

	map<CtuData*, Bool, ctuAddrIncCompare>	m_mCtuElement;
	CtuData *ctu = NULL;
	int x = 0, y = 0;

	UInt centroidX, centroidY;

	vector<ObjectCandidate*> begin;
	vector<ObjectCandidate*> end;

	while (currIndex != usefulCands.end())
	{
		//cout << "the number of frames is " << (*currIndex)->getPicNum()<<" ";
		//if ((*currIndex)->getPicNum() % 2 == 0)
		//cout << endl;

		(*currIndex)->getObjCentroid(centroidX, centroidY, false);
		//fout << "the number of frames is " << (*currIndex)->getPicNum() << "  X,Y  is " << centroidX <<"  "<< centroidY << endl;
		fout << (*currIndex)->getPicNum() << "  " << centroidX << "  " << centroidY << endl;
		(*currIndex)->setPredX(centroidX);
		(*currIndex)->setPredY(centroidY);

		currIndex++;
	}
	fout.close();
}



Void MovingObject::getObjectTraUpdated(std::vector<ObjectCandidate*> &usefulCands, int direction, vector<UInt>& picNum)
{
	string textName;
	int directionOne = 0;
	int directionTwo = 0;
	if (direction == 1)
	{
		directionOne = 1;
		directionTwo = 2;
		textName = "LR.txt";
	}
	if (direction == 2)
	{
		directionOne = 3;
		directionTwo = 4;
		textName = "TB.txt";
	}
	UInt ObjectNum = this->getMovObjectNum();
	UInt longDisPicNum = 0;
	char number[10];
	sprintf(number, "%d", ObjectNum);

	string outfileName ="./traDatas/";
	outfileName += number;

	outfileName += textName;
	fout.open(outfileName);
	if (!fout){
		cout << "file open failed.\n";
		exit(0);//program exit
	}
	cout << "detecting the LRTB of obect  " << number << endl;


	auto beginIndex = usefulCands.begin();



	if (beginIndex >= (usefulCands.end() - 3) || usefulCands.size()<4)//this function requires the usefulCands's size not smaller than 4
	{
		fout.close();
		return;
	}


	auto beginIndex2 = beginIndex + 1;
	auto beginIndex3 = beginIndex + 2;
	auto beginIndex4 = beginIndex + 3;




	vector<ObjectCandidate*> begin;
	vector<ObjectCandidate*> end;



	cout << "oneeee" << endl;
	

	UInt longPicNum = 0, begPicNum = 0;

	UInt longDis = 20;

	begPicNum = (*beginIndex4)->getPicNum();

	begin.clear();
	begin.push_back(*beginIndex);

	beginIndex2 = beginIndex + 1;
	begin.push_back(*beginIndex2);

	beginIndex3 = beginIndex + 2;
	begin.push_back(*beginIndex3);

	beginIndex4 = beginIndex + 3;
	begin.push_back(*beginIndex4);

	UInt centroidX = 0, centroidY = 0;
	UInt initPicNum = (*beginIndex)->getPicNum();


	UInt sumX = 0, sumY = 0;

	for (int i = 0; i < 4; i++)
	{
		begin[i]->getObjCentroid(centroidX, centroidY, false);
		sumX += centroidX;
		sumY += centroidY;

	}
	centroidX = sumX / 4;
	centroidY = sumY / 4;
	fout << initPicNum << " " << centroidX << "  " << centroidY << endl;

	auto endIndex = beginIndex;
	if (endIndex >= (usefulCands.end() - 3))
	{
		fout.close();
		return;
	}
	auto endIndex2 = endIndex + 1;
	auto endIndex3 = endIndex + 2;
	auto endIndex4 = endIndex + 3;


	while (endIndex < (usefulCands.end() - 3))
	{

		end.clear();
		end.push_back(*endIndex);

		endIndex2 = endIndex + 1;
		end.push_back(*endIndex2);

		endIndex3 = endIndex + 2;
		end.push_back(*endIndex3);

		endIndex4 = endIndex + 3;
		end.push_back(*endIndex4);


		UInt totalEndNum = 4;
		if (endIndex > (usefulCands.begin() + 3))
		{
			totalEndNum = 7;
			auto endIndex7 = endIndex - 3;
			auto endIndex6 = endIndex - 2;
			auto endIndex5 = endIndex - 1;
			end.push_back(*endIndex5);
			end.push_back(*endIndex6);
			end.push_back(*endIndex7);
		}

		int deltaOne = 0;
		int deltaTwo = 0;

		deltaOne = (*endIndex)->getEdgeWithDir(directionOne) - (*beginIndex)->getEdgeWithDir(directionOne);
		deltaTwo = (*endIndex)->getEdgeWithDir(directionTwo) - (*beginIndex)->getEdgeWithDir(directionTwo);


		int score = 0;
		int score2 = 0;
		int tempDeltaOne = 0;
		int tempDeltaTwo = 0;
		begPicNum = (*beginIndex)->getPicNum();
		longPicNum = (*endIndex)->getPicNum();
		if (((deltaOne >= 0 && deltaTwo >= 0) || (deltaOne <= 0 && deltaTwo <= 0)) && ((longPicNum - begPicNum) >= longDis))
		{
			score = 0;
			score2 = 0;


			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					tempDeltaOne = end[j]->getEdgeWithDir(directionOne) - begin[i]->getEdgeWithDir(directionOne);
					tempDeltaTwo = end[j]->getEdgeWithDir(directionTwo) - begin[i]->getEdgeWithDir(directionTwo);
					if (((tempDeltaOne*deltaOne)>0) && ((tempDeltaTwo*deltaTwo)>0) && ((deltaOne*deltaTwo) > 0))
					{
						score++;
					}
					if (((tempDeltaOne*deltaOne) >= 0) && ((tempDeltaTwo*deltaTwo) >= 0) && ((deltaOne*deltaTwo) >= 0))
					{
						score2++;
					}
				}
			}
			int sigma = 0;
			int deltaNoise = 0;


			for (int j = 0; j < totalEndNum; j++)
			{
				UInt centroidX1 = 0, centroidY1 = 0;
				UInt centroidX2 = 0, centroidY2 = 0;


				if (j == (totalEndNum-1))
				{
					end[totalEndNum - 1]->getObjCentroid(centroidX1, centroidY1, false);
					end[0]->getObjCentroid(centroidX2, centroidY2, false);
				}
				else
				{
					end[j]->getObjCentroid(centroidX1, centroidY1, false);
					end[j + 1]->getObjCentroid(centroidX2, centroidY2, false);
				}

				int sig = 0;

				if (direction == 1)
				{
					sig = int(centroidX1) - int(centroidX2);
					deltaNoise += sig;
					sigma += abs(sig);

				}

				if (direction == 2)
				{
					sig = int(centroidY1) - int(centroidY2);
					deltaNoise += sig;
					sigma += abs(sig);

				}

			}
			cout << "sigma is " << sigma << endl;

			if ((score >= 7 || score2 >= 13) && sigma < 130)
			{

				UInt centroidX = 0, centroidY = 0;
				UInt sumX = 0, sumY = 0;

				for (int i = 0; i < totalEndNum; i++)
				{
					end[i]->getObjCentroid(centroidX, centroidY, true);
					sumX += centroidX;
					sumY += centroidY;

				}
				centroidX = sumX / totalEndNum;
				centroidY = sumY / totalEndNum;
				fout << longPicNum << " " << centroidX << "  " << centroidY << endl;
				fout.close();
				return;
			}

		}
		
			endIndex++;
	}


	fout.close();
	return;
}

Void MovingObject::objSeg(std::vector<ObjectCandidate*> &usefulCands, int direction, vector<UInt>& picNum)
{	
	UInt ObjectNum = this->getMovObjectNum();
	UInt longDisPicNum = 0;
	char number[10];
	sprintf(number, "%d", ObjectNum);

	cout << "object segmenting  " << number << endl;


	auto beginIndex = usefulCands.begin();


	auto endIndex = beginIndex;

	UInt oldPicNum = 0;

	picNum.push_back(oldPicNum);

	map<CtuData*, Bool, ctuAddrIncCompare>	m_mCtuElement;
	CtuData *ctu = NULL;
	int x = 0, y = 0;

	m_mCtuElement = (*beginIndex)->getCtuElements();
	auto it = m_mCtuElement.begin();
	
	int beginX = 0, beginY = 0;
	while (it != m_mCtuElement.end())
	{
		ctu = (*it).first;
		x = ctu->getAbsx(); y = ctu->getAbsy();
	
		it++;
	}

	vector<ObjectCandidate*> begin;
	vector<ObjectCandidate*> end;


	while (endIndex != usefulCands.end())
	{
		if (beginIndex == usefulCands.end())
		{
			return;
		}

		if (endIndex == usefulCands.end())
		{
			return;
		}
		endIndex++;
	}


}

Void MovingObject::calMotionVec(std::vector<ObjectCandidate*> &usefulCands, vector<UInt> picLR, vector<UInt> picUD)
{
	//auto oldMidIndex = usefulCands.begin();
	//auto lastMidIndex = usefulCands.begin();
	//auto currMidIndex = usefulCands.begin();
	//auto tempIndex = usefulCands.begin();
	//auto temLastMidIndex = lastMidIndex;
	//auto temCurrMidIndex = currMidIndex;

	//UInt oldPicNum = 0, lastPicNum = 0, currPicNum = 0;
	//int i = 0, j = 0;
	//UInt currLRPicNum = picLR[i], currUDPicNum = picUD[j];
	//if (currLRPicNum != currUDPicNum)
	//{
	//	cout << "there exist some problems" << endl;
	//	int debug; cin >> debug;
	//}
	//int xPos = 0, yPos = 0;
	//UInt xPosUInt = 0,yPosUInt = 0;
	//(*oldMidIndex)->getObjectPos(xPosUInt, yPosUInt, 1);
	//xPos = xPosUInt; yPos = yPosUInt;
	//i++; j++;
	//bool isLR = false;
	//int count = 0;
	//while (i != picLR.size() || j != picUD.size())
	//{
	//	currLRPicNum = picLR[i], currUDPicNum = picUD[j];

	//	if (i == picLR.size())
	//		currLRPicNum = 1000000;
	//	if (j == picUD.size())
	//		currUDPicNum = 1000000;

	//	if (currLRPicNum <= currUDPicNum)
	//	{
	//		isLR = true;
	//		lastPicNum = picLR[i - 1];
	//		currPicNum = currLRPicNum;
	//		i++;
	//	}
	//	else
	//	{
	//		isLR = false;
	//		lastPicNum = picUD[j - 1];
	//		currPicNum = currUDPicNum;
	//		j++;
	//	}


	//	tempIndex = usefulCands.begin();
	//	//find the lastMidIndex
	//	while (tempIndex != usefulCands.end())
	//	{
	//		if (lastPicNum == (*tempIndex)->getPicNum())
	//		{
	//			lastMidIndex = tempIndex;
	//			break;
	//		}
	//		tempIndex++;
	//	}
	//	if (tempIndex == usefulCands.end())
	//	{
	//		cout << "cannot search the old index" << endl;
	//		int debug; cin >> debug;
	//	}
	//	tempIndex = usefulCands.begin();
	//	//find the newMidIndex
	//	while (tempIndex != usefulCands.end())
	//	{
	//		if (currPicNum == (*tempIndex)->getPicNum())
	//		{
	//			currMidIndex = tempIndex;
	//			break;
	//		}
	//		tempIndex++;
	//	}
	//	if (tempIndex == usefulCands.end())
	//	{
	//		cout << "cannot search the new index" << endl;
	//		int debug; cin >> debug;
	//	}

	//	CtuData *ctu = NULL;
	//	CtuData *ctuNew = NULL;
	//	vector<UInt>allBeginX, allBeginY;
	//	vector<UInt>allEndX, allEndY;
	//	vector<int> maxXVec;
	//	vector<int> maxYVec;
	//	UInt x = 0, y = 0;
	//	UInt beginX = 0, beginY = 0;
	//	UInt endX = 0, endY = 0;
	//	int deltaX = 0, deltaY = 0;
	//	map<CtuData*, Bool, ctuAddrIncCompare>	m_mCtuElement;
	//	map<CtuData*, Bool, ctuAddrIncCompare>	newCtuElement;
	//	m_mCtuElement = (*oldMidIndex)->getCtuElements();

	//	(*lastMidIndex)->getObjectPos(beginX, beginY,0);

	//	(*currMidIndex)->getObjectPos(endX, endY, 0);


	//	deltaX = endX - beginX;
	//	deltaY = endY - beginY;

	//	int tempX = 0, tempY = 0;
	//	int trueDeltaX = 0, trueDeltaY = 0;
	//	bool isExist = false;
	//	bool in = false;
	//	int max = -100;
	//	int ctuScore = 0;
	//	for (int m = -96 + deltaX; m <= deltaX + 96; m += 32)
	//	{
	//		for (int n = -96 + deltaY; n <= deltaY + 96; n += 32)
	//		{
	//			ctuScore = 0;
	//			for (int k = 0; k < 3; k++)
	//			{

	//				temLastMidIndex = lastMidIndex + k;
	//				temCurrMidIndex = currMidIndex + k;


	//				m_mCtuElement = (*temLastMidIndex)->getCtuElements();
	//				auto it = m_mCtuElement.begin();

	//				while (it != m_mCtuElement.end())
	//				{
	//					ctu = (*it).first;
	//					x = ctu->getAbsx(); y = ctu->getAbsy();
	//					x += m;
	//					y += n;
	//					in = (*temCurrMidIndex)->inObject(x, y);
	//					if (in == true)
	//					{
	//						ctuScore++;
	//					}
	//					it++;
	//				}
	//			}


	//			if (ctuScore >= max)
	//			{
	//				if (ctuScore > max)
	//				{
	//					fout << "the largest score is " << ctuScore << endl;
	//					cout << "the largest score is " << ctuScore << endl;
	//					max = ctuScore;
	//					maxXVec.clear();
	//					maxYVec.clear();
	//					maxXVec.push_back(m);
	//					maxYVec.push_back(n);
	//				}

	//				if (ctuScore == max)
	//				{
	//					maxXVec.push_back(m);
	//					maxYVec.push_back(n);
	//				}
	//			}

	//		}
	//	}

	//	in = false;
	//	currPicNum = (*currMidIndex)->getPicNum();
	//	oldPicNum = (*oldMidIndex)->getPicNum();
	//	lastPicNum = (*lastMidIndex)->getPicNum();
	//	count++;
	//	cout << "count is " << count << endl;
	//	cout << "currPicNum is "<<currPicNum<<endl;
	//	cout << "oldPicNum is " << oldPicNum << endl;
	//	cout << "lastPicNum is " << lastPicNum << endl;
	//	double ratio = (double(currPicNum - oldPicNum)) / (double(currPicNum - lastPicNum));
	//	cout << "ratio is " << ratio << endl;
	//	for (int i = 0; i < maxXVec.size(); i++)
	//	{
	//		if (isExist == true)
	//			break;
	//		
	//		tempX = xPos + maxXVec[i] *ratio;
	//		tempY = yPos + maxYVec[i] *ratio;


	//		in=(*currMidIndex)->inObject(tempX, tempY);
	//		if (in==true)
	//		{
	//			xPos = xPos + maxXVec[i] * ratio;
	//			yPos = yPos + maxYVec[i] * ratio;
	//			trueDeltaX = maxXVec[i] * ratio;
	//			trueDeltaY = maxYVec[i] * ratio;
	//			isExist = true;
	//		}	
	//	}

	//	if (isExist == false)
	//	{
	//		cout << "there is an unmatching point" << endl;
	//		for (int i = 0; i < maxXVec.size(); i++)
	//		{
	//			for (int ii = -64; ii <= 64; ii += 32)
	//			{
	//				for (int jj = -64; jj <= 64; jj += 32)
	//				{
	//					if (isExist == true)
	//						goto here;
	//					tempX = xPos + (maxXVec[i] + ii) *ratio;
	//					tempY = yPos + (maxYVec[i] + jj) *ratio;

	//					in = false;
	//					in = (*currMidIndex)->inObject(tempX, tempY);
	//					if (in == true)
	//					{
	//						xPos = xPos + (maxXVec[i] + ii) *ratio;
	//						yPos = yPos + (maxYVec[i] + jj) *ratio;
	//						trueDeltaX = (maxXVec[i] + ii) *ratio;
	//						trueDeltaY = (maxYVec[i] + jj) *ratio;
	//						isExist = true;
	//					}

	//				}
	//			}

	//		}
	//	}

	//here:
	//	if (isExist == false)
	//	{
	//		cout << "there is a wrong" << endl;
	//		//int debug; cin >> debug;
	//	}

	//	//if (oldMidIndex != usefulCands.begin())
	//	{
	//			(*currMidIndex)->setDeltaX(trueDeltaX);
	//			(*currMidIndex)->setDeltaY(trueDeltaY);
	//	}
	//	oldMidIndex = currMidIndex;


	//}
	
}





bool MovingObject::confirmObjectEdgeUniform(std::vector<ObjectCandidate*> &usefulCands, vector<ObjectCandidate*>::iterator& travelIndex, bool forward)
{
	UInt currEdgeRightToLeft = 0;
	UInt currEdgeBottomToTop = 0;
	UInt nextOneEdgeRightToLeft = 0;
	UInt nextOneEdgeBottomToTop = 0;
	UInt nextTwoEdgeRightToLeft = 0;
	UInt nextTwoEdgeBottomToTop = 0;

	if (forward == true)
	while (travelIndex != usefulCands.end())
	{
		currEdgeRightToLeft = (*travelIndex)->getEdgeWithDir(2) - (*travelIndex)->getEdgeWithDir(1);
		currEdgeBottomToTop = (*travelIndex)->getEdgeWithDir(4) - (*travelIndex)->getEdgeWithDir(3);


		auto nextOneIndex = travelIndex + 1;
		auto nextTwoIndex = travelIndex + 2;
		if (nextOneIndex == usefulCands.end())
		{
			nextOneIndex = travelIndex - 1;
			nextTwoIndex = travelIndex - 2;
		}
		else
		{
			if (nextTwoIndex == usefulCands.end())
			{
				nextTwoIndex = travelIndex - 1;
			}
			else
			{
				;
			}
		}

		nextOneEdgeRightToLeft = (*nextOneIndex)->getEdgeWithDir(2) - (*nextOneIndex)->getEdgeWithDir(1);
		nextOneEdgeBottomToTop = (*nextOneIndex)->getEdgeWithDir(4) - (*nextOneIndex)->getEdgeWithDir(3);
		nextTwoEdgeRightToLeft = (*nextTwoIndex)->getEdgeWithDir(2) - (*nextTwoIndex)->getEdgeWithDir(1);
		nextTwoEdgeBottomToTop = (*nextTwoIndex)->getEdgeWithDir(4) - (*nextTwoIndex)->getEdgeWithDir(3);


		UInt ratioOne1 = nextOneEdgeRightToLeft / currEdgeRightToLeft;
		UInt ratioOne2 = nextOneEdgeBottomToTop / currEdgeBottomToTop;
		UInt ratioTwo1 = nextOneEdgeRightToLeft / currEdgeRightToLeft;
		UInt ratioTwo2 = nextOneEdgeBottomToTop / currEdgeBottomToTop;
		if (ratioOne1 == 1 && ratioOne2 == 1)
			return true;
		if ((ratioOne1 == 1 && ratioOne2 == 1) || (ratioTwo1 == 1 && ratioTwo2 == 1))
		{
			return true;
		}
		else
			travelIndex++;
	}

	if (forward == false)
	while (travelIndex != usefulCands.begin())
	{
		currEdgeRightToLeft = (*travelIndex)->getEdgeWithDir(2) - (*travelIndex)->getEdgeWithDir(1);
		currEdgeBottomToTop = (*travelIndex)->getEdgeWithDir(4) - (*travelIndex)->getEdgeWithDir(3);

		auto nextOneIndex = travelIndex + 1;
		auto nextTwoIndex = travelIndex + 2;
		if (nextOneIndex == usefulCands.end())
		{
			nextOneIndex = travelIndex - 1;
			nextTwoIndex = travelIndex - 2;
		}
		else
		{
			if (nextTwoIndex == usefulCands.end())
			{
				nextTwoIndex = travelIndex - 1;
			}
			else
			{
				;
			}
		}

		nextOneEdgeRightToLeft = (*nextOneIndex)->getEdgeWithDir(2) - (*nextOneIndex)->getEdgeWithDir(1);
		nextOneEdgeBottomToTop = (*nextOneIndex)->getEdgeWithDir(4) - (*nextOneIndex)->getEdgeWithDir(3);
		nextTwoEdgeRightToLeft = (*nextTwoIndex)->getEdgeWithDir(2) - (*nextTwoIndex)->getEdgeWithDir(1);
		nextTwoEdgeBottomToTop = (*nextTwoIndex)->getEdgeWithDir(4) - (*nextTwoIndex)->getEdgeWithDir(3);


		UInt ratioOne1 = nextOneEdgeRightToLeft / currEdgeRightToLeft;
		UInt ratioOne2 = nextOneEdgeBottomToTop / currEdgeBottomToTop;
		UInt ratioTwo1 = nextOneEdgeRightToLeft / currEdgeRightToLeft;
		UInt ratioTwo2 = nextOneEdgeBottomToTop / currEdgeBottomToTop;
		if (ratioOne1 == 1 && ratioOne2 == 1)
			return true;
		if ((ratioOne1 == 1 && ratioOne2 == 1) || (ratioTwo1 == 1 && ratioTwo2 == 1))
		{
			return true;
		}
		else
			travelIndex--;
	}


	return false;

}





Bool MovingObject::confirmBoundryUniform(std::vector<ObjectCandidate*> &usefulCands, std::vector<ObjectCandidate*>::iterator &candIndex, UInt checkDir,UInt &checkStride)
{
	Bool stopFlag = false;
	Int curBoundry, confirmBoundry, confirmStride;
	ObjectCandidate *pCurCand, *pConfCand, *pFirstCand, *pSecondCand, *pThirdCand;
	auto confirmIndex = candIndex;
	auto curConfIndex = candIndex;
	pCurCand = *confirmIndex;
	curBoundry = pCurCand->getEdgeWithDir(checkDir);
	++confirmIndex;
	confirmStride = 0;
	checkStride = checkStride > 0 ? checkStride : (UInt)usefulCands.size();
	while (confirmIndex != usefulCands.end() && !stopFlag && checkStride > 0)
	{
		curConfIndex = confirmIndex;
		pConfCand = *confirmIndex;
		confirmBoundry = pConfCand->getEdgeWithDir(checkDir);
		if (confirmBoundry != curBoundry)
		{
			++confirmIndex;
			if (confirmIndex != usefulCands.end())
			{
				pFirstCand = *confirmIndex;
				if (pFirstCand->getEdgeWithDir(checkDir) == confirmBoundry)//next two have the same boundry
				{
					checkStride = confirmStride;
					stopFlag = true;
					candIndex = curConfIndex;
					break;
				}
				else
				{
					++confirmIndex;
					if (confirmIndex == usefulCands.end())//to then end 
					{
						if (pFirstCand->getEdgeWithDir(checkDir) == curBoundry)//go to the end of the route
							candIndex = confirmIndex;
						else
							candIndex = curConfIndex;//go back to current index 
						stopFlag = true;
						break;
					}
					else
					{
						pSecondCand = *confirmIndex;
						if (pSecondCand->getEdgeWithDir(checkDir) == confirmBoundry)//return to next boundry
						{
							auto fouthIndex = confirmIndex;
							++fouthIndex;
							if (fouthIndex != usefulCands.end() && pFirstCand->getEdgeWithDir(checkDir) == curBoundry)
							{
								pThirdCand = *fouthIndex;
								if (pThirdCand->getEdgeWithDir(checkDir) == curBoundry)//not consistant, change back to current boundry value
								{
									confirmIndex = fouthIndex;
									confirmStride += 4;
									++confirmIndex;
									if (confirmIndex == usefulCands.end())
									{
										candIndex = confirmIndex;
										break;
									}
									continue;
									stopFlag = false;
								}
								if (pThirdCand->getEdgeWithDir(checkDir) == confirmBoundry)//next step consistant 
								{
									candIndex = curConfIndex;
									//--candIndex;
									stopFlag = true;
									break;
								}
							}
							//next keep change 
							candIndex = curConfIndex;
							stopFlag = true;
							checkStride = confirmStride;
							break;
						}
						else
						{
							if (pFirstCand->getEdgeWithDir(checkDir) == pSecondCand->getEdgeWithDir(checkDir))
							{
								if (pFirstCand->getEdgeWithDir(checkDir) == curBoundry)
								{
									confirmStride += 3;
									stopFlag = false;
									continue;
								}
								candIndex = curConfIndex;
								stopFlag = true;
								checkStride = confirmStride;
								break;
							}
							else
							{
								if (curBoundry == pFirstCand->getEdgeWithDir(checkDir))//go to the first index 
								{
									confirmIndex = curConfIndex;
									++confirmStride;
								}
								else if (curBoundry == pSecondCand->getEdgeWithDir(checkDir))
								{
									confirmIndex = curConfIndex;
									++confirmIndex;
									confirmStride += 2;
								}
								else
								{
									candIndex = curConfIndex;
									stopFlag = true;
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				candIndex = confirmIndex;
				checkStride = confirmStride;
				stopFlag = true;
				break;
			}
		}
		--checkStride;
		++confirmStride;
		++confirmIndex;
		if (confirmIndex == usefulCands.end())
			candIndex = confirmIndex;
		if (!stopFlag)
			checkStride = checkStride < 1 ? 1 : checkStride;
	}
	return true;
}


Bool MovingObject::checkObjectsOverlapFrames(MovingObject* pCorObject)
{
	
	MovingObject* pObject1 = this;
	MovingObject* pObject2 = pCorObject;
	MovingObject* pObject = NULL;

	vector<UInt> picVectors1, picVectors2;
	UInt picNum = 0;

	UInt turn = 1;
	while (turn <= 2)
	{
		//cout << "ppppic pushing back" << endl;
		if (turn == 1)
			pObject = pObject1;
		else if (turn == 2)
			pObject = pObject2;
		else
		{
			cout << "the turn is not set right" << endl;
			exit(0);
		}

		UInt headPicNum = pObject->getHeadPointer()->getCurPic()->getPicNum();
		UInt tailPicNum = pObject->getTailPointer()->getCurPic()->getPicNum();
		VideoData* pCurVideo = pObject->getHeadPointer()->getCurPic()->getCurVideoData();
		PicData* pCurPic;
		while (headPicNum <= tailPicNum)
		{
			pCurPic = pCurVideo->getPic(headPicNum - 1);

			//ObjectCandidate* pCurObjCand;
			auto beg = pCurPic->getObjects().begin();
			while (beg != pCurPic->getObjects().end())
			{
				if ((*beg)->getMovingOjbect() == pObject)
				{
					picNum = (*beg)->getPicNum();

					if (turn == 1)
						picVectors1.push_back(picNum);
					if (turn == 2)
						picVectors2.push_back(picNum);


				}
				++beg;
			}

			++headPicNum;
		}

		turn++;
	}

	sort(picVectors1.begin(), picVectors1.end());
	sort(picVectors2.begin(), picVectors2.end());

	//calculate the same numbers in the two vectors
	UInt num1 = 0;
	UInt num2 = 0;


	auto beg = picVectors1.begin();
	while (beg != picVectors1.end())
	{
		if ((beg + 1) == picVectors1.end())
			break;
		num1 = *beg, num2 = *(beg + 1);
		if (num1 == num2)
		{
			beg=picVectors1.erase(beg);
			continue;
		}
		beg++;
	}

	auto beg2 = picVectors2.begin();
	while (beg2 != picVectors2.end())
	{
		if ((beg2 + 1) == picVectors2.end())
			break;
		num1 = *beg2, num2 = *(beg2 + 1);
		if (num1 == num2)
		{
			beg2 = picVectors2.erase(beg2);
			continue;
		}
		beg2++;
	}

	UInt overlapNum = 0;


	beg = picVectors1.begin();
	 beg2 = picVectors2.begin();
	while (beg != picVectors1.end() && beg2 != picVectors2.end())
	{
		num1 = *beg;
		num2 = *beg2;
		if (num1 == num2)
		{
			overlapNum++;
			beg++; 
			beg2++;
			
		}
		else

		if (num1 < num2)
			beg++;
		else
			beg2++;

	}

	//cout << "end checking overlap" << endl;
	/*UInt overlapNum = 0;
	for (UInt i = 0; i < num1; i++)
	{
		cout << "calculateing overlapping" << endl;
		for (UInt j = 0; j < num2; j++)
		if (picVectors1[i] == picVectors2[j])
		{
			overlapNum++;
			break;
		}
	}*/

	if (overlapNum <= 4)
		return true;
	else
		return false;
}


//
Bool MovingObject::removeShotMotion(UInt checkDir)
{
	Bool mergeFlag = false;
	Int curLen, priorLen, subseqLen;
	Int curDir, priorDir, subseqDir;
	std::pair<Int, Int> startAndEndIndex;
	std::pair<Int, Int>  stageAndLength;
	std::pair<std::pair<Int, Int>, std::pair<Int, Int>> dirInfo;
	std::vector<std::pair<std::pair<Int, Int>, std::pair<Int, Int>>> tempMotionInfo;
	auto curMotionIndex = m_mMotionStageAndLength[checkDir - 1].begin();
	auto priorMotionIndex = m_mMotionStageAndLength[checkDir - 1].begin();
	auto subseqMotionIndex = m_mMotionStageAndLength[checkDir - 1].begin();
	while (curMotionIndex != m_mMotionStageAndLength[checkDir - 1].end())
	{
		mergeFlag = false;
		priorMotionIndex = curMotionIndex;
		++curMotionIndex;
		if (curMotionIndex != m_mMotionStageAndLength[checkDir - 1].end())//next 
		{
			priorLen = priorMotionIndex->second.second;
			priorDir = priorMotionIndex->second.first;
			curLen = curMotionIndex->second.second;
			curDir = curMotionIndex->second.first;
			subseqMotionIndex = curMotionIndex;
			++subseqMotionIndex;
			if (subseqMotionIndex != m_mMotionStageAndLength[checkDir - 1].end())//three 
			{
				subseqLen = subseqMotionIndex->second.second;
				subseqDir = subseqMotionIndex->second.first;
				if (curDir == subseqDir)
				{
					dirInfo.first = priorMotionIndex->first;
					dirInfo.second = priorMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);

					dirInfo.first = curMotionIndex->first;
					dirInfo.second = curMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);

					curMotionIndex = subseqMotionIndex;
					continue;
				}
				if (curDir != priorDir && priorDir == subseqDir)
				{
					if ((curLen / (Float)(priorLen + subseqLen + curLen) < 0.25))
					//	||curLen<=3)
					{
						startAndEndIndex.first = priorMotionIndex->first.first;
						startAndEndIndex.second = subseqMotionIndex->first.second;
						stageAndLength.first = priorMotionIndex->second.first;
						stageAndLength.second = priorLen + curLen + subseqLen;
						dirInfo.first = startAndEndIndex;
						dirInfo.second = stageAndLength;
						tempMotionInfo.push_back(dirInfo);
						++subseqMotionIndex;
						mergeFlag = true;
						while (subseqMotionIndex != m_mMotionStageAndLength[checkDir - 1].end())
						{
							dirInfo.first = subseqMotionIndex->first;
							dirInfo.second = subseqMotionIndex->second;
							tempMotionInfo.push_back(dirInfo);
							++subseqMotionIndex;
						}
						break;
					}
					else
					{
						dirInfo.first = priorMotionIndex->first;
						dirInfo.second = priorMotionIndex->second;
						tempMotionInfo.push_back(dirInfo);
						continue;
					}
				}
				else
				{
					dirInfo.first = priorMotionIndex->first;
					dirInfo.second = priorMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);
					continue;
				}
			}
			else//two
			{
				if (curDir != priorDir && (curLen / (Float)(priorLen + curLen) < 0.25))
				{
						startAndEndIndex.first = priorMotionIndex->first.first;
						startAndEndIndex.second = curMotionIndex->first.second;
						stageAndLength.first = priorMotionIndex->second.first;
						stageAndLength.second = priorLen + curLen;
						dirInfo.first = startAndEndIndex;
						dirInfo.second = stageAndLength;
						tempMotionInfo.push_back(dirInfo);
				}
				else
				{
					dirInfo.first = priorMotionIndex->first;
					dirInfo.second = priorMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);

					dirInfo.first = curMotionIndex->first;
					dirInfo.second = curMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);
				}
				break;
			}
		}
		else
		{
			dirInfo.first = priorMotionIndex->first;
			dirInfo.second = priorMotionIndex->second;
			tempMotionInfo.push_back(dirInfo);
			break;
		}
	}
	if (tempMotionInfo.size()>0 && m_mMotionStageAndLength[checkDir - 1].size() > tempMotionInfo.size())// && mergeFlag)
	{
		m_mMotionStageAndLength[checkDir - 1] = tempMotionInfo;
		return true;
	}
	else
		return false;
}
//
Bool MovingObject::mergeShortMotion(UInt curBoundryIdx)
{
	Bool mergeFlag = false, smallStage = false, faultStage;
	Int curDirMotion;
	Int curLen, priorLen, subseqLen;
	Int curDir, priorDir, subseqDir;
	std::pair<Int, Int> startAndEndIndex;
	std::pair<Int, Int>  stageAndLength;
	std::pair<std::pair<Int, Int>, std::pair<Int, Int>> dirInfo;
	std::vector<std::pair<std::pair<Int, Int>, std::pair<Int, Int>>> tempMotionInfo;
	curDirMotion = ((curBoundryIdx % 2) == 0) ? 1 : -1;

	auto curMotionIndex = m_mMotionStageAndLength[curBoundryIdx - 1].begin();
	auto priorMotionIndex = m_mMotionStageAndLength[curBoundryIdx - 1].begin();
	auto subseqMotionIndex = m_mMotionStageAndLength[curBoundryIdx - 1].begin();
	while (curMotionIndex != m_mMotionStageAndLength[curBoundryIdx - 1].end())
	{
		mergeFlag = false;
		priorMotionIndex = curMotionIndex;
		++curMotionIndex;
		if (curMotionIndex != m_mMotionStageAndLength[curBoundryIdx - 1].end())//next 
		{
			priorLen = priorMotionIndex->second.second;
			priorDir = priorMotionIndex->second.first;
			curLen = curMotionIndex->second.second;
			curDir = curMotionIndex->second.first;
			subseqMotionIndex = curMotionIndex;
			++subseqMotionIndex;
			if (subseqMotionIndex != m_mMotionStageAndLength[curBoundryIdx - 1].end())//three 
			{
				subseqLen = subseqMotionIndex->second.second;
				subseqDir = subseqMotionIndex->second.first;
				if (false)//(curDir == subseqDir )
				{
					dirInfo.first = priorMotionIndex->first;
					dirInfo.second = priorMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);

					dirInfo.first = curMotionIndex->first;
					dirInfo.second = curMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);

					curMotionIndex = subseqMotionIndex;
					continue;
				}
				if (curDir!=subseqDir)//(curDir != priorDir && priorDir == subseqDir)
				{
					faultStage = curDir == curDirMotion ? false : true;
					if (faultStage == false)
					{
						if (curLen <= 3 && priorLen > 3 && subseqLen > 3)
							faultStage = true;
					}
					smallStage = ((curLen / (Float)(priorLen + subseqLen + curLen) < 0.25) && faultStage) ? true : false;
				//	faultMotion = (curDir != curDirMotion && curLen <= 4) ? true : false;
					
					if (smallStage )//||faultMotion)
					{
						startAndEndIndex.first = priorMotionIndex->first.first;
						startAndEndIndex.second = subseqMotionIndex->first.second;
						stageAndLength.first = priorMotionIndex->second.first;
						stageAndLength.second = priorLen + curLen + subseqLen;
						dirInfo.first = startAndEndIndex;
						dirInfo.second = stageAndLength;
						tempMotionInfo.push_back(dirInfo);
						++subseqMotionIndex;
						mergeFlag = true;
						while (subseqMotionIndex != m_mMotionStageAndLength[curBoundryIdx - 1].end())
						{
							dirInfo.first = subseqMotionIndex->first;
							dirInfo.second = subseqMotionIndex->second;
							tempMotionInfo.push_back(dirInfo);
							++subseqMotionIndex;
						}
						break;
					}
					else
					{
						dirInfo.first = priorMotionIndex->first;
						dirInfo.second = priorMotionIndex->second;
						tempMotionInfo.push_back(dirInfo);
						continue;
					}
				}
				else
				{
					dirInfo.first = priorMotionIndex->first;
					dirInfo.second = priorMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);
					continue;
				}
			}
			else//two
			{
				if (curDir != priorDir && (curLen / (Float)(priorLen + curLen) < 0.25))
				{
					startAndEndIndex.first = priorMotionIndex->first.first;
					startAndEndIndex.second = curMotionIndex->first.second;
					stageAndLength.first = priorMotionIndex->second.first;
					stageAndLength.second = priorLen + curLen;
					dirInfo.first = startAndEndIndex;
					dirInfo.second = stageAndLength;
					tempMotionInfo.push_back(dirInfo);
				}
				else
				{
					dirInfo.first = priorMotionIndex->first;
					dirInfo.second = priorMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);

					dirInfo.first = curMotionIndex->first;
					dirInfo.second = curMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);
				}
				break;
			}
		}
		else
		{
			dirInfo.first = priorMotionIndex->first;
			dirInfo.second = priorMotionIndex->second;
			tempMotionInfo.push_back(dirInfo);
			break;
		}
	}
	if (tempMotionInfo.size() > 0 && m_mMotionStageAndLength[curBoundryIdx - 1].size() > tempMotionInfo.size())// && mergeFlag)
	{
		m_mMotionStageAndLength[curBoundryIdx - 1] = tempMotionInfo;
		setMotionStageState(curBoundryIdx, true);
		return true;
	}
	else
		return false;

}
//
Void MovingObject::setMotionStageState(UInt curBoundryIdx, Bool resetFlag)
{
	Int curStageMotionDir, priorStageMotionDir;// , subseqStageMotionDir, curBoundryDir;
	auto stageLenIter = m_mMotionStageAndLength[curBoundryIdx - 1].begin();
	auto priorStageLenIter = m_mMotionStageAndLength[curBoundryIdx - 1].begin();
	if (resetFlag)
	{
		std::pair<Bool, Bool> stageState;
		std::pair<std::pair<Int, Int>, std::pair<Bool, Bool>> stageAndState;
		stageState.first = true;
		stageState.second = true;

		m_mMotionStageAndState[curBoundryIdx - 1].clear();
		while (stageLenIter != m_mMotionStageAndLength[curBoundryIdx - 1].end())
		{
			stageAndState.first = stageLenIter->first;
			stageAndState.second = stageState;
			m_mMotionStageAndState[curBoundryIdx - 1].push_back(stageAndState);
			++stageLenIter;
		}
	}
	auto currentStateIndex = m_mMotionStageAndState[curBoundryIdx - 1].begin();
	auto priorStateIndex = m_mMotionStageAndState[curBoundryIdx - 1].begin();
//	auto subseqStateIndex = m_mMotionStageAndState[curBoundryIdx - 1].begin();
	stageLenIter = priorStageLenIter;
	currentStateIndex->second.second = false;
	currentStateIndex->second.first = false;
	while (currentStateIndex != m_mMotionStageAndState[curBoundryIdx - 1].end())
	{
		priorStateIndex = currentStateIndex;
		priorStageLenIter = stageLenIter;

		++currentStateIndex;
		++stageLenIter;

		if (currentStateIndex != m_mMotionStageAndState[curBoundryIdx - 1].end())
		{
			priorStageMotionDir = priorStageLenIter->second.first;
			curStageMotionDir = stageLenIter->second.first;
			if (priorStageMotionDir != curStageMotionDir)
			{
				priorStateIndex->second.second = false;
				currentStateIndex->second.first = false;
			}
		}
		else
			break;
	}
}
//
Bool MovingObject::removeBoundryShortMotion(UInt boundryIdx)
{
	Bool mergeFlag = false;
	UInt boudryIdx = 1;
	Int curLen, priorLen, subseqLen;
	std::pair<Int, Int> stageInterval;
	std::pair<Int, Int> stageDirAndLen;
	std::pair<Bool, Bool> stageAndState;
	std::pair<std::pair<Int, Int>, std::pair<Int, Int>> stageIntervalAndLen;
	std::pair<std::pair<Int, Int>, std::pair<Bool, Bool>> stageIntervalAndState;
	std::vector<std::pair<std::pair<Int, Int>, std::pair<Int, Int>>> tempStageIntervalAndLen;
	std::vector<std::pair<std::pair<Int, Int>, std::pair<Bool, Bool>>> tempStageIntervalAndState;
	auto curLenIter = m_mMotionStageAndLength[boundryIdx - 1].begin();
	auto priLenIter = m_mMotionStageAndLength[boundryIdx - 1].begin();
	auto subLenIter = m_mMotionStageAndLength[boundryIdx - 1].begin();
	
	auto curStateIter = m_mMotionStageAndState[boundryIdx - 1].begin();
	auto priStateIter = m_mMotionStageAndState[boundryIdx - 1].begin();
	auto subStateIter = m_mMotionStageAndState[boundryIdx - 1].begin();

	while (curStateIter != m_mMotionStageAndState[boundryIdx - 1].end())
	{
		priStateIter = curStateIter;
		priLenIter = curLenIter;

		++curStateIter;
		++curLenIter;

		if (curStateIter != m_mMotionStageAndState[boundryIdx - 1].end())
		{
			subStateIter = curStateIter;
			subLenIter = curLenIter;

			++subStateIter;
			++subLenIter;

			curLen = curLenIter->second.second;
			priorLen = priLenIter->second.second;

			if (subStateIter != m_mMotionStageAndState[boundryIdx - 1].end())
			{
				subseqLen = subLenIter->second.second;

				if (curStateIter->second.first == false && curStateIter->second.second == false
					&& priStateIter->second.first == true && subStateIter->second.second == true)
				{
					if (curLenIter->second.second <= 3 && ((Float)curLen / (priorLen + curLen + subseqLen) < 0.75))
					{
						//merge them 
						stageInterval.first = priLenIter->first.first;
						stageInterval.second = subLenIter->first.second;

						stageDirAndLen.first = priLenIter->second.first;
						stageDirAndLen.second = priorLen + curLen + subseqLen;

						stageIntervalAndLen.first = stageInterval;
						stageIntervalAndLen.second = stageDirAndLen;

						tempStageIntervalAndLen.push_back(stageIntervalAndLen);
						++subLenIter;
						while (subLenIter != m_mMotionStageAndLength[boundryIdx - 1].end())
						{
							stageIntervalAndLen.first = subLenIter->first;
							stageIntervalAndLen.second = subLenIter->second;

							tempStageIntervalAndLen.push_back(stageIntervalAndLen);
							++subLenIter;
						}
						mergeFlag = true;
						break;
					}
					else
					{
						stageIntervalAndLen.first = priLenIter->first;
						stageIntervalAndLen.second = priLenIter->second;
						tempStageIntervalAndLen.push_back(stageIntervalAndLen);
						continue;
					}

				}
				else
				{
					stageIntervalAndLen.first = priLenIter->first;
					stageIntervalAndLen.second = priLenIter->second;
					tempStageIntervalAndLen.push_back(stageIntervalAndLen);
					continue;
				}
			}
			else
			{
				stageIntervalAndLen.first = priLenIter->first;
				stageIntervalAndLen.second = priLenIter->second;
				tempStageIntervalAndLen.push_back(stageIntervalAndLen);
				continue;
			}
		}
		else
		{
			stageIntervalAndLen.first = priLenIter->first;
			stageIntervalAndLen.second = priLenIter->second;
			tempStageIntervalAndLen.push_back(stageIntervalAndLen);
			break;
		}
	}
	if (tempStageIntervalAndLen.size() > 0 && m_mMotionStageAndLength[boundryIdx - 1].size() > tempStageIntervalAndLen.size())
	{
		m_mMotionStageAndLength[boundryIdx - 1] = tempStageIntervalAndLen;
		setMotionStageState(boundryIdx, true);
		return true;
	}
	else
		return false;
}
//
//Bool curStageUntrustful(std::vector<Int> &stageLen, std::vector<Int> &stageDir, UInt stageIndex, Int &mergeDir)
//{
//	Int curDir, firDir;
//	Int maxLen, minLen, firLen, secLen, aveLen1, aveLen2, priLen, curLen, subLen;
//	if (stageIndex < 3)
//		return false;
//	curDir = stageDir[stageIndex];
//	//curLen = stageLen[stageIndex];
//	firDir = stageDir[stageIndex - 2];
//	if (curDir != firDir)
//		return false;
//	firLen = stageLen[stageIndex - 3];
//	firLen = curLen;
//	secLen = stageLen[stageIndex - 2];
//	maxLen = firLen > secLen ? firLen : secLen;
//	minLen = firLen > secLen ? secLen : firLen;
//	priLen = stageLen[stageIndex - 1];
//	maxLen = maxLen > priLen ? maxLen : priLen;
//	minLen = minLen > priLen ? priLen : minLen;
//	subLen = stageLen[stageIndex + 1];
//	aveLen1 = subLen + curLen + priLen;
//	aveLen2 = priLen + secLen + curLen;
//	if ((abs(aveLen1 - firLen) + abs(aveLen1 - subLen)) > (abs(aveLen2 - firLen) + abs(aveLen2 - secLen)))
//		mergeDir = 1;
//	else
//		mergeDir = -1;
//	return true;
//}
//
//Bool MovingObject::boudryMotionFiter(UInt curBoundryIdx)
//{
//	Bool trustfulFlag;
//	Int curStageLen, curStageDir, mergeDir;
//	UInt  stageIndex, searchIndex;
//	std::vector<Int> stageAndLen;
//	std::vector<Int> stageAndDir;
//	auto curStageLenIter = m_mMotionStageAndLength[curBoundryIdx - 1].begin();
//	auto subStageLenIter = m_mMotionStageAndLength[curBoundryIdx - 1].begin();
//	while (curStageLenIter != m_mMotionStageAndLength[curBoundryIdx - 1].end())
//	{
//		curStageDir = curStageLenIter->second.first;
//		curStageLen = curStageLenIter->second.second;
//		stageAndLen.push_back(curStageLen);
//		++curStageLenIter;
//	}
//	stageIndex = 0;
//	auto curStageStateIter = m_mMotionStageAndState[curBoundryIdx - 1].begin();
//	auto priStageStateIter = m_mMotionStageAndState[curBoundryIdx - 1].begin();
//	auto subStageStateIter = m_mMotionStageAndState[curBoundryIdx - 1].begin();
//
//	while (curStageStateIter != m_mMotionStageAndState[curBoundryIdx - 1].end())
//	{
//		priStageStateIter = curStageStateIter;
//		++curStageStateIter;
//		++stageIndex;
//		if (curStageStateIter != m_mMotionStageAndState[curBoundryIdx - 1].end())
//		{
//			subStageStateIter = curStageStateIter;
//			++subStageStateIter;
//			++stageIndex;
//			if (subStageStateIter != m_mMotionStageAndState[curBoundryIdx - 1].end())
//			{
//				if (curStageStateIter->second.first == false && subStageStateIter->second.second == true)
//				{
//					trustfulFlag = curStageUntrustful(stageAndLen, stageAndDir, stageIndex, mergeDir);
//					if (trustfulFlag)
//					{
//						searchIndex = stageIndex;
//						curStageLenIter = m_mMotionStageAndLength[curBoundryIdx - 1].begin();
//						if (mergeDir == -1)//merge cur with left and right
//							searchIndex = stageIndex - 1;
//						if (mergeDir == 1)//merge prior whit its left and right
//							searchIndex = stageIndex - 2;
//						stageAndLen[searchIndex] = stageAndLen[searchIndex] + stageAndLen[searchIndex+1] + stageAndLen[searchIndex+2];
//						curStageLen = stageAndLen[searchIndex];
//						while (searchIndex > 0)
//						{
//							++curStageLenIter;
//							--searchIndex;
//						}
//						subStageLenIter = curStageLenIter;
//						++subStageLenIter;
//						subStageLenIter->second.second = -1;
//						++subStageLenIter;
//						subStageLenIter->second.second = -1;
//						curStageLenIter->second.second = curStageLen;
//						curStageLenIter->first.second = subStageLenIter->first.second;
//						curStageLenIter = m_mMotionStageAndLength[curBoundryIdx - 1].begin();
//						while (curStageLenIter != m_mMotionStageAndLength[curBoundryIdx - 1].end())
//						{
//							if (curStageLenIter->second.second == -1)
//							{
//								curStageLenIter = m_mMotionStageAndLength[curBoundryIdx - 1].erase(curStageLenIter);
//								continue;
//							}
//							++curStageLenIter;
//						}
//						setMotionStageState(curBoundryIdx, true);
//						return true;
//					}
//					else
//						continue;
//				}
//			}
//			else
//				break;//will be add some 
//		}
//		else
//			break;
//	}
//	return false;
//}
//

//
Bool MovingObject::removeShotStage(UInt checkDir)
{
	Int curLen, priorLen, subseqLen;
	Int curDir, priorDir, subseqDir;
	std::pair<Int, Int> startAndEndIndex;
	std::pair<Int, Int>  stageAndLength;
	std::pair<std::pair<Int, Int>, std::pair<Int, Int>> dirInfo;
	std::vector<std::pair<std::pair<Int, Int>, std::pair<Int, Int>>> tempMotionInfo;
	auto curMotionIndex = m_mMotionStageAndLength[checkDir - 1].begin();
	auto priorMotionIndex = m_mMotionStageAndLength[checkDir - 1].begin();
	auto subseqMotionIndex = m_mMotionStageAndLength[checkDir - 1].begin();
	while (curMotionIndex != m_mMotionStageAndLength[checkDir - 1].end())
	{
		priorMotionIndex = curMotionIndex;
		++curMotionIndex;
		if (curMotionIndex != m_mMotionStageAndLength[checkDir - 1].end())//next 
		{
			priorLen = priorMotionIndex->second.second;
			priorDir = priorMotionIndex->second.first;
			curLen = curMotionIndex->second.second;
			curDir = curMotionIndex->second.first;
			subseqMotionIndex = curMotionIndex;
			++subseqMotionIndex;
			if (subseqMotionIndex != m_mMotionStageAndLength[checkDir - 1].end())//three 
			{
				subseqLen = subseqMotionIndex->second.second;
				subseqDir = subseqMotionIndex->second.first;
				if ((priorDir != subseqDir) && (curLen<=3))
				{
					if (
						(curLen / (Float)(priorLen + subseqLen + curLen) < 0.25)
						)
					{
						startAndEndIndex.first = priorMotionIndex->first.first;
						startAndEndIndex.second = subseqMotionIndex->first.second;

						stageAndLength.first = priorMotionIndex->second.first;
						stageAndLength.second = priorLen + curLen + subseqLen;

						dirInfo.first = startAndEndIndex;
						dirInfo.second = stageAndLength;
						tempMotionInfo.push_back(dirInfo);
						++subseqMotionIndex;
						while (subseqMotionIndex != m_mMotionStageAndLength[checkDir - 1].end())
						{
							dirInfo.first = subseqMotionIndex->first;
							dirInfo.second = subseqMotionIndex->second;
							tempMotionInfo.push_back(dirInfo);
							++subseqMotionIndex;
						}
						break;
					}
					else
					{
						dirInfo.first = priorMotionIndex->first;
						dirInfo.second = priorMotionIndex->second;
						tempMotionInfo.push_back(dirInfo);
						continue;
					}
				}
				else
				{
					dirInfo.first = priorMotionIndex->first;
					dirInfo.second = priorMotionIndex->second;
					tempMotionInfo.push_back(dirInfo);
					continue;
				}
			}
			else//two
			{
				dirInfo.first = priorMotionIndex->first;
				dirInfo.second = priorMotionIndex->second;
				tempMotionInfo.push_back(dirInfo);

				dirInfo.first = curMotionIndex->first;
				dirInfo.second = curMotionIndex->second;
				tempMotionInfo.push_back(dirInfo);
				break;
			}
		}
		else
			break;
	}
	if (tempMotionInfo.size() > 0 && m_mMotionStageAndLength[checkDir - 1].size() > tempMotionInfo.size())
	{
		m_mMotionStageAndLength[checkDir - 1] = tempMotionInfo;
		return true;
	}
	else
		return false;
}
//
Void MovingObject::setBoundryMotionValue(UInt checkDir)
{
	UInt headPicIdx, tailPicIdx;
	Int motionDir, curDirBoundry, priorDirBoundry, motionVal;
	curDirBoundry = motionDir = motionVal = 0;
	auto dirMotionIdx = m_mDirAndMotion.begin();
	auto motionStageIdx = m_mMotionStageAndLength[checkDir - 1].begin();
	priorDirBoundry = -1;

	while (motionStageIdx != m_mMotionStageAndLength[checkDir - 1].end())
	{
		motionDir = motionStageIdx->second.first;
		headPicIdx = motionStageIdx->first.first;
		tailPicIdx = motionStageIdx->first.second;
		while (dirMotionIdx->first->getPicNum() != headPicIdx)
		{
			++dirMotionIdx;
		}
		if (priorDirBoundry < 0)
			priorDirBoundry = dirMotionIdx->first->getEdgeWithDir(checkDir);
		else
			priorDirBoundry = curDirBoundry;
		curDirBoundry = dirMotionIdx->first->getEdgeWithDir(checkDir);
		motionVal += curDirBoundry - priorDirBoundry;
		while (dirMotionIdx != m_mDirAndMotion.end() && dirMotionIdx->first->getPicNum() <= tailPicIdx)
		{
			dirMotionIdx->second[checkDir - 1].first = true;
			dirMotionIdx->second[checkDir - 1].second = motionVal;
			++dirMotionIdx;
		}
		++motionStageIdx;
	}
}

//
Void MovingObject::setDirectionMotionValue()
{
}


Void MovingObject::resetUseFulFlag()
{
	UInt curPicNum = m_pcHeadPointer->getCurPic()->getPicNum();
	UInt endPicNum = m_pcTailPointer->getCurPic()->getPicNum();
	UInt curObjNum = 1, curGroupNum = 1, lastGroupNum = 0, stride = 0, nextNum = 0;
	UInt curLeftEdge, curUpEdge, curRigEdge, curBotEdge;// , corLeftEdge, corUpEdge, corRigEdge, corBotEdge;
	ObjectCandidate *pTempCand, *latestCand;
	ObjectCandidate *pFirCand, *pSecCand;// , *pThirdCand, *pNextCand, *pPriorCand;
	std::vector<ObjectCandidate*> useFullCand;
	std::vector<ObjectCandidate*> groupCand;
	std::map<ObjectCandidate*, std::pair<Int, Int>> candAndDelta;
	//to record each edges and difference with prior 
	std::map<ObjectCandidate*, std::pair<Int, Bool>> leftEdgeAndDelt;
	std::map<ObjectCandidate*, std::pair<Int, Bool>> upEdgeAndDelt;
	std::map<ObjectCandidate*, std::pair<Int, Bool>> rigEdgeAndDelt;
	std::map<ObjectCandidate*, std::pair<Int, Bool>> botEdgeAndDelt;
	VideoData* pCurVideo = m_pcHeadPointer->getCurPic()->getCurVideoData();
	PicData* pCurPic;
//	Float padPxl;
	Bool matchFlag = false, trueEdgeFir = false, trueEdgeSec = false;
	auto picIdxBeg = m_mPicIndex.begin();
	while (picIdxBeg != m_mPicIndex.end())
	{
		curPicNum = picIdxBeg->first;
		curObjNum = picIdxBeg->second;
		pCurPic = pCurVideo->getPic(curPicNum - 1);
		pTempCand = pCurPic->getObjectWithIdx(curObjNum - 1);
		if (pTempCand->getUsefulFlag() && pTempCand->getGroupNum() != 0)
		{
			pTempCand->resetRelationWithCorCand();
			useFullCand.push_back(pTempCand);
		}
		++picIdxBeg;
	}
	latestCand = NULL;
	auto usefulCandBeg = useFullCand.begin();
	while (usefulCandBeg != useFullCand.end())
	{
		matchFlag = false;
		pFirCand = *usefulCandBeg;
		if (latestCand != pFirCand)
		{
			pFirCand->getObjEdge(curLeftEdge, curUpEdge, curRigEdge, curBotEdge, false);
			leftEdgeAndDelt[pFirCand].first = curLeftEdge;
			upEdgeAndDelt[pFirCand].first = curUpEdge;
			rigEdgeAndDelt[pFirCand].first = curRigEdge;
			botEdgeAndDelt[pFirCand].first = curBotEdge;
		}
		++usefulCandBeg;
		if (usefulCandBeg != useFullCand.end())
		{
			pSecCand = *usefulCandBeg;
			pFirCand->searchMaxCommonArea(pSecCand, matchFlag);
			if (matchFlag == true)
			{
				pFirCand->setNextMatch(true);
				pFirCand->setNextMatchCand(pSecCand);
				pSecCand->setPriorMatch(true);
				pSecCand->setPriorMatchCand(pFirCand);
			}
			else
			{
				if (pFirCand->getPriorMatch() == false)
					pFirCand->setUsefulFlag(false);
			}
		}
		else
		{
			if (pFirCand->getPriorMatch() == false)
				pFirCand->setUsefulFlag(false);
		}
		latestCand = pFirCand;
	}
}


ObjectCandidate* MovingObject::findNearestPriorSplitCand(MovingObject*& neighMovingObj)
{
	ObjectCandidate* nearestSplitCand = NULL;
	ObjectCandidate* neighCand = NULL;
	Bool stopSearch = false;
	cout << "begin finding nearest split candidate " << endl;
	PicData* curPic;
	UInt picNum;
	while (stopSearch == false)
	{
		curPic = m_pcHeadPointer->getCurPic();
		std::map<UInt, UInt> candidatesArea;
		UInt area = 0;
		//m_pcHeadPointer->getCurPic()->getCurVideoData()->getMovObjMap();

		while (curPic->getPicNum() <= m_pcTailPointer->getCurPic()->getPicNum())
		{
			picNum = curPic->getPicNum();
			area = curPic->getObjectArea(this);
			if (area > 0)
			{
				candidatesArea.insert(make_pair(curPic->getPicNum(), area));
				//cout << "the pic num is " << curPic->getPicNum() << " the area is " << area << endl;
			}
			curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(picNum);
			if (curPic == NULL)
				break;
		}

		if (candidatesArea.size() < 8)
			return NULL;

		std::map<UInt, UInt> group1, group2;

		UInt score1 = 0;
		UInt score2 = 0;
		Float ratio1 = 0;
		Float ratio2 = 0;

		auto beg = candidatesArea.begin();
		auto beg2 = candidatesArea.begin();

		curPic = m_pcHeadPointer->getCurPic();
		UInt leftDir = 0, upDir = 0, rigDir = 0, botDir = 0;

		while (beg != candidatesArea.end())
		{
			score1 = 0;
			score2 = 0;
			group1.clear(); group2.clear();
			beg2 = beg;
			for (UInt i = 0; i < 4; i++)
			{
				group1.insert(make_pair(beg2->first, beg2->second));
				beg2++;
			}

			for (UInt i = 0; i < 4; i++)
			{
				group2.insert(make_pair(beg2->first, beg2->second));
				beg2++;
			}

			auto groupBeg1 = group1.begin();
			auto groupBeg2 = group2.begin();

			while (groupBeg1 != group1.end())
			{
				groupBeg2 = group2.begin();
				while (groupBeg2 != group2.end())
				{

					if ((groupBeg1->second + 3) <= (groupBeg2->second))
					{
						//cout << "score1++" << endl;
						score1++;
					}
					if (groupBeg1->second <= groupBeg2->second)
						score2++;
					groupBeg2++;
				}
				groupBeg1++;
			}

			ratio1 = Float(score1) / Float(16);
			ratio2 = Float(score2) / Float(16);

			groupBeg1 = group1.end();
			groupBeg1--;
			if ((ratio1 >= 0.8) && (ratio2 >= 1) && ((groupBeg1->second + 3) <= group2.begin()->second))
			{
				ObjectCandidate* pCurObj = NULL;
				curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(group2.begin()->first - 1);
				std::vector<ObjectCandidate*> picObjects = curPic->getObjects();
				auto begObj = picObjects.begin();
				while (begObj != picObjects.end())
				{
					if ((*begObj)->getMovingOjbect() == this)
					{
						pCurObj = *begObj;
						Bool findFlag = false;
						ObjectCandidate* priorCorCand = pCurObj->findCorCand(groupBeg1->first - group2.begin()->first - 1, findFlag);
						cout << "preparing adding one" << endl;
						cout << "the pic num is " << group2.begin()->first << endl;
						
						if (priorCorCand == NULL)
							findFlag = false;
						else
							findFlag = true;
						//int aa; cin >> aa;

						if (findFlag)
						{
							cout << "added" << endl;
							cout << "the pic num is " << group2.begin()->first << endl;
							//int aa; cin >> aa;
							nearestSplitCand = pCurObj;
							UInt curCtuSize = pCurObj->getCtuElements().size();
							UInt curCtuSize2 = priorCorCand->getCtuElements().size();
							Bool objectMatch = false;
							Bool realMatch = false;
							Float ratio1 = 0, ratio2 = 0;
							objectMatch = pCurObj->calculateBoundingBoxOverlaps(priorCorCand, ratio1, ratio2);
							//if (objectMatch&&curCtuSize > curCtuSize2)
							if (objectMatch		&&	curCtuSize > curCtuSize2)
								realMatch = true;

							if (realMatch)
							{
								neighCand = priorCorCand;
								neighMovingObj = priorCorCand->getMovingOjbect();
							}
							else
							{
								UInt pubArea = pCurObj->findPubCtu(priorCorCand);
								if (pubArea != 0)
								{
									/*cout << "the pCurObj pic num is " << pCurObj->getPicNum() << endl;
									cout << "the  pCurObj  ctu num is " << pCurObj->getCtuElements().size() << endl;
									cout << "the corCand pic num is " << priorCorCand->getPicNum() << endl;
									cout << "the corCand ctu size is " << priorCorCand->getCtuElements().size() << endl;
									cout << "the pubArea is " << pubArea << endl;*/
									
									UInt MovingObjNum = this->getHeadPointer()->getCurPic()->getCurVideoData()->getInitialNum();
									neighMovingObj = new MovingObject(MovingObjNum);
									getHeadPointer()->getCurPic()->getCurVideoData()->increaseInitialNum();

									//produce candidates for the new object
									PicData* currPic = priorCorCand->getCurPic();
									UInt totalNum = 1;
									ObjectCandidate* newObjCand = NULL;

									UInt leftEdge_Large, upEdge_Large, rightEdge_Large, botEdge_Large = 0;
									
									priorCorCand->getObjEdge(leftEdge_Large, upEdge_Large, rightEdge_Large, botEdge_Large, true);
									Bool splitSuccess = false;
									//while (currPic->getPicNum()>=)
									//while (totalNum<=20)
									UInt headNum = priorCorCand->getMovingOjbect()->getHeadPointer()->getPicNum();
									while (currPic->getPicNum()>headNum&&totalNum<20)
									{
										UInt picNum = currPic->getPicNum();

										cout << "boundinggg boxxing" << endl;
										cout << "finish getting edge" << endl;
										if (splitSuccess == false)
										{
											splitSuccess = currPic->splitObj1AccordingToCand(priorCorCand->getMovingOjbect(), neighMovingObj, pCurObj, newObjCand);
											splitSuccess = false;
										}
										else
										{
											if (newObjCand == NULL)
												cout << "there is a NULL newObjCand" << endl;
											cout << "ready bounding box" << endl;
											currPic->splitObj1AccordingToBoundingBox(priorCorCand->getMovingOjbect(), neighMovingObj, newObjCand, leftEdge_Large, upEdge_Large, rightEdge_Large, botEdge_Large);
										}
										currPic=currPic->getCurVideoData()->getPic(picNum-2);
										totalNum++;
									}
									neighMovingObj->resetHeadPointer();
									neighMovingObj->resetTailPointer();
									MovingObject * priorObj = priorCorCand->getMovingOjbect();
									priorObj->findPriorObjAndMerge(neighMovingObj);

								}


							}

							break;
						}

					}
					++begObj;
				}
			}

			if (beg2 == candidatesArea.end())
			{
				break;
			}
			beg++;
		}

		if (nearestSplitCand == NULL)
		{
			this->resetHeadPointer();
			this->resetTailPointer();
			std::map<MovingObject*, Bool> priorObj = this->getPriorMovObj();
			if (priorObj.size() < 1)
				stopSearch = true;
			else
			if (priorObj.size() ==1)
			{
				auto beg = priorObj.begin();
				ObjectCandidate* head = NULL;
				ObjectCandidate* tail = NULL;
				MovingObject* pTempObj = NULL;
				pTempObj = beg->first;
				tail = pTempObj->getTailPointer();
				head = this->getHeadPointer();
				Float ratio1 = 0, ratio2 = 0;
				Bool matchFlag2 = false;
				matchFlag2 = head->calculateBoundingBoxOverlaps(tail, ratio1, ratio2);
				if (matchFlag2)
				{
					pTempObj->resetElement(this);
				}
				else
					stopSearch = true;
			}
			else
			{
				MovingObject* pFirst = NULL, *pSecond = NULL;
				MovingObject* pTempObj = NULL;
				ObjectCandidate* head = NULL;
				ObjectCandidate* tail = NULL;
				auto beg = priorObj.begin();
				while (beg != priorObj.end())
				{
					pTempObj = beg->first;
					tail = pTempObj->getTailPointer();
					head = this->getHeadPointer();
					Float ratio1 = 0, ratio2 = 0;
					Bool matchFlag2 = false;
					matchFlag2 = head->calculateBoundingBoxOverlaps(tail, ratio1, ratio2);
					if (matchFlag2)
					{
						if (pFirst == NULL)
							pFirst = pTempObj;
						else
						if (pSecond == NULL)
							pSecond = pTempObj;
					}

					beg++;
				}

				if (pFirst != NULL || pSecond != NULL)
				{
					if (pFirst != NULL)
						pFirst->resetElement(this);
					else
						pSecond->resetElement(this);
				}
				else
					stopSearch = true;
			}
		}
		else
			stopSearch = true;
	}
	return nearestSplitCand;

}

MovingObject* MovingObject::newAObjectFromHead(ObjectCandidate* pCand)
{
	ObjectCandidate* headCand = this->getHeadPointer();
	ObjectCandidate* tailCand = this->getTailPointer();
	UInt headPicNum = headCand->getPicNum();
	UInt tailPicNum = tailCand->getPicNum();
	UInt currPicNum = pCand->getPicNum();
	if (pCand->getPicNum() > tailPicNum)
	{
		/*cout << "> tailPicNum" << endl;
		cout << "pCand->getPicNum() is " << pCand->getPicNum() << endl;
		cout << "headCand->getPicNum() is " << headCand->getPicNum() << endl;
		cout << "tailCand->getPicNum() is " << tailCand->getPicNum() << endl;*/
		return NULL;
	}
	
	
	//newObj->setHeadPointer(headCand);

	if (m_uiMovObjectNum == 0) //it is not reasonale to merge a number 0 object with another object
	{
		cout << "object num is 0" << endl;
		return NULL;
	}

	cout << "debug1 " << endl;
	PicData* curPic;
	UInt picNum, newMovNum;
	std::vector<ObjectCandidate*> cand;
	newMovNum = getHeadPointer()->getCurPic()->getCurVideoData()->getInitialNum();
	getHeadPointer()->getCurPic()->getCurVideoData()->increaseInitialNum();
	MovingObject* newObj = new MovingObject(newMovNum);

	m_pcHeadPointer->getCurPic()->getCurVideoData()->addMovObjToMap(newObj);
	
	curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(currPicNum - 1);
	
	

	curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(currPicNum-1);

	while (curPic->getPicNum() <= m_pcTailPointer->getCurPic()->getPicNum())
	{
		picNum = curPic->getPicNum();
		curPic->resetMovingObj(this, newObj);//move element from first to second 
		curPic = m_pcHeadPointer->getCurPic()->getCurVideoData()->getPic(picNum);
		if (curPic == NULL)
		{
			break;
		}
	}

	//set the head and tail pointer
	newObj->resetHeadPointer();
	newObj->resetTailPointer();
	this->resetHeadPointer();
	this->resetTailPointer();

	cout << "debug2 " << endl;

	MovingObject* pDstMovingObj = NULL;
	

	cout << "debug3 " << endl;
	if (pCand->getPicNum() <= headPicNum)
	{
		;
		//m_uiMovObjectNum = 0;//m_uiMovObjectNum is the moving object number 
	}

	cout << "debug4 " << endl;
	newMovNum = m_pcHeadPointer->getCurPic()->getCurVideoData()->getInitialNum();
	m_pcHeadPointer->getCurPic()->getCurVideoData()->increaseInitialNum();

	newObj->setMovObjectNum(newMovNum);
	cout << "debug5 " << endl;
	return newObj;

}

//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*picture class*/
//Picture data class constructor
//parameters: frnNum: current frame number; 
//frawid: frame width
//frmhig: frame hegiht
//ctuwid ctuhig: CTU width and heigth
//depth: maximum split depth
PicData::PicData(UInt frmNum, UInt frmwid, UInt frmhig, UInt ctuwid, UInt ctuhig, UInt depth)
{
	UInt row, col;
	Int lenY = frmwid;


	row = frmwid / ctuwid + (frmwid % ctuwid? 1:0);
	col = frmhig / ctuhig + (frmhig%ctuhig?  1: 0);


	m_uiCtuInRow = row;
	m_uiCtuInCol = col;
	m_picWidth = frmwid;
	m_picHeght = frmhig;
	m_picSeqNum = frmNum;
	m_CtuHig = ctuhig;
	m_CtuWid = ctuwid;
	m_MaxDepth = depth;
	m_stride = row*ctuwid;
	m_ctus = row*col;
	m_PreType = MODE_NONE;
	for (UInt i = 0; i < col; i++)
	{
		for (UInt j = 0; j < row; j++)
		{
			UInt addr = i*row + j;
			CtuData* NewOne = new CtuData(addr, 0, ctuwid, ctuhig);
			NewOne->setPic(this);
			m_ctuElement.push_back(NewOne);
			if ((addr / m_uiCtuInRow == 0) || (addr%m_uiCtuInRow == 0)
				|| (addr / m_uiCtuInRow == (m_uiCtuInCol - 1))
				|| (addr%m_uiCtuInRow == (m_uiCtuInRow - 1)))
				NewOne->setBorderFlag(true);
		}
	}
}
PicData::PicData()
{
}
PicData::~PicData()
{

	m_curVideo = NULL;
	UInt ctuNumber = (UInt)m_ctuElement.size();
	for (UInt i = 0; i < ctuNumber; i++)
		delete m_ctuElement[i];
	m_vPxlElements.clear();
}
//
//Void PicData::setPxlElement()
//{
//	for (UInt i = 0; i < m_picHeght; i++)
//	{
//		std::vector<Int> pxlRow(m_picWidth, -1);
//		m_vPxlElements.push_back(pxlRow);
//	}
//}

bool PicData::findPredObject(UInt num)
{
	bool answer = false;
	for (int i = 0; i < predObjectVec.size(); i++)
	{
		if (num == predObjectVec[i]->getObjectNum())
			answer = true;
	}
	return answer;
}

void PicData::addPredObject(UInt num, UInt x, UInt y)
{
	predObject* pObject = new predObject(num);
	pObject->setPosition(x, y);
	pObject->setObjectNum(num);

	predObjectVec.push_back(pObject);
	
	
}

void PicData::getPredObjectPosition(UInt ObjectNum, UInt& beginX, UInt& beginY)
{
	bool exist = false;
	for (int i = 0; i < predObjectVec.size(); i++)
	{
		if (ObjectNum == predObjectVec[i]->getObjectNum())
		{
			predObjectVec[i]->getPosition(beginX, beginY);
			exist = true;
			break;
		}
	}

	if (exist == false)
	{
		cout << "getPredObjectPosition: try to find an object's potion which is not in the pic" << endl;
		exit(0);
	}
}

void PicData::setPredObjectX(UInt num, UInt x)
{
	bool exist = false;
	for (int i = 0; i < predObjectVec.size(); i++)
	{
		if (num == predObjectVec[i]->getObjectNum())
		{
			predObjectVec[i]->setXPosition(x);
			predObjectVec[i]->setXFlag(true);
			exist = true;
			break;
		}
	}

	if (exist == false)
	{
		cout << "try to find an object's X potion which is not in the pic" << endl;
		exit(0);
	}
}


void PicData::setPredObjectY(UInt num, UInt y)
{
	bool exist = false;
	for (int i = 0; i < predObjectVec.size(); i++)
	{
		if (num == predObjectVec[i]->getObjectNum())
		{
			predObjectVec[i]->setYPosition(y);
			predObjectVec[i]->setYFlag(true);
			exist = true;
			break;
		}
	}

	if (exist == false)
	{
		cout << "try to find an object's Y potion which is not in the pic" << endl;
		exit(0);
	}
}




void PicData::setPredObjectPosition(UInt ObjectNum, UInt X, UInt Y)
{
	bool exist = false;
	for (int i = 0; i < predObjectVec.size(); i++)
	{
		if (ObjectNum == predObjectVec[i]->getObjectNum())
		{
			predObjectVec[i]->setPosition(X, Y);
			exist = true;
			break;
		}
	}
	if (exist == false)
	{
		addPredObject(ObjectNum, X, Y);
	}
}


void PicData::checkObjectFlag(UInt ObjectNum, bool& setX, bool& setY)
{
	bool exist = false;
	int num = 0;
	for (int i = 0; i < predObjectVec.size(); i++)
	{
		if (ObjectNum == predObjectVec[i]->getObjectNum())
		{
			exist = true;
			num = i;
			break;
		}
	}
	if (exist == false)
	{
		cout << "try to check an object's potion which is not in the pic" << endl;
		exit(0);
	}
	predObjectVec[num]->getXFlag(setX);
	predObjectVec[num]->getYFlag(setY);
}




Void PicData::preSetCtuSta()
{
	CtuData *pCurCtu;
	for (auto beg = m_ctuElement.begin(); beg != m_ctuElement.end(); beg++)
	{
		pCurCtu = *beg;
		pCurCtu->classifyCtuStatus();
	}
}

Void  PicData::showCTUstatus()
{
	CtuData *pCurCtu;
	cout << "the picNum is " << endl;
	auto beg = m_ctuElement.begin();
	int i = 0;
	bool motion = false;
	while (beg != m_ctuElement.end())
	{
		if (i % 30 == 0)
			cout << "another"<<endl<<endl;
		pCurCtu = *beg;
		//motion=pCurCtu->getMovingFlag();
		UInt Num = 0;
		Num=pCurCtu->getCUNum();
		cout << Num << " ";
		beg++;
		
		i++;
	}
}


//
Void PicData::reSetCtuSta()
{
	auto beg = m_ctuElement.begin();
	while (beg != m_ctuElement.end())
	{
		(*beg)->possibleMotion();
		beg++;
	}
}
//
Void PicData::reDealCtuSta()
{
	auto beg = m_ctuElement.begin();
	while (beg != m_ctuElement.end())
	{
		(*beg)->reDealCtu();
		beg++;
	}
}



Void PicData::setBorderCtuStatus()
{
	UInt horUpIdx = 0, horBotIdx = 0, verLeftIdx = 0, verRigIdx = 0;
	UInt horStride = m_uiCtuInRow;
	UInt verStride = m_uiCtuInCol;
	horUpIdx = 0;
	verLeftIdx = 0;
	horBotIdx = (m_uiCtuInCol - 1)*m_uiCtuInRow;
	verRigIdx = m_uiCtuInRow - 1;
	if (m_picSeqNum == 13)
		int testPoint = 0;
	for (UInt hor = 0; hor < m_uiCtuInRow; hor++)
	{
		if (m_ctuElement[horUpIdx + hor]->getCtuSta() == STA_MOVOBJ)
		{
			if (m_ctuElement[horUpIdx + hor + m_uiCtuInRow]->getCtuSta() != STA_MOVOBJ)
				m_ctuElement[horUpIdx + hor]->setCtuSta(STA_POSSIBLE_OBJ);
		}
		if (m_ctuElement[horBotIdx + hor]->getCtuSta() == STA_MOVOBJ)
		{
			if (m_ctuElement[horBotIdx + hor - m_uiCtuInRow]->getCtuSta() != STA_MOVOBJ)
				m_ctuElement[horBotIdx + hor]->setCtuSta(STA_POSSIBLE_OBJ);
		}
	}

	for (UInt ver = 1; ver < m_uiCtuInCol - 1; ver++)
	{
		if (m_ctuElement[verLeftIdx + ver*m_uiCtuInRow]->getCtuSta() == STA_MOVOBJ)
		{
			if (m_ctuElement[verLeftIdx + ver*m_uiCtuInRow + 1]->getCtuSta() != STA_MOVOBJ)
				m_ctuElement[verLeftIdx + ver*m_uiCtuInRow]->setCtuSta(STA_POSSIBLE_OBJ);
		}
		if (m_ctuElement[verRigIdx + ver*m_uiCtuInRow]->getCtuSta() == STA_MOVOBJ)
		{
			if (m_ctuElement[verRigIdx + ver*m_uiCtuInRow - 1]->getCtuSta() != STA_MOVOBJ)
				m_ctuElement[verRigIdx + ver*m_uiCtuInRow]->setCtuSta(STA_POSSIBLE_OBJ);
		}
	}
}
Void PicData::dealObject()
{
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		UInt centroidX = 0, centroidY = 0;
		UInt leftEdge, upEdge, rightEdge, botEdeg = 0;
		(*beg)->getObjCentroid(centroidX, centroidY, true);
		(*beg)->getObjEdge(leftEdge, upEdge, rightEdge, botEdeg, true);
		(*beg)->chainCorObject(0, true);
		++beg;
	}

}
Void PicData::chainObjectCandidate()
{
	auto beg = m_vObject.begin();
	auto idx = m_picSeqNum;
	while (beg != m_vObject.end())
	{
		(*beg)->dealObjcetChain();
		++beg;
	}
}

void PicData::checkHeadFunc(MovingObject * pObj)
{
	UInt picNum = getPicNum();
	UInt hasBeenSet = 0;

	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pObj)
		{
			hasBeenSet = 1;
		}
		++beg;
	}
	if (picNum == pObj->getHeadPointer()->getPicNum())
	{
		if (hasBeenSet == 0)
		{
			cout << "head set test, head not set right, exit" << endl;
			cout << "pic is " << picNum << endl;
			exit(0);
		}
	}
	else
	if (picNum < pObj->getHeadPointer()->getPicNum())
	{
		if (hasBeenSet == 1)
		{
			cout << "object exists before the head frame, head not set right, exit" << endl;
			cout << "pic is " << picNum << endl;
			exit(0);
		}
	}
	

	if (picNum == pObj->getTailPointer()->getPicNum())
	{
		if (hasBeenSet == 0)
		{
			cout << "tail set test, tail not set right, exit" << endl;
			cout << "pic is " << picNum << endl;
			exit(0);
		}
	}
	else
	if (picNum > pObj->getTailPointer()->getPicNum())
	{
		if (hasBeenSet == 1)
		{
			cout << "object exists after the tail frame, tail not set right, exit" << endl;
			cout << "pic is " << picNum << endl;
			exit(0);
		}
	}
	std::multimap<MovingObject*, Bool, OBJECT_LENGTH_COMPARE> movingObjMap=this->getCurVideoData()->getMovObjMap();
	beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		MovingObject* pObj=(*beg)->getMovingOjbect();
		if (pObj->getMovObjectNum()!=0)
		if (movingObjMap.find(pObj) == movingObjMap.end())
		{
			cout << "pic num is " <<this->getPicNum()<< endl;
			cout << "there is an obj not in map, exit" << endl;
			int gg; cin >> gg;
		}
		beg++;
	}
	
}
//
Void PicData::resetMovingObj(MovingObject* pMovobj1, MovingObject* pMovobj2)
{
	
	if (pMovobj1 == pMovobj2)
		return;
	//cout << " begin reseting moving obj" << endl;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pMovobj1)
		{
			(*beg)->setMovingObject(pMovobj2);
			pMovobj1->eraseCandidate(*beg);
		
			/*if (m_picSeqNum < pMovobj2->getHeadPointer()->getCurPic()->getPicNum())
				pMovobj2->inreaseLength(1);
			if (m_picSeqNum > pMovobj2->getTailPointer()->getCurPic()->getPicNum())
				pMovobj2->inreaseLength(1);*/
		}
		++beg;
	}
	
}


Void	PicData::splitMovingObj(MovingObject* pOriObj, MovingObject* pMovobj1, MovingObject* pMovobj2)
{


	// calculate the relation between pMovobj1 and pMovobj2
	ObjectCandidate *head1 = pMovobj1->getHeadPointer();
	ObjectCandidate *head2 = pMovobj2->getHeadPointer();
	Bool splitLeftRight = 0;

	UInt centroidX1, centroidY1 = 0;
	UInt centroidX2, centroidY2 = 0;
	head1->getObjCentroid(centroidX1, centroidY1,true);
	head2->getObjCentroid(centroidX2, centroidY2,true);

	MovingObject* place1Object=NULL, *place2Object=NULL;
	int deltaX = int(centroidX1) - int(centroidX2);
	int deltaY = int(centroidY1) - int(centroidY2);

	/*cout << "the centroidX1,centroidY1 is " << centroidX1 << "  " << centroidY1 << endl;
	cout << "the centroidX2,centroidY2 is " << centroidX2 << "  " << centroidY2 << endl;
	int aa; cin >> aa;*/
	//place1Object is supposed to be at the left top place of the two objects
	UInt leftEdge1 = 0, upEdge1=0, rightEdeg1=0, botEdge1=0;
	UInt leftEdge2 = 0, upEdge2 = 0, rightEdeg2 = 0, botEdge2 = 0;
	head1->getObjEdge(leftEdge1, upEdge1, rightEdeg1, botEdge1, false);
	head2->getObjEdge(leftEdge2, upEdge2, rightEdeg2, botEdge2, false);

	UInt leftDir, upDir, rigDir, botDir;
	Bool atBorder1 = head1->atTheBorder(leftDir, upDir, rigDir, botDir);
	Bool atBorder2 = head2->atTheBorder(leftDir, upDir, rigDir, botDir);
	if (deltaX < 0 && rightEdeg1 <= leftEdge2 && upEdge1 <= upEdge2 && botEdge1 >= botEdge2&&atBorder1 == 0 && atBorder2==0)
	{
		splitLeftRight = 1;
		place1Object = pMovobj1;
		place2Object = pMovobj2;
	}
	else
	if (deltaX > 0 && rightEdeg2 <= leftEdge1 && upEdge2 <= upEdge1 && botEdge2 >= botEdge1&&atBorder1 == 0 && atBorder2 == 0)
	{
		splitLeftRight = 1;
		place1Object = pMovobj2;
		place2Object = pMovobj1;
	}
	else if (deltaX <= 0 && deltaY <= 0)
	{
		place1Object = pMovobj1;
		place2Object = pMovobj2;
	}
	else if (deltaX >= 0 && deltaY >= 0)
	{
		place1Object = pMovobj2;
		place2Object = pMovobj1;
	}
	else if(deltaY <= 0)
	{
		place1Object = pMovobj1;
		place2Object = pMovobj2;
	}
	else
	{
		place1Object = pMovobj2;
		place2Object = pMovobj1;
	}

	//splitLeftRight = 0;
	Float ratioLeftRight = Float(place1Object->getHeadPointer()->getCtuElements().size()) /
		Float(place1Object->getHeadPointer()->getCtuElements().size()+place2Object->getHeadPointer()->getCtuElements().size());
//	Float ratioLeftRight = 0.5;
	
	vector<ObjectCandidate*> objects;
	UInt totalArea = 0;
	UInt size = 0;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pOriObj)
		{
			objects.push_back(*beg);
			size = (*beg)->getCtuElements().size();
			totalArea = totalArea + size;
		}
		beg++;
	}


	if (totalArea== 1)
	{
		auto beg = m_vObject.begin();
		while (beg != m_vObject.end())
		{
			if ((*beg)->getMovingOjbect() == pOriObj)
			{
				(*beg)->setMovingObject(pMovobj1);
			}
			beg++;
		}
		return;
	}

	
	if (objects.size()>1)
	{
		beg = objects.begin();
		while (beg != objects.end())
		{
			if (beg == objects.begin())
				(*beg)->setMovingObject(place1Object);
			else
				(*beg)->setMovingObject(place2Object);
			beg++;
		}

		return;
	}

	for (auto beg = m_vObject.begin();beg != m_vObject.end();beg++)
	{		
		if ((*beg)->getMovingOjbect() == pOriObj)
		{
			//cout << "first ctu size is " << (*beg)->getCtuElements().size() << endl;
			(*beg)->setMovingObject(place2Object);

			CtuData* curCtu;			
		
			std::map<CtuData*, Bool, ctuAddrIncCompare> ctuElement = (*beg)->getCtuElements();
			
			//cout << "second ctu size is " << (*beg)->getCtuElements().size() << endl;
			//std::map<CtuData*, Bool, ctuAddrIncCompare> tempCtuElement;

			UInt size = ctuElement.size();
			UInt obj1Size = Float(size)*ratioLeftRight;
			

			ObjectCandidate* curOjb =  new ObjectCandidate(this);
			curOjb->setMovingObject(place1Object);
			//addNewObj(curOjb);
			
			auto begCtu = ctuElement.begin();
			//cout << "third ctu size is " << (*beg)->getCtuElements().size() << endl;
			//cout << "size is " << size << endl;
			curCtu = begCtu->first;
			//curOjb->setCurPic(curCtu->getPic());
			//curOjb->setPicNum(curCtu->getPic()->getPicNum());
			//cout << "the pic num is " << curCtu->getPic()->getPicNum() << endl;
			//cout << "the ctu Size is " << ctuElement.size()<<endl;
			//cout << "splitLR flag is " << splitLeftRight << endl;
			//cout << "obj1Size is " << obj1Size << endl;
			
			if (!splitLeftRight)
			{
				UInt i = 1;
				while (begCtu != ctuElement.end())
				{
					curCtu = begCtu->first;
					//curCtu->setCtuAddr(begCtu->first->getCtuAddr());
					if (i <= obj1Size)
					{
						
						curCtu->setObject(curOjb);
						
						curOjb->addCtuElement(curCtu);
						//cout << "after ctu size is " << curOjb->getCtuElements().size()<<endl;
						//(*beg)->eraseCtuElement(curCtu);
						/*if (i == obj1Size)
						{
						cout << endl;
						cout << endl;
						cout << endl;
						}*/
					}
					else
					{
						cout << "this is else, i is " << i << endl;
						cout << "curCtu address is " << curCtu->getCtuAddr() << endl;
						cout << "the begCtu address is " << begCtu->first->getCtuAddr() << endl;
						cout << "the ctu x,y is " << curCtu->getAbsx() << "  " << curCtu->getAbsy() << endl;
						(*beg)->addCtuElement(curCtu);
						//tempCtuElement.insert(*begCtu);
					}
					i++;
					begCtu++;
				}
			}
			else
			{
				std::map<CtuData*, Bool, ctuAddrIncCompare> leftCtu;
				std::map<CtuData*, Bool, ctuAddrIncCompare>rightCtu;
				Float ratio = ratioLeftRight; 
				(*beg)->splitCtusIntoLeftRight(leftCtu, rightCtu, ratio);
				curOjb->setCtuElements(leftCtu);
				(*beg)->setCtuElements(rightCtu);
			}


			//cout << "i is " << i << endl;
			//i = 1;
			//(*beg)->setCtuElements(tempCtuElement);
			/*begCtu = ctuElement.begin();
			while (begCtu != ctuElement.end())
			{
				curCtu = begCtu->first;
				if (i <= obj1Size)
				{
					(*beg)->eraseCtuElement(curCtu);
					i++;
					
					ctuElement = (*beg)->getCtuElements();
					begCtu = ctuElement.begin();
					continue;
				}
				begCtu++;
			}*/
			
			UInt centroidX, centroidY = 0;
			UInt leftEdge, upEdge, rightEdge, botEdeg = 0;
			(*beg)->getObjCentroid(centroidX, centroidY, true);
			(*beg)->getObjEdge(leftEdge, upEdge, rightEdge, botEdeg, true);
			
			curOjb->getObjCentroid(centroidX, centroidY, true);
			curOjb->getObjEdge(leftEdge, upEdge, rightEdge, botEdeg, true);
			/*cout << "the curOjb ctu size is " << curOjb->getCtuElements().size() << endl;
			cout << "the (*beg) ctu size is " << (*beg)->getCtuElements().size() << endl;
			cout << "the ctu in row is " << this->getCtuInRow() << endl;*/
			//int aa; cin >> aa;
			/*beg = m_vObject.begin();
			continue;*/
			break;
		}
	}

}


Bool PicData::splitObj1AccordingToCand(MovingObject* pLargeMovobj, MovingObject* pSmallMovobj2, ObjectCandidate* cand, ObjectCandidate*& newSplitCand)
{
	cout << "the curr pic is "<<getPicNum() << endl;
	cout << "the cand's pic is " << cand->getPicNum() << endl;
	UInt x, y = 0; cand->getObjCentroid(x,y,true);
	cout << "the cand's x,y is " << x<<"  "<< y << endl;
	cout << "the cand's ctu size is " << cand->getCtuElements().size() << endl;
	std::map<CtuData*, Bool, ctuAddrIncCompare> candCtu = cand->getCtuElements();

	std::map<CtuData*, Bool, ctuAddrIncCompare> newCandCtu;

	std::map<UInt, Bool> candCtuMapByAddress;
	auto begCandCtu = candCtu.begin();
	while (begCandCtu != candCtu.end())
	{
		candCtuMapByAddress.insert(make_pair(begCandCtu->first->getCtuAddr(), true));
		begCandCtu++;
	}
//	cout << "the candCtuMapByAddress size is " << candCtuMapByAddress.size() << endl;
	ObjectCandidate* newCand = NULL;

	UInt totalArea = getObjectArea(pLargeMovobj);
	if (totalArea <= 1)
	{
		return false;
	}

	UInt totalNum = 0;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pLargeMovobj)
		{
			totalNum++;
		}
		beg++;
	}
	if (totalNum > 1)
		return false;


	std::map<CtuData*, Bool, ctuAddrIncCompare> newCurrCtu;
	beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pLargeMovobj)
		{
			ObjectCandidate* currCand = (*beg);
			std::map<CtuData*, Bool, ctuAddrIncCompare> currCtu(currCand->getCtuElements());
			
			auto begCurrCtu = currCtu.begin();
			while (begCurrCtu != currCtu.end())
			{
				if (candCtuMapByAddress.find(begCurrCtu->first->getCtuAddr()) != candCtuMapByAddress.end())
				{
					newCandCtu.insert(*begCurrCtu);	
				}
				else
				{
					newCurrCtu.insert(*begCurrCtu);
				}

				begCurrCtu++;
			}

			if (newCurrCtu.size() == 0)
			{
				cout << "newCurrCtu.size is 0 " << endl;
				(*beg)->setCtuElements(currCtu);
				return false;
				//int tr; cin >> tr;
			}
			(*beg)->setCtuElements(newCurrCtu);
			newCurrCtu.clear();
		}

		beg++;
	}

	if (newCandCtu.size() > 0)
	{
		newCand = new ObjectCandidate(this);
		newCand->setCtuElements(newCandCtu);

		auto begCtu = newCandCtu.begin();
		while (begCtu != newCandCtu.end())
		{
			begCtu->first->setObject(newCand);
			begCtu++;
		}
		newCand->setMovingObject(pSmallMovobj2);
		newSplitCand = newCand;
	}

	totalArea = getObjectArea(pLargeMovobj);
	if (totalArea==0)
	{
		cout << "the object has been deleted" << endl;
		cout << "The pic num is " << getPicNum() << endl;
		int rr; cin >> rr;
	}
	return true;
}



Bool	PicData::splitObj1AccordingToBoundingBox(MovingObject* pLargeMovobj, MovingObject* pSmallMovobj2, ObjectCandidate* cand, UInt leftEdge_Large, UInt  upEdge_Large, UInt rightEdge_Large, UInt  botEdge_Large)
{
	UInt x, y = 0; cand->getObjCentroid(x, y, true);
	
	//cout << "the cand's ctu size is " << cand->getCtuElements().size() << endl;
	cout << "rrttre" << endl;
	std::map<CtuData*, Bool, ctuAddrIncCompare> candCtu = cand->getCtuElements();
	cout << "second rrttre" << endl;
	ObjectCandidate* newCand = NULL;
	ObjectCandidate* currCand = NULL;

	UInt totalArea = getObjectArea(pLargeMovobj);
	if (totalArea <= 1)
	{
		return false;
	}

	std::map<CtuData*, Bool, ctuAddrIncCompare> totalCtu;
	
	UInt candidateNum = 0;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pLargeMovobj)
		{
			currCand = *beg;
			std::map<CtuData*, Bool, ctuAddrIncCompare> tempCtu(currCand->getCtuElements());
			auto begCtuData = tempCtu.begin();
			while (begCtuData != tempCtu.end())
			{
				totalCtu.insert(*begCtuData);
				begCtuData++;
			}
			candidateNum++;
		}
		beg++;
	}

	cout << "tr" << endl;
	if (candidateNum > 1)
	{
		UInt leftEdge_Curr = 0, upEdge_Curr = 0, rightEdge_Curr = 0, botEdge_Curr = 0;
		this->getCtuEdge(totalCtu, leftEdge_Curr, upEdge_Curr, rightEdge_Curr, botEdge_Curr);
		auto begObj = m_vObject.begin();
		while (begObj != m_vObject.end())
		{
			cout << "hfere" << endl;
			if ((*begObj) == NULL)
				cout << "(*begObj) is NULL" << endl;
			if ((*begObj)->getMovingOjbect() == pLargeMovobj)
			{
				cout << "grttting" << endl;
				currCand = *begObj;
				std::map<CtuData*, Bool, ctuAddrIncCompare> currCtu(currCand->getCtuElements());
				std::map<CtuData*, Bool, ctuAddrIncCompare> newCurrCtu;
				std::map<CtuData*, Bool, ctuAddrIncCompare> newCandCtu;
				cout << "rghyyygrttting" << endl;
				auto begCurrCtu = currCtu.begin();
				while (begCurrCtu != currCtu.end())
				{
					CtuData* ctu = NULL;
					ctu = begCurrCtu->first;
					Bool ctuMatch = false;
					cout << "calculating bounding box "<< endl;
					ctuMatch = currCand->addCtuAccordingToBoundingBox(leftEdge_Curr, upEdge_Curr, rightEdge_Curr, botEdge_Curr, ctu, cand, leftEdge_Large, upEdge_Large, rightEdge_Large, botEdge_Large);
					cout << "end bounding box " << endl;
					if (ctuMatch)
					{
						newCandCtu.insert(*begCurrCtu);
					}
					else
					{
						newCurrCtu.insert(*begCurrCtu);
					}
					begCurrCtu++;
					cout << "debuggg poiyt" << endl;
				}

				if (newCandCtu.size() == 0)
					;//nothing else to do 
				else if (newCurrCtu.size() == 0)
					currCand->setMovingObject(pSmallMovobj2);
				else
				if (newCandCtu.size() != 0 && newCurrCtu.size() != 0)
				{

					newCand = new ObjectCandidate(this);
					newCand->setCtuElements(newCandCtu);

					auto begCtu = newCandCtu.begin();
					while (begCtu != newCandCtu.end())
					{
						begCtu->first->setObject(newCand);
						begCtu++;
					}
					newCand->setMovingObject(pSmallMovobj2);
					currCand->setCtuElements(newCurrCtu);
					//cout << "second sss" << endl;
					begObj = m_vObject.begin();
					continue;
				}

			}

			begObj++;
		}
		//cout << "returninggg" << endl;
		return true;
	}
	cout << "seee" << endl;

	CtuData* ctu = NULL;
	Bool ctuMatch = false;
	std::map<CtuData*, Bool, ctuAddrIncCompare> currCtu(currCand->getCtuElements());
	std::map<CtuData*, Bool, ctuAddrIncCompare> newCurrCtu;
	std::map<CtuData*, Bool, ctuAddrIncCompare> newCandCtu;
	auto begCurrCtu = currCtu.begin();
	while (begCurrCtu != currCtu.end())
	{
		ctu = begCurrCtu->first;
		UInt leftEdge_Curr = 0, upEdge_Curr = 0, rightEdge_Curr = 0, botEdge_Curr = 0;
		currCand->getObjEdge(leftEdge_Curr, upEdge_Curr, rightEdge_Curr, botEdge_Curr, true);
		
		ctuMatch = currCand->addCtuAccordingToBoundingBox(leftEdge_Curr, upEdge_Curr, rightEdge_Curr, botEdge_Curr, ctu, cand, leftEdge_Large, upEdge_Large, rightEdge_Large, botEdge_Large);

		if (ctuMatch)
		{
			newCandCtu.insert(*begCurrCtu);
		}
		else
		{
			newCurrCtu.insert(*begCurrCtu);
		}
		begCurrCtu++;
	}
	//cout << "newCandCtu.size() is " << newCandCtu.size() << "  newCurrCtu.size is " << newCurrCtu.size() << endl;
	//int gh; cin >> gh;

	if (newCandCtu.size() == 0 || newCurrCtu.size() == 0)
		return false;

	currCand->setCtuElements(newCurrCtu);

	if (newCandCtu.size() > 0)
	{
		newCand = new ObjectCandidate(this);
		newCand->setCtuElements(newCandCtu);

		auto begCtu = newCandCtu.begin();
		while (begCtu != newCandCtu.end())
		{
			begCtu->first->setObject(newCand);
			begCtu++;
		}
		newCand->setMovingObject(pSmallMovobj2);
	}

	totalArea = getObjectArea(pLargeMovobj);
	if (totalArea == 0)
	{
		cout << "splitObj1AccordingToBoundingBox, the object has been deleted" << endl;
		cout << "The pic num is " << getPicNum() << endl;
		int rr; cin >> rr;
	}
	return true;

}

Bool  PicData::splitObj1AccordingToPubCtus(MovingObject* pLargeMovobj, MovingObject* pMovobj2, std::map<CtuData*, Bool, ctuAddrIncCompare> pubCtu)
{
	//cout << "the curr pic is " << getPicNum() << endl;
	
	//cout << "the cand's ctu size is " << pubCtu.size() << endl;

	if (pubCtu.size() == 0)
	{
		cout << "the public Ctu is zero, exit" << endl;
		exit(0);
		return false;
	}
	std::map<CtuData*, Bool, ctuAddrIncCompare> candCtu = pubCtu;

	std::map<CtuData*, Bool, ctuAddrIncCompare> newCandCtu;

	std::map<UInt, Bool> candCtuMapByAddress;
	auto begCandCtu = candCtu.begin();
	while (begCandCtu != candCtu.end())
	{
		candCtuMapByAddress.insert(make_pair(begCandCtu->first->getCtuAddr(), true));
		begCandCtu++;
	}

	ObjectCandidate* newCand = NULL;

	UInt totalArea = getObjectArea(pLargeMovobj);
	if (totalArea <= 1)
	{
		return false;
	}

	std::map<CtuData*, Bool, ctuAddrIncCompare> newCurrCtu;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pLargeMovobj)
		{
			ObjectCandidate* currCand = (*beg);
			std::map<CtuData*, Bool, ctuAddrIncCompare> currCtu(currCand->getCtuElements());


			auto begCurrCtu = currCtu.begin();
			while (begCurrCtu != currCtu.end())
			{
				if (candCtuMapByAddress.find(begCurrCtu->first->getCtuAddr()) != candCtuMapByAddress.end())
				{
					newCandCtu.insert(*begCurrCtu);
					begCurrCtu->first->setObject(newCand);
				}
				else
				{
					newCurrCtu.insert(*begCurrCtu);
				}

				begCurrCtu++;
			}

			if (newCurrCtu.size() == 0)
			{
				cout << "newCurrCtu.size is 0 " << endl;
				int tr; cin >> tr;
			}
			(*beg)->setCtuElements(newCurrCtu);
			newCurrCtu.clear();

		}

		beg++;
	}

	if (newCandCtu.size() > 0)
	{
		newCand = new ObjectCandidate(this);
		newCand->setCtuElements(newCandCtu);
		newCand->setMovingObject(pMovobj2);
	}
	//else
	//{
	//	//cout << "the curr pic is " << getPicNum() << endl;
	//	//cout << "the new candidate does not produce" << endl;
	//	//int ty; cin >> ty;

	//}

	totalArea = getObjectArea(pLargeMovobj);
	if (totalArea == 0)
	{
		cout << "the object has been deleted" << endl;
		cout << "The pic num is " << getPicNum() << endl;
		int rr; cin >> rr;
	}
	return true;
}

//Bool	PicData::splitObj1AccordingToCtu(MovingObject* pMovobj1, MovingObject* pMovobj2, std::map<UInt, Bool> newCandCtu)
//{
//
//	if (newCandCtu.size() == 0)
//	{
//		cout << "the public Ctu is zero, exit" << endl;
//		exit(0);
//		return false;
//	}
//	std::map<CtuData*, Bool, ctuAddrIncCompare> candCtu = pubCtu;
//
//	std::map<CtuData*, Bool, ctuAddrIncCompare> newCandCtu;
//
//	std::map<UInt, Bool> candCtuMapByAddress;
//	auto begCandCtu = candCtu.begin();
//	while (begCandCtu != candCtu.end())
//	{
//		candCtuMapByAddress.insert(make_pair(begCandCtu->first->getCtuAddr(), true));
//		begCandCtu++;
//	}
//
//	ObjectCandidate* newCand = NULL;
//
//	UInt totalArea = getObjectArea(pLargeMovobj);
//	if (totalArea <= 1)
//	{
//		return false;
//	}
//
//	std::map<CtuData*, Bool, ctuAddrIncCompare> newCurrCtu;
//	auto beg = m_vObject.begin();
//	while (beg != m_vObject.end())
//	{
//		if ((*beg)->getMovingOjbect() == pLargeMovobj)
//		{
//			ObjectCandidate* currCand = (*beg);
//			std::map<CtuData*, Bool, ctuAddrIncCompare> currCtu(currCand->getCtuElements());
//
//
//			auto begCurrCtu = currCtu.begin();
//			while (begCurrCtu != currCtu.end())
//			{
//				if (candCtuMapByAddress.find(begCurrCtu->first->getCtuAddr()) != candCtuMapByAddress.end())
//				{
//					newCandCtu.insert(*begCurrCtu);
//					begCurrCtu->first->setObject(newCand);
//				}
//				else
//				{
//					newCurrCtu.insert(*begCurrCtu);
//				}
//
//				begCurrCtu++;
//			}
//
//			if (newCurrCtu.size() == 0)
//			{
//				cout << "newCurrCtu.size is 0 " << endl;
//				int tr; cin >> tr;
//			}
//			(*beg)->setCtuElements(newCurrCtu);
//			newCurrCtu.clear();
//
//		}
//
//		beg++;
//	}
//
//	if (newCandCtu.size() > 0)
//	{
//		newCand = new ObjectCandidate(this);
//		newCand->setCtuElements(newCandCtu);
//		newCand->setMovingObject(pMovobj2);
//	}
//	//else
//	//{
//	//	//cout << "the curr pic is " << getPicNum() << endl;
//	//	//cout << "the new candidate does not produce" << endl;
//	//	//int ty; cin >> ty;
//
//	//}
//
//	totalArea = getObjectArea(pLargeMovobj);
//	if (totalArea == 0)
//	{
//		cout << "the object has been deleted" << endl;
//		cout << "The pic num is " << getPicNum() << endl;
//		int rr; cin >> rr;
//	}
//	return true;
//
//}

Void  PicData::getCtuEdge(std::map<CtuData*, Bool, ctuAddrIncCompare> totalCtu, UInt &leftEdge, UInt &upEdge, UInt &rightEdge, UInt &botEdge)
{

	if (totalCtu.size() == 0)
	{
		cout << "the total ctu number is zero, getting object edge fails, exit" << endl;
		exit(0);
	}


	CtuData* curCtu = NULL;

	UInt m_uiLeftEdge = 0, m_uiUpEdge = 0, m_uiRightEdge = 0, m_uiBottomEdge = 0;
	UInt ctuSize = this->getCtuWid(), ctuCoordX = 0, ctuCoordY = 0;
	m_uiLeftEdge = this->getPicWid();
	m_uiUpEdge = this->getPicHig();
	m_uiRightEdge = m_uiBottomEdge = 0;

	//PicData*	currPic = getCurPic();
	//UInt ctuInRow = currPic->getCtuInRow();
	auto begCtu = totalCtu.begin();
	while (begCtu != totalCtu.end())
	{
		curCtu = begCtu->first;
		curCtu->getAbsXY(ctuCoordX, ctuCoordY);
		//UInt address=curCtu->getCtuAddr();
		//ctuCoordX = (address %ctuInRow)*ctuSize;
		//ctuCoordY = (address /ctuInRow)*ctuSize;
		m_uiUpEdge = ctuCoordY < m_uiUpEdge ? ctuCoordY : m_uiUpEdge;
		m_uiLeftEdge = ctuCoordX < m_uiLeftEdge ? ctuCoordX : m_uiLeftEdge;
		m_uiRightEdge = (ctuCoordX + ctuSize) > m_uiRightEdge ? (ctuCoordX + ctuSize) : m_uiRightEdge;
		m_uiBottomEdge = (ctuCoordY + ctuSize) > m_uiBottomEdge ? (ctuCoordY + ctuSize) : m_uiBottomEdge;
		++begCtu;
	}
	leftEdge = m_uiLeftEdge;
	upEdge = m_uiUpEdge;
	rightEdge = m_uiRightEdge;
	botEdge = m_uiBottomEdge;

}

Void PicData::getCtuCentroid(std::map<CtuData*, Bool, ctuAddrIncCompare> totalCtu, UInt& centroidX, UInt& centroidY)
{

	if (totalCtu.size() == 0)
	{
		cout << "the object's ctu number is zero, pic function getting object centroid fails, exit" << endl;
		exit(0);
	}


	CtuData* curCtu = NULL;
	UInt rightEdge = this->getPicWid();
	UInt bottomEdge = this->getPicHig();
	UInt ctuSize = this->getCtuWid();
	UInt ctuArea = ctuSize*ctuSize;
	UInt sumOfWeightX = 0;
	UInt sumOfWeightY = 0;
	UInt sumOfCtu = (UInt)totalCtu.size();
	UInt yPos = 0;
	UInt tempCentroidX = 0;
	UInt tempCentroidY = 0;

	auto begCtu = totalCtu.begin();
	while (begCtu != totalCtu.end())
	{
		curCtu = begCtu->first;
		yPos = curCtu->getAbsy();

		tempCentroidX += ctuArea*(curCtu->getAbsx() + ctuSize / 2);
		tempCentroidY += ctuArea*(curCtu->getAbsy() + ctuSize / 2);
		sumOfWeightX += ctuArea;
		sumOfWeightY += ctuArea;


		++begCtu;
	}

	if (sumOfWeightX != 0)
	{
		tempCentroidX = UInt(tempCentroidX / sumOfWeightX);
		tempCentroidY = UInt(tempCentroidY / sumOfWeightY);
	}
	if (tempCentroidX > rightEdge)tempCentroidX = rightEdge;
	if (tempCentroidY > bottomEdge) tempCentroidY = bottomEdge;
	centroidX = tempCentroidX;
	centroidY = tempCentroidY;

}


//
Void PicData::resetUsefulFlag(MovingObject* pMovObj, Bool flag)
{
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pMovObj)
			(*beg)->setUsefulFlag(false);
		++beg;
	}
}


Void PicData::objectSegmentation()
{
	CtuData* curCtu;

	auto beg = m_ctuElement.begin();
	while (beg != m_ctuElement.end())
	{
		curCtu = *beg;
		if (curCtu->getCtuSta() == STA_MOVOBJ && curCtu->getObject() == NULL)
		{
			ObjectCandidate* curOjb = new ObjectCandidate(this);
			//addNewObj(curOjb);
			//curOjb->setCurPic(curCtu->getPic());
			//curOjb->setPicNum(curCtu->getPic()->getPicNum());
			curCtu->setObject(curOjb);
			curOjb->addCtuElement(curCtu);
			curCtu->travelNeighborCtu(curOjb, false);
		}
		beg++;
	}
}

Void PicData::removNoise()
{
	Bool priorBackGround = false, nextBackGround = false;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getPriorCand().size() == 0 && (*beg)->getNextCand().size() == 0)
		{
			priorBackGround = (*beg)->removable(true, false);
			nextBackGround = (*beg)->removable(false, false);
			if (priorBackGround && nextBackGround)
				(*beg)->getMovingOjbect()->setMovObjectNum(0);
		}
		beg++;
	}
}


//
//set candidate useful flag
//if there is only one candidate in current frame

Void PicData::setCandidateUsefulFlag(MovingObject* pMovObj)
{
	Bool usefulFlag = false;
	UInt centroidX, centroidY, leftX, rigX, upY, botY;
	std::vector<ObjectCandidate*> scanedCandidate;
	//search useful candidates
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pMovObj)
		{
			(*beg)->getObjCentroid(centroidX, centroidY, true);
			(*beg)->getObjEdge(leftX, upY, rigX, botY, true);
			(*beg)->setPxlElement();
			(*beg)->setUsefulFlag(false);
			scanedCandidate.push_back((*beg));
		}
		++beg;
	}
	if (scanedCandidate.size() > 1)
	
		usefulFlag = false;
	
	else		
		usefulFlag = true;
		

	// add current candidate and its frame number and candidate index to the moving object
	auto scandCandBeg = scanedCandidate.begin();
	while (scandCandBeg != scanedCandidate.end())
	{
		//the condition to restrict the candidate not contact with boundries 
		//is not reasonable

		(*scandCandBeg)->setUsefulFlag(usefulFlag);
		pMovObj->addPicIdx(m_picSeqNum, (*scandCandBeg)->getObjNum());
		++scandCandBeg;
	}
}
//
Void PicData::setEdgeCtuCandFlag(MovingObject* pCurMovObj)
{
	ObjectCandidate* pCurObjCand;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pCurMovObj)
		{
			pCurObjCand = *beg;
			pCurObjCand->checkEdgeCtu();
		}
		++beg;
	}
}
//
Void PicData::recoverCandidate(MovingObject* pCurMovObje)
{
	ObjectCandidate* pCurObj;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pCurMovObje)
		{
			pCurObj = *beg;
			if (pCurObj->getObjNum() != 0)
				pCurObj->recoverCtu();
		}
		++beg;
	}
}

UInt PicData::getObjectArea(MovingObject* pCurMovObj)
{
	UInt totalArea = 0;
	UInt size = 0;
	ObjectCandidate* pCurObj;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pCurMovObj)
		{
			pCurObj = *beg;
			size=pCurObj->getCtuElements().size();
			totalArea += size;
			
		}
		++beg;
	}
	return totalArea;
}

std::vector<ObjectCandidate*> PicData::findMovObj(MovingObject* pCurMovObj)
{
	std::vector<ObjectCandidate*> candVec;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getMovingOjbect() == pCurMovObj)
			candVec.push_back(*beg);
		beg++;
	}

	return candVec;


}



//deal with one-to-many condition
Void PicData::dealOneToManyCondition()
{
//	MovingObject* pCurMovObj;
	auto beg = m_vObject.begin();
	while (beg != m_vObject.end())
	{
		if ((*beg)->getPriorCand().size() > 1)
		{
			(*beg)->dealOneToMany();
		}
		++beg;
	}
}


//write frame data to file
Void PicData::PicWrite(std::ostream& handle)
{
	UInt CtuInWid = m_picWidth / m_CtuWid + (m_picWidth%m_CtuWid ? 1 : 0);
	UInt CtuInHig = m_picHeght / m_CtuHig + (m_picHeght%m_CtuHig ? 1 : 0);
	Pel *OneRow = new Pel[m_stride];
	Pel *TwoRow = new Pel[(m_stride / 2)];
	Pel *det, *src;
	Int lenY = m_picWidth;
	Int lenUV = m_picWidth / 2;
	UInt uv_frm = m_picHeght / 2;
	UInt uv_hig = m_CtuHig / 2;
	UInt uv_wid = m_CtuWid / 2;
	UChar *buf = new UChar[m_picWidth]();
	UChar *fub = new UChar[(m_picWidth >> 1)]();
	CtuData *curCtu;
	//sortCtus();
	for (UInt col = 0; col < m_picHeght; col++)
	{
		UInt Cturow = col / m_CtuHig;//CTU  
		UInt row = col%m_CtuHig;	//PIXEL line
		det = OneRow;
		for (UInt wid = 0; wid < CtuInWid; wid++)
		{
			curCtu = getCtu(Cturow*CtuInWid + wid);
			src = curCtu->getOrgY(row);
			if (det == NULL || src == NULL)
			{
				printf(" error element read %s %d\n",__FILE__,__LINE__);
			}

			::memcpy(det, src, sizeof(Pel)*m_CtuWid);
		     det += m_CtuWid;
		}
		for (UInt x = 0; x < m_picWidth; x++)buf[x] = (UChar)OneRow[x];
			handle.write(reinterpret_cast<Char*>(buf), lenY);
	}
	::memset(fub, 0, sizeof(UChar)*lenUV);
	for (UInt vol = 0; vol < uv_frm; vol++)
	{
		
		UInt Cturow = vol / uv_wid;//  
		UInt row = vol%uv_hig;	//PIXEL line
		det = TwoRow;
		for (UInt wid = 0; wid < CtuInWid; wid++)
		{
			curCtu = getCtu(Cturow*CtuInWid + wid);
			src = curCtu->getOrgU(row);
			::memcpy(det, src, sizeof(Pel)*uv_wid);
			det += uv_wid;
		}
		for (UInt x = 0; x < lenUV; x++)
			fub[x] = (UChar)TwoRow[x];
		handle.write(reinterpret_cast<Char*>(fub), lenUV);
	}
	::memset(fub, 0, sizeof(UChar)*lenUV);
	for (UInt vol = 0; vol < uv_frm; vol++)
	{
		
		UInt Cturow = vol / uv_hig;//CTU  
		UInt row = vol%uv_hig;	//PIXEL line
		det = TwoRow;
		for (UInt wid = 0; wid < CtuInWid; wid++)
		{
			curCtu = getCtu(Cturow*CtuInWid + wid);
			src = curCtu->getOrgV(row);
			memcpy(det, src, (sizeof(Pel)*uv_wid));
			det += uv_wid;
		}
		for (UInt x = 0; x < lenUV; x++)
			fub[x] = (UChar)TwoRow[x];
		
		handle.write(reinterpret_cast<Char*>(fub), lenUV);
	}
	delete[]buf;
	delete[]fub;
	delete[]OneRow;
	delete[]TwoRow;
}


Void PicData::PicNewWrite(std::ostream &handle, UChar* buf1, UChar* buf2, Pel* oneRow, Pel* twoRow, PicData* bufPic, std::vector<Int> pxlRow,
	std::vector<std::vector<Int>>	m_vPxlEle, queue<UInt> &usefulColors, queue<UInt> &uselessColors)
{
	std::cout << "first" << endl;

	UInt readStride = 0, color = 0, pxlEleVal = 0;
	UInt CtuInWid = m_picWidth / m_CtuWid + (m_picWidth%m_CtuWid ? 1 : 0);
	UInt CtuInHig = m_picHeght / m_CtuHig + (m_picHeght%m_CtuHig ? 1 : 0);
	UInt leftEdge, upEdge, rightEdge, bottomEdge = 0;
	std::vector<std::vector<std::pair<Int, Bool>>> candPxlElement;

	UInt uv_frm = m_picHeght / 2, uv_hig = m_CtuHig / 2, uv_wid = m_CtuWid / 2;
	Pel *UVcomVal = new Pel[m_CtuWid](), *YcomVal = new Pel[m_CtuWid]();

	Pel *OneRow = oneRow, *TwoRow = twoRow;// *det, *src;
	Int lenY = m_picWidth, lenUV = m_picWidth / 2;
	UChar *buf = buf1, *fub = buf2;

	UInt num = getPicNum();
	std::cout << "pic num is " << num << endl;

	std::vector<MovingObject*> objects;

	UInt colorNum = 0;
	auto objBeg = m_vObject.begin();
	while (objBeg != m_vObject.end())
	{
		cout << "executing!!!" << endl;
		if ((*objBeg) == NULL)
		{
			cout << "there is a NULL pointer in output" << endl;
			exit(1);
		}
		if ((*objBeg)->getMovingOjbect() == NULL)
		{
			cout << "there is a NULL in output" << endl;
			exit(1);
		}
		if ((*objBeg)->getObjNum() != 0 && (*objBeg)->getMovingOjbect()->getMovObjectNum() != 0)
		{
			cout << "fffcolor" << endl;
			//to set the moving object a color number
			Bool colorFlag = (*objBeg)->getMovingOjbect()->colorHasBeenSet();

			if (num == (*objBeg)->getMovingOjbect()->getHeadPointer()->getPicNum() && colorFlag == false)
			//if (colorFlag == false)
			{
				if (!usefulColors.empty())
				{
					colorNum = usefulColors.front();
					usefulColors.pop();
				}
				else
				if (!uselessColors.empty())
				{
					colorNum = uselessColors.front();
					uselessColors.pop();
				}
				else
				{
					cout << "the colors have all been used" << endl;
					exit(0);
				}

				(*objBeg)->getMovingOjbect()->setColorNum(colorNum);
			}

			objects.push_back((*objBeg)->getMovingOjbect());// for debug


			colorFlag = (*objBeg)->getMovingOjbect()->colorHasBeenSet();
			if (num == (*objBeg)->getMovingOjbect()->getTailPointer()->getPicNum() && colorFlag == true)
			{
				colorNum = (*objBeg)->getMovingOjbect()->getColorNum();
				if (colorNum <= 7)
					usefulColors.push(colorNum);
				else
					uselessColors.push(colorNum);

				(*objBeg)->getMovingOjbect()->setColorFlag(false);
			}

			UInt tempNum = (*objBeg)->getMovingOjbect()->getColorNum();
			//cout << "the head number is " << (*objBeg)->getMovingOjbect()->getHeadPointer()->getPicNum() << endl;
			if (!(tempNum >= 1 && tempNum <= 14 && tempNum != 0))
			{
				colorFlag = (*objBeg)->getMovingOjbect()->colorHasBeenSet();
				cout << "there is a bug when setting colors, exit" << endl;
				cout << "colorNum is " << tempNum << endl;
				cout << "object Num is " << (*objBeg)->getMovingOjbect()->getMovObjectNum() << endl;
				cout << "the color flag is " << colorFlag << endl;
				cout << "the object num is " << (*objBeg)->getMovingOjbect()->getMovObjectNum()<<endl;
				cout << "the head num is " << (*objBeg)->getMovingOjbect()->getHeadPointer()->getPicNum()<<endl;
				cout << "the tail num is " << (*objBeg)->getMovingOjbect()->getTailPointer()->getPicNum() << endl;
				exit(0);
			}
		
			(*objBeg)->getObjEdge(leftEdge, upEdge, rightEdge, bottomEdge, true);

			/*num = getPicNum();
			if (num == 207)
			{
				cout << "the object's ctu size is " << (*objBeg)->getCtuElements().size() << endl;
				cout << "leftEdge, upEdge, rightEdge, bottomEdge is " << leftEdge<<"  "<<upEdge<<"  "<<rightEdge<<"  "<<bottomEdge << endl;
				int ss; cin >> ss;
			}*/

			candPxlElement = (*objBeg)->getPxlElement();
			cout << "candPxlElement size is " << candPxlElement.size() << endl;

			for (UInt verIdx = upEdge; verIdx < bottomEdge; verIdx++)
			{
				for (UInt horIdx = leftEdge; horIdx < rightEdge; horIdx++)
				{

					pxlEleVal = candPxlElement[verIdx - upEdge][horIdx - leftEdge].first;

					if (pxlEleVal != -1)
						m_vPxlEle[verIdx][horIdx] = tempNum;
					//else
						//m_vPxlEle[verIdx][horIdx] = -1;
				}
			}
		}

		++objBeg;
	}


	auto beg = objects.begin();
	while (beg != objects.end())
	{
		auto beg2 = beg + 1;
		while (beg2 != objects.end())
		{
			if (abs(int((*beg)->getColorNum()) - int((*beg2)->getColorNum())) % 14 == 0)
			{

				fout << "In frame " << num << ",  " << "the object " << (*beg)->getMovObjectNum() << " and object " << (*beg2)->getMovObjectNum() << " have the same color  " << (*beg)->getColorNum() << endl;
			}
			beg2++;
		}
		beg++;
	}



#if MASK_ORG
	std::string orgSeqName;
	ifstream inputFile;
	if (m_picSeqNum == 1)
	{
		std::cout << "current test sequence is :" << m_curVideo->getFileName() << endl;
		std::cin >> orgSeqName;
		
		
		inputFile.open(orgSeqName, ios::binary | ios::in);
		while (inputFile.fail())
		{
			cout << "file not exists, please input again" << endl;
			std::cin >> orgSeqName;
			inputFile.open(orgSeqName, ios::binary | ios::in);
		}
		inputFile.close();
		m_curVideo->setOrgFileName(orgSeqName);

	}
	else
		orgSeqName = m_curVideo->getOrgFileName();
	
	inputFile.open(orgSeqName, ios::binary | ios::in);

	UInt frameSkip = this->getCurVideoData()->getFrameNumSkip();

	readStride = (m_picSeqNum + frameSkip - 1)*(m_picHeght*m_picWidth * 3 / 2);
	inputFile.seekg(readStride, ios::beg);

	for (UInt i = 0; i < m_picHeght; i++)
	{
		inputFile.read(reinterpret_cast<char*>(buf), lenY);
		handle.write(reinterpret_cast<char*>(buf), lenY);

	}

	for (UInt vol = 0; vol < uv_frm; vol++)
	{

		inputFile.read(reinterpret_cast<char*>(fub), lenUV);
		for (UInt x = 0; x < lenUV; x++)
		{
			pxlEleVal = m_vPxlEle[vol * 2][x * 2];
			if (pxlEleVal != -1)
				fub[x] = ObjectColor[pxlEleVal % 14][1];
			else
				fub[x] = 128;// ObjectColor[pxlEleVal][1];
		}
		handle.write(reinterpret_cast<Char*>(fub), lenUV);
	}

	for (UInt vol = 0; vol < uv_frm; vol++)
	{
		inputFile.read(reinterpret_cast<char*>(fub), lenUV);
		for (UInt x = 0; x < lenUV; x++)
		{
			pxlEleVal = m_vPxlEle[vol * 2][x * 2];
			if (pxlEleVal != -1)
				fub[x] = ObjectColor[pxlEleVal % 14][2];
			else
				fub[x] = 128;
		}
		handle.write(reinterpret_cast<Char*>(fub), lenUV);
	}

	inputFile.close();


#else 
	for (UInt i = 0; i < m_picHeght; i++)
	{
		for (UInt x = 0; x < lenY; x++)
		{
			pxlEleVal = m_vPxlEle[i][x];
			if (pxlEleVal != -1)
				fub[x] = ObjectColor[pxlEleVal][0];
			else
				fub[x] = 0;
		}
		handle.write(reinterpret_cast<char*>(buf), lenY);
	}


	for (UInt vol = 0; vol < uv_frm; vol++)
	{
		for (UInt x = 0; x < lenUV; x++)
		{
			pxlEleVal = m_vPxlEle[vol * 2][x * 2];
			if (pxlEleVal != -1)
				fub[x] = ObjectColor[pxlEleVal][1];
			else
				fub[x] = 128;// ObjectColor[pxlEleVal][1];
		}
		handle.write(reinterpret_cast<Char*>(fub), lenUV);
	}
	for (UInt vol = 0; vol < uv_frm; vol++)
	{
		for (UInt x = 0; x < lenUV; x++)
		{
			pxlEleVal = m_vPxlEle[vol * 2][x * 2];
			if (pxlEleVal != -1)
				fub[x] = ObjectColor[pxlEleVal][2];
			else
				fub[x] = 128;
		}
		handle.write(reinterpret_cast<Char*>(fub), lenUV);
	}
#endif 

	delete[]UVcomVal;
	delete[]YcomVal;

}

//VideData constructor
VideoData::VideoData(UInt frmhig, UInt frmwid, UInt depth, UInt ctuhig, UInt ctuwid, std::string name ) :m_picNumber(0)
, m_PicCount(0)
, m_uiMaxMovingObjNum(0)
, m_uiInitialObjNum(1)
{
	FileName = name;
	m_handle.open(name, ios::binary | ios::out);
	m_picHeight = frmhig;
	m_picWidth = frmwid;
	m_CtuHeigt = ctuhig;
	m_CtuWid = ctuwid;
	m_depth = depth;
	numsAtBegSkiped = 0;
	maxFrameNum = 1000;

	if (numsAtBegSkiped >= maxFrameNum)
	{
		cout << "numsAtBegSkiped  is not smaller than maxFrameNum, error " << endl;
		exit(0);
	}
	//m_picNumber = maxFrameNum - numsAtBegSkiped;
	m_picNumber = 0;
}

//create a new picture for current video data set
PicData* VideoData::NewPic()
{

	if (m_PicCount >= maxFrameNum || m_PicCount<numsAtBegSkiped)
	{
		m_PicCount++;
			PicData* NewOne = new PicData(m_PicCount, m_picWidth, m_picHeight, m_CtuWid, m_CtuHeigt, m_depth);
		return NewOne;
	}
	else
	{
		m_PicCount++;
		m_picNumber++;
		PicData* NewOne = new PicData(m_PicCount - numsAtBegSkiped, m_picWidth, m_picHeight, m_CtuWid, m_CtuHeigt, m_depth);

		NewOne->setVideo(this);

		m_PicElement.push_back(NewOne);

		return NewOne;
	}
}

PicData* VideoData::getPic(UInt PicNum)
{
	if (PicNum >= m_PicElement.size() || PicNum < 0)
		return NULL;
	else
		return m_PicElement[PicNum];
}

/*
Void VideoData::GoChan()
{
	UInt i = 0;
	PicData* pPic;
	//char LogName[20];
	while (i < m_picNumber)
	{
		printf("deal fram %d\n", i);
		pPic = m_PicElement[i];
		pPic->sortCtus();
		i++;
	}
}
*/
Void VideoData::classifyCtu()
{
	UInt curPicNum = 1;

	//set ctu status according to its residual bits number and cantained CUs
	while (curPicNum < m_picNumber)
	{
		m_PicElement[curPicNum]->preSetCtuSta();
		curPicNum++;
	}
	//second deal with intra CTUs and CTUs 
	curPicNum = 1;
	while (curPicNum < m_picNumber)
	{
		m_PicElement[curPicNum]->reSetCtuSta();
		curPicNum++;
	}
	//if a CTU is surround by  CTUS classified as ctus with motion ,but current is not classified a ctu with motion 
	//set it as a CTU with motion 
	curPicNum = 1;
	while (curPicNum < m_picNumber)
	{
		m_PicElement[curPicNum]->reDealCtuSta();
		curPicNum++;
	}
	curPicNum = 1;
	while (curPicNum < m_picNumber)
	{
		m_PicElement[curPicNum]->reDealCtuSta();
		curPicNum++;
	}
	//set boundary ctus status ,if a CTU is in border area 
	//and it has no neighbor that not belong boundary area,
	//it will be set as background 
	curPicNum = 1;
	while (curPicNum < m_picNumber)
	{
		m_PicElement[curPicNum]->setBorderCtuStatus();
		curPicNum++;
	}

	//while (curPicNum < m_picNumber)
	
		//m_PicElement[curPicNum]->showCTUstatus();
	
		//int s; cin >> s;
	
}
//object segmentation
Void VideoData::objectSegmentation()
{
	MovingObject* curMovingObject;
	UInt curPicNum = 1;
	//set Ctus in a connected region belong to an object.
	curPicNum = 1;
	while (curPicNum < m_picNumber)
	{
		m_PicElement[curPicNum]->objectSegmentation();
		curPicNum++;
	}

	//search prior or next frame in certain area to find corresponding object candidate 
	//and join them up 
	cout << "begin dealing" << endl;
	curPicNum = 1;
	while (curPicNum < m_picNumber)
	{
		m_PicElement[curPicNum]->dealObject();
		curPicNum++;
	}

	cout << "finish dealing" << endl;
	//set each chain as a moving object 
	curPicNum = 1;
	while (curPicNum < m_picNumber)
	{
		m_PicElement[curPicNum]->chainObjectCandidate();
		curPicNum++;
	}
	//remove object candidates that may be noise 
	//if current object candidate is very small and 
	//only background Ctus corresponding in prior or next frame 
	//it will be set as background

	//curPicNum = 10000000;
	//while (curPicNum < m_picNumber)
	//{
	//	m_PicElement[curPicNum]->removNoise();
	//	curPicNum++;
	//}


	auto  tempMovObeBeg = m_vTempMovingObject.begin();
	while (tempMovObeBeg != m_vTempMovingObject.end())
	{
		curMovingObject = *tempMovObeBeg;
		if (curMovingObject->getMovObjectNum() == 0)
		{
			cout << "there is a zero object " << endl;
			int sr; cin >> sr;
		}
		if (curMovingObject->getMovObjectNum() != 0)
			m_mMovingObject.insert(std::make_pair(curMovingObject, true));
		//m_mMovingObject[curMovingObject] = true;
		++tempMovObeBeg;
	}
	m_vTempMovingObject.clear();
}

//
Void VideoData::reSetObject()
{
	cout << "need Karlan filter?(y/n)" << endl;
	std::string ans;
	std::string y;	y = 'y';
	std::string Y;	Y = 'Y';
	cin >> ans;

	if (ans == y || ans == Y)
	{

		cout << "after Kalman filtering,input any key to continue" << endl;
		int debug; cin >> debug;
	}
	else
		return;
	std::string textName = "tra.txt";
	std::string numberName = "newNum.txt";

	auto beg = m_mMovingObject.begin();
	
	while (beg != m_mMovingObject.end())
	{
		cout << "removing noise" << endl;
		
		int i = 0;
		int ObjNum = 0;
		std::ifstream fin;
		std::ifstream finNum;
		UInt ObjectNum = beg->first->getMovObjectNum();
		cout << "here is number " << ObjectNum << endl;
		char number[20];
		sprintf(number, "%d", ObjectNum);
		std::string inputfileName = number;
		std::string inputNumName = number;
		std::string buff;

		inputfileName += textName;
		inputNumName += numberName;

		if (ans == y || ans == Y)
		{
			finNum.open(inputNumName);

			getline(finNum, buff);
			ObjNum = atoi(buff.c_str());

			beg->first->setMovObjectNum(ObjNum);
			
			finNum.close();
		}
		

		//remove the txt files
		
	/*	remove(inputNumName.c_str());
		inputfileName = number;
		inputfileName += "TB.txt";
		remove(inputfileName.c_str());
		inputfileName = number;
		inputfileName += "LR.txt";
		remove(inputfileName.c_str());
		inputfileName = number;
		inputfileName += "tra.txt";
		remove(inputfileName.c_str());*/


		beg++;
	}
	
}

//
Void VideoData::joinBreakChain()
{
	std::multimap<MovingObject*, Bool, OBJECT_LENGTH_COMPARE> tempMovingObject;
	MovingObject* pTempMovingObject;
	UInt maxMovObjNum = m_uiInitialObjNum, extendTurn = 1;
	Bool extended = false, noiseObject = false, shortStride = false;
	resetInitialNum();
	increaseInitialNum();

	//The extendTurn 4 is added, it is used to split the error connected objects.
	//For example, if two objects (A and B) merge into one object(E). And then E split 
	//into two objects ( C and D), it is necessary to tell A is C or A is D.
	while (extendTurn <= 4)
	{
		/*if (extendTurn == 3)
		{
			extendTurn++;
			continue;
		}*/

		// For extendTurn 4, we only need to focus on the short stride objects. Since if 
		// two objects merge into one or one object splits into two objects, these conditions 
		// will only happen in the short stride distance.
		if (extendTurn == 1 || extendTurn == 4)
			shortStride = true;
		else
			shortStride = false;
		cout << "extendTurn is " << extendTurn << endl;

		/*if (extendTurn >=2)
		{
		int hh; cin >> hh;
		}*/


		cout << "begin searching " << endl;
		//extend current moving object to connect the broken
		auto beg = m_mMovingObject.begin();
		while (beg != m_mMovingObject.end())
		{
			if (beg->first->getMovObjectNum() <= maxMovObjNum
				&& beg->first->getMovObjectNum() != 0)
			{
				beg->first->extendChainConstrained(true, shortStride, extendTurn,false);
				beg->first->extendChainConstrained(false, shortStride, extendTurn,false);
			}
			++beg;
		}
		cout << "finish searching " << endl;

		if (extendTurn == 4)
		{
			
				writeInformationToTxt();
				//extendTurn++;
				
				cout << "finish writing" << endl;
				//continue;
				//exit(0);

			//// extention and merge them together
			//beg = m_mMovingObject.begin();
			//while (beg != m_mMovingObject.end())
			//{
			//	maxMovObjNum = m_uiInitialObjNum;
			//	//if (beg->first->getMovObjectNum() <= maxMovObjNum && beg->first->getMovObjectNum() != 0)
			//	//if (beg->first->getMovObjectNum() <= maxMovObjNum)
			//	{
			//		MovingObject* tempObj = beg->first;
			//		while (tempObj->getMovObjectNum() == 0)
			//		{
			//			tempObj = tempObj->getRelateMovObj();
			//		}
			//		//if (beg->first->getHeadExtentionFlag())
			//		if (tempObj->getTailExtentionFlag())
			//			tempObj->resetErrorConnect(extendTurn, false);
			//	}
			//	beg++;
			//}
			
			////new added part***********************************************************************************************
			//			tempMovingObject.clear();
			//			//remove useless moving object 
			//			beg = m_mMovingObject.begin();
			//			while (beg != m_mMovingObject.end())
			//			{
			//				if (beg->first->getMovObjectNum() == 0)
			//				{
			//					beg = m_mMovingObject.erase(beg);
			//					continue;
			//				}
			//
			//				else
			//				{
			//					pTempMovingObject = beg->first;
			//					tempMovingObject.insert(std::make_pair(beg->first, false));
			//					beg++;
			//				}
			//			}
			//			m_mMovingObject.clear();
			//
			//			m_mMovingObject.swap(tempMovingObject);
			//
			//			beg = m_mMovingObject.begin();
			//			while (beg != m_mMovingObject.end())
			//			{
			//				beg->first->releaseChainOutOfData(extendTurn);
			//				++beg;
			//			}
			//
			//			beg = m_mMovingObject.begin();
			//			while (beg != m_mMovingObject.end())
			//			{
			//				if (beg->first->getMovObjectNum() != 0)
			//				{
			//					beg->first->extendChainConstrained(true, shortStride, extendTurn,false);
			//					beg->first->extendChainConstrained(false, shortStride, extendTurn,false);
			//				}
			//				++beg;
			//			}
			////new added part********************************************************************************************************


			// extention and merge them together
			for (auto beg = m_mMovingObject.begin(); beg != m_mMovingObject.end(); beg++)
			{
				maxMovObjNum = m_uiInitialObjNum;
				//if (beg->first->getMovObjectNum() <= maxMovObjNum && beg->first->getMovObjectNum() != 0)
				if (beg->first->getMovObjectNum() <= maxMovObjNum)
				{
					MovingObject* tempObj = beg->first;
					while (tempObj->getMovObjectNum() == 0)
					{
						tempObj = tempObj->getRelateMovObj();
					}
					if (beg->first->getHeadExtentionFlag())
					{
						//tempObj->splitErrorMerge(extendTurn, true);
					}
						
				}
			}

			
			//beg = m_mMovingObject.begin();
			//while (beg != m_mMovingObject.end())
			//{
			//	cout << "eerr5555" << endl;
			//	maxMovObjNum = m_uiInitialObjNum;
			//	MovingObject* tempObj = beg->first;
			//	while (tempObj->getMovObjectNum() == 0)
			//	{
			//		//cout << "changeing" << endl;
			//		tempObj = tempObj->getRelateMovObj();
			//	}
			//	//if (beg->first->getTailExtentionFlag() && tempObj->getMovObjectNum() != 0)
			//	if (tempObj->getTailExtentionFlag())
			//		tempObj->splitErrorMerge(extendTurn, false);
			//	beg++;
			//}
		}

		if (extendTurn <= 3)
		{
			// extention and merge them together
			for (auto beg = m_mMovingObject.begin(); beg != m_mMovingObject.end(); beg++)
			{
				maxMovObjNum = m_uiInitialObjNum;
				if (beg->first->getMovObjectNum() <= maxMovObjNum && beg->first->getMovObjectNum() != 0)
				{
					if (beg->first->getHeadExtentionFlag())
						beg->first->dealConstrainedExtention(shortStride, true);
					if (beg->first->getTailExtentionFlag())
						beg->first->dealConstrainedExtention(shortStride, false);
				}
			}

			//remove useless moving object 
			
			for (auto beg = m_mMovingObject.begin();beg != m_mMovingObject.end();)
			{
				if (beg->first->getMovObjectNum() == 0)
					beg = m_mMovingObject.erase(beg);
				else
				{
					//pTempMovingObject = beg->first;
					tempMovingObject.insert(std::make_pair(beg->first, false));
					++beg;
				}
			}
			m_mMovingObject.clear();

			m_mMovingObject.swap(tempMovingObject);

			if (extendTurn <= 2)
			{
				//deal with condition that more than one object in prior frame correspond to current object

				beg = m_mMovingObject.begin();
				while (beg != m_mMovingObject.end())
				{
					beg->first->dealMergeConstrained(extendTurn);
					++beg;
				}

				////deal with the condition that one object corresponding to more than one object in next frame
				////or many objects correspond to one object in next frame
				beg = m_mMovingObject.begin();
				while (beg != m_mMovingObject.end())
				{
					beg->first->dealSplitConstrained(extendTurn);
					++beg;
				}
			}

			if (extendTurn <= 3)
			{
				beg = m_mMovingObject.begin();
				while (beg != m_mMovingObject.end())
				{
					noiseObject = beg->first->ObjectDeletable(extendTurn);
					if (noiseObject == true)
						beg->first->setMovObjectNum(0);
					beg++;
				}
			}
		}

		beg = m_mMovingObject.begin();
		while (beg != m_mMovingObject.end())
		{
			if (beg->first->getMovObjectNum() == 0)
			{

				beg=m_mMovingObject.erase(beg);
				continue;
			}
			else
			{
				pTempMovingObject = beg->first;
				tempMovingObject.insert(std::make_pair(beg->first, false));
				beg++;
			}
		}

		m_mMovingObject.clear();
		m_mMovingObject.swap(tempMovingObject);
		m_uiInitialObjNum = 1;
		beg = m_mMovingObject.begin();
		while (beg != m_mMovingObject.end())
		{
			beg->first->setMovObjectNum(m_uiInitialObjNum);
			++m_uiInitialObjNum;
			++beg;
		}


		beg = m_mMovingObject.begin();
		while (beg != m_mMovingObject.end())
		{
			beg->first->releaseChainOutOfData(extendTurn);
			++beg;
		}

	

		++extendTurn;
	}

	std::cout << "begin checking " << endl;
	auto tempMovObeBeg = m_mMovingObject.begin();
	UInt curPicNum = 0000001;
	while (tempMovObeBeg != m_mMovingObject.end())
	{
		curPicNum = 0000001;
		while (curPicNum < m_picNumber)
		{
			m_PicElement[curPicNum]->checkHeadFunc(tempMovObeBeg->first);
			curPicNum++;
		}
		tempMovObeBeg++;
	}
	cout << "finish checking" << endl;

}



Void VideoData::segObject()
{
	auto beg = m_mMovingObject.begin();
	while (beg != m_mMovingObject.end())
	{
		beg->first->segment();
		++beg;
	}

}
// detect motion trajectory for every moving object
Void VideoData::trajectoryDetection_Video_On()
{
	
	auto beg = m_mMovingObject.begin();
	while (beg != m_mMovingObject.end())
	{
		cout << endl;
		cout << "travelCand" << endl;
		//beg->first->travelCand();        //
		cout << "setUseFulCand" << endl;
		beg->first->setUseFulCand();
		cout << "setCandGroup" << endl;
		//beg->first->setCandGroup(); 


	//	beg->first->fullfillEmpty();
	//	beg->first->objectTrajectoryDetection();

		//to debug
		beg->first->detectMovObjeTrajectory();

		++beg;
		//beg->first->findCurTarjectory();
	}

	cout << "m_mMovingObject size is " << m_mMovingObject.size() << endl;
}


Void VideoData::writeInformationToTxt()
{
	string dirName = "./objectDatas/";

	DeleteEntireDir(dirName);

	////delete the whole package
	//int aa = _rmdir(dirName.c_str());
	//if (aa == -1)
	//{
	//	cout << "aa is -1" << endl;
	//	int fg; cin >> fg;
	//}
	int aa = _mkdir(dirName.c_str());

	string textName = "basicInformation.txt";
	string outfileName = dirName;
	outfileName += textName;

	fout.open(outfileName);
	if (!fout){
		cout << "file open failed.\n";
		exit(0);//program exit
	}

	UInt totalPicNum = m_picNumber;
	UInt	picHeight=		m_picHeight;
	UInt	picWidth=		m_picWidth;
	UInt	CtuWid=		m_CtuWid;
	UInt	CtuHeight=		m_CtuHeigt;
	UInt	begSkippedPic = numsAtBegSkiped;

	fout << "totalPicNum  " << totalPicNum << endl;
	fout << "picHeight  " << picHeight << endl;
	fout << "picWidth " << picWidth << endl;
	fout << "CtuWidth " << CtuWid << endl;
	fout << "CtuHeight " << CtuHeight << endl;
	fout << "begSkippedPic " << begSkippedPic << endl;

	fout.close();

	writeObjToTxt();
}


Void VideoData::writeObjToTxt()
{
	string dirName = "./objectDatas/";

	int aa=_mkdir(dirName.c_str());
	cout << "create file finished, aa  is " << aa << endl;
	//cin >> aa;
	UInt objNum = 0;

	string textName = "checkObjExit.txt";
	string outfileName = dirName;
	outfileName += textName;

	fout.open(outfileName);
	if (!fout){
		cout << "file open failed.\n";
		exit(0);//program exit
	}
	auto beg = m_mMovingObject.begin();
	while (beg != m_mMovingObject.end())
	{
		MovingObject* tempObj = beg->first;
		objNum = beg->first->getMovObjectNum();
		string objNumStr = to_string(objNum);
		fout << objNumStr << "  ";
		std::map<MovingObject*, Bool> priorObj=tempObj->getPriorMovObj();
		auto begPrior = priorObj.begin();
		while (begPrior != priorObj.end())
		{
			Bool flag = begPrior->second;
			objNumStr = to_string(begPrior->first->getMovObjectNum());
			fout <<"prior  "<< objNumStr << "  "<<flag<<"  ";
			begPrior++;
		}
		std::map<MovingObject*, Bool> NextObj = tempObj->getSubSeqMovObj();
		auto begNext = NextObj.begin();
		while (begNext != NextObj.end())
		{
			Bool flag = begNext->second;
			objNumStr = to_string(begNext->first->getMovObjectNum());
			fout << "next  " << objNumStr << "  " << flag << "  ";
			begNext++;
		}
		fout << endl;
		beg++;
	}
	fout.close();


	beg = m_mMovingObject.begin();
	while (beg != m_mMovingObject.end())
	{
		MovingObject* tempObj = beg->first;
		UInt tempPicNum = 0;
		objNum = tempObj->getMovObjectNum();
		
		std::multimap<UInt, ObjectCandidate*> objCand=tempObj->getObjectCandidates();

		if (objCand.size() == 0)
		{
			cout << "the object's candidate num is zero, writing to file error, exit" << endl;
			exit(0);
		}

		
		auto itCand = objCand.begin();
		while (itCand != objCand.end())
		{
			tempPicNum = itCand->first;
			ObjectCandidate* pCand = itCand->second;
			std::map<CtuData*, Bool, ctuAddrIncCompare> ctu = pCand->getCtuElements();
			vector<UInt> ctuAddr;
			auto begCtu = ctu.begin();
			while (begCtu != ctu.end())
			{
				ctuAddr.push_back(begCtu->first->getCtuAddr());
				begCtu++;
			}
			writeCandToTxt(objNum, tempPicNum, ctuAddr, pCand);
			itCand++;
		}

		beg++;
	}

}

Void VideoData::writeCandToTxt(UInt objNum, UInt picNum, vector<UInt>ctuAddr,ObjectCandidate* pCand)
{

	if (ctuAddr.size() == 0)
		return;
	std::ifstream fin;
	string dirName = "./objectDatas/";
	string objNumStr = to_string(objNum);
	string picNumStr = to_string(picNum);
	string outfileName = dirName;
	outfileName += objNumStr;
	outfileName += "obj_";
	outfileName += picNumStr;
	outfileName += "frame.txt";
	
	Bool existed = false;;
	std::ifstream ftest(outfileName);
	if (!ftest)
	{
		fout.open(outfileName);
	}
	else
	{
		ftest.close();
		fout.open(outfileName, ios::app);
		fout << " ";

	}
	//fout.open(outfileName);
	fout << "  ";
	if (!fout){
		cout << "file open failed.\n";
		exit(0);//program exit
	}

	auto beg = ctuAddr.begin();
	while (beg != ctuAddr.end())
	{
		fout << (*beg)<<"  ";
		beg++;
	}
	fout.close();
	//fout << endl;
	std::map<ObjectCandidate*, Bool> neighCand=pCand->getNeighbourCand();

	outfileName = dirName;
	outfileName += objNumStr;
	outfileName += "obj_";
	outfileName += picNumStr;
	outfileName += "frameNeighbour.txt";

	auto begCand = neighCand.begin();
	if (neighCand.size() > 0)
	{

		fout.open(outfileName, ios::app);
		fout << " ";
		auto begCand = neighCand.begin();
		while (begCand != neighCand.end())
		{
			fout << begCand->first->getMovingOjbect()->getMovObjectNum() << "  " << begCand->first->getPicNum() << "  ";

			begCand++;
		}
		fout.close();
	}
	

}


Bool VideoData::DeleteEntireDir(string dir)
{
	cout << "deleting the files" << endl;
	char* allFile = "*.txt";
	string allFileStr = dir;

	allFileStr += allFile;
	const char* filePointer = allFileStr.c_str();


	long handle = 0;
	_finddata_t file;
	int k;
	k = handle = _findfirst(filePointer, &file);
	string name = dir;
	name += file.name;
	remove(name.c_str());


	while (k != -1)
	{
		k = _findnext(handle, &file);
		//_findnext(handle, &file);
		string name = dir;
		name += file.name;
		//cout << file.name << endl;
		remove(name.c_str());
	}
	int aa = _rmdir(dir.c_str());
	_findclose(handle);
	Bool answer = false;

	cout << "deleting finished" << endl;
	if (aa == -1)
		answer = false;
	else
		answer = true;
	return answer;
}


Void VideoData::setYUVcomponetValue()
{
	UInt curPicNum = 0,color;
	PicData* curPic;
	UInt CtuInWid = m_picWidth / m_CtuWid + (m_picWidth%m_CtuWid ? 1 : 0);
	UInt CtuInHig = m_picHeight / m_CtuHeigt + (m_picHeight%m_CtuHeigt ? 1 : 0);
	CtuData *curCtu;
	Pel *UVcomVal = new Pel[m_CtuWid]();
	Pel *YcomVal = new Pel[m_CtuWid]();
	while (curPicNum < m_picNumber)
	{
		curPic = m_PicElement[curPicNum];
		for (UInt i = 0; i < CtuInHig; i++)
		{
			for (UInt j = 0; j < CtuInWid; j++)
			{
				curCtu = curPic->getCtu(i*CtuInWid + j);
			//	curCtu->createYuvBuf();
				curCtu->ShowDif(color, YcomVal, UVcomVal, NULL);
			}
		}
		curPicNum++;
	}
	delete[]UVcomVal;
	delete[]YcomVal;
}

Void VideoData::detection(Bool Newflag)
{
	if (!m_handle.is_open())
	{
		m_handle.open(FileName, ios::binary | ios::out);
	}

	if(Newflag==false)
	{
		PicData *beg;
		auto nums = m_PicElement.size();
		//setYUVcomponetValue();	//set color in ctu level 
		classifyCtu();			//classify ctus 
		objectSegmentation();	//merge neighbor ctus as an object candidate
		joinBreakChain();		//connect object candidates as a moving object
		trajectoryDetection_Video_On();
		//reSetObject();
		if (!m_PicElement.empty())
		{
			UInt stride = m_PicElement[0]->getStride();
			UChar *buf = new UChar[m_picWidth]();
			UChar *fub = new UChar[(m_picWidth >> 1)]();
			Pel *OneRow = new Pel[stride]();
			Pel *TwoRow = new Pel[(stride / 2)]();

//			CtuData* curCtu
			UInt CtuInWid = m_picWidth / m_CtuWid + (m_picWidth%m_CtuWid ? 1 : 0);
			UInt CtuInHig = m_picHeight / m_CtuHeigt + (m_picHeight%m_CtuHeigt ? 1 : 0);
			PicData* bufPic = new PicData(m_picNumber, m_picWidth, m_picHeight, m_CtuWid, m_CtuHeigt, m_depth);

			std::vector<std::vector<Int>>	m_vPxlElement;
			std::vector<std::vector<Int>>	m_vPxlElement2;
		
			//set the height and width CtuInHig*m_CtuHeigt and CtuInWid*m_CtuWid but not picHeight or picWidth because of some reasons

			cout << "m_CtuHeigt is " << m_CtuHeigt << endl;
			
			std::vector<Int> pxlRow(CtuInWid*m_CtuWid, -1);
			for (UInt verIdx = 0; verIdx < CtuInHig*m_CtuHeigt; verIdx++)
			{
				//std::vector<Int> pxlRow(CtuInWid*m_CtuWid, -1);
					m_vPxlElement.push_back(pxlRow);
					m_vPxlElement2.push_back(pxlRow);
				//pxlRow.clear();
			}
		
			cout << "m_vPxlElement size is " << m_vPxlElement.size() << endl;

			string textName = "sameColorObjects.txt";
			fout.open(textName);
			if (!fout){
				cout << "file open failed.\n";
				exit(0);//program exit
			}

			queue<UInt> usefulColors, uselessColors;
			for (UInt i = 1; i <= 7; i++)
				usefulColors.push(i);
			for (UInt i = 8; i <= 14; i++)
				uselessColors.push(i);

			UInt pic = 0;
			printf("\n frame number 1");
			while (nums)
			{
				std::vector<std::vector<Int>> tempElement;
				m_vPxlElement.clear();
				m_vPxlElement.swap(tempElement);
				m_vPxlElement.assign(m_vPxlElement2.begin(), m_vPxlElement2.end());
				//m_vPxlElement = m_vPxlElement2;
			
				beg = m_PicElement[pic];
				beg->PicNewWrite(m_handle, buf, fub, OneRow, TwoRow, bufPic, pxlRow, m_vPxlElement, usefulColors, uselessColors);
		
				m_PicCount--;
				nums--;
				pic++;
			}
			fout.close();
			//std::cout << "eel" << endl;

		delete bufPic;
		delete[]buf;
		delete[]fub;
		delete[]OneRow;
		delete[]TwoRow;
		//cout << "tttl" << endl;
		}
	}
}

VideoData::VideoData()
{
}

VideoData::~VideoData()
{
//	UInt objNum = (UInt)m_vcMovObj.size();
//	for (UInt i = 0; i < objNum; i++)
//		delete m_vcMovObj[i];

	//UInt picNumber = m_PicElement.size();
//	for (UInt i = 0; i < picNumber; i++)
//		delete m_PicElement[i];
	if (m_handle.is_open())
		m_handle.close();
}
