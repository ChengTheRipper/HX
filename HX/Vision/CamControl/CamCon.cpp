#include "CamCon.h"



int CamCon::init_all_cam(vector<shared_ptr<SingleCam>>& cam_vec)
{
	bool bIsDeviceOpen = false;              ///< �豸�Ƿ��Ѵ򿪱�ʶ
	bool bIsStreamOpen = false;              ///< �豸���Ƿ��Ѵ򿪱�ʶ
	try
	{
		if (!cam_vec.empty())
			return -1;
		int i = 0;
		const size_t totalDeviceNum = vectorDeviceInfo.size();
		CString a;
		a.Format(_T("%d ���豸"), totalDeviceNum);
		AfxMessageBox(a);

		if (totalDeviceNum == 0) // ��ȫ�Ѳ����豸
		{
			throw exception("No Device Detected");
		}
		//��ʱ�Ȳ��Ե�������κδ���
		/*else if (totalDeviceNum == 1)
		{
			throw exception("Only one Cam Is Detected");
		}*/

		cam_vec.resize(2);

		CGXDevicePointer ObjDevicePtr;
		
		while (i < totalDeviceNum)
		{

			AfxMessageBox(CString(vectorDeviceInfo[i].GetVendorName()));
			AfxMessageBox(CString(vectorDeviceInfo[i].GetSN()));//ͨ��Serial Number ���п������кô�����MAC����ֱ��
			gxstring serialNum = vectorDeviceInfo[i].GetSN();
		
			ObjDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(
				serialNum,
				GX_ACCESS_EXCLUSIVE);
			bIsDeviceOpen = true;

			if (serialNum.c_str() == CPublic::LEFT_CAM_SN())
			{
				shared_ptr<SingleCam> p(new SingleCam(ObjDevicePtr, serialNum, CPublic::LEFT_MATWINDOW_NAME()));
				cam_vec[LEFT] = p;

			}
			else if (serialNum.c_str() == CPublic::RIGHT_CAM_SN())
			{
				shared_ptr<SingleCam> p(new SingleCam(ObjDevicePtr, serialNum, CPublic::RIGHT_MATWINDOW_NAME()));
				cam_vec[RIGHT] = p;

			}
			else
			{
				throw exception("Found a new Cam");
			}

		}
	}
	
	
	catch (CGalaxyException& e)
	{
		cout << "������: " << e.GetErrorCode() << endl;
		cout << "����������Ϣ: " << e.what() << endl;
		AfxMessageBox(CString(e.what()));
		//�����ʼ���������⣬��ʾ�Ƿ�ѡ���Ƿ�����������
		//1.������
		//2.���и�λ
		//���cam_vec��Ϊ�գ���ֱ������������ִ�й��豸������
		if (!cam_vec.empty())
			cam_vec.clear();
	}
	catch (std::exception& e)
	{
		cout << "����������Ϣ: " << e.what() << endl;
		
		//�����ʼ���������⣬ѡ���Ƿ���и�λ�ж�

		if (string(e.what()) == "No Device Detected")
		{
			//ִ����Ӧ�Ĵ�ʩ�������ṩһ����ť�ٴ�ִ��init_all_cam
			AfxMessageBox(CString(e.what()));
		}
		//else if (string(e.what()) == "Only one Cam Is Detected")
		//{
		//	//ִ����Ӧ�Ĵ�ʩ�������ṩһ����ť�ٴ�ִ��init_all_cam
		//	//�����������ִ�м��������һ�����Ȳ���
		//}
		//���cam_vec��Ϊ�գ���ֱ������������ִ�й��豸������
		if (!cam_vec.empty())
			cam_vec.clear();
	}

	return 0;
}

int CamCon::CamInit()
{
	//��ʼ��
	IGXFactory::GetInstance().Init();
	//ö���豸
	IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
	if (0 == vectorDeviceInfo.size())
	{
		//cout << "�޿����豸!" << endl;
		CString C(L"�޿����豸!");
		AfxMessageBox(C);
		return -1;
	}
			
}

int CamCon::CamUnInit()
{
	IGXFactory::GetInstance().Uninit();
	return 0;
}

SingleCam::SingleCam(CGXDevicePointer m_ODP, gxstring m_serialNum, const char* windowname, cv::Rect roi )
	: ObjDevicePtr(m_ODP), serialNum(m_serialNum), showWindowName(windowname),ROI(roi), m_bIsOffline(false), m_bIsSnap(false), m_bIsOpened(true)/*��ʱ�豸����һ���Ǵ򿪵�*/
{
	bool bIsDeviceOpen = true;              ///< �豸�Ƿ��Ѵ򿪱�ʶ
	bool bIsStreamOpen = false;              ///< �豸���Ƿ��Ѵ򿪱�ʶ


	try 
	{
		//����

		ObjStreamPtr = ObjDevicePtr->OpenStream(0);
		bIsStreamOpen = true;

		//ע���豸���ߴ�����
		//GX_DEVICE_OFFLINE_CALLBACK_HANDLE hDeviceOffline = NULL;
		pDeviceOfflineEventHandler = new CSampleDeviceOfflineEventHandler();
		hDeviceOffline = ObjDevicePtr->RegisterDeviceOfflineCallback(pDeviceOfflineEventHandler, this);
		//��ȡԶ���豸���Կ�����
		ObjFeatureControlPtr = ObjDevicePtr->GetRemoteFeatureControl();
		//ע��ص��ɼ�
		pCaptureEventHandler = new CSampleCaptureEventHandler();
		ObjStreamPtr->RegisterCaptureCallback(pCaptureEventHandler, this);
		//ObjStreamPtr->RegisterCaptureCallback(pCaptureEventHandler, NULL);
	}
	
	catch (CGalaxyException &e)
	{
		//�ж��豸���Ƿ��Ѵ�
		if (bIsStreamOpen)
		{
			ObjStreamPtr->Close();
		}

		//�ж��豸�Ƿ��Ѵ�
		if (bIsDeviceOpen)
		{
			ObjDevicePtr->Close();
		}

		cout << "<" << e.GetErrorCode() << ">" << "<" << e.what() << ">" << endl;
		//��ʾ������ע��ʧ�ܣ������豸�Ѿ��ر�
	}
	catch (std::exception &e)
	{
		//�ж��豸���Ƿ��Ѵ�
		if (bIsStreamOpen)
		{
			ObjStreamPtr->Close();
		}

		//�ж��豸�Ƿ��Ѵ�
		if (bIsDeviceOpen)
		{
			ObjDevicePtr->Close();
		}

		cout << "<" << e.what() << ">" << endl;
		//��ʾ����ע��ʧ�ܣ������豸�Ѿ��ر�

	}
	
}
SingleCam::~SingleCam()
{
	ObjStreamPtr->UnregisterCaptureCallback();
	ObjStreamPtr->Close();//�ر���Ƶ��
	ObjDevicePtr->Close();//�ر��豸
	//�����¼��ص�ָ��
	if (NULL != pCaptureEventHandler)
	{
		delete pCaptureEventHandler;
		pCaptureEventHandler = NULL;
	}
	if (NULL != pDeviceOfflineEventHandler)
	{
		delete pDeviceOfflineEventHandler;
		pDeviceOfflineEventHandler = NULL;
	}
	if (NULL != pFeatureEventHandler)
	{
		delete pFeatureEventHandler;
		pFeatureEventHandler = NULL;
	}
}

void SingleCam::Record_start()
{
	try
	{
		ObjStreamPtr->StartGrab();
		ObjFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();
		m_bIsSnap = true;//��λִ�вɼ���־λ
	}
	catch (CGalaxyException& e)
	{
		cout << "������: " << e.GetErrorCode() << endl;
		cout << "����������Ϣ: " << e.what() << endl;
		AfxMessageBox(CString(e.what()));
	}
	catch (std::exception& e)
	{
		cout << "����������Ϣ: " << e.what() << endl;
	}

}
void SingleCam::Record_stop()
{
	
	ObjFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
	ObjStreamPtr->StopGrab();
	m_bIsSnap = false;	

	//ֹͣ�ɼ������Mat_Vec
	Mat resized;
	resize(src, resized, Size(PC_WIDTH / AS_RATIO, PC_HEIGHT / AS_RATIO));
	CPublic::Mat_Vec()[CPublic::ORIGINAL] = src;
	CPublic::Mat_Vec()[CPublic::RESIZED] = resized;
	CPublic::Mat_Vec()[CPublic::CROPPED_ORIGINAL] = src(ROI);
	cv::Rect roiResized(ROI.x / AS_RATIO, ROI.y / AS_RATIO, ROI.width / AS_RATIO, ROI.height / AS_RATIO);
	CPublic::Mat_Vec()[CPublic::CROPPED_RESIZED] = resized(roiResized);
	//imshow("resized", CPublic::Mat_Vec()[CPublic::CROPPED_ORIGINAL]);
	imwrite("test_pic.bmp", CPublic::Mat_Vec()[CPublic::CROPPED_ORIGINAL]);

}

void SingleCam::__ProcessOffline()
{
	try
	{
		printf("**********************Process Offline**********************\r");
		//�ж��豸�Ƿ�ֹͣ�ɼ�
		if (m_bIsSnap)
		{
			cout << "\n<Send stop snap command to device>" << endl;
			ObjFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
		}
	}
	catch (CGalaxyException &e)
	{
		cout << "<" << e.GetErrorCode() << ">" << "<" << e.what() << ">" << endl;
	}
	catch (std::exception &e)
	{
		cout << "<" << e.what() << ">" << endl;
	}

	try
	{
		//�ж��豸�Ƿ�ֹͣ�ɼ�
		if (m_bIsSnap)
		{
			ObjStreamPtr->StopGrab();
			m_bIsSnap = false;
		}
	}
	catch (CGalaxyException)
	{
		//do noting
	}
	catch (std::exception)
	{
		//do noting
	}

	try
	{
		//�ж��豸�Ƿ��
		if (m_bIsOpened)
		{
		//ע�����߻ص�����
			cout << "<Unregister device Offline callback>" << endl;
			ObjDevicePtr->UnregisterDeviceOfflineCallback(hDeviceOffline);

			//�ر������豸
			cout << "<Close Device>" << endl;
			ObjStreamPtr->Close();
			ObjDevicePtr->Close();
			m_bIsOpened = false;

		}
	}
	catch (CGalaxyException)
	{
		//do noting
	}
	catch (std::exception)
	{
		//do noting
	}
}

void PicConInit(HWND hWnd_dlg, int IDD, const char* windowname)
{
	cv::namedWindow(windowname, cv::WINDOW_AUTOSIZE);
	HWND hWnd_pic = GetDlgItem(hWnd_dlg, IDD);

	//�趨�������Ӳ�Դ�С
	CRect rect;
	GetWindowRect(hWnd_pic, rect);
	MoveWindow(hWnd_pic, rect.left, rect.top,
		PC_WIDTH / AS_RATIO, PC_HEIGHT / AS_RATIO, true);

	HWND hWnd = (HWND)cvGetWindowHandle(windowname);
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, hWnd_pic);
	::ShowWindow(hParent, SW_HIDE);
}

int StartCam(std::shared_ptr<SingleCam> p)
{
	/*if (SCV().empty())
	{
		AfxMessageBox(CString(" StartCam01 scv empty!"));
		return -1;
	}
	auto s1 = SCV().back();*/

	//��ROI_1��ָ��ָ������������ROI
	CPublic::ROI_1() = &(p->ROI);
	p->Record_start();
	return 0;
}

int StopCam(std::shared_ptr<SingleCam> p)
{
	/*if (SCV().empty())
	{
		AfxMessageBox(CString("  StopCam01 scv empty!"));
		return -1;
	}*/
	//auto s1 = SCV().back();

	p->Record_stop();
	return 0;
}

void NormalROI(Rect& input)
{
	if (input.x < 0)
		input.x = 0;
	else if (input.x > PC_WIDTH)
		input.x = PC_WIDTH;
	else if (input.y < 0)
		input.y = 0;
	else if (input.y > PC_HEIGHT)
		input.y = PC_HEIGHT;
}


//�豸�����¼�������
void CSampleDeviceOfflineEventHandler::DoOnDeviceOfflineEvent(void* pUserParam)
{
	SingleCam* Cam = static_cast<SingleCam*>(pUserParam);
	Cam->IsOffline() = true;
}

void CSampleCaptureEventHandler::DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* pUserParam)
{
	SingleCam* Cam = static_cast<SingleCam*>(pUserParam);
	if (Cam->IsOffline())
	{
		//���ߴ���
		//����
	}
	else
	{
		//�����ɼ�
		Cam->src.create(objImageDataPointer->GetHeight(), objImageDataPointer->GetWidth(), CV_8UC3);
		void* pRGB24Buffer = NULL;
		//����ԭʼ������BayerRG8ͼ��
		pRGB24Buffer = objImageDataPointer->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, true);
		memcpy(Cam->src.data, pRGB24Buffer, (objImageDataPointer->GetHeight()) * (objImageDataPointer->GetWidth()) * 3);
		cv::flip(Cam->src, Cam->src, ROTATE_90_CLOCKWISE);

		Mat img;
		resize(Cam->src, img, Size(Cam->src.cols / AS_RATIO, Cam->src.rows / AS_RATIO));

		cv::Rect& roi = Cam->ROI;//����ֱ�Ӱ�����������þ��������һ����Աָ�룬�β�ֱ���������ࣿ

		cv::Rect roi_resized(roi.x / 10, roi.y / 10, roi.width / 10, roi.height / 10);

		rectangle(img, roi_resized, Scalar(0, 0, 255), 8);
		cv::imshow(Cam->GetShowWindow(), img);
		cv::waitKey(1);
		//cout << "֡����" << objImageDataPointer->GetFrameID() << endl;
	}

	
}
