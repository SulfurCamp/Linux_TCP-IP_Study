#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFFSIZE 100

char message[BUFFSIZE];
int str_len;
pthread_mutex_t mutex;

void *socket_send_msg(void *arg);
void *socket_read_msg(void *arg);
void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;

	int stdin_fd=fileno(stdin);

	pthread_t keyborad_thread, socket_send_thread, socket_read_thread;
	void *thread_return;	

	//printf("stdin_fd : %d\n",stdin_fd);

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	// printf("sock : %d\n", sock);
	if(sock < 0){
		error_handling("socket() error");
	}
	
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		error_handling("connect() error!");
	}

	pthread_create(&socket_send_thread, NULL, socket_send_msg, (void*)&sock);
	pthread_create(&socket_read_thread, NULL, socket_read_msg, (void*)&sock);
	pthread_join(socket_send_thread, &thread_return);
	pthread_join(socket_read_thread, &thread_return);

	close(sock);
	return 0;
}

void *socket_send_msg(void *arg){
	int sock = *((int *) arg);
	while(1){
		fputs("문자열을 입력하세요(quit:종료) : ",stdout);
		fgets(message, sizeof(message), stdin );
		// fputc('\n', stdout);
		str_len = strlen(message)-1;
		message[str_len] = '\0';	//'\n'제거
		if(!strcmp(message, "quit")){
			close(sock);
			exit(0);
		}
		str_len=write(sock,message,str_len) ;    //입력 문자열을 서버로 전송	
		if(str_len <= 0){
			error_handling("write() error");
		}
		
		usleep(200);
	}
	return NULL;
}

void *socket_read_msg(void *arg){
	int sock = *((int *) arg);
	while(1){
		str_len=read(sock, message, sizeof(message) - 1);
		if(str_len > 0)
		{
			message[str_len] = '\0';
			printf("Message from server: %s \n", message);
		}
		else if(str_len == 0){ //서버 소켓 종료시
			fputs("server closed\n", stdout);
			close(sock);
			exit(0);
		}
		else {
			error_handling("read() error!");
		}
	}
	return NULL;
}

void error_handling(char *message)
{
//	perror("error_handling()");
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}