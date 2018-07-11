#ifndef __SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <string.h>
#include <thread>
#include <list>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <vector>

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
#include <sys/epoll.h>
#include <signal.h>
#include <fcntl.h>
#endif
const int EPOLLSIZE = 5000;
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

    virtual void process(Socket& s, char* buff, size_t len){
        bzero(buff,1024);
        size_t rlen = s.readn(buff, len);
        if(rlen == 0){
            return;
        }
        cout << buff << endl;
        s.writen("Hello\n", 6);
    }

    void start(){
        for (int i = 0; i < _tnum; ++i){
            int epfd = epoll_create(EPOLLSIZE);
            if(epfd < 0){
                perror(__func__);
                return ;
            }
            epollfds.push_back(epfd);
            thread t(&Server::run, this, i);
            thread_list.push_back(move(t));
            mutexs.push_back(new mutex());
            queues.push_back(new queue<Socket*>());
        }
		while (!stop) {
			Socket s = listen_sock.accept();
			if (s) {
				conn_sockets.push_back(s);
                auto tmp = --conn_sockets.end();
                connsock_map[s.fd()] = tmp;
                int index = s.fd() % _tnum;
                mutexs[index]->lock();
                queues[index]->push( &(*tmp) );
                mutexs[index]->unlock();
                cout << s.fd() << " add queue" << endl;
			}
            
		}

    }
    void run(int index){
        cout << "start thread id = " << this_thread::get_id() << endl;
        list<Socket*> new_sock;
        queue<Socket*>* q = queues[index];
        epoll_event events[EPOLLSIZE/3];
        int epollfd = epollfds[index];
        mutex* queue_mutex = mutexs[index];
        char buff[1024];

        while(true) {
             queue_mutex->lock();
             while(!q->empty()){
                 new_sock.push_back(q->front());
                 q->pop();
             }
             queue_mutex->unlock();

             for(auto i : new_sock){
                 fcntl(i->fd(), F_SETFL, O_NONBLOCK); 
                 add_event(epollfd, i->fd(), EPOLLIN | EPOLLET | EPOLLRDHUP);
                 cout << this_thread::get_id() << " add event fd = " << i->fd() << endl;
             }
             new_sock.clear();

             int ret = epoll_wait(epollfd, events, EPOLLSIZE/3, 5);
             for(int i = 0; i < ret; ++i){
                int rfd = events[i].data.fd;
                Socket& rsock = *connsock_map[rfd];
                if(events[i].events & EPOLLIN){
                    process(rsock, buff, 1024);
                }
                if(events[i].events & (EPOLLHUP | EPOLLRDHUP)){
                    delete_event(epollfd, rfd, EPOLLIN | EPOLLET);
                    cout << *rsock.get_sockaddr() << " is disconnect" << endl;
                    rsock.close();
                    conn_sockets.erase(connsock_map[rfd]);
                    connsock_map.erase(rfd);
                }
             }

        }
    }

    void add_event(int epollfd, int fd, int state){
        epoll_event ev;
        ev.events = state;
        ev.data.fd = fd;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    }

    void modify_event(int epollfd,int fd,int state){     
        epoll_event ev;
        ev.events = state;
        ev.data.fd = fd;
        epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
    }

    void delete_event(int epollfd,int fd,int state) {
        epoll_event ev;
        ev.events = state;
        ev.data.fd = fd;
        epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
    }

    operator bool() const{
        return valid;
    }
    ~Server(){
        for(auto& s : conn_sockets){
            s.close();
        }
        for(auto fd : epollfds){
            ::close(fd);
        }
        for(auto m : mutexs){
            delete m;
        }
        for(auto q : queues){
            delete q;
        }
    }

private:
	bool stop = false;
    bool valid = false;
    int _tnum;
    vector<int> epollfds;
    list<thread> thread_list;
    Socket listen_sock;
	list<Socket> conn_sockets;
    unordered_map<int, list<Socket>::iterator> connsock_map;
    vector<mutex*> mutexs;
    vector<queue<Socket*>*> queues;
};

#endif