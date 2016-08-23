//
//  myLibuvServer.cpp
//  libuv_server
//
//  Created by �κ��� on 16/8/7.
//  Copyright ? 2016�� �κ���. All rights reserved.
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
    _server.data = this;//connect֮��Ļص�û�������ģ����Խ���tcp_t��data���������ġ�
    
    //sockaddr_in����unix��׼��ʹ��uv_ip4_addr�ӿڸ�ֵip�˿�
    struct sockaddr_in addr;
    uv_ip4_addr(ip.c_str(), port, &addr);
    
    int ret = uv_tcp_bind(&_server, (const struct sockaddr*)&addr, 0);
    
    return getUvErr("bind "+T_toStr<int>(port),ret);
}

//��ʼ����
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

//�ûص������ں�listen socket���յ�һ�����ӵ�ʱ��
//�ú��������̡߳�client��read write���̳߳�
void myLibuvServer::on_new_connection(uv_stream_t *server, int status) {
//    1. ���ص�״̬
    if (!getUvErr("new connection", status)) {
        return;
    }
//    2. �õ�������
    myLibuvServer* mylibuvserver = (myLibuvServer*)server->data;
    assert((void*)&mylibuvserver->_server==(void*)server);
    
//    3. new һ��clientʵ��
    myLibuvClientCtx * ClientCtx = new myLibuvClientCtx(mylibuvserver->_server_loop);
    mylibuvserver->_clients.push_back(ClientCtx);
    
//    4. ClientCtx ����connect socket
    if(ClientCtx->accept(&mylibuvserver->_server_loop, server)){
        ClientCtx->read();
    }
}

//========================
myLibuvClientCtx::myLibuvClientCtx(uv_loop_t& ref ):isclosed(false){
}
myLibuvClientCtx::~myLibuvClientCtx( ){
}
//����connect socket
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
    ret = uv_tcp_getpeername(&_connect_handle, &peername, &namelen);//uv_tcp_getsockname�ǻ�ȡ������Ϣ uv_tcp_getpeername�ǻ�ȡ�Է���Ϣ
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
    LOG(INFO)<<"�յ��ͻ�������: "<<peerip<<":"<< check_addr.sin_port <<endl;
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
    //��ʼ����uv_stream_t����Ϣ��һ���ں˴������ȡ��Ϣ���Ȼص�buffer_alloc ����Ϣ�ŵ��ڴ档�ٻص�after_read������Ϣ
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
        /* Everything OK, but nothing read. ����������*/
    }else{
        LOG(INFO)<<"client : "<<buf->base;
        
        //׼��Ҫecho������
        char* tmp = new char[nread];
        memcpy(tmp, buf->base, nread);
        

        //�ú����ڲ���û�з���ռ䣬ֻ�ǰ�ָ��ͳ�����װ��һ��struct��
        uv_buf_t writebuf_ = uv_buf_init(tmp, (unsigned int)nread);
        
        //����һ��write request д�������handle��conn socket��  д��������buf
        uv_write_t write_req_;
        write_req_.data = tmp;//Ϊ��д���ͷſռ�
        
        //����connect socket��д�������һ����connect socketд�꣩����ص�after_write
        if (uv_write(&write_req_, connect_handle, &writebuf_, 1, after_write)) {
            printf("uv_write failed");
        }
    }
    //�Ѷ����������ͷ�
    free(buf->base);
}


void myLibuvClientCtx::after_write(uv_write_t* req, int status) {
    free(req->data);//�ͷ�д������
    if (status == 0){
        return;
    }else{
        printf("uv_write error: %s - %s\n",uv_err_name(status),uv_strerror(status));
    }
}