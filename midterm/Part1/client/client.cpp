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
vector<string> filelist;
int IOHandle(char * recvmsg){
	int ret = 0;
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
	if(command[0]=="exit"){
		ret = 1;
	}
	else if(command[0]=="get-file"){
		ret = 2;
		filelist.clear();
		for(int i=1;i<command.size();i++){
			filelist.push_back(command[i]);
		}
	}
	else if(command[0]=="get-file-list"){
		ret = 3;
	}
	return ret;
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
	
	while(1){ 
		char command[1024];
		cout << "% ";
		cin.getline(command,1024);
		int code = IOHandle(command);
		if(code==1){ //exit
			break;
		}
		socklen_t info_len = sizeof(info);
		sendto(udpFd,command,sizeof(command),MSG_CONFIRM,(const struct sockaddr *) &info, sizeof(info));
		if(code==3){
			char receivemsg[1024];
			recvfrom(udpFd,receivemsg,sizeof(receivemsg),MSG_WAITALL,(struct sockaddr *) &info,&info_len);
			cout << receivemsg << '\n';
			continue;
		}
		if(code==2){
			vector<char*> msgs;
			for(int i=0;i<filelist.size();i++){
				char receivemsg[1024];
				recvfrom(udpFd,receivemsg,sizeof(receivemsg),MSG_WAITALL,(struct sockaddr *) &info,&info_len);
				msgs.push_back(receivemsg);
				strcpy(receivemsg, msgs[i]);
				const char *filename = filelist[i].c_str();	
				FILE *fp = fopen(filename, "w");
				//fprintf(fp, "%s", receivemsg);
				fputs(receivemsg,fp);
				fclose(fp);
				bzero(receivemsg, 1024);
			}
		}
	}	
}
