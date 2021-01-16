#include "crossMarkDetector.hpp"

using namespace std;
using namespace cv;

Mat img;

int main(int argc, char* argv[]) {
    const char* imagename = "C:\\Users\\wsa\\Desktop\\1234.bmp";//�˴�Ϊ����ͼƬ·��
    FILE* stream1;
    freopen_s(&stream1, "linkTabel.txt", "r", stdin);
    
    VideoCapture capture(0);
    img = imread(imagename);
    capture >> img;
    
    cvtColor(img, img, COLOR_BGR2GRAY);
    img.convertTo(img, CV_32FC1); img = img / 255;

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
        
        //���̸���ȡ
        img.convertTo(img, CV_32FC1); img = img / 255;
        filter.feed(img);

        waitKey(1);
    } 
    /*
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
   
    waitKey(0);*/
    
    return 0;
}
