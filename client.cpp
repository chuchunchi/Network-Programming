#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;
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
	char receivemsg[1024];
	//welcome to game
	recv(tcpFd,receivemsg,sizeof(receivemsg),0);
	cout << receivemsg << endl;
	while(1){
		char command[1024];
		cin.getline(command,1024);
		int s=send(tcpFd,command,sizeof(command),0);
		if(s==-1) cout << "send error\n";
		char receivemsg[1024];
		recv(tcpFd,receivemsg,sizeof(receivemsg),0);
		cout << receivemsg << '\n';
	}	
}
