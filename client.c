#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/

//student code
int main(int argc, char **argv){

int p1[2];
pipe(p1);

fcntl(p1[1], F_SETFL, O_NONBLOCK);//non blocking assignment
fcntl(p1[0], F_SETFL, O_NONBLOCK);//non blocking assignment


//teacher client code***********************************************
int sockfd;//file descriptor for socket

 struct sockaddr_in servaddr;
 char sendline[MAXLINE], recvline[MAXLINE];





 //basic check of the arguments
 //additional checks can be inserted
 if (argc !=2) {
  perror("Usage: TCPClient <IP address of the server");
  exit(1);
 }

 //Create a socket for the client
 //If sockfd<0 there was an error in the creation of the socket
 if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  perror("Problem in creating the socket");
  exit(2);
 }

 //Creation of the socket
 memset(&servaddr, 0, sizeof(servaddr));
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr= inet_addr(argv[1]);
 servaddr.sin_port =  htons(SERV_PORT); //convert to big-endian order

 //Connection of the client to the socket
 if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
  perror("Problem in connecting to the server");
  exit(3);
 }

//student*****************************************
//************************************************

int child = fork();

if(child > 0){//parent
char buf1[MAXLINE];
fcntl(sockfd, F_SETFL, O_NONBLOCK);//non blocking socket SO close!
close(p1[1]);

while(1){
int readStatus = read(p1[0],buf1,MAXLINE);//read pipe NON BLOCKING

if(readStatus > 0){
//printf("you wrote : %s", buf1);//debugging
send(sockfd, buf1, strlen(buf1), 0);//send to server
}

int recv_stat = recv(sockfd, recvline, MAXLINE,0);//read data from server

if (recv_stat == 0){
   //error: server terminated prematurely NOT ORIGINAL
   perror("The server terminated prematurely");
   exit(4);
  }

if(recv_stat > 0){//if data print it
  printf("%s", "String received from the server: ");
  fputs(recvline, stdout);//print data on screen
  sleep(1);
}

}
}


if(child == 0){//child
close(p1[0]);

while(1){
char buf2[MAXLINE];
fgets(buf2, MAXLINE, stdin);
write(p1[1],buf2,MAXLINE);
}
}


exit(0);
}