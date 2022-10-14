#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <string>
#include <cstring>
using namespace std;
string _register(string username,string email,string password);
string _login(string username,string password);
string _logout();
string _gamerule();
string _startgame(int number);
string _exit();
string IOHandle(char *recvmsg){
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
			//cout << command[para] << endl;
			para++;
		}
	}
	if(command[0]=="register"){
		sendback =_register(command[1],command[2],command[3]);
	}
	else if(command[0]=="login"){
		sendback = _login(command[1],command[2]);	
	}
	else if(command[0]=="logout"){
		sendback = _logout();
	}
	else if(command[0]=="game-rule"){
		sendback = _gamerule();
	}
	else if(command[0]=="start-game"){
		sendback = _startgame(stoi(command[1]));
	}
	else if(command[0]=="exit"){
		sendback = _exit();
	}
    return sendback;
}

string _register(string username,string email,string password){
	return "reg";
}

string _login(string username,string password){
	return "log";
}

string _logout(){
	return "out";
}

string _gamerule(){
	return "*****Welcome to Game 1A2B*****\
		1. Each question is a 4-digit secret number.\
		2. After each guess, you will get a hint with the following information:\
		2.1 The number of \"A\", which are digits in the guess that are in the correct position.\
		2.2 The number of \"B\", which are digits in the guess that are in the answer but are in the wrong position.\
		The hint will be formatted as \"xAyB\".\
		3. 5 chances for each question.";
}

string _startgame(int number){
	return "start";
}

string _exit(){
	return "exit";
}

int main(int argc, char *argv[]){
	int portnum = atoi(argv[1]);
	vector<int> client_sds (10,0);
	struct sockaddr_in info,client_info;
	bzero(&info,sizeof(info));
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_port = htons(portnum);
	//udp socket create
	int udpFd = socket(AF_INET,SOCK_DGRAM,0);\
	if(udpFd==-1) printf("socket create fail.\n");
	int optu;
	int multiconnectu = setsockopt(udpFd,SOL_SOCKET,SO_REUSEADDR,(char *)&optu,sizeof(optu));
	if(multiconnectu==-1) printf("set sockopt fail!\n");
	
	int bu = bind(udpFd,(struct sockaddr *)&info,sizeof(info));
	if(bu==-1) printf("bind error\n");

	//tcp socket create
	int tcpFd = socket(AF_INET,SOCK_STREAM,0);
	char msg[40] = "*****Welcome to Game 1A2B*****";
	if(tcpFd==-1) printf("socket create fail.\n");
	int opt;
	int multiconnect = setsockopt(tcpFd,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(opt));
	if(multiconnect==-1) printf("set sockopt fail!\n");
	
	int b = bind(tcpFd,(struct sockaddr *)&info,sizeof(info));
	if(b==-1) printf("bind error\n");
	int l = listen(tcpFd,10);
	if(l==-1) printf("listen error\n");
	socklen_t info_size = sizeof(client_info);
	fd_set readfds;
	while(1){
		FD_ZERO(&readfds);
		FD_SET(tcpFd,&readfds);
		FD_SET(udpFd,&readfds);
		int maxsd = tcpFd;
		//add child socket to set
		for(int i=0;i<10;i++){
			if(client_sds[i]>0) FD_SET(client_sds[i],&readfds);
			if(client_sds[i]>maxsd) maxsd=client_sds[i];		
		}
		//select activity of one of the sockets
		int activity = select(maxsd+1,&readfds,NULL,NULL,NULL);
		if(activity==-1) printf("select error\n");
		//new connection
		if(FD_ISSET(tcpFd,&readfds)){
			int new_client = accept(tcpFd,(struct sockaddr *) &client_info, &info_size);
			if(new_client==-1) printf("connection error\n");
			else{
				printf("New connection.\n");
				send(new_client, msg, strlen(msg), 0);
			}
			for(int i=0;i<10;i++){
				if(client_sds[i]!=0) continue;
				else{
					//put new client to an empty client sd
					client_sds[i] = new_client;
					break;
				}
			}
		}
		if(FD_ISSET(udpFd,&readfds)){
			char bufu[1024];
			cout << "here!!\n";
			int r = recvfrom(udpFd,bufu,1024,MSG_WAITALL, (struct sockaddr *)&client_info,&info_size);
			if(r<=0){
				cout << "recv from udp error" <<'\n';
			}
			char sendback[1024];
			strcpy(sendback,"sendback");
			int s = sendto(udpFd,sendback,sizeof(sendback),MSG_CONFIRM,(const struct sockaddr *) &client_info,info_size);
			if(s<=0) cout << "sent back error!\n";
		}
		//old connection's operation
		for(int i=0;i<10;i++){
			if(FD_ISSET(client_sds[i],&readfds)){
				char buffer[1024];
				int r = recv(client_sds[i],buffer,1024,0);
				if(r==0){
					close(client_sds[i]);
					client_sds[i] = 0;
				}
				else if(r==-1) continue;
				else{
					char sendback[1024];
					cout << "iooutput" << IOHandle(buffer) << '\n';
					strcpy(sendback,IOHandle(buffer).c_str());
					//cout << client_sds[i] << '\n';
					send(client_sds[i],sendback,1024,0);
				}
			}
		}
	}	
}


