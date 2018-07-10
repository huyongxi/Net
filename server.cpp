#include "server.hpp"

std::ostream& operator<<(std::ostream& out, const SockAddr& addr) {
	return out << addr._ip << ":" << addr._port;
}

bool Socket::connect(const char* ip, unsigned short port) {
	addr = new SockAddr(ip, port);
	int r = ::connect(_fd, (SA*)addr->get_sockaddr_in(), sizeof(SA));
	if (r < 0) {
		std::cout << "connect " << *addr << " error" << std::endl;
		perror("");
		return false;
	}else{
		std::cout << "connect " << *addr << std::endl;
	}
	return true;
}

bool Socket::bind(const char* ip, unsigned short port){
	addr = new SockAddr(ip, port);
	int r = ::bind(_fd, (SA*)addr->get_sockaddr_in(), sizeof(SA));
	if (r < 0) {
		std::cout << "bind " << *addr << " error" << std::endl;
		perror("");
		return false;
	}else{
		std::cout << "bind " << *addr << std::endl;
	}
	return true;
}

bool Socket::listen(int backlog){
	int r = ::listen(_fd, backlog);
	if (r < 0) {
		std::cout << "listen " << *addr << " error" << std::endl;
		perror("");
		return false;
	}else{
		std::cout << "listen " << *addr << std::endl;
	}
	return true;
}

Socket Socket::accept(){
	sockaddr_in client_adddr;
	socklen_t len = sizeof(client_adddr);
	Socket sock("");
	int cfd = ::accept(_fd, (SA*)&client_adddr, &len);
	if (cfd < 0){
		perror("");
	}else{
		sock.addr = new SockAddr(&client_adddr);
		sock.valid = true;
		sock._fd = cfd;
		std::cout << *sock.addr << " is connected" << std::endl;
	}
	return sock;
}

ssize_t Socket::readn(void* buff, size_t nbytes){
	size_t nleft = nbytes;
	ssize_t nread;
	char* ptr = (char*)buff;

	while(nleft > 0){
		if( (nread = read(_fd,ptr,nleft)) < 0 ){
			if(errno == EINTR){
				nread = 0;			//interrupt read
			}else{
				return -1;
			}

		}else if(nread == 0){
			break;					//EOF
		}

		nleft -= nread;
		ptr += nread;
	}
	return nbytes - nleft;
}

ssize_t Socket::writen(const void* buff, size_t nbytes){
	size_t nleft = nbytes;
	ssize_t nwrite;
	const char* ptr = (char*)buff;

	while( nleft > 0){
		if( (nwrite = write(_fd,ptr,nleft)) <= 0 ){
			if (nwrite < 0 && errno == EINTR){			//nterrupt read
				nwrite = 0;
			}else{
				return -1;			//error
			}
		}

		nleft -= nwrite;
		ptr += nwrite;
	}
	return nbytes;
}