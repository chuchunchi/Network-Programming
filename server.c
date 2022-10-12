#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main(int argc, char *argv[]){
	int portnum = atoi(argv[1]);
	int tcpFd = socket(AF_INET,SOCK_STREAM,0);
	char* msg = "*****Welcome to Game 1A2B*****";
	if(tcpFd==-1) printf("socket create fail.\n");
	struct sockaddr_in info,client_info;
	bzero(&info,sizeof(info));
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_port = htons(portnum);
	int b = bind(tcpFd,(struct sockaddr *)&info,sizeof(info));
	if(b==-1) printf("bind error\n");
	int l = listen(tcpFd,10);
	if(l==-1) printf("listen error\n");
	int info_size = sizeof(client_info);
	int new_client = accept(tcpFd,(struct sockaddr *) &client_info, &info_size);
	if(new_client==-1) printf("connection error\n");
	else{
		printf("New connection.\n");
		send(new_client, msg, strlen(msg), 0); 
	}	
}
