#include "connection.h"
#include "worker.h"

#include<iostream>

Connection::Connection()
{
    con_worker = NULL;

    read_event = NULL;
    write_event= NULL;
}

Connection::~Connection()
{
    if (read_event && write_event)
    {
        event_free(read_event);
        event_free(write_event);
        std::cout << con_sockfd << " closed" << std::endl;
        close(con_sockfd);
    }
}

/* 删除worker中相应的con，并释放该con */
void Connection::FreeConnection(Connection *con)
{
    Worker *worker = con->con_worker;

    if (con->read_event && con->write_event)
    {
        Worker::ConnectionMap::iterator con_iter = worker->con_map.find(con->con_sockfd);
        worker->con_map.erase(con_iter);
    }

    delete con;
}

bool Connection::InitConnection(Worker *worker)
{
    con_worker = worker;

    try
    {
        //这里不能开太大，会爆内存！
        //后期可能需要在内存的使用上进行优化～
        con_intmp.reserve(10 * 1024);
        con_inbuf.reserve(10 * 1024);
        con_outbuf.reserve(10 * 1024);

        evutil_make_socket_nonblocking(con_sockfd);
        //test：监听读事件，从客户端读，然后回显
        read_event = event_new(con_worker->w_base, con_sockfd, EV_PERSIST | EV_READ, Connection::ConEventCallback, this);
        write_event = event_new(con_worker->w_base, con_sockfd, EV_PERSIST | EV_WRITE, Connection::ConEventCallback, this);
    }
    catch(std::bad_alloc)
    {
        std::cout << "InitConnection():bad_alloc" <<std::endl;
    }
    WantRead();

    return true;
}

/* 循环读写
 * 注意，在读的时候，此处ret为0时，可能是空字符串之类的
 * 所以在这里暂不做处理
 */
void Connection::ConEventCallback(evutil_socket_t sockfd, short event, void *arg)
{

    Connection *con = (Connection*)arg;

    if (event & EV_READ)
    {
        int cap = con->con_intmp.capacity();
        int ret = read(sockfd, &con->con_intmp[0], cap);

        if (ret == -1)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                FreeConnection(con);
                return;
            }
        }
        else if (ret == 0)
        {
            FreeConnection(con);
            return;
        }
        else
        {
            con->con_inbuf.clear();
            con->con_inbuf.append(con->con_intmp.c_str(), ret);
        }
        con->con_outbuf = con->con_inbuf;
        con->NotWantRead();
        con->WantWrite();
    }

    if (event & EV_WRITE)
    {
        int ret = write(sockfd, con->con_outbuf.c_str(), con->con_outbuf.size());

        if (ret == -1)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                FreeConnection(con);
                return;
            }
        }
        con->NotWantWrite();
        con->WantRead();
    }
}
