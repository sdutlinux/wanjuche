#ifndef _LISTENER_H
#define _LISTENER_H

#include <string>
#include <memory>

#include "mylibevent.h"

#include "util.h"

class Worker;

class Listener
{
public:
    typedef std::unique_ptr<event,EventDeleter> unique_event;
    typedef std::map<evutil_socket_t, std::unique_ptr<Connection>> ConnectionMap;
    Listener(const std::string &ip, unsigned short port);
    ~Listener();

    bool init_listener(Worker *worker);
    void add_listen_event();


};
#endif
