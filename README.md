## C++高并发服务器框架

主要参考自《Liunx多线程服务端编程：使用muduo C++网络库》，感谢。

#### 主体结构：

[Reactor模式](<https://www.cnblogs.com/crazymakercircle/p/9833847.html>) + [IO多路复用](https://www.zhihu.com/question/32163005/answer/55772739) + [非阻塞IO](https://www.zhihu.com/question/19732473)

主线程负责accept请求，以轮询调度的方式将请求分发给其他IO线程，IO线程负责管理请求。



#### 目前功能：

基于HTTP1.0协议实现了Web服务器，可处理静态资源。

- 实现了优雅关闭连接，平滑关闭服务器。
- 使用eventfd实现了跨线程唤醒，采用智能指针管理部分资源。
- 基于时间轮实现定时器功能，定时剔除不活跃连接。



#### 压力测试：



#### 等待升级：