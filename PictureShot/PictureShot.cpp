#include "PictureShot.h"
#include "stdio.h"
#include "stdlib.h"
#include "map"
#include "httpclient.h"

#define LOG_ERROR(...) fprintf(stderr, "ERROR: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#define LOG_WARN(...) fprintf(stderr, "WARN: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#define LOG_NPTICE(...) fprintf(stderr, "NOTICE: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#define LOG_INFO(...) fprintf(stderr, "INFO: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")

PictureShot::PictureShot()
{

}

PictureShot::~PictureShot()
{

}

int PictureShot::PicShot(std::string &sHostAddress)
{
	// sHostAddress = "http://ip:port/ocr/note";
	int errcode = 0;
	errcode = FullScreenShot();
	if (errcode == 0)
	{
		cv::setMouseCallback("img", OnMouseCut, 0);//���ûص����� 
		cv::waitKey(0);
		//����֮���ͼƬ���б��뷢�͵���Ӧ�Ľӿ�
		// dstMat ������װ��json�н���http����
		std::string sBase64;
		bool bFlag = TranMatToBase64(dstMat, "png", sBase64);
		if(bFlag)
		{
			SendToEngine(sBase64, sHostAddress);
		}
		else
		{
			LOG_ERROR("TranMatToBase64 Failed ! \n");
			return -1;
		}
	}
	return 0;
}

// ����ͼ
void PictureShot::OnMouseCut(int event, int x, int y, int flags, void *ustc)
{
	static cv::Point pre_pt = (-1, -1);//��ʼ���� 
	static cv::Point cur_pt = (-1, -1);//ʵʱ����
	cv::Point cirpoint = (-1, -1);     //Բ������
	int r = 0;
	cv::Mat tempMat;
	std::vector<cv::Mat>::iterator it ;
	for(it = vecmat.begin(); it!=vecmat.end(); it++)
	{
		srcMat = *it;
		if (&srcMat == NULL) 
		{
			return;
		}
	}

 char temp[16];
 if (event == CV_EVENT_LBUTTONDOWN)//������£���ȡ��ʼ���꣬����ͼ���ϸõ㴦��Բ 
 {
	 srcMat.copyTo(img);//��ԭʼͼƬ���Ƶ�img�� 
	 sprintf(temp,"(%d, %d)", x, y);
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
	 r = cv::min(abs(pre_pt.x - cur_pt.x), abs(pre_pt.y - cur_pt.y));
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
	 dstMat = srcMat(cv::Rect(cv::min(cur_pt.x, pre_pt.x), cv::min(cur_pt.y, pre_pt.y), width, height));
	 cv::waitKey(0);
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
	//����ReleaseDC�ͷŵ�GetDC
	m_MyImage.ReleaseDC();
	if(errcode == 0) 
	{
		vecmat.push_back(srcMat);
		return 0;
	} 
	else
	{
		return -1;
	}
}

// ��������
/**
http://ip:port/ocr/note
{
"app_id":"system",
"app_secret":"12345",
"img":"",
"seq": "f428b43e-5bc1-4bea-a93a-ed177ca4552e"
}
**/

// ��װ��json���͵�ָ���ӿ�
int PictureShot::SendToEngine(const std::string &sBase64, std::string &sHostAddress)
{
	// ����http��������ӿ�
	cJSON *Json_send = cJSON_CreateObject();
	if (!Json_send)
	{
		LOG_ERROR("cJSON_CreateObject ERROR!");
		return -2;
	}

	std::map<std::string, std::string> mapSigh;
	std::string seq;
	cJSON_AddStringToObject(Json_send, "app_id", "system");
	cJSON_AddStringToObject(Json_send, "app_secret", "123456");
	cJSON_AddStringToObject(Json_send, "img",sBase64.c_str() );
	cJSON_AddStringToObject(Json_send, "seq", seq.c_str());
	char *pSendVal = cJSON_Print(Json_send);
	std::string sContent;
	try
	{
		CHttpClient().PostsJson(sHostAddress, pSendVal, 30, sContent);
	}
	catch (...)
	{
		return -3;
	}

	cJSON_Delete(Json_send);
	if (pSendVal)
	{
		free(pSendVal);
	}

	// �������ص�JSON 
	cJSON *Json_conf = cJSON_Parse(sContent.c_str());
	if (!Json_conf)
	{		
		return -4;
	}

	int iResultCode = 0;
	cJSON *pTemp = cJSON_GetObjectItem(Json_conf, "code");
	if (pTemp)
	{
		iResultCode = pTemp->valueint;
	}
	if (iResultCode != 200)
	{
		std::string message;
		cJSON *pTempSeq = cJSON_GetObjectItem(Json_conf, "message");
		if (pTempSeq)
		{
			message = UTF_82ASCII(pTempSeq->valuestring);
		}
		printf("error code: %d, info: %s\n", iResultCode, message.c_str());
		return -5;
	}

	cJSON_Delete(Json_conf);
	Json_conf = NULL;

	return 0;
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

bool PictureShot::TranMatToBase64(const cv::Mat &img, std::string sImgType, std::string &sBase64)
{
	//Matתbase64
	std::vector<uchar> vecImg;
	std::vector<int> vecCompression_params;
	vecCompression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	vecCompression_params.push_back(90);
	sImgType = "." + sImgType;
	cv::imencode(sImgType, img, vecImg, vecCompression_params);
	sBase64 = Base64Encode(vecImg.data(), vecImg.size());
	return true;
}

std::string PictureShot::Base64Encode(const unsigned char* Data, int DataByte) 
{
	//�����
	const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	//����ֵ
	std::string s_Encode;
	unsigned char Tmp[4] = { 0 };
	int LineLength = 0;
	for (int i = 0; i < (int)(DataByte / 3); i++) 
	{
		Tmp[1] = *Data++;
		Tmp[2] = *Data++;
		Tmp[3] = *Data++;
		s_Encode += EncodeTable[Tmp[1] >> 2];
		s_Encode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
		s_Encode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
		s_Encode += EncodeTable[Tmp[3] & 0x3F];
		if (LineLength += 4, LineLength == 76) 
		{ 
			s_Encode += "\r\n"; LineLength = 0; 
		}
	}
	//��ʣ�����ݽ��б���
	int Mod = DataByte % 3;
	if (Mod == 1) 
	{
		Tmp[1] = *Data++;
		s_Encode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
		s_Encode += EncodeTable[((Tmp[1] & 0x03) << 4)];
		s_Encode += "==";
	}
	else if (Mod == 2) 
	{
		Tmp[1] = *Data++;
		Tmp[2] = *Data++;
		s_Encode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
		s_Encode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
		s_Encode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
		s_Encode += "=";
	}
	return s_Encode;
}


std::string PictureShot::UTF_82ASCII(const std::string& strUtfCode)
{
#ifdef WIN32

	std::string sRet;
	int widesize = MultiByteToWideChar (CP_UTF8, 0, strUtfCode.c_str(), -1, NULL, 0);  
	if (0 == widesize || widesize == ERROR_NO_UNICODE_TRANSLATION)  
	{  
		return sRet; 
	}    

	std::vector<wchar_t> resultstring(widesize); 
	int convresult = MultiByteToWideChar (CP_UTF8, 0, strUtfCode.c_str(), -1, &resultstring[0], widesize);  
	if (convresult != widesize)  
	{  
		return sRet; 
	}  

	LPCWSTR lpVal = &resultstring[0];
	int utf8size = ::WideCharToMultiByte(CP_ACP, 0, lpVal, -1, NULL, 0, NULL, NULL);  
	if (utf8size == 0)  
	{  
		return sRet; 
	}  

	std::vector<char> resultstringVec(utf8size);  
	convresult = ::WideCharToMultiByte(CP_ACP, 0, lpVal, -1, &resultstringVec[0], utf8size, NULL, NULL);  
	if (convresult != utf8size)  
	{  
		return sRet;  
	}  

	sRet = &resultstringVec[0];
	return sRet; 

#else

	return strUtfCode;

#endif
}

std::string PictureShot::ASCII2UTF_8(const std::string& strAsciiCode) 
{
#ifdef WIN32

	std::string sRet;
	int widesize = MultiByteToWideChar (CP_ACP, 0, strAsciiCode.c_str(), -1, NULL, 0);  
	if (0 == widesize || widesize == ERROR_NO_UNICODE_TRANSLATION)  
	{  
		return sRet; 
	}    

	std::vector<wchar_t> resultstring(widesize); 
	int convresult = MultiByteToWideChar (CP_ACP, 0, strAsciiCode.c_str(), -1, &resultstring[0], widesize);  
	if (convresult != widesize)  
	{  
		return sRet; 
	}  

	LPCWSTR lpVal = &resultstring[0];
	int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, lpVal, -1, NULL, 0, NULL, NULL);  
	if (utf8size == 0)  
	{  
		return sRet; 
	}  

	std::vector<char> resultstringVec(utf8size);  
	convresult = ::WideCharToMultiByte(CP_UTF8, 0, lpVal, -1, &resultstringVec[0], utf8size, NULL, NULL);  
	if (convresult != utf8size)  
	{  
		return sRet;  
	}  

	sRet = &resultstringVec[0];
	return sRet; 

#else

	return strAsciiCode;

#endif

}
