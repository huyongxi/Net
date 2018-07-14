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

list<Packet> Socket::readn(){
	size_t nleft = 1024;
	size_t sum = 0;
	ssize_t nread;
	char* ptr = &recvbuff[rpos];

repeate:
	while(nleft > 0){
		int curr_size = recvbuff.size();
		if(rpos + nleft > curr_size){
			recvbuff.resize(curr_size * 2);
			ptr = &recvbuff[rpos];
		}
		if( (nread = read(_fd, ptr, nleft)) < 0 ){
			if(errno == EAGAIN){		//nothing to read
				break;
			}
			if(errno == EINTR){
				nread = 0;			//interrupt read
			}else{
				//return -1;
				return list<Packet>();
			}

		}else if(nread == 0){
			break;					//EOF  client disconnect
		}
		nleft -= nread;
		rpos += nread;
		ptr += nread;
		sum += nread;
	}
	if(nleft == 0){
		nleft = 1024;
		goto repeate;
	}
	
	return parsePacket();
}

ssize_t Socket::writen(const void* buff, size_t nbytes){
	ssize_t nwrite;
	int curr_size = sendbuff.size();
	if(wpos + nbytes > curr_size){
		sendbuff.resize(wpos + nbytes);
	}
	memcpy(&sendbuff[wpos], buff, nbytes);   //move buff to sendbuff
	wpos += nbytes;
	size_t nleft = wpos;
	const char* ptr = &sendbuff[0];			//send from sendbuff

	while( nleft > 0){
		if( (nwrite = write(_fd,ptr,nleft)) <= 0 ){
			cout << "nwrite = " << nwrite << endl;
			if(nwrite < 0 && errno == EAGAIN){      //full
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

	memmove(&sendbuff[0], ptr, nleft);
	wpos = nleft;
	return wpos;			//sendbufff size
}


list<Packet> Socket::parsePacket(){
	list<Packet> packets;
	char* buff = &recvbuff[0];
	unsigned short len_size = sizeof(unsigned short);
	while(true){
		if(rpos <= len_size){
			break;
		}else{
			unsigned short len = *(unsigned short*)buff;
			if(len_size + len > rpos){
				break;
			}
			Packet p;
			p.len = len;
			p.data = new char[len];
			memcpy(p.data, buff+len_size, len);

			packets.push_back(move(p));

			buff += (len_size + len);
			rpos -= (len_size + len);
		}
	}
	memmove(&recvbuff[0], buff, rpos);
	printf("recv %d packet rpos = %d\n", packets.size(), rpos);
	return move(packets);
}