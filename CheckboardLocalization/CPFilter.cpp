//
//  CPFilter.cpp
//  Endoscopy
//
//  Created by 朱明珠 on 2020/11/24.
//

#include "Endoscopy.hpp"

bool CPF_omniCheck(Mat &Img, CPFilter_WorkSpace &FWS);      // 检查点检查
bool CPF_contourCheck(Mat &Img, CPFilter_WorkSpace &FWS);   // 边框检查
bool CPF_maskCheck(Mat &Img, CPFilter_WorkSpace &FWS);      // 模板检查
bool CPF_isLocMax(CPFilter_WorkSpace &FWS);                 // 局部极大检查

void CPFilter(Mat &Img, CPFilter_WorkSpace &FWS)
{
    assert(Img.type()==CV_32FC1);
    
    // 记录交叉点所用的指针
    int crossPtr = 0;
    
    // 全图搜索交叉点
    for (FWS.curPos.y=FWS.maskR; FWS.curPos.y<FWS.height-FWS.maskR; ++FWS.curPos.y) {
        for (FWS.curPos.x=FWS.maskR; FWS.curPos.x<FWS.width-FWS.maskR; ++FWS.curPos.x) {
            
            FWS.scoreMap.at<float>(FWS.curPos.y,FWS.curPos.x) = -1;
            // 三步检查
            if (!CPF_omniCheck(Img, FWS))       continue;
            if (!CPF_contourCheck(Img, FWS))    continue;
            if (!CPF_maskCheck(Img, FWS))       continue;
            // 交叉点落在的像素不得越界(超出图像或位于边框)
            if (FWS.crossCenterGlobalPos.x<FWS.maskR || FWS.crossCenterGlobalPos.x>=FWS.width-FWS.maskR)   continue;
            if (FWS.crossCenterGlobalPos.y<FWS.maskR || FWS.crossCenterGlobalPos.y>=FWS.height-FWS.maskR)   continue;
            // 交叉点落在的像素,不得已经有得分更高的交叉点
            if (FWS.correl<=FWS.scoreMap.at<float>(FWS.crossCenterGlobalPos.y,FWS.crossCenterGlobalPos.x)) continue;
            FWS.scoreMap.at<float>(FWS.crossCenterGlobalPos.y,FWS.crossCenterGlobalPos.x) = FWS.correl;
            // 记录交叉点信息
            FWS.crossPos[crossPtr] = FWS.crossCenterGlobalPos;
            FWS.cross[crossPtr] = FWS.crossCenterGlobal;
            FWS.blackLine[crossPtr] = FWS.crossLineEndsAngle[0];
            FWS.whiteLine[crossPtr] = FWS.crossLineEndsAngle[1];
            ++crossPtr;

            // 不超过最大交叉点数目
            if (crossPtr==FWS.maxNum) break;
        }
        if (crossPtr==FWS.maxNum) break;
    }
    // 非极大值抑制
    int valid = 0;
    for (int it=0;it<crossPtr;++it) {
        FWS.curPos = FWS.crossPos[it];
        if (CPF_isLocMax(FWS)) {
            FWS.crossPos[valid]  = FWS.crossPos[it];
            FWS.cross[valid]     = FWS.cross[it];
            FWS.blackLine[valid] = FWS.blackLine[it];
            FWS.whiteLine[valid] = FWS.whiteLine[it];
            ++valid;
        }
    }
    FWS.crossNum = valid;
    
    // display
    Mat ImgMark(FWS.height, FWS.width, CV_32FC3);
    cvtColor(Img, ImgMark, COLOR_GRAY2RGB);
    for (int it=0; it<FWS.crossNum; ++it) {
        circle(ImgMark, FWS.crossPos[it], 3, Scalar(0,1,0),1);
    }
    imshow("Show", ImgMark);
    //    imwrite("Img.bmp", 255*Img);
        imwrite("Mark_NMS.bmp", 255*ImgMark);
    //    imwrite("Img.bmp", 255*Img);
}

bool CPF_omniCheck(Mat &Img, CPFilter_WorkSpace &FWS)
{
    // 寄存到checkVal
    for (int it=0; it<8; ++it) {
        FWS.checkVal[it] = Img.at<float>(FWS.curPos.y+FWS.checkOff[it].y,FWS.curPos.x+FWS.checkOff[it].x);
    }
    // 判断间隙条件
    float maxGap = -1;
    for (int ia=0; ia<3; ++ia) {
        for (int ib=ia+1; ib<4; ++ib) {
            if (abs(FWS.checkVal[ia]-FWS.checkVal[ia+4])>FWS.T_maxInnerGap) continue;
            if (abs(FWS.checkVal[ib]-FWS.checkVal[ib+4])>FWS.T_maxInnerGap) continue;
            float tempGap = abs((FWS.checkVal[ia]+FWS.checkVal[ia+4])-(FWS.checkVal[ib]+FWS.checkVal[ib+4]))/2;
            if (tempGap>maxGap) {
                maxGap=tempGap;
            }
        }
    }
    if (maxGap<FWS.T_minOuterGap) return 0;
    return 1;
}

bool  CPF_contourCheck(Mat &Img, CPFilter_WorkSpace &FWS)
{
    // 计算边框均值
    float frameMean = 0;
    for (int it=0; it<FWS.maskFL; ++it) {
        FWS.frameVal[it] = Img.at<float>(FWS.curPos.y+FWS.frameOff[it].y,FWS.curPos.x+FWS.frameOff[it].x);
        frameMean = frameMean + FWS.frameVal[it];
    }
    frameMean = frameMean/FWS.maskFL;

    // 计算边框的二元颜色信息
    for (int it=0; it<FWS.maskFL; ++it) {
        if (FWS.frameVal[it]>frameMean) FWS.frameSgn[it] = 1;
        else                            FWS.frameSgn[it] = 0;
    }
    // 记录跳变信息,检查跳变次数
    FWS.jmpCount[0]=0;  FWS.jmpCount[1]=0;
    for (int ia=0; ia<FWS.maskFL; ++ia) {
        int ib;
        if (ia==0) ib=FWS.maskFL-1;
        else       ib=ia-1;
        if (FWS.frameSgn[ia]!=FWS.frameSgn[ib]) {
            if (FWS.jmpCount[FWS.frameSgn[ia]]<2) {
                FWS.jmpIdx[FWS.frameSgn[ia]][FWS.jmpCount[FWS.frameSgn[ia]]]=ia;
            }
            ++FWS.jmpCount[FWS.frameSgn[ia]];
        }
    }
    if (FWS.jmpCount[0]!=2 || FWS.jmpCount[1]!=2) return 0;
    // 计算跳变界线,检查亚像素交叉点
    for (int ia=0; ia<2; ++ia) {
        for (int ib=0; ib<2; ++ib) {
            int jmpA = FWS.jmpIdx[ia][ib];
            int jmpB = jmpA-1; if (jmpB<0) jmpB=FWS.maskFL-1;
    
            float dstA = abs(FWS.frameVal[jmpA]-frameMean);
            float dstB = abs(FWS.frameVal[jmpB]-frameMean);
            float dstSum = dstA+dstB;
            dstA = dstA/dstSum;
            dstB = dstB/dstSum;
    
            FWS.crossLineEnds[ia][ib].x = dstB*FWS.frameOff[jmpA].x+dstA*FWS.frameOff[jmpB].x;
            FWS.crossLineEnds[ia][ib].y = dstB*FWS.frameOff[jmpA].y+dstA*FWS.frameOff[jmpB].y;
        }
    }
    float center[3];
    FWS.crossLine[0][0] = FWS.crossLineEnds[0][0].y - FWS.crossLineEnds[0][1].y;
    FWS.crossLine[0][1] = FWS.crossLineEnds[0][1].x - FWS.crossLineEnds[0][0].x;
    FWS.crossLine[0][2] = FWS.crossLineEnds[0][0].x*FWS.crossLineEnds[0][1].y-
                  FWS.crossLineEnds[0][0].y*FWS.crossLineEnds[0][1].x;
    FWS.crossLine[1][0] = FWS.crossLineEnds[1][0].y - FWS.crossLineEnds[1][1].y;
    FWS.crossLine[1][1] = FWS.crossLineEnds[1][1].x - FWS.crossLineEnds[1][0].x;
    FWS.crossLine[1][2] = FWS.crossLineEnds[1][0].x*FWS.crossLineEnds[1][1].y-
                  FWS.crossLineEnds[1][0].y*FWS.crossLineEnds[1][1].x;
    center[0] = FWS.crossLine[0][1]*FWS.crossLine[1][2] - FWS.crossLine[0][2]*FWS.crossLine[1][1];
    center[1] = FWS.crossLine[0][2]*FWS.crossLine[1][0] - FWS.crossLine[0][0]*FWS.crossLine[1][2];
    center[2] = FWS.crossLine[0][0]*FWS.crossLine[1][1] - FWS.crossLine[0][1]*FWS.crossLine[1][0];
    FWS.crossCenter.x = center[0]/center[2];
    FWS.crossCenter.y = center[1]/center[2];
    
    if (abs(FWS.crossCenter.x)>FWS.T_maxCenterBias || abs(FWS.crossCenter.y)>FWS.T_maxCenterBias) return 0;

    FWS.crossCenterGlobal = Point2f(FWS.crossCenter.x+FWS.curPos.x+0.5, FWS.crossCenter.y+FWS.curPos.y+0.5);
    FWS.crossCenterGlobalPos.x = (int)FWS.crossCenterGlobal.x;
    FWS.crossCenterGlobalPos.y = (int)FWS.crossCenterGlobal.y;
    
    return 1;
}

bool CPF_maskCheck(Mat &Img, CPFilter_WorkSpace &FWS)
{
    // 计算跳变点的极坐标角度
    float temp;
    for (int ia=0; ia<2; ++ia) {
            temp = (atan2(FWS.crossLineEnds[ia][0].x-FWS.crossCenter.x,
                          FWS.crossCenter.y-FWS.crossLineEnds[ia][0].y))/CV_PI*180+45;
            if (temp<0) FWS.crossLineEndsAngle[ia] = 360 + temp;
            else        FWS.crossLineEndsAngle[ia] = temp;
    }
    // 对角度排序
    float angle[4], angleMark[4];
    if (FWS.crossLineEndsAngle[0]>FWS.crossLineEndsAngle[1]) {
        angle[0] = FWS.crossLineEndsAngle[1];
        angle[1] = FWS.crossLineEndsAngle[0];
        angleMark[0] = 1;
        angleMark[1] = 0;
    }
    else {
        angle[0] = FWS.crossLineEndsAngle[0];
        angle[1] = FWS.crossLineEndsAngle[1];
        angleMark[0] = 0;
        angleMark[1] = 1;
    }
    if ( (angle[1]-angle[0]<45) && (angle[1]-angle[0]>135) ) return 0; // 检查两类跳变角的夹角
    angle[2] = angle[0] + 180;  angleMark[2] = angleMark[0];
    angle[3] = angle[1] + 180;  angleMark[3] = angleMark[1];
    if (angle[2]>360) angle[2]=angle[2]-360;
    if (angle[3]>360) angle[3]=angle[3]-360;
    
    // 基于跳变信息,通过多重采样建立模板
    float ix=0.5-float(FWS.tmpMSAA.cols)/2-FWS.scaleMSAA*FWS.crossCenter.x;
    float iy;
    for (int x=0; x<FWS.tmpMSAA.cols; ++x, ++ix) {
        iy=FWS.scaleMSAA*FWS.crossCenter.y+float(FWS.tmpMSAA.rows)/2-0.5;
        for (int y=0; y<FWS.tmpMSAA.rows; ++y, --iy) {
            float temp = (atan2(ix, iy))/CV_PI*180+45;
            if (temp<0) temp = 360+temp;
            if (temp<angle[0] || temp>=angle[3])
                FWS.tmpMSAA.at<float>(y,x) = angleMark[3];
            if (temp<angle[1] && temp>=angle[0])
                FWS.tmpMSAA.at<float>(y,x) = angleMark[0];
            if (temp<angle[2] && temp>=angle[1])
                FWS.tmpMSAA.at<float>(y,x) = angleMark[1];
            if (temp<angle[3] && temp>=angle[2])
                FWS.tmpMSAA.at<float>(y,x) = angleMark[2];
        }
    }
    resize(FWS.tmpMSAA,FWS.tmp,Size(FWS.maskL,FWS.maskL),0,0,INTER_AREA);
    // 计算相关系数(得分)
    Mat crop(Img,Rect(FWS.curPos.x-FWS.maskR,FWS.curPos.y-FWS.maskR,FWS.maskL,FWS.maskL));
    Scalar meanTmp, meanCrop, stdTmp, stdCrop;
    meanStdDev(FWS.tmp, meanTmp, stdTmp);
    meanStdDev(crop, meanCrop, stdCrop);
    float covar = (FWS.tmp - meanTmp).dot(crop - meanCrop) / (FWS.maskL*FWS.maskL);
    FWS.correl = covar / (stdTmp[0] * stdCrop[0]);
    if (FWS.correl<FWS.T_minCorrelation) return 0;

//    if (fws.curpos.y>-1) {
//        mat cropshow, tmpshow;
//        normalize(crop, crop, 0, 1, norm_minmax);
//        cvtcolor(crop, cropshow, color_gray2rgb);
//        cvtcolor(fws.tmp, tmpshow, color_gray2rgb);
//
//        resize(cropshow,cropshow,size(50*crop.rows,50*crop.cols),0,0,inter_nearest);
//        resize(tmpshow,tmpshow,size(50*crop.rows,50*crop.cols),0,0,inter_nearest);
//
//        point ba(50*fws.crosslineends[0][0].x+175,50*fws.crosslineends[0][0].y+175);
//        point bb(50*fws.crosslineends[0][1].x+175,50*fws.crosslineends[0][1].y+175);
//        line(cropshow, ba, bb, scalar(0,0,1));
//        line(tmpshow, ba, bb, scalar(0,0,1));
//
//        point wa(50*fws.crosslineends[1][0].x+175,50*fws.crosslineends[1][0].y+175);
//        point wb(50*fws.crosslineends[1][1].x+175,50*fws.crosslineends[1][1].y+175);
//        line(cropshow, wa, wb, scalar(0,1,0));
//        line(tmpshow, wa, wb, scalar(0,1,0));
//
//        imshow("crop.bmp",cropshow);
//        imshow("tmp.bmp",tmpshow);
//
//        waitkey(1);
//        point a = fws.curpos;
//        float b = correl;
//        return 1;
//    }

    return 1;
}

bool CPF_isLocMax(CPFilter_WorkSpace &FWS)
{
    // 必须大于上方像素
    for (int ix=-FWS.maskR; ix<=FWS.maskR; ++ix)
        for (int iy=-FWS.maskR; iy<0; ++iy)
            if (FWS.scoreMap.at<float>(FWS.curPos.y,FWS.curPos.x) <= FWS.scoreMap.at<float>(FWS.curPos.y+iy,FWS.curPos.x+ix))
                return 0;
    // 必须大于左侧像素
    for (int ix=-FWS.maskR; ix<0; ++ix)
            if (FWS.scoreMap.at<float>(FWS.curPos.y,FWS.curPos.x) <= FWS.scoreMap.at<float>(FWS.curPos.y,FWS.curPos.x+ix))
                return 0;
    // 必须不小于右侧像素
    for (int ix=1; ix<=FWS.maskR; ++ix)
            if (FWS.scoreMap.at<float>(FWS.curPos.y,FWS.curPos.x) < FWS.scoreMap.at<float>(FWS.curPos.y,FWS.curPos.x+ix))
                return 0;
    // 必须不小于下方像素
    for (int ix=-FWS.maskR; ix<=FWS.maskR; ++ix)
        for (int iy=1; iy<=FWS.maskR; ++iy)
            if (FWS.scoreMap.at<float>(FWS.curPos.y,FWS.curPos.x) < FWS.scoreMap.at<float>(FWS.curPos.y+iy,FWS.curPos.x+ix))
                return 0;
    return 1;
}
