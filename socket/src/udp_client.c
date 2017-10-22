#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>

void sig_handler(int signo)
{
   if( SIGINT == signo )
   {
      printf(" client close.\n ");
      exit(1);
   }
}


int main(int argc, char* argv[])
{
   if(argc < 3)
   { 
      printf(" usage: %s ip port \n", argv[0]);
      exit(1);
   }   

   int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   if(sockfd < 0)
   {
      perror(" create socket error ");
      exit(1);
   }

   struct sockaddr_in serveraddr;
   memset(&serveraddr,0,sizeof(serveraddr));

   serveraddr.sin_family = AF_INET;
   serveraddr.sin_port = htons( atoi(argv[2]) );
   inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr);


  char buffer[1024] = "hello iotek";

    /*if(connect(sockfd, (struct sockaddr*)&serveraddr,
                        sizeof(serveraddr)) < 0){
       perror(" connect error \n");
       exit(1);
    }
  
    if(send( sockfd, buffer, sizeof(buffer), 0 ) < 0 ){
       perror(" send msg error \n");
       exit(1);
    }else{
      
    }*/

   if(sendto(sockfd, buffer, sizeof(buffer), 0,
            (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
   
      perror("sendto error");
      exit(1);   
   }else{
      memset(buffer, 0, sizeof(buffer));
      if( recv(sockfd, buffer, sizeof(buffer), 0) < 0){
         perror(" recv error ");
         exit(1);
      }else{
         printf("%s \n ", buffer);
      }
   }
   
   close(sockfd);
   return 0;
}
