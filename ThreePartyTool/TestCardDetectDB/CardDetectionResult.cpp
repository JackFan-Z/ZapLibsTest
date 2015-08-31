//
//  CardDetectionResult.cpp
//  CvDetectCard
//
//  Created by YUAN CHANG FAN on 2015/4/22.
//  Copyright (c) 2015年 Invisibi. All rights reserved.
//

#include "CardDetectionResult.h"

#include "ZapLib/guitools.hpp"

using namespace std;

bool CardDetectionResult::SetCorners(const std::vector<cv::Point2f>& corners ) {
    if (corners.size() != 4)  return false;
    
    for (size_t i=0; i<corners.size(); i++)
    {
        mCorners.push_back(corners[i].x);
        mCorners.push_back(corners[i].y);
    }
    return true;
};

void CardDetectionResult::SetInitCorners(cv::Rect rect) {
    mCorners.push_back(rect.x);
    mCorners.push_back(rect.y);
    mCorners.push_back(rect.x + rect.width);
    mCorners.push_back(rect.y);
    mCorners.push_back(rect.x + rect.width);
    mCorners.push_back(rect.y + rect.height);
    mCorners.push_back(rect.x);
    mCorners.push_back(rect.y + rect.height);
};

/**
 * Compare corners (Assume corners are in the same order alreay.
 *     // 1------2
 *     // |      |
 *     // 4------3
 * @param cornerPoints
 * @param diffThresh : Should be between 0~1. The bigger the diffThresh, the easier it returns true.
 * @return true: The two sets of corners are close
 *         false: The two sets of corners are different
 */
bool CardDetectionResult::Compare(std::vector< cv::Point2f >& cornerPoints, const float diffThresh, float& maxDiffRatio)
{
    if (mCorners.size() == 0 || cornerPoints.size() == 0)
    {
        LOG_ERROR("CardDetectionResult::Compare: Wrong input");
        return false;
    }
    
    if (mCorners.size() != cornerPoints.size() * 2)
    {
        LOG_ERROR("CardDetectionResult::Compare: different size");
        return false;
    }
    
    float distVer = (cornerPoints[2].y + cornerPoints[3].y - cornerPoints[0].y - cornerPoints[1].y) / 2;
    float distHor = (cornerPoints[1].x + cornerPoints[2].x - cornerPoints[0].x - cornerPoints[3].x) / 2;
    
    if (distVer < 1 || distHor < 1) return false;
    
    float dist = MAX(distHor, distVer);
    maxDiffRatio = 0;
    for (size_t i = 0, j = 0; i < cornerPoints.size(); i++, j+=2)
    {
        cv::Point2f thisCorner(mCorners[j], mCorners[j+1]);
        float diffRatio = cv::norm(thisCorner - cornerPoints[i]) / dist;
        if (diffRatio > maxDiffRatio)
        {
            maxDiffRatio = diffRatio;
        }
    }
    
    return (maxDiffRatio < diffThresh);
}
