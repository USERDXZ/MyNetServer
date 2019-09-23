/*
Copyright 2019.

Author: deng xiangzhou

version 1.0
*/

#include "EventLoop.h"
#include "HttpServer.h"
#include <signal.h>

static EventLoop* lp;

static void sighandler1(int sig_no)
{
    exit(0);
}

static void sighandler2(int sig_no)
{
    lp->quit();
}

int main(int argc, char *argv[])
{
    EventLoop loop;
    lp = &loop;

    signal(SIGUSR1, sighandler1);
    signal(SIGUSR2, sighandler2);
    signal(SIGINT, sighandler2);
    signal(SIGPIPE, SIG_IGN);             //服务器向关闭的连接发送信息会发送RST包，系统会给SIGPIPE信号导致服务器挂掉，这里忽略该信号

    int serverport = 8080;
    int numthread = 2;
    size_t timeout = 8;
    if(argc == 4)
    {
        serverport = atoi(argv[1]);
        numthread = atoi(argv[2]);
        timeout = atoi(argv[3]);
    }
    HttpServer server(&loop, serverport, numthread, timeout);
    server.start();
}