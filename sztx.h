
#include "stdafx.h"

using namespace cv;
using namespace std;
extern void diffcopy(Mat &src, Mat &dst);
extern void drawtheblock(Mat &src, Mat &dst, Point anchor, String str);
void rgb2cmyk(Mat &src, vector<Mat> &cmyk);
void rgb2cmyk2(Mat &src, Mat &cmyk);
Mat histgram(Mat &src);
extern Mat histgram3c(Mat &src,Size output_size);
Mat equalize3c(Mat &src);