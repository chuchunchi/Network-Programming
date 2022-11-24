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
string reg_usage = "Usage: register <username> <email> <password>";
string login_usage = "Usage: login <username> <password>";
string start_usage = "Usage: start-game <4-digit number>";
vector<int> client_sds (10,0);
vector<int> gametime (10,0);
vector<string> gameans (10,"");
string _register(string username,string email,string password);
string _login(string username,string password);
string _logout();
string _createpublic(string gameRoomID);
string _createprivate(string gameRoomID, string invitationCode);
string _listroom();
string _listuser();
string _startgame(string number);
string _exit();
string game(string guess);
vector<string> emails;
map<string, pair<string,string> > account; // {username: password,email}
vector<string> islogin(10,""); // username
vector<unsigned int> inRoom(10,0); //room ID
map<unsigned int, int> roomStart; // {RoomID: start or not }
map<unsigned int, int> roomIsPub; // {RoomID: is public or not}
string IOHandle(char *recvmsg){
	string sendback="";
	vector<string> command;
	command.push_back("");
	int para=0;
	for(int i=0;i<strlen(recvmsg);i++){
		if(recvmsg[i]=='\n'){
			break;
		}
		if(recvmsg[i]!=' '){
			command[para]+=recvmsg[i];
		}
		else{
			command.push_back("");
			//cou-t << command[para] << endl;
			para++;
		}
	}
	if(command[0]=="register"){
		if(command.size()!=4){
			sendback = reg_usage;
		}
		else sendback =_register(command[1],command[2],command[3]);
	}
	else if(command[0]=="login"){
		if(command.size()!=3){
			sendback = login_usage;
		}
		else sendback = _login(command[1],command[2]);	
	}
	else if(command[0]=="logout"){
		sendback = _logout();
	}
	else if(command[0]=="create"){
		if(command[1]=="public" && command[2]=="room" && command.size()==4){
			sendback = _createpublic(command[3]);
		}
		if(command[1]=="private" && command[2]=="room" && command.size()==5){
			sendback = _createprivate(command[3], command[4]);
		}
	}
	else if(command[0]=="list" && command[1]=="rooms"){
		sendback = _listroom();
	}
	else if(command[0]=="list" && command[1]=="users"){
		sendback = _listuser();
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
			sendback = _startgame(command[1]);
		}
		else{
			srand(time(NULL));
			int a = rand()%10000;
			sendback = _startgame(to_string(a));
		}
	}
	else if(command[0]=="exit"){
		sendback = _exit();
	}
	else if(gametime[currentindex]>0){
		if(command.size()==1){
			if(command[0].size()==4){
				try{stoi(command[0]);}
				catch(...){
					return "Your guess should be a 4-digit number.";
				}
				sendback = game(command[0]);
			}
			else sendback = "Your guess should be a 4-digit number.";
		}
		else sendback = "Your guess should be a 4-digit number.";
		
	}
    return sendback + '\n';
}
string _register(string username,string email,string password){
	string ret;
	map<string,pair<string,string> >::iterator itm;
	itm = account.find(username);
	vector<string>::iterator its;
	its = find(emails.begin(), emails.end(), email);
	if(itm!=account.end() || its!=emails.end()){
		ret = "Username or Email is already used";
	}
	else{
		account.insert(pair<string,pair<string,string> >(username,make_pair(password, email)));
		emails.push_back(email);
		ret = "Register Successfully";
	}
	return ret;
}

string _login(string username,string password){
	string ret;
	map<string,pair<string,string> >::iterator itm;
	itm = account.find(username);
	
	if(itm==account.end()){
		ret = "Username does not exist";
	}
	else if(islogin[currentindex]!=""){
		ret = "You already logged in as " + islogin[currentindex];
	}
	else if(password!=itm->second.first){
		ret = "Wrong password";
	}
	else{
		vector<string>::iterator it = find(islogin.begin(),islogin.end(),username);
		if(it != islogin.end()){
			ret = "Someone already logged in as " + username;
		}
		else{
			islogin[currentindex] = username;
			ret = "Welcome, " + username;
		}
	}
	return ret;
}

string _logout(){
	string ret;
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else if(inRoom[currentindex]!=0){
		ret = "You are already in game room " + to_string(inRoom[currentindex]) + ", please leave game room";
	}
	else{
		ret = "Goodbye, "+islogin[currentindex];
		islogin[currentindex] = "";
	}
	return ret;
}
string _createpublic(string strroomID){
	string ret;
	unsigned long tmp = stoul(strroomID);
	unsigned int roomID = tmp;
	vector<unsigned int>::iterator it = find(inRoom.begin(), inRoom.end(), roomID);
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else if(inRoom[currentindex]!=0){
		ret = "You are already in game room " + to_string(inRoom[currentindex]) + ", please leave game room";
	}
	else if(it!=inRoom.end()){
		ret = "Game room ID is used, choose another one";
	}
	else{
		inRoom[currentindex] = roomID;
		roomStart[roomID] = 0;
		roomIsPub[roomID] = 1;
		ret = "You create public game room " + strroomID;
	}
	return ret;
}
string _createprivate(string strroomID, string inviteCode){
	string ret;
	unsigned long tmp = stoul(strroomID);
	unsigned int roomID = tmp;
	unsigned long tmp2 = stoul(inviteCode);
	unsigned int code = tmp2; //TODO handle invite code
	vector<unsigned int>::iterator it = find(inRoom.begin(), inRoom.end(), roomID);
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else if(inRoom[currentindex]!=0){
		ret = "You are already in game room " + to_string(inRoom[currentindex]) + ", please leave game room";
	}
	else if(it!=inRoom.end()){
		ret = "Game room ID is used, choose another one";
	}
	else{
		inRoom[currentindex] = roomID;
		roomStart[roomID] = 0;
		roomIsPub[roomID] = 0;
		ret = "You create private game room " + strroomID;
	}
	return ret;
}
string _listroom(){
	string ret = "List Game Rooms";
	if(roomIsPub.size()==0){
		ret += "\nNo Rooms";
	}
	else{
		map<unsigned int, int>::iterator it1;
		map<unsigned int, int>::iterator it2 = roomStart.begin();
		int idx=0;
		for(it1=roomIsPub.begin();it1!=roomIsPub.end();it1++){
			string P = (it1->second)? ". (Public) Game Room " : ". (Private) Game Room ";
			string S = (it2->second)?  " is open for players" : " has started playing";
			ret += ('\n' + to_string(idx) + P + to_string(it1->first) + S);
			it2++;
			idx++;
		}
	}
	return ret;
}
string _listuser(){
	string ret = "List Users";
	if(account.size()==0){
		ret += "\nNo Users";
	}
	else{
		map<string, pair<string,string> >::iterator it;
		int idx=0;
		for(it=account.begin();it!=account.end();it++){
			string S = "";
			vector<string>::iterator its = find(islogin.begin(),islogin.end(),it->first);
			if(its != islogin.end()){
				S = "Online";
			}
			else{
				S = "Offline";
			}
			ret += ('\n' + to_string(idx) + ". " + it->first + "<" + it->second.second + "> " + S);
			idx++;
		}
	}
	return ret;
}

string _startgame(string number){
	string ret;
	gameans[currentindex] = number;
	if(islogin[currentindex]==""){
		ret = "Please login first.";
	}
	else{
		gametime[currentindex] = 5;
		ret = "Please typing a 4-digit number:";
	}
	return ret;
}

string game(string guess){
	string ret;
	int A=0,B=0;
	string ans = gameans[currentindex];
	if(ans==guess){
		ret = "You got the answer!";
		gametime[currentindex] = 0;
		gameans[currentindex] = "";
	}
	else{
		gametime[currentindex]--;
		for(int i=0;i<4;i++){
			if(ans[i]==guess[i]){
				A++;
				guess[i] = 'x';
				ans[i] = 'y';
			}
		}
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				if(ans[i]==guess[j]){
					B++;
					guess[j] = 'x';
					ans[i] = 'y';
					break;
				}
			}
		}
		ret = to_string(A)+"A"+to_string(B)+"B";
		if(gametime[currentindex]==0){
			gameans[currentindex] = "";
			ret += "\nYou lose the game!";
		}
	}
	return ret;
}
string _exit(){
	islogin[currentindex]="";	
	close(client_sds[currentindex]);
	client_sds[currentindex]=0;
	return "";
}

int main(int argc, char *argv[]){
	int portnum = 8888;
	if(argc==2){
		portnum = atoi(argv[1]);
		cout << portnum << endl;
	}
	
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
			char bufu[1024] = {};
			int r = recvfrom(udpFd,bufu,1024,MSG_WAITALL, (struct sockaddr *)&client_info,&info_size);
			if(r<=0){
				cout << "recv from udp error" <<'\n';
			}
			char* sendback;
			string ret = IOHandle(bufu);
			strcpy(sendback,ret.c_str());
			cout << sendback << endl;
			int s = sendto(udpFd,sendback,ret.size(),MSG_CONFIRM,(const struct sockaddr *) &client_info,info_size);
			
			if(s<=0) cout << "sent back error!\n";
		}
		//old connection's operation
		for(int i=0;i<10;i++){
			if(FD_ISSET(client_sds[i],&readfds)){
				currentindex = i;
				char buffer[1024] = {};
				int r = recv(client_sds[i],buffer,1024,0);
				cout << buffer << endl;
				if(r==0){
					islogin[currentindex] = "";
					close(client_sds[i]);
					client_sds[i] = 0;
				}
				else if(r==-1) continue;
				else{	
					char sendback[1024] = {};
					string ret = IOHandle(buffer);
					strcpy(sendback,ret.c_str());
					//cout << client_sds[i] << '\n';
					cout << sendback;
					send(client_sds[i],sendback,ret.size(),0);
				}
				memset(buffer, '\0', 1024);
			}
		}
	}	
}
