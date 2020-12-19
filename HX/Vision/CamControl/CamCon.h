#pragma once

#include "../../pch.h"
#include "../opencv_include.h"
#include "../../CPublic.h"

using namespace std;
using namespace cv;


//��ʼ��PitureControl�ؼ�
void PicConInit(HWND hWnd, int IDD, const char* windowname);



class CSampleDeviceOfflineEventHandler : public IDeviceOfflineEventHandler
{
public:

	void DoOnDeviceOfflineEvent(void* pUserParam);
};

//�û��̳����Ը����¼�������

class CSampleFeatureEventHandler : public IFeatureEventHandler
{
public:
	void DoOnFeatureEvent(const GxIAPICPP::gxstring& strFeatureName, void* pUserParam)
	{
		cout << "�յ��ع�����¼�!" << endl;
	}
};

//�û��̳вɼ��¼�������
class CSampleCaptureEventHandler : public ICaptureEventHandler
{
public:
	
	void DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* pUserParam);
	
};
class SingleCam;


/// <summary>
/// Cam����ȡ�������ɵ�����������߳�ʼ����������init + ɨ��
/// �����������
/// </summary>
class CamCon //Cam����
{
public:

	enum {
		LEFT,
		RIGHT,
	};


	CamCon() { CamInit(); };
	~CamCon() { CamUnInit(); };
	int init_all_cam(vector<shared_ptr<SingleCam>>& cam_vec);
	//��ʼ������
	int CamInit();
	int CamUnInit();


	static shared_ptr<SingleCam> LEFT_CAM;
	static shared_ptr<SingleCam> RIGHT_CAM;

public:
	
private:
	//��ֹ����
	CamCon(const CamCon&);
	CamCon operator=(const CamCon&);
	
	gxdeviceinfo_vector vectorDeviceInfo;//�������߰����������Ϣ
	Mat img;
	
};
class SingleCam//�������
{
	friend class CSampleCaptureEventHandler;
	friend class CamCon;
public:

	SingleCam(CGXDevicePointer m_ODP , gxstring m_serialNum, const char* windowname,cv::Rect roi = cv::Rect(PC_WIDTH / 3, 0, PC_WIDTH / 3, PC_HEIGHT) );
	~SingleCam();
public:
	void Record_start();
	void Record_stop();
	cv::Rect ROI;

	inline bool& IsOffline()
	{
		return m_bIsOffline;
	}

	gxstring GetSN() const
	{
		return serialNum;
	}
	string GetShowWindow() const
	{
		return showWindowName;
	}
	Mat GetSrc() const
	{
		return src;
	}

private:
	//��ֹ�������
	SingleCam(const SingleCam&);
	const SingleCam& operator=(const SingleCam&);
	//���ߴ���
	void __ProcessOffline();
	//��������
	void __Recovery();
	//����Handler
	IDeviceOfflineEventHandler *pDeviceOfflineEventHandler;///<�����¼��ص�����
	IFeatureEventHandler       *pFeatureEventHandler;///<Զ���豸�¼��ص�����
	ICaptureEventHandler       *pCaptureEventHandler;///<�ɼ��ص�����
	CGXFeatureControlPointer   ObjFeatureControlPtr;//�豸���Կ�����

	CGXDevicePointer ObjDevicePtr;//�豸ָ��
	CGXStreamPointer ObjStreamPtr;//����

	GX_DEVICE_OFFLINE_CALLBACK_HANDLE hDeviceOffline; //���߻ص����

	//��־λ����ʾ�豸�Ƿ�����
	bool m_bIsOffline;
	bool m_bIsSnap;
	bool m_bIsOpened;

	//���к�
	gxstring serialNum;
	//������ʾ��������
	string showWindowName;
	
	//Դͼ
	Mat src;
	//���к��ԭͼ��
	Mat croppedSrc;
	//��С���ԭͼ
	Mat resizedSrc;
	//��С��Ĳ���ͼ
	Mat resizedCropped;

};



//֮��˫�����ĵĴ���
int StartCam(std::shared_ptr<SingleCam> p);
int StopCam(std::shared_ptr<SingleCam> p);

static CamCon& CAMVEC()
{
	static CamCon c1;
	return c1;
}
static vector<shared_ptr<SingleCam>>& SCV()
{
	static vector<shared_ptr<SingleCam>> c1;
	return c1;
}



