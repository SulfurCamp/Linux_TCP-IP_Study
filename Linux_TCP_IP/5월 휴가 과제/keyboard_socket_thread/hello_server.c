#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUFFSIZE 100
void error_handling(char *message);
int main(int argc, char *argv[])
{
	int serv_sock; // 서버 소켓 파일 디스크립터
	int clnt_sock; // 클라이언트 소켓 파일 디스크립터
	struct sockaddr_in serv_addr; // 서버 주소 구조체
	struct sockaddr_in clnt_addr; // 클라이언트 주소 구조체
	socklen_t clnt_addr_size; // 클라이언트 주소 크기 변수
	char message[BUFFSIZE];
	int str_len;
	// 포트인자 확인
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	// TCP 서버 소켓 구성 1. socket
	serv_sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(serv_sock < 0)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr)); // 서버 주소 구조체 초기화
	serv_addr.sin_family=AF_INET; // IPv4 설정
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY); // 모든 IP로부터의 접속 허용
	serv_addr.sin_port=htons(atoi(argv[1])); // 포트 설정(문자열을 숫자로)
	// 소켓에 주소 할당 TCP 소켓 구성 2. bind
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr) )< 0 )
		error_handling("bind() error"); 
	// 클라이언트 접속 대기 큐 설정 TCP 서버 소켓 구성 3. listen
	if(listen(serv_sock, 5) < 0)
		error_handling("listen() error");
	// 클라이언트 주소 크기 저장
	clnt_addr_size=sizeof(clnt_addr);  
	do {
		// 클라이언트 연결 수락 TCP 서버 소켓 구성 4. accept
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
		printf("clnt_sock : %d new client connected\n",clnt_sock);
		if(clnt_sock < 0)
			error_handling("accept() error");  
		do {
			// 클라이언트로부터 메시지 수신 TCP 서버 소켓 구성 5. read/write
			str_len = read(clnt_sock, message, sizeof(message)-1);
			if(str_len > 0)
			{
				message[str_len] = '\0';
				fputs(message,stdout);
				fputc('\n',stdout);
				// 클라이언트에게 메시지 전송 TCP 서버 소켓 구성 5. read/write
				write(clnt_sock, message, str_len);
			}
			else if(str_len == 0)
				break;
			else
				error_handling("read() error!");
		}while(1);
		printf("clnt_sock : %d client disconnected\n",clnt_sock);
		// 클라이언트 소켓 종료 TCP 서버 소켓 구성 6. close
		close(clnt_sock);
	}while(1);
	// 서버 소켓 종료 TCP 서버 소켓 구성 6. close
	close(serv_sock);
	return 0;
}
void error_handling(char *message)
{
	perror("error_handling()");
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
