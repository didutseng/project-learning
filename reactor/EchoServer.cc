#include "Echoserver.h"
#include "TcpConnection.h"

#include <iostream>
#include <functional>

using std::cout;
using std::endl;
using std::bind;

MyTask::MyTask(const string &msg, const TcpConnectionPtr &con)
: _msg(msg)
, _con(con)
{

}
void MyTask::process()
{
    //所有的业务逻辑全部在该函数中进行处理
    //处理完毕之后需要将数据发出去
    _con->sendInLoop(_msg);
}

Echoserver::Echoserver(size_t threadNum, size_t queSize
                       , const string &ip
                       , unsigned short port)
: _pool(threadNum, queSize)
, _server(ip, port)
{

}

Echoserver::~Echoserver()
{

}

//服务器的启动与停止
void Echoserver::start()
{
    _pool.start();//线程池对象启动起来

    using namespace std::placeholders;
    _server.setAllCallback(std::bind(&Echoserver::onNewConnection, this, _1)
                           , std::bind(&Echoserver::onMessage, this, _1)
                           , std::bind(&Echoserver::onClose, this, _1));
    _server.start();//TcpServer对象启动起来
}

void Echoserver::stop()
{
    _pool.stop();
    _server.stop();
}

//三个回调
void Echoserver::onNewConnection(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has connected!!!" << endl;
}

void Echoserver::onMessage(const TcpConnectionPtr &con)
{
    //接收客户端的数据
    string msg = con->receive();
    cout << ">>>>recv msg from client " << msg << endl;

    //业务逻辑的处理如果比较复杂，可以业务逻辑的处理交个线程池
    MyTask task(msg, con);
    _pool.addTask(bind(&MyTask::process, task));
}

void Echoserver::onClose(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has closed!!!" << endl;
}
