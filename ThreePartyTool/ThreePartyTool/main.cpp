//
//  main.cpp
//  ThreePartyTool
//
//  Created by YUAN CHANG FAN on 2015/6/16.
//  Copyright (c) 2015年 Zappoint. All rights reserved.
//

#include <iostream>
#include "ZapLib/TessHelper.h"
#include "osdetect.h"
#include "ZapLib/Application.h"
#include "CardOcrResult.h"
#include "SerializationObjectArray.h"
#include "ZapLib/guitools.hpp"
#include "ZapLib/guitools_cv.h"
#include <sys/stat.h>
#include "ZapLib/SimpleStat.h"
#include "ZapLib/FilterOcrCardResult.h"


TessHelper tessHelper;
#define TEST_FAILED(x) \
LOG_ERROR("\n\n!!! %s Failed %s:%d", x, __FILE__,__LINE__);\
exit(-1);

std::string TESSDATA_PREFIX="/Users/jackf/Documents/androidstudio/ZappointPlus/subm_CvDetectCard/ZapOcr/sample";
using namespace cv;


SimpleStat simpleStat;
MY_APP(TestOcr, "/Users/jackf/Dropbox/Work/TestImage/0718");
MY_APP_INIT(TestOcr, intParam)
{
    //if(tessHelper.Init(TESSDATA_PREFIX.c_str(), "chi_tra") == false)
    if(tessHelper.Init(TESSDATA_PREFIX.c_str(), "eng") == false)
    {
        TEST_FAILED("Tess Init");
    }
    tessHelper.SetDefaultVariables();
    //Comment out this to do only one pass
    tessHelper.SetVariable("dopasses", "1");
    tesseract::TessBaseAPI* tess = tessHelper.GetTess();
    //tess->SetPageSegMode(tesseract::PSM_OSD_ONLY);
    tess->SetPageSegMode(tesseract::PSM_AUTO_OSD);
    
#if !MOBILE_PLATFORM
    // Control the level of std::out
    SetDebugLevel(1);
    // Control the color of std::out
    EnableXcodeColor(true);
    //TEST_ExtractStringWithTwoSeqPatterns();
#endif
    return 0;
}

MY_APP_PROCESSFILE(TestOcr, path)
{
    tesseract::TessBaseAPI* tess = tessHelper.GetTess();
    tess->SetOutputName(ReplaceExtension(path, ".txt").c_str());
    
    std::vector< std::vector<TextConfidence> > finalResult;
    Mat src0 = imread(path);
    ImShow("rects", src0);
    int key = waitKey(200);
    
    //tessHelper.RunCvOcr(path, finalResult);
    TIME_MEASURE_BEGIN
    tessHelper.RunCvOcr(src0, finalResult, 70);
    TIME_MEASURE_END("RunCvOcr")
    
    //bool getHOcrRet = tessHelper.SaveHOCRText(GetFileName(path, 1).c_str(), ReplaceExtension(path, "").c_str());
    //tessHelper.SaveThresholdImage(ReplaceExtension(path, ".png"));

    OSResults osr;
    tessHelper.GetOrientation(&osr);
    
    SimpleTestResult testResult(path, osr.best_result.orientation_id);
    testResult.mScores.push_back(osr.best_result.sconfidence);
    testResult.mScores.push_back(tessHelper.GetRecogTime());
    simpleStat.Add(testResult);
    
    std::vector<cv::Rect> rects;
    tessHelper.GetResultRects(TessHelper::OPTION_RECTS::Word, rects);
    tessHelper.Clear();
    
    int thickness = MAX(src0.cols / 400, 1);
    Mat tmp;
    resize(src0, tmp, Size(src0.cols * 9 / 5, src0.rows * 9 / 5));
    
    DrawRects(tmp, rects, Scalar(0,255,0), thickness);
    ImShow("rects", tmp);
    key = waitKey(200);
    return key;
}

MY_APP_END(TestOcr, intParam)
{
    simpleStat.DumpStat();
    return 0;
}

#if 0
MY_APP(BatchResize, "/Users/jackf/Downloads/TestImages/0617");
MY_APP_INIT(BatchResize, intParam)
{
    optionTestSubfolder = "raw";
    return 0;
}
MY_APP_PROCESSFILE(BatchResize, path)
{
    const int targetsizes[] = { 2200, 1900, 1600, 1300, 1000 };
    Mat src0 = imread(path);
    string outputdir, outputpath;
    char folder_name[64];
    for (int targetsize : targetsizes)
    {
        snprintf(folder_name, 64, "%d", targetsize);
        outputdir = CreateFolderPath(optionDBdirectory, folder_name);
        mkdir(outputdir.c_str(), 0755);
        
        Mat dst;
        ResizeToWorkingSize(src0, dst, targetsize);
        CreateFolderPath(outputdir, "");
        outputpath = outputdir + GetFileName(path, 1);
        imwrite(outputpath, dst);
    }
    return 0;
}
MY_APP_END(BatchResize, intParam)
{
    return 0;
}
#endif

//int main(int argc, const char * argv[]) {
//    zp::CardOcrResult ocrResult;
//
//    ocrResult.FullName = "JackFan";
//    ocrResult.Phones.push_back("0912123456");
//    ocrResult.Phones.push_back("0223456789");
//
//    SaveObjectToFile(ocrResult, "/Users/jackf/Documents/xcodePrj/Workspace1/ThreePartyTool/test1.txt", "OcrResult");
//
//    zp::CardOcrResult loadOcrResult;
//    LoadObjectFromFile("/Users/jackf/Documents/xcodePrj/Workspace1/ThreePartyTool/test.txt", loadOcrResult);
//    return 0;
//}
