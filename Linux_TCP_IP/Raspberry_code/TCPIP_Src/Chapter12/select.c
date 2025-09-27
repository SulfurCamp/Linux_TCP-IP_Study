#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
	fd_set reads, temps;
	int result, str_len;
	char buf[BUF_SIZE];
	struct timeval timeout;

	FD_ZERO(&reads);
	FD_SET(0, &reads); // 0 is standard input(console)

	/*
	timeout.tv_sec=5;
	timeout.tv_usec=5000;
	*/

	while(1)
	{
		temps=reads;
		timeout.tv_sec=0;
		timeout.tv_usec=500000; // 500000 * 10e-6 : 0.5sec
		result=select(1, &temps, 0, 0, &timeout);
					//(등록한 계수(fd+1), 읽기, 쓰기, 에러,타임아웃시간)
		if(result < 0)
		{
			puts("select() error!");
			perror("select()");
			break;
		}
		else if(result==0)
		{
			puts("Time-out!");
		}
		else 
		{
			if(FD_ISSET(0, &temps))
			{
				str_len=read(0, buf, BUF_SIZE);
				buf[str_len]=0;
				printf("message from console: %s", buf);
			}
		}
	}
	return 0;
}
