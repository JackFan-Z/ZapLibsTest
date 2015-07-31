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
static CardTracker* gTheCardTracker = NULL;

using namespace std;
MY_APP(TestCardDetection, "/Users/jackf/Dropbox/Work/TestImage/0731");

MY_APP_INIT(TestCardDetection, intParam)
{
    gTheCardTracker = new zp::CardTrackerFeature();
    gTheCardTracker->Init(NULL);
    return 0;
}

MY_APP_PROCESSFILE(TestCardDetection, path)
{
    return 0;
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