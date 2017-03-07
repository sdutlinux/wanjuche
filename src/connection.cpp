#include "connection.h"
#include "worker.h"

#include<iostream>

Connection::Connection():
    con_worker(nullptr),
    read_event(nullptr),
    write_event(nullptr)
{
}

Connection::~Connection()
{
    std::cout << con_sockfd << " closed" << std::endl;
    close(con_sockfd);
}

/* 删除worker中相应的con，并释放该con */
void Connection::free_connection(Connection *con)
{
    Worker *worker = con->con_worker;
    auto con_iter = worker->con_map.find(con->con_sockfd);
    if (con_iter != worker -> con_map.end()) {
        worker->con_map.erase(con_iter);
    }
}

bool Connection::init_connection(Worker *worker)
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
        read_event.reset(event_new(con_worker->w_base.get(), con_sockfd, EV_PERSIST | EV_READ, Connection::con_event_callback, this));
        write_event.reset(event_new(con_worker->w_base.get(), con_sockfd, EV_PERSIST | EV_WRITE, Connection::con_event_callback, this));
    }
    catch(std::bad_alloc)
    {
        std::cout << "InitConnection():bad_alloc" <<std::endl;
    }
    want_read();

    return true;
}

/* 循环读写
 * 注意，在读的时候，此处ret为0时，可能是空字符串之类的
 * 所以在这里暂不做处理
 */
void Connection::con_event_callback(evutil_socket_t sockfd, short event, void *arg)
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
                free_connection(con);
                return;
            }
        }
        else if (ret == 0)
        {
            free_connection(con);
            return;
        }
        else
        {
            con->con_inbuf.clear();
            con->con_inbuf.append(con->con_intmp.c_str(), ret);
        }
        con->con_outbuf = con->con_inbuf;
        con->not_want_read();
        con->want_write();
    }

    if (event & EV_WRITE)
    {
        int ret = write(sockfd, con->con_outbuf.c_str(), con->con_outbuf.size());

        if (ret == -1)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                free_connection(con);
                return;
            }
        }
        con->not_want_write();
        con->want_read();
    }
}

void Connection::want_read()
{
    event_add(read_event.get(), NULL);
}

void Connection::not_want_read()
{
    event_del(read_event.get());
}

void Connection::want_write()
{
    event_add(write_event.get(), NULL);
}

void Connection::not_want_write()
{
    event_del(write_event.get());
}
