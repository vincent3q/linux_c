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
#include <pthread.h>
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
}

    /*和客户端进行读写操作*/;
void do_service(int fd)
{
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

void out_fd(int fd)
{
   struct sockaddr_in addr;
   socklen_t len = sizeof(addr);

   if(getpeername(fd,
                  (struct sockaddr*)&addr, &len) < 0){
      perror(" getpeername error ");
      return;
   } 
   
   char ip[16];
   memset(ip, 0, sizeof(ip));
   int port = ntohs(addr.sin_port);
   inet_ntop(AF_INET,
             &addr.sin_addr.s_addr, ip, sizeof(ip));

   printf(" %16s (%5d) closed.\n ", ip, port);  
}

void* th_func(void* arg)
{
    int fd = (int)arg;
    do_service(fd);
    out_fd(fd);
    close(fd);
  
    return (void*)0;
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

   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr,
                    PTHREAD_CREATE_DETACHED);

   while(1){
       int fd = accept(sockfd,NULL,NULL);
       if(fd < 0){
           perror("accept error");
           continue;
       }
       //步骤5: 启动子线程调用IO（read/write）

       pthread_t th;
       int err;
       //以分离状态启动子线程
       if((err = pthread_create(&th, &attr,
                               th_func, (void*)fd)) != 0){
          perror("pthread create error");
       }   
    }
   pthread_attr_destroy(&attr);

   return 0;
}
