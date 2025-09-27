#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>

#define BUF_SIZE 30

void error_handling(char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);

int child_count=0;

void read_childproc(int sig)
{
	int status;
	pid_t id;
	while((id = waitpid(-1,&status, WNOHANG)) > 0){
		if(WIFEXITED(status))
		{
			printf("removed proc id: %d, return: %d\n", id, WEXITSTATUS(status));
			child_count++;
		}
	}
}

int main(int argc, char *argv[])
{
	int sock;
	pid_t pid1, pid2;
	char buf[BUF_SIZE];
	struct sockaddr_in serv_adr;

	signal(SIGCHLD, read_childproc);

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);  
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");

	pid1=fork();
	if(pid1==0)
		write_routine(sock, buf);
	else
	{	
		pid2=fork();
		if(pid2 == 0)
			read_routine(sock, buf); // 遺 紐⑦봽濡쒖꽭    ㅽ뻾以  -->  먯떇 꾨줈 몄뒪濡   ㅽ뻾
	while(child_count<2)
		sleep(1);
	
	}
	close(sock);
	return 0;

}
void read_routine(int sock, char *buf)
{
	while(1)
	{
		int str_len=read(sock, buf, BUF_SIZE);
		if(str_len==0)
			exit(1);

		buf[str_len]=0;
		printf("Message from server: %s", buf);
	}
}
void write_routine(int sock, char *buf)
{
	while(1)
	{
		fgets(buf, BUF_SIZE, stdin);
		if(!strcmp(buf,"q\n") || !strcmp(buf,"Q\n"))
		{	
			shutdown(sock, SHUT_WR);
			exit(2);
			return;
		}
		write(sock, buf, strlen(buf));
	}
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
