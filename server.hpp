#ifndef __SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <string.h>
#include <thread>
#include <list>
#include <queue>
#include <mutex>
#include <unordered_map>
#include  <vector>
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
    Socket(int family, int type, int protocol) {
        _fd = socket(family,type,protocol);
        if(_fd < 0){
            std::cout << "create socket failed!" << std::endl;
        }else{
            std::cout << "create socket success" << std::endl;
			valid = true;
        }
    }
    Socket(){}

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

    int close() const{
        return ::close(_fd);
    }

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
    _tnum(thread_num), listen_sock(AF_INET, SOCK_STREAM, 0)
    {
        if(listen_sock.bind(ip, port) && listen_sock.listen()){
            valid = true;
        }
    }

    virtual void process(const char* buff, size_t len){
        cout << buff << endl;
    }

    void start(){
        for (int i = 0; i < _tnum; ++i){
            thread t(&Server::run, this, i);
            thread_list.push_back(move(t));
            mutexs.push_back(new mutex());
            queues.push_back(new queue<Socket*>());
        }
		while (!stop) {
			Socket s = listen_sock.accept();
			if (s) {
				conn_sockets.push_back(s);
                connsock_map[s.fd()] = --conn_sockets.end();
                int index = s.fd() % _tnum;
                mutexs[index]->lock();
                queues[index]->push( &(*connsock_map[s.fd()]) );
                mutexs[index]->unlock();
                cout << s.fd() << " add queue" << endl;
			}
            
		}

    }
    void run(int index){
        cout << "start thread id = " << this_thread::get_id() << endl;
        list<Socket*> new_sock;
        queue<Socket*>* q = queues[index];
        while(true) {
             mutexs[index]->lock();
             while(!q->empty()){
                 new_sock.push_back(q->front());
                 q->pop();
             }
             mutexs[index]->unlock();
             if(new_sock.empty()){
                 //cout << "no new socket to process sleep 1s" <<endl;
                 sleep(1);
                 continue;
             }
             cout << this_thread::get_id() << " process: ";
             for(auto i : new_sock){
                 cout <<i->fd() << " ";
                 i->writen("Hello",6);
             }
             cout << endl;
             new_sock.clear();
        }
        process("Hello World", 11);
    }

    operator bool() const{
        return valid;
    }

private:
	bool stop = false;
    bool valid = false;
    int _tnum;
    list<thread> thread_list;
    Socket listen_sock;
	list<Socket> conn_sockets;
    unordered_map<int, list<Socket>::iterator> connsock_map;
    vector<mutex*> mutexs;
    vector<queue<Socket*>*> queues;
};

#endif