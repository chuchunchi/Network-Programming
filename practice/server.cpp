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
#include <set>
#include <map>
#include <time.h>
#include <algorithm>
using namespace std;
int currentindex;
vector<int> client_sds (10,0);
string _exit();
string game(string guess);

vector<string> islogin (10,"");
vector<string> IPs (10,"");
vector<int> mutemode(10,0);
int inde=0;
vector<int> ind (10,0);
string IOHandle(char *recvmsg){
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
			//cout << command[para] << endl;
			para++;
		}
	}
	if(command[0]=="mute"){
		if(mutemode[currentindex]){
			sendback = "You are already in mute mode.";
		}
		else{
			mutemode[currentindex] = 1;
			sendback = "Mute mode.";
		}
		
	}
	else if(command[0]=="unmute"){
		if(!mutemode[currentindex]){
			sendback = "You are already in unmute mode.";
		}
		else{
			mutemode[currentindex] = 0;
			sendback = "Unmute mode.";
		}
		
	}
	else if(command[0]=="yell"){
		char sb[1024] = {};
		string recvstr = string(recvmsg);
		string ret = islogin[currentindex] + ": " + recvstr.substr(5) + ".";
		strcpy(sb,ret.c_str());
		for(int i=0;i<10;i++){
			if(i!=currentindex && client_sds[i]!=0 && mutemode[i]!=1){
				send(client_sds[i],sb,ret.size(),0);
			}
		}
	}
	else if(command[0]=="tell"){
		string rcver = command[1];
		char sb[1024] = {};
		string recvstr = string(recvmsg);
		string ret = islogin[currentindex] + " told you: " + recvstr.substr(11) + ".";
		strcpy(sb,ret.c_str());
		vector<string>::iterator itv = find(islogin.begin(),islogin.end(),rcver);
		if(itv==islogin.end()){
			sendback = rcver + " does not exist.";
		}
		else{
			send(client_sds[distance(islogin.begin(),itv)],sb,ret.size(),0);
		}
	}
	else if(command[0]=="exit"){
		sendback = _exit();
	}
    return sendback;
}


string _exit(){
	islogin[currentindex]="";
	close(client_sds[currentindex]);
	client_sds[currentindex] = 0;
	return "";
}

int main(int argc, char *argv[]){
	int portnum = atoi(argv[1]);
	
	struct sockaddr_in info,client_info;
	bzero(&info,sizeof(info));
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_port = htons(portnum);
	
	//tcp socket create
	int tcpFd = socket(AF_INET,SOCK_STREAM,0);
	cout << "TCP server is running\n";
	
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
			inde++;
			for(int i=0;i<10;i++){
				if(client_sds[i]!=0) continue;
				else{
					//put new client to an empty client sd
					client_sds[i] = new_client;
					islogin[i] = "user" + to_string(inde);
					ind[i] = inde;
					char msg[1024] = "Welcome, user";
					strcat(msg,to_string(inde).c_str());
					strcat(msg,".");
					send(new_client, msg, strlen(msg), 0);
					break;
				}
			}
		}
		//old connection's operation
		for(int i=0;i<10;i++){
			if(FD_ISSET(client_sds[i],&readfds)){
				currentindex = i;
				char buffer[1024];
				int r = recv(client_sds[i],buffer,1024,0);
				if(r==0){
					islogin[currentindex] = "";
					close(client_sds[i]);
					client_sds[i] = 0;
				}
				else if(r==-1) continue;
				else{
					char sendback[1024];
					strcpy(sendback,IOHandle(buffer).c_str());
					send(client_sds[i],sendback,1024,0);
				}
			}
		}
	}	
}
