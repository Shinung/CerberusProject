// CamFuncsDll.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

//#include "stdafx.h"

#pragma comment(lib, "vfw32.lib") 
#pragma comment( lib, "comctl32.lib" )

#include "opencv.hpp"
#include <iostream>
#include <list>
#include <cstdio>
#include <time.h>
#include <string.h>

#define max_size 10
#define _DEBUG_

using namespace cv;
using namespace std;

//String face_cascade = "C:/Program Files (x86)/opencv310/sources/data/haarcascades/haarcascade_frontalface_alt.xml";
//String eye_cascade = "C:/Program Files (x86)/opencv310/sources/data/haarcascades/haarcascade_eye.xml";
String face_cascade = "haarcascade_frontalface_alt.xml";
//String eye_cascade = "haarcascade_eye.xml";

CascadeClassifier face;
CascadeClassifier eye;

static int bStart=0;//0 false, 1true
static int bStart2=0;
static bool bChk = false;
void clearb(char *buf)
{
	for(int i =0; i<sizeof(buf); i++)
	{
		buf[i] = 0;
	}
}

Mat MaskFace(Mat img, int type)
{
	//1. 가면, 2. 탈
	Mat gray;
	cvtColor(img,gray,CV_BGR2GRAY); // 컬러 -> 그레이 영상 변환
	vector<Rect> face_pos; // 얼굴 검출 결과를 표시하는 직사각형

	int lsz = 0, rsz = 0, tsz=0, bsz=0;
	cout << "VideoCam시작1" << endl;
	face.detectMultiScale(gray,face_pos,1.1,2,0|CV_HAAR_SCALE_IMAGE,Size(10,10));
	Mat face1, face2, tmpface1, tmpface2, img2;
	Mat acc;
	Rect rc1, rc2;
	Point sPoint;
	int cnt =0;

	Rect fPos[max_size], fPos1[max_size];
	Mat cut_pos[max_size],cut_pos1[max_size];
	Mat mask, fCut;
	Point center[max_size], center0[max_size];
	if(type==1)
	{
		//마스크
		Mat stck = imread("Resource/ironman.png");//400X400
		cout << "VideoCam시작2" << endl;
		img2 =  stck.clone();
		mask = Mat(img2.size(), img2.type(), Scalar::all(0));
		ellipse(mask,Point(200,200),Point(200,200),0,0,360,Scalar::all(255),-1);
		acc = Mat(img2 & mask);
		cout << "VideoCam시작3" << endl;
		for(int i=0; i<face_pos.size(); i++){
			if(i>max_size)
				break;
			fPos[i] = Rect(face_pos[i].x+lsz, face_pos[i].y+tsz,
				face_pos[i].width-lsz-rsz, face_pos[i].height-bsz-tsz);
			Point pt(fPos[i].x+fPos[i].width/2,fPos[i].y+fPos[i].height/2);//시작점
			Point pt2(fPos[i].width/2,fPos[i].height/2);//반지름길이

			center0[i] = pt;
			center[i] = pt2;	
			ellipse(img,center0[i],center[i],0,0,360,Scalar::all(0),-1);	
			resize(acc,acc,fPos[i].size(),0,0,INTER_LINEAR);
			fCut = img(fPos[i]) + acc;
			fCut.copyTo(img(fPos[i]));
			cnt++;

		}
		cout << "VideoCam시작4" << endl;
	}else if(type==2){
		Mat stck = imread("Resource/cawface.png");
		img2 =  stck.clone();
		mask = Mat(img2.size(), img2.type(), Scalar::all(0));
		ellipse(mask,Point(200,200),Point(200,200),0,0,360,Scalar::all(255),-1);
		acc = Mat(img2 & mask);
		for(int i=0; i<face_pos.size(); i++){
			if(i>max_size)
				break;
			fPos[i] = Rect(face_pos[i].x+lsz, face_pos[i].y+tsz,
				face_pos[i].width-lsz-rsz, face_pos[i].height-bsz-tsz);
			Point pt(fPos[i].x+fPos[i].width/2,fPos[i].y+fPos[i].height/2);//시작점
			Point pt2(fPos[i].width/2,fPos[i].height/2);//반지름길이

			center0[i] = pt;
			center[i] = pt2;	
			ellipse(img,center0[i],center[i],0,0,360,Scalar::all(0),-1);	
			resize(acc,acc,fPos[i].size(),0,0,INTER_LINEAR);
			fCut = img(fPos[i]) + acc;
			fCut.copyTo(img(fPos[i]));
			cnt++;

		}
	}

	return img;
}
Mat ChangeFace(Mat img)
{
	Mat gray;
	cvtColor(img,gray,CV_BGR2GRAY);
	vector<Rect> face_pos;

	int lsz = 0, rsz = 0, tsz=0, bsz=0;

	face.detectMultiScale(gray,face_pos,1.1,2,0|CV_HAAR_SCALE_IMAGE,Size(10,10));

	Mat face1, face2, tmpface1, tmpface2, img2;
	Rect rc1, rc2;
	Point sPoint;
	int cnt =0;

	Rect fPos[max_size], fPos1[max_size];
	Mat cut_pos[max_size],cut_pos1[max_size];
	Mat mask, fCut;
	Point center[max_size], center0[max_size];

	for(int i=0; i<face_pos.size(); i++){
		if(i>max_size)
			break;
		fPos[i] = Rect(face_pos[i].x+lsz, face_pos[i].y+tsz,
			face_pos[i].width-lsz-rsz, face_pos[i].height-bsz-tsz);
		Point pt(fPos[i].x+fPos[i].width/2,fPos[i].y+fPos[i].height/2);
		Point pt2(fPos[i].width/2,fPos[i].height/2);
		Point pt3(fPos[i].width/2,fPos[i].height/2);		
		fPos1[i] = Rect(pt,pt3);
		center0[i] = pt;
		center[i] = pt3;
		img2 = Mat(img,fPos[i]).clone();
		mask = Mat(img2.size(), img2.type(), Scalar::all(0));	
		ellipse(mask,pt2,pt3,0,0,360,Scalar::all(255),-1);
		fCut = Mat(img2 & mask);
		cut_pos[i] = fCut;
		cnt++;
	}

	Mat chg_pos[max_size];
	Mat ctmp, cnxt;
	if(cnt >= 2){
		Rect tmp, nxt;
		Mat mask;
		ctmp = cut_pos[cnt-1].clone();
		for(int i=0; i<cnt; i++)
		{			
			if(i==cnt-1)
			{	
				chg_pos[cnt-1] = cnxt.clone();
			}else if(i==0)
			{				
				cnxt = cut_pos[0].clone();
				chg_pos[i] = cut_pos[i+1].clone();												
			}else if(i>0 && i<cnt-1){
				chg_pos[i] = cut_pos[i+1].clone();				
			}
			ellipse(img,center0[i],center[i],0,0,360,Scalar::all(0),-1);								
			resize(chg_pos[i],chg_pos[i],fPos[i].size(),0,0,INTER_LINEAR);
			chg_pos[i] = img(fPos[i]) + chg_pos[i];			
			chg_pos[i].copyTo(img(fPos[i]));
		}
	}
	return img;
}
void SetV(int v){
	bStart = v;
}
void SetV2(int v){
	bStart2 = v;
}
extern "C" __declspec(dllexport)int __stdcall RecordF(int type)
{ //ptr true면 계속실행 
	//bStart=type;
	SetV(type);
	return type;
}
extern "C" __declspec(dllexport)int __stdcall Pics(int type)
{ //ptr true면 계속실행 
	SetV2(type);
	return type;
}
extern "C" __declspec(dllexport)int __stdcall VideoCam(int type)
{	
	bool pst = false, ipst = false;
	Mat img;
	VideoCapture capture;
	for (int i = 0; i < 10; i++)
	{
		capture.open(i);
		if (capture.isOpened())
			break;
	}
	face.load(face_cascade);
	if(!capture.isOpened())
	{
		cout << "Can not open Capture!" << endl;
		return -1;
	}
	Size size = Size((int) capture.get(CAP_PROP_FRAME_WIDTH),
		(int)capture.get(CAP_PROP_FRAME_HEIGHT));

	int fourcc = VideoWriter::fourcc('M','J','P','G');
	double fps = 24;// capture.get(CAP_PROP_FPS);//24;
	struct tm *st_time;
	char buf[100];	
	char * ptr = buf;
	char* str = "";
	time_t now = time(0);
	st_time = localtime(&now);
	int fdely = 1000/fps;
	Mat frame, grayImage, fImage;
	int cnt=0;
	int delay = 1;
	VideoWriter outputv;		
	int icnt =0;
	cout << "CamGo - " << type << endl;

	if(type!=-1){
		namedWindow("fImage", WINDOW_AUTOSIZE);
		//namedWindow("fImage", WINDOW_AUTOSIZE);
		resizeWindow("fImage",0,0);
	}
	for(;;)
	{
		capture >> frame;			
		if(frame.empty())
			return -1;
		int ckey1 = waitKey(delay);
		if(type==1)
		{
			str = "Iron";
			fImage = MaskFace(frame, 1);//마스크		
			cout << "마스크" << endl;
		}else if(type==2)
		{
			str = "Caw";
			fImage = MaskFace(frame, 2); //캐릭터
		}else if(type==3)
		{	
			str = "Switch";	
			cout << "교환" << endl;
			fImage = ChangeFace(frame);
		}
		else if(type==-1){
			destroyWindow("fImage");
			fImage.release();
			capture.release();
			if(bStart==1 && bChk==true)
				outputv.release();
			return -1;
		}else{
			str = "Basic";				
			fImage = frame.clone();
			cout << "기본" << endl;
		}
		
		clearb(buf);			
		//sprintf(buf, "../../Resource/snap.jpg");
		sprintf(buf, "Resource/snap.jpg");
		resize(fImage,fImage,size);
		imwrite(buf,fImage);

		if(ckey1 == 27)//esc
		{
			return -1;
		}
		else if(ckey1 == 32)//spacebar
		{	
			if(ipst==false)
			{
				ipst=true;
			}
		}
		else if(ckey1 == 13)//Enter			
		{
			cout <<" bStart "<< bStart <<" bChk "  << bChk << endl;
			if(pst==false || (bStart==0 && bChk==false))
			{
				bChk = false;
				bStart = 1;
				if(fImage.empty())
					break;			
				clearb(buf);
				sprintf(buf, "../../Picture/pic_%s%d.avi",str,cnt);			
				outputv = VideoWriter(buf,1,fps,size,true);
				resize(fImage,fImage,size);
				outputv << fImage;
				cnt++;
				pst = true;
			}else if(pst==true || (bStart==1 && bChk==true))
			{
				pst=false;
				bStart=0;
				bChk=false;
				cout<< "녹화끝";
				outputv.release();
			}
		}else if(ckey1==49)//1
		{
			destroyWindow("fImage");
			fImage.release();
			capture.release();
			if(pst==true)
				outputv.release();
			return 1;
		}else if (ckey1==50)//2
		{
			destroyWindow("fImage");
			fImage.release();
			capture.release();
			if(pst==true)
				outputv.release();
			return 2;
		}else if (ckey1==51)//3
		{
			destroyWindow("fImage");
			fImage.release();
			capture.release();
			if(pst==true)
				outputv.release();
			return 3;
		}

		if((bStart==1 && bChk==false))
			{
				bChk = true;
				if(fImage.empty())
					break;			
				clearb(buf);
				sprintf(buf, "../../Picture/pic_%s%d.avi",str,cnt);			
				outputv = VideoWriter(buf,1,fps,size,true);
				resize(fImage,fImage,size);
				outputv << fImage;
				cnt++;
				pst = true;
			}else if((bStart==1 && bChk==true))
			{
				pst=false;
				cout<< "녹화끝";
				outputv.release();
			}
		if(pst==true || (bStart==0 && bChk == true))
		{				
			bChk == false;
			if(fImage.empty())
				break;
			resize(fImage,fImage,size);
			outputv << fImage;						
		}

		if(ipst==true || bStart2==1)
		{
			clearb(buf);			
			sprintf(buf, "../../Picture/pic_%s_%d.jpg",str,icnt);	
			resize(fImage,fImage,size);
			imwrite(buf,fImage);
			ipst = false;
			bStart2 = 0;
			icnt++;
		}		
		//imshow("fImage", fImage);
	}
	return 0;
}
