#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/

int main (int argc, char **argv)
{

//***************************************
 int listenfd, connfd, n;
 pid_t childpid;
 socklen_t clilen;
 char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;
 //*******************STUDENT VARIABLES************
int read_pipe[2];
int write_pipe[2];

if(pipe(read_pipe) == -1)
printf("PIPE FAILED!");

if(pipe(write_pipe) == -1)
printf("PIPE FAILED");

//close 1 end of read and write pipes
close(read_pipe[1]);
close(write_pipe[0]);

fcntl(read_pipe[0], F_SETFL, O_NONBLOCK);//non blocking assignment read pipe
fcntl(connfd, F_SETFL, SOCK_NONBLOCK);//set socket to non blocking
//********************************************************************


 //Create a socket for the server
 //If sockfd<0 there was an error in the creation of the socket
 if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  perror("Problem in creating the socket");
  exit(2);
 }


 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons(SERV_PORT);

 //bind the socket
 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listenfd, LISTENQ);

 printf("%s\n","Server running...waiting for connections.");

 for ( ; ; ) {

  clilen = sizeof(cliaddr);
  //accept a connection
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

  printf("%s\n","Received request...");



  if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process

    printf ("%s\n","Child created for dealing with client requests");

    //close listening socket
    close (listenfd);



//THIS READ BLOCKS WAITING FOR CLIENT. ADD OUTER LOOP THAT READS. 1) BLOCKS IF PIPE IS OPEN WAITING FOR INCOMING WRITE. 2) IF NO WRITE, PIPES ARE CLOSED, THEN ITS SKIPPED AND SERVER WIATS FOR INCOMING CLIENT DATA
   while(1){

//****READ PIPE IF DATA AVAILABLE THEN PRINT TO CLIENT**************
    if(read(read_pipe[0],buf,MAXLINE)!=0){//if pipe has data read data and send to client
            send(connfd, buf, n, 0);
            puts("READ PIPE TRIGGERED\n");//debugging print statement
         }

      //*******READ SOCKET DATA FROM CLIENT************NEED TO ADD 'IF(N !=-1)
      n = recv(connfd, buf, MAXLINE,0);
      printf("%s","String received from and resent to the client:");
      puts(buf);
      send(connfd, buf, n, 0);

      //student code
      //write to pipe which should flow to parent relay
      puts("ABOUT TO HIT WRITE TO PIPE");//test
       write(write_pipe[1],buf,MAXLINE);
       puts("afterWrite");//debugging print statement
      //check if incoming pipe data from parent server, then print to child

      puts("AFTER READ IF");//debugging print statement


    }

    if (n < 0)
      printf("%s\n", "Read error");
    exit(0);
  }



  //parent process <<<<STUDENT CODE>>>>>>>>
  if(childpid > 0){
  //string to pass on data to children
  char incoming_string[MAXLINE];
  //loop reading and writing to pipes, act as relay
  while(1){
  read(read_pipe[0],incoming_string,MAXLINE);
  write(write_pipe[1], incoming_string,MAXLINE);
  }
  }
 //close socket of the server: this exicutes when new client connects
 close(connfd);

 }

}