//
//  CardDetectionResult.h
//  CvDetectCard
//
//  Created by YUAN CHANG FAN on 2015/4/22.
//  Copyright (c) 2015年 Invisibi. All rights reserved.
//

#ifndef __CvDetectCard__CardDetectionResult__
#define __CvDetectCard__CardDetectionResult__

#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "SerializationObjectArray.h"

class CardDetectionResult
{
public:
    CardDetectionResult(){};
    bool SetCorners(const std::vector<cv::Point2f>& corners );
    void SetInitCorners(cv::Rect rect);
    std::vector<float> GetCorners(){
        return mCorners;
    }

    // This method lets cereal know which data members to serialize
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive( mCorners ); // serialize things by passing them to the archive
    }
    
    bool Compare(std::vector< cv::Point2f >& cornerPoints, const float thresh, float& maxDiffRatio);
    
private:
    std::vector<float> mCorners;
    
};

class SerializationCardDetectResultArray : public SerializationObjectArray<CardDetectionResult>
{


};

#endif /* defined(__CvDetectCard__CardDetectionResult__) */
