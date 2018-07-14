#include "server.hpp"

int main(void){
    Socket s(AF_INET, SOCK_STREAM, 0);
    if(!s.connect("127.0.0.1", 8000)){
        return -1;
    }
    unsigned short len = 6;
    s.writen(&len, sizeof(len));
    s.writen("hello world", 12);
}