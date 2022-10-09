#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]){
	int IP = argv[1];
	int portnum = atoi(argv[2]);
	int udpFd = socket(AF_INET,SOCK_DGRAM,0);
        if(udpFd==-1) printf("socket create fail.");
        struct sockaddr_in info;
        bzero(&info,sizeof(info));
        info.sin_family = PF_INET;
        info.sin_addr.s_addr = inet_addr(IP);
        info.sin_port = htons(portnum);
	int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
	if(err==-1) printf("connect error");
	char receivemsg[];
	recv(udfFd,receivemsg,sizeof(receivemsg),0);
	printf("%s",receivemsg);

		
}
