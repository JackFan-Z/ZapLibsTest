//
//  CardOcrResult.h
//  ThreePartyTool
//
//  Created by YUAN CHANG FAN on 2015/6/16.
//  Copyright (c) 2015年 Zappoint. All rights reserved.
//

#ifndef ThreePartyTool_CardOcrResult_h
#define ThreePartyTool_CardOcrResult_h

#include <stdio.h>
#include <string>
#include <vector>
#include "SerializationObjectArray.h"

namespace zp
{
    
    class CardOcrResult
    {
    public:
        CardOcrResult(){};
        
        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive( CEREAL_NVP(FullName) ); // serialize things by passing them to the archive
            archive( CEREAL_NVP(Emails) ); // serialize things by passing them to the archive
            archive( CEREAL_NVP(Phones) ); // serialize things by passing them to the archive
        }
        std::string FullName;
        std::vector<std::string> Emails;
        std::vector<std::string> Phones;
    };
    
}


#endif
