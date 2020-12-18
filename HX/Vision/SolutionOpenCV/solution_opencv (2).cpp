#include "solution_opencv.h"

using namespace std;
using namespace cv;

solution_opencv::solution_opencv(const Mat& input)
{
	if (input.empty())
	{
		cout << "input is empty!" << endl;
	}
	src = input.clone();
}
solution_opencv::solution_opencv(const Mat& input, Rect roi)
{
	if (input.empty())
	{
		cout << "input is empty!" << endl;
	}
	src = input.clone()(roi);//??????
	m_roi = roi;
}

void solution_opencv::solution_preprocess( size_t PD_times, const size_t& G_ksize, const size_t& M_ksize)//Ԥ���� ������������ ��˹�˲�kSIZE, ��ֵ�˲�Ksize
{
	//������
	if (PD_times)
	{
		//PD_times *= 2;
		/*pyrDown(src, dst, Size((src.cols + 1) / 4, (src.rows + 1) / 4));*/
		//resize(src, dst, Size(src.cols / PD_times, src.rows / PD_times));
		resize(src, dst, Size(0, 0), 1 / pow(2, PD_times), 1 / pow(2, PD_times), INTER_AREA);
		

	}
	else
		dst = src;
	
	
	GaussianBlur(dst, dst, Size(G_ksize, G_ksize), 0);
	cvtColor(dst, dst, COLOR_BGR2GRAY);
	//medianBlur(dst, dst, M_ksize);
}

void solution_opencv::processed_to_threshold(const size_t& method, const size_t& thres)
{
	threshold(dst, threshold_img, thres, 255.0, method);
}


void solution_opencv::get_roi()
{
	//��ͨ����ͳ��
	Mat labels = Mat::zeros(threshold_img.size(), CV_32S);
	Mat stats, centroids;
	int num_labels = connectedComponentsWithStats(threshold_img, labels, stats, centroids, 8, CV_32S);


	printf("total labels : %d\n", (num_labels - 1));
	vector<Vec3b> colors(num_labels);

	// background color
	colors[0] = Vec3b(0, 0, 0);

	RNG rng(12345);//�����������

	// object color
	for (int i = 1; i < num_labels; i++) {
		colors[i] = Vec3b(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));//����ɫ��
	}

	// extract stats info

	int src_w = threshold_img.cols;
	int src_h = threshold_img.rows;



	for (int i = 1; i < num_labels; i++) {

		int cx = centroids.at<double>(i, 0);//����
		int cy = centroids.at<double>(i, 1);

		constexpr int offset = 40;

		int x = stats.at<int>(i, CC_STAT_LEFT) - offset;
		int y = stats.at<int>(i, CC_STAT_TOP) - offset;
		int w = stats.at<int>(i, CC_STAT_WIDTH) + offset;
		int h = stats.at<int>(i, CC_STAT_HEIGHT) + offset;
		int area = stats.at<int>(i, CC_STAT_AREA);

		if (area < 50000)//̫С����Ĳ�Ҫ
			continue;


		bool drawCircle_TOP = false, drawCircle_HEIGHT = false, drawCircle_LEFT = false, drawCircle_RIGHT = false;
		Mat thresImgClone = threshold_img.clone();

		double radiusVertical = h / 2 * 0.8, radiusHorizontal = w / 2 * 0.8;


		if (x <= 0)
		{
			x = stats.at<int>(i, CC_STAT_LEFT);
			circle(thresImgClone, Point(0, y + radiusVertical), radiusVertical, Scalar(0), -1);
		}
		/*namedWindow("ddd", WINDOW_FREERATIO);
		imshow("ddd", thresImgClone);
		waitKey();*/
		if (y <= 0)
		{
			y = stats.at<int>(i, CC_STAT_TOP);
			circle(thresImgClone, Point(x + radiusHorizontal, 0), radiusHorizontal, Scalar(0), -1);
		}
		/*imshow("ddd", thresImgClone);
		waitKey();*/
		if (x + w >= src_w)
		{
			w = stats.at<int>(i, CC_STAT_WIDTH);
			circle(thresImgClone, Point(src_w, y + radiusVertical), radiusVertical, Scalar(0), -1);

		}
		/*imshow("ddd", thresImgClone);
		waitKey();*/
		if (y + h >= src_h)
		{
			h = stats.at<int>(i, CC_STAT_HEIGHT);
			circle(thresImgClone, Point(x + radiusHorizontal, src_h), radiusHorizontal, Scalar(0), -1);

		}
		/*imshow("ddd", thresImgClone);
		waitKey();*/


		


		Rect rect(x, y, w, h);
		//roi = threshold_img(rect);
		roi = thresImgClone(rect);
		imshow("roi", roi);
	}
}


//�����
int ed2(const Point& lhs, const Point& rhs)
{
	return (lhs.x - rhs.x) * (lhs.x - rhs.x) + (lhs.y - rhs.y) * (lhs.y - rhs.y);
}

vector<Point> removeFromContour(const vector<Point>& contour, const vector<int>& defectsIdx)
{
	int minDist = INT_MAX;
	int startIdx = 0;
	int endIdx = 0;

	// �ҵ���������̵�ȱ�ݵ�
	for (int i = 0; i < defectsIdx.size(); ++i)
	{
		for (int j = i + 1; j < defectsIdx.size(); ++j)
		{
			float dist = ed2(contour[defectsIdx[i]], contour[defectsIdx[j]]);
			if (minDist > dist)
			{
				minDist = dist;
				startIdx = defectsIdx[i];
				endIdx = defectsIdx[j];
			}
		}
	}

	// Check if intervals are swapped
	if (startIdx <= endIdx)
	{
		int len1 = endIdx - startIdx;
		int len2 = contour.size() - endIdx + startIdx;
		if (len2 < len1)
		{
			swap(startIdx, endIdx);
		}
	}
	else
	{
		int len1 = startIdx - endIdx;
		int len2 = contour.size() - startIdx + endIdx;
		if (len1 < len2)
		{
			swap(startIdx, endIdx);
		}
	}

	// ȥ������Ҫ�ĵ�
	vector<Point> out;
	if (startIdx <= endIdx)
	{
		out.insert(out.end(), contour.begin(), contour.begin() + startIdx);
		out.insert(out.end(), contour.begin() + endIdx, contour.end());
	}
	else
	{
		out.insert(out.end(), contour.begin() + endIdx, contour.begin() + startIdx);
	}

	return out;
}

void solution_opencv::remove_plate_holder(const bool& is_left )
{
	if (roi.empty())
	{
		cout << "please complete roi extraction first! " << endl;
		exit(-1);
	}

	Mat se = getStructuringElement(MORPH_RECT, Size(20, 20), Point(-1, -1));

	Mat AMorph;

	morphologyEx(roi, AMorph, MORPH_CLOSE, se);
	//������
	imshow("after_morph", AMorph);

	//�Ҷ�תBGR
	Mat3b out;
	cvtColor(roi, out, COLOR_GRAY2BGR);

	vector<vector<Point>> contours;
	findContours(AMorph.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	vector<Point> pts = contours[0];
	//ѡ���contours
	for (const auto& v : contours)
	{
		if (v.size() > pts.size())
		{
			pts = v;
		}
	}

	//͹��
	vector<int> hullIdx;
	convexHull(pts, hullIdx, false);
	//͹ȱ�ݼ��
	vector<Vec4i> defects;
	convexityDefects(pts, hullIdx, defects);

	while (true)
	{
		// For debug
		Mat3b dbg;
		cvtColor(AMorph, dbg, COLOR_GRAY2BGR);

		vector<vector<Point>> tmp = { pts };
		drawContours(dbg, tmp, 0, Scalar(255, 127, 0));

		/*imshow("dbg", dbg);
		waitKey();*/

		vector<int> defectsIdx;
		for (const Vec4i& v : defects)
		{
			float depth = float(v[3]) / 256.f;
			if (depth > 10) //  ͨ��͹����ԭ����֮��ľ������ȱ��ɸ��
			{
				// �ҵ�ȱ�ݣ�����������Զ�ĵ��index������
				defectsIdx.push_back(v[2]);

				int startidx = v[0]; Point ptStart(pts[startidx]);
				int endidx = v[1]; Point ptEnd(pts[endidx]);
				int faridx = v[2]; Point ptFar(pts[faridx]);

				line(dbg, ptStart, ptEnd, Scalar(255, 0, 0), 1);
				line(dbg, ptStart, ptFar, Scalar(0, 255, 0), 1);
				line(dbg, ptEnd, ptFar, Scalar(0, 0, 255), 1);
				circle(dbg, ptFar, 4, Scalar(127, 127, 255), 2);
			}
		}

		//���ֻ��һ��ȱ�ݣ���ȥ��
		if (defectsIdx.size() < 2)
		{
			break;
		}
		//��ͻ�������ԭ������ɾ��
		pts = removeFromContour(pts, defectsIdx);
		convexHull(pts, hullIdx, false);
		convexityDefects(pts, hullIdx, defects);
	}


	// Draw result contour

	vector<Point> hull;
	convexHull(pts, hull, false);

	vector<vector<Point>> tmp = { hull };

	final_contour = Mat::zeros(out.rows, out.cols, CV_8U);

	drawContours(final_contour, tmp, 0, Scalar(255), 1);

	imshow("Result", final_contour);

}

void solution_opencv::GetContour()
{
	if (roi.empty())
	{
		cout << "please complete roi extraction first! " << endl;
		exit(-1);
	}

	Mat se = getStructuringElement(MORPH_RECT, Size(20, 20), Point(-1, -1));

	Mat AMorph;

	morphologyEx(roi, AMorph, MORPH_CLOSE, se);
	//������
	//imshow("after_morph", AMorph);

	//�Ҷ�תBGR
	Mat3b out;
	cvtColor(roi, out, COLOR_GRAY2BGR);

	vector<vector<Point>> contours;
	findContours(AMorph, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	vector<Point> pts;
	//for (const auto& v : contours)
	//{
	//	if (v.size() > 500)
	//	{
	//		pts = v;
	//		break;
	//	}
	//}

	//ѡ���contours
	size_t maxSize = contours[0].size();
	size_t j = 0;
	for (int i = 0; i < contours.size(); ++i)
	{
		if (maxSize < contours[i].size())
		{
			maxSize = contours[i].size();
			j = i;
		}
	}
	pts = contours[j];
	
	contours.clear();
	contours.push_back(pts);

	//// Draw result contour

	/*vector<Point> hull;
	convexHull(pts, hull, false);

	vector<vector<Point>> tmp = { hull };*/

	
	final_contour = Mat::zeros(out.rows, out.cols, CV_8UC1);
	drawContours(final_contour, contours, 0, Scalar(255), 1);

	//imshow("Result", final_contour);

}


struct LineAngle
{
	LS line;
	double angle;
	LineAngle(const LS& m_line, double& m_angle ) : line(m_line), angle(m_angle)
	{}
};


//double getDistance(cv::Point2d pointO, cv::Point2d pointA)
//{
//	double distance;
//	distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
//
//	return sqrtf(distance);
//}
void solution_opencv::get_lines()
{
	if (final_contour.empty())
	{
		cerr << "final_contour is empty! Did you run the preprocess ?" << endl;
		exit(-1);
	}

	EDLines testEDLines = EDLines(final_contour, 1.0, 50);//ֱ����ϣ���С����50����
	Mat line_img = testEDLines.getLineImage();//��ֱ��ͼ
	imshow("line_img", line_img);
	//waitKey();
	vector<LS> linePoints = testEDLines.getLines();//��ȡ����ֱ��
	
	
	vector<LineAngle> allLines;

	/*�趨�߽�ֵ��������еı߽��ϵ�ֱ��*/
	const size_t ratio = 100;
	const size_t& cols = final_contour.cols;
	const size_t& rows = final_contour.rows;

	const size_t xLowBound = cols / ratio, xHighBound = cols * (ratio - 1) / ratio;
	const size_t yLowBound = rows / ratio, yHighBound = rows * (ratio - 1) / ratio;
	////////////////////////////////////////////////////////////////


	//����ֱ�������飬ȥ������Ҫ��
	for ( LS& line : linePoints)
	{
		
		const double& sx = line.start.x; const double& sy = line.start.y;
		const double& ex = line.end.x;   const double& ey = line.end.y;
		if (sx <= xLowBound  && ex <= xLowBound || sx >= xHighBound && ex >= xHighBound)//���������ͼ��߽�
			continue;
		if (sy <= yLowBound && sy >= yHighBound || sy >= yHighBound && ey >= yHighBound)//�������ͼ��߽�
			continue;
		//ȥ������
		Point2d relative_cor(line.end - line.start);//���㷽������

		//double angle = (atan2( (double)relative_cor.y , (double)relative_cor.x));
		/*if (angle < 0)
			angle += PI;*/
		
		double angle = cv::fastAtan2(relative_cor.y, relative_cor.x);
		//�������������������е���������Ϊ0~180��, �����Ƕȵ�ͬʱ��ת��������
		if (angle > 180.0)
		{
			angle -= 180.0;
			swap(line.start, line.end);
		}

		allLines.emplace_back(line, angle);
		cout << "rex: " << relative_cor.x << "rey: " << relative_cor.y << endl;
		cout << " line start x: " << line.start.x << " line start y: " << line.start.y;
		cout << " line end   x: " << line.end.x << " line end   y: " << line.end.y  << " angle " << angle  << endl;
		
	}
	cout << endl;
	sort(allLines.begin(), allLines.end(),
		[](const LineAngle& l1, const LineAngle& l2)
	{
		return l1.angle < l2.angle;
	});
	vector<LineAngle> allLsNormalized;
	
	cout << allLines.size() << endl;
	

	/*for (const auto& itc : allLines)
	{
		cout << " line start x: " << itc.line.start.x << " line start y: " << itc.line.start.y;
		cout << " line end   x: " << itc.line.end.x << " line end   y: " << itc.line.end.y << " angle " << itc.angle << endl;
	}
	cout << endl;*/
	for (auto itc = allLines.begin(); itc != allLines.end() - 1; itc++)//���ظ���������һ��
	{
		LS& tmp1 = itc->line;
		LS& tmp2 = (itc + 1)->line;

		if (sqrt(ed2(tmp1.start, tmp2.start)) > 50.0)//�����ظ�����
		{
			continue;
		}

		tmp1.start = (tmp1.start + tmp2.start) / 2;
		tmp1.end =   (tmp1.end + tmp2.end) / 2;
		allLsNormalized.emplace_back(tmp1, itc->angle);
		
	}

	for (const auto& itc : allLsNormalized)
	{
		cout << " line start x: " << itc.line.start.x << " line start y: " << itc.line.start.y;
		cout << " line end   x: " << itc.line.end.x << " line end   y: " << itc.line.end.y << " angle "<<itc.angle <<endl;
	}


	//����ֱ�Ƕ�
	vector<pair<LineAngle, LineAngle>> rightAngles;

	for (auto i = allLsNormalized.begin(); i != allLsNormalized.end(); ++i)
	{
		for (auto j = i + 1; j != allLsNormalized.end(); ++j)
		{
			if (fabs(fabs(i->angle - j->angle) - 90) < 1.0)//����ʮ��
			{
				rightAngles.emplace_back(make_pair(*i, *j));
				++i; j = i + 1;
				if (i == allLsNormalized.end() || j == allLsNormalized.end())
					break;
			}
		}
	}


	
	
	/*if (rightAngles.size() > 1)
	{
		cout << "error!!!" << endl;
		return ;
	}*/
	size_t k = rightAngles.size();
	cvtColor(line_img, line_img, COLOR_GRAY2BGR);

	for (int i = 0; i < k; ++i)
	{
		LS& l1 = (rightAngles[i]).first.line;
		LS& l2 = (rightAngles[i]).second.line;


		line(line_img, l1.start, l1.end, Scalar(0, 255, 0), 1, LINE_AA, 0); // draw lines as green on image
		line(line_img, l2.start, l2.end, Scalar(0, 255, 0), 1, LINE_AA, 0); // draw lines as green on image

		Point2d crossPoint = SimpleMath::GetCrossPoint(l1.start, l1.end, l2.start, l2.end);

		circle(line_img, crossPoint, 3, Scalar(0, 0, 255), -1);

	/*	Point2d vector1 = l1.end - l1.start;
		Point2d vector2 = l2.end - l2.start;*/

		Point2d vector1 = l1.end - crossPoint;
		Point2d vector2 = l2.end - crossPoint;

		double t = SimpleMath::GetVectorAngle(vector1.x, vector1.y, vector2.x, vector2.y);

		/*double t = ((vector1.x * vector2.x) + (vector1.y * vector2.y)) / (sqrt(pow(vector1.x, 2) + pow(vector1.y, 2)) * sqrt(pow(vector2.x, 2) + pow(vector2.y, 2)));
		cout << "�����������ļн�Ϊ:" << acos(t) * (180 / PI) << "��" << endl;*/
		cout << "�����������ļн�Ϊ��" << t << "��" << endl;

	}

	


	//cout << "line.start.x" << vector1.x << " " << "line.start.y" << vector1.y << endl;
	//cout << "vertical" << endl;
	//cout << "line.start.x" << vector2.x << " " << "line.start.y" << vector2.y << endl;
	//cout << "vertical" << endl;
	namedWindow("ED", WINDOW_FREERATIO);

	
	imshow("ED", line_img);
}


void solution_opencv::timer_start()
{
	s_time = clock();
}

void solution_opencv::timer_stop_output(const string& str)
{
	e_time = clock();
	cout << str << "time has used: " << e_time - s_time << "ms!" << endl;
}

void solution_opencv::show_src()
{
	if (!src.empty())
		imshow("src", src);
	else
		cout << "dont show anything!" << endl;
	
}

void solution_opencv::show_dst()
{
	if (!dst.empty())
		imshow("dst", dst);
	else
		cout << "dst dont show anything!" << endl;
}

void solution_opencv::show_thres()
{
	if (!threshold_img.empty())
		imshow("threshold_img", threshold_img);
	else
		cout << "threshold_img dont show anything!" << endl;

}

void solution_opencv::show_roi()
{
	if (!roi.empty())
		imshow("roi", roi);
	else
		cout << "roi dont show anything!" << endl;
}
void solution_opencv::auto_detect_default()
{
	solution_preprocess(1, 3, 3);//Ԥ���� ������������ ��˹�˲�kSIZE, ��ֵ�˲�Ksize
	processed_to_threshold(THRESH_OTSU, 165);
	get_roi();
	remove_plate_holder();//ȥ��֧��
	get_lines();
}