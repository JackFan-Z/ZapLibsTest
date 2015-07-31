//
//  main.cpp
//  TestCardDetection
//
//  Created by YUAN CHANG FAN on 2015/7/31.
//  Copyright (c) 2015年 Zappoint. All rights reserved.
//

#include <iostream>
#include "ZapLib/Application.h"

MY_APP(TestCardDetection, "/Users/jackf/Dropbox/Work/TestImage/0731");

MY_APP_INIT(TestCardDetection, intParam)
{
}

MY_APP_PROCESSFILE(TestCardDetection, path)
{
}

MY_APP_END(TestCardDetection, intParam)
{
    return 0;
}