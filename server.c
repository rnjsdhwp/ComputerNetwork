#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define PORT_NUM 6012 
#define MAXLEN 256  

int sockfd,sockop, count=1, num=0,tcount,ackcount,Congwin,Threshold;
char *ack = "ACK";
char *outtime="timeout";
char *dupACK="ACK,ACK,ACK";
char *state="noloss";
char msg[MAXLEN];
socklen_t addrlen;
struct sockaddr_in addr, cliaddr;
struct mydata rdata;

struct mydata{
        char packet[MAXLEN];
};
void print(){
         printf(" -----------------------------------\n");
         printf("|    CongWin: %d, ThreshOld: %d      |\n",Congwin, Threshold);
         printf(" -----------------------------------\n");

}
void msgrecv(){
        print();
        for(int start=0; start<Congwin; start++){
                recvfrom(sockfd, (void *)&rdata, sizeof(rdata), 0, (struct sockaddr *)&cliaddr, &addrlen);
                printf("<--- 패킷%s 수신\n", rdata.packet);
        }
}
void setCongwin(){
        if(!strcmp(state,"timeout")){
                Threshold=Congwin/2;
                Congwin=1;
                state="noloss";
                return;
        }
        if(!strcmp(state,"3dupACK")){
                Congwin=Congwin/2;
                Threshold=Congwin;
                state="noloss";
                return;
        }

        if(Congwin<Threshold){
                Congwin*=2;
                if(Congwin>=Threshold)  Congwin=Threshold;
        }else if(Congwin>=Threshold)    Congwin++;
}

void noloss(){
	while(count<8){
		msgrecv();
		sleep(1);
                setCongwin();

		memset(rdata.packet, 0x00, sizeof(rdata.packet));
		sscanf(ack, "%s", rdata.packet);
                sendto(sockfd, (void *)&rdata, sizeof(rdata), 0, (struct sockaddr *)&cliaddr, addrlen);
                printf("                   %s 전송----->\n\n",rdata.packet);
		count++;
	}
}
void timeout(){
	num=2;
	tcount=0;
	while(count<11){
                msgrecv();
		sleep(1);
		tcount++;

                memset(rdata.packet, 0x00, sizeof(rdata.packet));
		
		if(tcount==5){
			sleep(3);
			sscanf(outtime, "%s", rdata.packet);
			state="timeout";
		}else	sscanf(ack, "%s", rdata.packet);
		
		setCongwin();
                sendto(sockfd, (void *)&rdata, sizeof(rdata), 0, (struct sockaddr *)&cliaddr, addrlen);
                printf("                   %s 전송----->\n\n",rdata.packet);
                count++;
        }
}
void threeACK(){
	num=3;
        tcount=0;
	ackcount=0;
        while(count<8){
		msgrecv();
                sleep(1);
                tcount++;

                memset(rdata.packet, 0x00, sizeof(rdata.packet));

                if(tcount==5){
                        sscanf(dupACK, "%s", rdata.packet);
                        state="3dupACK";
                }else   sscanf(ack, "%s", rdata.packet);
		
                sendto(sockfd, (void *)&rdata, sizeof(rdata), 0, (struct sockaddr *)&cliaddr, addrlen);
                
		if(!strcmp(state,"3dupACK")){
			char *ptr=strtok(rdata.packet,",");
 	                while(ptr!=NULL){
                        	ackcount++;
				printf("                   %s 전송----->\n\n",ptr);
                        	if(ackcount==3) state="3dupACK";
                        	ptr=strtok(NULL, ",");
                	}
		}else	printf("                   %s 전송----->\n\n",rdata.packet);
                setCongwin();
		count++;
        }

}
int main(int argc, char **argv){
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
                return 1;
        }
        memset((void *)&addr, 0x00, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(PORT_NUM);

	sockop=1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockop, sizeof(sockop));        

	addrlen = sizeof(addr);
        if(bind(sockfd, (struct sockaddr *)&addr, addrlen) == -1){
                return 1;
        }
   
       	while(1){
                addrlen = sizeof(cliaddr);
		recvfrom(sockfd, (void *)&msg, sizeof(msg), 0, (struct sockaddr *)&cliaddr, &addrlen);
		
		count=1;Congwin=1;Threshold=10;
		num=atoi(msg);
		switch(num){
			case 1:
			     noloss();
			     break;
			case 2:
			     timeout();
			     break;
			case 3:
			     threeACK();
			     break;
		}
	}
	close(sockfd);
}
