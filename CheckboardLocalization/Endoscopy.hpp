//
//  Endoscopy.hpp
//  Endoscopy
//
//  Created by 朱明珠 on 2020/11/19.
//

#ifndef Endoscopy_hpp
#define Endoscopy_hpp

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <stdio.h>

using namespace cv;
using namespace std;

struct CPFilter_WorkSpace {

    int height, width;        // 图像的高宽
    int maskR;
    int maskL, maskFL; // 掩模的半边长、边长和周长(像素数)
    int maxNum;               // 检测交叉点的上限(非极大值抑制之前)(影响内存预分配)
    Point curPos;             // 当前检查点(寄存)

    // for step 1. checkpoint check,检查点检查
    Point checkOff[8];        // 掩模8个方向上的检查点相对中心像素的偏移(从上方开始，顺时针到左上)
    float checkVal[8];        // 掩模8个方向上的检查点的取值(寄存)

    // step 2. frame check,边框检查
    Point* frameOff;          // 掩模轮廓像素相对中心像素的偏移
    float* frameVal;          // 掩模轮廓上的取值(寄存)
    bool* frameSgn;          // 掩模轮廓上的取值的极性(0-黑、1-白)(寄存)
    int jmpCount[2], jmpIdx[2][2];  // 用于计算掩模轮廓上的跳变像素(寄存)

    Point2f crossLineEnds[2][2];    // 例如 crossLineEnds[0][1] 表示顺时针跳黑界线的第二个跳变点(curPos为原点,x-y向右-下)(寄存)
    float   crossLine[2][3];        // 跳黑/白边界的直线参数(curPos为原点,x-y向右-下)(寄存)
    Point2f crossCenter;            // 跳黑/白边界的交点(curPos为原点,x-y向右-下)(寄存)
    Point2f crossCenterGlobal;      // 跳黑/白边界的亚像素交点(图像坐标系,x-y向右-下)(寄存)
    Point   crossCenterGlobalPos;   // 跳黑/白边界的交点(图像坐标系,x-y向右-下)(寄存)
    float   crossLineEndsAngle[2];  // 第一个黑/白色跳变点的极坐标角度(crossCenter为原点,左上为0°,顺时针)(寄存)

    // step 3. mask check,模板检查
    int scaleMSAA;              // 多重采样倍率
    Mat tmp, tmpMSAA;           // 模板,与多重采样模板(寄存)
    float correl;               // 当前区域与所生产模板的相关系数(寄存)

    // 算法阈值
    float T_maxInnerGap = 0.2;      // step 1. 中检查点对内部的最大差值
    float T_minOuterGap = 0.25;     // step 1. 中检查点对之间的最小差值
    float T_maxCenterBias = 1;      // step 2. 中亚像素交叉点的最大偏移(x-y独立计算)
    float T_minCorrelation = 0.85;  // step 3. 中交叉点与所生成模板相关系数的最小值,同时也是交叉点的最终得分

    // 检测结果记录
    Mat scoreMap;       // 得分图
    int crossNum;       // 检测到的交叉点总数
    Point2f* cross;     // 检测到的交叉点的亚像素坐标
    Point* crossPos;  // 检测到的交叉点的整数坐标
    float* score;     // 检测到的交叉点的得分
    float* blackLine; // 检测到的交叉点的跳黑界线的朝向（左上为0°,顺时针)
    float* whiteLine; // 检测到的交叉点的跳白界线的朝向（左上为0°,顺时针)
};

void CPFilter(Mat& Img, CPFilter_WorkSpace& FWS);

#endif /* Endoscopy_hpp */
