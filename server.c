#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
int portnum;

void server_connection(char type_name[]){
	int sockfd=0;
	if(type_name=="UDP"){
		sockfd = socket(AF_INET,SOCK_DGRAM,0);
	}
	else if(type_name=="TCP"){
		sockfd = socket(AF_INET,SOCK_STREAM,0);
	}
	if(sockfd==-1) printf("socket create fail.");
	struct sockaddr_in info;
	bzero(&info,sizeof(info));
	info.sin_family = PF_INET;
	info.sin_addr.s_addr = inet_addr("127.0.0.1");
	info.sin_port = htons(portnum);
	int success = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
	
	if(success==0) printf("%s server is running.\n",type_name);
	else printf("%s server connect fail.\n",type_name);
	return;
}


int main(int argc, char *argv[]){
	portnum = atoi(argv[1]);
	server_connection("UDP");
	server_connection("TCP");
}
