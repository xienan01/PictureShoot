/**
* Descrition: picture shot
* Date : 20190509
* author:
*
**/

#ifndef PICTURE_SHOT_H
#define PICTURE_SHOT_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"

 // НиЭМ
class PictureShot{

	PictureShot();
	~PictureShot();
public:
	void PicShot();


private:
	int FullScreenShot();

	int PictureShot::CImageToMat(CImage& cimage, cv::Mat& mat);

	void OnMouseCut(int event, int x, int y, int flags, void *ustc);

	std::vector<cv::Mat> vecmat;

	cv::Mat srcMat, dstMat, img;
};

#endif