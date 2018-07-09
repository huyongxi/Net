#ifndef __SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <string.h>
#if (defined(WIN32) || defined(WIN64))
#include <WinSock2.h>
#include <Windows.h>
#define close closesocket;
#define bzero(dst,len) memset(dst, 0, len);
#pragma comment(lib,"WS2_32")
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

typedef sockaddr SA;
class SockAddr {
public:
    SockAddr(const char* ip, unsigned short port,int family = AF_INET):
        _ip(ip),_port(port)
    {
        addr = new sockaddr_in();
        bzero(addr,sizeof(sockaddr_in));
        addr->sin_family = family;
        addr->sin_port = htons(port);
        addr->sin_addr.s_addr = inet_addr(ip);
        //inet_pton(family,ip,&addr->sin_addr);
    }

    SockAddr(const sockaddr_in& saddr){
        addr = new sockaddr_in(saddr);
        _ip = inet_ntoa(saddr.sin_addr);
        _port  = ntohs(saddr.sin_port);
    }

    ~SockAddr(){
        delete addr;
    }

    const char* ip(){
        return _ip;
    }
    unsigned short port(){
        return _port;
    }
    const sockaddr_in& get_sockaddr_in(){
        return *addr;
    }

    friend std::ostream& operator<<(std::ostream& out, const SockAddr& addr);

private:
    sockaddr_in* addr;
    const char* _ip;
    unsigned short _port;
};



class Socket {
public:
    Socket(int family = AF_INET,int type = SOCK_STREAM,int protocol = 0){
        _fd = socket(family,type,protocol);
        if(_fd < 0){
            std::cout << "create socket failed!" << std::endl;
        }else{
            std::cout << "create socket success" << std::endl;
			valid = true;
        }
    }

    size_t send(void* buf ,size_t len);
    size_t recv(void* buf,size_t len);

    bool bind(const char* ip);
	bool connect(const char* ip, unsigned short port);
    bool listen(int num);
    Socket accept();

    int fd(){
        return _fd;
    }

    operator bool() const {
        return valid;
    }

    ~Socket(){
        close(_fd);
    }
private:
    int _fd = 0;
    bool valid = false;
};

#endif