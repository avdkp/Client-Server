#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<unistd.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<signal.h>

struct addr{
	int id;
	char name[25];
};


struct child{
int fd[2];
int fd_[2];
int cli_no;
char name[25];
int pid;
int newsockfd;
struct child *next_child;
struct sockaddr_in cli_addr;
};

struct msg{
int fun;		//0 for sending private msg/broadcasting; 1 for getlist; 2 for naming; 3 for killing child; 5 for New connections;
char message[256];
struct addr dstn;
struct addr src;
};






char * get_list(struct child *children){
struct msg *str;
char ch[10];
str = (struct msg*)malloc(sizeof(struct msg));
str->src.id = -1;
strcpy(str->src.name,"server");
str->message[0] = '\n';
strcat(str->message,"<Client_No>(<Client_Name>)\n");
struct child *p;
p = children;
while(p!=NULL){
sprintf(ch,"%d",p->cli_no);
strcat(str->message,ch);
strcat(str->message,"(");
strcat(str->message,p->name);
strcat(str->message,")");
strcat(str->message,"\n");
p=p->next_child;
}

return (char *)str;

}






void do_processing(struct child *new_child){

struct sockaddr_in cli_addr;
cli_addr = new_child->cli_addr;
int newsockfd = new_child->newsockfd;
struct msg *data;
char str[25];
data = (struct msg *)malloc(sizeof(struct msg));
inet_ntop(AF_INET,&(cli_addr.sin_addr.s_addr),str,sizeof(str));
int pid = fork();
if(pid <0){ printf("\nFORK ERROR..."); return; }
while(1){
	if(pid > 0){
		
		bzero((char *)data,sizeof(struct msg));
		if(read(newsockfd,(char *)data,sizeof(struct msg)) > 0 ){
			if(data->dstn.id == 0 && data->fun == 5){	// New connection.......
				data->src.id = new_child->cli_no;
				strcpy(new_child->name,data->src.name);
				struct msg *data1;
				data1 = (struct msg*)malloc(sizeof(struct msg));
				sprintf(data1->message,"%d",new_child->cli_no);  //Sending Client id to client......
				write(newsockfd,(char*)data1,sizeof(struct msg));
			}
			if(data->dstn.id == -1 && data->fun == 2) strcpy(new_child->name,data->src.name);
			write(new_child->fd[1],(char *)data,sizeof(struct msg));
		}
		else { printf("\nClient %s:%d disconnected...",str,ntohs(cli_addr.sin_port)); kill(pid,SIGKILL); return ;}
	}
	else{
		if(read(new_child->fd_[0],(char *)data,sizeof(struct msg)))
			write(newsockfd,(char *)data,sizeof(struct msg));
	}	
	
}
}



//***************************************************************************
char * client_name(struct child *children, int cli_no){
struct child *p;
p = children;
while(p!=NULL){
	if(p->cli_no == cli_no) return p->name;
	p=p->next_child;
}
return "server";
}



//*************************************************************************
struct child* data_processing(struct child *children){
struct child *p,*q,*r,*temp;
p = children;
q = children;
struct msg *data;
data = (struct msg *)malloc(sizeof(struct msg));

struct timeval tv;
tv.tv_sec = 0;
tv.tv_usec = 5000;
fd_set fds;
int fun,dstn;

while(p!=NULL){
	FD_ZERO(&fds);
	FD_SET(p->fd[0],&fds);
	if(select(p->fd[0]+1,&fds,NULL,NULL,&tv)){
		read(p->fd[0],(char *)data,sizeof(struct msg));
		printf("\n%d(%s)->%d(%s):%s(protocol:%d)",p->cli_no,p->name,data->dstn.id,client_name(children,data->dstn.id),data->message,data->fun);
		fflush(stdout);

	
		fun = data->fun;
		dstn = data->dstn.id;
		if(dstn == -1){
			if(fun == 1) if(write(p->fd_[1],get_list(children),sizeof(struct msg))<0) printf("\nWriting from parent to child error.:");
			if(fun == 2) strcpy(p->name,data->src.name);
			if(fun == 3){
				if(p!=children){
					q->next_child = p->next_child;
					temp = p;
					p = p->next_child;
					free(temp);
				}
				if(p==children){
					children = p->next_child;
					p = p->next_child;
					free(q);
				}
				continue;
			}
		}
		else if(dstn == 0){
			if(fun == 0){ //Broadcasting......
				r = children;	
				while(r!=NULL){
					if(r == p){ r=r->next_child; continue; }
					write(r->fd_[1],(char *)data,sizeof(struct msg)); 
					r=r->next_child;
				}
			}
			if(fun == 5){ //Broadcasting New connection......
				strcpy(p->name,data->src.name);
				char str[50],str1[20];
				strcat(str,"'");
				strcat(str,data->src.name);
				strcat(str,"' Client no:");
				sprintf(str1,"%d",data->src.id);
				strcat(str,str1);
				strcat(str," is online....");
				r = children;	
				while(r!=NULL){
					if(r == p){ r=r->next_child; continue; }
					strcpy(data->message,str);
					data->src.id = -1;
					strcpy(data->src.name,"server");
					data->dstn.id = r->cli_no;
					strcpy(data->dstn.name,r->name);
					write(r->fd_[1],(char *)data,sizeof(struct msg)); 
					r=r->next_child;
				}	
			}		
			
		}
		else{
			r = children;
			while(r!=NULL){
				if(r->cli_no == dstn){
					write(r->fd_[1],(char *)data,sizeof(struct msg)); 
					break;
				} 
				r=r->next_child;
			}
			if(r==NULL){
				strcpy(data->message,"INVALID 	ADDRESS");
				data->src.id = -1;
				strcpy(data->src.name,"server");
				data->dstn.id = p->cli_no;
				strcpy(data->dstn.name,p->name);
				write(p->fd_[1],(char *)data,sizeof(struct msg));	
			}
		}

	
	}
	q=p;
	p=p->next_child;
}
return children;
}





//*****************************************************************************
void f(){printf("\n...DONE...\n");}
int main(int argc, char **argv){
atexit(&f);
int sockfd, newsockfd, portno, clilen;
if(argc < 2){  printf("\nenter: %s [portno]",argv[0]); return 1;}
int n=1;
struct sockaddr_in serv_addr, cli_addr;

sockfd = socket(AF_INET, SOCK_STREAM, 0);
if(sockfd < 0) { printf("Error creating socket...."); exit(1); }

bzero(&serv_addr,sizeof(struct sockaddr_in));
portno = atoi(argv[1]);
char str[256];
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(portno);
serv_addr.sin_addr.s_addr = INADDR_ANY;

if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
 { printf("\nBinding Error...."); exit(2);}


struct child *children, *end;
children = NULL;
listen(sockfd,5);
clilen = sizeof(cli_addr);



struct timeval tv;
tv.tv_sec = 5;
tv.tv_usec = 5000;
fd_set fds;
FD_ZERO(&fds);
FD_SET(sockfd,&fds);

while(1){
	FD_ZERO(&fds);
	FD_SET(sockfd,&fds);
	if(select(sockfd+1,&fds,NULL,NULL,&tv)){
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if(newsockfd < 0) printf("Connection failed.......");
	inet_ntop(AF_INET,&(cli_addr.sin_addr.s_addr),str,sizeof(str));
	printf("\nSuccesfully connected to %s:%d\n",str,ntohs(cli_addr.sin_port));
	


	struct child *p,*new_child;
	p=children;
	if(p==NULL){
		new_child = (struct child*)malloc(sizeof(struct child));
		children = new_child;
		end = children;	
	}
	else{
		while(p!=NULL){end=p;p=p->next_child;}
		new_child = (struct child*)malloc(sizeof(struct child));
		end->next_child = new_child;
	}
	new_child->next_child = NULL;
	new_child->cli_no = n; n++;
	new_child->newsockfd = newsockfd;
	pipe(new_child->fd);
	pipe(new_child->fd_);
	new_child->cli_addr = cli_addr;

	
	int pid = fork();
	new_child->pid = pid;
	if(pid == 0){ 
			do_processing(new_child);
			printf("\n...Client %s:%d disconnected...",str,ntohs(cli_addr.sin_port));
			struct msg *exit_data;
			exit_data = (struct msg*)malloc(sizeof(struct msg));
			exit_data->dstn.id = -1;
			exit_data->fun = 3;   //protocol 3 for closed clients....
			write(new_child->fd[1],(char *)exit_data,sizeof(struct msg));
			 _exit(0); 
		}

	
	close(newsockfd);
	}
	
	children = data_processing(children); sleep(1);
}

printf("\n....\n");return 0;
}
