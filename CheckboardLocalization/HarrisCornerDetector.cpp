#include <cstdio>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include <opencv2/highgui/highgui_c.h>
#include "HarrisCornerDetector.h"

using namespace std;
using namespace cv;

void CornerHarris::on_CornerHarris(Mat dst, Mat img, void*)
{
    Mat dstImage;//Ŀ��ͼ  
    Mat normImage;//��һ�����ͼ  
    Mat scaledImage;//���Ա任��İ�λ�޷������͵�ͼ  

    //���㵱ǰ��Ҫ��ʾ������ͼ���������һ�ε��ô˺���ʱ���ǵ�ֵ  
    dstImage = Mat::zeros(dst.size(), CV_32FC1);
    g_srcImage1 = img.clone();

    //���нǵ���  
    //������������ʾ�����С�����ĸ�������ʾSobel���ӿ׾���С�������������ʾHarris����
    cornerHarris(dst, dstImage, 2, 3, 0.04, BORDER_DEFAULT);

    // ��һ����ת��  
    normalize(dstImage, normImage, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
    convertScaleAbs(normImage, scaledImage);//����һ�����ͼ���Ա任��8λ�޷�������   

    // ����⵽�ģ��ҷ�����ֵ�����Ľǵ���Ƴ���  
    for (int j = 0; j < normImage.rows; j++)
    {
        for (int i = 0; i < normImage.cols; i++)
        {
            //Mat::at<float>(j,i)��ȡ����ֵ��������ֵ�Ƚ�
            if ((int)normImage.at<float>(j, i) > thresh + 80)
            {
                circle(g_srcImage1, Point(i, j), 5, Scalar(10, 10, 255), 2, 8, 0);
                circle(scaledImage, Point(i, j), 5, Scalar(0, 10, 255), 2, 8, 0);
            }
        }
    }

    imshow("�ǵ���", g_srcImage1);
    imshow("�ǵ���2", scaledImage);
}
