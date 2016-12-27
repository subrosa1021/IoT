#include "opencv2/opencv.hpp"
#include "opencv/cxcore.h"
#include <stdio.h>
#include <iostream>


#define NUM 3
#define IMAGE_CNT 2
#define MAXLEVEL 256


int main(){
	char *winName[NUM] = {"* OriginImage *", "ROI", "cvSubs"};

	IplImage *img[NUM];
	IplImage *image;
	CvCapture *capture = cvCaptureFromCAM(0);
	cvGrabFrame(capture);
	image = cvRetrieveFrame(capture);
	cvSaveImage("image.jpg",image);
	IplImage *src = cvLoadImage("image.jpg");

	img[0] = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, img[0], CV_BGR2GRAY);
	img[1] = (IplImage*)cvClone(img[0]);

	cvSetImageROI(img[1], cvRect(200, 60, 160, 180));
	cvSetImageROI(src, cvRect(200, 60, 160, 180));
	img[2] = (IplImage*)cvClone(img[1]);
	cvSubS(img[1], cvScalar(100,100,100), img[2]);

	cvResetImageROI(img[2]);
	cvSaveImage("emptyImg.jpg", src);



	for(int i=0 ; i<NUM ; i++){
		cvNamedWindow(winName[i]);
		cvShowImage(winName[i], img[i]);
	}

	cvWaitKey(0);

	for(int i=0 ; i<NUM ; i++){
	//	cvDestoryWindow(winName[i]);
		cvReleaseImage(&img[i]);
	}
	return 0;
}
