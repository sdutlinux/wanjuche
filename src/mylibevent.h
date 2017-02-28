#ifndef _MYLIBEVENT_H_
#define _MYLIBEVENT_H_
#include <event2/event.h>
#include <event2/util.h>

class EventBaseDeleter {
public:
    void operator()(event_base* ptr) const {
        event_base_free(ptr);
    }
};

class EventDeleter {
public:
    void operator()(event* ptr) const {
        event_free(ptr);
    }
};
#endif// _MYLIBEVENT_H_
