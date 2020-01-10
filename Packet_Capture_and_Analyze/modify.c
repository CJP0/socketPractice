#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
struct in_addr getSocketIP(int sockfd, struct ifreq ifr){
	struct in_addr ip;
	struct sockaddr_in *sockaddrPtr;
    if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
        perror("ioctl SIOCGIFADDR error");
        ip.s_addr = 0;
    }
    else {
        sockaddrPtr = (struct sockaddr_in *)&ifr.ifr_addr;
        ip = sockaddrPtr->sin_addr;
    }
	return ip;
}
void printData(char *str){
	int n=0;
	while(str[n] != '\0')
		for(int i=0; i<10; ++i){
			printf("%c", str[n++]);
			if(str[n] == '\0')
				return;
		}
		puts("");
}
int main(int argc, char *argv[]){
	struct ethhdr *peth;
	struct iphdr *pip;
	struct tcphdr *tcpHdr;
	struct udphdr *udpHdr;
	char *ptemp, *data, buf[500], ipTemp[10];
	struct sockaddr_in sourceSin;
	struct in_addr myIP;
	int sockfd, sockfd3, counter = 0, one = 1, size;
	
	memset(buf, 0, sizeof(buf));
	memset(ipTemp, 0, sizeof(ipTemp));
	
	sourceSin.sin_family = AF_INET;
	sourceSin.sin_addr.s_addr = inet_addr(argv[1]);
	sourceSin.sin_port = htons(atoi(argv[2]));
	
	if((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){
		printf("Error creating layer2 socket!\n");
		exit(1);
	}
		
	if((sockfd3 = socket(PF_INET, SOCK_RAW, IPPROTO_TCP)) < 0){
		printf("Error creating layer3 socket!\n");
		exit(1);
	}
		
	struct ifreq ifr, origIfr;
	strncpy(ifr.ifr_name, "enp0s3", IFNAMSIZ);
	
	if(ioctl(sockfd,SIOCGIFFLAGS,&ifr) == -1){
		perror("ioctl");
		exit(1);
	}
	
	origIfr = ifr;
	ifr.ifr_flags |= IFF_PROMISC;
	if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1){
		perror("ioctl");
		exit(1);
	}
	
	myIP = getSocketIP(sockfd, ifr);
	strcpy(ipTemp, inet_ntoa(myIP));
	
	if(setsockopt(sockfd3, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0){
		perror("setsockopt() error");
		exit(-1);
	}
	else
		printf("setsockopt() OK\n");
	
	for(int i=0; i<5; ){
		recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
		ptemp = buf;
		peth = (struct ethhdr *)ptemp;
		if(ntohs(peth->h_proto) != 0x0800)
			continue;
		
		ptemp += sizeof(struct ethhdr);
		pip = (struct iphdr *)ptemp;
		if(strncmp(inet_ntoa(*(struct in_addr*)&pip->saddr), "127", 3) == 0)
			continue;
		if(strcmp(inet_ntoa(*(struct in_addr*)&pip->saddr), argv[1]) == 0)
			continue;
		
		if(strcmp(inet_ntoa(*(struct in_addr*)&pip->daddr), ipTemp) == 0)
			continue;
		
		++i;
		puts("\n-------------------------");
		printf("original source : %s", inet_ntoa(*(struct in_addr*)&pip->saddr));
		pip->saddr = inet_addr(argv[1]);
		if(pip->protocol == IPPROTO_TCP){
			tcpHdr = (struct tcphdr*)(buf + sizeof(struct ethhdr) + sizeof(struct iphdr));
			printf(":%d\n", ntohs(tcpHdr->source));
			printf("original destination : %s:%d\n", inet_ntoa(*(struct in_addr*)&pip->daddr), ntohs(tcpHdr->dest));
			tcpHdr->source = htons(atoi(argv[2]));
			data = buf + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr);
			size = sizeof(buf) - (sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr));
		}
		else if(pip->protocol == IPPROTO_UDP){
			udpHdr = (struct udphdr*)(buf + sizeof(struct ethhdr) + sizeof(struct iphdr));
			printf(":%d\n", ntohs(udpHdr->source));
			printf("original destination : %s:%d\n", inet_ntoa(*(struct in_addr*)&pip->daddr), ntohs(udpHdr->dest));
			udpHdr->source = htons(atoi(argv[2]));
			data = buf + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
			size = sizeof(buf) - (sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr));
		}
		puts("originalData:");
		printData(data);
		memset(data, 0, size);
		strcpy(data, "modifyTest");
		if(sendto(sockfd3, ptemp, sizeof(buf) - sizeof(struct ethhdr), 0, (struct sockaddr *)&sourceSin, sizeof(sourceSin)) < 0){
		   perror("sendto() error");
		   exit(-1);
		}
		else
			printf("\npacket %d sendto() OK, new source : %s:%u\n", i, argv[1], atoi(argv[2]));
		usleep(30000);
	}
		
	if(ioctl(sockfd, SIOCSIFFLAGS, &origIfr) == -1){
		perror("ioctl");
		exit(1);
	}
	
	close(sockfd);
	close(sockfd3);
}




