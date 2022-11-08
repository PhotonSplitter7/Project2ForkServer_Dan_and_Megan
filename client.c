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

//TODO after printing message in client window, print a bunch of spaces "                " of multiple lines to clear out below
#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/

int
main(int argc, char **argv) 
{
 int sockfd;
 struct sockaddr_in servaddr;
 char sendline[MAXLINE], recvline[MAXLINE];
 //student variables****
 pid_t child = 1;
 //pipe setup for communicating with child client 1)child client listens for input for user, blocking. 2) parent loops non blocking waiting for either socket data from server or pipe data
 //from child client
 int p1[2];
 pipe(p1);
 fcntl(p1[0], F_SETFL, O_NONBLOCK);//non blocking assignment
 fcntl(p1[1], F_SETFL, O_NONBLOCK);//non blocking assignment
	
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

 //student code ************************
 fcntl(sockfd, F_SETFL, O_NONBLOCK);//make socket non blocking
 //fork to parent and child
 child = fork();


 //parent process
 if(child > 0){
 //close pipe to write, read only
 close(p1[1]);

 while(1){

 //read input MOVE TO CHILD
 	fgets(sendline, MAXLINE, stdin);

 	//send to server
   send(sockfd, sendline, strlen(sendline), 0);

   if (recv(sockfd, recvline, MAXLINE,0) == 0){
    //error: server terminated prematurely
    perror("The server terminated prematurely");
    exit(4);
   }
   printf("%s", "String received from the server: ");
   fputs(recvline, stdout);
 }

 }

//child process to wait for user input
if(child == 0){
//close pipe to read, write only
close(p1[0]);
while(1){
char child_buf[MAXLINE];
//read input
fgets(child_buf, MAXLINE, stdin);
write(p1[1],child_buf,MAXLINE);
}
}





 exit(0);
}
