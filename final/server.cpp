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
		cout << mutemode[currentindex] << endl;
		if(mutemode[currentindex]==1){
			cout << "already" << endl;
			sendback = "You are already in mute mode.";
		}
		else{
			cout << "yes inside" << endl;
			mutemode[currentindex] = 1;
			sendback = "Mute mode.";
		}
		
	}
	else if(command[0]=="unmute"){
		cout << mutemode[currentindex] << endl;
		if(mutemode[currentindex]==0){
			cout << "already" << endl;
			sendback = "You are already in unmute mode.";
		}
		else{
			cout << "yes inside" << endl;
			mutemode[currentindex] = 0;
			sendback = "Unmute mode.";
		}
		
	}
	else if(command[0]=="yell"){
		char sb[1024] = {};
		string recvstr = string(recvmsg);
		string ret = islogin[currentindex] + ": " + recvstr.substr(5);
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
		string ret = islogin[currentindex] + " told you: " + recvstr.substr(6+command[1].size());
		strcpy(sb,ret.c_str());
		vector<string>::iterator itv = find(islogin.begin(),islogin.end(),rcver);
		int rcveridx = distance(islogin.begin(),itv);
		if(itv==islogin.end()){
			sendback = rcver + " does not exist.";
		}
		else if(rcver==islogin[currentindex]){
			sendback = "Can't send message to yourself.";
		}
		else if(mutemode[rcveridx]){
			sendback = "Receiver is muted.";
		}
		else{
			send(client_sds[rcveridx],sb,ret.size(),0);
		}
	}
	else if(command[0]=="exit"){
		sendback = _exit();
	}
	else{
		sendback = "Invalid input.";
	}
    return sendback;
}


string _exit(){
	islogin[currentindex]="";
	close(client_sds[currentindex]);
	client_sds[currentindex] = 0;
	mutemode[currentindex] = 0;
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
					
					string welcome = string("**************************\n")+"*Welcome to the BBS server.*\n"+"**************************\n"+"Welcome,user";
					char msg[1024] = {};
					strcpy(msg,welcome.c_str());
					strcat(msg,to_string(inde).c_str());
					strcat(msg,".\n");
					cout << msg << endl;
					send(new_client, msg, strlen(msg), 0);
					memset(msg, '\0', 1024);
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
					mutemode[currentindex] = 0;
				}
				else if(r==-1) continue;
				else{
					char sendback[1024] = {0};
					string ret = IOHandle(buffer);
					if(ret!=""){
						strcpy(sendback,ret.c_str());
						send(client_sds[i],sendback,1024,0);
					}
					
					memset(sendback, '\0', 1024);
				}
				memset(buffer, '\0', 1024);
			}
			
		}
	}	
}
