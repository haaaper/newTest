#include <netCore.h>
void cli_read_cb(struct bufferevent *bev, void *ctx)
{
	char* msg = "林子大了什么鸟都有";
	struct evbuffer* input = bufferevent_get_input(bev);
	struct evbuffer* output = bufferevent_get_output(bev);

	char buffer[1024] = { 0 };
	int n = 0;
	while ((n = evbuffer_remove(input, buffer, sizeof(buffer))) > 0)
	{
		printf("cli_read_cb:%s\n", buffer);
	}

	//bufferevent_write(bev, msg, strlen(msg));  

	evbuffer_add(output, msg, strlen(msg));
}


void write_cb(struct bufferevent *bev, void *ctx)
{
	struct evbuffer *output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0)
	{
		//printf("flushed answer\n");  
		//bufferevent_free(bev);  
	}
}

void event_cb(struct bufferevent *bev, short what, void *ctx)
{
	if (what & BEV_EVENT_EOF)
	{
		printf("Connection closed.\n");
	}
	else if (what & BEV_EVENT_ERROR)
	{
		char sz[100] = { 0 };
		strerror_s(sz, 100, errno);
		printf("Got an error on the connection: %s\n",
			sz);/*XXX win32*/
	}
	else if (what & BEV_EVENT_TIMEOUT)
	{
		printf("Connection timeout.\n");
	}
	else if (what & BEV_EVENT_CONNECTED)
	{
		printf("Connect succeed\n");
		//客户端链接成功后，给服务器发送第一条消息    
		char *reply_msg = "hello,libeventserver!";
		bufferevent_write(bev, reply_msg, strlen(reply_msg));
		return;
	}
	/* None of the other events can happen here, since we haven't enabled
	* timeouts */
	bufferevent_free(bev);
}


int main()
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	evthread_use_windows_threads();//win上设置
#else
	evthread_use_pthreads();    //unix上设置
#endif

	struct sockaddr_in addr = { 0 };
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	char lszIp[20] = "192.168.31.243";
	addr.sin_addr.s_addr = inet_addr(lszIp);
	addr.sin_port = htons(9995);



	struct event_base* base = event_base_new();
	if (!base)
	{
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	evthread_make_base_notifiable(base);

	bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

	bufferevent_setcb(bev, cli_read_cb, write_cb, event_cb, NULL);

	if (0 != bufferevent_socket_connect(bev, (const sockaddr*)&addr, sizeof(addr)))
	{
		printf("connect to server failed! IP(%s) Port(%d)\n", lszIp, 9995);
		return 1;
	}

	int i  = bufferevent_enable(bev, EV_READ | EV_WRITE | EV_SIGNAL);

	i = event_base_dispatch(base);

	bufferevent_free(bev);

	event_base_free(base);

	return 0;
}