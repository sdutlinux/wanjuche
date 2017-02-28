#ifndef _MASTER_H
#define _MASTER_H

#include <string>

#include "worker.h"
#include "mylibevent.h"


class Master
{
public:

    Master();
    ~Master();

    bool start_master();

    static void master_exit_signal(evutil_socket_t signo, short event, void *arg); //SIGINT信号回调函数

    static void master_chld_signal(evutil_socket_t signo, short event, void *arg); //SIGCHLD信号回调函数

    Worker              worker;
    typedef std::unique_ptr<event_base,EventBaseDeleter> unique_event_base;
    typedef std::unique_ptr<event,EventDeleter> unique_event;
    unique_event_base  m_base;
    unique_event       m_exit_event;
    unique_event       m_chld_event;

    int                 nums_of_child;  //子进程个数
};

#endif
