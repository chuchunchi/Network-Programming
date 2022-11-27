#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <queue>
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
map<unsigned int, string> gameans;
map<unsigned int, int> gameround;
map<unsigned int, int> curplayer; // index in userinroom vector
string _register(string username,string email,string password);
string _login(string username,string password);
string _logout();
string _createpublic(string gameRoomID);
string _createprivate(string gameRoomID, string invitationCode);
string _listroom();
string _listuser();
string _joinroom(string gameRoomID);
string _invite(string email);
string _listinvite();
string _accept(string email, string code);
string _leaveroom();
string _startgame(string rounds, string number);
string _guess(string number);
string _exit();
string game(string guess, unsigned int roomID);
vector<string> emails;
map<string, pair<string,string> > account; // {username: password,email}
vector<string> islogin(10,""); // username
vector<unsigned int> inRoom(10,0); //room ID
map<unsigned int, int> roomStart; // {RoomID: start or not }
map<unsigned int, int> roomIsPub; // {RoomID: is public or not}
map<unsigned int, vector<int> > userinroom; // {RoomID: [room's user's idx]}
vector<unsigned int> ismanager(10,0); // invitation code
map<string, vector<int> > invitations; // {username: inviter's idx}
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
	else if(command[0]=="join" && command[1]=="room"){
		sendback = _joinroom(command[2]);
	}
	else if(command[0]=="invite"){
		sendback = _invite(command[1]);
	}
	else if(command[0]=="list" && command[1]=="invitations"){
		sendback = _listinvite();
	}
	else if(command[0]=="accept"){
		sendback = _accept(command[1], command[2]);
	}
	else if(command[0]=="leave" && command[1]=="room"){
		sendback = _leaveroom();
	}
	else if(command[0]=="start" && command[1]=="game"){
		if(command.size()==4){
			sendback = _startgame(command[2],command[3]);
		}
		else{
			srand(time(NULL));
			int a = rand()%10000;
			sendback = _startgame(command[2],to_string(a));
		}
	}
	else if(command[0]=="guess"){
		sendback = _guess(command[1]);
	}
	else if(command[0]=="exit"){
		sendback = _exit();
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
		curplayer[roomID] = 0;
		roomIsPub[roomID] = 1;
		vector<int> tmp;
		tmp.push_back(currentindex);
		userinroom[roomID] = tmp;
		ismanager[currentindex] = 1;
		ret = "You create public game room " + strroomID;
	}
	return ret;
}
string _createprivate(string strroomID, string inviteCode){
	string ret;
	unsigned long tmp = stoul(strroomID);
	unsigned int roomID = tmp;
	unsigned long tmp2 = stoul(inviteCode);
	unsigned int code = tmp2;
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
		curplayer[roomID] = 0;
		vector<int> tmp;
		tmp.push_back(currentindex);
		userinroom[roomID] = tmp;
		ismanager[currentindex] = code;
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
			string S = (it2->second)?  " has started playing" : " is open for players";
			ret += ('\n' + to_string(idx+1) + P + to_string(it1->first) + S);
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
			ret += ('\n' + to_string(idx+1) + ". " + it->first + "<" + it->second.second + "> " + S);
			idx++;
		}
	}
	return ret;
}
string _joinroom(string strroomID){
	unsigned long tmp = stoul(strroomID);
	unsigned int roomID = tmp;
	string ret = "";
	string ret2other = "";
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else if(inRoom[currentindex]!=0){
		ret = "You are already in game room " + to_string(inRoom[currentindex]) + ", please leave game room";
	}
	else{
		map<unsigned int, int>::iterator itm;
		itm = roomIsPub.find(roomID);
		if(itm==roomIsPub.end()){
			ret = "Game room " + strroomID + " is not exist";
		}
		else if(itm->second==0){
			ret = "Game room is private, please join game by invitation code";
		}
		else if(roomStart[roomID]){
			ret = "Game has started, you can't join now";
		}
		else{ //success
		inRoom[currentindex] = roomID;
			ret = "You join game room " + strroomID;
			ret2other = "Welcome, " + islogin[currentindex] + " to game!\n";
			for(int i=0;i<userinroom[roomID].size();i++){
				char sendback[1024] = {};
				strcpy(sendback,ret2other.c_str());
				send(client_sds[userinroom[roomID][i]],sendback,ret2other.size(),0);
			}
			userinroom[roomID].push_back(currentindex);
		}
	}
	return ret;
}
string _invite(string email){
	vector<string>::iterator ite = find(emails.begin(), emails.end(), email);
	int invitee_idx = distance(emails.begin(),ite);
	string ret;
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else if(inRoom[currentindex]==0){
		ret = "You did not join any game room";
	}
	else if(!ismanager[currentindex] || roomIsPub[inRoom[currentindex]]){
		ret = "You are not private game room manager";
	}
	else if(islogin[invitee_idx]==""){
		ret = "Invitee not logged in";
	}
	else{
		ret = "You send invitation to " + islogin[invitee_idx] + "<" + email + ">";
		string ret2invitee = "You receive invitation from " + islogin[currentindex] + "<" + emails[currentindex] + ">\n";
		invitations[islogin[invitee_idx]].push_back(currentindex);
		char sendback[1024] = {};
		strcpy(sendback, ret2invitee.c_str());
		send(client_sds[invitee_idx],sendback,ret2invitee.size(),0);
	}
	return ret;
}
string _listinvite(){
	string ret = "";
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else{
		ret = "List invitations";
		if(invitations[islogin[currentindex]].size()==0){
			ret += "\nNo Invitations";
		}
		else{
			//TODO: in ascending order of GameRoomID
			/*for(int i=0;i<invitations[islogin[currentindex]].size();i++){
				int inviter_idx = invitations[islogin[currentindex]][i];
				ret += ('\n' + to_string(i) + ". " + islogin[inviter_idx] + "<" + emails[inviter_idx] + "> invite you to join game room " + to_string(inRoom[inviter_idx]) + ", invitation code is " + to_string(ismanager[inviter_idx]));
			}*/
			set < pair<unsigned int, int>, less<pair<unsigned int, int> > > s;
			for(int i=0;i<invitations[islogin[currentindex]].size();i++){
				int inviter_idx = invitations[islogin[currentindex]][i];
				s.insert(make_pair(inRoom[inviter_idx],inviter_idx));
			}
			set<pair<unsigned int, int> >::iterator itset = s.begin();
			int idx = 0;
			for(;itset!=s.end();itset++){
				int inviter_idx = itset->second;
				ret += ('\n' + to_string(++idx) + ". " + islogin[inviter_idx] + "<" + emails[inviter_idx] + "> invite you to join game room " + to_string(inRoom[inviter_idx]) + ", invitation code is " + to_string(ismanager[inviter_idx]));
			}
		}
	}
	return ret;
}
string _accept(string email, string strcode){
	string ret = "";
	unsigned long tmp = stoul(strcode);
	unsigned int code = tmp;
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else if(inRoom[currentindex]!=0){
		ret = "You are already in game room " + to_string(inRoom[currentindex]) + ", please leave game room";
	}
	else{
		int notexist = 1;
		int inviter_idx;
		for(int i=0;i<invitations[islogin[currentindex]].size();i++){
			inviter_idx = invitations[islogin[currentindex]][i];
			if(emails[inviter_idx]==email){
				notexist = 0;
				break;
			}
		}
		if(notexist){
			ret = "Invitation not exist";
		}
		else if(ismanager[inviter_idx]!=code){
			ret = "Your invitation code is incorrect";
		}
		else if(roomStart[inRoom[inviter_idx]]){
			ret = "Game has started, you can't join now";
		}
		else{ //success
			unsigned int roomID = inRoom[inviter_idx];
			ret = "You join game room " + to_string(roomID);
			string ret2other = "Welcome, " + islogin[currentindex] +" to game!\n";
			for(int i=0;i<userinroom[roomID].size();i++){
				char sendback[1024] = {};
				strcpy(sendback,ret2other.c_str());
				send(client_sds[userinroom[roomID][i]],sendback,ret2other.size(),0);
			}
			inRoom[currentindex] = roomID;
			userinroom[roomID].push_back(currentindex);
		}
	}
	return ret;
}
string _leaveroom(){
	string ret = "";
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else if(inRoom[currentindex]==0){
		ret = "You did not join any game room";
	}
	else{ //success
		unsigned int roomID = inRoom[currentindex];
		string ret2other = "";
		if(ismanager[currentindex]!=0){
			ret = "You leave game room " + to_string(roomID);
			ret2other = "Game room manager leave game room " + to_string(roomID) + ", you are forced to leave too\n";
			for(int i=0;i<userinroom[roomID].size();i++){
				int user_idx = userinroom[roomID][i];
				inRoom[user_idx] = 0;
				if(user_idx!=currentindex){
					char sendback[1024] = {};
					strcpy(sendback,ret2other.c_str());
					send(client_sds[user_idx],sendback,ret2other.size(),0);
				}
			}
			//TODO invitations
			if(!roomIsPub[roomID]){
				map<string, vector<int> >::iterator itm;
				for(itm=invitations.begin();itm!=invitations.end();itm++){
					vector<int>::iterator it = find(itm->second.begin(),itm->second.end(),currentindex);
					if(it!=itm->second.end()){
						itm->second.erase(it);
					}
				}
			}
			inRoom[currentindex] = 0;
			roomIsPub.erase(roomID);
			roomStart.erase(roomID);
			userinroom.erase(roomID);
			ismanager[currentindex] = 0;
			
		}
		else if(roomStart[roomID]){
			ret = "You leave game room " + to_string(roomID) + ", game ends";
			ret2other = islogin[currentindex] + " leave game room " + to_string(roomID) + ", game ends\n";
			for(int i=0;i<userinroom[roomID].size();i++){
				int user_idx = userinroom[roomID][i];
				if(user_idx!=currentindex){
					char sendback[1024] = {};
					strcpy(sendback,ret2other.c_str());
					send(client_sds[user_idx],sendback,ret2other.size(),0);
				}
			}
			vector<int>::iterator itv = find(userinroom[roomID].begin(),userinroom[roomID].end(),currentindex);
			inRoom[currentindex] = 0;
			userinroom[roomID].erase(itv);
			roomStart[roomID] = 0;
			curplayer[roomID] = 0;
		}
		else{
			ret = "You leave game room " + to_string(roomID);
			ret2other = islogin[currentindex] + " leave game room " + to_string(roomID) + "\n";
			for(int i=0;i<userinroom[roomID].size();i++){
				int user_idx = userinroom[roomID][i];
				if(user_idx!=currentindex){
					char sendback[1024] = {};
					strcpy(sendback,ret2other.c_str());
					send(client_sds[user_idx],sendback,ret2other.size(),0);
				}
			}
			vector<int>::iterator itv = find(userinroom[roomID].begin(),userinroom[roomID].end(),currentindex);
			userinroom[roomID].erase(itv);
			inRoom[currentindex] = 0;
		}
	}
	return ret;
}
string _startgame(string rounds, string number){
	string ret = "";
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else if(inRoom[currentindex]==0){
		ret = "You did not join any game room";
	}
	else if(ismanager[currentindex]==0){
		ret = "You are not game room manager, you can't start game";
	}
	else if(roomStart[inRoom[currentindex]]){
		ret = "Game has started, you can't start again";
	}
	else{
		try{stoi(number);}
		catch(...){
			return "Please enter 4 digit number with leading zero";
		}
		if(number.size()!=4){
			ret = "Please enter 4 digit number with leading zero";
		}
		else{ //success
			int roomID = inRoom[currentindex];
			curplayer[roomID] = 0;
			roomStart[roomID] = 1;
			ret = "Game start! Current player is " + islogin[currentindex];
			string ret2other = "Game start! Current player is " + islogin[currentindex] + "\n";
			for(int i=0;i<userinroom[roomID].size();i++){
				int user_idx = userinroom[roomID][i];
				if(user_idx!=currentindex){
					char sendback[1024] = {};
					strcpy(sendback,ret2other.c_str());
					send(client_sds[user_idx],sendback,ret2other.size(),0);
				}
			}
			gameans[roomID] = number;
			gameround[roomID] = stoi(rounds);
		}
	}
	
	return ret;
}
string _guess(string number){
	string ret = "";
	unsigned int roomID = inRoom[currentindex];
	if(islogin[currentindex]==""){
		ret = "You are not logged in";
	}
	else if(inRoom[currentindex]==0){
		ret = "You did not join any game room";
	}
	else if(!roomStart[inRoom[currentindex]] && ismanager[currentindex]!=0){
		ret = "You are game room manager, please start game first";
	}
	else if(!roomStart[inRoom[currentindex]] && ismanager[currentindex]==0){
		ret = "Game has not started yet";
	}
	else if(currentindex!=userinroom[roomID][curplayer[roomID]]){
		ret = "Please wait..., current player is " + islogin[userinroom[roomID][curplayer[roomID]]];
	}
	else{
		try{stoi(number);}
		catch(...){
			return "Please enter 4 digit number with leading zero";
		}
		if(number.size()!=4){
			ret = "Please enter 4 digit number with leading zero";
		}
		else{ //success
			ret = game(number, roomID);
		}
	}
	return ret;
}
string game(string guess, unsigned int roomID){
	string ret = "";
	string result = "";
	int A=0,B=0;
	string org_guess = guess;
	string ans = gameans[roomID];
	if(ans==guess){
		ret = islogin[currentindex] + " guess '" + ans + "' and got Bingo!!! " + islogin[currentindex] + " wins the game, game ends";
		gameround[roomID] = 0;
		gameans[roomID] = "";
		roomStart[roomID] = 0;
		curplayer[roomID] = 0;
	}
	else{
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
		result = to_string(A)+"A"+to_string(B)+"B";
		ret = islogin[currentindex] + " guess '" + org_guess + "' and got '" + result + "'";
		if(currentindex==userinroom[roomID].back()){
			gameround[roomID]--;
			if(gameround[roomID]<=0){
				ret += "\nGame ends, no one wins";
				roomStart[roomID] = 0;
			}
			curplayer[roomID] = 0;
		}
		else{
			curplayer[roomID]++;
		}
	}
	for(int i=0;i<userinroom[roomID].size();i++){
		int user_idx = userinroom[roomID][i];
		if(user_idx!=currentindex){
			string ret2 = ret+"\n";
			char sendback[1024] = {};
			strcpy(sendback,ret2.c_str());
			send(client_sds[user_idx],sendback,ret2.size(),0);
		}
	}
	return ret;
}
string _exit(){
	unsigned int roomID = inRoom[currentindex];
	if(ismanager[currentindex]!=0){
		for(int i=0;i<userinroom[roomID].size();i++){
			int user_idx = userinroom[roomID][i];
			inRoom[user_idx] = 0;
		}
		if(!roomIsPub[roomID]){
			map<string, vector<int> >::iterator itm;
			for(itm=invitations.begin();itm!=invitations.end();itm++){
				vector<int>::iterator it = find(itm->second.begin(),itm->second.end(),currentindex);
				if(it!=itm->second.end()){
					itm->second.erase(it);
				}
			}
		}
		roomIsPub.erase(roomID);
		roomStart.erase(roomID);
		userinroom.erase(roomID);
		ismanager[currentindex] = 0;
	}
	islogin[currentindex] = "";	
	close(client_sds[currentindex]);
	client_sds[currentindex] = 0;
	inRoom[currentindex] = 0;
	return "";
}

int main(int argc, char *argv[]){
	int portnum = 8888;
	if(argc==2){
		portnum = atoi(argv[1]);
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
			char sendback[1024] = {};
			string ret = IOHandle(bufu);
			strcpy(sendback,ret.c_str());
			//cout << sendback << endl;
			int s = sendto(udpFd,sendback,ret.size(),MSG_CONFIRM,(const struct sockaddr *) &client_info,info_size);
			
			if(s<=0) cout << "sent back error!\n";
		}
		//old connection's operation
		for(int i=0;i<10;i++){
			if(FD_ISSET(client_sds[i],&readfds)){
				currentindex = i;
				char buffer[1024] = {};
				int r = recv(client_sds[i],buffer,1024,0);
				//cout << buffer << endl;
				if(r==0){ //EOF
					_exit();
				}
				else if(r==-1) continue;
				else{	
					char sendback[1024] = {};
					string ret = IOHandle(buffer);
					strcpy(sendback,ret.c_str());
					//cout << client_sds[i] << '\n';
					//cout << sendback;
					send(client_sds[i],sendback,ret.size(),0);
				}
				memset(buffer, '\0', 1024);
			}
		}
	}	
}
