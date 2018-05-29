#include<stdio.h>    
#include <signal.h>
#include <netCore.h>

int main(int argc, char **argv)
{
	struct event_base *base;
	struct evconnlistener *listener;
	struct event *signal_event;

	struct sockaddr_in sin;

#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
	evthread_use_windows_threads();//win上设置
#else
	evthread_use_pthreads();    //unix上设置
#endif

	//创建event_base
	base = event_base_new();
	if (!base)
	{
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	evthread_make_base_notifiable(base);
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	//sin.sin_addr.s_addr = inet_addr("192.168.31.243"); 监听所有本机地址

	//基于eventbase 生成listen描述符并绑定
	//设置了listener_cb回调函数，当有新的连接登录的时候
	//触发listener_cb
	listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
		LEV_OPT_LEAVE_SOCKETS_BLOCKING|LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE
		| LEV_OPT_THREADSAFE, -1,
		(struct sockaddr*)&sin,
		sizeof(sin));

	if (!listener)
	{
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}

	//设置终端信号，当程序收到SIGINT后调用signal_cb
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

	if (!signal_event || event_add(signal_event, NULL)<0)
	{
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}
	//event_base消息派发
	event_base_dispatch(base);

	//释放生成的evconnlistener
	evconnlistener_free(listener);
	//释放生成的信号事件
	event_free(signal_event);
	//释放event_base
	event_base_free(base);

	printf("netServer Closed!\n");
	return 0;
}