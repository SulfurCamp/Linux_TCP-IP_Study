#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h> // select 함수 헤더
#define BUFFSIZE 100

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char message[BUFFSIZE];
	int str_len;

	fd_set read_fds;
	int maxfd;

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
	// 검사 대상을 지정할 변수에 stdin또는 sock을 넣기 위한 연산자
	// 소켓으로 부터 값이 들어오면 sock, 키보드 입력이 들어오면 stdin_fd
	maxfd = sock > stdin_fd ? sock : stdin_fd;
	//	maxfd = sock + 1;

	fputs("문자열을 입력하세요(quit:종료) : \n", stdout);
	fflush(stdout);

	do {
//  ---------------------- select fd:stdin_sd------------------------
		// 파일 디스크립터 설정
		FD_ZERO(&read_fds);
		FD_SET(stdin_fd, &read_fds);
		FD_SET(sock, &read_fds);
		// select함수를 사용해서 여러 입력을 동시 감지
		// 키보드 입력, 서버로부터 오는 네트워크 데이터를 동시에 감지
		if(select(maxfd + 1, &read_fds,0,0,0) < 0)
				error_handling("select() error");
		// 키보드 입력이 호출이 되면 아래 실행문을 진행
		if(FD_ISSET(stdin_fd, &read_fds))
		{
			if(fgets(message, sizeof(message), stdin) <= 0)
				break;
			
			str_len= strlen(message)-1;//입력 문자열을 서버로 전송
			message[str_len]='\0';
				
			if(!strcmp(message, "quit"))
			{
				printf("클라이언트를 종료합니다.\n");
				break;
			}

			str_len=write(sock,message,str_len);
		
			if(str_len <= 0)
				error_handling("write() error");
		}
		// 소켓으로부터 데이터를 받으면 아래 실행문을 진행
		if(FD_ISSET(sock, &read_fds))
		{
			str_len = read(sock, message, sizeof(message)-1);
			if(str_len>0)
			{
				message[str_len]='\0';
				fputs("Message from server: ", stdout);
				fputs(message, stdout);
				fputc('\n', stdout);
				fputs("문자열을 입력하세요(quit:종료) : \n", stdout);
				fflush(stdout);
			}
			else if(str_len == 0)
			{
				printf("서버가 연결을 종료했습니다.\n");
				break;
			}
			else
				error_handling("read() error!");
		}

	}while(1);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	perror("error_handling()");
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
