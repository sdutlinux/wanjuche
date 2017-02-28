#include "worker.h"
#include "master.h"
#include "util.h"
#include <iostream>

Worker::Worker(Master* m) :
    master(m),
    w_base(event_base_new()),
    w_exit_event(evsignal_new(w_base.get(), SIGINT, Worker::worker_exit_signal, w_base.get()))
{
}

Worker::~Worker()
{
    std::cout << "Worker closed" << std::endl;
}

void Worker::run()
{
    evsignal_add(w_exit_event.get(), nullptr);
    event_base_dispatch(w_base.get());
    return;
}

void Worker::worker_exit_signal(evutil_socket_t signo, short event, void *arg)
{
    event_base_loopexit((struct event_base*)arg, nullptr);
    return;
}
