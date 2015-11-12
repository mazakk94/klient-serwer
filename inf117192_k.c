#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>

#define SIZE 30


struct msgbuf {
    long type;
    int cmd;
    char nick[10];
    char text[256];
    char date[30];
    int pid;
    int status;
};

struct singleMessage { // od najstarszej
    char nick[10];
    char date[30];
    char text[256];
};

void addToMessageBox(struct singleMessage *messages, struct msgbuf message){
    int i;
    int isFull = 1;
    for(i = 0; i < 10; i++){
	if(strlen(messages[i].nick) == 0){
	    isFull = 0;
	    strcpy(messages[i].text, message.text);
	    strcpy(messages[i].date, message.date);
	    strcpy(messages[i].nick, message.nick);
	    break;
	}
    }
    
    if(isFull){
	for(i = 0; i < 9; i++){
	    strcpy(messages[i].text, messages[i+1].text);
	    strcpy(messages[i].date, messages[i+1].date);
	    strcpy(messages[i].nick, messages[i+1].nick);
	}
	strcpy(messages[9].text, message.text);
	strcpy(messages[9].date, message.date);
	strcpy(messages[9].nick, message.nick);
    }
};

void clearMessageBox(struct singleMessage *messages){
    int i;
    for(i = 0; i < 10; i++){
	strcpy(messages[i].text, "");
	strcpy(messages[i].date, "");
	strcpy(messages[i].nick, "");
    }
}

void printMessageBox(struct singleMessage *messages){
    int i;
    for(i = 0; i < 10; i++){
	if(strlen(messages[i].text) == 0){
	    printf("break!\n");
	    break;
	}
	printf("%s\n", messages[i].date);
	printf("%s\n", messages[i].nick);
	printf("%s\n", messages[i].text);
    }
}

void SendMessage(struct msgbuf message, int id, int command){
    message.type = 1;
    message.cmd = command;
    message.pid = getpid();
    msgsnd(id, &message, sizeof(message), 0); 
}

void printMenu(){
    printf("1 - Zaloguj sie\n2 - Wyswietl liste uzytkownikow\n3 - Wyswietl liste grup\n4 - Zmien nazwe uzytkownika\n");
    printf("5 - Zapisz sie do grupy\n6 - Wypisz sie z grupy\n8 - Wyslij/odbierz wiadomosc\n");
    printf("9 - Wyslij wiadomosc do grupy uzytkownikow\n10 - Wyloguj sie\n");
}

char *printAnswer(int status){
    char *answer[256];
     switch(status){
	case 0:
	    printf("Wszystko OK\n");
	    break;
	case 1:
	    printf("Taki nick juz istnieje\n");
	    break;
	case 2:
	    printf("Jestes zalogowany\n");
	    break;
	case 3:
	    printf("Przepelnienie tablicy\n");
	    break;
	case 4:
	    printf("Jestes juz w grupie\n");
	    break;
	case 5:
	    printf("Grupa nie istnieje\n");
	    break;
	case 6:
	    printf("Nie jestes w grupie\n");
	    break;
	case 7:
	    printf("Nie istnieje taki nick\n");
	    break;
	case 8:
	    printf("Nie jestes zalogowany\n");
	    break;
    }
    return *answer;
}

int main (int args, char* argv[]) {
    int status;
    int id;
    struct msgbuf message;
    struct singleMessage messageBox[10];
    clearMessageBox(messageBox);
    
    char currentTime[SIZE];
    time_t curtime = time (NULL);
    struct tm *loctime = localtime (&curtime);
    strftime (currentTime, SIZE, "%I:%M, %B %d \n", loctime);

    id = msgget (15071410, 0644 | IPC_CREAT); // tworzy kolejke
    //printf("id = %d\n", id);

    int command;

    printMenu();


    while(1){
	curtime = time (NULL);
	loctime = localtime (&curtime);
	strftime (currentTime, SIZE, "%I:%M, %B %d \n", loctime);
	strcpy(message.date, currentTime);
	printf("TIME:\n%s\n", currentTime);
    
	printf("Czekam na komende od klienta...\n");
	scanf("%d", &command);
	printf("Twoj pid: %d\n", getpid());
	int msgstatus;
	switch(command){
	    case 0:{
		
		break;
	    }
	    case 1:
	    {
		printf("Podaj swój nick:\n");
		scanf("%s", message.nick);
		SendMessage(message, id, command);
		msgstatus = msgrcv(id, &message, sizeof(message), getpid(), MSG_NOERROR);
		break;
	    }
	    case 2:
	    {
		SendMessage(message, id, command);
		msgstatus = msgrcv(id, &message, sizeof(message), getpid(), MSG_NOERROR);
		break;
	    }
	    case 3:
	    {
		SendMessage(message, id, command);
		msgstatus = msgrcv(id, &message, sizeof(message), getpid(), MSG_NOERROR);	    
		break;
	    }
	    case 4:{		
		printf("Podaj nowa nazwe uzytkownika:\n");
		scanf("%s", message.nick);
		SendMessage(message, id, command);
		msgstatus = msgrcv(id, &message, sizeof(message), getpid(), MSG_NOERROR);
		break;
	    }
	    case 5:{
		printf("Podaj nazwe grupy, do ktorej chcesz sie zapisac:\n");
		scanf("%s", message.nick);
		SendMessage(message, id, command);
		msgstatus = msgrcv(id, &message, sizeof(message), getpid(), MSG_NOERROR);
		break;
	    }
	    case 6:{
		printf("Podaj nazwe grupy, z ktorej chcesz sie wypisac:\n");
		scanf("%s", message.nick);
		SendMessage(message, id, command);
		msgstatus = msgrcv(id, &message, sizeof(message), getpid(), MSG_NOERROR);
		break;
	    }
	    case 8:{
		//odbieranie
		while( (msgstatus = msgrcv(id, &message, sizeof(message), getpid(), MSG_NOERROR | IPC_NOWAIT) ) != -1){
		    printf("while case 8\n");
		    addToMessageBox(messageBox, message);
		    //system("CLEAR");
		    printMessageBox(messageBox);
		}
		int z;
		for(z = 0; z < 10; z++){
		    if(strlen(messageBox[z].text) == 0)
			break;
		    printf("%s\t %s\n%s\n", messageBox[z].date, messageBox[z].nick, messageBox[z].text);
		}

		
		//wysylanie
		printf("Podaj nick odbiorcy:\n");
		scanf("%s", message.nick);
		printf("Tresc:\n");
		scanf("%s", message.text);
		SendMessage(message, id, command);
		break;
	    }
	    case 11:{
		printf("Podaj nazwe grupy, ktora chcesz wyswietlic\n");
		scanf("%s", message.nick);
		break;
	    }
	    
	}
	//printf("jeszcze nie wyslalem\n");

	//while(1){ // odbiór wiadomosci z serwera
	//    printf("Sprawdzam czy odebrano wiadomosc...\n");
	    if(msgstatus == -1){
		printf("Nie odebrano (msgstatus = %d)\n", msgstatus);
		//break;
	    } else {

	    
		status = message.status;
		printAnswer(status);
		if(message.cmd == 2 || message.cmd == 3){
		    printf("Info od serwera:\n%s\n", message.text);
		} else if(message.cmd == 8){
		    printf("%s\nOtrzymales wiadomosc od %s:\n%s\n", message.date, message.nick, message.text);
		}
		
	    } 

    }


    printf("koniec!\n");
    return 0;
}