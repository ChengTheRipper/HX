#pragma once

#include "../opencv_include.h"
#include "../STL_include.h"

using namespace cv;
using namespace std;


const string CALLIB_PIC_DIR("Cam_Cal");
const string PREFIX("CAL");
class CalLib
{
public:
	//total picture number , ���߽ǵ������� �̱߽ǵ������� ʵ�ʷ���߳�
	CalLib(size_t m_TP = 20 , size_t m_CLS = 11, size_t m_CSS = 8, size_t rwssl = 30, String m_configFileName = "calibration.xml") :
		TOTAL_PIC(m_TP), CORNERS_LS(m_CLS), CORNERS_SS(m_CSS), RWSSL(rwssl), configFileName(m_configFileName)
	{
		bisRectified = load_distortion_file();
	};
	//�Ƿ��Ѿ�����
	bool bisRectified;
	
	bool cal_intrinsic_distortion(); //�����������

	//���������ͼ��
	void CalLibGrabPic();

	//��ȡ�ڲξ���ͻ���ϵ������ͼƬ���н���, ���棬������ʱ�ϳ�
	bool RectifyImg(Mat& input, Mat& output);
	
	//��ʼ��Pnp
	bool initPnP(vector<Point2f> imagePoints, vector<Point3f> objectPoints);
	//��ȡʵ��λ������
	bool GetRealWorldCoordinate(const cv::Point2d& imagePoint, cv::Point3d& outputPoint); //��ȡʵ����������


private:
	//��ȡ���еĻ���ͼƬ
	bool cal_lib_init();
	//�������������ڲξ���
	bool load_distortion_file();

	//�����Ӧ�Ծ����Լ���Ӧ�ĸ�������
	bool GetPnPParam(vector<Point2f> imagePoints, vector<Point3f> objectPoints);



	const size_t TOTAL_PIC;
	const size_t CORNERS_LS;
	const size_t CORNERS_SS;
	const size_t RWSSL;

	//a vector string that stores all the pic path
	vector<string>     filename;
	vector<cv::Mat>	   VecMat;
	string configFileName;

	//�ڲξ���
	Mat camera_matrix;
	//�������
	Mat distortion_coefficients;
	//��ξ�������
	cv::Mat rvec;//��ת����
	cv::Mat tvec;//ƽ������
	cv::Mat rotationMatrix;   //ͨ�������������
	cv::Mat leftSideMat;      //��ʽ��������
	cv::Mat rightSideMat;     //��ʽ�Ҳ������
	cv::Mat rotationMatrixInv;//��ת���������
	cv::Mat cameraMatrixInv;  //�ڲξ��������

	

	void take_picture();

	
};

