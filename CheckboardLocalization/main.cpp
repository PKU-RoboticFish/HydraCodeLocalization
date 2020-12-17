#include "CircleDetector.hpp"
#include "crossMarkDetector.hpp"
#include "CalcMatrix.h"
#include <vector>

using namespace std;
using namespace cv;

Mat img;

int main(int argc, char* argv[]) {
    const char* imagename = "C:\\Users\\wsa\\Desktop\\123.bmp";//�˴�Ϊ����ͼƬ·��
    /*
    VideoCapture capture(0);
    img = imread(imagename);
    capture >> img;
    
    CircleDetect CD;

    crossMarkDetectorParams Dparams;
    Dparams.height = img.rows;
    Dparams.width = img.cols;
    crossPointResponderParams Rparams;
    crossMarkDetector filter(Dparams, Rparams);
    
    while (true){
        capture >> img;

        if (img.empty()) {
            fprintf(stderr, "Can not load image %s\n", imagename);
            return -1;
        }
        
        cvtColor(img, img, COLOR_BGR2GRAY);
        
        //Բ�μ��
        
        //CD.DetectCircle(img);
        
        //���̸���ȡ
        img.convertTo(img, CV_32FC1); img = img / 255;
        filter.feed(img);

        //ͳ����������
        Matrix chess{};
        chess.CalcMat(FWS);

        waitKey(1);
    }*/
    
    
    // ������ͼƬ
    Mat img = imread(imagename);
    cvtColor(img, img, COLOR_BGR2GRAY);
    img.convertTo(img, CV_32FC1); img = img / 255;

    crossMarkDetectorParams Dparams;
    Dparams.height = img.rows;
    Dparams.width = img.cols;
    crossPointResponderParams Rparams;

    crossMarkDetector filter(Dparams, Rparams);
    filter.feed(img);

    //Matrix chess{};
    //chess.CalcMat(FWS);

    waitKey(10000);

    return 0;
}
