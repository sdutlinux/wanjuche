#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <string>
#include <queue>

#include "event2/event.h"
#include "event2/util.h"

#include "util.h"

class Worker;

class Connection
{
public:
    Connection();
    ~Connection();

    bool InitConnection(Worker *worker);

    static void ConEventCallback(evutil_socket_t fd, short event, void *arg);

    Worker             *con_worker;

    evutil_socket_t     con_sockfd;
    struct event       *read_event;
    struct event       *write_event;

    std::string         con_inbuf;
    std::string         con_intmp;
    std::string         con_outbuf;

    static void FreeConnection(Connection *con);


private:
    void WantRead();
    void NotWantRead();
    void WantWrite();
    void NotWantWrite();

};

#endif
