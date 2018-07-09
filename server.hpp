#ifndef __SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>


class Socket {
public:
    Socket(int family = AF_INET,int type = SOCK_STREAM,int protocol = 0){
        fd = socket(family,type,protocol);
        if(fd < 0){
            std::cout << "create socket failed!" << std::endl;
        }else{
            std::cout << "create socket success" << std::endl;
        }
    }

    size_t send(void* buf ,size_t len){

    }

    size_t recv(void* buf,size_t len){
        
    }

    ~Socket(){
        close(fd);
    }
private:
    int fd = 0;
};

#endif