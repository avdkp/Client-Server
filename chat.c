#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void show_server(){
system("arp > _temp");
FILE *fp = fopen("_temp","rb");
char str[100];
fscanf(fp,"%[^\n]%*c",str);
int n=0;
while(1){
	fscanf(fp,"%[^\n]%*c",str);
	n++;
	
	if(feof(fp)) break;
}




fseek(fp,0,SEEK_SET);
char ip[n][25];
fscanf(fp,"%[^\n]%*c",str);
int i=0;
while(i<4){
	fscanf(fp,"%[^\n]%*c",str);
	sscanf(str,"%s",ip[i]);
	
	i++;
}
fclose(fp);
system("rm _temp");


int a[n]; for(i=0;i<n;i++) a[i] = 0;
int sockfd;
char port[10];
strcpy(port,"4444");
struct sockaddr_in serv_addr;
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(atoi(port));
sockfd = socket(AF_INET,SOCK_STREAM,0);
int flags = fcntl(sockfd, F_GETFL, 0);
fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
for(i=0;i<n;i++){
	serv_addr.sin_addr.s_addr = inet_addr(ip[i]);
	if( connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) > 0){ 
		a[i] = 1;
		printf("\n%s is active..",ip[i]); fflush(stdout);
	}
	
}


printf("\nServers online are...");
for(i=0;i<n;i++)if(a[i]) printf("\n%s",ip[i]);


system("netstat -a | grep 4444 | wc -l > _temp");
fp = fopen("_temp","rb");
fscanf(fp,"%d",&i);
fclose(fp);
system("rm _temp");
if(i) printf("\n127.0.0.1");

}







void f(){printf("\n...DONE...\n");}
int main(int argc, char **argv){
atexit(&f);
if(argc == 1){
printf("usage: %s show_server|create_server|join_server <IP>",argv[0]);
exit(0);
}
if(!strcmp(argv[1],"show_server")) show_server();
if(!strcmp(argv[1],"create_server")){
	system("netstat -a | grep 4444 | wc -l > _temp");
	FILE *fp = fopen("_temp","rb");
	int i;
	fscanf(fp,"%d",&i);
	fclose(fp);
	system("rm _temp");
	if(i){
		printf("Server already created...");
		exit(0);
	}
	printf("Creating Server...");
	int pid = fork();
	if(pid == 0){
		umask(0);
		setsid();
		close(STDIN_FILENO);
        	close(STDOUT_FILENO);
        	close(STDERR_FILENO);
		system("./server 4444");
	}
	

}
if(!strcmp(argv[1],"join_server")){
	if(argc < 3){ printf("usage: %s join_server <IP>",argv[0]); exit(0);}
	char str[50];
	strcpy(str,"./client ");
	strcat(str,argv[2]);
	strcat(str," 4444");
	system(str);



}



return 0;
}	
