#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //for bzero()
#include <unistd.h>  //for close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<sys/time.h>
#define PortNumber 5555

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
    struct sockaddr_in address, client_address;
	struct timeval endT, *startT;
	double latency;
    int sock, byte_recv, client_address_length, byte_sent;
    char buffer[500], *str;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)    printf("Error creating socket\n");
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PortNumber);
    address.sin_addr.s_addr = INADDR_ANY;  
    
    if (bind(sock,(struct sockaddr *)&address, sizeof(address)) == -1) {
        printf("error binding\n");
        close(sock);
	} 
    bzero(&client_address, sizeof(client_address));
	client_address_length = sizeof(client_address_length);
    while(1){
        byte_recv = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &client_address_length);
        if (byte_recv < 0)
			printf("Error recving packet\n");
		gettimeofday(&endT,NULL);
		puts("----------------------------------");
		/*
		puts("--------startT---");
		printTimeval(startT);
		puts("--------endT---");
		printTimeval(endT);*/
		startT = (struct timeval *)buffer;
		str = (char *)(buffer + sizeof(struct timeval));
		latency = (double)(1000000 * (endT.tv_sec-startT->tv_sec)+ endT.tv_usec-startT->tv_usec)/1000000;
		printf("Latency(client to server)=%lfs\n", latency);
		printf("Throughput=%lfbyte/s\n", computeThroughput(counterArrChar(str), latency));
		//puts(buffer);
        printf("Received packet : %s\n",str);
		byte_sent = sendto(sock, "Welcome!\n", 10, 0, (struct sockaddr *)&client_address, client_address_length);
		if (byte_sent < 0)    printf("Error sending packet\n");
		
	}
}
