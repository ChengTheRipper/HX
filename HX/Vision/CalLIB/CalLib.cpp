#include "CalLib.h"







bool CalLib::load_distortion_file()
{
	FileStorage fs(configFileName, FileStorage::READ);
	if (!fs.isOpened())
	{
		cout << "Cant find " << configFileName << endl;
		cout << "Have you run the rectification yet?" << endl;
		return false;
	}
	
	distortion_coefficients = Mat();
	camera_matrix = Mat(3, 3, CV_32FC1);
	fs["intrinsic_matrix"] >> camera_matrix;
	fs["distortion_coeffs"] >> distortion_coefficients;
	fs.release();

	cout << camera_matrix;

	return true;
}
bool CalLib::GetPnPParam(vector<Point2f> imagePoints, vector<Point3f> objectPoints)
{
	if (camera_matrix.empty() || distortion_coefficients.empty())
	{
		bool flag = load_distortion_file();
		if (flag == false)
			return false;
	}
	rvec = Mat(1, 3, cv::DataType<double>::type);
	tvec = Mat(1, 3, cv::DataType<double>::type);
	rotationMatrix = Mat(3, 3, cv::DataType<double>::type);

	

	solvePnP(objectPoints, imagePoints, camera_matrix, distortion_coefficients, rvec, tvec);

	rotationMatrixInv = rotationMatrix.inv();
	cameraMatrixInv = camera_matrix.inv();

	leftSideMat  = rotationMatrixInv * cameraMatrixInv;
	rightSideMat = rotationMatrixInv * tvec;
	return true;
}
bool CalLib::RectifyImg(Mat& input, Mat& output)
{
	if (camera_matrix.empty())
	{
		if (!load_distortion_file())
		{
			return false;
		}
	}
	output = input.clone();

	undistort(input, output, camera_matrix, distortion_coefficients);
	
	return true;
}

double getDistance(cv::Point2d pointO, cv::Point2d pointA)
{
	double distance;
	distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);

	return sqrtf(distance);
}


bool CalLib::initPnP(vector<Point2f> imagePoints, vector<Point3f> objectPoints)
{
	if (bisRectified)
	{
		GetPnPParam(imagePoints, objectPoints);
	}

	return true;
}

bool CalLib::GetRealWorldCoordinate(const cv::Point2d& imagePoint, cv::Point3d& outputPoint)
{
	Mat uvPoint = (Mat_<double>(3, 1) << imagePoint.x, imagePoint.y, 1);

	constexpr size_t height = 0;//���е�ʵ��ƽ�涼Ĭ����ͬһƽ��

	leftSideMat *= uvPoint;

	double s = (height + rightSideMat.at<double>(2, 0) / leftSideMat.at<double>(2, 0));

	cout << "P = " << rotationMatrixInv * (s * cameraMatrixInv * uvPoint - tvec) << std::endl;

	return true;
}

bool CalLib::cal_lib_init()
{
	//ִ�����պ�������ΰ��
	//ʹ��glob��ȡ�ļ����µ������ļ�
	glob(CALLIB_PIC_DIR, filename);

	if (filename.empty())
	{
		cout << "cant find any chessboard, retification failed!" << endl;
		return false;
	}

	for (size_t i = 0; i < filename.size(); ++i)
	{
		cout << filename[i] << endl;
		Mat src = imread(filename[i]);
		if (!src.data)
			cerr << "Problem loading image!!!" << endl;
		VecMat.push_back(src);
	}

	

	return true;
}


bool CalLib::cal_intrinsic_distortion()
{
	
	if (!cal_lib_init())
		return false;
	//���̸�Ĵ�С��ÿ��/�еĽǵ����
	const Size boardsize = Size(CORNERS_LS, CORNERS_SS);
	//��¼ÿ��ͼ�еĽǵ�
	vector<vector<Point2f>> cor_pixpos_vec;
	//��¼����ͼ���еĽǵ��ܺ�
	int pic_cor_count = 0;
	//���̸�ͼ��
	Mat srcImg;
	//ͼ��ĸ���
	int image_count = 0;


	for (int i = 0; i < TOTAL_PIC; i++)
	{
		//��¼ÿ��ͼ�������
		//string Img_files = string("Cam_cal/left") +to_string(i) + string(".jpg");
		//��ȡͼ������ȡʧ��������ʾ

		srcImg = VecMat[i];
		/*if (!srcImg.data)
		{
			cout << "��ȡ" <<  << "ʧ�ܣ���ȷ��Ŀ¼���Ƿ���ڸ�ͼƬ��" << endl;
			waitKey();
			continue;
		}*/

		//ת��Ϊ�Ҷ�ͼ��
		Mat grayImg;
		cvtColor(srcImg, grayImg, COLOR_BGR2GRAY);

		//�������̸�ͼ��ǵ�
		vector<Point2f> corners;
		bool cornerfound = findChessboardCorners(grayImg, boardsize, corners, CALIB_CB_ADAPTIVE_THRESH);

		{
			//���������һ��ͼ�����ǵ�֮������ؾ����ƶ��ڲξ����Ƿ���ȷ
			if (i == 0)
			{
				Point2f &p1 = corners[0], &p2 = corners[1];
				Point2f& p3 = corners[2], & p4 = corners[3];
				cout << "point1 x " << p1.x << "point1 y" << p1.y << endl;
				cout << "point2 x " << p2.x << "point2 y" << p2.y << endl;
				cout << "pixel distance" << getDistance(p1, p2) << endl;
				cout << "pixel distance2" << getDistance(p1, p2) << endl;
			}
		}


		if (!cornerfound)
		{
			cout << "�� " << i + 1 <<" ��ͼƬ�ǵ����ʧ��" << endl;
			continue;
		}
		Mat tmp = grayImg.clone();
		drawChessboardCorners(tmp, boardsize, corners, true);
		stringstream s; string fileName;
		s << i << ".bmp";
		s >> fileName;
		imwrite(fileName, tmp);
		//�������̸�ͼ��ǵ�Ѱ�������ؽǵ�
		cornerSubPix(grayImg, corners, Size(5, 5), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 30, 0.001));

		//�ۼƽǵ�ĸ���
		pic_cor_count += corners.size();
		//������ͼ��Ľǵ���ӵ��б���
		cor_pixpos_vec.push_back(corners);
		//ͼ�����+1
		image_count++;
		cout << "���ͼƬ " << i <<" ��Ϣ�ռ�" << endl;
	}
	Size square_size = Size(RWSSL, RWSSL);

	vector<Point3f> tempPointset;
	for (int h = 0; h < boardsize.height; h++)
	{
		for (int w = 0; w < boardsize.width; w++)
		{
			Point3f tempPoint;
			tempPoint.x = w * square_size.width;
			tempPoint.y = h * square_size.height;
			tempPoint.z = 0;
			tempPointset.push_back(tempPoint);
		}
	}
	vector<vector<Point3f>> object_Points(image_count, tempPointset);//�����ҵı궨�岻�䣬�������е�ͼƬ����һ����

	//��¼���
	//   fx,  0, cx
	//    0, fy, cy
	//    0,  0, 1
	//�ڲξ���Ĳ���
	//Mat intrinsic_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
	Mat intrinsic_matrix = Mat::eye(3, 3, CV_64F);
	//��¼�������Ĳ���
	Mat distortion_coeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));
	//��¼ÿ��ͼ�����ת����
	vector<cv::Mat> rotation_vectors;
	//��¼ÿ��ͼ���λ�ƾ���
	vector<cv::Mat> translation_vectors;

	calibrateCamera(object_Points, cor_pixpos_vec, srcImg.size(), intrinsic_matrix, distortion_coeffs, rotation_vectors, translation_vectors, 0);

	double total_err = 0.0;
	for (int i = 0; i < image_count; i++)
	{
		vector<Point2f> image_points2;//��������Ľǵ���������

		//��ȡÿ��ͼ����������ϵ�µĽǵ�����
		vector<Point3f> tempPointSet = object_Points[i];
		//����������ڲκ���μ���ó�ͼ������ϵ�µĽǵ�����
		projectPoints(tempPointSet, rotation_vectors[i], translation_vectors[i], intrinsic_matrix, distortion_coeffs, image_points2);

		//��¼��ʵͼ������ϵ�µĽǵ�����
		vector<Point2f> tempImagePoint = cor_pixpos_vec[i];

		//������ó���ͼ������ϵ�µĽǵ� �� ��ʵͼ������ϵ�µĽǵ�����ֱ��������ͼ����
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		Mat image_points2Mat = Mat(1, image_points2.size(), CV_32FC2);
		for (size_t i = 0; i != tempImagePoint.size(); i++)
		{
			image_points2Mat.at<Vec2f>(0, i) = Vec2f(image_points2[i].x, image_points2[i].y);
			tempImagePointMat.at<Vec2f>(0, i) = Vec2f(tempImagePoint[i].x, tempImagePoint[i].y);
		}
		//��������ͼ����������
		double err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
		//�����������
		total_err += err /= (CORNERS_LS * CORNERS_SS);//point_counts[0]
		cout << "��" << i << "��ͼ���ƽ����" << err << "����" << endl;
	}
	cout << "����ƽ����" << total_err / image_count << "����" << endl << endl;


	//������궨����д��xml�ļ���
	FileStorage fs1("calibration.xml", FileStorage::WRITE);
	fs1 << "intrinsic_matrix" << intrinsic_matrix;
	fs1 << "distortion_coeffs" << distortion_coeffs;

	camera_matrix = intrinsic_matrix.clone();
	distortion_coefficients = distortion_coeffs.clone();

	fs1.release();
}
