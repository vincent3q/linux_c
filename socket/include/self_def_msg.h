ifndef __MSG_H__
#define __MSG_H__

#include <sys/types.h>

typedef struct{
   char head[10]; // 协议头部
   char checknum; //校验码
   char buffer[512]; //数据
}Msg;

extern int write_msg(int sockfd,
                    char* buff, size_t len);

extern int read_msg(int sockfd,
                    char* buff, size_t len);

#endif
