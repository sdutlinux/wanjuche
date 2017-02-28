#ifndef _LISTENER_H
#define _LISTENER_H

#include <string>

#include "mylibevent.h"

#include "util.h"

class Worker;

class Listener
{
public:
    Listener(const std::string &ip, unsigned short port);
    ~Listener();

    bool init_listener(Worker *worker);
    void add_listen_event();

    static void listen_event_callback(evutil_socket_t fd, short event, void *arg);

    Worker             *listen_worker;
    evutil_socket_t     listen_sockfd;
    struct sockaddr_in  listen_addr;
    struct event       *listen_event;
    uint64_t            cnt_connection;
};


#endif
