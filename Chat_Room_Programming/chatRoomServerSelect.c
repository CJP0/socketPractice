#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/time.h>
#include <sys/select.h>
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

void sendClientID(int fd, int id){
	struct msgHeader tempMsgHeader;
	tempMsgHeader.fd=fd;
	tempMsgHeader.clientID=id;
	tempMsgHeader.serverMsg=1;
	if(send(fd, (char *)&tempMsgHeader, sizeof(struct msgHeader), 0) == -1){
        printf("send clientID serverMsg to client%d error\n", id);
    }
}

int main(int argc, char *argv[]){
	int serverfd, recvfd, recvtemp;
    socklen_t sockleng = sizeof(struct sockaddr_in);
    struct sockaddr_in serveraddr, clientaddr;
    pid_t childid;
	msgHeaderPointer tempMsgHeader;
	fd_set masterfdset, readfdset;
	int maxfd;
	char *tempArr, recvbuf[MAXMSGSIZE];
	FD_ZERO(&masterfdset);
	FD_ZERO(&readfdset);
	
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
	FD_SET(serverfd, &masterfdset);
	maxfd = serverfd;
	
    printf("\nListening at %d port, wating connection.....\n", serverPort);
	
	while(1){
		readfdset = masterfdset;
		if (select(maxfd+1, &readfdset, NULL, NULL, NULL) == -1) {
			printf("select error\n");
			exit(1);
		}
		for(int i=0;i<=maxfd;++i){
			if(FD_ISSET(i, &readfdset)){
				if(i == serverfd){
					if((recvfd = accept(serverfd, (struct sockaddr *)&clientaddr, &sockleng)) == 0){
						printf("accept failed!\n");
						continue;
					}
					FD_SET(recvfd, &masterfdset);
					printf("client%d connect\n", ++clientCounter);
					if(recvfd>maxfd)
						maxfd = recvfd;
					sendClientID(recvfd, clientCounter);
				}
				else{
					recvtemp=recv(i, recvbuf, MAXMSGSIZE, 0);
					if(recvtemp<0){
						printf("Receive msg err\n");
						close(i);
						FD_CLR(i, &masterfdset);
					}
					else if(recvtemp == 0){
						printf("socket %d hung up\n", i);
						close(i);
						FD_CLR(i, &masterfdset);
					}
					else{
						tempMsgHeader = (msgHeaderPointer)recvbuf;
						printf("client%d : %s", tempMsgHeader->clientID, recvbuf+sizeof(struct msgHeader));
						for(int j=0;j<=maxfd;++j){
							if(FD_ISSET(j, &masterfdset) && j != serverfd && j != i){
								if(send(j, recvbuf, MAXMSGSIZE, 0)<0){
									printf("broadcast socket%d message to socket%d error\n", i, j);
								}
							}
						}
					}
				}
			}
		}
    }
    close(serverfd);
    return 0;
}