
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
 //char buf[MAXLINE];//MOVED BELOW TO CLEAR ON EACH LOOP
 struct sockaddr_in cliaddr, servaddr;
//student variables
//pipe setup
//int parent_to_child_pipe[2];
//int child_to_parent_pipe[2];
//pipe(parent_to_child_pipe);
//pipe(child_to_parent_pipe);
//non blocking pipe ends
//fcntl(parent_to_child_pipe[0], F_SETFL, O_NONBLOCK);//non blocking assignment
//fcntl(child_to_parent_pipe[0], F_SETFL, O_NONBLOCK);//non blocking assignment
//fcntl(parent_to_child_pipe[1], F_SETFL, O_NONBLOCK);//non blocking assignment
//fcntl(child_to_parent_pipe[1], F_SETFL, O_NONBLOCK);//non blocking assignment



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
//printf("\nNO CLIENTS CONNECTING!\n");
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
    //set pipes to non blocking
//    fcntl(parent_to_child_pipe[0], F_SETFL, O_NONBLOCK);//non blocking assignment
//    fcntl(child_to_parent_pipe[0], F_SETFL, O_NONBLOCK);//non blocking assignment
//    fcntl(parent_to_child_pipe[1], F_SETFL, O_NONBLOCK);//non blocking assignment
//    fcntl(child_to_parent_pipe[1], F_SETFL, O_NONBLOCK);//non blocking assignment

    //close listening socket
    close (listenfd);
//    close(parent_to_child_pipe[1]);//listens to parent on this pipe
//    close(child_to_parent_pipe[0]);//writes to parent on this pipe

    printf ("%s\n","Child created for dealing with client requests");


    while (1)  {
    //buffer for storing data, cleared on each while loop
    char buf[MAXLINE];
    //read incoming socket data
      n = recv(connfd, buf, MAXLINE,0);

    //if incoming socket data resend to client
      //if server connection lost close child
      if(n == 0){
      printf("disconnect, closing child server!\n");
      exit(0);
      }
      //if data read print to console and send to client
      if(n > 0) {
      printf("%s","String received from and resent to the client:");//old
      puts(buf);//old
      send(connfd, buf, n, 0);//old
      //write(child_to_parent_pipe[1],buf,MAXLINE);//if data received from client, pipe it to parent ISSUE IS HERE!!!!!fails

      sleep(1);//debug
      }
//      //no socket data, check pipe for message and pass to client
//      else{
//
//         //sleep(1);
//      ///int to hold pipe filled or empty status
//      //read pipe and get read status
////      read_status = read(parent_to_child_pipe[0], buf, MAXLINE);
//      //if data, then run, else do nothing
////      switch (read_status) {
////             case -1:
////                   //PIPE EMPTY DO NOTHING
////                   printf("\n(EMPTY SOCKET1)");
////                   sleep(2);
////                  break;
////                    //case 0 means all bytes are read and EOF(end of conv.)
////              case 0:
////                      //do nothing if no data
////                      printf("\n(EMPTY SOCKET2)");
////                      sleep(2);
////                      break;
////              default:
////                 //if data read, pass it on to client
////                 send(connfd, buf, n, 0);
////                 printf("MESSAGE SENT TO PIPE!\n");
////                 sleep(2);
////                 break;
////              }
//        }



    }

    if (n < 0)
      printf("%s\n", "Read error");
    exit(0);
  }

 close(connfd);
 sleep(1);

 //if its above 0 its a parent process- only deals with pipes not sockets


     //set pipes to non blocking
//     fcntl(child_to_parent_pipe[0], F_SETFL, O_NONBLOCK);//non blocking assignment
//     fcntl(parent_to_child_pipe[1], F_SETFL, O_NONBLOCK);//non blocking assignment

     //close listening socket
//     close(parent_to_child_pipe[0]);
//     close(child_to_parent_pipe[1]);

 //printf("parent says HI!\n");

 char buf2[MAXLINE];
 int read_status;
// read_status = read(child_to_parent_pipe[0], buf2, MAXLINE);
//
// switch (read_status) {
//        case -1:
//
//             printf("(PIPE EMPTY)\n");
//             sleep(3);
//             break;
//
//         // case 0 means all bytes are read and EOF(end of conv.)
//         case 0:
//             printf("End of conversation\n");
//             sleep(1);
//             // read link
//             //close(child_to_parent_pipe[0]);
//
//             //exit(0);
//         default://if data read from pipe write to other server children
//
//             // text read
//             // by default return no. of bytes
//             // which read call read at that time
//             printf("writing to children!\n");
//             printf("%s\n", buf2);
//             write(parent_to_child_pipe[1],buf2, read_status);//program explodes here!!!!!
//             sleep(1);
//         }



 }

}