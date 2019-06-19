#include "PictureShot.h"
#include "stdio.h"
#include "stdlib.h"

#include "atlimage.h"
#include "atltime.h"
#include "conio.h"

void PictureShot::PicShot()
{
	int errcode = 0;
	errcode = FullScreenShot();
	if (errcode == 0)
	{
		cv::imshow("img", img);
		cv::setMouseCallback("img", OnMouseCut, 0);//调用回调函数 
		cv::waitKey(0);
	}
	else
	{
		return;
	}
}


//截取图片为png
int PictureShot::FullScreenShot()
{
    HDC hDCScreen = ::GetDC(NULL);//首先获取到屏幕的句柄    
    int nBitPerPixel = GetDeviceCaps(hDCScreen, BITSPIXEL);//获取到每个像素的bit数目
    int nWidthScreen = GetDeviceCaps(hDCScreen, HORZRES);
    int nHeightScreen = GetDeviceCaps(hDCScreen, VERTRES);
    //创建一个CImage的对象
    CImage m_MyImage;
    //Create实例化CImage，使得其内部的画布大小与屏幕一致
    m_MyImage.Create(nWidthScreen, nHeightScreen, nBitPerPixel);
    //获取到CImage的 HDC,但是需要手动ReleaseDC操作,下面是MSDN的说明
    //Because only one bitmap can be selected into a device context at a time, 
    //you must call ReleaseDC for each call to GetDC.
    HDC hDCImg = m_MyImage.GetDC();
    //使用bitblt 将屏幕的DC画布上的内容 拷贝到CImage上
    BitBlt(hDCImg, 0, 0, nWidthScreen, nHeightScreen, hDCScreen, 0, 0, SRCCOPY);

	// 转化成Mat存储到vector中
	cv::Mat srcMat;
	int errcode = CImageToMat(m_MyImage, srcMat);
	if(errcode == 0) {
		vecmat.push_back(srcMat);
	} else {
		return -1;
	}
    //前面调用了GetDC所以需要调用ReleaseDC释放掉
    //详情请参见MSDN
    m_MyImage.ReleaseDC();
    return 0;
}


// 鼠标截图
void PictureShot::OnMouseCut(int event, int x, int y, int flags, void *ustc)
{
	static cv::Point pre_pt = (-1, -1);//初始坐标 
	static cv::Point cur_pt = (-1, -1);//实时坐标
	cv::Point cirpoint = (-1, -1);//圆心坐标
	int r = 0;
	cv::Mat tempMat;
	std::vector<cv::Mat>::iterator it ;
	for(it=vecmat.begin(); it!=vecmat.end(); it++) {
		srcMat = *it;
		if (&srcMat == NULL) {
			return;
		}
	}

 char temp[16];
 if (event == CV_EVENT_LBUTTONDOWN)//左键按下，读取初始坐标，并在图像上该点处划圆 
 {
	 srcMat.copyTo(img);//将原始图片复制到img中 
	 sprintf(temp, "(%d,%d)", x, y);
	 pre_pt = cv::Point(x, y);
	 putText(img, temp, pre_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255), 1, 8);//在窗口上显示坐标 
	 circle(img, pre_pt, 2, cv::Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);//划圆 
	// imshow("img", img);
 }
 else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))//左键没有按下的情况下鼠标移动的处理函数 
 {
	 img.copyTo(tempMat);//将img复制到临时图像tempMat上，用于显示实时坐标 
	 sprintf(temp, "(%d,%d)", x, y);
	 cur_pt = cv::Point(x, y);
	 putText(tempMat, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255));//只是实时显示鼠标移动的坐标 
	// cv::imshow("img", tempMat);
 }
 else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))//左键按下时，鼠标移动，则在图像上划矩形 
 {
	 img.copyTo(tempMat);
	 sprintf(temp, "(%d,%d)", x, y);
	 cur_pt = cv::Point(x, y);
	 putText(tempMat, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255));
	 rectangle(tempMat, pre_pt, cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);//在临时图像上实时显示鼠标拖动时形成的矩形 
	// cv::imshow("img", tempMat);
 }
 else if (event == CV_EVENT_LBUTTONUP)//左键松开，将在图像上划矩形 
 {
	 srcMat.copyTo(img);
	 sprintf(temp, "(%d,%d)", x, y);
	 cur_pt = cv::Point(x, y);
	 putText(img, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255));
	 //circle(img, pre_pt, 2, Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
	 //rectangle(img, pre_pt, cur_pt, Scalar(0, 255, 0, 0), 1, 8, 0);//根据初始点和结束点，将矩形画到img上 
	 cirpoint.x = (pre_pt.x + cur_pt.x) / 2;
	 cirpoint.y = (pre_pt.y + cur_pt.y) / 2;
	 r = min(abs(pre_pt.x - cur_pt.x), abs(pre_pt.y - cur_pt.y));
	 r = r / 2.0;
	 circle(img, cirpoint, r, cv::Scalar(0, 0, 225, 0), 1, 8, 0); //绘制第一个圆，半径为100，圆心（350，300），线宽为7
	 img.copyTo(tempMat);
	 //截取矩形包围的图像，并保存到dstMat中 
	 int width = abs(pre_pt.x - cur_pt.x);
	 int height = abs(pre_pt.y - cur_pt.y);
	 if (width == 0 || height == 0)
	 {
		 printf("width == 0 || height == 0");
		 return;
	 }
	 dstMat = srcMat(cv::Rect(min(cur_pt.x, pre_pt.x), min(cur_pt.y, pre_pt.y), width, height));
	 cv::waitKey(0);
 }
}

 // CImageToMat
int PictureShot::CImageToMat(CImage& cimage, cv::Mat& mat)
{
	if (true == cimage.IsNull())
	{
		return -1;
	}
	int nChannels = cimage.GetBPP() / 8;
	if ((1 != nChannels) && (3 != nChannels))
	{
		return -2;
	}
	int nWidth    = cimage.GetWidth();
	int nHeight   = cimage.GetHeight();
	//重建mat
	if (1 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC1);
	}
	else if(3 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC3);
	}
	//拷贝数据
	uchar* pucRow;//指向数据区的行指针
	uchar* pucImage = (uchar*)cimage.GetBits();//指向数据区的指针
	int nStep = cimage.GetPitch();  //每行的字节数,注意这个返回值有正有负
	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				pucRow[nCol] = *(pucImage + nRow * nStep + nCol);
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0 ; nCha < 3; nCha++)
				{
					pucRow[nCol * 3 + nCha] = *(pucImage + nRow * nStep + nCol * 3 + nCha);
				}
			}
		}
	}
	return 0;
}

