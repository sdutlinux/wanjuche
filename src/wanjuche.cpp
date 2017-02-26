#include <sys/types.h>
#include <sys/socket.h> // listen bind accept socket setsockopt
#include <netinet/in.h>
#include <arpa/inet.h>

/* Required by event.h. */
#include <sys/time.h>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstring> //memset
#include <fcntl.h>
#include <cerrno>
#include <err.h>

#include <iostream>

/* Libevent. */
#include <event2/event.h>

struct event_base *base = event_base_new();

struct client {
    struct event *ev_read;
};

int set_noblock(int fd) {
    int flags;

    flags = fcntl(fd,F_GETFL);
    if (flags < 0)
        return flags;
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
        return -1;

    return 0;
}

void on_read(int fd, short ev, void *arg) {
    struct client* client = (struct client*) arg;
    u_char buf[8196];
    int len,wlen;
    len = read(fd, buf, sizeof(buf));
    if (len == 0) {
        std::cout << "Client sidconnected." << std::endl;
        close(fd);
        event_del(client -> ev_read);
        free(client);
        return;
    } else if (len < 0) {
        std::cout << "Socket failure, disconnect client:" << std::endl;
        close(fd);
        event_del(client -> ev_read);
        free(client);
        return;
    }

    wlen = write(fd, buf, len);
    if (wlen < len) {
        std::cout << "Short write, not all data echoed back to client." << std::endl;
    }
}

void on_accept(int fd, short ev, void *arg) {
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct client *cli;
    client_fd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        warn("accept failed");
        return;
    }

    if (set_noblock(client_fd) < 0)
        warn("failed to set client socket non-blocking");

    cli = (client*)std::calloc(1, sizeof(*cli));
    if (cli == nullptr)
        err(1, "malloc failed");

    cli->ev_read = event_new(base,client_fd, EV_READ|EV_PERSIST, on_read, cli);

    event_add(cli -> ev_read, nullptr);
    printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr));
}

int main(int argc, char* argv[], char* envp[]) {
    int listen_fd;
    struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        err(1, "listen failed");

    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on,
                   sizeof(reuseaddr_on)) == -1)
        err(1, "setsockopt failed");
    std::memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(8888);
    if (bind(listen_fd, (struct sockaddr*)&listen_addr,sizeof(listen_addr)) < 0)
        err(1, "bind failed");
    if (listen(listen_fd,5) < 0)
        err(1, "listen failed");
    if(set_noblock(listen_fd) < 0)
        err(1, "failed to set server socket to non-blocking");

    struct event *ev_accept;
    ev_accept = event_new(base,listen_fd,EV_READ|EV_PERSIST,on_accept,nullptr);

    event_add(ev_accept,nullptr);
    event_base_dispatch(base);
    return 0;

}
