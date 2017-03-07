#include "master.h"
#include "worker.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>

Master::Master(int n_child) :
    m_base(nullptr),
    m_exit_event(nullptr),
    m_chld_event(nullptr),
    nums_of_child(n_child),
    worker(nullptr)
{
}

Master::~Master()
{
    std::cout << "Master Closed" << std::endl;
}


bool Master::start_master()
{
    std::cout << "Start Master" << std::endl;
    //创建一定数量的worker
    while (nums_of_child > 0)
    {
        switch (fork())
        {
            case -1:
                return false;
            case 0:
            {
                worker.reset(new Worker(ip,port));
                worker -> run(); //worker子进程入口
                return true;
            }
            default:
                --nums_of_child;
                break;
        }
    }

    m_base.reset(event_base_new());
    m_exit_event.reset(evsignal_new(m_base.get(), SIGINT, Master::master_exit_signal, m_base.get()));
    m_chld_event.reset(evsignal_new(m_base.get(), SIGCHLD, Master::master_chld_signal, this));

    //Master监听信号，一个用于退出，一个用于处理结束的子进程
    evsignal_add(m_exit_event.get(), nullptr);
    evsignal_add(m_chld_event.get(), nullptr);

    //开始事件循环
    event_base_dispatch(m_base.get());
    return true;
}

void Master::master_exit_signal(evutil_socket_t signo, short event, void *arg)
{
    //通知所有子进程。暂时不需要，因为程序不是守护进程。
    //所有子进程都跟终端关联，都会收到SIGINT
    //kill(0, SIGINT);

    //结束事件循环
    event_base_loopexit((struct event_base*)arg, nullptr);
    return;
}

//防止子进程僵死，使用waitpid而不是wait->可能多个子进程同时关闭
void Master::master_chld_signal(evutil_socket_t signo, short event, void *arg)
{
    Master *master = (Master *)arg;
    pid_t   pid;
    int     stat;
    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
    {
        ++(master->nums_of_child);
        std::cout << "Child " << pid << " terminated" << std::endl;
    }
}
