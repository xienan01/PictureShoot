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
		cv::setMouseCallback("img", OnMouseCut, 0);//调用回调函数 
		cv::waitKey(0);
		//处理之后的图片进行编码发送到对应的接口
		// dstMat 编码组装到json中进行http发送
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

// 鼠标截图
void PictureShot::OnMouseCut(int event, int x, int y, int flags, void *ustc)
{
	static cv::Point pre_pt = (-1, -1);//初始坐标 
	static cv::Point cur_pt = (-1, -1);//实时坐标
	cv::Point cirpoint = (-1, -1);     //圆心坐标
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
 if (event == CV_EVENT_LBUTTONDOWN)//左键按下，读取初始坐标，并在图像上该点处划圆 
 {
	 srcMat.copyTo(img);//将原始图片复制到img中 
	 sprintf(temp,"(%d, %d)", x, y);
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
	 r = cv::min(abs(pre_pt.x - cur_pt.x), abs(pre_pt.y - cur_pt.y));
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
	 dstMat = srcMat(cv::Rect(cv::min(cur_pt.x, pre_pt.x), cv::min(cur_pt.y, pre_pt.y), width, height));
	 cv::waitKey(0);
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
	//调用ReleaseDC释放掉GetDC
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

// 发送样例
/**
http://ip:port/ocr/note
{
"app_id":"system",
"app_secret":"12345",
"img":"",
"seq": "f428b43e-5bc1-4bea-a93a-ed177ca4552e"
}
**/

// 组装到json发送到指定接口
int PictureShot::SendToEngine(const std::string &sBase64, std::string &sHostAddress)
{
	// 发送http请求到引擎接口
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

	// 解析返回的JSON 
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

bool PictureShot::TranMatToBase64(const cv::Mat &img, std::string sImgType, std::string &sBase64)
{
	//Mat转base64
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
	//编码表
	const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	//返回值
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
	//对剩余数据进行编码
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
