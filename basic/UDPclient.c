#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //for bzero()
#include <unistd.h>  //for close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/time.h>

#define Server_PortNumber 5555
#define Server_Address "127.0.0.1"

void printTimeval(struct timeval time){
	puts("-----printTimeval---------");
	printf("tv_sec : %ld\n", time.tv_sec);
	printf("tv_usec : %ld\n", time.tv_usec);
	puts("--------------------------");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in address;
	struct timeval *tv;
    int sock, byte_sent;
    char buffer[200], *str;
	memset(buffer, 0, sizeof(buffer));
	
	printf("Packet content : ");
	str = (char *)(buffer + sizeof(struct timeval));
	tv = (struct timeval *)buffer;
    fgets(str, sizeof(buffer) - sizeof(struct timeval), stdin);
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)    printf("Error creating socket\n");

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(Server_PortNumber);
    address.sin_addr.s_addr = inet_addr(Server_Address);
    int address_length = sizeof(address);
	gettimeofday(tv,NULL);
	//printTimeval(tv);
    byte_sent = sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&address, address_length);
    if (byte_sent < 0)    printf("Error sending packet\n");
	//puts(packet);
	if(0 > recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&address, &address_length))
		printf("Error sending packet\n");
	else
		printf("Server response : %s\n", buffer);
    close(sock);
    return 0;
}
