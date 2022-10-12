#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]){
	char * IP = argv[1];
	int portnum = atoi(argv[2]);
	int tcpFd = socket(AF_INET,SOCK_STREAM,0);
        if(tcpFd==-1) printf("socket create fail.\n");
        struct sockaddr_in info;
        bzero(&info,sizeof(info));
        info.sin_family = AF_INET;
        info.sin_addr.s_addr = inet_addr(IP);
        info.sin_port = htons(portnum);
	int err = connect(tcpFd,(struct sockaddr *)&info,sizeof(info));
	if(err==-1) printf("connect error\n");
	char receivemsg[100];
	recv(tcpFd,receivemsg,sizeof(receivemsg),0);
	printf("%s\n",receivemsg);

		
}
