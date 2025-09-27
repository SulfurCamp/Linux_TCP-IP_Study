#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#define BUFFSIZE 100

void error_handling(char *message);
void* send_thread(void* arg);
void* recv_thread(void* arg);
// 전역변수 선언 
char message[BUFFSIZE];
int sock;
int str_len;

int main(int argc, char* argv[])
{
	struct sockaddr_in serv_addr;
	// 쓰레드를 생성하고 제어하기 위한 변수 선언
	// snd_thread : 키보드 입력 쓰레드
	// rcv_thread : 서버 수신 메시지 쓰레드
	pthread_t snd_thread, rcv_thread;

	int stdin_fd=fileno(stdin);
	printf("stdin_fd : %d\n",stdin_fd);

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	// 클라이언트 TCP  소켓 함수 호출 1. socket
	sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
		error_handling("socket() error");
	// 소켓 네트워크 서버환경 선언
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	// 클라이언트 TCP 소켓 함수 호출 2.connect
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)		
		error_handling("connect() error!");
	// 새로운 쓰레드 생성 각각 send_thread, recv_thread 함수를 수행
	pthread_create(&snd_thread, 0, send_thread, 0);
	pthread_create(&rcv_thread, 0, recv_thread, 0);
	// 쓰레드 종료를 기다리는 동기화 함수
	// 쓰레드가 종료될 때까지 main 함수가 대기하도록 한다.
	pthread_join(snd_thread, 0);
	pthread_join(rcv_thread, 0);

	close(sock);
	return 0;

}
// send_thread()함수는 클라이언트 프로그램에서 키보드 입력을 처리하고
// 입력값을 서버로 전송하는 역할을 수행
//  -------------------- keyboard_thread() ------------------------
void *send_thread(void *arg)
{
	fputs("문자열을 입력하세요(quit:종료) : ",stdout);
	fflush(stdout);

	do
	{
		if(fgets(message, sizeof(message), stdin ) <= 0)
		break;
		// 키보드로부터 입력받은 문자열에서 개행문자를 제거
		str_len = strlen(message);
		if(str_len > 0 && message[str_len-1] == '\n')
			message[str_len-1] ='\0';
		// quit입력을 받으면 종료
		if(!strcmp(message, "quit"))
		{	
			fputs("클라이언트를 종료합니다.\n", stdout);
			break;
		}

		str_len=write(sock,message,str_len) ;  //입력 문자열을 서버로 전송
		if(str_len <= 0)
		{
			error_handling("write() error");
			break;
		}

	}while(1);
	// 프로그램 종료
	exit(1);
	return NULL;
}
//  --------------------------------------------------------------
// 서버로부터 메시지를 읽는 역할을 수행하는 쓰레드
//  --------------------- socket_thread() ------------------------
void *recv_thread(void *arg)
{
	char message[BUFFSIZE];
	int str_len;
	
	do
	{
		// 서버로부터 온 메시지를 읽어서 변수에 저장
		str_len=read(sock, message, sizeof(message)-1);
		if(str_len > 0)
		{
			message[str_len] = '\0';
			fputs("Message from server: ",stdout);
			fputs(message, stdout);
			fputc('\n',stdout);
			fputs("문자열을 입력하세요.(종류는 quit): ", stdout);
			fflush(stdout);

		}
		else if(str_len == 0) //서버 소켓 종료시
		{
			fputs("서버 연결 종료되었습니다.", stderr);
			break;
		}
		else 
			error_handling("read() error!");
	}while(1);

	return NULL;
}
//  ------------------------------------------------------

void error_handling(char *message)
{
	perror("error_handling()");
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
