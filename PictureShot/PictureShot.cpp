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
		cv::setMouseCallback("img", OnMouseCut, 0);//���ûص����� 
		cv::waitKey(0);
	}
	else
	{
		return;
	}
}


//��ȡͼƬΪpng
int PictureShot::FullScreenShot()
{
    HDC hDCScreen = ::GetDC(NULL);//���Ȼ�ȡ����Ļ�ľ��    
    int nBitPerPixel = GetDeviceCaps(hDCScreen, BITSPIXEL);//��ȡ��ÿ�����ص�bit��Ŀ
    int nWidthScreen = GetDeviceCaps(hDCScreen, HORZRES);
    int nHeightScreen = GetDeviceCaps(hDCScreen, VERTRES);
    //����һ��CImage�Ķ���
    CImage m_MyImage;
    //Createʵ����CImage��ʹ�����ڲ��Ļ�����С����Ļһ��
    m_MyImage.Create(nWidthScreen, nHeightScreen, nBitPerPixel);
    //��ȡ��CImage�� HDC,������Ҫ�ֶ�ReleaseDC����,������MSDN��˵��
    //Because only one bitmap can be selected into a device context at a time, 
    //you must call ReleaseDC for each call to GetDC.
    HDC hDCImg = m_MyImage.GetDC();
    //ʹ��bitblt ����Ļ��DC�����ϵ����� ������CImage��
    BitBlt(hDCImg, 0, 0, nWidthScreen, nHeightScreen, hDCScreen, 0, 0, SRCCOPY);

	// ת����Mat�洢��vector��
	cv::Mat srcMat;
	int errcode = CImageToMat(m_MyImage, srcMat);
	if(errcode == 0) {
		vecmat.push_back(srcMat);
	} else {
		return -1;
	}
    //ǰ�������GetDC������Ҫ����ReleaseDC�ͷŵ�
    //������μ�MSDN
    m_MyImage.ReleaseDC();
    return 0;
}


// ����ͼ
void PictureShot::OnMouseCut(int event, int x, int y, int flags, void *ustc)
{
	static cv::Point pre_pt = (-1, -1);//��ʼ���� 
	static cv::Point cur_pt = (-1, -1);//ʵʱ����
	cv::Point cirpoint = (-1, -1);//Բ������
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
 if (event == CV_EVENT_LBUTTONDOWN)//������£���ȡ��ʼ���꣬����ͼ���ϸõ㴦��Բ 
 {
	 srcMat.copyTo(img);//��ԭʼͼƬ���Ƶ�img�� 
	 sprintf(temp, "(%d,%d)", x, y);
	 pre_pt = cv::Point(x, y);
	 putText(img, temp, pre_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255), 1, 8);//�ڴ�������ʾ���� 
	 circle(img, pre_pt, 2, cv::Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);//��Բ 
	// imshow("img", img);
 }
 else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))//���û�а��µ����������ƶ��Ĵ����� 
 {
	 img.copyTo(tempMat);//��img���Ƶ���ʱͼ��tempMat�ϣ�������ʾʵʱ���� 
	 sprintf(temp, "(%d,%d)", x, y);
	 cur_pt = cv::Point(x, y);
	 putText(tempMat, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255));//ֻ��ʵʱ��ʾ����ƶ������� 
	// cv::imshow("img", tempMat);
 }
 else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))//�������ʱ������ƶ�������ͼ���ϻ����� 
 {
	 img.copyTo(tempMat);
	 sprintf(temp, "(%d,%d)", x, y);
	 cur_pt = cv::Point(x, y);
	 putText(tempMat, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255));
	 rectangle(tempMat, pre_pt, cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);//����ʱͼ����ʵʱ��ʾ����϶�ʱ�γɵľ��� 
	// cv::imshow("img", tempMat);
 }
 else if (event == CV_EVENT_LBUTTONUP)//����ɿ�������ͼ���ϻ����� 
 {
	 srcMat.copyTo(img);
	 sprintf(temp, "(%d,%d)", x, y);
	 cur_pt = cv::Point(x, y);
	 putText(img, temp, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255));
	 //circle(img, pre_pt, 2, Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
	 //rectangle(img, pre_pt, cur_pt, Scalar(0, 255, 0, 0), 1, 8, 0);//���ݳ�ʼ��ͽ����㣬�����λ���img�� 
	 cirpoint.x = (pre_pt.x + cur_pt.x) / 2;
	 cirpoint.y = (pre_pt.y + cur_pt.y) / 2;
	 r = min(abs(pre_pt.x - cur_pt.x), abs(pre_pt.y - cur_pt.y));
	 r = r / 2.0;
	 circle(img, cirpoint, r, cv::Scalar(0, 0, 225, 0), 1, 8, 0); //���Ƶ�һ��Բ���뾶Ϊ100��Բ�ģ�350��300�����߿�Ϊ7
	 img.copyTo(tempMat);
	 //��ȡ���ΰ�Χ��ͼ�񣬲����浽dstMat�� 
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
	//�ؽ�mat
	if (1 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC1);
	}
	else if(3 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC3);
	}
	//��������
	uchar* pucRow;//ָ������������ָ��
	uchar* pucImage = (uchar*)cimage.GetBits();//ָ����������ָ��
	int nStep = cimage.GetPitch();  //ÿ�е��ֽ���,ע���������ֵ�����и�
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

