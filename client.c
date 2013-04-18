#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFLEN 1028


void error(char *msg)
{
    perror(msg);
    exit(0);
}

// intoarce lungimea bufferului care urmeaza sa fie scris in fisierul primit
char *getNumber (char *buffer)
{  

	int i = 0;
	char *ret=strdup("");;

	while (!isalpha(buffer[i]))
		i++;
	strncpy(ret,buffer,i);

	return ret;
}


int main(int argc, char *argv[])
{
	int sockfd, n;
    int fdmax;
	int fd;
	char *client_name;
	char *filename = strdup ("");
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFLEN];
    if (argc < 3) {
       fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
       exit(0);
    }  
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    serv_addr.sin_family = AF_INET;
    
	client_name = strdup (argv[1]);				//Nume    
	inet_aton(argv[2], &serv_addr.sin_addr);	//IP
	serv_addr.sin_port = htons(atoi(argv[3]));	//PORT
    
    
    if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");// s-a realizat conexiunea

	n = send(sockfd,client_name,strlen(client_name), 0);
    		if (n < 0) 
        		 error("ERROR writing to socket");  //trimit numele   
    
    
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(0,&read_fds);
    FD_SET(sockfd,&read_fds);
    fd_set tmp_fds;
      
    while(1){
    	tmp_fds = read_fds; 
    	if (select(sockfd + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
		error("ERROR in select");

	if( FD_ISSET(sockfd,&tmp_fds) ){
		char buffer[BUFLEN];
		bzero(buffer,BUFLEN);
		int sz = recv(sockfd,buffer,100,0);

		// a cazut serverul		
		if (sz==0){
			printf ("\n********* SORRY, THE SERVER IS DOWN ********\n");			
			return;
		}


		// se scriu dat in fisier
		if (strcmp(filename,"")!=0){
			char *num = getNumber (buffer);
			int nr  = atoi(num);
			char *toWrite = buffer + strlen(num);
			write (fd,toWrite,nr);
			if (nr < 1024){
				filename = strdup("");		
				close(fd);
			}
			
		}		
			
		
		if (sz>0){
			printf("%s\n",buffer);
		}
			
		if (strncmp(buffer,"ERROR:",6)==0)
			return;
		
	 }
			
    
    	if( FD_ISSET(0,&tmp_fds) ){    
  			//citesc de la tastatura
		
    		memset(buffer, 0 , BUFLEN);
    		fgets(buffer, BUFLEN-1, stdin);
			printf("Citit de la tastatura : %s\n",buffer);
		
		if (strncmp(buffer,"quit",4)==0)
			return 0;
		
		/*if (strncmp(buffer,"getfile",7)==0){

			char *buff = strdup(buffer);
			char *name = strtok(buff," ");
			name = strtok(NULL," ");
			filename = strdup(buffer+8+strlen(name)+1);
			sprintf(filename,"%s_primit",filename);

			fd = open(filename,O_WRONLY|O_CREAT,0644);
  				if (fd<0) 
    				printf("\n Error creating the file. \n");
  				
		}*/
    		

			//trimit mesaj la server
    		n = send(sockfd,buffer,strlen(buffer), 0);
    		if (n < 0) 
        		 error("ERROR writing to socket");
        }
		usleep(100000);
    }
    return 0;
}


