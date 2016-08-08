//
//  myLibuvServer.hpp
//  libuv_server
//
//  Created by 何宏洲 on 16/8/7.
//  Copyright © 2016年 何宏洲. All rights reserved.
//

#ifndef myLibuvServer_hpp
#define myLibuvServer_hpp

#include <stdio.h>
#include <string>
#include <list>
#include "uv.h"
class myLibuvClientCtx;
class myLibuvServer{
public:
    myLibuvServer();
    ~myLibuvServer();
    bool init(const std::string& addr,int port);
    bool Start();
private:
    static void on_new_connection(uv_stream_t *server, int status);

    
private:
    uv_loop_t _server_loop;
    uv_tcp_t _server;
    std::list<myLibuvClientCtx*> _clients;
    const int DEFAULT_BACKLOG = 8;
};


class myLibuvClientCtx{
public:
    myLibuvClientCtx(uv_loop_t& ref);
    ~myLibuvClientCtx();
    bool init();
    uv_tcp_t* getConnectHandle();
    void close();
    bool accept(uv_loop_t* loopref,uv_stream_t* server_handle);
    bool read();
private:
    static void after_close(uv_handle_t* connect_handle);
    static void after_read(uv_stream_t* handle,ssize_t nread,const uv_buf_t* buf);
    static void buffer_alloc(uv_handle_t* handle,size_t suggested_size,uv_buf_t* buf);
    static void after_write(uv_write_t* req, int status);
private:
    uv_loop_t* _loop_ref;
    uv_tcp_t _connect_handle;
    std::string peerip;
    int peerport;
    bool isclosed;
};

#endif /* myLibuvServer_hpp */
