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
#include "ZapLib/preprocessimg.hpp"


TessHelper tessHelper;
#define TEST_FAILED(x) \
LOG_ERROR("\n\n!!! %s Failed %s:%d", x, __FILE__,__LINE__);\
exit(-1);

std::string TESSDATA_PREFIX="/Users/jackf/Documents/androidstudio/ZappointPlus/subm_CvDetectCard/ZapOcr/sample";
using namespace cv;


SimpleStat simpleStat;
//MY_APP(TestOcr, "/Users/jackf/Downloads/TestImages/0810_CardOcr");
//MY_APP(TestOcr, "/Users/jackf/Dropbox/Work/TestImage/0810");
MY_APP(TestOcr, "/Users/jackf/Dropbox/Work/TestImage/0718");
//MY_APP(TestOcr, "/Users/jackf/Downloads/TestImages/0810_OSD/0718");


bool progressJavaCallback(void* progress_this, int progress, int left, int right,
                          int top, int bottom)
{
    TessHelper* tessHelper = (TessHelper*) progress_this;
    return true;
}
bool cancelFunc(void* cancel_this, int words) {
    //LOG_ERROR("cancelFunc");
    return false;
}

MY_APP_INIT(TestOcr, intParam)
{
    //if(tessHelper.Init(TESSDATA_PREFIX.c_str(), "chi_tra") == false)
    if(tessHelper.Init(TESSDATA_PREFIX.c_str(), "eng") == false)
    {
        TEST_FAILED("Tess Init");
    }
    tessHelper.SetDefaultVariables();
    
    tesseract::TessBaseAPI* tess = tessHelper.GetTess();
    //tess->SetPageSegMode(tesseract::PSM_OSD_ONLY);
    tess->SetPageSegMode(tesseract::PSM_AUTO_OSD);

    //Comment out this to do only one pass
    tessHelper.SetVariable("dopasses", "1");
    //tessHelper.SetVariable("textord_debug_images", "true");
    //tessHelper.SetVariable("tessedit_dump_pageseg_images", "true");
    //tessHelper.SetVariable("textord_tabfind_show_initialtabs", "true");
    //tessHelper.SetVariable("textord_tabfind_show_partitions", "1");
    //tessHelper.SetVariable("textord_tabfind_show_initial_partitions", "true");
    //tessHelper.SetVariable("textord_debug_printable", "true");
    //tessHelper.SetVariable("textord_tabfind_find_tables", "false");
    //tessHelper.SetVariable("textord_tabfind_show_columns", "true");
    //tessHelper.SetVariable("textord_tabfind_show_strokewidths", "1" );
    //tessHelper.SetVariable("textord_debug_tabfind", "1");
    tessHelper.SetVariable("textord_tabfind_merge_blocks", "false");
    //tessHelper.SetVariable("language_model_penalty_non_dict_word", "2.9");
    //tessHelper.SetVariable("language_model_penalty_non_freq_dict_word", "2.9");
    //tessHelper.SetVariable("enable_new_segsearch", "true");
    STRING value;
    tess->GetVariableAsString("language_model_penalty_non_dict_word", &value);
    LOG_D("language_model_penalty_non_dict_word = %s", value.c_str());
    tess->GetVariableAsString("language_model_penalty_non_freq_dict_word", &value);
    LOG_D("language_model_penalty_non_freq_dict_word = %s", value.c_str());
    tess->GetVariableAsString("enable_new_segsearch", &value);
    LOG_D("enable_new_segsearch=%s", value.c_str());
    
    tessHelper.SetProgressCallback(progressJavaCallback);
    tessHelper.SetCancelCallback(cancelFunc);
    
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
    if (path.find(".png") != string::npos)
    {
        LOG_D("The file is skipped");
        return 0;
    }
    
    tesseract::TessBaseAPI* tess = tessHelper.GetTess();
    tess->SetOutputName(ReplaceExtension(path, ".txt").c_str());
    
    std::vector< std::vector<TextConfidence> > finalResult;
    Mat src0 = imread(path);
    ImShow("rects", src0);
    int key = waitKey(200);
    
    //tessHelper.RunCvOcr(path, finalResult);
    TIME_MEASURE_BEGIN
#if 1
    float scale = 1.8f;
    tessHelper.RunCvOcr(src0, finalResult, 70);
#else
    Mat gray;
    GetGrayImage(src0, gray, false);
    float scale = 1.f;
    if (scale > 1.f)
    {
        resize(gray, gray, Size(gray.cols * scale, gray.rows * scale));
    }
    tessHelper.RunOcr(gray, Rect());
    tessHelper.GetOcrResultForCard(finalResult, 75);
#endif
    TIME_MEASURE_END("RunCvOcr")
    OSResults osr;
    int tessOrientation = tessHelper.GetOrientation(&osr);
    
    zp::CardOcrResult ocrResult;
    ocrResult.set(finalResult);
    ocrResult.setTessOrientation(tessOrientation);
    SaveObjectToFile(ocrResult, ReplaceExtension(path, ".txt"), "CardOcrResult");
    
    //bool getHOcrRet = tessHelper.SaveHOCRText(GetFileName(path, 1).c_str(), ReplaceExtension(path, "").c_str());
    //tessHelper.SaveThresholdImage(ReplaceExtension(path, ".png"));
    
    SimpleTestResult testResult(path, osr.best_result.orientation_id);
    testResult.mScores.push_back(osr.best_result.sconfidence);
    testResult.mScores.push_back(tessHelper.GetRecogTime());
    simpleStat.Add(testResult);
    
    std::vector<cv::Rect> regionRects;
    tessHelper.GetResultRects(TessHelper::OPTION_RECTS::Textline, regionRects);
    std::vector<cv::Rect> textLineRects;
    tessHelper.GetResultRects(TessHelper::OPTION_RECTS::Component, textLineRects);
    tessHelper.Clear();
    
    Mat tmp;
    resize(src0, tmp, Size(src0.cols * scale, src0.rows * scale));
    int thickness = MAX(src0.cols / 400, 1);
    
    for (int k=0; k<regionRects.size(); k++)
    {
        regionRects[k].x -= thickness * 4;
        regionRects[k].y -= thickness * 4;
        regionRects[k].width += thickness * 8;
        regionRects[k].height += thickness * 8;
    }
    DrawRects(tmp, regionRects, Scalar(255,100,0), thickness);
    DrawRects(tmp, textLineRects, Scalar(0,255,0), thickness);
    ImShow("rects", tmp);
    //imwrite("/Users/jackf/Downloads/TestImages/0803/BadForAddress_1.jpg", tmp);
    key = waitKey(0);
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
