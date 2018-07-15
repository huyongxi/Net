#include "server.hpp"

int thread_func(string test_str){
    Socket s(AF_INET, SOCK_STREAM, 0);
    if(!s.connect("127.0.0.1", 8000)){
        return -1;
    }

    fcntl(s.fd(), F_SETFL, O_NONBLOCK); 
    int epfd = epoll_create(1);
    s.set_epollfd(epfd);
    Server::add_event(epfd, s.fd(), EPOLLIN | EPOLLET);
    epoll_event events[1];
    unsigned short len;
    while(true){
        len = test_str.size() + 1;
        s.writen(&len, sizeof(len));
        s.writen(test_str.c_str(), len);

        int ret = epoll_wait(epfd, events, 1, 5);
        cout << "ret = " << ret << endl;
        for(int i = 0; i < ret; ++i){
            if(events[i].events & EPOLLIN){
                cout << "EPOLLIN" << endl;
                auto packets = s.readn();
                for(auto& i : packets){
                    s.writen(&i.len, sizeof(i.len));
                    s.writen(i.data, i.len);
                    //sleep(1);
                }
            }
            if(events[i].events & EPOLLOUT){
                Server::delete_event(epfd, s.fd(), EPOLLOUT | EPOLLET);
                cout << "EPOLLOUT" << endl;
                s.writen("", 0);
            }
        }
    }
}


int main(void){

    int tnum = 200;
    vector<thread> vt;
    for(int i = 0; i < tnum; ++i){
        vt.push_back(thread(thread_func, string("test")));
    }
    for(auto& t : vt){
        t.join();
    }

    return 0;
}