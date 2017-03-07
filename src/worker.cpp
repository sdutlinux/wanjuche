#include "worker.h"
#include "util.h"
#include <iostream>

Worker::Worker(const std::string &ip, unsigned short port) :
    w_base(event_base_new()),
    w_exit_event(evsignal_new(w_base.get(), SIGINT, Worker::worker_exit_signal, w_base.get())),
    listen_event(nullptr)
{
    listen_addr.sin_family      = AF_INET;
    listen_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    listen_addr.sin_port        = htons(port);
}

Worker::~Worker()
{
    std::cout << "Worker closed" << std::endl;
}

void Worker::run()
{
    std::cout << "Start Worker" << std::endl;
    init_listener();
    evsignal_add(w_exit_event.get(), nullptr);
    event_base_dispatch(w_base.get());
    return;
}

void Worker::worker_exit_signal(evutil_socket_t signo, short event, void *arg)
{
    event_base_loopexit((struct event_base*)arg, nullptr);
    return;
}

void Worker::listen_event_callback(evutil_socket_t sockfd, short event, void *arg)
{
    evutil_socket_t con_fd;
    struct sockaddr_in con_addr;
    socklen_t addr_len  = sizeof(con_addr);
    if (-1 == (con_fd = accept(sockfd, (struct sockaddr*)&con_addr, &addr_len)))
    {
        //std::cout << "Thundering herd" <<std::endl;
        return ;
    }

    Worker *worker  = (Worker*)arg;
    std::unique_ptr<Connection> con(new Connection);

    con->con_sockfd = con_fd;

    pid_t pid = getpid();
    std::cout << "listen accept: " << con->con_sockfd << " by process " << pid <<std::endl;

    if (!con->init_connection(worker))
    {
        Connection::free_connection(con.get());
        return ;
    }

    con->con_worker->con_map[con->con_sockfd] = std::move(con);

}
bool Worker::init_listener()
{
    if (-1 == (listen_sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        return false;
    }

    //非阻塞
    evutil_make_socket_nonblocking(listen_sockfd);

    int reuse = 1;

    //重用
    setsockopt(listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (0 != bind(listen_sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)))
    {
        return false;
    }
    if (0 != listen(listen_sockfd, 5))
    {
        return false;
    }

    listen_event.reset(event_new(w_base.get(), listen_sockfd, EV_READ | EV_PERSIST, Worker::listen_event_callback, this));
    event_add(listen_event.get(), NULL);
    return true;
}
