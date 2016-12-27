#include "opencv2/opencv.hpp"
#include "opencv/cxcore.h"
#include "opencv/highgui.h"

int main(){
	IplImage *dstImage = cvLoadImage("OriginImg.jpg");

	IplImage *GrayImage = cvCreateImage(cvSize(dstImage->width, dstImage->height), IPL_DEPTH_8U, 1);
	IplImage *resizeImage = cvCreateImage(cvSize(dstImage->width*2, dstImage->height*1.75), IPL_DEPTH_8U, 1);

	cvConvertImage(dstImage, GrayImage, CV_BGR2GRAY);
	cvResize(GrayImage, resizeImage,1);
	// text put
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 10.0, 10.0);
	cvPutText(resizeImage, "Test text", cvPoint(100, 100), &font, CV_RGB(0, 0, 0));

	for(int i=0 ; i<resizeImage->width ; i+=100)
		cvLine(resizeImage, cvPoint(i, resizeImage->height - resizeImage->height), cvPoint(i, resizeImage->height), CV_RGB(255,255,255));

	CvSize text_size;
	int baseline;
	cvGetTextSize("OpenCV Programming", &font, &text_size, &baseline);

	cvNamedWindow("Drawing Graphics", 1);
	cvSetWindowProperty("Drawing Graphics", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	cvShowImage("Drawing Graphics", resizeImage);
	cvWaitKey(0);

	cvDestroyAllWindows();
	cvReleaseImage(&dstImage);

	return 0;
}
