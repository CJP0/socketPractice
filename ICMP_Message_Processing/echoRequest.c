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
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <limits.h>
#include <float.h>
unsigned short csum (unsigned short *buf, int n){
    unsigned long sum;
    for (sum = 0; n > 0; n--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

int main(int argc, char *argv[]){
	char *ptemp, buf[500], recvBuf[500];
	struct icmp *icmpHdr = (struct icmp *) buf, *icmpHdrRecv;
	struct ip *ipHdr;
	struct sockaddr_in destSin;
	struct  timeval start, end;
	double diff, mini = DBL_MAX, max = 0, sum = 0;
	int sockfd;
	
	memset(buf, 0, sizeof(buf));
	if((sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
		printf("Error creating socket!\n");
		exit(1);
	}
	destSin.sin_family = AF_INET;
	destSin.sin_addr.s_addr = inet_addr(argv[1]);
	
	icmpHdr->icmp_type = ICMP_ECHO;
	icmpHdr->icmp_code = 0;
	icmpHdr->icmp_hun.ih_idseq.icd_id = getpid();
	
	for(int i=0;i<10;++i){
		
		icmpHdr->icmp_hun.ih_idseq.icd_seq = i+1;
		icmpHdr->icmp_cksum = 0;
		icmpHdr->icmp_cksum = csum((unsigned short *) buf, (sizeof(struct icmp)));
		gettimeofday(&start,NULL);
		if(sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&destSin, sizeof(destSin)) < 0){
		   perror("sendto() error");
		   exit(-1);
		}
		
		recvfrom(sockfd, recvBuf, sizeof(recvBuf), 0, NULL, NULL);
		gettimeofday(&end,NULL);
		ipHdr = (struct ip *)recvBuf;
		icmpHdrRecv = (struct icmp *)(recvBuf + ((ipHdr->ip_hl)<<2));  
		
		diff = 1000 * (end.tv_sec-start.tv_sec) + (double)(end.tv_usec-start.tv_usec) / 1000;
		printf("replyfrom : %s, icmp_type = %d, icmp_code = %d, icmp_seq = %d, RTT : %.3lfms\n", inet_ntoa(*(struct in_addr*)&ipHdr->ip_src)\
		, icmpHdrRecv->icmp_type, icmpHdrRecv->icmp_code, icmpHdrRecv->icmp_hun.ih_idseq.icd_seq, diff);
		sum += diff;
		if(max<diff)
			max = diff;
		if(mini>diff)
			mini = diff;
		
	}
	printf("\nMax RTT : %.3lfms, Min RTT : %.3lfms, Avg RTT : %.3lfms\n", max, mini, sum/10);
	close(sockfd);
}