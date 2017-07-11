#include <Windows.h>
#include <process.h>

#include "opencv.hpp"

//#define __DEBUG__

using namespace cv;
using namespace std;

Size inVideoSize;
int fps, delay;

Rect selection;
Rect trackWindow;

Mat hImage, mask, dstImage, handROIImage;
Mat trackingHist, backProject;

int histSize = 8;
float valueRange[] = { 0, 180 };
const float* ranges[] = { valueRange };
int channels = 0;

bool flagInit = false;
VideoCapture inputVideo;

void(*CamViewCallBack)(char*);

HANDLE hThrShowInitCam;
bool flagThrShowInitCam = false;


extern "C" __declspec(dllexport) int __stdcall InitHandDetection(int *out_delay,
	int *out_width, int *out_height, void(*callback)(char*))
{
	inputVideo.open(0);
	if (!inputVideo.isOpened())
		return -1;

	inVideoSize = Size((int)inputVideo.get(CAP_PROP_FRAME_WIDTH),
		(int)inputVideo.get(CAP_PROP_FRAME_HEIGHT));

	fps = (int)inputVideo.get(CAP_PROP_FPS);
	if (fps <= 0) fps = 24;

	delay = 1000 / fps;
	*out_delay = delay;

	*out_width = inVideoSize.width; *out_height = inVideoSize.height;

	selection = Rect(inVideoSize.width / 2 - 50, 100, 200, 300);

	CamViewCallBack = callback;
	flagInit = true;

#ifdef __DEBUG__
	namedWindow("InitCam");
	namedWindow("handROIImage");
	namedWindow("handInRange");
	namedWindow("mask");
	namedWindow("hImage");
	namedWindow("backProject");
#endif // __DEBUG__


	return 1;
}

extern "C" __declspec(dllexport) void __stdcall ReleaseHandDetection()
{
	inputVideo.release();
	flagInit = false;

#ifdef __DEBUG__
	destroyWindow("InitCam");
	destroyWindow("handROIImage");
	destroyWindow("handInRange");
	destroyWindow("mask");
	destroyWindow("hImage");
	destroyWindow("backProject");
#endif // __DEBUG__
}

void GetHchAndMask(const Mat& frame)
{
	Mat hsvImage;
	cvtColor(frame, hsvImage, CV_BGR2HSV);

	inRange(hsvImage, Scalar(0, 60, 0),
		Scalar(30, 255, 255), mask);

	int ch[] = { 0, 0 };
	hImage.create(hsvImage.size(), CV_8U);
	mixChannels(&hsvImage, 1, &hImage, 1, ch, 1);

#ifdef __DEBUG__
	imshow("mask", mask);
	imshow("hImage", hImage);
#endif // __DEBUG__
}

extern "C" __declspec(dllexport) int __stdcall TrackingINIT()
{
	if (!flagInit)
		return -2;
	Mat frame;

	inputVideo >> frame;
	if (frame.empty())
		return -1;

	GetHchAndMask(frame);

	Mat hImageROI(hImage, selection), maskROI(mask, selection);
	calcHist(&hImageROI, 1, &channels, maskROI, trackingHist, 1, &histSize, ranges);
	normalize(trackingHist, trackingHist, 0, 255, CV_MINMAX);
	trackWindow = selection;

	return 1;
}

int GetHandContour(const Mat &bProject, vector<Point> &hContour)
{
	vector<vector<Point>> contours;
	findContours(bProject, contours, noArray(),
		RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	if (contours.size() < 1)
		return 0;

	int maxK = 0;
	double maxArea = contourArea(contours[0]);
	for (int k = 1; k < contours.size(); k++)
	{
		double area = contourArea(contours[k]);
		if (area > maxArea)
		{
			maxK = k;
			maxArea = area;
		}
	}
	hContour = contours[maxK];

	return 1;
}

int GetHandROIImage(const Mat &curFrame, const RotatedRect &trBox, Mat &out)
{
	Mat retImg(inVideoSize, CV_8UC3);
	Rect br = trBox.boundingRect();
	Mat copyBP;

	backProject.copyTo(copyBP);

	retImg = Scalar::all(0);

	vector<Point> handContour;
	GetHandContour(copyBP, handContour);
	if (handContour.size() < 1)
		return 0;

	Point corner, cornerBelow;
	double fMaxDist = 0, fMaxDistBelow = 0;
	for (int i = 0; i < handContour.size(); i++)
	{
		Point pt = handContour[i];
		double fDist = sqrt((double)((pt.x - trBox.center.x) * (pt.x - trBox.center.x) +
			(pt.y - trBox.center.y) * (pt.y - trBox.center.y)));
		if (pt.y < trBox.center.y && fDist > fMaxDist)
		{
			fMaxDist = fDist;
			corner = pt;
		}
		if (pt.y > trBox.center.y && fDist > fMaxDistBelow)
		{
			fMaxDistBelow = fDist;
			cornerBelow = pt;
		}
	}

	br.x = trBox.center.x - trBox.size.width;
	if (corner.y < trBox.boundingRect().y)
		br.y = corner.y;
	else
		br.y = trBox.boundingRect().y;
	br.width = trBox.size.width * 2;
	br.height = cornerBelow.y - br.y;

	if (br.x < 0) br.x = 0;
	if (br.y < 0) br.y = 0;
	if (br.x + br.width > inVideoSize.width) br.width = inVideoSize.width - br.x;
	if (br.y + br.height > inVideoSize.height) br.height = inVideoSize.height - br.y;

	rectangle(dstImage, br, Scalar(0, 255, 255), 2);

	Mat bpMask;

	bpMask = Mat(backProject.size(), CV_8UC3);
	bpMask = Scalar::all(0);
	//circle(handMask, trackBox.center, trackBox.size.width, Scalar(255, 255, 255), -1);
	rectangle(bpMask, br, Scalar(255, 255, 255), -1);

	Mat andCalcImg, convertedMask;
	bitwise_and(curFrame, bpMask, andCalcImg);
	mask.copyTo(convertedMask);
	cvtColor(convertedMask, convertedMask, CV_GRAY2BGR);
	bitwise_and(andCalcImg, convertedMask, andCalcImg);

	cvtColor(andCalcImg, out, CV_BGR2HSV);

	circle(dstImage, corner, 5, Scalar(255, 0, 255), 2);
	circle(dstImage, cornerBelow, 5, Scalar(180, 0, 255), 2);
	line(dstImage, trBox.center, corner, Scalar(0, 255, 255), 2);

	return 1;
}

extern "C" __declspec(dllexport) int __stdcall RunHandDetection()
{
	if (!flagInit)
		return -2;
	Mat frame;

	inputVideo >> frame;
	if (frame.empty())
		return -1;

	RotatedRect trackBox;

	TermCriteria criteria = TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 10, 2);

	frame.copyTo(dstImage);

	GetHchAndMask(dstImage);
	calcBackProject(&hImage, 1, &channels, trackingHist, backProject, ranges);
	backProject &= mask;


	erode(backProject, backProject, Mat());
	erode(backProject, backProject, Mat());
	dilate(backProject, backProject, Mat());
	dilate(backProject, backProject, Mat());

#ifdef __DEBUG__
	imshow("backProject", backProject);
#endif // __DEBUG__

	trackBox = CamShift(backProject, trackWindow, criteria);

	if (!GetHandROIImage(dstImage, trackBox, handROIImage))
	{
		vector<int> params;
		params.push_back(IMWRITE_JPEG_QUALITY);
		params.push_back(90);

		imwrite("CamFrame.jpg", dstImage, params);
		CamViewCallBack("CamFrame.jpg");
		return 1;
	}

#ifdef __DEBUG__
	imshow("handROIImage", handROIImage);
#endif // __DEBUG__

	Mat handInRange;
	inRange(handROIImage, Scalar(0, 59, 0), Scalar(20, 255, 255), handInRange);

#ifdef __DEBUG__
	imshow("handInRange", handInRange);
#endif // __DEBUG__

	vector<Point> handContour;
	if (!GetHandContour(handInRange, handContour))
	{
		vector<int> params;
		params.push_back(IMWRITE_JPEG_QUALITY);
		params.push_back(90);

		imwrite("CamFrame.jpg", dstImage, params);
		CamViewCallBack("CamFrame.jpg");
		return 1;
	}

	vector<int> hull;
	convexHull(handContour, hull);

	vector<Point> ptsHull;
	for (int k = 0; k < hull.size(); k++)
	{
		int i = hull[k];
		ptsHull.push_back(handContour[i]);
	}
	drawContours(dstImage, vector<vector<Point>>(1, ptsHull), 0,
		Scalar(255, 0, 0), 2);

	vector<Vec4i> defects;
	convexityDefects(handContour, hull, defects);
	for (int k = 1; k < defects.size(); k++)
	{
		Vec4i v = defects[k];
		Point ptStart = handContour[v[0]];
		Point ptEnd = handContour[v[1]];
		Point ptFar = handContour[v[2]];
		float depth = v[3] / 256.0;
		if (depth > 20)
		{
			//line(dstImage, ptStart, ptFar, Scalar(0, 255, 0), 2);
			//line(dstImage, ptEnd, ptFar, Scalar(0, 255, 0), 2);
			//circle(dstImage, ptStart, 6, Scalar(0, 0, 255), 2);
			circle(dstImage, ptEnd, 6, Scalar(0, 0, 255), 2);
			circle(dstImage, ptFar, 6, Scalar(0, 255, 0), 2);
		}
	}

	vector<int> params;
	params.push_back(IMWRITE_JPEG_QUALITY);
	params.push_back(90);

	imwrite("CamFrame.jpg", dstImage, params);
	CamViewCallBack("CamFrame.jpg");

	return 1;
}

extern "C" __declspec(dllexport) int __stdcall ShowInitCam()
{
	if (!flagInit)
		return -2;

	Mat frame;

	inputVideo >> frame;
	if (frame.empty())
		return -1;

	rectangle(frame, selection, Scalar(0, 0, 255), 2);

#ifdef __DEBUG__
	imshow("InitCam", frame);
#endif

	vector<int> params;
	params.push_back(IMWRITE_JPEG_QUALITY);
	params.push_back(90);

	imwrite("CamFrame.jpg", frame, params);
	CamViewCallBack("CamFrame.jpg");

	return 1;
}