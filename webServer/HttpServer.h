#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_

#include "TcpServer.h"
#include "Timer.h"
#include <string>
#include <string.h>

typedef struct _HttpRequestContext{
    std::string method;
    std::string url;
    std::string version;
    std::map<std::string, std::string> header;
    std::string body;
}HttpRequestContext;

/* 
typedef struct _HttpResponseContext {
    std::string version;
    std::string statecode;
    std::string statemsg;
	std::map<std::string, std::string> header;
	std::string body;
};
*/

class HttpServer
{
    typedef std::shared_ptr<TcpConnection> spTcpConnection;
    typedef std::shared_ptr<Entry> spEntry;
    typedef std::weak_ptr<Entry> wpEntry;
public:
    HttpServer(EventLoop* loop, int serverport, const int numthread, size_t timeout = 0);
    ~HttpServer();

    void start();

private:
    void messageCallback(const spTcpConnection& conn, std::string&);

    void connectionCallback(const spTcpConnection& conn);

    void sendCompleteCallback(const spTcpConnection& conn);

    void HttpError(const int err_num, const std::string short_msg, const _HttpRequestContext &httprequestcontext, std::string &responseContext);

    EventLoop* loop_;

    TcpServer TcpServer_;

    std::unique_ptr<TimerManager> Timer_;

    size_t timeout_;
};
#endif