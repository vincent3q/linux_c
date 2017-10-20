#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "../include/self_def_msg.h"

int sockfd;

void sig_handler(int signo)
{
    if(signo == SIGINT)
    {
        printf("server close\n");
        close(sockfd);
        exit(1);
    }
    if( signo == SIGCHLD )
    {
        printf(" child process dead...\n ");
        wait(0); //回收
    }
}

void out_addr(struct sockaddr_in *clientaddr)
{
    int port = ntohs(clientaddr->sin_port);
    char ip[16];
    memset(ip,0,sizeof(ip));
    inet_ntop(AF_INET,
             &clientaddr->sin_addr.s_addr, ip, sizeof(ip));
    printf("client: %s(%d) conneted\n", ip, port);
}

void do_service(int fd)
{
    /*和客户端进行读写操作*/;
    char buff[512];
    while(1){
        memset(buff,0,sizeof(buff));
        printf(" start read and write...\n ");

        size_t size;
        if((size = read_msg(fd,
                             buff,sizeof(buff))) < 0){
           perror("protocal error");
           break;
        }else if( size == 0 ){
           break;
        }else{
           printf(" %s \n", buff);
           if(write_msg(fd,buff,sizeof(buff)) < 0) {
              if(errno == EPIPE)  // 客户端断开，不完整的管道，产生SIGPIPE信号
                 break;
              perror("protocal error");
           }
        }

    }
}

int main(int argc, char* argv[])
{
   if(argc < 2)
   {
       printf("usage: %s #port\n", argv[0]);
       exit(1);
   }

   // ctrl + c 终止服务
   if(signal(SIGINT, sig_handler) ==  SIG_ERR)
   {
       perror("signal sigint error");
       exit(1);
   }
   //子进程结束信号
   if(signal(SIGCHLD, sig_handler) ==  SIG_ERR)
   {
       perror("signal sigchild error");
       exit(1);
   }

   //步骤1:  创建socket
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   //步骤2: 绑定地址（ip，port)
   struct sockaddr_in serveraddr;
   memset(&serveraddr, 0 ,sizeof(serveraddr));
   serveraddr.sin_family = AF_INET;  //IPV4
   serveraddr.sin_port = htons(atoi(argv[1]));
   serveraddr.sin_addr.s_addr = INADDR_ANY;


  if(bind(sockfd, (struct sockaddr*)&serveraddr,
                             sizeof(serveraddr)) < 0){
       perror("bind error");
       exit(1);
   }


   // 步骤3: 监听
   if(listen(sockfd, 10) < 0){
       perror("listen error");
       exit(1);
   }

   // 步骤4: 调用accept
   struct sockaddr_in clientaddr;
   socklen_t clientaddr_len = sizeof(clientaddr);
   while(1){
       int fd = accept(sockfd,
                      (struct sockaddr*)&clientaddr,
                                        &clientaddr_len);
       if(fd < 0){
           perror("accept error");
           continue;
       }
       //步骤5: 启动子进程调用IO（read/write）

       pid_t pid = fork();
       if(pid < 0){
           continue;
       }else if( pid == 0){   // child process
          out_addr(&clientaddr);
          do_service(fd);
          close(fd);
          break;
       }else{
           close(fd);
       }
    }

   return 0;
}
