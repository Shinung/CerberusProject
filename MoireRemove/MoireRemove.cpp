#pragma comment(lib, "vfw32.lib") 
#pragma comment(lib, "comctl32.lib")

#include "opencv.hpp"

//#include "opencv\cv.h"
//#include "opencv\highgui.h"

using namespace std;
using namespace cv;

// Mat 원본 이미지
Mat srcImage;
// Mat 변환 후 이미지
Mat dstImage;

void ChangeXY(Mat &m)
{
	float fValue;

	for (int y = 0; y<m.rows; y++)
		for (int x = 0; x<m.cols; x++)
		{
			fValue = m.at<float>(y, x);
			//	if((x+y)%2==1) // odd number
			if ((x + y) % 2 == 1 && fValue != 0)
				m.at<float>(y, x) = -fValue;
		}
}

void Calc(Mat &input, Mat &output)
{
	ChangeXY(input);

	// 푸리에 변환
	Mat dftA;
	dft(input, dftA, DFT_COMPLEX_OUTPUT);

	// 관심영역 설정
	Rect rectRoi = Rect(input.cols / 4, input.rows / 4,
		input.cols / 2, input.rows / 2);

	// 백업 클론이미지 생성
	Mat backImage = dftA.clone();

	// 백업 관심영역 설정
	Mat backROI = backImage(rectRoi);

	// 전체 0
	dftA = Scalar::all(0);

	// desti 관심영역 설정
	Mat destiROI = dftA(rectRoi);
	// 백업해뒀던 backROI copy -> dsetiROI
	backROI.copyTo(destiROI);

	// 역푸리에
	dft(dftA, output, DFT_INVERSE | DFT_SCALE | DFT_REAL_OUTPUT);

	// 원점 분산
	ChangeXY(output);
}

extern "C" __declspec(dllexport) void __stdcall InitMR()
{
	/*namedWindow("MoireRemove");
	namedWindow("Original");*/
	return;
}

extern "C" __declspec(dllexport) int __stdcall MoireStart(char* fileName)
{
	char str[30] = { 0, };

	// load
	srcImage = imread(fileName);
	if (srcImage.empty())
		return -1;

	// 자료형 변환
	srcImage.convertTo(dstImage, CV_32F);

	vector<Mat> channels(3);

	//vector<Mat> channelsFinal;
	split(dstImage, channels); //채널분리

							   //imshow("Red", channels[2]);
							   //imshow("Green", channels[1]);
							   //imshow("Blue", channels[0]);

	Mat blue, green, red;
	Calc(channels[0], blue);
	Calc(channels[1], green);
	Calc(channels[2], red);

	vector<Mat> channelsDft = { blue, green, red };

	merge(channelsDft, dstImage);

	//return dstImage;

	dstImage.convertTo(dstImage, CV_8U);

	// 최종결과
	imshow("MoireRemove", dstImage);
	// 원본영상
	imshow("Original", srcImage);

	//waitKey();

	return 1;
}

extern "C" __declspec(dllexport) void __stdcall Save(char* fileName)
{
	vector<int> params;
	params.push_back(IMWRITE_JPEG_QUALITY);
	params.push_back(100);
	imwrite(fileName, dstImage, params);
}

extern "C" __declspec(dllexport) void __stdcall CarNumberStart(char* fileName)
{
	Mat image, image2, image3, drawing;  //  Make images.
	Rect rect, temp_rect;  //  Make temporary rectangles.
	vector<vector<Point> > contours;  //  Vectors for 'findContours' function.
	vector<Vec4i> hierarchy;

	double ratio, delta_x, delta_y, gradient;  //  Variables for 'Snake' algorithm.
	int select, plate_width, count, friend_count = 0, refinery_count = 0;
	

	image = imread(fileName);  //  Load an image file.
	//srcImage = image;
	imshow("Original", image);
	//waitKey(1);
	

	image.copyTo(image2);  //  Copy to temporary images.
	image.copyTo(image3);  //  'image2' - to preprocessing, 'image3' - to 'Snake' algorithm.

	cvtColor(image2, image2, CV_BGR2GRAY);  //  Convert to gray image.
	//imshow("Original->Gray", image2);
	//waitKey(0);

	Canny(image2, image2, 100, 300, 3);  //  Getting edges by Canny algorithm.
	imshow("Canny", image2);
	//waitKey(1);


	//  Finding contours.
	findContours(image2, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point());
	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Rect> boundRect2(contours.size());

	//  Bind rectangle to every rectangle.
	for (int i = 0; i< contours.size(); i++) {
		approxPolyDP(Mat(contours[i]), contours_poly[i], 1, true);
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
	}

	drawing = Mat::zeros(image2.size(), CV_8UC3);

	for (int i = 0; i< contours.size(); i++) {

		ratio = (double)boundRect[i].height / boundRect[i].width;

		//  Filtering rectangles height/width ratio, and size.
		if ((ratio <= 2.5) && (ratio >= 0.5) && (boundRect[i].area() <= 700) && (boundRect[i].area() >= 100)) {

			drawContours(drawing, contours, i, Scalar(0, 255, 255), 1, 8, hierarchy, 0, Point());
			rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 1, 8, 0);

			//  Include only suitable rectangles.
			boundRect2[refinery_count] = boundRect[i];
			refinery_count += 1;
		}
	}

	boundRect2.resize(refinery_count);  //  Resize refinery rectangle array.

	imshow("Contours&Rectangles", drawing);
	//waitKey(0);



	//  Bubble Sort accordance with X-coordinate.
	for (int i = 0; i<boundRect2.size(); i++) {
		for (int j = 0; j<(boundRect2.size() - i - 1); j++) {
			if (boundRect2[j].tl().x > boundRect2[j + 1].tl().x) {
				temp_rect = boundRect2[j];
				boundRect2[j] = boundRect2[j + 1];
				boundRect2[j + 1] = temp_rect;
			}
		}
	}


	for (int i = 0; i< boundRect2.size(); i++) {

		rectangle(image3, boundRect2[i].tl(), boundRect2[i].br(), Scalar(0, 255, 0), 1, 8, 0);

		count = 0;

		//  Snake moves to right, for eating his freind.
		for (int j = i + 1; j<boundRect2.size(); j++) {

			delta_x = abs(boundRect2[j].tl().x - boundRect2[i].tl().x);

			if (delta_x > 150)  //  Can't eat snake friend too far ^-^.
				break;

			delta_y = abs(boundRect2[j].tl().y - boundRect2[i].tl().y);


			//  If delta length is 0, it causes a divide-by-zero error.
			if (delta_x == 0) {
				delta_x = 1;
			}

			if (delta_y == 0) {
				delta_y = 1;
			}


			gradient = delta_y / delta_x;  //  Get gradient.
			//cout << gradient << endl;

			if (gradient < 0.25) {  //  Can eat friends only on straight line.
				count += 1;
			}
		}

		//  Find the most full snake.
		if (count > friend_count) {
			select = i;  //  Save most full snake number.
			friend_count = count;  //  Renewal number of friends hunting.
			rectangle(image3, boundRect2[select].tl(), boundRect2[select].br(), Scalar(255, 0, 0), 1, 8, 0);
			plate_width = delta_x;  //  Save the last friend ate position.
		}                           //  It's similar to license plate width, Right?
	}

	//  Drawing most full snake friend on the image.
	rectangle(image3, boundRect2[select].tl(), boundRect2[select].br(), Scalar(0, 0, 255), 2, 8, 0);
	line(image3, boundRect2[select].tl(), Point(boundRect2[select].tl().x + plate_width, boundRect2[select].tl().y), Scalar(0, 0, 255), 1, 8, 0);

	//imshow("Rectangles on original", image3);
	//waitKey(0);



	//  Shows license plate, and save image file.
	Mat carNumber = image(Rect(boundRect2[select].tl().x - 20, boundRect2[select].tl().y - 20, plate_width + 40, plate_width*0.3));
	imshow("Result", carNumber);
	dstImage = carNumber;
	//waitKey(0);

	//imwrite("Plates/1-1.JPG", image(Rect(boundRect2[select].tl().x - 20, boundRect2[select].tl().y - 20, plate_width + 40, plate_width*0.3)));
}



//__cdecl 방법

//extern "C"
//{
//	__declspec(dllexport) void Start(char* fileName, char* windowName);
//}
//extern void __cdecl Start(char* fileName, char* windowName)
//{
//	char str[30] = { 0, };
//	int x;
//	int y;
//	float fValue;
//
//	// load
//	srcImage = imread(fileName, IMREAD_GRAYSCALE);
//	if (srcImage.empty())
//		return;
//
//	// 자료형 변환
//	srcImage.convertTo(dstImage, CV_32F);
//
//	// 원점이동
//	for (y = 0; y<dstImage.rows; y++)
//		for (x = 0; x<dstImage.cols; x++)
//		{
//			fValue = dstImage.at<float>(y, x);
//			//	if((x+y)%2==1) // odd number
//			if ((x + y) % 2 == 1 && fValue != 0)
//				dstImage.at<float>(y, x) = -fValue;
//		}
//
//	// 푸리에 변환
//	Mat dftA;
//	dft(dstImage, dftA, DFT_COMPLEX_OUTPUT);
//
//	// 스펙트럼 이미지용 생성
//	//dstImage = Scalar::all(0);
//	Mat specImage = dftA.clone();
//
//	// 관심영역 설정
//	Rect rectRoi = Rect(srcImage.cols / 4, srcImage.rows / 4,
//		srcImage.cols / 2, srcImage.rows / 2);
//
//	// 백업 클론이미지 생성
//	Mat backImage = dftA.clone();
//
//	// 백업 관심영역 설정
//	Mat backROI = backImage(rectRoi);
//
//	// split용 이미지
//	Mat splitImage[2];
//	split(specImage, splitImage);		// 실수부 허수부 나누기
//
//	// magnitude용 이미지
//	Mat magF;
//	magnitude(splitImage[0], splitImage[1], magF);
//
//	magF += Scalar(1);
//	log(magF, magF);
//
//	// nomalize용 이미지
//	Mat norImage;
//	normalize(magF, norImage, 0, 255, NORM_MINMAX, CV_8U);
//
//	// 스펙트럼 이미지 show
//	imshow("spectrumImage", norImage);
//
//	// 스펙트럼의 정보에 따른 결과물의 이진화 과정
//	/*for (int y = 0; y < norImage.rows; y++)
//	{
//	for (int x = 0; x < norImage.cols; x++)
//	{
//	if (norImage.at<float>(y, x) >= 100 || norImage.at<float>(y, x) < 0)
//	{
//	dftA.at<float>(y, x) = 0;
//	dftA.at<float>(y, x+ norImage.cols) = 0;
//	}
//	}
//	}*/
//
//	// 전체 0
//	dftA = Scalar::all(0);
//
//	// desti 관심영역 설정
//	Mat destiROI = dftA(rectRoi);
//	// 백업해뒀던 backROI copy -> dsetiROI
//	backROI.copyTo(destiROI);
//
//	split(dftA, splitImage);		// 실수부 허수부 나누기
//
//	magnitude(splitImage[0], splitImage[1], magF);
//
//	magF += Scalar(1);
//	log(magF, magF);
//
//	normalize(magF, dstImage, 0, 255, NORM_MINMAX, CV_8U);
//
//	// 모아레 제거 스펙트럼
//	imshow("moireRemoveImage", dstImage);
//
//	// 역푸리에
//	dft(dftA, dstImage, DFT_INVERSE | DFT_SCALE | DFT_REAL_OUTPUT);
//
//	// 원점 분산
//	for (y = 0; y<dstImage.rows; y++)
//		for (x = 0; x<dstImage.cols; x++)
//		{
//			fValue = dstImage.at<float>(y, x);
//			//	if((x+y)%2==1) // odd number
//			if ((x + y) % 2 == 1 && fValue != 0)
//				dstImage.at<float>(y, x) = -fValue;
//		}
//
//	dstImage.convertTo(dstImage, CV_8U);
//	// 최종결과
//	imshow(windowName, dstImage);
//	// 원본영상
//	imshow("srcImage", srcImage);
//
//	waitKey();
//}
