//#define _CRT_SECURE_NO_WARNINGS

//#ifdef _SECURE_SCL
//#undef _SECURE_SCL
//#define _SECURE_SCL 1
//#else 
//#define _SECURE_SCL 1
//#endif
#pragma once 
#include "TLibCommon\TypeDef.h"
#include "TLibCommon\TComYuv.h"
#include "TLibCommon\TComRom.h"
#include "TLibCommon\TComMv.h"
#include <utility>
#include <vector>
#include <queue>
#include <string>
#include <deque>
#include <map>
#include <fstream>
#include <memory.h>
#include <errno.h>
#include <direct.h>
#include <algorithm>
#include <math.h>
#include <iostream>
#include<direct.h>
#include<sstream>  
using namespace std;
#define MASK_ORG 1
#define TEST_NEW 1
#define FRAME_STRIDE 5
//#define FRAME_STRIDE 5
#define CHECK_STRIDE 10
#define CONSISTAN_GROUP_NUMBER 4
#define CHECK_GROUP_NUMBER 3
#define CTUdevMovSize 4
#define CTUsize 32
const UInt Threshold = 3;//8;
const UInt threshold = 3;
const UInt MinCUSize = 4;
const UInt FilterSize = 4;

//classes 
class PuUnit;
class CUData;
class CtuData;

class PicData;

class VideoData;

class ObjectCandidate;
class MovingObject;

template <class T> class Vector;

typedef std::multimap<std::vector<ObjectCandidate*>, std::map< std::pair<Int, Int>, Float>> MatchedGroup;
// 
enum SortBases
{
	NOTSORTED, BY_INTRA_MODE, BY_SMALL_MVD, BY_BIG_MVD, BY_REFER_SPATIAL, BY_REFER_TEMPERAL, BY_ZERO_MV
};
//
enum Status
{																								//TEST mode 
	STA_UNSORTED, STA_BG, STA_MOVOBJ, STA_NOTSURE, STA_INTRA, STA_POSSIBLE_BG, STA_POSSIBLE_OBJ, STA_POSSIBLE_BOUNDRY, NOSTATUS = 15
};

enum MotionDirection
{
	NO_DIRECTION, MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN
};

enum motionDir{ forwardDir, backwardDir, noMove };

struct RankPuByDir
{
	Bool operator()(const MVP_DIR& firDir,const MVP_DIR& sedDir)
	{
		return (Int)firDir < (Int)sedDir;
	}
};


//prediction unit class
class PuUnit
{
private:
	UInt				m_uPartIdx;		  // index in current CB
	UInt				m_auMvd[2];
	UInt				m_coordx;		  // coordinate in a CTU
	UInt				m_coordy;
	UInt				m_PuWidth;        // width and heigth
	UInt				m_PuHeigh;

	UInt				m_uScanIndex;     //z-scan order index in LCU
	UInt				m_uRefFrmIndex;      // ref_frame_number

	UInt				m_PuAddInPicX;       // coordinate in a picture 
	UInt				m_PuAddInPicY;
	UInt				m_uScale;
	Status				m_status;		//  Pu status
	Status				m_ePreStatus;	//previous status if current status changed
	SortBases			m_SortBasis;

	Bool				m_bTravel;
	Bool				m_skipFlag;		// curPu is skip model
	Bool				m_mergFlag;        //MV prediciton is Merge Model
	Bool				m_bZeroCand;    //t: mvp is zero 
	Bool				m_bTempFlag;		//t: if temporal_refenerce Pu can be used
	Bool				m_bStaSettled;
	Bool				m_bMvEqlZero;    //t: the mv of current pu is zero 
	Bool				m_bMvUeqlZero;
	Bool				m_statueChanged;

	/*  MV  */
	Int					m_iHorMV;
	Int					m_iVerMV;
	/************************************************************
	flags of  available neighbor PU 
	*************************************************************/
	Bool				m_bA1Avail;
	Bool				m_bB1Avail;
	Bool				m_bB0Avail;
	Bool				m_bA0Avail;
	Bool				m_bB2Avail;
	/****************************************************************************************
	for AMVP mode
	*****************************************************************************************/
	Bool				m_MvpIsSure;   //t: if MVP could be sure in the condition of AMVP 
	UInt				m_uAMVPIndex;		//t: index of AMVP
	UInt				m_uSmvpNum;		//t: number of avaiable spatial_mvp
	Bool				m_SmvpEqual;
	Bool				m_SmvpMerged;   //t: whether could determine two avaiable SMVP are the same 
	Bool				m_bSuspectFlag;

	/*****************************************************************************************
	flags for merge model
	******************************************************************************************/
	UInt				m_uMergeIndex;       //t: Merge MV Index 
	Bool				m_bMrgCandIsSure;		//t: which candidate is used is sure 
	Bool				m_bMrgStatusIsSure;		//the status of candidate is sure
	Bool				m_bMrgBaseIsSure;		//the start of merge chain is sure
	Bool				m_MergeWihtZeroMv;  //t: only zero_mv can be used

	
	CtuData*			m_curCtu;
	CUData*				m_curCb;
	PuUnit*				m_pcRefPu;   //t: first reference Pu for mvp
	PuUnit*				m_pcSecRefPu;	//t: second reference PU for MVP
	PuUnit*				m_pcTempPu;
	PuUnit*				m_pcAmvpRefPu;    //t: the right reference PU for AMVP mode
	PuUnit*				m_pcMergBasePu;

	PuUnit*				m_pcPuMergeCurr;
	//neighbor pus 
	PuUnit*					m_pLTPu;	//top left 
	PuUnit*					m_pLBPu;	//left bottom
	PuUnit*					m_pRTPu;	//right top 
	PuUnit*					m_pRBPu;	//right bottom 
	MVP_DIR					m_sMvpDir[2];	//spatial MV candidates direction 
	MVP_DIR					m_tMvpDir;		
	PredMode				m_ePredMode;	

	std::vector<PuUnit*>	m_vPusReferCurr;        
//	SubArea*				m_subArea;
	//candidate pu for merge model
	std::map<MVP_DIR, PuUnit*, RankPuByDir>		m_mMergeCandidatePus;

public:
	PuUnit();
	PuUnit(UInt num);
	~PuUnit();
	/* set and get fake MVs */
	/*not sure if this will be needed */
	Void destroy();
	Void setHorMV(Int horVal)										{ m_iHorMV = horVal; }
	Int  getHorMV()													{ return m_iHorMV; }
	Void setVerMV(Int verVal)										{ m_iVerMV = verVal; }
	Int  getVerMV()													{ return m_iVerMV; }

	Void setPredMode(PredMode mode)									{ m_ePredMode = mode; }
	PredMode getPredMode()											{ return m_ePredMode; }
	Void setTravel(Bool flag)										{ m_bTravel = flag; }
	Bool getTravel()												{ return m_bTravel; }
	Void setMvd(UInt x, UInt y)										{ m_auMvd[0] = x; m_auMvd[1] = y; }
	Void getMvd(UInt &x, UInt &y)									{ x = m_auMvd[0]; y = m_auMvd[1]; }
	Void setSmvpEqualFlag(Bool flag)								{ m_SmvpEqual = flag; }
	Bool getSmvpEqualFlag()											{ return m_SmvpEqual; }
	Void setPuWidth(UInt wid)										{ m_PuWidth = wid; }
	UInt getPuWidth()												{ return m_PuWidth; }
	Void setPuHeight(UInt hig)										{ m_PuHeigh = hig; }
	UInt getPuHeight()												{ return m_PuHeigh; }

	Void setMergFlag(Bool flag)										{ m_mergFlag = flag; m_auMvd[0] = 0; m_auMvd[1] = 0; }
	Void getMergFlag(Bool &flag)									{ flag = m_mergFlag; }
	Bool getMergflag()												{ return m_mergFlag; }

	Void setMergIndex(UInt num)										{ m_uMergeIndex = num; }
	UInt getMergIndex()												{ return m_uMergeIndex; }

	Void setPUnum(UInt num)											{ m_uPartIdx = num; }
	Void getNum(UInt &num)											{ num = m_uPartIdx; }

	Void setRefFrmInd(UInt Refnum)									{ m_uRefFrmIndex = Refnum; }
	UInt getRefFrmInd()												{ return m_uRefFrmIndex; }

	Void setMvdPInd(UInt MvdPInd)									{ m_uAMVPIndex = MvdPInd; }
	UInt getMvPInd()												{ return m_uAMVPIndex; }

	Void setStatus(Status sta)										{ m_status = sta; }
	Void changeStatus(Status newStatus)								{ m_ePreStatus = m_status; m_status = newStatus; m_statueChanged = true; }
	Status getStatus()												{ return m_status; }
	Status getPreStatus()											{ return m_ePreStatus; }
	Bool getStaChangFlag()											{ return m_statueChanged; }
	Void setStaChangFlag(Bool changed_flag)							{ m_statueChanged = changed_flag; }
	Void resetSta()													{ m_status = m_ePreStatus; }

	Void PredChang();

	Void setCurCtu(CtuData* cur)									{ m_curCtu = cur; }
	CtuData* getCtu()												{ return m_curCtu; }
	Void setCurCb(CUData* curcb)									{ m_curCb = curcb; }
	CUData* getCb()													{ return m_curCb; }

	Void setXY(UInt x, UInt y)										{ m_coordx = x; m_coordy = y; }
	Void getXY(UInt &x, UInt &y)									{ x = m_coordx; y = m_coordy; }

	Void setZeroCand(Bool flag)										{ m_bZeroCand = flag; }
	Bool getZeroCand()												{ return m_bZeroCand; }

	Void setNullPredPu()											{ m_bMvEqlZero = false; m_pcRefPu = NULL; m_pcSecRefPu = NULL; }
	/////////////////////////////////////////
	Void setA1Flag(Bool flag)			{ m_bA1Avail = flag; }
	Bool getA1Flag()					{ return m_bA1Avail; }
	Void setB1Flag(Bool flag)			{ m_bB1Avail = flag; }
	Bool getB1Flag()					{ return m_bB1Avail; }
	Void setB0Flag(Bool flag)			{ m_bB0Avail = flag; }
	Bool getB0Flag()					{ return m_bB0Avail; }
	Void setA0Flag(Bool flag)			{ m_bA0Avail = flag; }
	Bool getA0Flag()					{ return m_bA0Avail; }
	Void setB2Flag(Bool flag)			{ m_bB2Avail = flag; }
	Bool getB2Flag()					{ return m_bB2Avail; }
	//////////////////////////////////////////////////////////////////
	Void setTmpFlag(Bool flag)										{ m_bTempFlag = flag; }
	Bool getTmpFlag()												{ return m_bTempFlag; }

	Void setTmpPu(PuUnit* ppu)										{ m_pcTempPu = ppu; }
	PuUnit* getTmppu()												{ return m_pcTempPu; }

	Void setSecSppu(PuUnit* ppu)									{ m_pcSecRefPu = ppu; }
	PuUnit* getSecPredPu()											{ return m_pcSecRefPu; }

	Void setFinal(PuUnit* ppu)										{ m_pcAmvpRefPu = ppu; }
	PuUnit* getFinal()												{ return m_pcAmvpRefPu; }
	Void addMvpCandidate(MVP_DIR mvpDir, PuUnit* pcPu)				{ m_mMergeCandidatePus[mvpDir] = pcPu; }
	std::map<MVP_DIR, PuUnit*, RankPuByDir>  getDirAndPu()			{ return m_mMergeCandidatePus; }
	Void setRefPU(PuUnit* ppu)
	{
		if (m_pcRefPu == NULL)m_pcRefPu = ppu;
		else if (m_pcSecRefPu == NULL)setSecSppu(ppu);
		else
		{
			printf("too much SMVP RefPu \n%s %s \n", __FILE__,__FUNCTION__,__LINE__);
			printf("system error");
		}
	}

	PuUnit* getPpu()											{ return m_pcRefPu; }

	Bool getSetFlag()											{ return m_bStaSettled; }
	Void setSetFlag(Bool flag)									{ m_bStaSettled = flag; }

	Void setMergeWithZeroMv(Bool flag)							{ m_MergeWihtZeroMv = flag; }
	Bool getMergeWithZeroMv()									{ return m_MergeWihtZeroMv; }

	Void setSkipFlag(Bool fg)									{ m_skipFlag = fg; }
	Bool getSkipFlag()											{ return m_skipFlag; }

	Void setMergeCandIsSure(Bool bFlag)							{ m_bMrgCandIsSure = bFlag; }
	Bool getMergeCandIsSure()									{ return m_bMrgCandIsSure; }
	Void setStatusIsSure(Bool flag)								{ m_bMrgStatusIsSure = flag; }
	Bool getStatusIsSure()										{ return m_bMrgStatusIsSure; }
	Void setMrgBaseIsSure(Bool flag)							{ m_bMrgBaseIsSure = flag; }
	Bool MergeBaseIsSure()										{ return m_bMrgBaseIsSure; }

	Void setSuspectFlag(Bool flag)								{ m_bSuspectFlag = false; }
	Bool getSuspectFlag()										{ return m_bSuspectFlag; }


	Void setSpatMvpDir(MVP_DIR dir)								{ if (m_sMvpDir[0] == MD_NONE)m_sMvpDir[0] = dir; else m_sMvpDir[1] = dir; }
	Void setTempMvpDir(MVP_DIR dir)								{ m_tMvpDir = dir; }
	MVP_DIR getSpaMvpDir(UInt num)								{ return m_sMvpDir[num]; }
	MVP_DIR getTmpMvpDir()										{ return m_tMvpDir; }

	Void setScale(UInt scale)									{ m_uScale = scale; }
	UInt getScale()												{ return m_uScale; }

	Void setSmvpNum(Int num)									{ m_uSmvpNum = num; }
	Int getSmvpNum()											{ return m_uSmvpNum; }

	Void setZeroMv(Bool flag)									{ m_bMvEqlZero = flag; }
	Bool getZeroMv()											{ return m_bMvEqlZero; }
	Void setMvUeqlZero(Bool flag)								{ m_bMvUeqlZero = flag; }
	Bool getMvUeqlZero()										{ return m_bMvUeqlZero; }

	Void setBaseMergPu(PuUnit* pu)								{ m_pcMergBasePu = pu; }
	PuUnit* getBaseMergPu()										{ return m_pcMergBasePu; }

	Void setPuMergeCur(PuUnit* pPu)								{ m_pcPuMergeCurr = pPu; }
	PuUnit* getPuMergeCur()										{ return m_pcPuMergeCurr; }

	Void setsMvpMergeFlag(Bool flag)							{ m_SmvpMerged = flag; }
	Bool getsMvpMergeFlag()										{ return m_SmvpMerged; }

	Void setMvpIsSure(Bool flag)								{ m_MvpIsSure = flag; }
	Bool getMvpIsSure()											{ return m_MvpIsSure; }

	Void setPuAbsAddrXY(UInt x, UInt y)							{ m_PuAddInPicX = x; m_PuAddInPicY = y; }
	Void getPuAbsAddrXY(UInt &x, UInt &y)						{ x = m_PuAddInPicX; y = m_PuAddInPicY; }
	UInt getPuAbsAddrX()										{ return m_PuAddInPicX; }
	UInt getPuAbsAddrY()										{ return m_PuAddInPicY; }

	Void setAbsPartInd(UInt ind)								{ m_uScanIndex = ind; }

	UInt getAbsPartInd()										{ return m_uScanIndex; }
	Void DealWithTwoSmvp();
	Void dealTwoSmvp();
	Void setSortBasis(SortBases basis)							{ m_SortBasis = basis; }
	SortBases getSortBasis()									{ return m_SortBasis;  }
	//get neighbor PUS 
	Void setLTPu(PuUnit* LTPu)									{ m_pLTPu = LTPu; }
	PuUnit* getLTPu()											{ return m_pLTPu; }
	Void setLBPu(PuUnit* LBPu)									{ m_pLBPu = LBPu; }
	PuUnit* getLBPu()											{ return m_pLBPu; }
	Void setRTPu(PuUnit* RTPu)									{ m_pRTPu = RTPu; }
	PuUnit* getRTPu()											{ return m_pRTPu; }
	Void setRBPu(PuUnit* RBPu)									{ m_pRBPu = RBPu; }
	PuUnit* getRBPu()											{ return m_pRBPu; }

//	Void setCurSubArea(SubArea* pSubArea)						{ m_subArea = pSubArea; }
//	SubArea* getCurSubArea()									{ return m_subArea; }

	UInt getAreaSize()											{ return m_PuWidth*m_PuHeigh; }
	Void getInfoFromRefPu(PuUnit* refPu, Bool tmpFlag);
	//Void setArea();//							{ m_Area = area; }//area->addNewPu(this); }
	std::vector<PuUnit*> getPusReferCur()						{ return m_vPusReferCurr; }
	Void addPusReferCurr(PuUnit* pPU)							{ m_vPusReferCurr.push_back(pPU); }

	/*try to find the right merge candidate or reduce the size of possible candidates*/
	Void searchMergeCandidate();
	/*try to find the right mvp for AMVP mode */
	Void searchAMVP();
	/********************************************************
	find relationship between one and another merge candidat
	!!!!!!they may should be re-writen!!!!!!!!!!!!!!!!!!!!!!!!!
	********************************************************/
	// above and above right 
	Bool relationAboveAndAR(Bool &equalFlag);// B1 & B0
	//relation left above and above 
	Bool relationAboveAndLA(Bool &equalFlag); // B1 & B2
	//relation between left and above 
	Bool relationLeftAndAbove(Bool &equalFlag);// A1 & B1
	//relation between left and leftAbove 
	Bool relationLeftAndLA(Bool &equalFlag);//A1 & B2
	//relation between leftbottom and left 
	Bool relationLBAndLeft(Bool &equalFlag);//A1 & A0

	/////////////////////////////////
	//below left and above right 
	Bool relationBLAndAR(Bool &equalFlag); // A0 & B0
	//below left and above 
	Bool relationBLAndAbove(Bool &equalFlag); // A0 & B1
	//below left and above left 
	Bool relationBLAndAL(Bool &equalFlag); // A0 & B2
	// above left and above right
	Bool relationALAndAR(Bool &equalFlag);// B2 & B0
	//left and above right
	Bool relationLeftAndAR(Bool &equalFlag);//A1 & B0
	//
};

//coding block class
class CUData
{
private:
	UInt					m_puNums;		//t: number of PUs of current CB
	UInt					m_iCBSizeInBits;	//total coded number of bits 
	UInt					m_CBWidth;		
	UInt					m_CBheight;
	UInt					m_cuDepth;			
	UInt					m_coord[2];    //t: coordinate in current CTU/LCU
	PredMode				m_PredInf;			//t: type of predicion
	PartSize				m_PartSize;		//t: partion type 
	CtuData*				m_curCtu;	
	std::vector<PuUnit*>	m_PuUnits;	
public:
	CUData();
	~CUData();
	CUData(UInt wid, UInt hig, UInt x, UInt y, UInt dep);
	Void setCBSizeInBits(UInt bitsNum)												{ m_iCBSizeInBits = bitsNum; }
	UInt getCBSizeInBits()															{ return m_iCBSizeInBits; }
	Void setPreMode(PredMode info)													{ m_PredInf = info; }
	PredMode getPreMode()																{ return m_PredInf; }
	UInt getx()																		{ return m_coord[0]; }
	UInt gety()																		{ return m_coord[1]; }
	Void setPartSize(PartSize pars)													{ m_PartSize = pars; }
	PartSize getPartSize()															{ return m_PartSize; }
	Void setPuNums(UInt num)														{ m_puNums = num; }
	UInt getPuNums()																{ return m_puNums; }
	UInt getCBWidth()																{ return m_CBWidth; }
	UInt getCBHeight()																{ return m_CBheight; }
	Void setDepth(UInt dep)															{ m_cuDepth = dep; }
	UInt getDepth()																	{ return m_cuDepth; }
	Void setCurCtu(CtuData* cur)													{ m_curCtu = cur; }
	CtuData* getCtu()																{ return m_curCtu; }


	/////////////set color value in different way /////////////////////////////////////////////////////////////////////////////////////
	Void setCBAndCUColor(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv, Pel* Ycom, Pel* UVcom, UInt colorIndex);
	/*ste Y U V components value for current CB*/
	Void setCBcolor(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv, Pel* Ycom, Pel* UVcom,UInt colorIndex);
	//set Y,U,V components value
	Void setYUVcompValue(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv, Pel* Ycom, Pel* UVcom);
	//this function is just set U,V component values
	Void setUVComponent(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv, Pel* Ycom, Pel* UVcom);
	Void AsigElem(std::vector<Pel*> Elemy, std::vector<Pel*> Elemu, std::vector<Pel*> Elemv);

	Void addNewPart(PuUnit* NewPu);
	Void setMinCUs();

	//to classify PU ,not used now 
	//Void Classify();
	Void GoPUs();
};
// class coding unit tree Or largest coding unit 
class CtuData
{
private:
	Status					m_eCtuStatus;			//CTU status 
	Status					m_eInitialSta;				//CTU status, classified by CUS and  nuber of bits
	Bool					m_bMovingFlag;		//current contain PUs that classified as moving PU
	Bool					m_bBigMvdFlag;		//current ctu contain PUs that with big MVD 
	Bool					m_bTraveled;		//if current ctu istraved 
	Bool					m_bBorderCtu;		//this flag means whether current is  on the edge of current frame
	Bool					m_bEdgeCtu;			//this means whether current ctu is on the edge of current object
	Bool					m_bCentCtu;
	UInt					m_uiSKipPortion;	//propotion of  skip PUs 
	UInt					m_fIntraPortion;
	UInt					m_uiBitsNum;		//total bits number of current CTU
	UInt					m_uiResidBitsNum;	// totoal bits number of residual bits number 
	UInt					m_CtuAddr;			//address in ctu level in current frame 
	UInt					m_CtuWidth;			//width  of ctu
	UInt					m_CtuHeight;			//height of ctu 
	UInt					m_uvHeight;			//uv component height and width 
	UInt					m_uvWidth;

	UInt					m_depth;			//max divided depth of current ctu 

	UInt					m_absX;				//coordinate 
	UInt					m_absY;
	//	UInt m_CtuCod[2];
	PicData*				m_pPic;				//
	ObjectCandidate*		m_pCurObj;			//object in current picture that ctu belong  to 
	std::vector<CUData*>	m_CbData;			//coding blocks in current ctu

	std::vector<Pel*>		m_OrgY;
	//	Pel**					m_pYcomponent;
	std::vector<Pel*>		m_OrgU;
	//Pel**					m_pUcomponent;
	std::vector<Pel*>		m_OrgV;
	//	Pel**					m_pVcomponent;
	//raser scan order
	std::vector<PuUnit**>	m_puPointers;		//pointers to each PU element 
	SliceType				m_SliType;			//slice type 
	UInt					oderOfSmallCtu;      //order should be 0,1,2,3
	UInt					numofPartInSmallCtu;  //number of partitions in small CTU
public:
	CtuData();
	~CtuData();
	CtuData(UInt ctuaddr, UInt depth, UInt ctuwid, UInt ctuhig);

	Void createYuvBuf(Bool outputFlag);
	Void copyYuvBuf(CtuData *pCtu);
	Void freeYuvBuf();

	Void setMovingFlag(Bool flag)													{ m_bMovingFlag = flag; }
	UInt getDept()																	{ return m_depth; }
	Bool getMovingFlag()															{ return m_bMovingFlag; }
	Void setBigMvdFlag(Bool flag)													{ m_bBigMvdFlag = flag; }
	Bool getBigMvdFlag()															{ return m_bBigMvdFlag; }
	Void setTraveledFlag(Bool flag)													{ m_bTraveled = flag; }
	Bool getTraveledFlag()															{ return m_bTraveled; }
	Void setBorderFlag(Bool flag)													{ m_bBorderCtu = flag; }
	Bool getBorderFlag()															{ return m_bBorderCtu; }
	Void setEdgeFlag(Bool edgeFlag)													{ m_bEdgeCtu = edgeFlag; }
	Bool getEdgeFlag()																{ return m_bEdgeCtu; }
	Void setCentFlag(Bool cent)														{ m_bCentCtu = cent; }
	Bool getCentFlag()																{ return m_bCentCtu; }
	Void addSkipPortion(UInt sum)													{ m_uiSKipPortion += sum; }
	Float getSkipProtion()															{ return (Float)m_uiSKipPortion / ((m_CtuWidth >> 2)*(m_CtuHeight >> 2)); }
	Void addIntraProtion(UInt intraSize)											{ m_fIntraPortion += intraSize; }
	Float getIntraPortion()															{ return (Float)m_fIntraPortion / ((m_CtuWidth >> 2)*(m_CtuHeight >> 2)); }
	Bool intraCtu();	//if current CTU is coded as intra 
	void setOrderOfSmallCtu(UInt order);
	UInt getOrderOfSmallCtu()														{ return 	oderOfSmallCtu; }
	void setPartInSmallCtu(UInt part)												{ numofPartInSmallCtu = part; }
	UInt	getPartInSmallCtu()														{ return	numofPartInSmallCtu; }  //number of partitions in small CTU
	//get neighbor CTUS surround current CTU 
	CtuData* getLeftNeighbor();
	CtuData* getLeftTopNeighbor();
	CtuData* getBottomLeftNeighbor();

	CtuData* getUpNeighbor();
	CtuData* getRightTopNeighbor();
	CtuData* getRigNeighbor();
	CtuData* getDownNeighbor();
	CtuData* getBotRightNeighbor();
	CtuData* getNeighbourCTU(UInt num);
	
	UInt getCtuAddr()	const															{ return m_CtuAddr; }
	Void setCtuAddr(UInt addr)															{ m_CtuAddr = addr; }
	Void setBitsNum(UInt bitsNum)														{ m_uiBitsNum = bitsNum; }
	UInt getBitsNum()																	{ return m_uiBitsNum; }
	Void setResidBits(UInt bitsNum)														{ m_uiResidBitsNum += bitsNum; }
	UInt getResidBits()																	{ return m_uiResidBitsNum; }
	UInt getCtuHeight()																	{ return m_CtuHeight; }
	UInt getCtuWid()																	{ return m_CtuWidth; }
	UInt getMaxDepth();
	std::vector<CUData*> getCB()														{ return m_CbData; }
	//void totalCuInSmallCtu(std::vector<UInt>& totalNum);	//partition each CTU into four small CTUs
	//void totalCuInSmallCtu(UInt& totalNum, UInt order);	//partition each CTU into four small CTUs	
	//void partInSurroundSmallCtu(UInt order, vector<UInt>& totalNum,UInt& atPicEdge);	//partition in 8 surround small CTUs
	

	Pel* getOrgY(UInt n)																
	{
		auto size = m_OrgY.size();
		return m_OrgY[n]; 
	}
	Pel* getOrgU(UInt n)																{ return m_OrgU[n]; }
	Pel* getOrgV(UInt n)																{ return m_OrgV[n]; }

	Void setAbsXY(UInt x, UInt y)														{ m_absX = x; m_absY = y; }
	Void getAbsXY(UInt &x, UInt &y)														{ x = m_absX; y = m_absY; }
	UInt getAbsx()																		{ return m_absX; }
	UInt getAbsy()																		{ return m_absY; }
	Void setYcom();///////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	PuUnit** getPup(UInt n)																{ return m_puPointers[n]; }
	UInt getCUNum()																		{ return (UInt)m_CbData.size(); }
	Void addNewCB(CUData* cb)															{ m_CbData.push_back(cb); }
	PuUnit* getPu(UInt num)
	{
		// m_CtuHig;
		UInt y = num / (m_CtuHeight >> 2);
		UInt x = num % (m_CtuWidth >> 2);
		return m_puPointers[y][x];
	}
	PuUnit* getPuTopLeft(UInt num)
	{
		UInt y = (num / (m_CtuHeight >> 2)) >> 2;
		UInt x = (num % (m_CtuWidth >> 2)) >> 2;
		x = x << 2;
		y = y << 2;
		return m_puPointers[y][x];
	}
	PuUnit* getPuXY(UInt x, UInt y)
	{
		if (m_puPointers[y][x] == NULL)
		{
			system("pause");
			return NULL;
		}
		else  return m_puPointers[y][x];
	}
	//
	Void setCtuSta(Status sta)							{ m_eCtuStatus = sta; }
	Status getCtuSta()									{ return m_eCtuStatus; }
	CtuData* getCorCtu(Bool subSeqFlag);

	//classify ctu status by CUs and residual bits 
	Void classifyCtuStatus();
	//deal with CTUs that have small amount of residual bits number
	//if current ctu has neighbor CTU calssified as part of an object, 
	//and the corresponding ctu in next or prior frame also classified as part of an object,
	//current ctu will be calssified as part of an object
	Void possibleMotion();
	Void checkPosition();
	//deal with  ctus not classified ,but   surrounded by ctus classified as part of an object.
	Void reDealCtu();
	Void travelNeighborCtu(ObjectCandidate* pObj, Bool recoverFlag);
	//set Y U V component value
	Void ShowDif(UInt &color_num, Pel* Ycom, Pel* UVCom,CtuData* pCtu);

	Void addOrgYUV(UInt corx, UInt cory, TComYuv* pCom);

	Void setSliType(SliceType tp)								{ m_SliType = tp; }
	Void getSliType(SliceType &tp)								{ tp = m_SliType; }

	Void setPic(PicData* ppic)									{ m_pPic = ppic; }
	PicData* getPic()											{ return  m_pPic; }

	Void setObject(ObjectCandidate* pObj)						{ m_pCurObj = pObj; }
	ObjectCandidate* getObject()								{ return m_pCurObj; }

	/*=====================================================================================*/
	//classify PUs ,not used now 
	//Void Classify();
	//trace back to classify each PU ,not used now 
	Void GoPuChain();
//	Void preSetCtuStatus();
	Void setYuvBuf();
};


//this will not need any more...
/***********************
struct ObjectCoreElement
{
	std::vector<std::vector<Int>> m_vElement;
//	std::vector<std::vector<Int>> m_mEetendCore;
	UInt m_uiMinSize;
	UInt height;
	UInt width;
	UInt ultraUp;
	UInt ultraLeft;
	UInt ultraRight;
	UInt ultraBottom;
};
*********************************/
//detected motion information 
struct PicOrder
{
	Bool operator ()(const UInt firstNum, const UInt secNum)
	{
		return firstNum < secNum;
	}
};

struct motionInfo
{
	motionInfo() :motionDir(NO_DIRECTION), multiDelta(false)
	{
		deltaVal[0] = 0;
		deltaVal[1] = 0;
		probVal[0] = 0;
		probVal[1] = 0;
	}
	MotionDirection     motionDir;			//moving direction:

	Bool				multiDelta;
	UInt				deltaVal[2];		//shift value
	Float				probVal[2];			//probability
};
struct motionInformation
{
	motionInformation() :deltaX(0),
		deltaY(0), 
		probaVal(0)
	{}
	Int deltaX;
	Int deltaY;
	Float probaVal;
};
struct deltaInfo
{
	deltaInfo() :deltaX(0),
		deltaY(0),
		xWayRatio(0),
		yWayRatio(0)
	{}
	Int deltaX;
	Int deltaY;
	Float xWayRatio;
	Float yWayRatio;
};
struct deltCompare
{
	Bool operator()(const motionInfo *dirOne, const motionInfo *dirTwo)
	{
		return dirOne->deltaVal < dirTwo->deltaVal;
	}
};
//compare ctu addresses, in raster scan order 
struct ctuAddrIncCompare
{
	Bool operator()(const CtuData* areOne, const CtuData* areaTwo)
	{
		return areOne->getCtuAddr() < areaTwo->getCtuAddr();
	}
};

class ObjectCandidate
{
public:
	struct candInPicOrder
	{
		Bool operator()(const ObjectCandidate *firCand, const ObjectCandidate *secCand)
		{
			return firCand->getPicNum() < secCand->getPicNum();
		}
	};
private:
	UInt						m_uiCurObjNum;		//order number  in current picture
	UInt						m_uiPicNum;			//frame number
	//the two parameters are used to set matched well candidates as a group
	UInt						m_uiGroupNum;		//to record total candidate number in a group
	UInt						m_uiGroupOrder;		//index of current candidate in a group

	UInt						m_uiCentroidX;		//centroid  coordinate
	UInt						m_uiCentroidY;

	UInt						m_uiUpEdge;			//boundries of current candidate
	UInt						m_uiLeftEdge;		
	UInt						m_uiRightEdge;
	UInt						m_uiBottomEdge;

	Bool						m_bUseFul;			//if current candidate not matched with its neighbors 

	Bool						m_bBigDelta;		//if frontiers changed more than one ctu size....
	Bool						m_bTraveled;		//be traveled flag 

	Bool						m_bMatchWithPrior;
	Bool						m_bMatchWithNext;

	Bool						m_bShow;			//not used ... ...

	Bool						m_bPotentialHead;	// not useful ... ...
	Bool						m_bPotentialTail;

	Bool						m_bTrueLeftEdge;	// not useful ... ...
	Bool						m_bTrueUpEdge;
	Bool						m_bTrueRigEdge;
	Bool						m_bTrueBotEdge;

	Bool						connectHeadFlag;
	Bool						connectTailFlag;
	//save motion information corresponding each corresponding candidate
	//corresponding candidate   motion vectors  <deltaX deltaY>
	std::map<ObjectCandidate*, std::vector<std::pair<Int, Int>>, candInPicOrder> m_vMotionInfo;

	//save delta information when two frames general matched ... ...
	std::map<ObjectCandidate*, std::vector<deltaInfo>, candInPicOrder>	      m_mDeltaInf;

	//save motion directions when two frames frontier matched 
	std::vector<std::pair<MotionDirection, MotionDirection>>				 m_mMotionDirInfo;
	MotionDirection															m_enuInheritHorDir;//... ...
	MotionDirection															m_enuInheritVerDir;//... ...
	
	UInt																	  m_uiLatestMatchedPicNumInorder;

	//std::pair<Int, Int>												m_vTempVector;
	//std::vector<ObjectCandidate*>									m_vCheckGroup;

	//std::vector<std::map<ObjectCandidate*,Bool>>					m_aMapGroups;
	//std::multimap<std::vector<ObjectCandidate*>, std::pair<Bool,std::pair<Int,Int>>>				
	MatchedGroup														m_aMapGroups;
	//std::vector<std::pair<Int, Int>>								m_aMotionVectors;

	//the variable is used to record frontier difference occured candidates in the following pictures
	//std::vector < ObjectCandidate*, std::vector<std::pair<Int, Int>> > m_vSkipPoint;

	PicData*					m_pcCurPic;
	//for shift 
	//     subsequence candidate       motion exist  
	std::map<ObjectCandidate*, Bool> m_mSubseqCand;

	//the code below may not used
	std::map<ObjectCandidate*, std::pair<Int, Int>>				m_mDelta;

	
	//the last one that resemble with current 
	ObjectCandidate*						m_pcSubseqEnd;
	/*-------------------------------------------------------*/

	std::pair<ObjectCandidate*, Bool>			m_pairPriorOjbect;		
	std::pair<ObjectCandidate*, Bool>			m_pariPextOjbect;

	ObjectCandidate*							m_pcNextOjbect;
	ObjectCandidate*							m_pcPriorOjbect;
	ObjectCandidate*							m_pcPriorMatchCand;
	ObjectCandidate*							m_pcNextMatchCand;

	//this will not need any more 
	//ObjectCoreElement*							m_pcCoreSize;
	
	//[row][col]
	//std::vector<std::vector<Int>>				m_vCtuData;
	std::vector<std::vector<Int>>				m_vCtuAddr;

	//this is pxl element in current object candidate
	//the key in map is the number of current object 
	//the value in map means if current pixel is common area
	std::vector<std::vector<std::pair<Int, Bool>>>				m_vPxlElement;


	//std::multimap<Int, Int>						m_mDetlatXY;
	std::map<CtuData*, Bool, ctuAddrIncCompare>				m_mCtuElement;

	std::map<ObjectCandidate*, Bool>						m_mPriorCandidates;
	std::map<ObjectCandidate*, Bool>						m_mNextCandidates;

	std::map<ObjectCandidate*, Bool>						neighbourCandidates;

	MovingObject*											m_pcMovingObject;
	//the first UInt is the CTU address, the second UInt is the order of the small CTU
	//in CTU, the third UInt is the partition of the current small CTU
	std::vector<std::vector<UInt>>							currSmallCtuAndPartition;
	std::vector<std::vector<UInt>>							nextSmallCtuAndPartition;
	vector<int>											weightOfDirs;
	vector<int>											longDistanceDirs;  //to record the distance of the object from the 
	UInt												motionLeftAndRight;		
	UInt												motionTopAndBottom;		
	int													deltaX;
	int													deltaY;
	int													predX;
	int													predY;
	
	int													flagX;
	int													flagY;
	vector<int>											flagXVec;
	vector<int>											flagYVec;
	bool												leftRight;
	bool												upDown;

	//new added private members
	std::map<ObjectCandidate*, Bool >					priorSmallCand;
	//current frame to the frame 30 frames away
public:
	//ObjectCandidate()									;// { m_iCurObjNum = 0; }
	ObjectCandidate(PicData* currPic);
	~ObjectCandidate();//{};

	Void		setCurPic(PicData* pic)								{ m_pcCurPic = pic; }
	PicData*	getCurPic()			 						    	{ return m_pcCurPic; }
	Void		setPicNum(UInt picNum)								{ m_uiPicNum = picNum; }
	UInt		getPicNum()			const							{ return m_uiPicNum; }
	Void		setTraveledFlag(Bool flag)							{ m_bTraveled = flag; }
	Bool		getTraveledFlag()									{ return m_bTraveled; }
	Void		setPriorMatch(Bool flag)							{ m_bMatchWithPrior = flag; }
	Bool		getPriorMatch()										{ return m_bMatchWithPrior; }
	Void		setNextMatch(Bool flag)								{ m_bMatchWithNext = flag; }
	Bool		getNextMatch()										{ return m_bMatchWithNext; }
	Void		resetRelationWithCorCand()							{ m_bMatchWithNext = m_bMatchWithPrior = false; }
	Void		setShow(Bool flag)									{ m_bShow = flag; }
	Bool		getShow()											{ return m_bShow; }
	Void		setUsefulFlag(Bool flag)							{ m_bUseFul = flag; }
	Bool		getUsefulFlag()										{ return m_bUseFul; }
	Void		setHeadPointFlag(Bool flag)							{ m_bPotentialHead = flag; }
	Bool		getHeadPointFlag()									{ return m_bPotentialHead; }
	Void		setTailPointFlag(Bool flag)							{ m_bPotentialTail = flag; }
	Bool		getTailPointFlag()									{ return m_bPotentialTail; }

	Void		setLeftEdegFlag(Bool edgeFlag)						{ m_bTrueLeftEdge = edgeFlag; }
	Bool		getLeftEdgeFlag()									{ return m_bTrueLeftEdge; }
	Void		setUpEdegFlag(Bool edgeFlag)						{ m_bTrueUpEdge = edgeFlag; }
	Bool		getUpEdgeFlag()										{ return m_bTrueUpEdge; }
	Void		setRigEdegFlag(Bool edgeFlag)						{ m_bTrueRigEdge = edgeFlag; }
	Bool		getRigEdgeFlag()									{ return m_bTrueRigEdge; }
	Void		setBotEdegFlag(Bool edgeFlag)						{ m_bTrueBotEdge = edgeFlag; }
	Bool		getBotEdgeFlag()									{ return m_bTrueBotEdge; }

	//Void		setCoreSize(ObjectCoreElement *pCore)				{ m_pcCoreSize = pCore; }
	//ObjectCoreElement*  getCoreSize()								{ return m_pcCoreSize; }
	Void			setMovingObject(MovingObject* pMovObj);			
	MovingObject*	getMovingOjbect()								{ return m_pcMovingObject; }

	Void		setBigDelta(Bool Flag)								{ m_bBigDelta = Flag; }
	Bool		getBigDelta()										{ return m_bBigDelta; }
	Void addCtuElement(CtuData* pCtu)								{ m_mCtuElement[pCtu] = false; pCtu->setObject(this); }
	Void eraseCtuElement(CtuData* pCtu);

	std::map<CtuData*, Bool, ctuAddrIncCompare> getCtuElements()		{ return m_mCtuElement; }
	Void clearCtuElements();											
	Void setCtuElements(map<CtuData*, Bool, ctuAddrIncCompare> tempCtu);
	std::map<CtuData*, Bool, ctuAddrIncCompare>& getCtuElemRef()		{ return m_mCtuElement; }
	//this is ObjectCandidate class
	Void setPxlElement();
	std::vector<std::vector<std::pair<Int, Bool>>>& getPxlElement()		{ return m_vPxlElement; }
	//std::vector<std::vector<Int>> &getCtuData()							{ return m_vCtuData; }
	Void setObjNum(UInt num)											{ m_uiCurObjNum = num; }
	UInt getObjNum()													{ return m_uiCurObjNum; }
	Void setGroupNum(UInt Num)											{ m_uiGroupNum = Num; }
	UInt getGroupNum()													{ return m_uiGroupNum; }
	Void setGroupOrder(UInt orderNum)									{ m_uiGroupOrder = orderNum; }
	UInt getGroupOrder()												{ return m_uiGroupOrder; }
	//
	Void setPriorObject(ObjectCandidate* pObj)						{ m_pcPriorOjbect = pObj; }
	ObjectCandidate* getPriorObject()								{ return m_pcPriorOjbect; }
	Void setNextObject(ObjectCandidate* pObj)						{ m_pcNextOjbect = pObj; }
	ObjectCandidate* getNextObject()									{ return m_pcNextOjbect; }

	Void setPriorMatchCand(ObjectCandidate* pObj)					{ m_pcPriorMatchCand = pObj; }
	ObjectCandidate* getPriorMatchCand()							{ return m_pcPriorMatchCand; }

	Void setNextMatchCand(ObjectCandidate* pObj)					{ m_pcNextMatchCand = pObj; }
	ObjectCandidate* getNextMatchCand()								{ return m_pcNextMatchCand; }
	std::vector<std::vector<UInt>> getCurrSCtuAndPart()				{ return currSmallCtuAndPartition; }
	std::vector<std::vector<UInt>> getNextSCtuAndPart()				{ return nextSmallCtuAndPartition; }
	void setCurrSCtuAndPart(std::vector<std::vector<UInt>> input)				{ currSmallCtuAndPartition = input; }
	void setNextSCtuAndPart(std::vector<std::vector<UInt>> input)				{ nextSmallCtuAndPartition = input; }

	Void addPriorCand(ObjectCandidate* pPriorOjbect, Bool flag)		{ m_mPriorCandidates[pPriorOjbect] = flag; }
	std::map<ObjectCandidate*, Bool> getPriorCand()					{ return m_mPriorCandidates; }
	

	Void addNextCand(ObjectCandidate* pNextObject, Bool flag)		{ m_mNextCandidates[pNextObject] = flag; }
	std::map<ObjectCandidate*, Bool> getNextCand()					{ return m_mNextCandidates; }
	UInt getLeftEdge()		{ return m_uiLeftEdge; }
	UInt getUpEdge()		{ return m_uiUpEdge; }
	UInt getRightEdge()		{ return m_uiRightEdge; }
	UInt getBottomEdge()	{ return m_uiBottomEdge; }
	UInt getEdgeWithDir(UInt edgeEir);
	//get the object edge. Attention please, since the calculation of this function is large, it is better to execute 
	//this function at the very beginning of the object.最好在objectCandidate产生的时候就计算一次object edge.
	Void getObjEdge(UInt &leftEdge, UInt &upEdge, UInt &rightEdeg, UInt &botEdge, Bool reCalculate);
	
	Void addMotionInfo(ObjectCandidate* pCorCand, std::vector<std::pair<Int, Int>> motionInfo)	{ m_vMotionInfo[pCorCand] = motionInfo; }
	std::map<ObjectCandidate*, std::vector<std::pair<Int, Int>>, candInPicOrder> &getMotionInfoRef()	{ return m_vMotionInfo; }
	std::map<ObjectCandidate*, std::vector<std::pair<Int, Int>>, candInPicOrder> getMotionInfo()		{ return m_vMotionInfo; }
	
	//
	std::vector<std::pair<MotionDirection, MotionDirection>> &getMotionDirectionInfo(){ return m_mMotionDirInfo; }
	Void setInheritHorDir(MotionDirection inheritHorDir)				    { m_enuInheritHorDir = inheritHorDir; }
	MotionDirection getInheritHorDir()										{ return m_enuInheritHorDir; }
	Void setInheritVerDir(MotionDirection inheritVerDir)					{ m_enuInheritVerDir = inheritVerDir; }
	MotionDirection getInheritVerDir()										{ return m_enuInheritVerDir; }
	std::map<ObjectCandidate*, std::vector<deltaInfo>, candInPicOrder> &getDeltaInf(){ return m_mDeltaInf; }
	std::vector<deltaInfo> &getDeltaWithCorCand(ObjectCandidate *pCorCand) { return m_mDeltaInf[pCorCand]; }
	Bool matchWithAnother(ObjectCandidate *pCorCand)	{ return m_mDeltaInf[pCorCand].size() == 1 ? true : false; }/////////////
	
	Bool corCandIsSearched(ObjectCandidate* pCorCand){ return !(m_mDeltaInf[pCorCand].empty()); }///////////////////////////////


	//Void getMotionVector(Int &deltaX, Int &deltaY)	{ deltaX = m_aMotionVectors.rbegin()->first; deltaY = m_aMotionVectors.rbegin()->second; }
	//Void addGroup(std::vector<ObjectCandidate*> tempGroup)			{ m_aMapGroups.push_back(tempGroup); }
	//std::multimap<std::vector<ObjectCandidate*>,std::pair<Bool,std::pair<Int,Int>>>  
	MatchedGroup	&getMatchGroup()									{ return m_aMapGroups; }
	//Bool addNewMatchElem(ObjectCandidate* pCorCand);// , Int deltaX, Int detlaY);

	Void mergeCandidate(ObjectCandidate* pCorObj);

	//get the object centroid. Attention please, since the calculation of this function is large, it is better to execute 
	//this function at the very beginning of the object.最好在objectCandidate产生的时候就计算一次object centroid.
	Void getObjCentroid(UInt &centroidX, UInt &centroidY, Bool reCalculate);

	//to check if the candidate is at the head or tai frames. For example. if the object is at the frames 1,2,3,4,5,6,7,8,9,10,
	// now the candidate is at the 2nd frame, we can say it is at the head frames of the object
	Bool checkCandAtObjectHeadsOrTails(Bool head);
	
	//if current candidate contact with frame boundries
	Bool atTheBorder(UInt &leftDir, UInt &upDir, UInt &rigDir, UInt &botDir);

	//remove candidates that with edge difference more than two CTUS
	Bool bigGapWithNeighbors(ObjectCandidate *pPriorCand, ObjectCandidate *pNextCand,UInt checkDir);
	Bool priorAndNextHaveSomeBoundry(ObjectCandidate *pPrior, ObjectCandidate *pNext, UInt checkDir);
	//added new function 
	Bool checkFrontierRealtaion(ObjectCandidate* pCorCand, MotionDirection curDir, Int &deltaEdge);

	////////////////////////////////////////////////
	Void checkBoundryMatch(ObjectCandidate* pCorCand, Bool &matchFlag, UInt checkDir, Int deltaVal);


	//general match function,enclude size ratio and overlaping ratio 
	Void matchWithCorrespongdingCand(ObjectCandidate* pCorCand, Bool &matchFlag);
	Void checkFrontierMatch(ObjectCandidate* pCorCand, Bool &motionExist, ObjectCandidate* pCheckCand, Bool verifyFlag);
	Bool checkAnotherDir(ObjectCandidate* pCorCand,ObjectCandidate* pCurCand);
	//for record trajectory of subsequence candidate 
	Void addSubseqCand(ObjectCandidate* subseqCand, Bool motionFlag)            { m_mSubseqCand[subseqCand] = motionFlag; }
	std::map<ObjectCandidate*, Bool> &getSubseqCand()                           { return m_mSubseqCand; }

	Void setSubseqEnd(ObjectCandidate* subend)									{ m_pcSubseqEnd = subend; }
	ObjectCandidate* getSubseqEnd()												{ return m_pcSubseqEnd; }

	//calculate maximum overlapping pixels of current object candidate and the corresponding one 
	//with shifting  the corresopond one to different directions
	Bool calculateMaxOverlaps(ObjectCandidate* pCorObject, Float &padPixel, Bool shiftFlag, Bool &matchFlag);

	//compare current object candidate with corresponging object candidate 
	Bool compareWithCorObject(ObjectCandidate* pCorObject, Bool fillCandFlag);

	//find the corresponding object in the same place but in the diffrent frame
	Bool findCorObject(MovingObject * pMovingObj, ObjectCandidate* pCorObject, ObjectCandidate*& returnObject);


	//to check if the two moving objects have many overlapping sames. Such as the object1 is in the frame 1,3,4,5,6,7,8
	//and object two is in the frame 2,3,5,6,7,8. so they have 5 overlapping frames(3,5,6,7,8)
	Bool checkObjectsOverlapFrames(ObjectCandidate* pCorObject);

	Bool judgeMatch(ObjectCandidate* pCorObject);

	//connect corresponding object candidates that belong to one moving-object.
	Void chainCorObject(Int frameInterval, Bool forward);
	//set current object candidates to a moving-object
	Void dealObjcetChain();

	//if current object candidate  is noise and can be removalbe, usually for object candidate that only contain less than three CTUs
	//if current objectcandidate corresponding to back_ground block in prior and succeeding frames , this will be set as background block
	//strictflag: if this flag is true ,only corresponding block is moving block will be used for judgement
	//            if this flag is false,if corresponding block is moving block or possible block will be used for judgement
	Bool removable(Bool forward, Bool strictFlag);

	//!function to be changed or refine //
	Bool dealOneToMany();
	Bool dealManyToOne();
	Void seperate();
	Void assignMovObjToPriorCandidate();
	Void assignMovObjToSubseqCandidate();
	//to replace Void extendMvoingObject(Int frameInterval, Bool &findFlag);
	Void endExtention(Int frameInterval, Bool &findFlag, Bool ShortStep, UInt extendTurn, Bool specialSearch);
//	Void setEdgeCtu();
	Void checkEdgeCtu();
	Void recoverCtu();
	/*=====================================================================================*/
	//! all the functions not   used now  !//
	//search commom area of two candidates
	//current candidate fixed, move the corresponding candidate 
	//get the max overlapping area and the motion vector <deltaX, deltaY>
	Void searchMaxCommonArea(ObjectCandidate* pCorObj, Bool &matchFlag);
	Void matchWithCorCandGeneralElder(ObjectCandidate* pCorCand, Bool &corMatched, Bool orderFlag);
	Bool checkMatchWithGroup(ObjectCandidate* pCorcand, UInt groupSize);
	Void removeSubMatchGroups(std::vector<ObjectCandidate*> subGroup);
	//Void checkFrontierMatchWithMV(ObjectCandidate* pCorCand, std::vector<motionInf> &vMotInf);// std::vector<std::pair<Int, Int>> &deltaXY);

	//calculate  overlapping pixels of current object candidate and the corresponding one in next frame.
	Void calculateOverlaps(ObjectCandidate* pCorObject, Float &padPixel, Bool fractionFlag);


	//this function is used to scan the follow functions to calculate their edge match degree
	Void calEdgeShift(ObjectCandidate* corCand, UInt &horDir, UInt &verDir);

	//if delta X or delta Y is greater than ctu size when matching current with anoth,
	// the shift distance is not useful and should be delted
	Void delBigDelta();
	//calculate delta with corresponding candidate
	//corCand: corresponding candidate 
	//priorDir: if motion direction exist 
	//if current candidate resemble with corresponding candidate return true 
	//else return false and stop calculation
	Bool calDeltaWithCorCand(ObjectCandidate* corCand, UInt priorHorDir, UInt priorVerDir, UInt &curHorDir, UInt &curVerDir);

	Bool calMatchWithDirection(ObjectCandidate* pCorObject);// , UInt direction);

	//functions that not completed 
	Void calDeltaXY(std::vector<ObjectCandidate*>& candidates);
	Void addSubseqDelta(ObjectCandidate* subseqCand, Int deltaX, Int deltaY)	{ m_mDelta[subseqCand] = std::make_pair(deltaX, deltaY); }
	std::map<ObjectCandidate*, std::pair<Int, Int>> &getSubseqDelta()			{ return m_mDelta; }

	Void setEdgeDelVal(UInt dir, Int val);
	Int  getEdgeDelVal(UInt dir);
	//if two or more objcet candidates corresponding to one object candidate in next frame,  set them belong to the same moving-object 
	Void mergePriorObject();
	//if one object candidate corresponding to more than one object candidates in the next frame, set the object candidates in next frame belong to the same moving-object
	Void mergeTailObject();
	// extend current moving object to both direction , before and after.
	//frameInterval means distance between current frame and corresponding frame.
	//findflag : if corresponding object candidate was found

		//search common of a group of candidates 
	Void searchCommonArea(std::vector<ObjectCandidate*>& candidates);
	/*----------------------------------------------------------------------------------------------------------------------*/
	Void calculateBestMatch(std::vector<ObjectCandidate*> &groupCandidates);//not used 
	Void searchCommonAreaOld(ObjectCandidate* pCorObj);				//not used 

	//functions that not used now 
	//calculate directions with delta
	Void calcuDirWithCorCand(ObjectCandidate* pCorCand);
	//only one direction occured delta
	Void oneDirCompare(ObjectCandidate* pCorCand, MotionDirection curDir, Int deltaVal);
	//two directions occured delta
	Void twoDirCompare(ObjectCandidate* pCorCand);
	//three directions occured delta 
	Void threeDirCompare(ObjectCandidate* pCorCand, MotionDirection falseDir);
	
	//test if the vertical and horizontal edges of two candidates match
	//corCand: corresponding candidate 
	//verDir: left or right 
	//horDir: up or bottom
	//deltaVer: vertical shift
	//deltaHor: horizontal shift
	//verMatch and horMatch return match result 
	Void edgeMatch(ObjectCandidate* corCand, UInt verDir, UInt horDir, Bool &verMatch, Bool &horMatch);//not completed 
	/*----------------------------------------------------------------------------------------------------------------------*/

	//find if shift exist between current candidate and corresponding candidate
	//corCand: corresponding candidate
	//if return 0: no shift 
	//return 1: left 
	//return 2: righte
	//return 3: up
	//return 4: bottom
	Void detectShiftDir(ObjectCandidate* corCand, UInt orgHorDir, UInt orgVerDir, UInt &curHorDir, UInt &curVerDir);
	void setWeightOfDir(vector<int> input)									{ weightOfDirs=input; }
	void setlongDistanceDirs(vector<int> input)								{ longDistanceDirs = input; }

	void setMotionLeftRight(UInt motionLeftRight)							{ motionLeftAndRight = motionLeftRight; }
	void setMotionTopBottom(UInt motionTopBottom)							{ motionTopAndBottom = motionTopBottom; }

	UInt getMotionLeftRight()												{ return motionLeftAndRight ; }
	UInt getMotionTopBottom()												{ return motionTopAndBottom ; }
	void setDeltaX(int x)													{ deltaX = x; }
	void setDeltaY(int y)													{ deltaY = y; }
	int getDelaX()															{ return deltaX; }
	int getDelaY()															{ return deltaY; }
	void setPredX(int x)													{ predX = x; }
	void setPredY(int y)													{ predY = y; }
	void getPredXY(int& x, int& y)											{ x=predX,y=predY; }


	void setFlagX(int x)													{ flagX = x; }
	void setFlagY(int y)													{ flagY = y; }
	void getflagXY(int& x, int& y)											{ x = flagX, y = flagY; }
	void setFlagXVec(vector<int> x)													{ flagXVec = x; }
	void setFlagYVec(vector<int> y)													{ flagYVec = y; }
	vector<int> getFlagXVec()													{ return flagXVec; }
	vector<int> getFlagYVec()													{ return flagYVec; }
	void setLeftRight(bool  flag)												{ leftRight = flag; }
	void setUpDown(bool  flag)													{ upDown= flag; }
	void getLRUD(bool&  flag1, bool& flag2)										{ flag1 = leftRight; flag2 = upDown; }

	void getObjectPos(UInt& xPos, UInt& yPos, int flag = 1);
	void getObjectCentroid(UInt& xPos, UInt& yPos);
	bool inObject(UInt x, UInt y);
	bool findSegmentCtu();
	Bool getHeadFlag()															{ return connectHeadFlag; }
	Bool getTailFlag()															{ return connectTailFlag; }
	Void setHeadFlag(Bool flag)													{ connectHeadFlag = flag; }
	Void setTailFlag(Bool flag)													{ connectTailFlag = flag; }
	//void findSegmentCtu();

	//new added functions
	ObjectCandidate* findCorCand(Int frameInterval, Bool &findFlag);
	
	std::map<ObjectCandidate*, Bool > getPriorSmallCand()							{ return priorSmallCand; }
	Void setPriorSmallCand(ObjectCandidate* cand, Bool match)						{ priorSmallCand[cand] = match; }
	Void splitCtusIntoLeftRight(std::map<CtuData*, Bool, ctuAddrIncCompare>& leftCtu, std::map<CtuData*, Bool, ctuAddrIncCompare>& rightCtu, Float leftRightRatio);
	
	Void addNeighbourCand(ObjectCandidate* pNeighbourOjbect, Bool flag)		{ neighbourCandidates[pNeighbourOjbect] = flag; }
	std::map<ObjectCandidate*, Bool> getNeighbourCand()					{ return neighbourCandidates; }
	//Void setNeighbourCand();

	// Find the public ctus that two objects have in common. Here the two objects should be in the different frame. If they are 
	// in the same frame, they should not have common Ctus.
	UInt findPubCtu(ObjectCandidate* pObject);

	//calculate  overlapping ratio of current object candidate and the corresponding one.
	Bool calculateBoundingBoxOverlaps(ObjectCandidate* pCorObject, Float &ratio1, Float &ratio2);

	Void clearPriorCands()														{ m_mPriorCandidates.clear(); }
	Void clearNextCands()														{ m_mNextCandidates.clear(); }
	Bool addCtuAccordingToBoundingBox(UInt leftEdge_Curr, UInt upEdge_Curr, UInt rightEdge_Curr, UInt botEdge_Curr, CtuData* ctu, ObjectCandidate* pSmallCand, UInt leftEdge_Large, UInt  upEdge_Large, UInt rightEdge_Large, UInt  botEdge_Large);
	//Bool addCtuAccordingToBoundingBox(UInt leftEdge_Curr, UInt upEdge_Curr, UInt rightEdge_Curr, UInt botEdge_Curr, CtuData* ctu, std::map<CtuData*, Bool, ctuAddrIncCompare> pSmallCandCtu, UInt leftEdge_Large, UInt  upEdge_Large, UInt rightEdge_Large, UInt  botEdge_Large);


};


class MovingObject
{
public:
	struct candInOrder
	{
		Bool operator()(const ObjectCandidate *firCand, const ObjectCandidate *secCand)
		{
			return firCand->getPicNum() < secCand->getPicNum();
		}
	};
private:
	UInt			m_uiMovObjectNum;
	UInt			m_uiSeqLength;//total frame length current moving object  exist
	UInt			m_uiCommonSize;		//
	UInt			m_uiGroups;
	UInt            m_colorNum;		
	Bool			m_colorFlag;		//this flag indicates whether the color of this object has been set. If
										//the object has two candidates in one frame, this flag can have a good effect
	Bool			m_bSubSeq;			//if an object candidate correspond to more than one candidate
										//set each corresponding candidate as a solo object 
										//then determin if they are split or two object 
	Bool			m_bRealHead;
	//m_bStopSearchFoward;
	Bool			m_bRealTail;
	//m_bStopSearchBackward;
	Bool			m_bIncreased;//if current moving object extended to more frames

	Bool			m_bHeadExtended;
	Bool			m_bTailExtended;
	ObjectCandidate*		m_pcHeadPointer;	//point to the first frame that current moving object occured
	ObjectCandidate*		m_pcTailPointer;	//point to the last frame that current moving object occured


	ObjectCandidate*		m_pcBegUseful;		//point to the first object candidate that is useful
	ObjectCandidate*		m_pcRBegUseful;		//point to the last  object candidate that is useful
	MovingObject*			m_pcRelateMovObj;
	std::map<MovingObject*, Bool>	m_mPriorMovingObject;		// possible  missing part of current chain in prior frames 
	std::map<MovingObject*, Bool>	m_mSubSeqMovingObject;	// possible  missing part of current chain in subsequent frames 

	//std::map<MovingObject*, pair<Bool,ObjectCandidate*>>	m_mPriorMovingObjectNew;		// possible  missing part of current chain in prior frames 
	//std::map<MovingObject*, pair<Bool, ObjectCandidate*>>	m_mSubSeqMovingObjectNew;	// possible  missing part of current chain in subsequent frames 
	//std::vector<ObjectCandidate*>	candidateElement;	//its own candidates
	std::map<UInt, UInt>					m_mGroupAndLength;
	std::multimap<UInt, UInt, PicOrder>		m_mPicIndex;
	std::map<ObjectCandidate*, ObjectCandidate*>	m_mCandPair;
	//may be not used now 
	std::map<Int, Int>								m_mDeltaXY;
	//
	std::map<ObjectCandidate*,std::vector<std::pair<Int,Int>>>				m_mMatchedCandidates;
	std::map<ObjectCandidate*, std::vector<Int>>           m_mCandidatesAndMotion;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//     for each candidate in a picture  four direction checked  bool: if checked  Int: motion 
	//     each candidate correspondign with  four element : 0 means left, 1 means right, 2 menas up,  3 means donw
	std::map<ObjectCandidate*, std::vector<std::pair<Bool, Int>>, candInOrder>                          m_mDirAndMotion;

	//    boundry index             start and end PicIndex           motion directon; consistant length 
	std::map<UInt, std::vector<std::pair<std::pair<Int, Int>, std::pair<Int, Int>>>>				   m_mMotionStageAndLength;
	std::map<UInt, std::vector<std::pair<std::pair<Int, Int>, Int>>>								   m_mMotionStageAndDelta;
	//	  boundry index				start and end PicIndex			 motion direction consistant 
	std::map<UInt, std::deque<std::pair<std::pair<Int, Int>, std::pair<Bool, Bool>>>>				   m_mMotionStageAndState;
	std::map<UInt, std::deque<std::pair<std::pair<Int, Int>, std::pair<Int, Bool>>>>				   m_mMotionStageAndDirection;
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	std::multimap<std::vector<ObjectCandidate*>, std::pair<Bool, std::pair<Int, Int>>>     m_multimapGroupAndMvInfo;

	//the following members are the new added private members

	// the candidate which should be in the moving object, after these candidates, there may be a split.
	std::map<ObjectCandidate*, Bool>           subsequentSplitCand;
	std::map<ObjectCandidate*, Bool>           priorSplitCand;
	std::map<MovingObject*, vector< UInt>>	neighSplitObjects;

	std::multimap<UInt, ObjectCandidate*>		objectCandidates;

	std::map<MovingObject*, Bool>			   susNextObj;
	std::map < MovingObject*, Bool>           susPriorObj;
														
public:
	//////////////////functions used now ////////////////////////////////////////////
	//MovingObject();
	MovingObject(UInt objNum);
	~MovingObject();
	Void    clearElement();
	//Void    setCandPxlElement();

	Void	setMovObjectNum(UInt num)						{ m_uiMovObjectNum = num; }//set and get current moving object number
	UInt	getMovObjectNum()								{ return m_uiMovObjectNum; }

	Void    setSeqLength(UInt len)							{ m_uiSeqLength = len; }	//operation with length
	UInt    getSeqLength()			const					{ return m_uiSeqLength; }
	Void    inreaseLength(UInt len)							{ m_uiSeqLength += len; }
	UInt	getGroupsNum()									{ return m_uiGroups; }
	Void	increaseGroupNUm()								{ ++m_uiGroups; }

	Void	setColorNum(UInt num)							{ m_colorNum = num; m_colorFlag = true; }//set and get current moving object color number
	UInt	getColorNum()									{ return m_colorNum; }
	Void	setColorFlag(Bool flag)							{ m_colorFlag =flag; }//set and get current moving object color flag
	Bool	colorHasBeenSet()									{ return m_colorFlag; }

	Void    setIncreasedFlag(Bool flag)						{ m_bIncreased = flag; }
	Bool    getIncreasedFlag()								{ return m_bIncreased; }

	Void	setSubSeqFlag(Bool subSeq)						{ m_bSubSeq = subSeq; }
	Bool	getSubSeqFlag()									{ return m_bSubSeq; }
	Void	setRealHeadFlag(Bool flag)						{ m_bRealHead = flag; }
	Bool	getRealHeadFlag()								{ return m_bRealHead; }
	Void	setRealTail(Bool flag)							{ m_bRealTail = flag; }
	Bool    getBackwardFlag()								{ return m_bRealTail; }
	Void	setHeadExtentionFlag(Bool flag)					{ m_bHeadExtended = flag; }
	Bool	getHeadExtentionFlag()							{ return m_bHeadExtended; }
	Void    setTailExtentionFlag(Bool flag)					{ m_bTailExtended = flag; }
	Bool    getTailExtentionFlag()							{ return m_bTailExtended; }
	Void	setHeadPointer(ObjectCandidate* pcHead)			{ m_pcHeadPointer = pcHead; }
	ObjectCandidate* getHeadPointer()						{ return m_pcHeadPointer; }
	Void	setTailPointer(ObjectCandidate* pcTail)			{ m_pcTailPointer = pcTail; }
	ObjectCandidate* getTailPointer()						{ return m_pcTailPointer; }

	Void	setRelateMovObj(MovingObject* pMovObj)			{ m_pcRelateMovObj = pMovObj; }
	MovingObject* getRelateMovObj()							{ return m_pcRelateMovObj; }

	Void	addPriorMovObj(MovingObject* pMovObj, Bool flag)			{ m_mPriorMovingObject[pMovObj] = flag; }
	std::map<MovingObject*, Bool> getPriorMovObj()						{ return m_mPriorMovingObject; }
	Void	clearPriorMovObj()											{ m_mPriorMovingObject.clear(); }
	Void	addSubSeqMovObj(MovingObject* pMovObj, Bool flag)			{ m_mSubSeqMovingObject[pMovObj] = flag; }
	std::map<MovingObject*, Bool> getSubSeqMovObj()						{ return m_mSubSeqMovingObject; }
	Void	clearSubSeqMovObj()											{ m_mSubSeqMovingObject.clear(); }
	
	//Void	addCandidates(ObjectCandidate* pObj)						{ candidateElement.push_back(pObj); }
	//Void	clearCandidates()											{ candidateElement.clear(); }
	//std::vector<ObjectCandidate*> getCandidates()						{ return candidateElement; }
	//Void	setCandidates(std::vector<ObjectCandidate*> pObj)			{ candidateElement = pObj; }

	Void	addPicIdx(UInt picIdx, UInt objIdx)							{ m_mPicIndex.insert(std::make_pair(picIdx, objIdx)); }
	std::multimap<UInt, UInt, PicOrder>& getPicIdx()					{ return m_mPicIndex; }
	Void    addCandidatePair(ObjectCandidate* pFir, ObjectCandidate* pSec) { m_mCandPair[pFir] = pSec; }
	std::map<ObjectCandidate*, ObjectCandidate*> getCandidatePair()		{ return m_mCandPair; }
	Void	addDeltaXY(Int deltaX, Int deltaY)							{ m_mDeltaXY[deltaX] = deltaY; }
	std::map<Int, Int> getDeltaXY()										{ return m_mDeltaXY; }
//	Void	addGroupIdx(UInt groupIdx, UInt groupLen)		{ m_mGroupAndLength[groupIdx] = groupLen; }
	Void	setGroupLen(UInt groupIdx, UInt groupLen)					{ m_mGroupAndLength[groupIdx] = groupLen; }


	std::map<ObjectCandidate*, std::vector<std::pair<Int, Int>>> &getCandAndMatchGroups(){ return m_mMatchedCandidates; }

	Void	copyPriorMovObj(MovingObject* pDst);
	Void	copySubSeqMovObj(MovingObject* pDst);
	//
	Void	assignMovObjToPrior(MovingObject* pSrc);
	Void	assignMovObjToSubseq(MovingObject* pSrc);
	/* if two moving objects are actual the same one , merge them */
	Void	resetElement(MovingObject *pDstMovingObj);			//merge with another moving object

	std::map<MovingObject*, Bool> getConnectedObjects(Bool headExtend);

	/* set useful flag , if current */
	Void    setUsefulFlag(Bool flag);

	Bool	extendChainConstrained(Bool forwardFlag, Bool ShortStep, UInt extendTurn, Bool specialSearch);

	Void	dealConstrainedExtention(Bool ShortStep, Bool head);

	Void	dealMergeConstrained(UInt Exturn);

	Void	dealSplitConstrained(UInt Exturn);


	Bool	ObjectDeletable(UInt extendTurn);

	Void	releaseChainOutOfData(UInt extendTurn);
	//
	//set ctus set as background but surrounded by ctus set as object
	Void	travelCand();

	//useful candidates means that there only one candidate in the corresponding frame
	Void	setUseFulCand();

	//Void	resetUsefulCand();

	//divide current candidate sequence to different groups and each group contain candidates that mathed well 
	Void	setCandGroup();

	Void    fullfillEmpty();

	//not compeleted or to be added 
	Void	releaseErrorConnection();
	Void	extendUnconstrianed();

	/* remove shot chain that only occured in few frames */
	/* should be rewrite for more condition should be add for judgment */
	Void    removeNoise();

	//work on now --------------------------------------------------------------------------------
	//========================into process 
	Void seperate();//========================into step 
	Void detectMovObjeTrajectory();//========================into step 
	Void segment();//========================into step

//	Void detectMotionDirection(std::vector<ObjectCandidate*> &usefulCands, std::vector<ObjectCandidate*>::iterator &travelIndex, UInt checkDir);
	//========================first step 
	Void checkMotionDirection(std::vector<ObjectCandidate*> &usefulCands, std::vector<ObjectCandidate*>::iterator &travelIndex, UInt checkDir);
	//Void MovingObject::markOneFourthCTUInFrames(std::vector<ObjectCandidate*> &usefulCands, vector<ObjectCandidate*>::iterator travelIndex);
	Void getObjectTra(std::vector<ObjectCandidate*> &usefulCands, int direction = 1);//direction 1=leftRight,direction 2=upBottom
	Void getCandCentroid(std::vector<ObjectCandidate*> &usefulCands, int direction, vector<UInt>& picNum);
	Void getObjectTraUpdated(std::vector<ObjectCandidate*> &usefulCands, int direction, vector<UInt>& picNum);
	Void calMotionVec(std::vector<ObjectCandidate*> &usefulCands, vector<UInt> picNum1, vector<UInt> picNum2);

	Void objSeg(std::vector<ObjectCandidate*> &usefulCands, int direction, vector<UInt>& picNum);
	//to check whether the height and width and the number of total CTUs of current Object is in consistent with the next Object or the next two object 
	bool confirmObjectEdgeUniform(std::vector<ObjectCandidate*> &usefulCands, vector<ObjectCandidate*>::iterator& travelIndex,bool forward);
	//set motion stage
	//======================== sub-step of first step
	Void addMotionStepInfo(UInt dirIdx, Int startPicIdx, Int endPicIdx, Int motionDir, UInt len);//useful

	//new function================
	Void setMotionStageState(UInt curBoundryIdx,Bool resetFlag);
	//remove short motion stage 
	
	//first step 
	Bool removeBoundryShortMotion(UInt curBoundryIdx);//will be used !!!!!!!!!!!!!!!!
	//Bool boudryMotionFiter(UInt curBoundryIdx);
	//second 
	//remove incorrent by neighbor 


	//Bool directionMotionFilter(UInt curBOundr)====================================!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	//smooth direction motion filter=================!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//remove shot motion may be fault
	Bool removeShotMotion(UInt checkDir);//useful!!!!!!!!!!!!!may not used any more 

	//merge short motion 
	Bool mergeShortMotion(UInt curBoundryIdx);


	//remove shot motion that consist no more than 3 frames 
	Bool removeShotStage(UInt checkDir);//useful
	//
	//set motion value for one boundry
	Void setBoundryMotionValue(UInt checkDir);//useful


	//set motion value for one direction =====not completed or maybe unused any more 
	Void setDirectionMotionValue();


	//check a serious candidates with consistant boundry value   --useful
	Bool confirmBoundryUniform(std::vector<ObjectCandidate*> &usefulCands, std::vector<ObjectCandidate*>::iterator &candIndex, UInt checkDir,UInt &checkStride);

	//to check if the two moving objects have many overlapping sames. Such as the object1 is in the frame 1,3,4,5,6,7,8
	//and object two is in the frame 2,3,5,6,7,8. so they have 5 overlapping frames(3,5,6,7,8)
	Bool checkObjectsOverlapFrames(MovingObject* pTempObj);
	
	// below functions : not used !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	/*-------------------- old function-------------------------*/
	//new function to change scheme

	//first step:

	Void    resetUseFulFlag();

	//replace the function above 
	//trajectory detection ,will be realized in two step :
	// firs calmotiondDir for each candidate
	//second go over alll the result in the first step to find the reliable direction 
	//this function may not suitable ,should be changed 
	/*=====================================================================================*/
	/* merge fraction  this is to extend head and tail
	if head pointer or tail pointer point to more than one object candidate ,
	according to the condition ,determine if more one object merged or current was splitted
	*/

	//new added functions
	Void	addSuspiciousPriorObj(MovingObject* pObjPrior, Bool flag)				{ susPriorObj[pObjPrior] = flag; }
	std::map<MovingObject*, Bool> getSuspiciousPriorObj()							{ return	susPriorObj; }
	Void	addSuspiciousNextObj(MovingObject* pObjNext, Bool flag)					{ susNextObj[pObjNext] = flag; }
	std::map<MovingObject*, Bool> getSuspiciousNextObj()								{ return  susNextObj; }

	//to find the nearest split candidate
	ObjectCandidate* findNearestPriorSplitCand(MovingObject*& neighMovingObj);
	MovingObject* newAObjectFromHead(ObjectCandidate* head);// break the current head to produce a new object

	//to split the error merge objects.For example, if two objects (A and B) merge into one object(E). And then E split 
	//into two objects ( C and D), it is necessary to tell A is C or A is D. Then we need to merge A and C or A and D together.
	Void	splitErrorMerge(UInt Exturn, Bool head);

	Void	resetErrorConnect(UInt Exturn, Bool head);

	Bool	splitOneObjectIntoTwo(MovingObject* subSeqObj1, MovingObject*subSeqObj2, MovingObject* &pObject3, UInt& splitBePic, UInt& slitEndPic);

	Bool	mergeTwobjectsIntoOne(MovingObject*pSubLargeObject, UInt& mergeBePic, UInt& mergeEndPic);
	
	Void addNeighSplitObj(MovingObject*objs, vector< UInt> begAndEndNums)  { neighSplitObjects.insert(make_pair(objs, begAndEndNums)); }
	std::map<MovingObject*, vector< UInt>>  getNeighSplitObj()						{ return neighSplitObjects; }

	//this function judges if A is the same object as C or D. If return is true, 
	//it means object A is the same as object C, object E is the middle object
	Bool compareObjects(MovingObject* objectA, std::vector<Float> A_velocity, MovingObject* objectB, std::vector<Float> B_velocity,
		MovingObject* objectC, std::vector<Float> C_velocity, MovingObject* objectD, std::vector<Float> D_velocity, MovingObject* objectE=NULL);

	Bool compareObjects(std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> priorObj1, std::vector<Float> A_velocity, std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> priorObj2, std::vector<Float> B_velocity,
		std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> subSeqObj1, std::vector<Float> C_velocity, std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>>subSeqObj2, std::vector<Float> D_velocity);

	Bool compareObjects3objects(std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> priorObj1, std::vector<Float> A_velocity, std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> subSeqObj1, std::vector<Float> C_velocity, std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>>subSeqObj2, std::vector<Float> D_velocity);

	Void addCandidate(UInt picNum, ObjectCandidate* pObject);
	Void eraseCandidate(ObjectCandidate* candidate);
	std::multimap<UInt, ObjectCandidate*> getObjectCandidates()						{ return objectCandidates; }
	Void resetHeadPointer();
	Void resetTailPointer();
	Void resetObjLength();

	//Find a object candidate before or after a specific pic number. If cannot find, return NULL
	ObjectCandidate* findCandBeforeOrAfterPicNum(UInt num, Bool forward);

	ObjectCandidate* findCandAtPicNum(UInt num);

	Void resetPriorAndSubObjs();

	std::vector<std::map<CtuData*, Bool, ctuAddrIncCompare>> getObjCtus(Bool head);

	Void findPriorObjAndMerge(MovingObject* nextObj);

	Bool objVelocityCompare(MovingObject *  pObj, Bool thisTailToPObjHead);
	
	Void testFunc();

	Void calculateCandVelocity();

	Void writeCandEdge(std::vector<ObjectCandidate*> &usefulCands);

	Void getCandEdge();

	Void EdgeNoiseRemove(map<UInt, vector<UInt>> eachObjCandsEdge);

	map<UInt, UInt> EdgeNoiseRemoveOnDirection(map<UInt, UInt> edgeVec);

	motionDir checkFrontierAndGetVel(map<UInt, UInt> edgeMap, Float& velocity);

	Bool checkConditionAndGetVel(map<UInt, UInt> edgeMap1, map<UInt, UInt> edgeMap2, Bool Vec1LargeThanVec2, Float& vel);
};

class predObject
{
private:
	int deltaX;
	int deltaY;
	UInt x;
	UInt y;
	bool setX;
	bool setY;
	UInt objectNum;
public:
	predObject(UInt num)														{ objectNum=num; setX = false; setY = false; x = 0; y = 0; }
	void getPosition(UInt &potionX, UInt &potionY)								{  potionX=x, potionY=y; }
	void setPosition(UInt potionX, UInt potionY)									{ x = potionX, y = potionY; }
	void setObjectNum(UInt num)													{ objectNum=num; }
	UInt getObjectNum()															{ return objectNum; }
	void getXFlag(bool &flagX)													{ flagX=setX; }
	void getYFlag(bool &flagY)													{ flagY=setY; }
	void setXFlag(bool set)														{ setX = set; }
	void setYFlag(bool set)														{ setY = set; }
	void setXPosition(UInt set)														{ x = set; }
	void setYPosition(UInt set)														{ y = set; }
	void setDeltaX(int x)														{ deltaX = x; }
	void setDeltaY(int y)														{ deltaX = x; }
};
//picture class
class PicData
{
private:
	PredMode					m_PreType;
	UInt						m_picSeqNum;
	UInt						m_picWidth;
	UInt						m_picHeght;
	UInt						m_stride;
	UInt						m_ctus;
	UInt						m_CtuWid;
	UInt						m_CtuHig;
	UInt						m_MaxDepth;

	UInt						m_uiCtuInRow;
	UInt						m_uiCtuInCol;
	PredMode					m_predInfo;

	VideoData*					m_curVideo;
	std::vector<CtuData* >		m_ctuElement;
	std::vector<ObjectCandidate*>		m_vObject;
	//Y component value, 
	//key of map is Y component value,
	//and value of the key is color index to distinguish different object
	std::vector<std::vector<Int>>		m_vPxlElements;

	vector<CtuData*>			smallCTU;
	vector<predObject*>			predObjectVec;
	vector<predObject*>			totalObjectVec;
public:
	PicData();
	~PicData();
	PicData(UInt frmNum, UInt frmwid, UInt frmhig, UInt ctuwid, UInt ctuhig,UInt depth);
	//Void	 setPxlElement();
	vector<vector<Int>>& getPxlElement()					{ return m_vPxlElements; }
	UInt	 getCtuInRow()									{ return m_uiCtuInRow; }
	UInt	 getCtuInCol()									{ return m_uiCtuInCol; }
	//Void setPredInfo(PredMode pred)														{ m_predInfo = pred; }
	Void	 setPreType(PredMode inf)						{ m_PreType = inf; }
	PredMode getPreType()									{ return m_PreType; }
	UInt	getStride()										{ return m_stride; }
	CtuData* getCtu(UInt num)								{ return m_ctuElement[num]; }
	Void	 PicWrite(std::ostream &handle);
	Void	 PicNewWrite(std::ostream &handle, UChar* buf1, UChar* buf2, Pel* oneRow, Pel* twoRow, PicData* bufPic, 
		std::vector<Int> pxlRow, std::vector<std::vector<Int>>	m_vPxlElement, queue<UInt> &usefulColors, queue<UInt> &uselessColors);

	CtuData* getCtuEle(UInt num)							{ return m_ctuElement[num]; }
	// m_picSeqNum means the next picture number,the same as the number in HIKVISION yuv player
	UInt	getPicNum()				const					{ return m_picSeqNum; }
	//
	UInt	getPicHig()										{ return m_picHeght; }
	UInt	getPicWid()										{ return m_picWidth; }
	UInt	getCtuWid()										{ return m_CtuWid; }
	UInt	getCtuHig()										{ return m_CtuHig; }

	Void	addNewObj(ObjectCandidate* pObj)				{ m_vObject.push_back(pObj); pObj->setObjNum((UInt)m_vObject.size()); }
	//Void	eraseObj(ObjectCandidate* pObj)					{ m_vObject.erase(pObj); }
	std::vector<ObjectCandidate*>& getObjects()				{ return m_vObject; }
	ObjectCandidate* getObjectWithIdx(UInt idx)				{ return m_vObject[idx]; }

	Void	setVideo(VideoData* pVideo)						{ m_curVideo = pVideo; }
	VideoData*	getCurVideoData()							{ return m_curVideo; }

	void addSmallCtu(CtuData* ctu)							{ smallCTU.push_back(ctu); }

	vector<predObject*>getPredObject()						{ return predObjectVec; }
	void setTotalPredObject(vector<predObject*> total)		{ totalObjectVec = total; }

	bool findPredObject(UInt num);

	void addPredObject(UInt num, UInt x, UInt y);
	void setPredObjectX(UInt num, UInt x);
	void setPredObjectY(UInt num, UInt y);

	void getPredObjectPosition(UInt ObjectNum, UInt& beginX, UInt& beginY);
	void setPredObjectPosition(UInt ObjectNum, UInt beginX, UInt beginY);
	void checkObjectFlag(UInt ObjectNum, bool& setX, bool& setY);
	//CTUs contain cus that with MVD greater than threshold will be set as object
	//number of CUs and residual bits number greater than threshold will be set as possible_object
	Void    preSetCtuSta();

	Void    showCTUstatus();

	//deal with intra CTUs,if current ctu has more than two neighbor ctus that calssified as object
	//this will be calssified as object 
	Void	reSetCtuSta();

	//deal with ctus that surrounded by ctus classified as moving ctus ,but the status of current ctu is not determined
	Void    reDealCtuSta();

	//deal with calssified as moving ctus in picture boundary
	//if current ctu has no neighbor ctus that not in boundary,
	//this will be regard as noise and reset it belong to background
	Void	setBorderCtuStatus();

	//set ctus that joint together as an object candidate
	Void	objectSegmentation();

	//search corresponding object candidate in next frame 
	Void    dealObject();

	//set a connected object candidate chain as a moving object 
	Void	chainObjectCandidate();

	void checkHeadFunc(MovingObject * pObj);

	//merge two moving object that they actuall one chain but broken//////////////////
	Void	resetMovingObj(MovingObject* pMovobj1, MovingObject* pMovobj2);


	Void    resetUsefulFlag(MovingObject* pMovOjb, Bool useflag);

	Void	removNoise();

	//functions to be added ,not completed now 

	Void	setCandidateUsefulFlag(MovingObject* pCurMovObj);
	Void	setEdgeCtuCandFlag(MovingObject* pCurMovObj);
	Void	recoverCandidate(MovingObject* pCurMovObj);
	

	/*=====================================================================================*/
	//!not used now  may be deleted later //////////////////////////////////////////////////////
	//deal with object candidate correspond to two or more  object candidates 
	//split may occured 
	Void	dealOneToManyCondition();
	Void	dealManyToOneCondition();

	//new added functions
	UInt getObjectArea(MovingObject* pCurMovObj);

	std::vector<ObjectCandidate*> findMovObj(MovingObject* pCurMovObj);

	Void	splitMovingObj(MovingObject* pOriObj, MovingObject* pMovobj1, MovingObject* pMovobj2);

	//we need to split object1 according to the candidate's shape
	Bool	splitObj1AccordingToCand(MovingObject* pMovobj1, MovingObject* pMovobj2, ObjectCandidate* cand, ObjectCandidate*& newObjCand);
	Bool	splitObj1AccordingToBoundingBox(MovingObject* pMovobj1, MovingObject* pMovobj2, ObjectCandidate* cand, UInt leftEdge_Large, UInt  upEdge_Large, UInt rightEdge_Large, UInt  botEdge_Large);
	Bool	splitObj1AccordingToPubCtus(MovingObject* pMovobj1, MovingObject* pMovobj2,std::map<CtuData*, Bool, ctuAddrIncCompare> newCandCtu);
	Bool	splitObj1AccordingToCtu(MovingObject* pMovobj1, MovingObject* pMovobj2, std::map<UInt, Bool> newCandCtu);

	Void getCtuEdge(std::map<CtuData*, Bool, ctuAddrIncCompare> totalCtu, UInt &leftEdge, UInt &upEdge, UInt &rightEdge, UInt &botEdge);

	Void getCtuCentroid(std::map<CtuData*, Bool, ctuAddrIncCompare> totalCtu, UInt& centroidX, UInt& centroidY);
};

//
struct OBJECT_LENGTH_COMPARE
{
	Bool operator()(const MovingObject* firMovObj, const MovingObject* secMovObj)
	{
		return firMovObj->getSeqLength() > secMovObj->getSeqLength();
	}
};

struct OBJECT_HEAD_COMPARE
{
	Bool operator()(MovingObject* firMovObj, MovingObject* secMovObj)
	{
		//firMovObj->resetHeadPointer();
		if (firMovObj->getHeadPointer()->getPicNum() != secMovObj->getHeadPointer()->getPicNum())
			return firMovObj->getHeadPointer()->getPicNum() > secMovObj->getHeadPointer()->getPicNum();
		else
		{
			CtuData* ctu1 = firMovObj->getHeadPointer()->getCtuElements().begin()->first;
			CtuData* ctu2 = secMovObj->getHeadPointer()->getCtuElements().begin()->first;
			return ctu1->getCtuAddr() > ctu2->getCtuAddr();
		}

	}
};


//video class
class VideoData
{
private:
	std::string				FileName;
	std::string				m_sOrgFileName;			//original YUV sequence name ,to set mask on it if need to show
	std::ofstream			m_handle;
	UInt					m_picNumber;
	UInt					m_PicCount;
	UInt					m_picHeight;
	UInt					m_picWidth;
	UInt					m_CtuWid;
	UInt					m_CtuHeigt;
	UInt					m_depth;
	//
	UInt					m_uiMaxMovingObjNum;	//to recorde the max number of detected moving object 
	UInt					m_uiInitialObjNum;		//used  to  set order number to each moving object 

	std::deque<PicData *>	m_PicElement;
	std::vector <MovingObject*>		m_vTempMovingObject;

	//moving object map, bool means wheather current moving object is useful
	std::multimap<MovingObject*, Bool, OBJECT_LENGTH_COMPARE>	m_mMovingObject;

	//new added members
	UInt numsAtBegSkiped;  //the num of pics to be skipped at the beginning
	UInt maxFrameNum;

public:
	VideoData();
	~VideoData();
	//constructor
	VideoData(UInt frmhig, UInt frmwid, UInt depth, UInt ctuhig, UInt ctuwid, std::string name = "DetectResult.yuv");
	std::string getFileName()					{ return FileName; }
	//
	Void setOrgFileName(std::string orgName)	{ m_sOrgFileName = orgName; }
	std::string getOrgFileName()				{ return m_sOrgFileName; }
	PicData * getPicData(int num)				{ return m_PicElement[num]; }
	UInt getPicHig()							{ return m_picHeight; }
	UInt getPicWid()							{ return m_picWidth; }
	Void setMaxMovOjbNum(UInt num)				{ m_uiMaxMovingObjNum = num; }
	UInt getMaxMovOjbNum()						{ return m_uiMaxMovingObjNum; }
	Void setInitialNum(UInt num)				{ m_uiInitialObjNum = num; }
	Void increaseInitialNum()					{ m_uiInitialObjNum++; }
	Void resetInitialNum()						{ m_uiMaxMovingObjNum = 100 * (1 + m_uiInitialObjNum / 100); }
	UInt getInitialNum()						{ return m_uiInitialObjNum; }
	//return a picture with picture number PicNum.
	PicData* getPic(UInt PicNum);
	//return number of total pictures
	UInt	getPicNumInVideo()								{ return m_picNumber; }
	Void	addMovObjToVector(MovingObject* pcMovObj)		{ m_vTempMovingObject.push_back(pcMovObj); }
	std::vector<MovingObject*>	getTempMovObj()				{ return m_vTempMovingObject; }
	Void	addMovObjToMap(MovingObject* pcMovObj)			{ m_mMovingObject.insert(std::make_pair(pcMovObj, false)); }
	Void	addMovObjToMap(MovingObject* pcMovObj, Bool flag){ m_mMovingObject.insert(std::make_pair(pcMovObj, flag)); }
	std::multimap<MovingObject*, Bool, OBJECT_LENGTH_COMPARE> getMovObjMap()		{ return m_mMovingObject; }

	//write YUV components 
	Void	detection(Bool Newflag = false);//writePic(Bool Newflag = false);

	//set YUV components 
	Void	setYUVcomponetValue();

	//add a new picture to current video sequence and return the pointer 
	PicData* NewPic();

	//set ctu status as : moving_object  possible_moving_object  background  possible_background
	Void	classifyCtu();

	//merge adjacent ctus as one object candidate
	Void	objectSegmentation();

	//search before and after frames to find matched objects
	Void	joinBreakChain();


	//search before and after frames to find matched objects
	Void	segObject();

	/*------belown  new  function  to be added  ,to supplement or to replace old functions -----*/ 
	//try to detect the trajectory of current moving object.
	Void	trajectoryDetection_Video_On();

	Void reSetObject();
	/*------above  new function  to be added  ,to supplement or to replace old functions -----*/

	Void	trajectoryDet();

	//
	Void	dealWithLongChain();
	Void	dealWithMovingObject();

	//not used now 
	//extend each moving object to 
	//if current head pointer or tail pointer point to more than one object candidate
	//deal with them: merge or set as a cross point
//	Void	motionExtention(); delete

	//connect break chains that may be the same one
//	Void    extendMovChain(); delete
	// 
//	Void removeMovingObject(MovingObject* pMovObj);
//	Void GoChan();
//	Void dealWithPicture();
//deal with pictures to find candidate area

	//new added function
	UInt getFrameNumSkip()									{ return numsAtBegSkiped; }

	Void writeInformationToTxt();
	Void writeObjToTxt();
	Void writeCandToTxt(UInt objNum, UInt picNum, vector<UInt>ctuAddr , ObjectCandidate* pCand);
	Bool DeleteEntireDir(string dir);
};


template <class T1, class T2, class T3> class Map :public map<T1,T2,T3>
{
private:
	map<T1,T2,T3> newMap;
public:	
	Void clear();
};

