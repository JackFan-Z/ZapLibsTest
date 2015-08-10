//
//  SerializationObjectArray.h
//  CvDetectCard
//
//  Created by YUAN CHANG FAN on 2015/4/23.
//  Copyright (c) 2015年 Invisibi. All rights reserved.
//

#ifndef __CvDetectCard__SerializationObjectArray__
#define __CvDetectCard__SerializationObjectArray__

#include <stdio.h>
#include <vector>
#include <fstream>
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>
#include "json/json.h"
#include "ZapLib/filetools.hpp"

template <class _T>
bool SaveObjectToFile(const _T& object, std::string path, const std::string& preferName, bool overwriteIfExist)
{
    if (overwriteIfExist==false)
    {
        std::ifstream ifile(path);
        if( ifile.is_open() )
        {
            const size_t WARNING_SIZE = 3*1024*1024;
            ifile.seekg(0, std::ios::end);
            streampos size = ifile.tellg();
            if (size > WARNING_SIZE)
            {
                std::cout << " ! Warning: the file " << path << "size is big. " << size << std::endl;
            }
            else if (size > 0)
            {
                std::cout << path << " exists" << std::endl;
                return false;
            }
        }
    }
    std::ofstream ofile(path);
    cereal::JSONOutputArchive archive(ofile);
    archive(cereal::make_nvp(preferName.c_str(), object) ); // specify a name of your choosing
    return true;
}

template <class _T>
bool SaveObjectToFile(const _T& object, std::string path, const std::string& preferName)
{
    return SaveObjectToFile(object, path, preferName, false);
}

template <class _T>
bool LoadObjectFromFile(const std::string path, _T& object)
{
    std::ifstream ifile(path);
    if (ifile.is_open()) {
        cereal::JSONInputArchive archive(ifile);
        archive( object ); // NVPs not strictly necessary when loading
        return true;
    }
    else
    {
        return false;
    }
}

template <class _T>
bool SaveObjectToJson(const _T& object, std::string& jsonString, const std::string& preferName)
{
    std::ostringstream jsonTmp;
    {
        cereal::JSONOutputArchive archive(jsonTmp);
        
        archive(cereal::make_nvp(preferName.c_str(), object) ); // specify a name of your choosing
    }
    jsonString = jsonTmp.str();
    //std::cout << jsonString << std::endl;
    return true;
}

template <class _T>
bool LoadObjectFromJson(const std::string& jsonString, _T& object)
{
    std::istringstream jsonIStream;
    jsonIStream.str(jsonString);
    cereal::JSONInputArchive archive(jsonIStream);
    
    archive( object ); // NVPs not strictly necessary when loading
    return true;
}

template <class _T>
class SerializationObjectArray
{
public:
    SerializationObjectArray();
    void Add(const _T& object){
        mList.push_back(object);
    }
    void Clear() {
        mList.clear();
    };

    bool Save(const std::string& path);
    bool Load(const std::string& path);
protected:
    std::vector<_T> mList;
    Json::Value mJObjs;
};

template <class _T>
SerializationObjectArray<_T>::SerializationObjectArray() : mJObjs(Json::arrayValue)
{}

const static std::string preferName = "myobject";

template <class _T>
bool SerializationObjectArray<_T>::Save(const std::string& path)
{
    std::ofstream f(path);
    if (!f.is_open())
    {
        std::cout << "Failed to open " << path << std::endl;
        return false;
    }
    if (mList.size()==0)    return false;
    
    for (size_t i=0; i<mList.size(); i++)
    {
        std::string jsonString;
        SaveObjectToJson(mList[i], jsonString, "myobject");
        // Let's parse it
        Json::Value root;
        Json::Reader reader;
        bool parsedSuccess = reader.parse(jsonString,
                                          root,
                                          false);
        if (parsedSuccess == false)
        {
            std::cout << "Failed to parse " << std::endl;
            continue;
        }
        //Json::Value jMyObject = root["myobject"];
        mJObjs.append(root);
    }
    f << mJObjs;
    return true;
}

template <class _T>
bool SerializationObjectArray<_T>::Load(const std::string& path)
{
    string text;
    if (ReadTextFile(path, text) == false) return false;
    
    // Let's parse it
    Json::Value root;
    Json::Reader reader;
    bool parsedSuccess = reader.parse(text,
                                      root,
                                      false);
    if (parsedSuccess == false)
    {
        std::cout << "Failed to open " << path << std::endl;
        return false;
    }
    
    mList.clear();
    for( int i=0; i<root.size(); i++)
    {
        Json::Value tmpJson = root[i];
        _T object;
        LoadObjectFromJson(tmpJson.toStyledString(), object);
        mList.push_back(object);
        
    }
    return true;
}

#endif /* defined(__CvDetectCard__SerializationObjectArray__) */
