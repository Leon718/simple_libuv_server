//
//  CommanDef.hpp
//  libuv_server
//
//  Created by 何宏洲 on 16/8/7.
//  Copyright © 2016年 何宏洲. All rights reserved.
//

#ifndef CommanDef_hpp
#define CommanDef_hpp

#include <stdio.h>
#include <sstream>
#include <string>
/*
 convert other data to string
 usage :
 string str = T_toStr<int>(12345);
 */
template<class T> std::string T_toStr(T tmp)
{
    std::stringstream ss;
    ss << tmp;
    return ss.str();
}

static int Str_toI(const std::string& str)
{
    return atoi(str.c_str());
}
static long long Str_toLL(const std::string& str)
{
    return atoll(str.c_str());
}

bool getUvErr(const std::string& command,int errcode);

#endif /* CommanDef_hpp */
