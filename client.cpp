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
string reg_usage = "Usage: register <username> <email> <password>";
string login_usage = "Usage: login <username> <password>";
string start_usage = "Usage: start-game <4-digit number>";
string IOHandle(char * recvmsg,int * istcp){
	string sendback="";
	vector<string> command;
	command.push_back("");
	int para=0;
	cout << strlen(recvmsg) << endl;
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
		try{stoi(command[1])}
		catch(...){
			sendback = start_usage;
		}
		if(command.size()!=2) sendback = start_usage;
	}
	else if(command[0]=="game-rule"){
		*istcp = 0;
	}
    return sendback;
}
int main(int argc, char *argv[]){
	char * IP = argv[1];
	int portnum = atoi(argv[2]);
	// udp socket create
	int udpFd = socket(AF_INET,SOCK_DGRAM,0);
    if(udpFd==-1) printf("udp socket create fail.\n");
    struct sockaddr_in udpinfo;
    bzero(&udpinfo,sizeof(udpinfo));
	int erru = connect(udpFd,(struct sockaddr *)&udpinfo,sizeof(udpinfo));
	if(erru==-1) printf("connect error\n");

	//tcp socket create
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
		int istcp=1;
		string usage = IOHandle(command,&istcp);
		if(usage!=""){
			cout << usage << '\n';
			continue;
		}
		if(istcp){
			int s=send(tcpFd,command,sizeof(command),0);
			if(s==-1) cout << "send error\n";
			char receivemsg[1024];
			recv(tcpFd,receivemsg,sizeof(receivemsg),0);
			cout << receivemsg << '\n';
		}
		else{
			int s=send(udpFd,command,sizeof(command),0);
			if(s==-1) cout << "send error\n";
			char receivemsg[1024];
			recv(udpFd,receivemsg,sizeof(receivemsg),0);
			cout << receivemsg << '\n';
		}
	}	
}
