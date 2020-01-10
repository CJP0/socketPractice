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
#include <arpa/inet.h>
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

void printProtocol(struct iphdr *pip){
	printf("IP->protocol = ");
	switch(pip->protocol)
		{
		case IPPROTO_TCP:
			printf("TCP\n");
			break;
		case IPPROTO_UDP:
			printf("UDP\n");
			break;
		case IPPROTO_ICMP:
			printf("ICMP\n");
			break;
		case IPPROTO_IGMP:
			printf("IGMP\n");
			break;
		}
}

void printPacketInfo(struct ethhdr *ethHdr, struct iphdr *ipHdr, struct tcphdr *tcpHdr, int counter){
	printf("\npacket %d:\n", counter); 
	printf("Source MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", ethHdr->h_source[0], ethHdr->h_source[1], ethHdr->h_source[2], ethHdr->h_source[3], ethHdr->h_source[4], ethHdr->h_source[5]);		
	printf("Destination MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", ethHdr->h_dest[0], ethHdr->h_dest[1], ethHdr->h_dest[2], ethHdr->h_dest[3], ethHdr->h_dest[4], ethHdr->h_dest[5]);
	printProtocol(ipHdr);
	printf("IP->src_ip = %s\n", inet_ntoa(*(struct in_addr*)&ipHdr->saddr));
	printf("IP->dst_ip = %s\n", inet_ntoa(*(struct in_addr*)&ipHdr->daddr));
	printf("Src_port = %d\n", ntohs(tcpHdr->source));
	printf("Dst_port = %d\n", ntohs(tcpHdr->dest));
}

int main(int argc, char *argv[]){
	struct ethhdr *ethHdr;
	struct iphdr *ipHdr;
	struct tcphdr *tcpHdr;
	char *ptemp, buf[100];
	struct in_addr myIP, sourceIP, destIP;
	int sock, counter = 0;
	
	if((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){
		printf("Error creating socket!\n");
		exit(1);
	}
	
	struct ifreq ifr, origIfr;
	if(argc < 2)
        strncpy(ifr.ifr_name, "enp0s3", IFNAMSIZ);
    else
        strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
	
	if(ioctl(sock,SIOCGIFFLAGS,&ifr) == -1){
		perror("ioctl");
		exit(1);
	}
		
	myIP = getSocketIP(sock, ifr);
	printf("myIP : %s\n", inet_ntoa(myIP));
	
	origIfr = ifr;
	ifr.ifr_flags |= IFF_PROMISC;
	if(ioctl(sock, SIOCSIFFLAGS, &ifr) == -1){
		perror("ioctl");
		exit(1);
	} 
	
	while(counter < 5){
		recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
		ptemp = buf;
		ethHdr = (struct ethhdr *)ptemp;
		ipHdr = (struct iphdr *)(ptemp + sizeof(struct ethhdr));
		tcpHdr = (struct tcphdr *)(ptemp + sizeof(struct ethhdr) + (ipHdr->ihl << 2));
		sourceIP = *(struct in_addr*)&ipHdr->saddr;
		destIP = *(struct in_addr*)&ipHdr->daddr;
		if(ntohs(ethHdr->h_proto) == 0x0800 && ipHdr->protocol == IPPROTO_UDP && myIP.s_addr == destIP.s_addr){
			++counter;
			printPacketInfo(ethHdr, ipHdr, tcpHdr, counter);
		}
		
	}
	
	if(ioctl(sock, SIOCSIFFLAGS, &origIfr) == -1){
		perror("ioctl");
		exit(1);
	}
	close(sock);
}




