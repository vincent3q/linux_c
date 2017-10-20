#include "self_def_msg.h"
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>

static unsigned char msg_check(Msg *message)
{
    unsigned char s = 0;
    int i;
    for(i=0; i < sizeof( message->head ) ; i++){
        s += message->head[i];
    }

    for(i=0; i < sizeof( message->buffer ); i++){
        s += message->buffer[i];
    }

    return s;
}

int write_msg(int sockfd, char* buff, size_t len)
{
    Msg message;
    memset(&message, 0 , sizeof(message));
    strcpy(message.head,"iotek2017");
    memcpy(message.buffer, buff, len);

    message.checknum = msg_check(&message);
    if(write(sockfd, &message,
                     sizeof(message)) != sizeof(message)){
       return -1;
    }
}

int read_msg(int sockfd, char* buff, size_t len)
{
    Msg message;
    memset(&message, 0, sizeof(message));
    size_t size;
    if(( size = read(sockfd,
                    &message, sizeof(message))) < 0 ){
       return -1;
    }else if( 0 == size)
    {
       return 0;
    }

    unsigned char s = msg_check(&message);
    if( ( s == (unsigned char)message.checknum)
        && (!strcmp("iotek2017", message.head))){

        memcpy(buff, message.buffer, len);
        return sizeof(message);
    }

    return -1;
}
