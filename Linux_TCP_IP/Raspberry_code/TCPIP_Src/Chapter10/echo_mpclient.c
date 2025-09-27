#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);
void read_childproc(int sig);

int cnt = 0;

int main(int argc, char *argv[])
{
	int sock;
	pid_t pid1, pid2;
	char buf[BUF_SIZE];
	struct sockaddr_in serv_adr;
	struct sigaction act;
	int state, status2;

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	act.sa_handler=read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	state=sigaction(SIGCHLD, &act, 0);
	
	sock=socket(PF_INET, SOCK_STREAM, 0);  
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	printf("%d", act.sa_flags);
	pid1=fork();
	if(pid1==0)
		write_routine(sock, buf);
	else{ 
		pid2=fork();
		if(pid2==0){
			read_routine(sock, buf);
		}
	}
	// printf("waitpid : %d",waitpid(-1, &status2, WNOHANG));
	while(cnt != 2);
	close(sock);
	return 0;
}

void read_childproc(int sig)
{
	cnt++;
	pid_t pid;
	int status;
	pid=waitpid(-1, &status, WNOHANG);
	printf("return : %d ", WEXITSTATUS(status));
	printf("removed proc id: %d \n", pid);
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