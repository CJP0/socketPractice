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
#define MAXMSGSIZE 1000
#define serverPort 5000
#define serverAddress "127.0.0.1"
typedef struct msgHeader *msgHeaderPointer;
struct msgHeader{
	int fd;
	int clientID;
	int serverMsg;
	int exit;
};
char sendbuf[MAXMSGSIZE];
char recvbuf[MAXMSGSIZE];
int sockfd, myClientID;
msgHeaderPointer sendMsgHeader, recvMsgHeader;

void *sendMsg(void *arg){
	sendMsgHeader = (struct msgHeader *)sendbuf;
	sendMsgHeader->fd = sockfd;
	sendMsgHeader->clientID = myClientID;
	sendMsgHeader->exit = 0;
	sendMsgHeader->serverMsg = 0;
	while(1){
		fgets(sendbuf + sizeof(struct msgHeader), MAXMSGSIZE - sizeof(struct msgHeader), stdin);
		
		if(strncmp(sendbuf + sizeof(struct msgHeader), "exit", 4) == 0){
			sendMsgHeader->exit = 1;
            if(send(sockfd, sendbuf, MAXMSGSIZE, 0) == -1){
                printf("Error sending exit message!\n");
            }
            close(sockfd);
            exit(0);
        }
        if(send(sockfd, sendbuf, MAXMSGSIZE, 0) == -1){
            printf("Error sending message!\n");
        }
        bzero(sendbuf + sizeof(struct msgHeader), MAXMSGSIZE - sizeof(struct msgHeader));
	}
}

int main(int argc, char *argv[]){
	struct sockaddr_in seraddr, recvaddr;
	pthread_t sendThread;
	msgHeaderPointer tempMsgHeader;
    bzero(sendbuf, MAXMSGSIZE);
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Error creating socket!\n");
        return 1;
    }
	bzero(&seraddr, sizeof(struct sockaddr_in));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = inet_addr(serverAddress);
    seraddr.sin_port = htons(serverPort);
	
    if(connect(sockfd, (struct sockaddr *)&seraddr, sizeof(struct sockaddr)) == -1){
        printf("connect failed!\n");
        close(sockfd);
		return 1;
    }
	
	while(1){
		bzero(recvbuf, MAXMSGSIZE);
		if(recv(sockfd, recvbuf, MAXMSGSIZE, 0) == -1){
			printf("recv myClientID error\n");
		}
		tempMsgHeader = (struct msgHeader *)recvbuf;
		if(tempMsgHeader->serverMsg){
			myClientID = tempMsgHeader->clientID;
			break;
		}
	}
	printf("--------client%d successful connect to %s:%d--------\n", myClientID, inet_ntoa(seraddr.sin_addr), ntohs(seraddr.sin_port));
	
    pthread_create(&sendThread, NULL, sendMsg, NULL);
	
	while(1){
		bzero(recvbuf, MAXMSGSIZE);
        if(recv(sockfd, recvbuf, MAXMSGSIZE, 0) == -1){
            printf("recv error\n");
        }
        
        recvMsgHeader = (struct msgHeader *)recvbuf;
		printf("client%d : %s", recvMsgHeader->clientID, recvbuf+sizeof(struct msgHeader));
	}
	return 0;
}