#ifndef _WORKER_H
#define _WORKER_H

#include <string>
#include <memory>
#include <map>

#include "mylibevent.h"
#include "util.h"
#include "connection.h"

class Worker {
 public:
    Worker(const std::string &ip, unsigned short port);
    ~Worker();
    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;

    static void worker_exit_signal(evutil_socket_t signo, short event, void *arg);
    static void listen_event_callback(evutil_socket_t fd, short event, void *arg);

    void run();
    bool init_listener();
    void add_listen_event();
    bool init_connection();
    typedef std::unique_ptr<event_base,EventBaseDeleter> unique_event_base;
    typedef std::unique_ptr<event,EventDeleter> unique_event;
    typedef std::map<evutil_socket_t, std::unique_ptr<Connection>> ConnectionMap;
    unique_event_base w_base;
    unique_event w_exit_event;
    unique_event listen_event;
    sockaddr_in listen_addr;
    evutil_socket_t listen_sockfd;
    ConnectionMap con_map;
};

#endif
