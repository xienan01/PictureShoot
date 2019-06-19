#include "pictureShot.h"
#include "iostream"
#include "stdio.h"
#include "stdlib.h"


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"



void main()
{
cv::Mat Img;
 srcMat = cv::imread("1.jpg");
 srcMat.copyTo(img);
 srcMat.copyTo(tempMatMat);
 cv::namedWindow("img");//定义一个img窗口 
 cv::setMouseCallback("img", on_mouse, 0);//调用回调函数 
 cv::imshow("img", img);
 cv::waitKey(0);
}

	return  0;
}



