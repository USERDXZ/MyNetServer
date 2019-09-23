#include "HttpServer.h"
#include <sstream>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

const int MAXBUF = 8192;
//void* srcp;
//struct stat sbuf;

HttpServer::HttpServer(EventLoop* loop, int serverport, const int numthread, size_t timeout)
    :loop_(loop),
    TcpServer_(loop, serverport, numthread),
    Timer_(new TimerManager),
    timeout_(timeout)
{
    TcpServer_.SetMessageCallback(std::bind(&HttpServer::messageCallback, this, std::placeholders::_1, std::placeholders::_2));
    if (timeout != 0)
        TcpServer_.SetConnectionCallback(std::bind(&HttpServer::connectionCallback, this, std::placeholders::_1));
    //TcpServer_.SetsendCompleteCallback(std::bind(&HttpServer::sendCompleteCallback, this, std::placeholders::_1));
    
    /*内存映射方案
    std::string path_ = "./index.html";
    stat(path_.c_str(), &sbuf);
    int srcfd;
    if((srcfd = open(path_.c_str(), O_RDONLY, 0)) < 0)
        perror("open error");
    if((srcp = mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0)) == ((void*) -1))
        perror("mmap error");
    close(srcfd);
    */
}

HttpServer::~HttpServer()
{
    /*内存映射方案
    if(munmap(srcp, sbuf.st_size) < 0)
        perror("munmap error");
    */
}

void HttpServer::start()
{
    TcpServer_.start();
    Timer_->start();
    loop_->loop();
}

void HttpServer::messageCallback(const spTcpConnection& conn, std::string& msg)
{
    wpEntry wpEntry_(conn->getWpEntry());
    spEntry spEntry_(wpEntry_.lock());
    if(spEntry_)
        Timer_->AddToTimer(spEntry_, timeout_);

    struct _HttpRequestContext requestContext;
    std::string crlf("\r\n"), crlfcrlf("\r\n\r\n");
    size_t prev = 0, next = 0, pos_colon = 0;
    std::string key, value;

    if ((next = msg.find(crlf, prev)) != std::string::npos)
    {
        std::string first_line(msg.substr(prev, next - prev));
        prev = next;
        std::stringstream sstream(first_line);
        sstream >> (requestContext.method);
        sstream >> (requestContext.url);
        sstream >> (requestContext.version);
    }
    else
    {
        std::cout << "msg" << msg << std::endl;
        std::cout << "[INFO]Error in httpPraser: http_request_line isn't complete!" << std::endl;
        return ;
    }
    size_t pos_crlfcrlf = 0;
    if((pos_crlfcrlf = msg.find(crlfcrlf)) != std::string::npos)
    {
        while(prev != pos_crlfcrlf)
        {
            next = msg.find(crlf, prev+2);
            pos_colon = msg.find(":", prev + 2);
            key = msg.substr(prev + 2, pos_colon-prev-2);
            value = msg.substr(pos_colon + 2, next-pos_colon-2);
            prev = next;
            requestContext.header.insert(
                std::pair<std::string, std::string>(key, value));
        }
    }
    else{
        std::cout << "msg" << msg << std::endl;
        std::cout << "[INFO]Error in httpPraser: http_request_header isn't complete!" << std::endl;
        return ;
    }
    requestContext.body = msg.substr(pos_crlfcrlf + 4);
    msg.clear();

    std::string responsebody;
    std::string path;
    std::string querystring;
    std::string responseContext;

    if((requestContext.method != "GET") && (requestContext.method != "POST"))
    {
        //std::cout << "Method Not Implemented" << std::endl;
        HttpError(501, "Method Not Implemented", requestContext, responseContext);
        conn->send(responseContext);
        return ;
    }
    size_t pos = requestContext.url.find("?");
    if(pos != std::string::npos)
    {
        path = requestContext.url.substr(0, pos);
        querystring = requestContext.url.substr(pos+1);
    }else
    {
        path = requestContext.url;
    }
    
    /*长连接
    auto iter = requestContext.header.find("Connection");
    if(iter != httprequestcontext.header.end())
    {
        keepalive_ = (iter->second == "Keep-Alive");
    }
    else
    {
        if(httprequestcontext.version == "HTTP/1.1")
        {
            keepalive_ = true;//HTTP/1.1默认长连接
        }
        else
        {
            keepalive_ = false;//HTTP/1.0默认短连接
        }            
    }
    */

    if("/" == path)
    {
        path = "/index.html";
    }
    
    if(path[0] != '.')
        path.insert(0, ".");
    FILE* fp = NULL;
    if((fp = fopen(path.c_str(), "rb")) == NULL)
    {
        HttpError(404, "Not Found", requestContext, responseContext);
        return;
    }
    else
    {
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        while(fread(buffer, sizeof(buffer), 1, fp) == 1)  //可以mmap内存映射优化
        {
            responsebody.append(buffer);
            memset(buffer, 0, sizeof(buffer));
        }
        if(feof(fp))
        {
            responsebody.append(buffer);
        }        
        else
        {
            std::cout << "error fread" << std::endl;
        }        	
        fclose(fp);
    }
    /*内存映射方案
    struct stat sbuf;
    if(stat(path.c_str(), &sbuf) < 0)
    {
        HttpError(404, "Not Found", requestContext, responseContext);
        conn->send(responseContext);
        return ;
    }
    else
    {
       
        void* srcp;
        int srcfd;
        if((srcfd = open(path.c_str(), O_RDONLY, 0)) < 0)
            perror("open error");
        if((srcp = mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0)) == ((void*) -1))
            perror("mmap error");
        close(srcfd);
        responsebody.append((char*)srcp);
        if(munmap(srcp, sbuf.st_size) < 0)
            perror("munmap error");
    */
    std::string filetype("text/html"); 
    responseContext += requestContext.version + " 200 OK\r\n";
    responseContext += "Server: GuGu's NetServer/0.1\r\n";
    responseContext += "Content-Type: " + filetype + "; charset=utf-8\r\n";
    responseContext += "Connection: close\r\n";
    responseContext += "Content-Length: " + std::to_string(responsebody.size()) + "\r\n";
    responseContext += "\r\n";
    responseContext += responsebody;
    conn->send(responseContext);
}

void HttpServer::connectionCallback(const spTcpConnection& conn)
{
    std::shared_ptr<Entry> newEntry(new Entry(conn));
    Timer_->AddToTimer(newEntry, timeout_);

    wpEntry wpEntry_(newEntry);
    conn->setWpEntry(wpEntry_);
    //std::cout << "setWpEntry success" << std::endl;
}

void HttpServer::sendCompleteCallback(const spTcpConnection& conn)
{
}

void HttpServer::HttpError(const int err_num, const std::string short_msg, const _HttpRequestContext &httprequestcontext, std::string &responseContext)
{
    std::string responsebody;
    responsebody += "<html><title>出错了</title>";
    responsebody += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head>";
    responsebody += "<style>body{background-color:#f;font-size:14px;}h1{font-size:60px;color:#eeetext-align:center;padding-top:30px;font-weight:normal;}</style>";
    responsebody += "<body bgcolor=\"ffffff\"><h1>";
    responsebody += std::to_string(err_num) + " " + short_msg;
    responsebody += "</h1><hr><em> Chen Shuaihao's NetServer</em>\n</body></html>";

    std::string httpversion;
    if(httprequestcontext.version.empty())
    {
        httpversion = "HTTP/1.0";
    }
    else
    {
        httpversion = httprequestcontext.version;
    }   
    responseContext += httpversion + " " + std::to_string(err_num) + " " + short_msg + "\r\n";
    responseContext += "Server: Chen Shuaihao's NetServer/0.1\r\n";
    responseContext += "Content-Type: text/html\r\n";
    responseContext += "Connection: close\r\n";
    responseContext += "Content-Length: " + std::to_string(responsebody.size()) + "\r\n";
    responseContext += "\r\n";
    responseContext += responsebody;
}