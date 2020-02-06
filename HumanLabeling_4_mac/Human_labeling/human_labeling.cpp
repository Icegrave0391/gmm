// image_generate_from_video.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

//#include <opencv/cv.h>
//#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>


using namespace std;
using namespace cv;

Mat img0, img1, IMG, ori;
int imgNum = 0;
int fileIndex = -1;

//paras to calculate the offsets of one CTU
int CTUsize = -1;
int imgWidth = -1;
int imgHeight = -1;
int CtuInWidth = -1;
int CtuInHeight = -1;
void GetFileNames(string path,vector<string>& filenames)
{
    DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(path.c_str())))
        return;
    while((ptr = readdir(pDir))!=0) {
//        cout << ""
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
            cout << "++++++++++filePath: " << path.c_str()<<endl
            << "++++++++++filename: " << ptr->d_name<<endl;
            char * loc = strstr(ptr->d_name, ".jpg");
            if(loc != NULL){
                filenames.push_back(path + ptr->d_name);
            }
        }
    }
    closedir(pDir);
}

vector<int> Rect2CTUs(int x, int y, int width, int height)
{
	vector<int> ret;
	assert(imgWidth != -1 && imgHeight != -1 && CtuInWidth != -1 && CtuInHeight != -1);
	for (int CtuYidx = 0; CtuYidx < CtuInHeight; CtuYidx++)
	{
		for (int CtuXidx = 0; CtuXidx < CtuInWidth; CtuXidx++)
		{
			if (x < (CtuXidx + 1) * CTUsize &&
				CtuXidx * CTUsize  < (x + width) &&
				(CtuYidx + 1) *CTUsize > y &&
				CtuYidx *CTUsize < (y + height))
				ret.push_back(CtuYidx*CtuInWidth + CtuXidx);
		}
	}
	return ret;
}
inline int PointToCTU(int x, int y)
{
	assert(imgWidth != -1 && imgHeight != -1 && CtuInWidth != -1 && CtuInHeight != -1);
	for (int CtuYidx = 0; CtuYidx < CtuInHeight; CtuYidx++)
	{
		for (int CtuXidx = 0; CtuXidx < CtuInWidth; CtuXidx++)
		{
			if (x < (CtuXidx + 1) * CTUsize &&
				CtuXidx * CTUsize  < (x) &&
				(CtuYidx + 1) *CTUsize > y &&
				CtuYidx *CTUsize < (y))
				return (CtuYidx*CtuInWidth + CtuXidx);
		}
	}
	return -1;
}
enum RectStatus { inUse, removed };
struct Rectangle
{
	int x;
	int y;
	int width;
	int height;
	int id;
	vector<int> CTUoffsets;//the CTU is annotated as if offset in the picture, that is, for example, if a CTU is the 3rd CTU in the first row in the whole picture, then it's represented as "2" in this.
	RectStatus status;
	Rectangle(int id, int x, int y, int width, int height) : x(x), y(y), width(width), height(height), id(id), status(inUse) ,CTUoffsets(Rect2CTUs(x,y,width,height)){
	}
};


vector<vector<Rectangle> > rectSeq;
vector<vector<Rectangle> > predictedRectSeq;


static void _split(const string &s, char delim,vector<string> &elems) 
{
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
}
vector<string> split(const string &s, char delim) {
	vector<string> elems;
	_split(s, delim, elems);
	return elems;
}
void ReadRect(const char* fileName) 
{
	rectSeq.clear();
	assert(imgNum != 0);
	rectSeq.resize(imgNum);
	predictedRectSeq.resize(imgNum);
	//FILE *annotFile = fopen(fileName, "r");
	//char buffer[40];
	ifstream annotFile(fileName, ios::in);
	if (annotFile.fail())
		return;

	string line;
	int frameNum = -1;
	while (getline(annotFile, line))
	{
		if (line.back() == ' ')
			line.pop_back();
		vector<string> nums = split(line, ',');// nums[0] : which frame(start with 0),nums[1]: how many rect, (nums[2],nums[3],nums[4],nums[5],nums[6]) i.e. (id,x,y,w,h)
		if (nums[0].compare("CTUs") == 0)
		{
			int rectId = stoi(nums[1]);
			vector<int> CTUs;
			for (int numIdx = 2; numIdx < nums.size(); numIdx++)
				CTUs.push_back(stoi(nums[numIdx]));
			bool changedFlag = false;
			for(vector<Rectangle>::iterator I = rectSeq[frameNum].begin() ; I!=rectSeq[frameNum].end(); I++)
				if (I->id == rectId) {
					I->CTUoffsets = CTUs;
					changedFlag = true;
					break;
				}
			assert(changedFlag);
			continue;
		}
		frameNum = stoi(nums[0]);
		int numRect = stoi(nums[1]);
		for (int rectIndex = 0; rectIndex < numRect; rectIndex++)
			rectSeq[frameNum].push_back(Rectangle(stoi(nums[2 + rectIndex * 6]), stoi(nums[3 + rectIndex * 6]), stoi(nums[4 + rectIndex * 6]), stoi(nums[5 + rectIndex * 6]), stoi(nums[6 + rectIndex * 6])));		
	}
	annotFile.close();
}

//pick related paras
const int pickWidth = 3;
enum FunctionStatus{picked,draw,notUse};
enum DisplayMode{showNormal,showPred};
FunctionStatus functionStatus = notUse;
DisplayMode displayMode = showNormal;
enum pickedEdge { invalideEdge,upEdge, leftEdge, rightEdge, bottomEdge };
pickedEdge pickedRectEdge = invalideEdge;
int pickedRectIndex = -1;// valide number start with 0(to fit the array index)


//draw related paras
int drawLeftUpX = -1;
int drawLeftUpY = -1;

inline void DrawRectangle(Mat & img,int x,int y,int width,int height,int id)
{
	putText(img, to_string(id), Point(x,y-5), FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 1);
	line(img, Point(x, y), Point(x + width, y), Scalar(0, 255, 0));
	line(img, Point(x, y), Point(x, y + height), Scalar(0, 255, 0));
	line(img, Point(x + width, y), Point(x + width, y + height), Scalar(0, 255, 0));
	line(img, Point(x, y + height), Point(x + width, y + height), Scalar(0, 255, 0));
}
inline void DrawCTUs(Mat & img,vector<int> CTUoffsets)
{
	assert(img.depth() == CV_8U);
	const int channels = img.channels();
	assert(channels == 3);
	for (vector<int>::iterator I = CTUoffsets.begin(); I != CTUoffsets.end(); I++)
	{
		int x = *I % CtuInWidth;
		int y = *I / CtuInWidth;
		for (int i = y*CTUsize; i < (y + 1) * CTUsize; i++)
			for (int j = x*CTUsize; j < (x + 1)*CTUsize; j++)
				if (i >= imgHeight || j >= imgWidth)
					continue;
				else
					img.ptr<uchar>(i)[j*channels+1] = img.ptr<uchar>(i)[j*channels+1] + 50 > 254 ? 254 : img.ptr<uchar>(i)[j*channels+1] + 50;
	}
}
bool RemoveIfPossible(int CTUidx)
{
	vector<Rectangle>::iterator I, Iend;
	if (displayMode == showNormal)
	{
		I = rectSeq[fileIndex].begin();
		Iend = rectSeq[fileIndex].end();
	}
	else if (displayMode == showPred)
	{
		I = predictedRectSeq[fileIndex].begin();
		Iend = predictedRectSeq[fileIndex].end();
	}
	for (; I != Iend; I++)
	{
		vector<int>::iterator ifFound = find(I->CTUoffsets.begin(), I->CTUoffsets.end(), CTUidx);
		if (ifFound != I->CTUoffsets.end())
		{
			I->CTUoffsets.erase(ifFound);
			return true;
		}
	}
	return false;
}
int FindClosest(int CTUidx)
{
	vector<int> around;
	if (CTUidx >= CtuInWidth) // is not on the upper edge of the picture
	{
		around.push_back(CTUidx - CtuInWidth);
		if (CTUidx % CtuInWidth != 0)// is not on the left edge of the picure
			around.push_back(CTUidx - CtuInWidth - 1);
		if (CTUidx % CtuInWidth != (CtuInWidth - 1))// is not on the right edge of the picture
			around.push_back(CTUidx - CtuInWidth + 1);
	}
	if (CTUidx % CtuInWidth != 0)
	{
		around.push_back(CTUidx - 1);
		if (CTUidx < (CtuInHeight - 1) * CtuInWidth)// is not on the low edge
			around.push_back(CTUidx - 1 + CtuInWidth);
	}
	if (CTUidx < (CtuInHeight - 1) * CtuInWidth)
	{
		around.push_back(CTUidx + CtuInWidth);
		if ((CTUidx % CtuInWidth) != (CtuInWidth - 1))// is not on the right edge of the picture
			around.push_back(CTUidx + CtuInWidth + 1);
	}
	if ((CTUidx % CtuInWidth) != (CtuInWidth - 1))
		around.push_back(CTUidx + 1);

	vector<Rectangle>::iterator I, Iend;
	if (displayMode == showNormal)
	{
		I = rectSeq[fileIndex].begin();
		Iend = rectSeq[fileIndex].end();
	}
	else if (displayMode == showPred)
	{
		I = predictedRectSeq[fileIndex].begin();
		Iend = predictedRectSeq[fileIndex].end();
	}
	for (; I != Iend; I++)
	{
		for (vector<int>::iterator thisCTU = I->CTUoffsets.begin(); thisCTU != I->CTUoffsets.end(); thisCTU++)
		{
			if (find(around.begin(), around.end(), *thisCTU) != around.end())
			{
				I->CTUoffsets.push_back(CTUidx);
				return I->id;// one the neigboring 8 CTUs of the picked CTU has already beem marked, so this picked CTU is to be added into this 
			}
		}
	}
	return -1;
}
void mouseHandler(int event, int x, int y, int flags, void *param) 
{
	if (event == EVENT_LBUTTONDOWN) 
	{
		switch (functionStatus)
		{
		case notUse:
			{
				int rectToPick = 0;
				vector<Rectangle>::iterator I, Iend;
				if (displayMode == showNormal)
				{
					I = rectSeq[fileIndex].begin();
					Iend = rectSeq[fileIndex].end();
				}
				else if (displayMode == showPred)
				{
					I = predictedRectSeq[fileIndex].begin();
					Iend = predictedRectSeq[fileIndex].end();
				}
				else
					assert(false);
				for (; I != Iend; ++I, ++rectToPick)
				{
					Rect tempRect(I->x, I->y - pickWidth, I->width, pickWidth * 2);
					if (tempRect.contains(Point(x, y)))
					{
						functionStatus = picked;
						pickedRectIndex = rectToPick;
						pickedRectEdge = upEdge;
						break;
					}
					tempRect = Rect(I->x - pickWidth, I->y, pickWidth * 2, I->height);
					if (tempRect.contains(Point(x, y)))
					{
						functionStatus = picked;
						pickedRectIndex = rectToPick;
						pickedRectEdge = leftEdge;
						break;
					}
					tempRect = Rect(I->x + I->width - pickWidth, I->y, pickWidth * 2, I->height);
					if (tempRect.contains(Point(x, y)))
					{
						functionStatus = picked;
						pickedRectIndex = rectToPick;
						pickedRectEdge = rightEdge;
						break;
					}
					tempRect = Rect(I->x, I->y + I->height - pickWidth, I->width, 2 * pickWidth);
					if (tempRect.contains(Point(x, y)))
					{
						functionStatus = picked;
						pickedRectIndex = rectToPick;
						pickedRectEdge = bottomEdge;
						break;
					}
				}
				if (functionStatus == picked)
					break;
				int pickedCTU = PointToCTU(x, y);
				while (pickedCTU == -1)
				{
					cout << "illegal click, click again" << endl;
					break;
				}
				bool removed = RemoveIfPossible(pickedCTU);
				if (removed)
					break;
				int closestId = FindClosest(pickedCTU);
				if (closestId == -1)// no neigbor is found
				{
					functionStatus = draw;
					drawLeftUpX = x;
					drawLeftUpY = y;
				}
				break;
			}
			
		case picked:
		{
			Rectangle * ptrRect = nullptr;
			if (displayMode == showPred)
				ptrRect = &predictedRectSeq[fileIndex][pickedRectIndex];
			else
				ptrRect = &rectSeq[fileIndex][pickedRectIndex];
			int oldHeight = ptrRect->height;
			int oldWidth = ptrRect->width;
			int oldX = ptrRect->x;
			int oldY = ptrRect->y;
			vector<int> CTUtoModify;
			switch (pickedRectEdge)
			{
			case upEdge:
				{
					int gap = oldY - y;
					if (gap > 0) // new is bigger,i.e. y is smaller than oldY
					{
						CTUtoModify = Rect2CTUs(oldX, y, oldWidth, gap);
						for (vector<int>::iterator I = CTUtoModify.begin(); I != CTUtoModify.end(); I++)
							if(find(ptrRect->CTUoffsets.begin(), ptrRect->CTUoffsets.end(),*I) == ptrRect->CTUoffsets.end())
								ptrRect->CTUoffsets.push_back(*I);
					}
					else if (gap < 0 && ( oldY  < (y - (y % CTUsize) - 1) ))// new is smaller
					{
						CTUtoModify = Rect2CTUs(oldX, oldY, oldWidth, -(oldY - (y - (y % CTUsize) - 1)));
						vector<int>::iterator I = ptrRect->CTUoffsets.begin();
						for (; I != ptrRect->CTUoffsets.end();)
						{
							if (find(CTUtoModify.begin(), CTUtoModify.end(), *I) != CTUtoModify.end())// this element should be deleted
								I = ptrRect->CTUoffsets.erase(I);
							else
								++I;
						}
					}
					ptrRect->height += (oldY - y);
					assert(ptrRect->height > 0);
					ptrRect->y = y;
					break;
				}
			case leftEdge:
				{
					int gap = oldX - x;
					if (gap > 0)
					{
						CTUtoModify = Rect2CTUs(x, oldY, gap , oldHeight);
						for (vector<int>::iterator I = CTUtoModify.begin(); I != CTUtoModify.end(); I++)
							if (find(ptrRect->CTUoffsets.begin(), ptrRect->CTUoffsets.end(), *I) == ptrRect->CTUoffsets.end())
								ptrRect->CTUoffsets.push_back(*I);
					}
					else if (gap < 0 && (oldX   < (x - x % CTUsize - 1) - 1))
					{
						CTUtoModify = Rect2CTUs(oldX, oldY, -(oldX - (x - x% CTUsize - 1) - 1), oldHeight);
						vector<int>::iterator I = ptrRect->CTUoffsets.begin();
						for (; I != ptrRect->CTUoffsets.end();)
						{
							if (find(CTUtoModify.begin(), CTUtoModify.end(), *I) != CTUtoModify.end())// this element should be deleted
								I = ptrRect->CTUoffsets.erase(I);
							else
								++I;
						}
					}
					ptrRect->width += (oldX - x);
					assert(ptrRect->width > 0);
					ptrRect->x = x;
					break;

				}
			case rightEdge:
				{
					int gap = x - (oldX + oldWidth);
					if (gap > 0)
					{
						CTUtoModify = Rect2CTUs(oldX + oldWidth, oldY, gap, oldHeight);
						for (vector<int>::iterator I = CTUtoModify.begin(); I != CTUtoModify.end(); I++)
							if (find(ptrRect->CTUoffsets.begin(), ptrRect->CTUoffsets.end(), *I) == ptrRect->CTUoffsets.end())
								ptrRect->CTUoffsets.push_back(*I);
					}
					else if (gap < 0 && (x + (CTUsize - x % CTUsize) + 1) < (oldX + oldWidth))
					{
						CTUtoModify = Rect2CTUs((x + (CTUsize - x % CTUsize) + 1), oldY, (oldX + oldWidth) - (x + (CTUsize - x % CTUsize) + 1), oldHeight);
						vector<int>::iterator I = ptrRect->CTUoffsets.begin();
						for (; I != ptrRect->CTUoffsets.end();)
						{
							if (find(CTUtoModify.begin(), CTUtoModify.end(), *I) != CTUtoModify.end())// this element should be deleted
								I = ptrRect->CTUoffsets.erase(I);
							else
								++I;
						}
					}
					ptrRect->width += (x - (oldX + oldWidth));
					assert(ptrRect->width > 0);
					break;
				}
			case bottomEdge:
				{
					int gap = (y - (oldY + oldHeight));
					if (gap > 0)
					{
						CTUtoModify = Rect2CTUs(oldX, oldY + oldHeight, oldWidth, y - oldY - oldHeight);
						for (vector<int>::iterator I = CTUtoModify.begin(); I != CTUtoModify.end(); I++)
							if (find(ptrRect->CTUoffsets.begin(), ptrRect->CTUoffsets.end(), *I) == ptrRect->CTUoffsets.end())
								ptrRect->CTUoffsets.push_back(*I);
					}
					else if (gap < 0 && (y + (CTUsize - y% CTUsize) + 1) < (oldY + oldHeight))
					{
						CTUtoModify = Rect2CTUs(oldX, y + (CTUsize - y% CTUsize) + 1, oldWidth, (oldY + oldHeight) - (y + (CTUsize - y% CTUsize) + 1));
						vector<int>::iterator I = ptrRect->CTUoffsets.begin();
						for (; I != ptrRect->CTUoffsets.end();)
						{
							if (find(CTUtoModify.begin(), CTUtoModify.end(), *I) != CTUtoModify.end())// this element should be deleted
								I = ptrRect->CTUoffsets.erase(I);
							else
								++I;
						}
					}
					ptrRect->height += (y - (oldY + oldHeight));
					assert(ptrRect->height > 0);
					break;
				}
			default:
				assert(false);
			}
			pickedRectEdge = invalideEdge;
			pickedRectIndex = -1;
			functionStatus = notUse;
			break;
		}
		case draw:
		{
			int boxId;
			cin >> boxId;
			int numBoxes = rectSeq[fileIndex].size();
			while (boxId < numBoxes)// [0,numBox-2] means to add to the existing box
			{
				cout << "input number should not be smaller than existing box id: " << numBoxes - 1 << endl;
				cin >> boxId;
			}
			rectSeq[fileIndex].push_back(Rectangle(boxId, x > drawLeftUpX ? drawLeftUpX : x,
				y > drawLeftUpY ? drawLeftUpY : y,
				abs(drawLeftUpX - x),
				abs(drawLeftUpY - y)));
			drawLeftUpX = -1;
			drawLeftUpY = -1;
			functionStatus = notUse;
			break;
		}
		default:
			break;
		}
	} 
	else if(event == EVENT_RBUTTONDOWN) {
		int rectToPick = 0;
		for (vector<Rectangle>::iterator I = rectSeq[fileIndex].begin(); I != rectSeq[fileIndex].end(); ++I, ++rectToPick)
		{
			Rect tmpRect1(I->x, I->y - pickWidth, I->width, pickWidth * 2);
			Rect tmpRect2(I->x - pickWidth, I->y, pickWidth * 2, I->height);
			Rect tmpRect3(I->x + I->width - pickWidth, I->y, pickWidth * 2, I->height);
			Rect tmpRect4(I->x, I->y + I->height - pickWidth, I->width, 2 * pickWidth);
			if (tmpRect1.contains(Point(x, y)) || tmpRect2.contains(Point(x, y)) || tmpRect3.contains(Point(x, y)) || tmpRect4.contains(Point(x, y)))
			{
				int idToRemove = I->id;
				//travese to remove all the rest
				for (int tmpFileIdx = fileIndex; tmpFileIdx < imgNum; tmpFileIdx++)
				{
					for (vector<Rectangle>::iterator I1 = rectSeq[tmpFileIdx].begin(); I1 != rectSeq[tmpFileIdx].end(); ++I1, ++rectToPick)
					{
						if (I1->id == idToRemove)
							I1->status = removed;
					}
				}
				break;
			}
		}
	} 
	else if(event == EVENT_MOUSEMOVE) 
	{
		img0 = img1.clone();
		ori = img1.clone();
		//draw cross
		line(ori, Point(x, 0), Point(x, ori.size().height), Scalar(0, 255, 0));
		line(ori, Point(0, y), Point(ori.size().width, y), Scalar(0, 255, 0));
		line(img0, Point(x, 0), Point(x, img0.size().height), Scalar(0, 255, 0));
		line(img0, Point(0, y), Point(img0.size().width,y), Scalar(0, 255, 0));
		stringstream ss;
		ss << x << "," << y;
		string str = ss.str();
		putText(img0, str, Point(0, img0.size().height - 5), FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 1);
		// draw existing rect according to status
		int rectIdx = 0;
		vector<Rectangle>::iterator I, Iend;
		if (displayMode == showNormal)
		{
			I = rectSeq[fileIndex].begin();
			Iend = rectSeq[fileIndex].end();
		}
		else if (displayMode == showPred)
		{
			I = predictedRectSeq[fileIndex].begin();
			Iend = predictedRectSeq[fileIndex].end();
		}
		for (; I !=Iend; ++I, ++rectIdx)
		{
			if (pickedRectIndex != rectIdx || (pickedRectIndex == rectIdx && functionStatus != picked))
			{
				DrawRectangle(img0, I->x, I->y, I->width, I->height, I->id);
				DrawCTUs(img0, I->CTUoffsets);
			}
				
			else if (functionStatus == picked && pickedRectIndex == rectIdx)
			{	
				int oldHeight = I->height;
				int oldWidth = I->width;
				int oldX = I->x;
				int oldY = I->y;
				vector<int> CTUtoModify;
				if (pickedRectEdge == upEdge)
				{
					oldHeight += (oldY - y);
					assert(oldHeight > 0);
					DrawRectangle(img0, I->x, y, I->width, oldHeight, I->id);
				}
				else if (pickedRectEdge == leftEdge)
				{
					oldWidth += (oldX - x);
					assert(oldWidth > 0);
					DrawRectangle(img0, x, oldY, oldWidth, oldHeight, I->id);
				}
				else if (pickedRectEdge == rightEdge)
				{
					oldWidth += (x - (oldX + oldWidth));
					assert(oldWidth > 0);
					DrawRectangle(img0, oldX, oldY, oldWidth, oldHeight, I->id);
				}
				else if (pickedRectEdge == bottomEdge)
				{
					oldHeight += (y - (oldY + oldHeight));
					assert(oldHeight > 0);
					DrawRectangle(img0, oldX, oldY, oldWidth, oldHeight, I->id);
				}
				else
					assert(false);
			}
		}
		// draw additional rect if in draw mode
		if (functionStatus == draw)
		{
			DrawRectangle(img0, x > drawLeftUpX ? drawLeftUpX : x,
				y > drawLeftUpY ? drawLeftUpY : y,
				abs(drawLeftUpX - x),
				abs(drawLeftUpY - y), -1);
		}
		resize(img0, IMG, Size(), 2.0, 2.0, INTER_CUBIC);
		imshow("image", img0);
		imshow("bigWindow", IMG);
		imshow("ori", ori);
	}
}
int retriveIndex(char * fileName)
{
	string filename(fileName);
	size_t startPos = filename.find_first_of("0123456789");
	size_t endPos = filename.find_last_of("0123456789");
	assert(startPos != string::npos && endPos != string::npos);
	int fileIndex = atoi(filename.substr(startPos, endPos - startPos + 1).c_str());
	return fileIndex - 1;
}
string GetParent(string folder,string & folderName)
{
	size_t pos1 = folder.find_last_of('/');
	assert(pos1 != string::npos);
	size_t pos2 = folder.substr(0,pos1).find_last_of('/');
	if (pos2 == pos1 - 1)// consecutive backslashes
	{
		pos1 = pos2;
		pos2 = folder.substr(0, pos2).find_last_of('/');
	}
	else if (pos2 == string::npos || pos2 < (pos1 - 1))
		pos2 = pos2;
	else
		assert(false);
	folderName = folder.substr(pos2 + 1, pos1  - pos2 - 1);
	return folder.substr(0, pos2) + '/';
}
string GetImgName(char * filename, int index)
{
	string  fileNameStr = filename;
	size_t startPos = fileNameStr.find_first_of("0123456789");
	size_t endPos = fileNameStr.find_last_of("0123456789");
	int length = endPos - startPos + 1;
	stringstream ss;
	ss << fileNameStr.substr(0, startPos) << setfill('0') << setw(length) << index << fileNameStr.substr(endPos + 1);
	return ss.str();
}

Mat GenerateCanny(Mat & img)
{
	Mat detected_edges, src_gray;
	cvtColor(img1, src_gray, CV_BGR2GRAY);
	blur(src_gray, detected_edges, Size(3, 3));
	Canny(detected_edges, detected_edges, 120, 120 * 3, 3);
	return detected_edges;
	//imshow("temp", detected_edges);
}

inline int SanityCheck(int value, int bound)
{
	if (value < 0)
		return 0;
	else if (value > bound)
		return bound;
	else
		return value;
}
const int adjustStep = 10;
const int pixelMargin = 40;
void Predict(Mat & oldCanny, Mat & newCanny, int oldFileIndex, int rectId)
{
	int channels = oldCanny.channels();
	int nRows = oldCanny.rows;
	int nCols = oldCanny.cols * channels;
	uchar pixel;
	int oldEdgePixelNum = 0;
	int newEdgePixelNum = 0;
	Rectangle tmpRect(-1, -1, -1, -1, -1);
	//get the old rect, search pred queue first
	for (vector<Rectangle>::iterator I = predictedRectSeq[fileIndex].begin(); I != predictedRectSeq[fileIndex].end(); I++) {
		if (I->id == rectId)
		{
			tmpRect.x = I->x;tmpRect.y = I->y;tmpRect.width = I->width;tmpRect.height = I->height;tmpRect.id = I->id;
		}
	}
	if (tmpRect.x == -1) {// not found
		for (vector<Rectangle>::iterator I = rectSeq[fileIndex].begin(); I != rectSeq[fileIndex].end(); I++) {
			if (I->id == rectId)
			{
				tmpRect.x = I->x;tmpRect.y = I->y;tmpRect.width = I->width;tmpRect.height = I->height;tmpRect.id = I->id;
			}
		}
	}
	assert(tmpRect.x != -1);
	//ofstream pixelFile("canny_pixel.txt");
	//get the edge pixel num in the old canny map
	for (int i = tmpRect.y; i < (tmpRect.y+tmpRect.height); ++i)
		for (int j = tmpRect.x; j < (tmpRect.x +tmpRect.width); ++j)
			if (oldCanny.ptr<uchar>(i)[j] > 0) //i.e. the edge map should has only two pixel values i.e. 0, 255 
				oldEdgePixelNum++;
	
	// if any of the four edges of the rect intersect with the edges detected, pull them out-ward for adjustStep pixels
	//upper edge of the rectangle
	bool intersected = false;
	for (int colIndex = tmpRect.x; colIndex < (tmpRect.x + tmpRect.width); colIndex++)
		if (newCanny.ptr<uchar>(tmpRect.y)[colIndex] > 0)
			intersected = true;
	if (intersected)// cross the detected edge, so pull back a little bit
	{
		tmpRect.y = SanityCheck(tmpRect.y - adjustStep, nRows - 1);
		tmpRect.height = SanityCheck(tmpRect.y + tmpRect.height + adjustStep, nRows - 1) - tmpRect.y;
	}
	intersected = false;
	//left edge of the rectangle
	for (int rowIndex = tmpRect.y; rowIndex < (tmpRect.y + tmpRect.height); rowIndex++)
		if (newCanny.ptr<uchar>(rowIndex)[tmpRect.x] > 0)
			intersected = true;
	if (intersected)
	{
		tmpRect.x = SanityCheck(tmpRect.x - adjustStep,nCols - 1);
		tmpRect.width = SanityCheck(tmpRect.width + tmpRect.x +  adjustStep,nCols - 1) - tmpRect.x;
	}
	intersected = false;
	//right edge of the rectangle
	for (int rowIndex = tmpRect.y; rowIndex < (tmpRect.y + tmpRect.height); rowIndex++)
		if (newCanny.ptr<uchar>(rowIndex)[tmpRect.x + tmpRect.width] > 0)
			intersected = true;
	if (intersected)
		tmpRect.width = SanityCheck(tmpRect.width + tmpRect.x + adjustStep, nCols - 1) - tmpRect.x;
	intersected = false;
	// bottom edge of the rectangle
	for (int columnIndex = tmpRect.x; columnIndex < (tmpRect.x + tmpRect.width); columnIndex++)
		if (newCanny.ptr<uchar>(tmpRect.y + tmpRect.height)[columnIndex] > 0)
			intersected = true;
	if (intersected)
		tmpRect.height = SanityCheck(tmpRect.y + tmpRect.height + adjustStep, nRows - 1) - tmpRect.y;

	//shrink to fit
	//shrink upper edge to meet the detected edge
	// no more sanity check for the shrinking
	intersected = false;
	do 
	{
		tmpRect.y += 1;
		tmpRect.height -= 1;
		for (int colIndex = tmpRect.x; colIndex < (tmpRect.x + tmpRect.width); colIndex++)
			if (newCanny.ptr<uchar>(tmpRect.y)[colIndex] > 0)
				intersected = true;
	} while (intersected == false);
	//shrink left edge to meet the detected edge
	intersected = false;
	do {
		tmpRect.x += 1;
		tmpRect.width -= 1;
		for (int rowIndex = tmpRect.y; rowIndex < (tmpRect.y + tmpRect.height); rowIndex++)
			if (newCanny.ptr<uchar>(rowIndex)[tmpRect.x] > 0)
				intersected = true;
	} while (intersected == false);
	//shrink right edge to meet the detected edge
	intersected = false;
	do {
		tmpRect.width -= 1;
		for (int rowIndex = tmpRect.y; rowIndex < (tmpRect.y + tmpRect.height); rowIndex++)
			if (newCanny.ptr<uchar>(rowIndex)[tmpRect.x + tmpRect.width] > 0)
				intersected = true;
	} while (intersected == false);
	//shrink bottom edge to meet the detected edge
	intersected = false;
	do {
		tmpRect.height -= 1;
		for (int columnIndex = tmpRect.x; columnIndex < (tmpRect.x + tmpRect.width); columnIndex++)
			if (newCanny.ptr<uchar>(tmpRect.y + tmpRect.height)[columnIndex] > 0)
				intersected = true;
	} while (intersected == false);

	//calculate the detected edge pixel num in current region
	for (int i = tmpRect.y; i < (tmpRect.y + tmpRect.height); ++i)
		for (int j = tmpRect.x; j < (tmpRect.x + tmpRect.width); ++j)
			if (newCanny.ptr<uchar>(i)[j] > 0) //i.e. the edge map should has only two pixel values i.e. 0, 255 
				newEdgePixelNum++;

	//further shrink the rectangle to make the pixel num consistent, shrink one edge for one pixel at one time
	int count = 0;
	while (newEdgePixelNum > (oldEdgePixelNum + pixelMargin))
	{
		switch (count % 4)
		{
		case 0:
			tmpRect.y += 1;tmpRect.height -= 1;break;
		case 1:
			tmpRect.x += 1;tmpRect.width -= 1;break;
		case 2:
			tmpRect.width -= 1; break;
		case 3:
			tmpRect.height -= 1; break;
		default:
			assert(false); break;
		}
		newEdgePixelNum = 0;
		for (int i = tmpRect.y; i < (tmpRect.y + tmpRect.height); ++i)
			for (int j = tmpRect.x; j < (tmpRect.x + tmpRect.width); ++j)
				if (newCanny.ptr<uchar>(i)[j] > 0) //i.e. the edge map should has only two pixel values i.e. 0, 255 
					newEdgePixelNum++;
		count++;
	}
	tmpRect.CTUoffsets = Rect2CTUs(tmpRect.x, tmpRect.y, tmpRect.width, tmpRect.height);
	predictedRectSeq[oldFileIndex+1].push_back(tmpRect);
	return;
}
int main(int argc, char* argv[])
{
	if (argc != 4) {
		cerr<<"Usage: "<<argv[0]<<"   <dataFolder> <# of pics> <CTUsize>"<<endl<<endl;
		return -1;
	}

	string dataFolder = argv[1];
	
	imgNum = atoi(argv[2]);
	CTUsize = atoi(argv[3]);
	size_t len = dataFolder.size();
	if (len == 0) {
		cerr<<"ERROR: dataFolder cannot be empty"<<endl<<endl;
		return -1;
	}

	// Add '\' at the end if needed
	if (dataFolder[len - 1] != '/')
		dataFolder += '/';
	string folderName;
	string parentFolder = GetParent(dataFolder, folderName);

//	struct _finddata_t file;
//	long lf;
//
//	if((lf = _findfirst((dataFolder + "*.jpg").c_str(), &file)) == -1) {
//		cerr<<"No jpg file can be found in folder: "<<dataFolder<<endl<<endl;
//		return -1;
//	}
    vector<string> filenames;
    printf("+++++test+++++");
    GetFileNames(dataFolder, filenames);
    string str = filenames[0];
    char *file_name = new char[str.length() + 1];
    strcpy(file_name, str.c_str());
    cout << "++++++++++++test file name:" << file_name << endl;
	fileIndex = retriveIndex(file_name);
    cout <<"+++++++++++++file index: " << fileIndex << endl;
	assert(fileIndex == 0);

	namedWindow("image", WINDOW_AUTOSIZE);
	namedWindow("bigWindow", WINDOW_AUTOSIZE);
	namedWindow("ori", WINDOW_AUTOSIZE);
	namedWindow("temp", WINDOW_AUTOSIZE);
	setMouseCallback( "image", mouseHandler, NULL );

	cout<<endl<<"Processing image "<<file_name<<" ..."<<endl<<endl;

	img0 = imread(file_name, 1);
    if (img0.cols == 0) {
     cout << "Error reading file " << file_name << endl;
     return -1;
    }
	img1 = img0.clone();// img1 is img0's back up
	ori = img0.clone();
	imgWidth = img0.cols;
	imgHeight = img0.rows;
	CtuInWidth = imgWidth / CTUsize + (imgWidth%CTUsize ? 1 : 0);
	CtuInHeight = imgHeight / CTUsize + (imgHeight % CTUsize ? 1 : 0);
	resize(img0, IMG, Size(), 2.0, 2.0, INTER_CUBIC);

	string dataFile = parentFolder + folderName + ".txt";
	ReadRect(dataFile.c_str());

	imshow("image", img0);
	imshow("bigWindow", IMG);
	imshow("ori", ori);

	while(true) 
	{
		cout<<endl<<"Waiting for keying a key"<<endl<<endl;
		int key = waitKey(0);
		if(key == 'b')// backward i.e. last frame
		{
			int newFileIndex = fileIndex - 1;
			fileIndex = newFileIndex < 0 ? 0 : newFileIndex;
			string lastImg = GetImgName(file_name, fileIndex + 1);
			cout << endl << "Processing image " << lastImg << " ..." << endl << endl;
            printf("handle filepath: %s\n" , lastImg.c_str());
			img0 = imread(lastImg.c_str());
			img1 = img0.clone();
		}
		else if (key == 'f') //forward i.e. next frame
		{
			// Get the next file
			displayMode = showNormal;
			int newFileIndex = fileIndex + 1;
			fileIndex = newFileIndex >(imgNum - 1) ? imgNum - 1 : newFileIndex;
			string lastImg = GetImgName(file_name, fileIndex + 1);
			cout << endl << "Processing image " << lastImg << " ..." << endl << endl;
			img0 = imread(lastImg.c_str());
			img1 = img0.clone();
		}
		else if (key == 'c')// apply canny based prediction and forward based on the assumption that the bbox has been manually fine-tuned
		{
			int newFileIndex = fileIndex + 1;
			fileIndex = newFileIndex >(imgNum - 1) ? imgNum - 1 : newFileIndex;
			if (fileIndex == newFileIndex)
			{
				// check if already predict
				displayMode = showPred;
				string lastImg = GetImgName(file_name, fileIndex + 1);
				if (predictedRectSeq[fileIndex].size() == 0)
				{
					Mat cannyOld = GenerateCanny(img1);
					cout << endl << "Canny processing image " << lastImg << " ..." << endl << endl;
					img0 = imread(lastImg.c_str());
					img1 = img0.clone();
					Mat cannyNew = GenerateCanny(img1);
					vector<Rectangle>::iterator I;
					vector<Rectangle>::iterator Iend;
					if ((displayMode == showPred && predictedRectSeq[fileIndex - 1].size() == 0) || displayMode == showNormal) {// maybe the predicted results are flushed into the rectSeq	
						I = rectSeq[fileIndex - 1].begin();
						Iend = rectSeq[fileIndex - 1].end();
					}
					else {
						I = predictedRectSeq[fileIndex - 1].begin();
						Iend = predictedRectSeq[fileIndex - 1].end();
					}
					for ( ;I!=Iend;I++)
						Predict(cannyOld, cannyNew, fileIndex - 1, I->id);// put the result into predictedRectSeq
				}
				else
				{
					cout << endl << "Canny cached image " << lastImg << " ..." << endl << endl;
					img0 = imread(lastImg.c_str());
					img1 = img0.clone();
				}
					
			}
			else//reach the last frame
				cout << "Reached the last frame, cannot perform canny prediction" << endl;
		}
		else if (key == 's')// save i.e. flush the changes into the file
		{
			// store the suppressed hotspots into files.
			cout << endl << "Updating hotspot file ..." << endl << endl;
			FILE *wid = fopen(dataFile.c_str(), "w");
			for (int frameIndex = 0; frameIndex < rectSeq.size(); frameIndex++)
			{
				int valideNum = 0;
				for (int recIndex = 0; recIndex < rectSeq[frameIndex].size(); recIndex++)
					if (rectSeq[frameIndex][recIndex].status != removed)
						valideNum++;
				fprintf(wid, "%d,%d", frameIndex, valideNum);
				for (int recIndex = 0; recIndex < rectSeq[frameIndex].size(); recIndex++)
					if (rectSeq[frameIndex][recIndex].status != removed)
						fprintf(wid, ",%d,%d,%d,%d,%d,null", rectSeq[frameIndex][recIndex].id, rectSeq[frameIndex][recIndex].x, rectSeq[frameIndex][recIndex].y,
							rectSeq[frameIndex][recIndex].width, rectSeq[frameIndex][recIndex].height);// null is added in order to be compatible with the annotator
				fprintf(wid, "\n");
				for (int recIndex = 0; recIndex < rectSeq[frameIndex].size(); recIndex++)
				{
					if (rectSeq[frameIndex][recIndex].status != removed)
					{
						fprintf(wid, "CTUs,%d", rectSeq[frameIndex][recIndex].id);
						for (vector<int>::iterator I = rectSeq[frameIndex][recIndex].CTUoffsets.begin(); I != rectSeq[frameIndex][recIndex].CTUoffsets.end(); I++)
							fprintf(wid, ",%d", *I);
						fprintf(wid, "\n");
					}
				}
			}
			fclose(wid);
		}
		else if (key == 27)//'esc' key
		{
			if(functionStatus == draw)
			{	
				functionStatus = notUse;
				drawLeftUpX = -1;
				drawLeftUpY = -1;
			}
		}
		else if(key == 'g'){//flush the predicted results into the main branch i.e. the rectSeq, since that's where results are written into file
			for (int frameIdx = 0; frameIdx < imgNum; frameIdx++)
			{
				for (vector<Rectangle>::iterator predI = predictedRectSeq[frameIdx].begin(); predI != predictedRectSeq[frameIdx].end(); predI++)
				{
					for (vector<Rectangle>::iterator normalI = rectSeq[frameIdx].begin(); normalI != rectSeq[frameIdx].end(); normalI++)
					{
						if (predI->id == normalI->id)
						{
							normalI->x = predI->x; normalI->y = predI->y; normalI->width = predI->width; normalI->height = predI->height;
							normalI->CTUoffsets = predI->CTUoffsets;
						}
					}
				}
				predictedRectSeq[frameIdx].clear();
			}
			cout << "results are flushed!" << endl;
		}
		else {
			cout<<"undefined command!"<<endl;
		}

		img0 = img1.clone();
		ori = img0.clone();
		if (displayMode == showPred && predictedRectSeq[fileIndex].size() > 0)
		{
			for (vector<Rectangle>::iterator I = predictedRectSeq[fileIndex].begin(); I != predictedRectSeq[fileIndex].end(); ++I)
				if (I->status != removed)
				{
					DrawRectangle(img0, I->x, I->y, I->width, I->height, I->id);
					DrawCTUs(img0, I->CTUoffsets);
				}
		}
		else
			for (vector<Rectangle>::iterator I = rectSeq[fileIndex].begin(); I != rectSeq[fileIndex].end(); ++I)
				if(I->status != removed)
				{
					DrawRectangle(img0, I->x, I->y, I->width, I->height, I->id);
					DrawCTUs(img0, I->CTUoffsets);
				}

		resize(img0, IMG, Size(), 2.0, 2.0, INTER_CUBIC);
		imshow("image", img0);
		imshow("bigWindow", IMG);
		imshow("ori", ori);
	}

	destroyWindow("image");
	destroyWindow("ori");
	destroyWindow("bigWindow");

	return 0;
}


