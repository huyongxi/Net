#ifndef __SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <string.h>
#include <thread>
#include <list>
#include <queue>
#include <unordered_map>
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
#include <errno.h>
#include <stdio.h>
#endif
using namespace std;
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
        if( ip == ""){
            addr->sin_addr.s_addr = htonl(INADDR_ANY);
        }else{
            addr->sin_addr.s_addr = inet_addr(ip);
        }
        //inet_pton(family,ip,&addr->sin_addr);
    }

    SockAddr(const sockaddr_in* saddr){
        addr = new sockaddr_in();
        memcpy(addr, saddr, sizeof(sockaddr_in));
        _ip = inet_ntoa(saddr->sin_addr);
        _port  = ntohs(saddr->sin_port);
    }

    SockAddr(const SockAddr&) = delete;
    SockAddr& operator=(const SockAddr&) = delete;

    ~SockAddr(){
        delete addr;
    }

    const char* ip(){
        return _ip;
    }
    unsigned short port(){
        return _port;
    }
    const sockaddr_in* get_sockaddr_in(){
        return addr;
    }

    friend std::ostream& operator<<(std::ostream& out, const SockAddr& addr);

private:
    sockaddr_in* addr;
    const char* _ip;
    unsigned short _port;
};



class Socket {
public:
    Socket(int family = AF_INET,int type = SOCK_STREAM,int protocol = 0) {
        _fd = socket(family,type,protocol);
        if(_fd < 0){
            std::cout << "create socket failed!" << std::endl;
        }else{
            std::cout << "create socket success" << std::endl;
			valid = true;
        }
    }
    Socket(const char* empty){}

    Socket(const Socket& s){
        _fd = s._fd;
        valid = s.valid;
        addr = new SockAddr(s.get_sockaddr()->get_sockaddr_in());
    }
    Socket& operator=(const Socket& s){
        if(this != &s){
            _fd = s._fd;
            valid = s.valid;
            if(addr){
                delete addr;
            }
            addr = new SockAddr(s.get_sockaddr()->get_sockaddr_in());
        }
        return *this;
    }

   
    ssize_t readn(void* buff, size_t nbytes);
    ssize_t writen(const void* buff, size_t nbytes);

    bool bind(const char* ip, unsigned short port);
	bool connect(const char* ip, unsigned short port);
    bool listen(int backlog = 1024);
    Socket accept();

    int fd() const{
        return _fd;
    }

    SockAddr* get_sockaddr() const{
        return addr;
    }

    operator bool() const {
        return valid;
    }

    ~Socket(){
        close(_fd);
        if(addr){
            delete addr;
        }
    }
private:
    int _fd = 0;
    bool valid = false;
    SockAddr* addr = nullptr;
};


class Server{
public:
    Server(const char* ip ,unsigned short port ,int thread_num = 4):
    _tnum(thread_num)
    {
        if(listen_sock.bind(ip, port) && listen_sock.listen()){
            valid = true;
        }
    }

    virtual void process(char* buff, size_t len){
        cout << buff << endl;
    }

    void start(){
        for (int i = 0; i < _tnum; ++i){
            thread* temp = new thread(&Server::run,this);
            thrs.push_back(temp);
            thr_map[temp->get_id()] = temp;
        }
		while (!stop) {
			Socket s = listen_sock.accept();
			if (s) {
				conn_sockets.push_back()
			}
		}

    }
    void run(){
        cout << "start thread id = " << this_thread::get_id() << endl;
        cout << *listen_sock.get_sockaddr() << endl;
        process("Hello World", 11);
    }

    operator bool() const{
        return valid;
    }

private:
	bool stop = false;
    bool valid = false;
    int _tnum;
    list<thread*> thrs;
    unordered_map<thread::id, thread*> thr_map;
    Socket listen_sock;
	list<Socket> conn_sockets;
};

#endif