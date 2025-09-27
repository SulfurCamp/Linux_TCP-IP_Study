#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

#define BUFFSIZE 100
void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char message[BUFFSIZE];
	int str_len;
	
	struct timeval timeout;
	int stdin_fd=fileno(stdin);
	fd_set reads, cpy_reads;
	socklen_t adr_sz;
	int fd_max, fd_num, i;

	// printf("stdin_fd : %d\n",stdin_fd);

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // socket의 파일 디스크립터를 전달

	// printf("sock = %d", sock);

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

	FD_ZERO(&reads);
	FD_SET(sock, &reads); // fd = 3
	FD_SET(stdin_fd, &reads); // fd = 0
	// FD_SET(1, &reads); // fd = 1
	fd_max = sock;
	int flag;
	do {
		cpy_reads = reads;
		timeout.tv_sec = 10;
		fputs("문자열을 입력하세요(quit:종료) : \n",stdout);
		fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout);
		if(fd_num > 0){
//  ---------------------- select fd:stdin_fd------------------------
			if(FD_ISSET(stdin_fd, &cpy_reads)){
				// printf("FD_ISSET(stdin_fd, &cpy_reads) : %d\n", FD_ISSET(stdin_fd, &cpy_reads));
				fgets(message, sizeof(message), stdin);
				str_len = strlen(message)-1;
				message[str_len] = '\0';	//'\n'제거

				if(!strcmp(message, "quit")){
					break;
				}
				
				str_len=write(sock,message,str_len) ;    //입력 문자열을 서버로 전송
				
				if(str_len <= 0){
					error_handling("write() error");
				}
				flag = 1;
			}
//  ------------------------------------------------------
//  ---------------------- select fd:sock ------------------------
			if(FD_ISSET(sock, &cpy_reads) || flag == 1){
				// printf("FD_ISSET(sock, &cpy_reads) : %d\n", FD_ISSET(sock, &cpy_reads));
				str_len=read(sock, message, sizeof(message)-1);
				if(str_len > 0)
				{
					message[str_len] = '\0';
					printf("Message from server: %s \n", message);  
				}
				else if(str_len == 0){ //서버 소켓 종료시
					fputs("socket server closed\n", stdout);
					break;
				}
				else{
					error_handling("read() error!");
				}
				flag = 0;
			}
//  ------------------------------------------------------
		}
		else if(fd_num == 0){ // time-out
			// fputs("time out", stdout);
			continue;
		}
		else{  //error
			puts("select() error!");
			perror("select()");
			break;
		}
	} while(1);
	shutdown(sock,SHUT_RDWR);
	close(sock);
	return 0;
}
void error_handling(char *message)
{
//	perror("error_handling()");
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
