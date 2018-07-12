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
	Socket sock;
	int cfd = ::accept(_fd, (SA*)&client_adddr, &len);
	if (cfd < 0){
		perror(__func__);
	}else{
		sock.addr = new SockAddr(&client_adddr);
		sock.valid = true;
		sock._fd = cfd;
		std::cout << *sock.addr << " is connected" << std::endl;
	}
	return move(sock);
}

ssize_t Socket::readn(){
	size_t nleft = 1024;
	size_t sum = 0;
	ssize_t nread;
	char* ptr = &recvbuff[rpos];

repeate:
	while(nleft > 0){
		cout << "rpos = " << rpos << endl;
		int curr_size = recvbuff.size();
		if(rpos + nleft > curr_size){
			recvbuff.resize(curr_size * 2);
			ptr = &recvbuff[rpos];
		}
		cout << "rpos = " << rpos << endl;
		if( (nread = read(_fd, ptr, nleft)) < 0 ){
			cout << "nread = " << nread << endl;
			if(errno == EAGAIN){		//nothing to read
				break;
			}
			if(errno == EINTR){
				nread = 0;			//interrupt read
			}else{
				return -1;
			}

		}else if(nread == 0){
			break;					//EOF  client disconnect
		}
		cout << "nread = " << nread << endl;
		nleft -= nread;
		cout << "rpos = " << rpos << endl;
		rpos += nread;
		ptr += nread;
		sum += nread;
		cout << "rpos = " << rpos << endl;
	}
	if(nleft == 0){
		nleft = 1024;
		goto repeate;
	}

	parsePacket();

	return sum;
}

ssize_t Socket::writen(const void* buff, size_t nbytes){
	size_t nleft = nbytes;
	ssize_t nwrite;
	const char* ptr = (char*)buff;
	char* temp_buff = nullptr;
	if(wpos != 0){			//sendbuff not empty
		cout << "wpos = " << wpos << endl;
		temp_buff = new char[wpos + nbytes];
		memcpy(temp_buff, &recvbuff[0], wpos);
		memcpy(temp_buff+wpos, buff, nbytes);
		wpos = 0;
		ptr = temp_buff;
	}
	while( nleft > 0){
		if( (nwrite = write(_fd,ptr,nleft)) <= 0 ){
			cout << "nwrite = " << nwrite << endl;
			if(nwrite < 0 && errno == EAGAIN){      //full
				int curr_size = sendbuff.size();
				if(wpos + nleft > curr_size){
					sendbuff.resize(curr_size * 2);
				}
				memcpy(&sendbuff[wpos], ptr, nleft);
				wpos += nleft;
				Server::add_event(_epfd, _fd, EPOLLOUT | EPOLLET);
				break;
			}
			if (nwrite < 0 && errno == EINTR){			//nterrupt read
				nwrite = 0;
			}else{
				return -1;			//error
			}
		}
		cout << "nwrite = " << nwrite << endl;
		nleft -= nwrite;
		ptr += nwrite;
	}
	if(temp_buff){
		delete[] temp_buff;
	}
	return nbytes - nleft;
}


void Socket::parsePacket(){
	cout << "rpos = " << rpos << endl;
	recvbuff[rpos] = 0;
	cout << &recvbuff[0] << endl;
	rpos = 0;
}