#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //for bzero()
#include <unistd.h>  //for close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<sys/time.h>
#define PortNumber 5000

void printTimeval(struct timeval time){
	puts("-----printTimeval---------");
	printf("tv_sec : %ld\n", time.tv_sec);
	printf("tv_usec : %ld\n", time.tv_usec);
	puts("--------------------------");
}

double computeThroughput(int n, double time){
	//printf("sizeof(buffer)=%d\n", n);
	return (double)n*(1/time);
}

int counterArrChar(char buffer[]){
	int n=0;
	while(buffer[n])
		n++;
	return n;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr,client_addr;
	struct timeval receiveT, *sendT;
    int sock, byte_recv, server_addr_length = sizeof(server_addr), client_addr_length=sizeof(client_addr), recfd, rec_len; 
    char buffer[200], *str;
	double latency;
	memset(buffer, 0, sizeof(buffer));
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)    printf("Error creating socket\n");
    bzero(&server_addr, server_addr_length);
    bzero(&client_addr, client_addr_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PortNumber);
    server_addr.sin_addr.s_addr = INADDR_ANY;  
	
    if (bind(sock,(struct sockaddr *)&server_addr, server_addr_length) == -1) {
        printf("error binding!\n");
        close(sock);
		return 0;
	}
    if (listen(sock, 20) == -1) {
        printf("listen failed!\n");
        close(sock);
		return 0;
	}
	while(1){
		if((recfd = accept(sock,(struct sockaddr *)&client_addr,&client_addr_length))==-1) {
			printf("accept failed!\n");
			continue;
		}
		byte_recv = recv(recfd, buffer, sizeof(buffer),0);
		if (byte_recv < 0)    printf("Error recving packet\n");
		gettimeofday(&receiveT,NULL);
		puts("----------------------------------");
		sendT = (struct timeval *)buffer;
		str = (char *)(buffer + sizeof(struct timeval));
		latency = (double)(1000000 * (receiveT.tv_sec-sendT->tv_sec)+ receiveT.tv_usec-sendT->tv_usec)/1000000;
		printf("Latency(client to server)=%lfs\n", latency);
		printf("Throughput=%lfbyte/s\n", computeThroughput(counterArrChar(str), latency));
		
		printf("Received packet : %s\n",str);
		if(send(recfd, "Welcome!\n", 10,0) == -1)  
            printf("send error"); 
		close(recfd);
	}
	close(sock);
	return 0;
}