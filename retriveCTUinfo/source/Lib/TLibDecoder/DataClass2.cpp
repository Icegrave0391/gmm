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


Void MovingObject::findPriorObjAndMerge(MovingObject* nextObj)
{
	cout << "finding prior to merge " << endl;
	cout << "this moving obj num " << this->getMovObjectNum() << endl;
	cout << "this head pic num is  " << this->getHeadPointer()->getPicNum() << endl;
	cout << "this tail pic num is  " << this->getTailPointer()->getPicNum() << endl;
	cout << "nextObj head is " << nextObj->getHeadPointer()->getPicNum() << endl;
	cout << "nextObj tail is " << nextObj->getTailPointer()->getPicNum() << endl;
	int gt; cin >> gt;
	auto beg = m_mPriorMovingObject.begin();
	while (beg != m_mPriorMovingObject.end())
	{
		MovingObject* tempObj = beg->first;
		while (tempObj->getMovObjectNum() == 0)
			tempObj = tempObj->getRelateMovObj();
		if (tempObj->getTailPointer()->getPicNum() < nextObj->getHeadPointer()->getPicNum())
		{
			cout << "have resettting111" << endl;
			cout << "tempObj->getTailPointer() pic num is " << tempObj->getTailPointer()->getPicNum() << endl;

			UInt x1 = 0, y1 = 0;
			UInt x2 = 0, y2 = 0;
			tempObj->getTailPointer()->getObjCentroid(x1, y1, true);
			nextObj->getHeadPointer()->getObjCentroid(x2, y2, true);
			UInt deltaX = UInt(abs(int(x1) - int(x2)));
			UInt deltaY = UInt(abs(int(y1) - int(y2)));
			UInt delta = sqrt(deltaX*deltaX + deltaY*deltaY);
			cout << "delta is " << delta << endl;
			int tt; cin >> tt;
			{
				if (delta < 300)
				tempObj->resetElement(nextObj);
				return;
			}
		}
		beg++;
	}

	//return;
	std::multimap<MovingObject*, Bool, OBJECT_LENGTH_COMPARE> objects = this->getHeadPointer()->getCurPic()->getCurVideoData()->getMovObjMap();

	auto begObj = objects.begin();
	while (begObj != objects.end())
	{
		MovingObject* currObj = begObj->first;
		while (currObj->getMovObjectNum() == 0)
			currObj = currObj->getRelateMovObj();
		//currObj->resetPriorAndSubObjs();
		if (currObj->getMovObjectNum() != 0)
		{
			std::map<MovingObject*, Bool> subObjs = currObj->getSubSeqMovObj();
			auto begSub = subObjs.begin();
			while (begSub != subObjs.end())
			{
				MovingObject* tempObj = begSub->first;
				while (tempObj->getMovObjectNum() == 0)
					tempObj = tempObj->getRelateMovObj();
				if (tempObj == this)
				{
					cout << "there is a this " << endl;
					if (currObj->getTailPointer()->getPicNum() < nextObj->getHeadPointer()->getPicNum())
					{
						cout << "have resettting222" << endl;
						cout << "currObj->getTailPointer() pic num is " << currObj->getTailPointer()->getPicNum() << endl;

						UInt x1 = 0, y1 = 0;
						UInt x2 = 0, y2 = 0;
						currObj->getTailPointer()->getObjCentroid(x1, y1, true);
						nextObj->getHeadPointer()->getObjCentroid(x2, y2, true);
						UInt deltaX = UInt(abs(int(x1) - int(x2)));
						UInt deltaY = UInt(abs(int(y1) - int(y2)));
						UInt delta = sqrt(deltaX*deltaX + deltaY*deltaY);
						cout << "delta is " << delta << endl;
						int gy; cin >> gy;
						{
							if (delta < 300)
							currObj->resetElement(nextObj);
							return;
						}

					}

				}

				begSub++;
			}
		}
		begObj++;
	}
}


Void MovingObject::splitErrorMerge(UInt Exturn, Bool head)
{
	//while (this->getMovObjectNum() == 0)
	//	this = this->getRelateMovObj();
	if (head)
	{
		//head extention
		if (m_mPriorMovingObject.size() < 1)
		{
			m_bRealHead = true;
			return;
		}

		cout << "heeeeello split error merge, head" << endl;

		std::map<MovingObject*, Bool>	connectedMovingObject;
		connectedMovingObject = this->getConnectedObjects(true);

		//if (m_mPriorMovingObject.size() == 1 && m_pcHeadPointer->getPriorCand().size() <= 1)
		if (connectedMovingObject.size() >= 1)
		{
			Bool stopFlag = false, constrainFlag = true;
			MovingObject* pTempMovObj = connectedMovingObject.begin()->first;
			Bool corFlag = connectedMovingObject.begin()->second;

			while (pTempMovObj->getMovObjectNum() == 0)
				pTempMovObj = pTempMovObj->getRelateMovObj();

			pTempMovObj->resetPriorAndSubObjs();
			Bool errorConnect = false;
			Bool matchFlag = false;
			cout << "233333" << endl;
			cout << "head num is " << getHeadPointer()->getPicNum() << endl;
			cout << "tail num is " << getTailPointer()->getPicNum() << endl;
			cout << "pTempMovObj->getSubSeqMovObj().size() is " << pTempMovObj->getSubSeqMovObj().size() << endl;
			cout << "getHeadPointer()->getNeighbourCand().size() is " << getHeadPointer()->getNeighbourCand().size() << endl;
			/*if (getHeadPointer()->getPicNum() == 481)
			{
			cout << "here is a 481" << endl;
			cout << "pTempMovObj head num is " << pTempMovObj->getHeadPointer()->getPicNum() << endl;
			cout << "pTempMovObj tail num is " << pTempMovObj->getTailPointer()->getPicNum() << endl;
			cout << "this moving obj num is " << this->getMovObjectNum() << endl;
			}*/

			//int ssl; cin >> ssl;
			std::map<ObjectCandidate*, Bool> neighbourCand;
			if (pTempMovObj->getSubSeqMovObj().size() > 1 || (getHeadPointer()->getNeighbourCand().size() >= 1))
			{
				if (pTempMovObj->getSubSeqMovObj().size() == 2)
				{
					matchFlag = false;
					std::map<MovingObject*, Bool> subSeqObj = pTempMovObj->getSubSeqMovObj();
					auto beg = subSeqObj.begin();
					while (beg != subSeqObj.end())
					{
						/*if (getHeadPointer()->getPicNum() == 481)
						{
						cout << "here is a 481" << endl;
						cout << "beg head is " << beg->first->getHeadPointer()->getPicNum()<< endl;
						cout << "beg tail is " << beg->first->getTailPointer()->getPicNum() << endl;
						}*/
						if (beg->first == this)
						{
							errorConnect = true;
							matchFlag = true;
							break;
						}
						beg++;
					}
				}

				if (errorConnect == false)
				{
					neighbourCand = getHeadPointer()->getNeighbourCand();
					auto beg = neighbourCand.begin();
					while (beg != neighbourCand.end())
					{
						UInt cenX = 0, cenY = 0;
						beg->first->getObjCentroid(cenX, cenY, true);
						cout << "the x,y of neighbour is " << cenX << " " << cenY << endl;
						cout << "the pic of neighbour is " << beg->first->getPicNum() << endl;
						while (pTempMovObj->getMovObjectNum() == 0)
							pTempMovObj = pTempMovObj->getRelateMovObj();

						if (beg->first->getMovingOjbect() == pTempMovObj)
						{
							errorConnect = true;
							break;
						}
						beg++;
					}
				}
			}
			cout << "pTempMovObj head num is " << pTempMovObj->getHeadPointer()->getPicNum() << endl;
			cout << "pTempMovObj tail num is " << pTempMovObj->getTailPointer()->getPicNum() << endl;
			cout << "errorConnect is " << errorConnect << endl;
			cout << "corFlag is " << corFlag << endl;

			//int ss; cin >> ss;
			if (errorConnect)
			{
				cout << "the third round" << endl;

				MovingObject* pObj1, *pObj2;
				auto subSeqMovObj = pTempMovObj->getSubSeqMovObj();
				if (subSeqMovObj.size() > 1 && matchFlag)
				{
					cout << "it is getSubSeqMovObj().size() > 1, size is " << subSeqMovObj.size() << endl;
					auto beg = subSeqMovObj.begin();
					pObj1 = beg->first;
					if (pObj1 == NULL)
						cout << "pObject1 is NULL" << endl;
					while (pObj1->getMovObjectNum() == 0)
						pObj1 = pObj1->getRelateMovObj();
					beg++;
					pObj2 = beg->first;
					if (pObj2 == NULL)
						cout << "pObject2 is NULL" << endl;
					while (pObj2->getMovObjectNum() == 0)
						pObj2 = pObj2->getRelateMovObj();

				}
				else
				if (neighbourCand.size() > 0)
				{
					pObj2 = this;

					pObj1 = pTempMovObj->newAObjectFromHead(pObj2->getHeadPointer());
					if (pObj1 == pTempMovObj || pObj1 == NULL)
					{
						cout << "there is sth wrong here, exit" << endl;
						exit(0);
					}
				}
				else
				{
					return;
				}

				cout << "debug getting subsequent object" << endl;
				//int tt; cin >> tt;

				MovingObject* subSeqObj1 = pObj1;
				MovingObject* subSeqObj2 = pObj2;

				Float ratio1 = 0, ratio2 = 0;
				Bool match1 = false, match2 = false;

				ObjectCandidate * objectHeadCand1 = subSeqObj1->getHeadPointer();
				ObjectCandidate * objectHeadCand2 = subSeqObj2->getHeadPointer();

				cout << "subSeqObj1 haed is " << subSeqObj1->getHeadPointer()->getPicNum() << endl;
				cout << "subSeqObj1 tail is " << subSeqObj1->getTailPointer()->getPicNum() << endl;
				cout << "subSeqObj2 haed is " << subSeqObj2->getHeadPointer()->getPicNum() << endl;
				cout << "subSeqObj2 tail is " << subSeqObj2->getTailPointer()->getPicNum() << endl;

				if (objectHeadCand1->getPicNum() <= pTempMovObj->getTailPointer()->getPicNum())
				{
					objectHeadCand1 = subSeqObj1->findCandBeforeOrAfterPicNum(pTempMovObj->getTailPointer()->getPicNum(), true);
				}

				if (objectHeadCand2->getPicNum() <= pTempMovObj->getTailPointer()->getPicNum())
				{
					objectHeadCand2 = subSeqObj2->findCandBeforeOrAfterPicNum(pTempMovObj->getTailPointer()->getPicNum(), true);
				}

				if (objectHeadCand1 == NULL || objectHeadCand2 == NULL)
				{
					cout << "Ready to NULL return" << endl;
					cout << "objectHeadCand1  is " << objectHeadCand1 << endl;
					cout << "objectHeadCand2  is " << objectHeadCand2 << endl;
					return;
				}

				//MovingObject::findCandBeforeOrAfterPicNum(UInt num, Bool forward)
				match1 = pTempMovObj->getTailPointer()->calculateBoundingBoxOverlaps(objectHeadCand1, ratio1, ratio2);
				match2 = pTempMovObj->getTailPointer()->calculateBoundingBoxOverlaps(objectHeadCand2, ratio1, ratio2);

				if (match1 == false || match2 == false)
				{
					if (match1)
					{
						pTempMovObj->resetElement(subSeqObj1);
						//subSeqObj1->clearPriorMovObj();
						//pTempMovObj->copyPriorMovObj(subSeqObj1);
					}
					if (match2)
					{
						pTempMovObj->resetElement(subSeqObj2);
						//subSeqObj2->clearPriorMovObj();
						//pTempMovObj->copyPriorMovObj(subSeqObj2);
					}
					cout << "redeady to return " << endl;
					cout << "match 1 is " << match1 << endl;
					cout << "match 2 is " << match2 << endl;
					//int gh; cin >> gh;
					return;
				}

				std::map<MovingObject*, vector< UInt>> neighObj = subSeqObj1->getNeighSplitObj();
				if (neighObj.find(subSeqObj2) != neighObj.end())
				{
					auto beg = neighObj.begin();
					while (beg != neighObj.end())
					{
						cout << "the nieghBour obj num is " << beg->first->getMovObjectNum() << endl;
						beg++;
					}
					cout << "find , ready to return " << endl;
					//int ty; cin >> ty;
					return;
				}
				//int tt; cin >> tt;

				cout << "here is a bug point 45" << endl;

				Bool splitFinish = false;
				MovingObject* pObject3 = NULL;
				UInt begSplitPicNum = 0, endSplitPicNum = 0;

				MovingObject* priorObj1 = pObject3;
				MovingObject* priorObj2 = pTempMovObj;
				splitFinish = priorObj2->splitOneObjectIntoTwo(subSeqObj1, subSeqObj2, priorObj1, begSplitPicNum, endSplitPicNum);


				if (splitFinish == true)
				{
					return;
				}

			}
		}
	}
	else
	{
		//tail extention
		if (m_mSubSeqMovingObject.size() < 1)
		{
			m_bRealTail = true;
			return;
		}

		cout << "433333 hello split error merge, tail" << endl;

		this->resetPriorAndSubObjs();
		std::map<MovingObject*, Bool>	connectedMovingObject;
		connectedMovingObject = this->getConnectedObjects(false);

		auto beg = connectedMovingObject.begin();
		while (beg != connectedMovingObject.end())
		{
			MovingObject* pTempMovObj = beg->first;

			while (pTempMovObj->getMovObjectNum() == 0)
				pTempMovObj = pTempMovObj->getRelateMovObj();

			MovingObject* movingObjLast = this;
			MovingObject* movingObjNext = pTempMovObj;

			ObjectCandidate* pFirst = pTempMovObj->getHeadPointer();
			ObjectCandidate* pLast = this->getTailPointer();

			//find the similar candidate 
			UInt picNum = pLast->getPicNum();
			ObjectCandidate* corCand = NULL;
			ObjectCandidate* neighCand = NULL;
			UInt findNum = 1;
			Bool changeSize = false;
			Bool match = true;

			while (findNum <= 1)
			{
				corCand = movingObjNext->findCandBeforeOrAfterPicNum(picNum, true);
				if (corCand != NULL)
				{
					Float ratio1 = 0, ratio2 = 0;

					match = pLast->calculateBoundingBoxOverlaps(corCand, ratio1, ratio2);

					if (match)
					{
						neighCand = movingObjNext->findCandAtPicNum(picNum);
						if (neighCand == NULL)
							break;
						UInt size1 = pLast->getCtuElements().size();
						UInt size2 = neighCand->getCtuElements().size();
						if (size1 > size2)
						{
							std::multimap<UInt, ObjectCandidate*> objCand = movingObjNext->getObjectCandidates();
							// aasign the candidates from movingObjNext to pLast
							auto begCand = objCand.begin();
							while (begCand != objCand.end())
							{
								UInt candPic = begCand->first;
								if (candPic > picNum)
								{
									begCand->second->setMovingObject(movingObjLast);
									cout << "ressettting" << endl;
								}
								begCand++;
							}

							movingObjNext->resetObjLength();
							UInt seqLength = movingObjNext->getSeqLength();
							if (seqLength > 0)
							{
								movingObjNext->resetHeadPointer();
								movingObjNext->resetTailPointer();
							}
							else
								movingObjNext->resetElement(movingObjLast);
							movingObjLast->resetHeadPointer();
							movingObjLast->resetTailPointer();
							//return;
							changeSize = true;
							break;
						}
					}

				}
				findNum++;
			}

			Bool splitFlag = false;

			cout << "there is a tail here " << endl;
			cout << "the tai pic num is  " << pLast->getPicNum() << endl;
			cout << "the connected object size is " << connectedMovingObject.size() << endl;
			int gg; cin >> gg;
			UInt begSplitPicNum = 0, endSplitPicNum = 0;
			if (match)
			{
				std::cout << "4333tail" << endl;
				if (changeSize == false)
					splitFlag = movingObjLast->mergeTwobjectsIntoOne(movingObjNext, begSplitPicNum, endSplitPicNum);
				else
					splitFlag = movingObjNext->mergeTwobjectsIntoOne(movingObjLast, begSplitPicNum, endSplitPicNum);
				if (splitFlag)
					return;
			}

			beg++;
		}
	}
}



Void	MovingObject::resetErrorConnect(UInt Exturn, Bool head)
{
	if (head)
	{
	}
	else
	{
		//tail extention
		if (m_mSubSeqMovingObject.size() < 1)
		{
			m_bRealTail = true;
			return;
		}


		std::map<MovingObject*, Bool>	connectedMovingObject;
		connectedMovingObject = this->getConnectedObjects(false);

		auto beg = connectedMovingObject.begin();
		while (beg != connectedMovingObject.end())
		{

			MovingObject* pTempMovObj = beg->first;

			while (pTempMovObj->getMovObjectNum() == 0)
				pTempMovObj = pTempMovObj->getRelateMovObj();

			MovingObject* movingObjLast = this;
			MovingObject* movingObjNext = pTempMovObj;

			ObjectCandidate* pLast = this->getTailPointer();

			//find the similar candidate 
			UInt picNum = pLast->getPicNum();
			ObjectCandidate* corCand = NULL;
			ObjectCandidate* neighCand = NULL;

			Bool changeSize = false;
			Bool match = true;
			cout << "ready ro judge, this tail pic num is " << this->getTailPointer()->getPicNum() << endl;
			cout << "hello2099ppl79oi7" << endl;
			//
			cout << "byebye" << endl;

			cout << "the pTempMovObj head is " << pTempMovObj->getHeadPointer()->getPicNum() << endl;
			int fr; cin >> fr;

			corCand = movingObjNext->findCandBeforeOrAfterPicNum(picNum, true);
			if (corCand != NULL)
			{
				Float ratio1 = 0, ratio2 = 0;

				match = pLast->calculateBoundingBoxOverlaps(corCand, ratio1, ratio2);

				if (match)
				{
					cout << "the corCand pic num is " << corCand->getPicNum() << endl;
					cout << "the pLast pic num is " << pLast->getPicNum() << endl;

					//int tt; cin >> tt;
					neighCand = movingObjNext->findCandAtPicNum(picNum);
					if (neighCand == NULL)
						break;
					UInt size1 = pLast->getCtuElements().size();
					UInt size2 = neighCand->getCtuElements().size();
					cout << "size1 is " << size1 << endl;
					cout << "size2 is " << size2 << endl;
					if (size1 > size2)
					{
						cout << "resettiiingg" << endl;
						std::multimap<UInt, ObjectCandidate*> objCand = movingObjNext->getObjectCandidates();
						// aasign the candidates from movingObjNext to pLast
						auto begCand = objCand.begin();
						while (begCand != objCand.end())
						{
							UInt candPic = begCand->first;
							if (candPic > picNum)
							{
								begCand->second->setMovingObject(movingObjLast);
								//cout << "ressettting" << endl;
							}
							begCand++;
						}

						movingObjNext->resetObjLength();
						UInt seqLength = movingObjNext->getSeqLength();
						if (seqLength > 0)
						{
							movingObjNext->resetHeadPointer();
							movingObjNext->resetTailPointer();
						}
						else
							movingObjNext->resetElement(movingObjLast);
						movingObjLast->resetHeadPointer();
						movingObjLast->resetTailPointer();
						//return;
						changeSize = true;
						break;
					}
				}

			}


			beg++;
		}

	}

}


Void MovingObject::calculateCandVelocity()
{
	auto cand=this->getObjectCandidates();
	map<UInt, UInt> CandXPos;
	map<UInt, UInt> CandYPos;

	UInt x, y = 0;
	UInt currPicNum = 0;
	UInt nextPicNum = 0;
	UInt priorPicNum = 0;
	Bool nextNewPic = false;
	Bool onltOnePic = true;
	auto beg = cand.begin();
	auto begPrior = beg;
	auto begNext = beg;
	std::map<CtuData*, Bool, ctuAddrIncCompare> tempCtu;
	std::map<CtuData*, Bool, ctuAddrIncCompare> totalCtu;

	//Map<CtuData*, Bool, ctuAddrIncCompare> tempCTU;
	//Map<CtuData*, Bool, ctuAddrIncCompare> totalCTU;
	if (cand.size() == 0)
	{
		cout << "the cand number is zero, error, exit" << endl;
		exit(0);
	}
	PicData* pic = NULL;
	while (beg != cand.end())
	{
		if (beg == cand.begin())
		{
			if (cand.size() > 1)
			{
				currPicNum = beg->first;
				auto beg2 = beg;
				beg2++;
				nextPicNum = beg2->first;
				if (currPicNum != nextPicNum)
				{
					onltOnePic = true;
					nextNewPic = true;
				}
				else
				{
					onltOnePic = false;
					nextNewPic = false;
				}
			}
			else
			{
				onltOnePic = true;
				nextNewPic = true;
			}
		}
		else
		{
			currPicNum = beg->first;
			begPrior = beg;
			begPrior--;
			priorPicNum = begPrior->first;
			begNext = beg;
			begNext++;
			nextPicNum = begNext->first;
			if (priorPicNum != currPicNum&&currPicNum != nextPicNum)
			{
				onltOnePic = true;
				nextNewPic = true;
			}
			else
			if (priorPicNum != currPicNum&&currPicNum == nextPicNum)
			{
				onltOnePic = false;
				nextNewPic = false;
			}
			else
			if (priorPicNum == currPicNum&&currPicNum != nextPicNum)
			{
				onltOnePic = false;
				nextNewPic = true;
			}
			else
			{
				cout << "setting average velocity error, exit" << endl;
				exit(0);
			}
		}

		UInt picNum = beg->first;
		beg->second->getObjCentroid(x,y,false);
		if (onltOnePic==true)
		{
			CandXPos.insert(make_pair(picNum,x));
			CandYPos.insert(make_pair(picNum,y));
			totalCtu.clear();
			std::map<CtuData*, Bool, ctuAddrIncCompare> emptyCtu;
			totalCtu.swap(emptyCtu);
			beg++;
			continue;
		}
		else
		if (onltOnePic == false && nextNewPic==true)
		{
			tempCtu.clear();
			std::map<CtuData*, Bool, ctuAddrIncCompare> emptyCtu;
			totalCtu.swap(emptyCtu);
			tempCtu=beg->second->getCtuElements();
			auto begCtu = tempCtu.begin();
			while (begCtu != tempCtu.end())
			{
				totalCtu.insert(*begCtu);
				begCtu++;
			}


			pic->getCtuCentroid(totalCtu,x,y);
			CandXPos.insert(make_pair(picNum, x));
			CandYPos.insert(make_pair(picNum, y));
			
			totalCtu.clear();
			totalCtu.swap(emptyCtu);
		}
		else if (onltOnePic == false && nextNewPic == false)
		{
			tempCtu = beg->second->getCtuElements();
			auto begCtu = tempCtu.begin();
			while (begCtu != tempCtu.end())
			{
				totalCtu.insert(*begCtu);
				begCtu++;
			}
		}
		else
		{
			cout << "there is a bug in calculateCandVelocity, exit" << endl;
			exit(0);
		}

		beg++;
	}



}



//template <class T1,class T2, calss T3>
//Map<T1,T2>::Map()
//{
//}


template <class T1,class T2,class T3>
Void Map<T1,T2, T3>::clear()
{
	newVec.clear();
	map<T1,T2, T3> emptyMap;
	newVec.swap(emptyMap);
}



Void MovingObject::writeCandEdge(std::vector<ObjectCandidate*> &usefulCands)
{
	ofstream fout;
	string textName = "edge.txt";

	UInt ObjectNum = this->getMovObjectNum();

	UInt longDisPicNum = 0;
	char number[10];
	sprintf(number, "%d", ObjectNum);
	string outfileName = number;

	outfileName += textName;
	fout.open(outfileName);
	if (!fout){
		cout << "file open failed.\n";
		exit(0);//program exit
	}
	cout << "detecting the ObjectEdge of obect  " << number << endl;


	ObjectCandidate* tempCand = NULL;
	auto beg = usefulCands.begin();
	while (beg != usefulCands.end())
	{
		tempCand = *beg;
		UInt leftEdge, upEdge, rightEdge, botEdge;
		tempCand->getObjEdge(leftEdge, upEdge, rightEdge, botEdge, false);

		fout << tempCand->getPicNum() << "  " << leftEdge << "  " << upEdge << "  " << rightEdge << "  " << botEdge << endl;

		beg++;
	}
	fout.close();
}

Void MovingObject::getCandEdge()
{
	Bool onlyOne = false;
	vector<ObjectCandidate*>usefulCands;
	if (objectCandidates.size() <= 2)
		return;
	auto beg=objectCandidates.begin();
	auto begPrior = beg;
	beg++;
	auto begNext = beg;
	begNext++;
	while (begNext != objectCandidates.end())
	{
		if (begPrior->second->getPicNum() != beg->second->getPicNum() && beg->second->getPicNum() != begNext->second->getPicNum())
			usefulCands.push_back(beg->second);

		begPrior++;
		beg++;
		begNext++;
	}

	map<UInt, vector<UInt>> candEdge;
	ObjectCandidate *tempCand = NULL;
	auto begCand = usefulCands.begin();
	while (begCand != usefulCands.end())
	{
		tempCand = *begCand;
		UInt picNum = tempCand->getPicNum();

		UInt leftEdge=0, upEdge=0, rightEdge=0, botEdge=0;
		vector<UInt> edge;
		tempCand->getObjEdge(leftEdge, upEdge, rightEdge, botEdge, false);
		edge.push_back(leftEdge);
		edge.push_back(upEdge);
		edge.push_back(rightEdge);
		edge.push_back(botEdge);
		candEdge.insert(make_pair(picNum, edge));
		begCand++;
	}

	EdgeNoiseRemove(candEdge);

}

Void MovingObject::EdgeNoiseRemove(map<UInt, vector<UInt>> eachObjCandsEdge)
{
	map<UInt, UInt> leftEdgeMap;
	map<UInt, UInt> upEdgeMap;
	map<UInt, UInt> rightEdgeMap;
	map<UInt, UInt> botEdgeMap;

	auto beg = eachObjCandsEdge.begin();
	while (beg != eachObjCandsEdge.end())
	{
		UInt picNum = beg->first;
		vector<UInt> edges = beg->second;
		UInt leftEdge, upEdge, rightEdge, botEdge;
		leftEdge = edges[0];
		upEdge = edges[1];
		rightEdge = edges[2];
		botEdge = edges[3];

		leftEdgeMap.insert(make_pair(picNum, leftEdge));

		upEdgeMap.insert(make_pair(picNum, upEdge));

		rightEdgeMap.insert(make_pair(picNum, rightEdge));

		botEdgeMap.insert(make_pair(picNum, botEdge));
		beg++;
	}

	leftEdgeMap = EdgeNoiseRemoveOnDirection(leftEdgeMap);
	upEdgeMap = EdgeNoiseRemoveOnDirection(upEdgeMap);
	rightEdgeMap = EdgeNoiseRemoveOnDirection(rightEdgeMap);
	botEdgeMap = EdgeNoiseRemoveOnDirection(botEdgeMap);

	Float velLeft, velUp, velRight, velBot;
	motionDir dirLeft, dirUp, dirRight, dirBottom;
	dirLeft = checkFrontierAndGetVel(leftEdgeMap, velLeft);

	dirUp = checkFrontierAndGetVel(upEdgeMap, velUp);

	dirRight = checkFrontierAndGetVel(rightEdgeMap, velRight);

	dirBottom = checkFrontierAndGetVel(botEdgeMap, velBot);


	Float leftRightVel = 0, topBotVel = 0;
	if ((dirLeft == forwardDir&& dirRight == forwardDir) || (dirLeft == noMove&& dirRight == forwardDir)||(dirLeft==forwardDir&&dirRight==noMove))
	{
		if (dirRight == forwardDir)
			leftRightVel = velRight;
		else
			leftRightVel = velLeft;
	}
	
}

map<UInt, UInt> MovingObject::EdgeNoiseRemoveOnDirection(map<UInt, UInt> edgeMap)
{
	if (edgeMap.size() >= 4)
	{
		auto begVec1 = edgeMap.begin();
		auto begVec2 = begVec1;
		begVec2++;
		auto begVec3 = begVec2;
		begVec3++;
		auto begVec4 = begVec3;
		begVec4++;

		Bool edge2Del = false;

		while (begVec4 != edgeMap.end())
		{
			edge2Del = false;
			int delta21 = int(begVec2->second) - int(begVec1->second);
			if (abs(delta21) >= 2 * CTUsize)
			{
				int delta32 = int(begVec3->second) - int(begVec2->second);
				int delta42 = int(begVec4->second) - int(begVec2->second);
				if (delta21 > 0)
				{
					if (delta32 <= -1 * CTUsize || delta42 <= -1 * CTUsize)
						edge2Del = true;
				}
				else
				{
					if (delta32 >= 1 * CTUsize || delta42 >= 1 * CTUsize)
						edge2Del = true;
				}
			}

			if (edge2Del == true)
			{
				edgeMap.erase(begVec2);
				begVec2 = begVec1;
				begVec2++;
				begVec3 = begVec2;
				begVec3++;
				begVec4 = begVec3;
				begVec4++;
				continue;
			}

			begVec1++;
			begVec2++;
			begVec3++;
			begVec4++;
		}
	}
	return edgeMap;

}


motionDir MovingObject::checkFrontierAndGetVel(map<UInt, UInt> edgeMap, Float& velocity)
{
	motionDir direction = noMove;
	if (edgeMap.size() < 8)
		return noMove;

	auto curr = edgeMap.end();
	curr--;

	map<UInt, UInt> endGroup;
	UInt num = 1;
	while (num <= 5)
	{
		endGroup.insert(*curr);
		num++;
		curr--;
	}

	curr = edgeMap.end();
	curr--;

	auto prior = edgeMap.end();
	UInt i = 1;
	while (i <= 3)
	{
		prior--;
		UInt edge = prior->second;
		i++;
	}

	auto prior5 = prior;
	for (int i = 0; i < 4; i++)
		prior5--;

	UInt currEdge = curr->second;
	UInt priorEdge = 0;
	while (prior5 != edgeMap.begin())
	{
		priorEdge = prior->second;


		if (currEdge != priorEdge)
		{
			auto temp = prior;
			map<UInt, UInt> priorGroup;
			UInt num = 1;

			while (num <= 5)
			{
				priorGroup.insert(*temp);
				num++;
				temp--;
			}

			Bool match = false;
			if (currEdge > priorEdge)
			{
				match = checkConditionAndGetVel(endGroup, priorGroup, true, velocity);
				if (match)
				{
					direction = forwardDir;

					return direction;
				}
			}
			else
			{
				match = checkConditionAndGetVel(endGroup, priorGroup, false, velocity);
				if (match)
				{
					direction = backwardDir;
					return direction;
				}
			}

		}
		prior--;
		prior5--;

	}

	return direction;

}

Bool MovingObject::checkConditionAndGetVel(map<UInt, UInt> edgeMap1, map<UInt, UInt> edgeMap2, Bool Vec1LargeThanVec2, Float& velocity)
{
	UInt score = 0;
	if (Vec1LargeThanVec2)
	{

		auto beg = edgeMap1.begin();
		while (beg != edgeMap1.end())
		{
			auto beg2 = edgeMap2.begin();
			while (beg2 != edgeMap2.end())
			{
				if ((beg->second) < (beg2->second))
					return false;
				else
				if ((int(beg->second) - int(beg2->second)) >= CTUsize)
					score++;
				beg2++;
			}
			beg++;
		}
	}
	else
	{

		auto beg = edgeMap1.begin();
		while (beg != edgeMap1.end())
		{
			auto beg2 = edgeMap2.begin();
			while (beg2 != edgeMap2.end())
			{
				if ((beg->second) > (beg2->second))
					return false;
				else
				if ((int(beg->second) - int(beg2->second)) <= (-1 * CTUsize))
					score++;
				beg2++;
			}
			beg++;
		}

	}

	UInt totalNum = edgeMap1.size()*edgeMap2.size();
	if (Float(score) / Float(totalNum) >= 0.8)
	{
		//calculate the average velocity
		Float averageEdge1 = 0;
		auto beg = edgeMap1.begin();
		while (beg != edgeMap1.end())
		{
			averageEdge1 += beg->second;
			beg++;
		}
		averageEdge1 = Float(averageEdge1) / Float(edgeMap1.size());
		Float averageEdge2 = 0;
		beg = edgeMap2.begin();
		while (beg != edgeMap2.end())
		{
			averageEdge2 += beg->second;
			beg++;
		}
		averageEdge2 = Float(averageEdge2) / Float(edgeMap2.size());

		auto temp = edgeMap1.end();
		temp--;
		UInt pic1 = temp->first;
		temp = edgeMap2.end();
		temp--;
		UInt pic2 = temp->first;
		if (pic1 <= pic2)
		{
			cout << "the pic1 is smaller than pic2, error" << endl;
			exit(0);
		}
		velocity = (averageEdge1 - averageEdge2) / Float(pic1 - pic2);


		return true;
	}
	else
		return false;
}