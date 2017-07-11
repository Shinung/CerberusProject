#pragma comment(lib, "vfw32.lib") 
#pragma comment( lib, "comctl32.lib" )

#include "opencv.hpp"

using namespace cv;
using namespace std;

#define __IMSHOW__
#define __DIST__
//#define __OUTPUTCONSOLE__

Size inVideoSize;
int fps, delay;

Mat frame, hsvImage, binaryImage;

Scalar maskLower(100, 100, 100), maskUpper(150, 255, 255); // blue - laptop
//Scalar maskLower(0, 100, 100), maskUpper(10, 255, 255);
//Scalar mask1Lower(160, 100, 100), mask1Upper(179, 255, 255);

bool flagInit = false;
VideoCapture inputVideo;

void(*CamViewCallBack)(char*);

extern "C" __declspec(dllexport) int __stdcall InitPointHandDetection(int *out_delay,
	int *out_width, int *out_height, void(*callback)(char*))
{
	bool flag = false;
	int camNum = -1;
	
	for (int i = 0; i < 9; i++)
	{
		inputVideo.open(i);
		if (inputVideo.isOpened())
		{
			camNum = i;
			flag = true;
			break;
		}
	}

	if (!flag)
		return -1;

	inVideoSize = Size((int)inputVideo.get(CAP_PROP_FRAME_WIDTH),
		(int)inputVideo.get(CAP_PROP_FRAME_HEIGHT));

	fps = (int)inputVideo.get(CAP_PROP_FPS);
	if (fps <= 0) fps = 24;

	delay = 1000 / fps;
	*out_delay = delay;

	*out_width = inVideoSize.width; *out_height = inVideoSize.height;

	CamViewCallBack = callback;
	flagInit = true;

#ifdef __IMSHOW__
	namedWindow("InitCam");
	namedWindow("hsvImage");
	namedWindow("binaryImage");
	namedWindow("MoireRemove");
	namedWindow("Original");
	namedWindow("Canny");
	namedWindow("Contours&Rectangles");
	namedWindow("Result");
#endif // __IMSHOW__

	return camNum;
}

extern "C" __declspec(dllexport) void __stdcall ReleasePointHandDetection()
{
	inputVideo.release();
	flagInit = false;

//#ifdef __IMSHOW__
//	destroyWindow("InitCam");
//	destroyWindow("hsvImage");
//	destroyWindow("binaryImage");
//#endif // __IMSHOW__
}

void getBinaryRedRange()
{
	inRange(hsvImage, maskLower, maskUpper, binaryImage);
	//Mat mask;
	//inRange(hsvImage, maskLower, maskUpper, mask);

	//Mat mask1;
	//inRange(hsvImage, mask1Lower, mask1Upper, mask1);

	//addWeighted(mask, 1.0, mask1, 1.0, 0.0, binaryImage);
	////binaryImage = mask + mask1;

	//erode(binaryImage, binaryImage, Mat());
	erode(binaryImage, binaryImage, Mat());
	dilate(binaryImage, binaryImage, Mat());
}

void findPointHand(vector<Point2f>& fCenttroid, vector<Rect>& fRect)
{
	Mat img_labels, stats, centroids;
	int cnt_labels;

	cnt_labels = connectedComponentsWithStats(binaryImage, img_labels, stats, centroids, 4);

	if (cnt_labels < 2)
		return;

	int maxi = -1, maxArea = 0;
	for (int i = 1; i < cnt_labels; i++)
	{
		int area = stats.at<int>(i, CC_STAT_AREA);
		if (maxArea < area)
		{
			maxArea = area;
			maxi = i;
		}
	}
	fCenttroid.push_back(Point2f(centroids.at<double>(maxi, 0), centroids.at<double>(maxi, 1)));
	fRect.push_back(Rect(stats.at<int>(maxi, CC_STAT_LEFT), stats.at<int>(maxi, CC_STAT_TOP),
		stats.at<int>(maxi, CC_STAT_WIDTH), stats.at<int>(maxi, CC_STAT_HEIGHT)));

	int maxj = -1;
	maxArea = 0;
	for (int j= 1; j < cnt_labels; j++)
	{
		int area = stats.at<int>(j, CC_STAT_AREA);
		if (maxi != j && maxArea < area)
		{
			maxArea = area;
			maxj = j;
		}
	}
	fCenttroid.push_back(Point2f(centroids.at<double>(maxj, 0), centroids.at<double>(maxj, 1)));
	fRect.push_back(Rect(stats.at<int>(maxj, CC_STAT_LEFT), stats.at<int>(maxj, CC_STAT_TOP),
		stats.at<int>(maxj, CC_STAT_WIDTH), stats.at<int>(maxj, CC_STAT_HEIGHT)));

#ifdef __OUTPUTCONSOLE__
	cout << "labels: " << cnt_labels << endl;
	for (int i = 0; i < 2; i++)
	{
		cout << "centroid: " << fCenttroid[i] << endl;
		cout << "rect: " << fRect[i] << endl;
	}
#endif // __IMSHOW__

}

void writeImg()
{
	vector<int> params;
	params.push_back(IMWRITE_JPEG_QUALITY);
	params.push_back(90);

	imwrite("CamFrame.jpg", frame, params);
}

extern "C" __declspec(dllexport) int __stdcall ShowWebCam(double *x, double *y)
{
	if (!flagInit)
		return -2;

	vector<Point2f> foundedCentroid;
	vector<Rect> foundedRect;

	inputVideo >> frame;

	if (frame.empty())
		return -1;

	cvtColor(frame, hsvImage, CV_BGR2HSV);

	getBinaryRedRange();
	findPointHand(foundedCentroid, foundedRect);

	if (foundedCentroid.size() == 2)
	{
		double dist = norm(foundedCentroid[0] - foundedCentroid[1]);

		if (dist < 35)
		{
			*x = foundedCentroid[0].x;
			*y = foundedCentroid[0].y;
		}

#ifdef __DIST__
		string outDist = "Distance: ";
		char s1[20];
		sprintf_s(s1, sizeof(s1), "%lf", dist);
		outDist.append(s1);

		putText(frame, outDist, Point(400, 400), FONT_HERSHEY_COMPLEX_SMALL, 1.0, Scalar(0, 0, 255));


		for (int i = 0; i < 2; i++)
		{
			circle(frame, foundedCentroid[i], 5, Scalar(255, 0, 0));
			rectangle(frame, foundedRect[i], Scalar(0, 255, 0));
		}
#endif // __DIST__
	}

	writeImg();
	CamViewCallBack("CamFrame.jpg");

#ifdef __IMSHOW__
	imshow("InitCam", frame);
	imshow("hsvImage", hsvImage);
	imshow("binaryImage", binaryImage);
#endif // __IMSHOW__


	return 1;
}