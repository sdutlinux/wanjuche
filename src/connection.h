#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <string>
#include <memory>
#include "mylibevent.h"
#include "util.h"

class Worker;

class Connection
{
public:
    Connection();
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    bool init_connection(Worker *worker);

    static void con_event_callback(evutil_socket_t fd, short event, void *arg);
    typedef std::unique_ptr<event,EventDeleter> unique_event;

    Worker *con_worker;

    evutil_socket_t con_sockfd;
    unique_event read_event;
    unique_event write_event;

    std::string con_inbuf;
    std::string con_intmp;
    std::string con_outbuf;

    static void free_connection(Connection *con);


private:
    void want_read();
    void not_want_read();
    void want_write();
    void not_want_write();

};

#endif
