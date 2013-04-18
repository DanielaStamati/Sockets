#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>

#define MAX_CLIENTS  5
#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(1);
}

typedef struct client {
	
	char *name;
	char *IP;
	char *shared[20];
	int shared_no;
	int nr_msg;
	int  port;
	int socket;
	int n;
	unsigned long init_time;	

} client;


char *unsharefile(char *filename,int socket, client clienti[], int cli_no)
{
	int i =	getIndexBySocket(socket,clienti,cli_no);
	int j;
	char *ret = strdup("INFO: You do not share this file.");
 
	for (j=0; j<clienti[i].shared_no; j++){

		if (strcmp(clienti[i].shared[j],filename)==0){
			clienti[i].shared[j]="";
			sprintf(ret,"INFO: File \"%s\" unshared.",filename);
		}
	}

	return ret;
}


//imi scoate spatiile albe de la sfarsitul inputului
char *trimming (char *rawFileName)
{
	int i = strlen(rawFileName);
	char *ret=strdup("");	
	while (!isalpha(rawFileName[i])){
		i--;
	}
	
	strncpy(ret, rawFileName, i+1);
	return ret;
}




char *sharefile(char *filename,int socket, client clienti[], int cli_no)
{
	int i =	getIndexBySocket(socket,clienti,cli_no);
	int j;
	char *ret = strdup("INFO: File already shared.");


	for (j=0; j<clienti[i].shared_no; j++)
		if (strcmp(clienti[i].shared[j],filename)==0)
			return ret;

	clienti[i].shared[clienti[i].shared_no] = strdup (filename);
	clienti[i].shared_no++;

	sprintf(ret,"INFO: File \"%s\" shared.",filename);
	return ret;
}
								


// intoarce lista clientilor conectati 
char * listClients(client clienti[], int cli_no)
{
	char *clients = strdup("Clients connected:");
	int i;
	
	for (i = 0; i<cli_no; i++)
		if (strlen(clienti[i].name)>0){

			char *aux = strdup(clienti[i].name);
			//sprintf(clients,"%s\n%s",clients,aux);
			strcat(clients,"\n");
			strcat(clients,aux);
		}
	strcat(clients,"\n");
	strcat(clients,"\0");
	
		
	return clients;
}

// intoarce informatia pentru un client
char * getClientInfo(client clienti[], int cli_no, char *client)
{
	int i;
	char *info = strdup ("This client is not connected.\n");
	for (i=0; i<cli_no; i++){

			if (strncmp(clienti[i].name,client,strlen(clienti[i].name))==0)
				{
					time_t seconds;
					seconds = time (NULL); // timpul curent
					unsigned long time = seconds - clienti[i].init_time; //timpul de la inceputul conexiunii
 					
					sprintf(info,"Name: %s\nConnected on port: %d\nConnected for %lu seconds",
							client,clienti[i].port,time);
					strcat(info,"\n");
					strcat(info,"\0");
					return info;
				}
	}

	strcat(info,"\n");
	strcat(info,"\0");
	return info;
}



//intoarce fisierele partajate de un client
char * getShare(client clienti[], int cli_no, char *client)
{
	int i;
	int j;
	int sharedNo = 0;
	char *shareList = strdup ("Shared files:\n");
	for (i=0; i<cli_no; i++){

			if (strncmp(clienti[i].name,client,strlen(clienti[i].name))==0)
				{
					for (j=0; j<clienti[i].shared_no; j++)
						if (strcmp(clienti[i].shared[j],"")!=0){
							sharedNo++;
							sprintf(shareList,"%s\n%s",shareList,clienti[i].shared[j]);	
						}				

					strcat(shareList,"\n");
					strcat(shareList,"\0");
					//printf("%s",shareList);
					if (sharedNo == 0)
						shareList  = strdup ("No files are shared.");
					return shareList;
				}
	}

	sprintf(shareList,"This client is not connected.\n");
	return shareList;
}
				

//intoarce pozitia in vectorul de clienti cautand dupa valoarea socketului pe care e conectat
int getIndexBySocket (int socket, client *clienti, int cli_no )
{
	int i;	
	for (i=0; i<cli_no; i++)
		if (clienti[i].socket == socket){

			return i;
			
		}

	printf("Nu exista socketul!\n");
	return -1;
}

//intoarce pozitia in vectorul de clienti cautand dupa numele clientului
int getIndexByName(client clienti[], char *name, int cli_no)
{
	int i;
	for (i=0; i<cli_no; i++)
		if(strcmp(name, clienti[i].name)==0)
			return i;
	return -1;
}

//intoarce socketul cautand dupa numele clientului
int getSocketByName(client clienti[], char *name, int cli_no)
{
	int i;
	for (i=0; i<cli_no; i++)
		if(strcmp(name, clienti[i].name)==0)
			return clienti[i].socket;
	return -1;
}


//intoarce toti clientii conectati IP-ul, port-ul si fisirele partajate
char *getStatus(client clienti[], int cli_no)
{

	int i,j,hasFiles = 0, hasClients = 0;
	char *status = strdup("Clients connected:\n\n");
	for (i=0; i<cli_no; i++)
		if (clienti[i].socket>-1){//ip, port, fisiere
			hasClients = 1;
			sprintf(status,"%sClient %s: \n\tIP:%s\n\tPORT:%d\n\tShared files:\n\t\t",status,clienti[i].name, clienti[i].IP, clienti[i].port);
			hasFiles = 0;
			for (j=0; j<clienti[i].shared_no; j++)
				if (strcmp(clienti[i].shared[j],"")!=0){
					hasFiles = 1;
					sprintf(status,"%s%s\n\t\t",status,clienti[i].shared[j]);
				}
			if (hasFiles==0){
				sprintf (status,"%sNONE\n",status);
			}
		}

	if(hasClients==0)
		status = "No clients are connected.\n";
			
	return status;
}
			
	



		
int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, j;
	client clienti[20];
	int cli_no = 0;
	int apeluri=0;
	 

    fd_set read_fds;	//multimea de citire folosita in select()
    fd_set tmp_fds;	//multime folosita temporar 
    int fdmax;		//valoare maxima file descriptor din multimea read_fds

    if (argc < 2) {
         fprintf(stderr,"Usage : %s port\n", argv[0]);
         exit(1);
     }

     //golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
     
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
     
    portno = atoi(argv[1]);

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
    serv_addr.sin_port = htons(portno);
     
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
              error("ERROR on binding");
     
	listen(sockfd, MAX_CLIENTS);

    //adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;
	
    FD_ZERO(&read_fds);
    FD_SET(0,&read_fds);
    FD_SET(sockfd,&read_fds);

    // main loop
	while (1) {
		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");

		if( FD_ISSET(0,&tmp_fds) ){    
  			//citesc de la tastatura
	    		memset(buffer, 0 , BUFLEN);
	    		fgets(buffer, BUFLEN-1, stdin);
				printf("Citit de la tastatura : %s\n",buffer);
			
			//se solicita statutul serverului			
			if (strncmp(buffer,"status",6)==0){
					
				char *status  = getStatus(clienti,cli_no);
				printf("%s",status);

			}

			//se solicita inchiderea conxiunii			
			if (strncmp(buffer,"quit",4)==0){
			
				
				printf ("\n******** CONNECTION CLOSED ********\n");
				return;
							

			}
		


			FD_CLR(0, &tmp_fds); 
		}
		
	
		for(i = 1; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
			
				if (i == sockfd) {
					// a venit ceva pe socketul de ascultare = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1 ) {
						
						error("ERROR: Connection denied.");
					} 
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}

					time_t seconds;
					seconds = time (NULL);
					clienti[cli_no].IP = strdup(inet_ntoa(cli_addr.sin_addr));
					clienti[cli_no].port = ntohs(cli_addr.sin_port);
					clienti[cli_no].init_time = seconds;
					clienti[cli_no].nr_msg = 0;
					clienti[cli_no].socket = newsockfd;
					clienti[cli_no].shared_no = 0;
					
					printf("New connection established. IP: %s, port: %d, time: %lu, socket no: %d\n", 
							clienti[cli_no].IP, clienti[cli_no].port,clienti[cli_no].init_time, clienti[cli_no].socket);
					cli_no++;
				
		
				}
					
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis	
							int loc = getIndexBySocket(i,clienti,cli_no);
							clienti[loc].socket=-1;
							clienti[loc].name = "";
							close(i);
							printf("\nINFO: socket %d hung up\n", i);
						} else {
							error("ERROR in recv");
						}
						close(i); 
						FD_CLR(i, &read_fds);//scot socketul din lista
					} 
					
					else { //recv intoarce >0
						
							int loc = getIndexBySocket(i,clienti,cli_no);
							if (clienti[loc].nr_msg == 0){
								clienti[loc].name = "";
								//verific daca mai exista un client cu acelasi nume in sistem
								if (getIndexByName(clienti,buffer,cli_no)>-1){
										char *auxBuff;										
										bzero(auxBuff,strlen(auxBuff));

										auxBuff = strdup("ERROR: The connection with this client is already established.");										
										clienti[loc].socket=-1;
										clienti[loc].name = "";
										n = send(i,auxBuff,strlen(auxBuff), 0);
    												if (n < 0) 
        		 										error("ERROR writing on socket");
										FD_CLR(i, &read_fds);
										close(i);
										printf ("Connection on socket %d closed\n",i);
									}
								else{// daca este un client nou setez numele
										
										clienti[loc].name = strdup(buffer);												
										printf("Client name: %s\n",clienti[loc].name);
										clienti[loc].nr_msg++;
									}
							}
							else{// daca numele este deja setat => am primit un mesaj
								printf ("Message recieved on socket %d: %s\n",i, buffer);
								clienti[loc].nr_msg++;
								if (strncmp(buffer,"listclients",11)==0){
																	
										char *clients = strdup(listClients(clienti,cli_no));
										printf("%s",clients);
										n = send(i,clients,strlen(clients), 0);
    												if (n < 0) 
        		 										error("ERROR writing to socket");	
										}
								else if (strncmp(buffer,"infoclient",10)==0){
										
											char *nm;										
											nm = strtok(buffer," ");
											nm = strtok(NULL," ");
											
											char *info = getClientInfo(clienti,cli_no,nm);
											//printf("%s",info);
											n = send(i,info,strlen(info), 0);
														if (n < 0) 
        		 										error("ERROR writing to socket");														
										
										}

								else if (strncmp(buffer,"message",7)==0){
										
											char *nm, *buff;
											buff = strdup(buffer);

											nm = strtok(buff," ");
											nm = strtok(NULL," ");

											//"scot" din buffer primele strlen("message")+1+strlen(nume client)+1
											char *message = strdup(buffer+8+strlen(nm)+1);
											int sock = getSocketByName(clienti,nm,cli_no);

											if (sock==-1){
														n = send(i,"This client is not connected.",strlen("This client is not connected."), 0);
														if (n < 0) 
        		 										error("ERROR writing to socket");
														}	
											else{
														n = send(sock,message,strlen(message), 0);	
														if (n < 0) 
        		 										error("ERROR writing to socket");
													}
											}

								else if (strncmp(buffer,"sharefile",9)==0){
										
										char *filename = trimming(strdup(buffer+10));
										//char *filename = strdup(buffer+10);
										char *fileShared = sharefile(filename,i,clienti, cli_no);
										
										n = send(i,fileShared,strlen(fileShared), 0);	
														if (n < 0) 
        		 										error("ERROR writing to socket");										 
																							
										
										}

								else if (strncmp(buffer,"unsharefile",11)==0){
										
										char *filename = trimming(strdup(buffer+12));
										//char *filename = strdup(buffer+10);
										
										char *fileUnShared = unsharefile(filename,i,clienti, cli_no);
										
										n = send(i,fileUnShared,strlen(fileUnShared), 0);	
														if (n < 0) 
        		 											error("ERROR writing to socket");												 
																							
										
										}
								

								else if (strncmp(buffer,"getshare",8)==0){
										
										char *nm;										
											nm = strtok(buffer," ");
											nm = strtok(NULL," ");
											
											char *info = getShare(clienti,cli_no,nm);

											n = send(i,info,strlen(info), 0);
														if (n < 0) 
        		 										error("ERROR writing to socket");										 
																							
										
										}


									else{

											n = send(i,"Unknown command.",strlen("Unknown command."), 0);
														if (n < 0) 
        		 										error("ERROR writing to socket");	
										}
													



								/*else if (strncmp(buffer,"getfile",7)==0){
										
										cadru seq;

										char *buff = strdup(buffer);
										char *name = strtok(buff," ");
										name = strtok(NULL," ");
										int sock = getSocketByName(clienti,name,cli_no);
	
										if (sock == -1)
												n = send(i,"This client is not connected.",strlen("This client is not connected."), 0);
														if (n < 0) 
        		 										error("ERROR writing to socket");

										//"scot" din buffer primele strlen("message")+1+strlen(nume client)+1
										char *filename = strdup(buffer+8+strlen(name)+1);
										filename = trimming(filename);

										int fd = open(filename, O_RDONLY);

											if (fd<0){
												n = send(i,"The file does not exist.",strlen("The file does not exist."), 0);
														if (n < 0) 
        		 										error("ERROR writing to socket");
											 }
																		
										 while ( (seq.infoQ= read(fd,&seq.BUFF, 1024))>0){
												
											char *toSend=strdup("");
											bzero (toSend,strlen(toSend));
											
											sprintf(toSend,"%d%s",seq.infoQ,seq.BUFF);

											if (seq.infoQ<1024)
												close(fd);

											n = send(sock, toSend, strlen(toSend), 0);
														if (n < 0) 
        		 										error("ERROR writing to socket");
										}
								
								}*/	
																		
									
								}
					}
				} 
			}
		}
     }


     close(sockfd);
   
     return 0; 
}


