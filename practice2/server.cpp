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
map<string,int> account;
vector<string> islogin (10,"");
vector<string> IPs (10,"");
vector<uint16_t> Ports (10,0);
int inde=0;
int isexit=0;
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
	if(command[0]=="show-accounts"){
		sendback = "ACCOUNT1: " + to_string(account["ACCOUNT1"]) + "\nACCOUNT2: " + to_string(account["ACCOUNT2"]);
	}
	else if(command[0]=="deposit"){
		if(command.size()!=3){
			sendback = "Usage: deposit <account> <money>";
		}
		else{
			int money;
			try{money = stoi(command[2]);}
			catch(...){
				sendback = "Deposit a non-positive number into accounts.";
				return sendback;
			}
			if(money<=0){
				sendback = "Deposit a non-positive number into accounts.";
			}
			else{
				account[command[1]] += money;
				sendback = "Successfully deposits " + command[2] + " into " + command[1] + ".";
			}
		}
	}
	else if(command[0]=="withdraw"){
		if(command.size()!=3){
			sendback = "Usage: withdraw <account> <money>";
		}
		else{
			int money;
			try{money = stoi(command[2]);}
			catch(...){
				sendback = "Withdraw a non-positive number into accounts.";
				return sendback;
			}
			if(money<=0){
				sendback = "Withdraw a non-positive number into accounts.";
			}
			else if(money>account[command[1]]){
				sendback = "Withdraw excess money from accounts.";
			}
			else{
				account[command[1]] -= money;
				sendback = "Successfully withdraws " + command[2] + " from " + command[1] + ".";
			}
		}
	}
	else if(command[0]=="exit"){
		sendback = _exit();
	}
	sendback += "\n";
    return sendback;
}


string _exit(){
	string ret = "Bye "+ islogin[currentindex]+".";
	//islogin[currentindex]="";	
	isexit = 1;
	return ret;
}

int main(int argc, char *argv[]){
	int portnum = atoi(argv[1]);
	account["ACCOUNT1"] = 0;
	account["ACCOUNT2"] = 0;
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
			isexit=0;
			for(int i=0;i<10;i++){
				if(client_sds[i]!=0) continue;
				else{
					//put new client to an empty client sd
					client_sds[i] = new_client;
					islogin[i] = "user" + to_string(inde);
					ind[i] = inde;
					char msg[1024] = "Welcome, you are user";
					strcat(msg,to_string(inde).c_str());
					strcat(msg,".");
					send(new_client, msg, strlen(msg), 0);
					cout << "New connection from " << inet_ntoa(client_info.sin_addr) << ":" << ntohs(client_info.sin_port) << " user" << inde << endl;
					string tmp(inet_ntoa(client_info.sin_addr));
					IPs[i] = tmp;
					Ports[i] = ntohs(client_info.sin_port);
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
					cout << islogin[i] << " " << IPs[i]<<":"<<Ports[i] << " disconnected" << endl;
					islogin[currentindex] = "";
					close(client_sds[i]);
					client_sds[i] = 0;
				}
				else if(r==-1) continue;
				else{
					char sendback[1024];
					strcpy(sendback,IOHandle(buffer).c_str());
					send(client_sds[i],sendback,1024,0);
					if(isexit){
						cout << islogin[i] << " " << IPs[i]<<":"<<Ports[i] << " disconnected" << endl;
						islogin[currentindex] = "";
						close(client_sds[currentindex]);
						client_sds[currentindex]=0;
					}
				}
			}
		}
	}	
}
