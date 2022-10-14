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
int isexit=0;
string reg_usage = "Usage: register <username> <email> <password>";
string login_usage = "Usage: login <username> <password>";
string start_usage = "Usage: start-game <4-digit number>";
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
	if(command[0]=="register"){
		*istcp = 0;
		if(command.size()!=4){
			sendback = reg_usage;
		}
	}
	else if(command[0]=="login"){
		if(command.size()!=3){
			sendback = login_usage;
		}
	}
	else if(command[0]=="start-game"){
		if((command.size()!=1&&command.size()!=2)||(command.size()==2&&command[1].size()!=4)){
			sendback = start_usage;
			return sendback;
		}
		if(command.size()==2){
			try{stoi(command[1]);}
			catch(...){
				sendback = start_usage;
				return sendback;
			}
		}
	}
	else if(command[0]=="game-rule"){
		*istcp = 0;
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
	// udp socket create
	int udpFd = socket(AF_INET,SOCK_DGRAM,0);
	if(udpFd==-1) printf("udp socket create fail.\n");
	
	//tcp socket create
	int tcpFd = socket(AF_INET,SOCK_STREAM,0);
	if(tcpFd==-1) printf("socket create fail.\n");
	int err = connect(tcpFd,(struct sockaddr *)&info,sizeof(info));
	if(err==-1) printf("connect error\n");
	char receivemsg[1024];
	//welcome to game
	recv(tcpFd,receivemsg,sizeof(receivemsg),0);
	cout << receivemsg << '\n';
	while(1){ 
		char command[1024];
		cin.getline(command,1024);
		int istcp=1;
		string usage = IOHandle(command,&istcp);
		if(usage!=""){
			cout << usage << '\n';
			continue;
		}
		if(istcp){
			int s=send(tcpFd,command,sizeof(command),0);
			if(s==-1) cout << "send error\n";
			if(isexit){
				close(tcpFd);
				close(udpFd);
				break;
			}
			char receivemsg[1024];
			recv(tcpFd,receivemsg,sizeof(receivemsg),0);
			cout << receivemsg << '\n';
		}
		else{
			socklen_t info_len = sizeof(info);
			sendto(udpFd,command,sizeof(command),MSG_CONFIRM,(const struct sockaddr *) &info, sizeof(info));
			char receivemsg[1024];
			recvfrom(udpFd,receivemsg,sizeof(receivemsg),MSG_WAITALL,(struct sockaddr *) &info,&info_len);
			cout << receivemsg << '\n';
		}
	}	
}
