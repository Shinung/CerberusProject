// FindLetter.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#pragma comment(lib, "vfw32.lib") 
#pragma comment( lib, "comctl32.lib" )

#include "opencv.hpp"

#include <ObjBase.h>

#define EXPORT_VOID		extern "C" __declspec(dllexport)void __stdcall
#define EXPORT_INT		extern "C" __declspec(dllexport)int __stdcall

using namespace cv;
using namespace std;

extern "C" __declspec(dllexport)void __stdcall Loading();
extern "C" __declspec(dllexport)void __stdcall LabelImage(char* filename);
extern "C" __declspec(dllexport)void __stdcall OptimizeLabels();
extern "C" __declspec(dllexport)char* __stdcall FindLetters();
extern "C" __declspec(dllexport)void __stdcall FreeVariables();

Mat image_ABCD[26];
Mat* Roi;
int numOfLabels;
int maxArea;
vector<KeyPoint> keypoints1, keypoints2;
Mat descriptors1, descriptors2;
Ptr<KAZE> kazeF;
Ptr<DescriptorMatcher> matcher;
vector< DMatch > goodMatches;
Mat image;
Mat stats;
//image_ABCD 배열씀
extern "C" __declspec(dllexport)void __stdcall Loading()
{
	cout<<"Loading..."<<endl;
	char Alphabet = 'a';
	for(int i = 0 ; i <= 25; i++, Alphabet++)
	{
		image_ABCD[i] = imread("Alphabets2/" + String(sizeof(Alphabet),Alphabet) + ".jpg", IMREAD_GRAYSCALE);
		if (image_ABCD[i].empty())
		{
			cout<<Alphabet<<"의 위치를 찾을 수 없습니다"<<endl;
			exit(-1);
		}
	}
	cout<<"Loading Complete"<<endl;
}

extern "C" __declspec(dllexport)void __stdcall LabelImage(char* filename)
{
	cout<<"Labeling..."<<endl;
	Mat img_binary, img_color;
	image = imread(filename, IMREAD_GRAYSCALE);

	if(image.empty())
	{
		cout<<"원본영상이 없습니다"<<endl;
		exit(-1);
	}
	threshold(image, img_binary, 127, 255, THRESH_BINARY_INV);  
	cvtColor(image, img_color, COLOR_GRAY2BGR); 
	//레이블링
	//////////////////////////////
	Mat img_labels, centroids;
	numOfLabels = connectedComponentsWithStats(img_binary, img_labels,   
		stats, centroids, 8,CV_32S);
	//레이블링
	//////////////////////////////
	for (int i = 1; i < numOfLabels; i++)
	{
		int area = stats.at<int>(i, CC_STAT_AREA);
		int left = stats.at<int>(i, CC_STAT_LEFT);
		int top = stats.at<int>(i, CC_STAT_TOP);
		int width = stats.at<int>(i, CC_STAT_WIDTH);
		int height = stats.at<int>(i, CC_STAT_HEIGHT);

		if (area > maxArea)
			maxArea = area;

		rectangle(img_color, Point(left, top), Point(left + width, top + height),Scalar(0, 0, 255), 1);
	}
	vector<int>params;
	params.push_back(IMWRITE_JPEG_QUALITY);
	params.push_back(100);
	imwrite("labeled.jpg", img_color, params);
	cout<<"Labeling Complete"<<endl;
}

//Roi 배열 씀
extern "C" __declspec(dllexport)void __stdcall OptimizeLabels()
{
	cout<<"Optimizing..."<<endl;

	Mat img_color;
	int maxleft = 0;
	int rememberP = 0;
	int numOfLabels2 = numOfLabels;
	
	cvtColor(image, img_color, COLOR_GRAY2BGR); 

	for(int i = 1 ; i < numOfLabels2; i++)
	{
		int area = stats.at<int>(i, CC_STAT_AREA);
		if(area < maxArea / 4)
		{
			numOfLabels--;
		}
	}
	Roi = new Mat[numOfLabels-1];

	for (int j = 0; j<numOfLabels-1; j++)
	{
		for (int p = 1; p<numOfLabels2; p++)
		{
			int area = stats.at<int>(p, CC_STAT_AREA);
			//최적화
			if(area > maxArea/4)
			{
				int left = stats.at<int>(p, CC_STAT_LEFT);
				int top = stats.at<int>(p, CC_STAT_TOP);
				int width = stats.at<int>(p, CC_STAT_WIDTH);
				int height = stats.at<int>(p, CC_STAT_HEIGHT);
				//왼쪽으로 정렬
				if (left > maxleft)
				{
					maxleft = left;
					rememberP = p;
					Rect ROI(left, top, width, height);
					Roi[numOfLabels - j - 2] = img_color(ROI);
				}
			}
		}
		stats.at<int>(rememberP, CC_STAT_AREA) = NULL;
		stats.at<int>(rememberP, CC_STAT_LEFT) = NULL;
		stats.at<int>(rememberP, CC_STAT_TOP) = NULL;
		stats.at<int>(rememberP, CC_STAT_WIDTH) = NULL;
		stats.at<int>(rememberP, CC_STAT_HEIGHT) = NULL;

		maxleft = 0;
	}
	cout<<"Optimizing Complete"<<endl;
}

//Result 배열씀
extern "C" __declspec(dllexport)char* __stdcall FindLetters()
{
	cout<<"Finding Letters..."<<endl;
	cout<<"numOfLabels: "<<numOfLabels<<endl;
	char* Result = new char[numOfLabels-1];

	for(int j = 0 ; j < numOfLabels-1; j++)
	{
		char Alphabet = 'a';
		char result = 0;
		int maxMatch = 0;
		int temp = 0;
		kazeF = KAZE::create();
		kazeF->detectAndCompute(Roi[j], noArray(), keypoints2, descriptors2);

		for (int i = 0; i <= 25; i++, Alphabet++)
		{
			kazeF->detectAndCompute(image_ABCD[i], noArray(), keypoints1, descriptors1);

			int k = 2;
			matcher = DescriptorMatcher::create("BruteForce");

			void *MatchArr = CoTaskMemAlloc(sizeof(vector< vector< DMatch > >));
			vector< vector< DMatch > > *matches = new (MatchArr) vector< vector< DMatch > >;

			matcher->knnMatch(descriptors1, descriptors2, *matches , k);

			float nndrRatio = 0.6f;
			for (int t = 0; t < matches->size(); t++)
			{
				if (matches->at(t).size() == 2 
					&& matches->at(t).at(0).distance <= nndrRatio * matches->at(t).at(1).distance)
				{
					goodMatches.push_back(matches->at(t)[0]);
				}
			}

			if(goodMatches.size() > 40)
			{
				temp = maxMatch;
				maxMatch = goodMatches.size();
				Result[j] = Alphabet;
				keypoints1.clear();
				keypoints2.clear();
				goodMatches.clear();
				CoTaskMemFree(MatchArr);
				break;
			}

			if(maxMatch < goodMatches.size())
			{
				temp = maxMatch;
				maxMatch = goodMatches.size();
				Result[j] = Alphabet;
			}

			keypoints1.clear();
			keypoints2.clear();
			goodMatches.clear();
			CoTaskMemFree(MatchArr);			
		}
		if(Result[j] < 61 || Result[j] > 121)
			Result[j] = '?';
	}
	cout<<"Finding Letters Complete"<<endl;
	return Result;
	delete[] Result;
}

extern "C" __declspec(dllexport)void __stdcall FreeVariables()
{
	delete[] image_ABCD;
	delete[] Roi;
}