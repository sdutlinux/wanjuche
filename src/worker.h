#ifndef _WORKER_H
#define _WORKER_H

#include <string>
#include <memory>

#include "mylibevent.h"

class Master;

class Worker {

public:
    Worker(Master* m);
    ~Worker();

    void run();
    static void worker_exit_signal(evutil_socket_t signo, short event, void *arg);
    typedef std::unique_ptr<event_base,EventBaseDeleter> unique_event_base;
    typedef std::unique_ptr<event,EventDeleter> unique_event;
    Master *master;
    unique_event_base w_base;
    unique_event w_exit_event;

    std::string s_inbuf;
    std::string s_intmp;
    std::string s_outbuf;
};

#endif
