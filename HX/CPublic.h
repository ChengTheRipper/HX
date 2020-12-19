#pragma once
#include "Vision/opencv_include.h"
#include "Vision/STL_include.h"
using namespace std;

//���������������
enum
{
	PC_WIDTH = 5496,
	PC_HEIGHT = 3672,
	AS_RATIO = 10,
};

struct CPublic
{

	//����һ�������Mat��vector������
	//��һ���������ȫû���Ź���ԭͼ
	//�ڶ���λ�ô�������Ź���ԭͼ
	//��������Ų��к��ԭͼ
	enum
	{
		ORIGINAL,
		RESIZED,
		CROPPED_ORIGINAL,
		CROPPED_RESIZED,
	};
	static std::vector<cv::Mat>& Mat_Vec()
	{
		static std::vector<cv::Mat> Mat_Vec(4);
		return Mat_Vec;
	}


	//��������ʾ��������
	static constexpr char* LEFT_MATWINDOW_NAME()
	{
		return "view1";
	}
	static constexpr char* RIGHT_MATWINDOW_NAME()
	{
		return "view2";
	}

	//���������SN
	static const string& LEFT_CAM_SN()
	{
		static const string LCSN("LT0200091521");
		return LCSN;
	}
	//���������SN
	static const string& RIGHT_CAM_SN()
	{
		static const string RCSN("LT0200091520");
		return RCSN;
	}


	//�������1��ROI��ָ����
	static cv::Rect*& ROI_1()
	{
		static cv::Rect* roi_1 = NULL;
		return roi_1;
	}

	//�����ã�ģ�����1��ROI,ֱ�ӷ���һ��Rect����
	static cv::Rect& ROI1Temp()
	{
		static cv::Rect ro1(PC_WIDTH / 3, 0, PC_WIDTH / 3, PC_HEIGHT);
		return ro1;
	}

	static std::vector<bool>& allRuntimeFlag()
	{
		static std::vector<bool> b;
		return b;
	}
	 
	//�������2��ROI����ʱ�Ȳ�����
	static cv::Rect* ROI_2();

	/* static bool& Selection_enabled()
	{
		static bool flag = false;
		return flag;
	}
	*/
	static bool leftSelectionEnabled;






};



