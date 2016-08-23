//
//  main.cpp
//  libuv_server
//
//  Created by 何宏洲 on 16/8/2.
//  Copyright © 2016年 何宏洲. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "uv.h"
#include "easylogging++.h"
#include "myLibuvServer.hpp"
INITIALIZE_EASYLOGGINGPP
#define DEFAULT_BACKLOG 10
uv_loop_t* loop;


int servertest(){
    
    myLibuvServer myserver;
    myserver.init("0.0.0.0", 8099);
    return myserver.Start();
    
}
//===========================//===========================//===========================
//uv_mutex_t mymutex;
//#include <unistd.h>
//float fibonacci(int n)
//{
//    if(n==1)
//        return 1;
//    if(n==2)
//        return 1;
//    return fibonacci(n-1)+fibonacci(n-2);
//}
//void fib(uv_work_t *req) {
//    int n = *(int *) req->data;
//    if (random() % 2)
//        sleep(1);
//    else
//        sleep(3);
//    long fib = 1;
//    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
//    {
//        uv_mutex_lock(&mymutex);
//        printf("fib : %d\n",uv_thread_self());
//        uv_mutex_unlock(&mymutex);
//    }
//}
//
//void after_fib(uv_work_t *req, int status) {
//    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *) req->data);
//    uv_mutex_lock(&mymutex);
//    printf("after_fib : %d\n",uv_thread_self());
//    uv_mutex_unlock(&mymutex);
//}
//#define FIB_UNTIL 10
//int eventlooptest(){
//    uv_mutex_init(&mymutex);
//    uv_mutex_lock(&mymutex);
//    printf("main : %d\n",uv_thread_self());
//    uv_mutex_unlock(&mymutex);
//    
//    loop = uv_default_loop();
//    
//    int data[FIB_UNTIL];
//    uv_work_t req[FIB_UNTIL];
//    int i;
//    for (i = 0; i < FIB_UNTIL; i++) {
//        data[i] = i;
//        req[i].data = (void *) &data[i];
//        uv_queue_work(loop, &req[i], fib, after_fib);
//    }
//    
//    return uv_run(loop, UV_RUN_DEFAULT);
//}
//===========================//===========================//===========================
//uv_loop_t *loop;
//uv_async_t async;
//void fake_download(uv_work_t *req) {
//    {
//        uv_mutex_lock(&mymutex);
//        printf("fake_download : %d\n",uv_thread_self());
//        uv_mutex_unlock(&mymutex);
//    }
//    int size = *((int*) req->data);
//    int downloaded = 0;
//    double percentage;
//    while (downloaded < size) {
//        percentage = downloaded*100.0/size;
//        async.data = (void*) &percentage;
//        uv_async_send(&async);
//        
//        sleep(1);
//        downloaded += (200+random())%1000; // can only download max 1000bytes/sec,
//        // but at least a 200;
//    }
//    
//}
//void print_progress(uv_async_t *handle) {
//    {
//        uv_mutex_lock(&mymutex);
//        printf("print_progress : %d\n",uv_thread_self());
//        uv_mutex_unlock(&mymutex);
//    }
//    double percentage = *((double*) handle->data);
//    fprintf(stderr, "Downloaded %.2f%%\n", percentage);
//}
//void after(uv_work_t *req, int status) {
//    {
//        uv_mutex_lock(&mymutex);
//        printf("after : %d\n",uv_thread_self());
//        uv_mutex_unlock(&mymutex);
//    }
//    fprintf(stderr, "Download complete\n");
//    uv_close((uv_handle_t*) &async, NULL);
//}
//int Inter_thread_communication(){
//    
//    uv_mutex_init(&mymutex);
//    uv_mutex_lock(&mymutex);
//    printf("main : %d\n",uv_thread_self());
//    uv_mutex_unlock(&mymutex);
//    
//    loop = uv_default_loop();
//    
//    uv_work_t req;
//    int size = 10240;
//    req.data = (void*) &size;
//    
//    uv_async_init(loop, &async, print_progress);
//    uv_queue_work(loop, &req, fake_download, after);
//    
//    return uv_run(loop, UV_RUN_DEFAULT);
//}
//===========================//===========================//===========================

#pragma comment(lib, "libuv.lib")
int main() {
//    return Inter_thread_communication();
//    return eventlooptest();
//    return idletest();
    return servertest();
    return 0;
}
