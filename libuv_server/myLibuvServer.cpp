//
//  myLibuvServer.cpp
//  libuv_server
//
//  Created by 何宏洲 on 16/8/7.
//  Copyright ? 2016年 何宏洲. All rights reserved.
//

#include <assert.h>
#include "myLibuvServer.hpp"
#include "easylogging++.h"
#include "CommanDef.hpp"
#ifdef WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#endif

using namespace std;

myLibuvServer::myLibuvServer(){
    uv_loop_init(&_server_loop);
}
myLibuvServer::~myLibuvServer(){
    if (!_clients.empty()) {
        for (auto client:_clients) {
            free(client);
            client = nullptr;
        }
        _clients.clear();
    }
}

bool myLibuvServer::init(const string& ip,int port){
    if (ip.empty()) {
        LOG(ERROR)<<"ip is empty"<<endl;
    }
    if (port<1024) {
        LOG(ERROR)<<"ip is less than 1024"<<endl;
    }
    
    uv_tcp_init(&_server_loop, &_server);
    _server.data = this;//connect之后的回调没有上下文，所以借用tcp_t的data传递上下文。
    
    //sockaddr_in就是unix标准。使用uv_ip4_addr接口赋值ip端口
    struct sockaddr_in addr;
    uv_ip4_addr(ip.c_str(), port, &addr);
    
    int ret = uv_tcp_bind(&_server, (const struct sockaddr*)&addr, 0);
    
    return getUvErr("bind "+T_toStr<int>(port),ret);
}

//开始监听
bool myLibuvServer::Start(){
    int ret = uv_listen((uv_stream_t*) &_server, DEFAULT_BACKLOG, on_new_connection);
    if (!getUvErr("Listen",ret)) {
        return false;
    }
    
    ret =  uv_run(&_server_loop, UV_RUN_DEFAULT);
    if (!getUvErr("uv_run",ret)) {
        return false;
    }
    return true;
}

//该回调是在内核listen socket接收到一个链接的时候。
//该函数放主线程。client的read write放线程池
void myLibuvServer::on_new_connection(uv_stream_t *server, int status) {
//    1. 检查回调状态
    if (!getUvErr("new connection", status)) {
        return;
    }
//    2. 得到上下文
    myLibuvServer* mylibuvserver = (myLibuvServer*)server->data;
    assert((void*)&mylibuvserver->_server==(void*)server);
    
//    3. new 一个client实例
    myLibuvClientCtx * ClientCtx = new myLibuvClientCtx(mylibuvserver->_server_loop);
    mylibuvserver->_clients.push_back(ClientCtx);
    
//    4. ClientCtx 接收connect socket
    if(ClientCtx->accept(&mylibuvserver->_server_loop, server)){
        ClientCtx->read();
    }
}

//========================
myLibuvClientCtx::myLibuvClientCtx(uv_loop_t& ref ):isclosed(false){
}
myLibuvClientCtx::~myLibuvClientCtx( ){
}
//接收connect socket
bool myLibuvClientCtx::accept(uv_loop_t* loopref,uv_stream_t* server_handle){
    if (loopref==nullptr||server_handle==nullptr) {
        return false;
    }
    _loop_ref = loopref;
    
    uv_tcp_init(_loop_ref, &_connect_handle);
    _connect_handle.data = this;
    
    int ret = uv_accept(server_handle, (uv_stream_t*)&_connect_handle);
    if (!getUvErr("myLibuvClientCtx accept", ret)) {
        this->close();
        return false;
    }
    
    
    struct sockaddr peername;
    int namelen = sizeof(peername);
    ret = uv_tcp_getpeername(&_connect_handle, &peername, &namelen);//uv_tcp_getsockname是获取本机信息 uv_tcp_getpeername是获取对方信息
    if (!getUvErr("uv_tcp_getpeername", ret)) {
        return false;
    }
    struct sockaddr_in check_addr = *(struct sockaddr_in*) &peername;
    char check_ip[17];
    ret = uv_ip4_name(&check_addr, (char*)check_ip, sizeof check_ip);
    if (!getUvErr("uv_ip4_name", ret)) {
        return false;
    }
    peerip = check_ip;
    //peerport = ntohs(check_addr.sin_port);
    LOG(INFO)<<"收到客户端链接: "<<peerip<<":"<< check_addr.sin_port <<endl;
    return true;
}
uv_tcp_t* myLibuvClientCtx::getConnectHandle(){
    return &_connect_handle;
}
void myLibuvClientCtx::close(){
    uv_close((uv_handle_t*)&_connect_handle, myLibuvClientCtx::after_close);
}
void myLibuvClientCtx::after_close(uv_handle_t* connect_handle){
    if (connect_handle==nullptr) {
        return;
    }
    myLibuvClientCtx* ClientCtx = (myLibuvClientCtx*)connect_handle->data;
    ClientCtx->isclosed = true;
    LOG(INFO)<<"ClientCtx is close successufully ,"<<ClientCtx->peerip<<":"<<ClientCtx->peerport<<endl;
}
bool myLibuvClientCtx::read()
{
    //开始监听uv_stream_t的消息，一旦内核从网络读取消息，先回调buffer_alloc 把消息放到内存。再回调after_read处理消息
    int ret = uv_read_start((uv_stream_t*)&this->_connect_handle, myLibuvClientCtx::buffer_alloc, myLibuvClientCtx::after_read);
    if (!getUvErr("myLibuvClientCtx read", ret)) {
        return false;
    }else{
        return true;
    }
}
void myLibuvClientCtx::buffer_alloc(uv_handle_t* connect_handle,size_t suggested_size,uv_buf_t* buf) {
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}
void myLibuvClientCtx::after_read(uv_stream_t* connect_handle,ssize_t nread,const uv_buf_t* buf) {
    if (connect_handle==nullptr||connect_handle->data == nullptr) {
        return;
    }
    myLibuvClientCtx* context = (myLibuvClientCtx*)connect_handle->data;
    if (nread < 0) {
        assert(nread == UV_EOF);
        context->close();
    }else if (nread == 0) {
        /* Everything OK, but nothing read. 正常，忽略*/
    }else{
        LOG(INFO)<<"client : "<<buf->base;
        
        //准备要echo的数据
        char* tmp = new char[nread];
        memcpy(tmp, buf->base, nread);
        

        //该函数内部并没有分配空间，只是把指针和长度组装成一个struct。
        uv_buf_t writebuf_ = uv_buf_init(tmp, (unsigned int)nread);
        
        //创建一个write request 写入对象是handle（conn socket）  写入内容是buf
        uv_write_t write_req_;
        write_req_.data = tmp;//为了写完释放空间
        
        //监视connect socket的写入情况（一旦往connect socket写完），会回调after_write
        if (uv_write(&write_req_, connect_handle, &writebuf_, 1, after_write)) {
            printf("uv_write failed");
        }
    }
    //把读到的数据释放
    free(buf->base);
}


void myLibuvClientCtx::after_write(uv_write_t* req, int status) {
    free(req->data);//释放写的数据
    if (status == 0){
        return;
    }else{
        printf("uv_write error: %s - %s\n",uv_err_name(status),uv_strerror(status));
    }
}