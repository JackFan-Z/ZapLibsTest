//
//  CardOcrResult.cpp
//  ThreePartyTool
//
//  Created by YUAN CHANG FAN on 2015/8/9.
//  Copyright (c) 2015年 Zappoint. All rights reserved.
//

#include <stdio.h>
#include "ZapLib/TessResultParseCard.h"
#include "ZapLib/FilterOcrCardResult.h"
#include "CardOcrResult.h"


namespace zp
{
    CardOcrResult::CardOcrResult():tessOrientation(0)
    {
        
    }
    
    void CardOcrResult::set(const std::vector<std::vector<TextConfidence> > &finalResult)
    {
        for (int i=0; i < finalResult.size(); i++)
        {
            for (TextConfidence tc: finalResult[i])
            {
                switch (i) {
                    case IdxCOcr_FullName:
                        FullName = tc.getText();
                        break;
                    case IdxCOcr_EMail:
                        Emails.push_back(tc.getText());
                        break;
                    case IdxCOcr_Phone:
                        string phone = tc.getText();
                        CovertPhoneFormat(phone);
                        Phones.push_back(phone);
                        break;
                }
                
                if (i == IdxCOcr_FullName) break;
            }
        }
    }
}