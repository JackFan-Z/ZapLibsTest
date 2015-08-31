//
//  cdmain.cpp
//  CvDetectCard
//
//  Created by YUAN CHANG FAN on 2015/4/27.
//  Copyright (c) 2015年 Invisibi. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include "ZapLib/optionparser.h"
#include "ZapLib/guitools.hpp"
#include "ZapLib/guitools_cv.h"
#include "ZapLib/CardTrackerFeature.h"
#include "ZapLib/ImageMarker.h"
#include "ZapLib/zMyLine.h"
#include "ZapLib/SimpleStat.h"
#include "ZapLib/preprocessimg.hpp"
#include "CardDetectionResult.h"

using namespace cv;

// -----------------
// Programe options
// -----------------
string optionDBdirectory = "/Users/jackf/GoogleDrive/Zappoint/Image\ Processing/CardDetectionDB";
string optionTestSubfolder = "Test1";
string optionTestSourcePaths = ""; //"Text1_fail.txt";
bool optionXCodeColor = false;
int optionDebugLevel = 2;

bool optionRunDetection = true;
bool optionBatchTest = false;
int optionCheckSpecificIndex = -1;
bool optionDebugStepByStep = false;
// -----------------
static CardTracker* gTheCardTracker = NULL;

char window_name[200] = "Card Corners";
Mat gDispImg;
ImageMarker imgMarker;
RectList gRectList;
const int CORNERS_TO_SAVE = 4;

Rect rectMove;
bool isMovingCorner()
{
    return (rectMove.width > 0);
}
bool frameModified;
bool isModified()
{
    return frameModified;
}
void setModified(){ frameModified = true; }
void resetModified(){ frameModified = false; }

int execoption(OptionParser& op, int argc, char** argv)
{
    /**************************************************************************
     * Adding options
     */
    op.add(new Option::Single<string>(&optionDBdirectory, "db", 'd',
                                      "Database directory"));
    op.add(new Option::Single<string>(&optionTestSubfolder, "subfolder", 's',
                                      "Test subfolder"));
    op.add(new Option::Single<string>(&optionTestSourcePaths, "sources", 'p',
                                      "Test source paths (e.g. Test1_fail.txt)"));
    op.add(new Option::Single<bool>(&optionXCodeColor, "xc", 'c',
                                    "XCode color (true/false)"));
    op.add(new Option::Single<int>(&optionDebugLevel, "debuglevel", 'l',
                                   "debug level 1~5"));
    
    op.add(new Option::Single<bool>(&optionRunDetection, "detect", 'r',
                                    "run detection (true/false)"));
    op.add(new Option::Single<bool>(&optionBatchTest, "test", 't',
                                    "run test in batch mode (true/false)"));
    op.add(new Option::Single<int>(&optionCheckSpecificIndex, "index", 'i',
                                    "check specific index"));
    op.add(new Option::Single<bool>(&optionDebugStepByStep, "debugbystep", '?',
                                   "debug level by step (true/false)"));

    /**************************************************************************
     * Parse arguments. Pass "true" for pruning the argv/argc after parsing.
     * If the built-in help was invoked, the method returns false, abort.
     */
    if (!op.parse(argc, argv, true))
    {
        return 0;
    }
    return 1;
}
void _DrawMousePointer(Mat& DisplayImg, const Point& p)
{
    circle(DisplayImg, p, 5, Scalar(50,50,255), 2);
}
void MyRedrawImage(int x, int y)
{
    imgMarker.Redraw(window_name, gRectList, Rect(), &gDispImg);
    if (x>=0 && y>=0)
    {
        _DrawMousePointer(gDispImg, Point(x,y));
    }
    
    if (isModified()==false) {
        std::vector< Rect > DispRectList;
        gRectList.GetRectList(DispRectList, true);
        for (int i=0; i<DispRectList.size(); i++)
        {
            if (i >= CORNERS_TO_SAVE)
            {
                continue;
            }
            
            Rect dispRect = DispRectList[i];
            char cNum[10];
            sprintf(cNum, "%d", i);
            Point pt = Point(dispRect.x,
                             dispRect.y - 5);
            Scalar color = Scalar(255 - (i*50), i*50, 0);
    //        cv::putText(gDispImg, cNum,
    //                    pt,
    //                    cv::FONT_HERSHEY_COMPLEX, 2, color);
            QtFont font = fontQt("Times",30, color, QT_FONT_BOLD);
            addText(gDispImg, cNum, pt, font);
        }
    }
    imshow(window_name, gDispImg);
}
void MyRedrawImage()
{
    MyRedrawImage(-1, -1);
}

void on_mouse(int event,int x,int y,int flag, void*) {
    
    static double roi_x0 = -1;  // (roi_x0, roi_y0) is the coordinate of first corner
    static double roi_y0 = -1;
    static double roi_x1 = -1;  // (roi_x1, roi_y1) is the coordinate of second corner
    static double roi_y1 = -1;
    
    //    if (event != EVENT_MOUSEMOVE)
    //    {
    //        LOG_INFO("on_mouse e=%d  %d,%d  flag=%d", event,x,y,flag);
    //    }
    
    if(event == EVENT_LBUTTONUP) {
        
        if (x < 0 || y < 0){
            roi_x0 = roi_x1 = x;
            roi_y0 = roi_y1 = y;
        } else {
            roi_x0 = roi_x1 = x;
            roi_y0 = roi_y1 = y;
        }
        if (isMovingCorner())
        {
            gRectList.AddDispRect(x - rectMove.width/2, y-rectMove.height/2, rectMove.width, rectMove.height);
            rectMove.width = rectMove.height = 0;
            MyRedrawImage();
        }
        else
        {
            int idx = gRectList.DispContains(x, y);
            LOG_V("on_mouseDown e=%d  %d,%d  flag=%d. h=%d", event,x,y,flag, idx);
            if (idx >= 0 && idx < CORNERS_TO_SAVE) {
                size_t size = gRectList.GetSize();
                for (int i=0; size > 4 && i < size-4; i++)
                {
                    gRectList.DeleteLast();
                }
                
                rectMove = gRectList.GetRect(idx, true);
                setModified();
                gRectList.Delete(idx);
                MyRedrawImage(x, y);
            }
        }
    }
    else if (event == EVENT_MOUSEMOVE && isMovingCorner())
    {
        MyRedrawImage(x, y);
    }
    
    if (x<0 || y<0) {
        return;
    }
}
static bool MyKeyHandling(const char* window, Mat dispImg, int& key);
static const string OBJECT_NAME = "CardDetectionResult";
const int KEY_RELOAD = 1;
const int KEY_ESC = 1048603;
const int KEY_OFFSET = 1048576;
int loadFrame(string inputName, const Mat& src, std::vector<cv::Point2f> detectedCorners, bool batchMode, SimpleStat& simpleStat)
{
    int key = 0;
    resetModified();
    
    //Mat src = imread(inputName, IMREAD_COLOR);
    CardDetectionResult result;
    bool loadRet = LoadObjectFromFile(ReplaceExtension(inputName, ".txt"), result);
    if (loadRet == false)
    {
        if (optionBatchTest)
        {
            PrintOutMsg(" !!! error. No corners text file");
        }
        
        if (detectedCorners.size() == 4)
        {
            result.SetCorners(detectedCorners);
            detectedCorners.clear();
        }
        else
        {
            result.SetInitCorners(Rect(src.cols/3, src.rows/3, src.cols/3, src.rows/3));
        }
        setModified();
    }
    
    
    static int totalTest = 0;
    static int correct = 0;
    totalTest++;
    bool match = false;
    float maxDiffRatio = 1.f;
    if (loadRet && detectedCorners.size() == 4)
    {
        if (result.Compare(detectedCorners, 0.05f, maxDiffRatio))
        {
            match = true;
            correct++;
        }
    }
    LOG_INFO("Correction (%d) Ratio = %.2f", match ? 1 : 0, (float) correct / totalTest);
    SimpleTestResult testResult(inputName, match ? 1 : 0);
    testResult.mScores.push_back(maxDiffRatio);
    simpleStat.Add(testResult);
    if (batchMode)
    {
        return 0;
    }
        
    std::vector<float> corners = result.GetCorners();
    
    imgMarker.Create(src);
    imgMarker.SetScale(800);
    imgMarker.mThickness = 4;
    gRectList.Clear();
    gRectList.SetScale(imgMarker.GetScale());
    const int rectSize = 12 / imgMarker.GetScale();
    for (size_t i=0; i< corners.size(); i+=2)
    {
        gRectList.AddRect(corners[i] - rectSize/2, corners[i+1] - rectSize/2, rectSize, rectSize, Scalar(255,0,0));
    }
    
    if (detectedCorners.size() > 0)
    {
        for (size_t i=0; i< detectedCorners.size(); i++)
        {
            gRectList.AddRect(detectedCorners[i].x - rectSize/2, detectedCorners[i].y - rectSize/2, rectSize, rectSize,
                              match ? Scalar(150,250,100) : Scalar(150,150,255));
        }
    }
    
    MyRedrawImage();
    displayOverlay(window_name, inputName, 1500);
    if (MyKeyHandling(window_name, gDispImg, key))
    {
        std::vector<cv::Point2f> cornerPoints;
        for (int i=0; i < CORNERS_TO_SAVE; i++) {
            Rect tmp = gRectList.GetRect(i, false);
            cornerPoints.push_back(Point2f(tmp.x + tmp.width/2, tmp.y + tmp.height/2));
        }
        cv::Point2f center(0,0);
        for (unsigned int i = 0; i < cornerPoints.size(); i++)
            center += cornerPoints[i];
        center *= (1. / cornerPoints.size());
        sortCorners(cornerPoints, center);
        
        CardDetectionResult result;
        result.SetCorners(cornerPoints);
        SaveObjectToFile(result, ReplaceExtension(inputName, ".txt"), OBJECT_NAME);
        key = KEY_RELOAD;
    }
    return key;
}

int processFrame(Mat src, std::vector<cv::Point2f>& Corners)
{
    Corners.clear();
    if (src.empty())
    {
        return -1;
    }

    /*
         int oriCols = src.cols;
    std::vector< std::vector<cv::Point2f> > squares;
     ResizeToWorkingSize(src, src, 600);
     float ratio = oriCols / (float) src.cols;

    DetectRectHoughLines detectrect;
    detectrect.SetDebugImgOption(optionDebugStepByStep ? DetectRect::DebugImgOption::YES : DetectRect::DebugImgOption::No);
    if(!optionBatchTest)
    {
        detectrect.NamedWindow();
    }
    detectrect.Run(src, squares);
    */
    bool ret = gTheCardTracker->ProcessFrame(&src, Corners);
    gTheCardTracker->ResetState(); // Do run tracking code
    
    int key = 0;
    return key;
}

int CropAndEnhance(const Mat& src, const std::vector<cv::Point2f>& Corners, Mat& dst, string path)
{
    bool ret = GetQuadMat(src, dst, Corners, src.cols > src.rows ? src.cols : src.rows, 60);
    if (!ret) return -1;
    
    imwrite(path, dst);
    
    Mat inpainted;
    TIME_MEASURE_BEGIN
    //InpaintCardBorder(dst, inpainted, dst.cols / 60 + 1);
    CutOutCardBorder(dst, inpainted);
    char msg[128] = {0};
    snprintf(msg, 128, "CutOutCardBorder");
    TIME_MEASURE_END(msg);
    
    imwrite(ReplaceExtension(path, ".inp.jpg"), inpainted);
    
    return 0;
}

int CropAndFilter(const Mat& src, const std::vector<cv::Point2f>& Corners, Mat& dst, string path)
{
    Mat quadImg;
    if (Corners.size() > 0)
    {
        bool ret = GetQuadMat(src, quadImg, Corners, src.cols > src.rows ? src.cols : src.rows, 60);
        if (!ret) return -1;    }
    else
    {
        quadImg = src;
    }
    
    string mainName = GetFileName(path, 1);
//    string newSrcPath = path;
//    path = newSrcPath.replace( path.find(mainName), mainName.size(), "output1/" + mainName);
    imwrite(path, quadImg);
    
    
    Mat tmpGray;
    GetGrayImage(quadImg, tmpGray, false);
    double thresholdValue0 = cv::threshold(tmpGray, tmpGray, 0, 255, THRESH_BINARY + THRESH_OTSU);
    LOG_D("Ori otsu thres %.1f", thresholdValue0);
    
    Mat out, out1;
    TIME_MEASURE_BEGIN
    int size = src.cols / 120;
    if (size < 5) size = 5;
    GetGrayUniformBackgroundByMorphological(quadImg, dst, cv::MORPH_RECT, size);
    TIME_MEASURE_END("Filter");
    
    imwrite(ReplaceExtension(path, ".filter.jpg"), dst);
    
     //ResizeToWorkingSize(src1, src1, 800);
     //cvtColor(src, src1, COLOR_BGR2GRAY);
     TIME_MEASURE_BEGIN_
     double thresholdValue = cv::threshold(dst, out, 0, 255, THRESH_BINARY + THRESH_OTSU);
        imwrite(ReplaceExtension(path, ".binary.jpg"), out);
    char msg[64] = {0};
    snprintf(msg, 64, "Otsu (%.1f)", thresholdValue);
     TIME_MEASURE_END_(msg);
    
    /*
     ResizeToWorkingSize(out, out1, 800);
     imshow("otsu", out1);
     
     TIME_MEASURE_BEGIN_
     cv::adaptiveThreshold(src1, out, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 31, 1);
     TIME_MEASURE_END_("MeanC");
     ResizeToWorkingSize(out, out1, 800);
     imshow("a mean c", out1);
     
     TIME_MEASURE_BEGIN_
     cv::adaptiveThreshold(src1, out, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 31, 1);
     TIME_MEASURE_END_("MeanG");
     ResizeToWorkingSize(out, out1, 800);
     imshow("a Gaussian", out1);
     
     TIME_MEASURE_BEGIN_
     DoBinarize(src1, out);
     TIME_MEASURE_END_("WOLFJOLION");
     ResizeToWorkingSize(out, out1, 800);
     imshow("WOLFJOLION", out1);
     */
    return 0;
}

int loadAndQuadFrame(string path)
{
    CardDetectionResult result;
    bool loadRet = LoadObjectFromFile(ReplaceExtension(path, ".txt"), result);
    if (loadRet == false)
    {
        LOG_D("Failed to load text file for corners");
        return 0;
    }
    Mat src = imread(path);
    ImShow("src", src);
    Mat dst;
    std::vector<float> cornersInFloat = result.GetCorners();
    std::vector<Point2f> Corners;
    for (int i=0; i<cornersInFloat.size(); i+=2)
    {
        Corners.push_back(Point2f(cornersInFloat[i], cornersInFloat[i+1]));
    }
    bool ret = GetQuadMat(src, dst, Corners, src.cols > src.rows ? src.cols : src.rows, 60);
    ImShow("Quad", dst, 1000, -1, -1);
    int key = waitKey();
    return key;
}

int main(int argc, char** argv)
{
    OptionParser op(true, "Card Detector 2015/08/28\n\tby Jack Fan");
    if (execoption(op, argc, argv)==0)
    {
        return 0;
    }
    optionDBdirectory = CreateFolderPath(optionDBdirectory, "");
    
    SetDebugLevel(optionDebugLevel);
    if (optionBatchTest)
    {
        string printOutPath = optionDBdirectory + optionTestSubfolder + "_temp.txt";
        SetPrintOutFile(printOutPath.c_str());
    }
    else
    {
        EnableXcodeColor(optionXCodeColor);
    }
    
    gTheCardTracker = new zp::CardTrackerFeature();
    gTheCardTracker->Init(NULL);
    
    std::vector<std::string> files = std::vector<std::string>();
    string dir = CreateFolderPath(optionDBdirectory, optionTestSubfolder);
    GetFileList getFileList(dir.c_str());
    if (optionTestSourcePaths.length() == 0)
    {
        if (optionTestSubfolder.length() == 0)
        {
            PrintOutMsg("Search all subfolders");
            getFileList.GetList(files, 2);
        }
        else
        {
            getFileList.GetList(files);
        }
    }
    else
    {
        dir = "";
        PrintOutMsg("Get files from %s", optionTestSourcePaths.c_str() );
        getFileList.GetList(files, optionDBdirectory + optionTestSourcePaths);
    }
    
    int totalfiles = (int) files.size();
    //for (int i = totalfiles - 1; i >= 0; i-- )
    
    LOG_INFO("Load frame and show corners now. Total=%d", totalfiles);
    namedWindow(window_name);
    moveWindow(window_name, 10, 10);
    setMouseCallback(window_name, on_mouse);
    
    SimpleStat simpleStat;
    for (int i = 0; i < totalfiles; i++ )
    {
        string inputPath = dir + files[i];
        LOG_INFO("[%d / %d] Load %s", i, files.size(), files[i].c_str());
//        if (files[i].find("New") == 0)
//        {
//            LOG_D("Skip %s", files[i].c_str());
//            continue;
//        }
        
        if (optionCheckSpecificIndex >= 0 && i < optionCheckSpecificIndex) continue;

        if (optionBatchTest)
        {
            // also print out progress to std::out
            PrintOutMsg("[%d / %d] Load %s", i, files.size(), files[i].c_str());
        }
        std::vector<cv::Point2f> Corners;
        int key;
        Mat src = imread(inputPath, IMREAD_COLOR);
        if (src.empty())
        {
            LOG_ERROR("Failed to open %s", inputPath.c_str());
            continue;
        }

        if (optionBatchTest || optionRunDetection)
        {
            TIME_MEASURE_BEGIN
            processFrame(src, Corners);
            TIME_MEASURE_END("ProcessFrame")
            
            key = loadFrame(inputPath, src, Corners, optionBatchTest, simpleStat);
            simpleStat.GetLastResult().mScores.push_back(timeSpent / 1000);

        } else {
            key = loadAndQuadFrame(inputPath);
        }
//        Mat dst;
//        string path = "/Users/jackf/Downloads/TestImages/0521_ocr/output2/" + files[i];
//        //CropAndEnhance(src, Corners, dst, path);
//        cv::GaussianBlur(src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
//        CropAndFilter(src, Corners, dst, path);
//        ImShow("result", dst);
        //waitKey();
//        Mat mserOutMask;
//        TIME_MEASURE_BEGIN
//        dst.copyTo(mserOutMask);
//        mserExtractor(dst, mserOutMask);
//        TIME_MEASURE_END("MSER");
//        imshow("mask", mserOutMask);
//        waitKey();
        
        if ( key == KEY_ESC || key == KEY_ESC - KEY_OFFSET )
        {
            PrintOutMsg("ESC key is pressed. Exit.");
            break;
        }
        else if (key == KEY_RELOAD)
        {
            i--;
        }
    }
    
    if (optionBatchTest)
    {
        simpleStat.DumpStat(optionDBdirectory + optionTestSubfolder + "_temp.csv");
        simpleStat.DumpDescriptions(0, optionDBdirectory + optionTestSubfolder + "_fail.txt");
    }
    
    return 0;
}

bool MyKeyHandling(const char* window, Mat dispImg, int& iKey)
{
    const int Key_Offset = 1048576;
    enum KeyBindings {
        Key_EnterMac = 1048589,
        Key_Enter = 1048586,  Key_ESC = 1048603,  Key_Space = 1048608, Key_d = 1048676,
        Key_8a = 1114040, Key_8b = 1048632, Key_9a = 1048633, Key_9b = 1114041,
        Key_2a = 1048626, Key_2b = 1114034, Key_3a = 1048627, Key_3b = 1114035,
        Key_4a = 1048628, Key_4b = 1114036, Key_5a = 1048629, Key_5b = 1114037,
        Key_6a = 1048630, Key_6b = 1114038, Key_7a = 1048631, Key_7b = 1114039,
        Key_w = 1048695, Key_W = 1114199, Key_h = 1048680, Key_H = 1114184,
        Key_z = 1048698, Key_Z = 1114202, Key_c = 1048675, Key_s = 1048691,
        Key_e = 1048677, Key_p = 1048688, Key_t = 1048692, Key_j = 1048682,
        Key_a = 1048673, Key_m = 1048685,
        Key_add = 1048637, Key_minus=1048621
    };
    bool cont = false;
    do {
        // Get user input
        iKey = -1;
        while (iKey <= 0)
        {
            iKey = waitKey(30);
            imshow(window, dispImg);
        }
        iKey += Key_Offset;
        switch(iKey) {
            case Key_ESC:
            case Key_Space:
                if (isModified()) {
                    cont = true;
                    resetModified();
                    displayOverlay(window_name, "Are you sure to lose changes ?", 1500);
                } else {
                    cont = false;
                }
                break;
            case Key_s:
                if (isMovingCorner()) {
                    cont = true;
                    displayOverlay(window_name, "One corner is not set yet.", 1500);
                } else if (isModified()){
                    cont = false;
                    displayOverlay(window_name, "Saved", 1500);
                    return true;
                } else {
                    cont = false;
                }
                break;
            default:
                cont = true;
                displayOverlay(window_name, "Press ESC or s", 1500);
                break;
        }
    }while (cont);
    
    return false;
}
