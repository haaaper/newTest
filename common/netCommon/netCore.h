#ifndef  _NET_CORE_H_
#define _NET_CORE_H_
#include<string.h>  
#include<event2/event.h>    
#include<event2/listener.h>    
#include<event2/bufferevent.h>    
#include<event2/thread.h>
#include<event2/buffer.h>

const char MESSAGE[] = "Hello, NewConnection!\n";
const int PORT = 9995;
#define MAX_LINE_LENGTH (1024)

void read_cb(struct bufferevent *bev, void *user_data);
void conn_writecb(struct bufferevent *bev, void *user_data);
void conn_eventcb(struct bufferevent *bev, short events, void *user_data);

void signal_cb(evutil_socket_t sig, short events, void *user_data);
void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
	struct sockaddr *sa, int socklen, void *user_data);




#endif // !_NET_CORE_H_


