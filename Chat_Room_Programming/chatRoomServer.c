#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/time.h>
#include <pthread.h>
#define MAXCONNECT 100
#define MAXMSGSIZE 1000
#define serverPort 5000
typedef struct msgHeader *msgHeaderPointer;
struct msgHeader{
	int fd;
	int clientID;
	int serverMsg;
	int exit;
};
int onlineClientNum = 0, clientCounter = 0;
char sendbuf[MAXMSGSIZE];
msgHeaderPointer clientInfoArr[MAXCONNECT];
pthread_t recvThread;

void broadcast(char *buf, int id){
	for(int i = 0; i < onlineClientNum; i++){
        if(clientInfoArr[i]->clientID == id){
            continue;
        }
        if(send(clientInfoArr[i]->fd, buf, MAXMSGSIZE, 0) == -1){
            printf("broadcast client%d message to client%d error\n", id, clientInfoArr[i]->clientID);
        }
    }
    bzero(buf, MAXMSGSIZE);
}

void rmFd(int fd){
    for(int i = 0; i < onlineClientNum; i++){
        if(clientInfoArr[i]->fd == fd){
			free(clientInfoArr[i]);
            for(; i < onlineClientNum; i++){
                clientInfoArr[i] = clientInfoArr[i+1];
            }
        }
    }
	--onlineClientNum;
}

void *recvMsg(void *buf){
	msgHeaderPointer recvMsgHeader = (msgHeaderPointer)buf;
    int recvClientID = recvMsgHeader->clientID;
	int recvClientfd = recvMsgHeader->fd;
	char *recvbuf = (char *)buf;
    while(1){
        if(recv(recvClientfd, recvbuf, MAXMSGSIZE, 0) == -1){
            printf("Receive msg err\n");
			exit(1);
			continue;
        }
        if(recvMsgHeader->exit == 1){
            rmFd(recvClientfd);
            close(recvClientfd);
			free(recvbuf);
            return NULL;
        }
		printf("client%d : %s", recvClientID, recvbuf+sizeof(struct msgHeader));
        broadcast(recvbuf, recvClientID);
    }
    close(recvClientfd);
    return NULL;
}

void recordClient(int fd, int n){
	msgHeaderPointer tempMsgHeader = (struct msgHeader *)calloc(1, sizeof(struct msgHeader));
	tempMsgHeader->fd = fd;
	tempMsgHeader->clientID = n;
	clientInfoArr[onlineClientNum++] = tempMsgHeader;
}

int main(int argc, char *argv[]){
	int serverfd, recvfd;
    socklen_t sockleng = sizeof(struct sockaddr_in);
    struct sockaddr_in serveraddr, clientaddr;
    pid_t childid;
	msgHeaderPointer tempMsgHeader;
	char *tempArr;
	
	if( (serverfd = socket(AF_INET, SOCK_STREAM, 0) ) == -1){
        printf("Error creating socket!\n");
        return 1;
    }
	
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(serverPort);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
	
	if(bind(serverfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr)) == -1){
        printf("error binding!\n");
        close(serverfd);
		return 1;
    }
	
    if(listen(serverfd, 100) == -1){
        printf("listen failed!\n");
        close(serverfd);
		return 1;
    }
	
    printf("\nListening at %d port, wating connection.....\n", serverPort);
	
	while(1){
        if((recvfd = accept(serverfd, (struct sockaddr *)&clientaddr, &sockleng)) == 0){
            printf("accept failed!\n");
            continue;
        }
		
		recordClient(recvfd, ++clientCounter);
        printf("client%d connect\n", clientCounter);

        tempArr = (char *)calloc(MAXMSGSIZE, sizeof(char));
		tempMsgHeader = (msgHeaderPointer)tempArr;
		tempMsgHeader->fd = recvfd;
		tempMsgHeader->clientID = clientCounter;
		tempMsgHeader->serverMsg = 1;
		if(send(recvfd, tempArr, MAXMSGSIZE, 0) == -1){
            puts("send clientID serverMsg error");
			continue;
        }
		tempMsgHeader->serverMsg = 0;
        if((pthread_create(&recvThread, NULL, recvMsg, tempArr)) != 0){
            printf("error creat recvThread\n");
			continue;
        }
		
    }
    close(serverfd);
    return 0;
}