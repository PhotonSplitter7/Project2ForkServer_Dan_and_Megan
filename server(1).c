#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/

int main (int argc, char **argv)
{
 int listenfd, connfd, n;
 pid_t childpid = 1;//changed to start at not 0 so child process wont run until fork
 socklen_t clilen;
 char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;
//student variables
//pipe setup
int parent_to_child_pipe[2];
int child_to_parent_pipe[2];
pipe(parent_to_child_pipe);
pipe(child_to_parent_pipe);
//non blocking pipe ends
fcntl(parent_to_child_pipe[0], F_SETFL, O_NONBLOCK);//non blocking assignment
fcntl(child_to_parent_pipe[0], F_SETFL, O_NONBLOCK);//non blocking assignment
fcntl(parent_to_child_pipe[1], F_SETFL, O_NONBLOCK);//non blocking assignment
fcntl(child_to_parent_pipe[1], F_SETFL, O_NONBLOCK);//non blocking assignment



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

//DEBUGGING THIS WORKS SO FAR! SERVER DOESNT BLOCK WAITING FOR CONNECTION
fcntl(connfd, F_SETFL, O_NONBLOCK);//WORKS!!!!
fcntl(listenfd, F_SETFL, O_NONBLOCK);
//********************************

  clilen = sizeof(cliaddr);
  //accept a connection
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);//BLOCKS HERE WHEN WAITING FOR CONNECTION

//if client connected, then create child process and proceed, otherwise loop again and check for client connection
if(connfd <= 0){
if(errno == EWOULDBLOCK || errno == EAGAIN){
printf("\nNO CLIENTS CONNECTING!\n");
}
else
printf("\nSOMETING WONG!");
}

else{
  printf("%s\n","Received request...");
  childpid = fork ();
}


  if ( childpid == 0 ) {//if it’s 0, it’s child process

    //set socket to non blocking
    fcntl(connfd, F_SETFL, O_NONBLOCK);//WORKS!!!!
    fcntl(listenfd, F_SETFL, O_NONBLOCK);

    printf ("%s\n","Child created for dealing with client requests");


    //close listening socket
    close (listenfd);
    close(parent_to_child_pipe[1]);//listens to parent on this pipe
    close(child_to_parent_pipe[0]);//writes to parent on this pipe

char welcome[] = "WELCOME BUDDY";//DEBUGGING
send(connfd, welcome, n, 0);//DEBUGGING  ----this string only is sent when client tries to send data!!!!!!!!!!


    while (1)  {
    //read incoming socket data
      n = recv(connfd, buf, MAXLINE,0);

    //if incoming socket data resend to client
      if(n != -1) {
      //printf("%s","String received from and resent to the client:");
      //puts(buf);
      //send(connfd, buf, n, 0);
      write(child_to_parent_pipe[1],buf,MAXLINE);//if data received from client, pipe it to parent

      }
      //no socket data, check pipe for message and pass to client
      else{
//printf("empty socket!\n");//debugging
      int read_status;//int to hold pipe filled or empty status
      //read pipe and get read status
      read_status = read(parent_to_child_pipe[0], buf, MAXLINE);
      //if data, then run, else do nothing
      switch (read_status) {
             case -1:
                   //PIPE EMPTY DO NOTHING
                   //printf("\n(EMPTY SOCKET)");
                  break;
                    //case 0 means all bytes are read and EOF(end of conv.)
              case 0:
                      //do nothing if no data
                      //printf("\n(EMPTY SOCKET)");
                      break;
              default:
                 //if data read, pass it on to client
                 send(connfd, buf, n, 0);
                 break;
              }
        }
    }

    if (n < 0)
      printf("%s\n", "Read error");
    exit(0);
  }





  if(childpid > 0){//if > 0 its a parent process
  close(parent_to_child_pipe[0]);//listens to child on this pipe
  close(child_to_parent_pipe[1]);//writes to child on this pipe
  char parent_buf[MAXLINE];//buffer to hold data from children

  int parent_read_status;//int to hold pipe filled or empty status
        //read pipe and get read status
        parent_read_status = read(child_to_parent_pipe[0], parent_buf, MAXLINE);
        //if data, then run, else do nothing
        switch (parent_read_status) {
               case -1:
              // printf("\n(line 133!)\n");
                     //PIPE EMPTY DO NOTHING
                    break;
                      //case 0 means all bytes are read and EOF(end of conv.)
                case 0:
                        //do nothing if no data
                       // printf("\n(line 139!)\n");
                        break;
                default:
              //  printf("\n(line 142 exicuted special!!!\n");
                   //if data read, pass it on to children through pipe
                   write(parent_to_child_pipe[1],parent_buf,MAXLINE);
                   break;
                }
  }
 //close socket of the server
 //printf("\n(line 149)\n");
// char testPipeString[] = "THIS IS A TEST ON LINE 150!!!!!!!!!!\n";
// write(parent_to_child_pipe[1],testPipeString,MAXLINE);
 close(connfd);

 }

}