#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main(int argc, char *argv[]){
	int portnum = atoi(argv[1]);
	int udpFd = socket(AF_INET,SOCK_DGRAM,0);
	char* msg = "*****Welcome to Game 1A2B*****";
	if(udpFd==-1) printf("socket create fail.");
	struct sockaddr_in info,client_info;
	bzero(&info,sizeof(info));
	info.sin_family = PF_INET;
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_port = htons(portnum);
	bind(udpFd,(struct sockaddr *)&info,sizeof(info));
	listen(udpFd,5);
	int new_client = accept(udpFd,(struct sockaddr *) client_info, &sizeof(client_info));
	if(new_client==-1) printf("connection error");
	else{
		printf("New connection.");
		send(new_client, msg, strlen(msg), 0); 
	}	
}
