/**
* Descrition: picture shot
* Date : 20190509
* author:
*
**/

#ifndef PICTURE_SHOT_H
#define PICTURE_SHOT_H

#include "atlimage.h"
#include "atltime.h"
#include "conio.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "cJSON.h"
 // НиЭМ
class PictureShot{
public:
	PictureShot();

	int PicShot(std::string &sHostAddress);

	~PictureShot();

	int PicShot();


private:
	int FullScreenShot();

	int PictureShot::CImageToMat(CImage& cimage, cv::Mat& mat);

	void OnMouseCut(int event, int x, int y, int flags, void *ustc);

private:
	bool TranMatToBase64(const cv::Mat &img, std::string sImgType, std::string &sBase64);

	std::string Base64Encode(const unsigned char* Data, int DataByte);

	int SendToEngine(const std::string &sBase64, std::string &sHostAddress);

	std::string UTF_82ASCII(const std::string& strUtfCode);

	std::string ASCII2UTF_8(const std::string& strAsciiCode);

	std::vector<cv::Mat> vecmat;

	cv::Mat srcMat, dstMat, img;
};

#endif