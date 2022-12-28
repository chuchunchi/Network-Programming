#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <vector>
using namespace std;
int isexit = 0;
string IOHandle(char * recvmsg,int * istcp){
	string sendback="";
	vector<string> command;
	command.push_back("");
	int para=0;
	for(int i=0;i<strlen(recvmsg);i++){
		if(recvmsg[i]!=' '){
			command[para]+=recvmsg[i];
		}
		else{
			command.push_back("");
			para++;
		}
	}
	else if(command[0]=="exit"){
		isexit = 1;
	}
    return sendback;
}
int main(int argc, char *argv[]){
	char * IP = argv[1];
	int portnum = atoi(argv[2]);
	struct sockaddr_in info;
	bzero(&info,sizeof(info));
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = inet_addr(IP);
	info.sin_port = htons(portnum);
	
	//tcp socket create
	int tcpFd = socket(AF_INET,SOCK_STREAM,0);
	if(tcpFd==-1) printf("socket create fail.\n");
	int err = connect(tcpFd,(struct sockaddr *)&info,sizeof(info));
	if(err==-1) printf("connect error\n");
	char receivemsg[1024];
	
	fd_set readset;

	//recv(tcpFd,receivemsg,sizeof(receivemsg),0);
	//cout << receivemsg << '\n';
	while(1){ 
		cout << "% ";
		FD_SET(tcpFd, &readset);
		FD_SET(STDIN_FILENO, &readset);
		fdmax = tcpFd;
		if(select(fdmax+1,&readset,NULL,NULL,NULL)<0){
			printf(stdout, "select() error\n");
		}
		if(FD_ISSET(tcpFd,&readset)){
			char receivemsg[1024];
			recv(tcpFd,receivemsg,sizeof(receivemsg),0);
			cout << receivemsg << '\n';
		}
		if(FD_ISSET(STDIN_FILENO,&readset)){
			char command[1024] = {0};
			read(STDIN_FILENO, command, sizeof(command)-1);
			string usage = IOHandle(command,&istcp);
			int s=send(tcpFd,command,sizeof(command),0);
			if(s==-1) cout << "send error\n";
			
			if(isexit){
				close(tcpFd);
				break;
			}
		}
		
	}	
}
