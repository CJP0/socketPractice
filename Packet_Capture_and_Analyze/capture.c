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
int main(int argc, char *argv[]){
	struct ethhdr *peth;
	struct iphdr *pip;
	char *ptemp, buf[100];
	int sock, counter = 0, ipCounter = 0, arpCounter = 0, rarpCounter = 0, tcpCounter = 0, udpCounter = 0, icmpCounter = 0, igmpCounter = 0;
	
	if((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){
		printf("Error creating socket!\n");
		exit(1);
	}
		
	struct ifreq ifr, origIfr;
	strncpy(ifr.ifr_name, "enp0s3", IFNAMSIZ);
	
	if(ioctl(sock,SIOCGIFFLAGS,&ifr) == -1){
		perror("ioctl");
		exit(1);
	}
	
	origIfr = ifr;
	ifr.ifr_flags |= IFF_PROMISC;
	if(ioctl(sock, SIOCSIFFLAGS, &ifr) == -1){
		perror("ioctl");
		exit(1);
	}
	
	while(counter++ < 200)
	{
		recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
		ptemp = buf;
		peth = (struct ethhdr *)ptemp;
		switch(ntohs(peth->h_proto))
		{
			case 0x0800:
				ipCounter++;
				break;
			case 0x0806:
				arpCounter++;
				break;
			case 0x8035:
				rarpCounter++;
				break;
		}
		ptemp += sizeof(struct ethhdr);
		pip = (struct iphdr *)ptemp;
		switch(pip->protocol)
		{
		case IPPROTO_TCP:
			tcpCounter++;
			break;
		case IPPROTO_UDP:
			udpCounter++;
			break;
		case IPPROTO_ICMP:
			icmpCounter++;
			break;
		case IPPROTO_IGMP:
			igmpCounter++;
			break;
		}
	}
	
	puts("------statistics------");
	printf("IP : %d\n", ipCounter);
	printf("ARP : %d\n", arpCounter);
	printf("RARP : %d\n", rarpCounter);
	printf("TCP : %d\n", tcpCounter);
	printf("UDP : %d\n", udpCounter);
	printf("ICMP : %d\n", icmpCounter);
	printf("IGMP : %d\n", igmpCounter);
	puts("-----finish-----");
	
	if(ioctl(sock, SIOCSIFFLAGS, &origIfr) == -1){
		perror("ioctl");
		exit(1);
	}
	close(sock);
}




