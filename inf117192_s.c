#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define GROUP_SIZE 3
#define USERS_SIZE 18
#define KEY 15071410


struct msgbuf {
    long type;
    int cmd;
    char nick[10];
    char text[256];
    char date[30];
    int pid;
    int status;
};

struct user {
    char nick[10];
    int pid;
};

struct group {
    char name[10]; //nazwa grupy
    struct user users[10]; //pidy uzytkownikow
};


void ClearUsers(struct user *users, int size){
    int i;
    for(i = 0; i < size; i++){
	strcpy(users[i].nick, "");
	users[i].pid = 0;
    }
}

void ClearGroups(struct group *groups){
    int i;
    for(i = 0; i < GROUP_SIZE; i++){
	strcpy(groups[i].name, "");
	ClearUsers(groups[i].users, 10);
    }
}

void InitGroup(struct group *groups, char* name){
    int i;
    int flag = 0; //nie dodano
    for(i = 0; i < GROUP_SIZE; i++){
	if(strlen(groups[i].name)==0){
	    strcpy(groups[i].name, name);
	    flag = 1;
	    break;
	}
    }
    
    if(flag == 0)
	printf("Przepelnienie tablicy grup");
}

void LoadGroupsFromFile(struct group *groups){
    FILE *file = fopen("config.txt", "r");
    int i = 0;
    char groupName[10];
    ClearGroups(groups);
    strcpy(groupName, "");
    for(i = 0; i < 3; i++){
	fscanf(file, "%s", groupName);
	InitGroup(groups, groupName);
    }
    fclose(file);
}

/****** 3 ******/
void SendGroups(struct msgbuf message, struct group *groups, int id){
    message.type = message.pid;
    //strcat(
    char msg[256];
    strcpy(msg, "");
    int i;
    char dig[2];
    for(i=0;i<GROUP_SIZE;i++){
	if(strlen(groups[0].name)==0){
	    strcat(msg, "Lista grup pusta!\n");
	    break;
	}
	if(strlen(groups[i].name)==0)
	    break;

	strcat(msg, "GROUP ");
	dig[0] = (char)(((int)'0')+i);
	dig[1]= '\0';
	
	strcat(msg, dig);
	strcat(msg, "\nNAME: ");
	strcat(msg, groups[i].name);
	strcat(msg, "\n");
    }
    strcpy(message.text, msg);
    msgsnd(id, &message, sizeof(message), 0);
}

/****** 1 ******/
void LoginUser(int i, struct user *users, struct msgbuf message){
    strcpy(users[i].nick, message.nick);
    users[i].pid = message.pid;
}

void SendMessage(int id, int pid, int status, char* text, struct msgbuf message){
    message.type = pid;
    message.status = status;
    strcpy(message.text, text);
    msgsnd(id, &message, sizeof(message), 0);
}

/****** 1 ******/
int Login(struct msgbuf message, struct user *users, int id){
    int i;
    int flag = 1; //przepelnienie tablicy
    for(i = 0; i < USERS_SIZE; i++){
	if(message.pid == users[i].pid){			// jak sa takie same pidy
	    SendMessage(id, message.pid, 2, "", message); 	//Juz jestes zalogowany!	    //printf("Istnieje uzytkownik o tym samym pid !\n");
	    return 2;
	}

	if((int)strlen(users[i].nick)==0){					 //doszedl do miejsca w ktorym jest wolny slot i nie ma takiego samego usera
	    flag = 0; 							//tablica nie jest przepelniona
	    SendMessage(id, message.pid, 0, "", message);//Zalogowales sie			 
	    LoginUser(i, users, message);
	    return 0;
	}
	if(strcmp(message.nick, users[i].nick) == 0){			 //jak sa takie same nicki
	    SendMessage(id, message.pid, 1, "", message);	//Podany uzytkownik juz istnieje!\n	 //printf("Podany uzytkownik juz istnieje!\n");
	    return 1;
	}
    }
    
    if(flag == 1){
	SendMessage(id, message.pid, 3, "", message);	//Przepelnienie tablicy uzytkowników!
	return 3;
    }

    return 0;
}

void PrintUsers(struct user *users){
    int i;
    for(i=0;i<USERS_SIZE;i++){
	printf("\nUSER %d:\n", i);
	printf("pid = %d\n", users[i].pid);
	printf("nick = %s\n", users[i].nick);
    }
}
/****** 11 ******/
void PrintGroupUsers(struct msgbuf message, struct group *groups, int id){
    int i,j,flag = 0; // flag = 0 oznacza ze nie ma takiej grupy
    for(i = 0; i < GROUP_SIZE; i++){
	if(strcmp(groups[i].name, message.nick) == 0){
	    flag = 1; // jest grupa o podanej nazwie
	    break;
	}
    }
    
    if(flag == 0){
	 SendMessage(id, message.pid, 1, "", message); //Nie ma takiej grupy\n
    } else {
    
	for(j = 0; j < 10; j++){
	    
	    if(groups[i].users[0].pid == 0){
		printf("Grupa jest pusta!\n");
		break;
	    }
	    if(groups[i].users[j].pid == 0)
		break;
	
	    printf("\nUSER %d:\n", j);
	    printf("pid = %d\n", groups[i].users[j].pid);
	    printf("nick = %s\n", groups[i].users[j].nick);
	}
	 SendMessage(id, message.pid, 1, "", message); //Wyswietlilem liste po stronie klienta\n
    }
}

/****** 2 ******/
void SendUsers(struct msgbuf message, struct user *users, int id){
    message.type = message.pid;
    //strcat(
    char msg[256];
    strcpy(msg, "");
    int i;
    char dig[3];
    for(i=0;i<USERS_SIZE;i++){
	if(strlen(users[0].nick)==0){
	    strcat(msg, "Lista uzytkownikow pusta!\n");
	    break;
	}
	if(strlen(users[i].nick)==0)
	    break;

	strcat(msg, "USER ");
	if (i>9) {
	    dig[0] = (char)(((int)'0')+i/10);
	    dig[1] = (char)(((int)'0')+i%10);
	    dig[2]= '\0';
	} else {
	    dig[0] = (char)(((int)'0')+i);
	    dig[1]= '\0';
	    dig[2]= '\0';
	}
	strcat(msg, dig);
	strcat(msg, "\nNick: ");
	strcat(msg, users[i].nick);
	strcat(msg, "\n");
    }
    strcpy(message.text, msg);
    msgsnd(id, &message, sizeof(message), 0);

}

void FixNick(struct group *groups, struct msgbuf message){
    int i,j;
    for(i = 0; i < GROUP_SIZE; i++)
	for(j = 0; j < 10; j++)
	    if(message.pid == groups[i].users[j].pid)
		strcpy(groups[i].users[j].nick, message.nick);    
}

/****** 4 ******/
int ChangeNick(struct msgbuf message, struct group *groups, struct user *users, int id){
			//wszystko ok - 0  /taki nick juz istnieje - 1   nie jestes zalogowany - 8
    int i;
    int userID;
    int flag = 0;
    for(i = 0; i < USERS_SIZE; i++){
	if(message.pid == users[i].pid) {
	    userID = i; //zapisujemy sobie w ktorym miejscu siedzi user
	    flag = 1;	//jestes zalogowany
	}
    }
	
    if(flag == 0){
	SendMessage(id, message.pid, 8, "", message);	// nie jestes zalogowany
	return 8;
    }    
    
    for(i = 0; i < USERS_SIZE; i++){
	if(strcmp(message.nick, users[i].nick) == 0){
	    SendMessage(id, message.pid, 1, "", message);	//taki nick istnieje
	    return 1;		
	}    
    }
    
    
    strcpy(users[userID].nick, message.nick);
    FixNick(groups, message);
    SendMessage(id, message.pid, 0, "", message); //Zmiana nicku wykonana pomyslnie\n
    return 0;

}

/****** 5 ******/
int AddToGroup(struct msgbuf message, struct group *groups, struct user *users, int id){
 /* 	0 - dodany do grupy
	3 - przepelnienie tablicy
	4 - juz jestes w grupie
	5 - grupa nie istnieje
	8 - nie jestes zalogowany */
    int i,j;
    int flag = 0; //grupa nie istnieje / uzytkownik niezalogowany
    char tempUserNick[10];
    printf("Pid w funkcji = %d\n", message.pid);
    printf("Temp User Nick = %s\n", tempUserNick);
    for(i = 0; i < USERS_SIZE; i++){
	printf("users[%d].pid = %d\n", i, users[i].pid);
    	if(message.pid == users[i].pid){
	    printf("wszedlem w i = %d po pidzie = %d\n", i, users[i].pid);
	    strcpy(tempUserNick, users[i].nick);			//	pobieramy nick osoby ktora wyslala 
	    printf("W IFIE: Temp User Nick = %s\n", tempUserNick);	//	komunikat, bo w nicku jest nazwa grupy
	    flag = 1;
	    break;
	}
    }
    
    if(flag == 0){
	SendMessage(id, message.pid, 8, "", message); //Najpierw sie zaloguj
	return 8;
    }
    
    flag = 0; //uzywamy flage poraz drugi
    for(i = 0; i < GROUP_SIZE; i++){
	if(strcmp(groups[i].name, message.nick) == 0){ //wchodzimy do grupy o podanej nazwie
	    flag = 1;
	    for(j = 0; j < 10; j++){
		if(strcmp(groups[i].users[j].nick, tempUserNick) == 0){
		    SendMessage(id, message.pid, 4, "", message); //Juz jestes w tej grupie\n
		    return 4;
		}
		printf("strlen(groups[%d].users[%d].nick = %d\n", i, j, (int)strlen(groups[i].users[j].nick));
		if(strlen(groups[i].users[j].nick) == 0){
		    strcpy(groups[i].users[j].nick, tempUserNick);
		    groups[i].users[j].pid = message.pid;
		    SendMessage(id, message.pid, 0, "", message); //Zostales dodany do grupy\n
		    return 0;
		}		
	    }		
	    //przepelnienie
	    SendMessage(id, message.pid, 3, "", message); //Nie ma dla ciebie miejsca w tej grupie\n
	    return 3;
	}
    }
    
    if(flag == 0){
	SendMessage(id, message.pid, 5, "", message); //Grupa o podanej nazwie nie istnieje\n
	return 5;
    }     
    printf("returnuje niemozliwy przypadek");
    return 100;
}

void FixGroup(int i, struct group *groups, int groupID){
    int k;
    for(k = i; k < 9; k++){
	strcpy(groups[groupID].users[k].nick, groups[groupID].users[k+1].nick);
	groups[groupID].users[k].pid =groups[groupID].users[k+1].pid;
    }
   
}

void FixUsers(int j, struct user *users){
    int i;
    for(i = j; i < USERS_SIZE - 1; i++){
	 strcpy(users[i].nick, users[i+1].nick);		// usuwamy uzytkownika z listy userów
	 users[i].pid = users[i+1].pid;
    }
}

/****** 6 ******/
int LeaveFromGroup(struct msgbuf message, struct group *groups, struct user *users, int id){
  /*	0 - udalo sie wyjsc z grupy
	5 - nie ma takiej grupy
	6 - nie ma cie w grupie
	8 - najpierw sie zaloguj */
    int i,j,k;				
    for(i = 0; i < USERS_SIZE; i++){
	if(message.pid == users[i].pid) { //znalezlismy usera (jest zalogowamy), tutaj trzeba usunac z grupy i zreturnowac				
	    
	    for(j = 0; j < GROUP_SIZE; j++){		//usuwamy z grupy
		if(strcmp(message.nick, groups[j].name)==0){//jestesmy w grupie ktora user podal
		    for(k = 0; k < 10; k++){
			if(groups[j].users[0].pid == 0){
			    SendMessage(id, message.pid, 6, "", message); //Nie ma cie w grupie (grupa jest pusta)\n
			    return 6;
			}
		    
			if(groups[j].users[k].pid == message.pid){
			    groups[j].users[k].pid = 0;
			    strcpy(groups[j].users[k].nick, "");
			    FixGroup(k, groups, j);
			    SendMessage(id, message.pid, 0, "", message); //Opusciles wybrana grupe\n
			    return 0;
			}
		    }// nie znalezlismy tego samego pidu w grupie
		    SendMessage(id, message.pid, 6, "", message); //Nie ma cie w grupie\n
		    return 6;
		    
		}
	    } //jak wyjdziemy z petli to znaczy ze nie ma takiej grupy
	    
	    SendMessage(id, message.pid, 5, "", message); //Nie ma takiej grupy\n
	    return 5;
	}
    } //nie znalezlismy takiego samego pidu
    SendMessage(id, message.pid, 8, "", message); //Najpierw sie zaloguj\n
    return 8;    
}

/****** 8 ******/
char* PidToNick(struct user *users, int pid){
    int i; 
    printf("PIDTONICK\n");
    //char temp[1];
    for(i = 0; i < USERS_SIZE; i++)
	if(users[i].pid == pid){
	    printf("nick: %s\n pid = %d\n", users[i].nick, users[i].pid);
	    return users[i].nick;
	}
    return "";   
}

int NickToPid(struct user *users, char *nick){
    int i;
    printf("NICKTOPID\n");
    for(i = 0; i < USERS_SIZE; i++)
	if(strcmp(users[i].nick, nick) == 0){
	    printf("pid = %d\nnick = %s\n", users[i].pid, users[i].nick);
	    return users[i].pid;
	}
    return 0;
}

int SenderInfo(struct msgbuf message, struct user *users, int id){
  /*0 - wszystko ok
    7 - nie istnieje taki nick
    8 - nie jestes zalogowany*/
    int i;
    int loggedIn = 0;
    for(i = 0; i < USERS_SIZE; i++)
	if(users[i].pid == message.pid){
	    loggedIn = 1;
	    break;
	}
	
    if(loggedIn == 1){
	for(i = 0; i < USERS_SIZE; i++){
	    if(strcmp(message.nick, users[i].nick) == 0 && message.pid != users[i].pid){
		SendMessage(id, message.pid, 0, "", message);
		return 0;
	    }	
	} // nie istnieje nick
	SendMessage(id, message.pid, 7, "", message);
	return 7;
    } else {
	SendMessage(id, message.pid, 8, "", message);
	return 8;
    }
	
}

void MessageToReceiver(struct msgbuf message, struct user *users, int id){
    printf("wykonuje MessageToReceiver\nmessage.nick: %s\n", message.nick);
    int tempPid = NickToPid(users, message.nick);
    char tempNick[10];
    strcpy(tempNick, PidToNick(users, message.pid));
    printf("temp pid = %d\ntemp nick: %s\n", tempPid, tempNick);
    
    strcpy(message.nick, tempNick);
   
    SendMessage(id, tempPid, 0, message.text, message);
}

/****** 10 ******/ 
int Logout(struct msgbuf message, struct group *groups, struct user *users, int id){
    int i,j,k;				//     0 - udalo sie wylogowac    8 - nie byles zalogowany
    for(i = 0; i < USERS_SIZE; i++){
	if(message.pid == users[i].pid) { 				
	    
	    for(j = 0; j < GROUP_SIZE; j++){		//usuwamy z grupy
		for(k = 0; k < 10; k++){
		    if(groups[j].users[0].pid == 0)
			break;
		    if(groups[j].users[k].pid == message.pid){
			groups[j].users[k].pid = 0;
			strcpy(groups[j].users[k].nick, "");
			FixGroup(k, groups, j);
			break;
		    }
		}
	    }
	    
	    strcpy(users[i].nick, "");		// usuwamy uzytkownika z listy userów
	    users[i].pid = 0;
	    FixUsers(i, users);
	    SendMessage(id, message.pid, 0, "", message); //Wlasnie sie wylogowales\n
	    return 0;
	}
    } //nie znalezlismy takiego samego pidu
    SendMessage(id, message.pid, 8, "", message); //Nie byles zalogowany\n
    return 8;       
}



// ******************************************  MAIN  ***************************************************

int main (int args, char* argv[]) {
    int id = msgget (KEY, 0644 | IPC_CREAT);
    int status;
    struct msgbuf message;
    struct user users[USERS_SIZE];
    struct group groups[GROUP_SIZE];
    ClearUsers(users, USERS_SIZE); 		//czyscimy liste userów
   

    LoadGroupsFromFile(groups);
    //printf("%d\n", id);

    while(1) {
	//printf("Czekam na komende od klienta...\n");
	int msgstatus = msgrcv(id, &message, sizeof(message), 1, MSG_NOERROR);
	if(msgstatus != -1){
	   printf("Otrzymalem wiadomosc od: %d\n status: %d\n", message.pid, msgstatus);

	    switch(message.cmd){
		    case 1:{
		    printf("Pid klienta: %d\n", message.pid);
		    status = Login(message, users, id);
		    printf("Status logowania: %d\n", status); 
		    break;			// PrintUsers(users);
		}
		    case 2:{
		    SendUsers(message, users, id);
		    break;
		}
		    case 3:{
		    SendGroups(message, groups, id);
		    break; 
		}
		    case 4:{
		    status = ChangeNick(message, groups, users, id);
		    printf("Status zmiany uzytkownika: %d\n", status);
		    break;
		}
		    case 5:{   
		    status = AddToGroup(message, groups, users, id);
		    printf("Status dodawania do grupy: %d\n", status);
		    break;
		}
		    case 6:{
		    status = LeaveFromGroup(message, groups, users, id);
		    printf("Status opuszczania grupy: %d\n", status);	
		    break;
		}
		    case 8:{
		    status = SenderInfo(message, users, id);
		    printf("Status wysylania do nadawcy %d\n", status);
		    if(status == 0)
			MessageToReceiver(message, users, id);
		    
		    //printf("Status info do nadawcy: %d\n", status);
		    //printf("message.pid = %d\n", message.pid);
		    //char nick[10];
		    //strcpy(nick, PidToNick(users, message.pid));
		    //printf("nick from pid = %s\n", nick);
		    
		    //int newPid = NickToPid(users, message.nick);
		    //printf("pid from nick = %d\n", newPid);
		    //status = MessageToReceiver();
		    
		    
		    break;
		}
		    case 10:{
		    status = Logout(message, groups, users, id);
		    printf("Status wylogowania: %d\n", status);
		    break;
		}
		    case 11: {
		    PrintGroupUsers(message, groups, id);
		    break;
		    
		}
	    }
	} else {// koniec ifa
	  //  printf("msgstatus : %d\n", msgstatus);
	}
	
    }     
     
    return 0;
    
}


					    // typ wiadomości, musi być ten sam, jak 0 to wszystko ma zbierać
						      // id, &s, wielkość s, 0, msn_noerror 6- długość, 5 - msgtype, 0- nie wiem
							// rcv zwraca rozmiar albo -1 jak jest error

/* BRUDNOPIS
 * 
 * 
 *     message.type = message.pid;
       message.status = 0;
       strcpy(message.text, "wszystko OK\n");
       msgsnd(id, &message, sizeof(message), 0);						
 */

 