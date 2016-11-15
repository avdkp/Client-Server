#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<signal.h>
#define HELP printf("\nCommands available: getlist,disconnect,name_<name>,send_<cli_id>,help");

struct addr{
	int id;
	char name[25];
};

struct msg{
	int fun; //1 getlist 2.naming 3.killing closed clients 4.Broadcasting 5.Newconnection 
	char message[256];
	struct addr dstn;
	struct addr src;
};


int main(int argc,char **argv)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));


	sockfd = socket(AF_INET,SOCK_STREAM,0);	
	if(sockfd == -1)
	{ 
		printf("Error creating socket...");
		return 1; 
	}

	if( connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
	{
		printf("\nError connecting to server...."); 
		return 1; 
	}

	char str[25];

	struct msg *data;
	data = (struct msg *)malloc(sizeof(struct msg));
	printf("Enter your name:");
	scanf("%s",data->src.name);
	data->fun = 5;
	data->dstn.id = 0;
	strcpy(data->dstn.name,"server");
	write(sockfd,(char *)data,sizeof(struct msg));
	struct msg *data1=(struct msg *)malloc(sizeof(struct msg));

	
	/*
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 5000;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sockfd+1,&fds);
	if(select(sockfd+1,&fds,NULL,NULL,&tv))

	*/

	while(1){// Getting Client id...
		if(read(sockfd,(char*)data1,sizeof(struct msg)) > 0)
		data->src.id = atoi(data1->message);
		if(data->src.id > 0) break;
		else printf("\nWaiting to get client id...");
	}
		printf("\nCongratulation your are connected your client id is %d",data->src.id);

	HELP
	int pid = fork();
	int n=0;
	
	

	
	while(1){

		if(pid == 0){
			printf("\ncommand:");scanf("%s",str);
			if(!strcmp(str,"getlist")){
				data->fun = 1;
				data->dstn.id = -1;
				strcpy(data->message,"getlist");
				write(sockfd,(char *)data,sizeof(struct msg));
			}

			else if(sscanf(str,"name_%s",data->src.name)){
				data->fun = 2;
				data->dstn.id = -1;
				write(sockfd,(char *)data,sizeof(struct msg));
			}


			else if(sscanf(str,"send_%d",&data->dstn.id) > 0) 
			while(1){printf(":");
				data->fun = 0;
				fflush(stdin); scanf("%[^.]%*[^\n]%*c",data->message);
				if(!strcmp(data->message,"disconnect")) break;
				if( write(sockfd,(char *)data,sizeof(struct msg)) < 0 ) 
				printf("\nSorry Message could not be sent..");
			}
			else HELP
		}

		

		if(pid > 0){
			//char str[sizeof(struct msg)];
			
			//bzero(str,sizeof(str));
			bzero((char *)data1,sizeof(struct msg));
			if(read(sockfd,(char*)data1,sizeof(struct msg)) > 0 ) 
			{
				//data1 = (struct msg *)str;
				printf("\n(\x1b[32m%d\x1b[0m)\x1b[32m%s\x1b[0m: \x1b[32m%s\x1b[0m\n",data1->src.id,data1->src.name,data1->message);}
			else{
				 printf("\nServer Disconnected....");
				 kill(pid,SIGKILL);
				 break;
			}
		}


	}



	
	printf("\n....\n"); return 0;
}







