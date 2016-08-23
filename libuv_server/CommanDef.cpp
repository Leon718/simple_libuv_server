//
//  CommanDef.cpp
//  libuv_server
//
//  Created by 何宏洲 on 16/8/7.
//  Copyright © 2016年 何宏洲. All rights reserved.
//

#include "CommanDef.hpp"
#include "uv.h"
#include "easylogging++.h"
using namespace std;
bool getUvErr(const std::string& command,int errcode){
    if (errcode == 0) {
        return true;
    }else{
        //LOG(ERROR)<<command<<" 失败，"<<uv_err_name(errcode)<<" "<<uv_strerror(errcode)<<endl;
		return false;
    }
}