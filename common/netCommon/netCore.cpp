#include "netCore.h"


void conn_writecb(struct bufferevent *bev, void *user_data)
{
	//ȡ��bufferevent ��output����
	struct evbuffer *output = bufferevent_get_output(bev);
	//����Ϊ0����ôд��ϣ��ͷſռ�
	/*if (evbuffer_get_length(output) == 0)
	{
		printf("flushed answer\n");
		bufferevent_free(bev);
	}*/
}

//������Ϊ�¼��ص�������д�Լ���Ҫ���Ĺ��ܾ���
//���ǵ��ͷ�buffevent�ռ�
void conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
	if (events & BEV_EVENT_EOF)
	{
		printf("Connection closed.\n");
	}
	else if (events & BEV_EVENT_ERROR)
	{
		printf("Got an error on the connection: %d\n",
			(errno));/*XXX win32*/
	}
	else if (BEV_EVENT_TIMEOUT | BEV_EVENT_READING == events)
	{
		printf("read timeOut  %d\n",(errno));/*XXX win32*/
	}
	else if (BEV_EVENT_TIMEOUT | BEV_EVENT_WRITING == events)
	{
		printf("write timeOut  %d\n", (errno));/*XXX win32*/
	}

	bufferevent_free(bev);
}

//����׽���źź����baseloop��ֹ
void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = { 2, 0 };

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}


void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
	struct sockaddr *sa, int socklen, void *user_data)
{
	//��sockaddrǿ��ת��Ϊ sockaddr_in
	sockaddr_in sin;
	memcpy(&sin, sa, sizeof(sin));
	// ȡ��ip�Ͷ˿ں�
	printf("accept connect from %s %d\n", inet_ntoa(sin.sin_addr), sin.sin_port);
	
	struct event_base *base = (struct event_base *)user_data;
	struct bufferevent *bev;

	//����һ��bufferevent�����ڶ�����д
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE| BEV_OPT_THREADSAFE);
	if (!bev)
	{
		fprintf(stderr, "Error constructing bufferevent!");
		//event_base_loopbreak(base); δ֪��Ϊ ���ַ������з�
		return;
	}
	//������д�ص��������¼��Ļص�����
	bufferevent_setcb(bev, read_cb, conn_writecb, conn_eventcb, NULL);

	//���ó�ʱ�¼�
	//���ö�д��ʱʱ�� 10s
	struct timeval loReadTimeOuter = { 10, 0 };
	struct timeval loWriteTimeOuter = { 10, 0 };
	bufferevent_set_timeouts(bev, &loReadTimeOuter, &loWriteTimeOuter);

	//bufferevent����д�¼��ص�
	bufferevent_enable(bev, EV_WRITE| EV_READ| EV_SIGNAL);
}

void read_cb(bufferevent *bev, void *user_data)
{
	char lszMsg[MAX_LINE_LENGTH + 1] = { 0 };
	int liLen = 0;
	int liWriteLen = 0;
	evutil_socket_t fd = bufferevent_getfd(bev);
	while (liLen = bufferevent_read(bev, lszMsg, MAX_LINE_LENGTH), liLen > 0)
	{
		lszMsg[liLen] = '\0';
		printf("fd=%u, read len = %d read msg: %s\n", fd, liLen, lszMsg);
		liWriteLen = bufferevent_write(bev, lszMsg, liLen);
		if (-1 == liWriteLen)
		{
			printf("[bufferevent_write]:error occur!\n");
		}
	}
}