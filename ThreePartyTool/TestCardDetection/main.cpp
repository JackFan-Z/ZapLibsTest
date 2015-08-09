//
//  main.cpp
//  TestCardDetection
//
//  Created by YUAN CHANG FAN on 2015/7/31.
//  Copyright (c) 2015年 Zappoint. All rights reserved.
//

#include <iostream>
#include "ZapLib/Application.h"
#include "ZapLib/guitools.hpp"
#include "ZapLib/guitools_cv.h"
#include "ZapLib/CardTrackerFeature.h"
#include "ZapLib/preprocessimg.hpp"
static CardTracker* gTheCardTracker = NULL;

using namespace std;
using namespace cv;

string dir = "/Users/jackf/Downloads/TestImages/0604/raw";
//string dir = "/Users/jackf/Dropbox/Work/TestImage/0731";
MY_APP(TestCardDetection, dir);

MY_APP_INIT(TestCardDetection, intParam)
{
    gTheCardTracker = new CardTracker();
    gTheCardTracker->Init(NULL);
#if !MOBILE_PLATFORM
    // Control the level of std::out
    SetDebugLevel(3);
    // Control the color of std::out
    //EnableXcodeColor(true);
#endif
    return 0;
}

MY_APP_PROCESSFILE(TestCardDetection, path)
{
    Mat src0 = imread(path);
    std::vector<cv::Point2f> Corners;
    bool ret = gTheCardTracker->ProcessFrame(&src0, Corners);
    if (ret == false)
    {
        LOG_ERROR("Fail to detect the card");
        return -1;
    }
    int fullFOVSize = 3200;
    int degreeFOV = 30;
    Mat quadImg;
    ret = GetQuadMat( src0, quadImg, Corners, fullFOVSize, degreeFOV);
    Mat cutImage;
    CutOutCardBorder( quadImg, cutImage);
    ImShow("Result", cutImage);
    int key;
    key = waitKey(100);
    return key;
}

MY_APP_END(TestCardDetection, intParam)
{
    if (gTheCardTracker != NULL)
    {
        delete gTheCardTracker;
        gTheCardTracker = NULL;
    }
    return 0;
}