
#include "stdafx.h"
#include "sztx.h"
//using namespace std;
//using namespace cv;

static void help(void);
void shiftDFT(Mat &src);
void DFT(Mat &src, Mat &dst);
Mat show_spectrum_magnitude(Mat &complexImg, bool shift = false);
Mat createIdealfilter(Size size_of_filter, int D);
Mat createGaussianfilter(Size size_of_filter, double sigma_h, double sigma_v);
Mat createbutterworthfilter(Size size_of_filter, int D, int order);
Mat do_filter(Mat &dft_result, Mat filter);

int w_pic = int(1920 / 6), h_pic = int(1080 / 6);

int lpf(string option, string img){
	Mat src = imread(img,CV_LOAD_IMAGE_GRAYSCALE);
	Mat dst(h_pic * 3, w_pic * 3, CV_8UC3);
	Mat bgr[3];

	if (src.empty())
	{
		return -1;
	}
	if (option=="--help"||option=="--h")
	{
		help();
		return 0;
	}
	resize(src, src, Size(w_pic, h_pic));
	//imshow("src", src);
	split(src, bgr);
	drawtheblock(bgr[2], dst, Point(0, 0), "r");
	//drawtheblock(bgr[1], dst, Point(0, h_pic), "g");
	//drawtheblock(bgr[0], dst, Point(0, h_pic * 2), "b");
	imshow("final", dst);

	int type = 0;
	if (strcmp(option.c_str(),"--ideal")==0||strcmp(option.c_str(),"--i")==0)
	{
		;
	}
	else if (strcmp(option.c_str(), "--gaussian")==0||strcmp(option.c_str(),"--g")==0)
	{
		type = 1;
	}
	else if (strcmp(option.c_str(), "--butterworth")==0||strcmp(option.c_str(),"--b")==0)
	{
		type = 2;
	}
	else if (option=="--example")
	{
		type = 3;
	}
	else
	{
		cout << "invalid option,please check again" << endl;
		return -1;
	}

	Mat dft_result;
	DFT(src, dft_result);
	shiftDFT(dft_result);
	imshow("show the freq spectrum before filter", show_spectrum_magnitude(dft_result, false));

	switch (type)
	{
	case 0:
	{
		cout << "Use Ideal filter now,please enter the filter radius " << endl<<
			"if it is non positive,radius = min(src.rows,src.cols)/4" << endl;
		int tmp;
		cin >> tmp;
		Mat filter = createIdealfilter(dft_result.size(), tmp);
		imshow("dst", do_filter(dft_result,filter));
	}
	break;

	case 1:
	{
		cout << "use gaussian filter,please input the sigma for horizontal & vertical" << endl <<
			"if it is non positive,sigma=0.3*((ksize-1)*0.5 - 1) + 0.8" << endl
			<< "sigma should be less than min(dft_result.rows,dft_result.cols),which is : "
			<<min(dft_result.rows,dft_result.cols)<<endl;
		double sigma_h,sigma_v;
		cin >> sigma_h>>sigma_v;
		Mat filter = createGaussianfilter(dft_result.size(), sigma_h,sigma_v);
		imshow("dst", do_filter(dft_result, filter));
	}
	break;

	case 2:
	{
		cout << "use butterworth filter,please input the D_0 & order,the less the stronger" << endl
			<< "D_0 should less than min(src.rows,src.cols),which is :" << min(src.rows, src.cols) << endl;
		int d_0, order;
		cin >> d_0>>order;
		Mat filter = createbutterworthfilter(dft_result.size(), d_0, order);
		imshow("dst", do_filter(dft_result, filter));
	}
	break;
	default:
		break;
	}


	waitKey(0);
	return 0;
}

Mat do_filter(Mat &dft_result, Mat filter)
{
	mulSpectrums(dft_result, filter, dft_result, DFT_ROWS);

	Mat dst;
	//imshow("after filter the dft", show_spectrum_magnitude(dft_result, false));

	shiftDFT(dft_result);//֮ǰ�����������ڽ�������
	idft(dft_result, dst);
	normalize(dst, dst, 0, 1, CV_MINMAX);
	//imshow("after filter the dft",show_spectrum_magnitude(dft_result, false));
	Mat planes[2];
	split(dst, planes);
	dst = planes[0];
	return dst;
}

//D����Ƶ�ʾ���ԭ�㴦�ľ��룬
Mat createbutterworthfilter(Size size_of_filter, int D, int order)
{
	Mat filter(size_of_filter,CV_32F,Scalar::all(0));
	Point center(size_of_filter.width / 2, size_of_filter.height / 2);
	double radius;
	for (int i = 0; i < size_of_filter.height; i++)
	{
		for (int j = 0; j < size_of_filter.width; j++)
		{
			radius = sqrt(pow(j - center.x, 2.0) + pow(i - center.y, 2.0));
			filter.at<float>(i, j) = (float)(1 / 1 + pow(radius/D, 2 * order));
		}
	}
	Mat planes[2] = { filter, filter };
	merge(planes, 2, filter);
	return filter;
}

Mat createGaussianfilter(Size size_of_filter, double sigma_h,double sigma_v)
{
	double sig_h,sig_v;
	if (sigma_h<=0)
	{
		sig_h = 0.3*((size_of_filter.width - 1)*0.5 - 1) + 0.8;
	}
	else
	{
		sig_h = sigma_h;
	}
	if (sigma_h <= 0)
	{
		sig_v = 0.3*((size_of_filter.height - 1)*0.5 - 1) + 0.8;
	}
	else
	{
		sig_v = sigma_v;
	}
	Mat gaussianFilter(size_of_filter,CV_32F,Scalar::all(0));
	Mat filter_w = getGaussianKernel(size_of_filter.width, sig_h,CV_32F);
	Mat filter_h = getGaussianKernel(size_of_filter.height, sig_v,CV_32F);
	gaussianFilter = filter_h*filter_w.t();
	normalize(gaussianFilter, gaussianFilter, 0, 1, NORM_MINMAX);

	Mat planes[2] = { gaussianFilter, gaussianFilter };
	merge(planes, 2, gaussianFilter);
	return gaussianFilter;
}

Mat createIdealfilter(Size size_of_filter, int D)
{
	int radius = 0;
	if (D>0)
	{
		radius = D;
	}
	else
	{
		radius = min(size_of_filter.width, size_of_filter.height) / 4;
	}
	Mat filter(size_of_filter, CV_32F, Scalar::all(0));
	circle(filter, Point(size_of_filter.width / 2, size_of_filter.height / 2), radius, Scalar::all(1), -1);
	Mat to_merge[] = { filter, filter };
	merge(to_merge, 2, filter);
	return filter;
}

Mat show_spectrum_magnitude(Mat &complexImg,bool shift)
{
	Mat planes[2], dst;
	split(complexImg, planes);
	magnitude(planes[0], planes[1], dst);
	dst += Scalar::all(1);
	log(dst, dst);
	if (shift)
	{
		shiftDFT(dst);
	}
	normalize(dst, dst, 0, 1, CV_MINMAX);
	return dst;
}

void DFT(Mat &src, Mat &dst)
{
	copyMakeBorder(src, dst, 0, getOptimalDFTSize(src.rows) - src.rows,
		0, getOptimalDFTSize(src.cols) - src.cols, BORDER_CONSTANT, Scalar::all(0));
	Mat planes[] = { Mat_<float>(dst), Mat::zeros(dst.size(), CV_32F) };
	merge(planes, 2, dst);
	dft(dst, dst);
}

static void help(void){
	cout
		<< "[option][img_path -- default template.jpg]" << endl
		<< "options : " << endl
		<< "\t--example --ideal --gaussian --butterworth" << endl;
}

void shiftDFT(Mat &src)
{
	src = src(Rect(0, 0, src.cols&-2, src.rows&-2));
	int cx = src.cols / 2;
	int cy = src.rows / 2;
	Mat tmp, q0, q1, q2, q3;
	q0 = src(Rect(0, 0, cx, cy));
	q1 = src(Rect(cx, 0, cx, cy));
	q2 = src(Rect(cx, cy, cx, cy));
	q3 = src(Rect(0, cy, cx, cy));
	q0.copyTo(tmp);
	q2.copyTo(q0);
	tmp.copyTo(q2);
	q1.copyTo(tmp);
	q3.copyTo(q1);
	tmp.copyTo(q3);
}

