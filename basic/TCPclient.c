#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //for bzero()
#include <unistd.h>  //for close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/time.h>

#define Server_PortNumber 5000
#define Server_Address "127.0.0.1"

void printTimeval(struct timeval time){
	puts("-----printTimeval---------");
	printf("tv_sec : %ld\n", time.tv_sec);
	printf("tv_usec : %ld\n", time.tv_usec);
	puts("--------------------------");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr;
	struct timeval *sendT;
    int rec_len, sock, byte_sent, server_addr_length = sizeof(server_addr);
    char buffer[200], *str;
	
	memset(buffer, 0, sizeof(buffer));
	sendT = (struct timeval *)buffer;
	str = (char *)(buffer + sizeof(struct timeval));
	printf("Packet content : ");
	fgets(str, sizeof(buffer) - sizeof(struct timeval), stdin);
	
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)    printf("Error creating socket!\n");

    bzero(&server_addr, server_addr_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Server_PortNumber);
    server_addr.sin_addr.s_addr = inet_addr(Server_Address);

    if (connect(sock, (struct sockaddr *)&server_addr,server_addr_length)==-1) {
        printf("connect failed!\n");
        close(sock);
	}
	gettimeofday(sendT,NULL);
    byte_sent = send(sock, buffer, sizeof(buffer),0);
    if (byte_sent < 0)    
		printf("Error sending packet!\n");   
	memset(buffer, 0, sizeof(buffer));
	if((rec_len = recv(sock, buffer, sizeof(buffer),0)) == -1) {  
       printf("recv error\n");  
    }  
    buffer[rec_len]  = '\0';  
    printf("Server response : %s \n",buffer); 
    close(sock);
    return 0;
}