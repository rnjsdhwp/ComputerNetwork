#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define PORT_NUM 6012
#define MAXLEN 256 
 
int Congwin, Threshold, sendnum, check, count, ackcount;
int sockfd;
char selectnum[MAXLEN];
struct sockaddr_in addr;
struct mydata sdata, recvaddr;
socklen_t addrlen;
char *state="noloss";

struct mydata{
	char packet[MAXLEN];
};
void print(){
	 printf(" -----------------------------------\n");
         printf("|    CongWin: %d, ThreshOld: %d      |\n",Congwin, Threshold);
         printf(" -----------------------------------\n");

}
void msgsend(){
	print();
	for(int start=0; start<Congwin; start++){
        	sprintf(sdata.packet, "%d", sendnum);
                sendto(sockfd, (void *)&sdata, sizeof(sdata), 0, (struct sockaddr *)&addr, addrlen);
                printf("               패킷%s 전송----->\n",sdata.packet);
                sendnum++;
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
		if(Congwin>=Threshold)	Congwin=Threshold;
        }else if(Congwin>=Threshold)	Congwin++;
}
void noLoss(){
	memset(sdata.packet, 0x00, MAXLEN);
		
	addrlen=sizeof(addr);
	char msg[MAXLEN]="1";
	sendto(sockfd, (void *)&msg, sizeof(sdata), 0, (struct sockaddr *)&addr, addrlen);
	
	while(count<8){
		msgsend();
		setCongwin();

		memset(sdata.packet, 0x00, MAXLEN);
		recvfrom(sockfd, (void *)&sdata, sizeof(sdata), 0, (struct sockaddr *)&recvaddr, &addrlen);
		printf("<--- %s 수신\n\n",sdata.packet);

		count++;
	}
}
void timeout(){
	memset(sdata.packet, 0x00, MAXLEN);
	addrlen=sizeof(addr);
	char msg[MAXLEN]="2";
        sendto(sockfd, (void *)&msg, sizeof(sdata), 0, (struct sockaddr *)&addr, addrlen);
	while(count<11){
                msgsend();

		memset(sdata.packet, 0x00, MAXLEN);
                recvfrom(sockfd, (void *)&sdata, sizeof(sdata), 0, (struct sockaddr *)&recvaddr, &addrlen);
		if(!strcmp(sdata.packet, "timeout")){
			printf("<--- %s 발생!!!\n\n",sdata.packet);
			state="timeout";
		}else{
			printf("<--- %s 수신\n\n",sdata.packet);
		}
		setCongwin();
		sleep(1);
                count++;		
	}
}
void threeACK(){
	memset(sdata.packet, 0x00, MAXLEN);
	addrlen=sizeof(addr);
	char msg[MAXLEN]="3";
        sendto(sockfd, (void *)&msg, sizeof(sdata), 0, (struct sockaddr *)&addr, addrlen);
	while(count<8){
                msgsend();
                memset(sdata.packet, 0x00, MAXLEN);
		recvfrom(sockfd, (void *)&sdata, sizeof(sdata), 0, (struct sockaddr *)&recvaddr, &addrlen);
		char *ptr=strtok(sdata.packet, ",");
		while(ptr!=NULL){
			ackcount++;
			printf("<--- %s 수신\n\n",ptr);
			if(ackcount==3)	state="3dupACK";
			ptr=strtok(NULL,",");
		}
		ackcount=0;
		setCongwin();

                count++;
        }
}
int main(int argc, char **argv){
        if (argc != 2){
                printf("Usage : %s [ipaddress]\n", argv[0]);
                return 1;
        }
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
                return 1;
        }
        memset((void *)&addr, 0x00, sizeof(addr));
        memset((void *)&sdata, 0x00, sizeof(sdata));
 
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(argv[1]);
        addr.sin_port = htons(PORT_NUM);
 
	while(1){
	        printf("\n\n -----------------------------------\n");
	     	printf("|    원하는 동작을 선택해주세요.    |\n");
		printf(" -----------------------------------\n");
		printf("|  1. 손실이 일어나지 않는 상태     |\n");
		printf("|  2. timeout이 발생했을 때         |\n");
		printf("|  3. 3dupACK가 발생했을 때         |\n");
		printf("|  4. 종료(quit 입력)               |\n");
		printf(" -----------------------------------\n");
		printf("선택: ");
		fgets(selectnum, MAXLEN, stdin);

		if(strncmp(selectnum, "quit\n", 5)==0)	break;
		
		sendnum=1;Congwin=1;Threshold=10;count=1;check=0;ackcount=0;
		int num=atoi(selectnum);
		switch(num){
			case 1:
				noLoss();
				break;
			case 2:
				timeout();
				break;
			case 3:
			        threeACK();	
				break;
			default:
				printf("없는 숫자입니다.\n");
				break;
		}
        }
        close(sockfd);
}
