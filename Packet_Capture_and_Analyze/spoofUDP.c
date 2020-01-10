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
unsigned short csum (unsigned short *buf, int n){
    unsigned long sum;
    for (sum = 0; n > 0; n--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

int main(int argc, char *argv[]){
	char *ptemp, buf[1000];
	struct iphdr *ipHdr = (struct iphdr*)buf;
	struct udphdr *udpHdr = (struct udphdr*)(buf + sizeof(struct iphdr));
	struct sockaddr_in sourceSin, destSin;
	int sockfd, one = 1;
	memset(buf, 0, sizeof(buf));
	if((sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP)) < 0){
		printf("Error creating socket!\n");
		exit(1);
	}
	sourceSin.sin_family = AF_INET;
	destSin.sin_family = AF_INET;
	sourceSin.sin_addr.s_addr = inet_addr(argv[1]);
	destSin.sin_addr.s_addr = inet_addr(argv[3]);
	sourceSin.sin_port = htons(atoi(argv[2]));
	destSin.sin_port = htons(atoi(argv[4]));
	ipHdr->ihl = 5;
	ipHdr->version = 4;
	ipHdr->tos = 16;
	ipHdr->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr);
	ipHdr->id = htons(500);
	ipHdr->frag_off = 0;
	ipHdr->protocol = 17;
	ipHdr->ttl = 128;
	ipHdr->saddr = inet_addr(argv[1]);
	ipHdr->daddr = inet_addr(argv[3]);
	
	udpHdr->source = htons(atoi(argv[2]));
	udpHdr->dest = htons(atoi(argv[4]));
	udpHdr->len = htons(sizeof(struct udphdr));
	ipHdr->check = csum((unsigned short *) buf, (sizeof(struct iphdr) + sizeof(struct udphdr)));
	if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0){
		perror("setsockopt() error");
		exit(-1);
	}
	else
		printf("setsockopt() OK\n");
	printf("source : %s:%u, dest : %s:%u\n", argv[1], atoi(argv[2]), argv[3], atoi(argv[4]));
	for(int i=0;i<5;++i){
		if(sendto(sockfd, buf, ipHdr->tot_len, 0, (struct sockaddr *)&sourceSin, sizeof(sourceSin)) < 0){
		   perror("sendto() error");
		   exit(-1);
		}
		else
			printf("packet %d sendto() OK\n", i);
	}
	close(sockfd);
}