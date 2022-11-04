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
#include <filesystem>
#include <fstream>
using namespace std;
namespace fs = std::filesystem;
vector<string> filelist;
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
	if(command[0]=="get-file-list"){
		string path = ".";
		sendback += "File: ";
		for (const auto & entry : fs::directory_iterator(path)){
			sendback += entry.path().filename();
			sendback += " ";
		}
	}
	else if(command[0]=="get-file"){
		filelist.clear();
		for(int i=1;i<command.size();i++)
		{
			filelist.push_back(command[i]);
			sendback = "";
			/*const char *filename = command[i].c_str();	
			FILE *fp = fopen(filename, "r");
			char content[1024];
			while(fgets(content, 1024, fp)!=NULL){
				string tmp(content);
				sendback += tmp;
			}*/
		}
	}
    return sendback;
}


int main(int argc, char *argv[]){
	int portnum = atoi(argv[1]);
	
	struct sockaddr_in info,client_info;
	bzero(&info,sizeof(info));
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_port = htons(portnum);
	//udp socket create
	int udpFd = socket(AF_INET,SOCK_DGRAM,0);
	cout << "UDP server is running\n";
	if(udpFd==-1) printf("socket create fail.\n");
	int optu;
	int multiconnectu = setsockopt(udpFd,SOL_SOCKET,SO_REUSEADDR,(char *)&optu,sizeof(optu));
	if(multiconnectu==-1) printf("set sockopt fail!\n");
	
	int bu = bind(udpFd,(struct sockaddr *)&info,sizeof(info));
	if(bu==-1) printf("bind error\n");
	
	socklen_t info_size = sizeof(client_info);
	fd_set readfds;
	while(1){
		FD_ZERO(&readfds);
		FD_SET(udpFd,&readfds);
		if(FD_ISSET(udpFd,&readfds)){
			char bufu[1024];
			int r = recvfrom(udpFd,bufu,1024,MSG_WAITALL, (struct sockaddr *)&client_info,&info_size);
			if(r<=0){
				cout << "recv from udp error" <<'\n';
			}
			
			if(IOHandle(bufu)!=""){
				char sendback[1024];
				strcpy(sendback,IOHandle(bufu).c_str());
				int s = sendto(udpFd,sendback,sizeof(sendback),MSG_CONFIRM,(const struct sockaddr *) &client_info,info_size);
				if(s<=0) cout << "sent back error!\n";
			}
			else{
				for(int i=0;i<filelist.size();i++){
					const char *filename = filelist[i].c_str();	
					FILE *fp = fopen(filename, "r");
					char sendback[1024]="";
					char tmp[1024]="";
					while(fgets(tmp, 1024, fp)!=NULL){
						strcat(sendback,tmp);
					}
					fclose(fp);
					int s = sendto(udpFd,sendback,sizeof(sendback),MSG_CONFIRM,(const struct sockaddr *) &client_info,info_size);
					if(s<=0) cout << "sent back error!\n";
				}
				
			}
		}
	}	
}
