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
	evthread_use_windows_threads();//win������
#else
	evthread_use_pthreads();    //unix������
#endif

	//����event_base
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
	//sin.sin_addr.s_addr = inet_addr("192.168.31.243"); �������б�����ַ

	//����eventbase ����listen����������
	//������listener_cb�ص������������µ����ӵ�¼��ʱ��
	//����listener_cb
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

	//�����ն��źţ��������յ�SIGINT�����signal_cb
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

	if (!signal_event || event_add(signal_event, NULL)<0)
	{
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}
	//event_base��Ϣ�ɷ�
	event_base_dispatch(base);

	//�ͷ����ɵ�evconnlistener
	evconnlistener_free(listener);
	//�ͷ����ɵ��ź��¼�
	event_free(signal_event);
	//�ͷ�event_base
	event_base_free(base);

	printf("netServer Closed!\n");
	return 0;
}