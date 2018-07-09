#include "server.hpp"

std::ostream& operator<<(std::ostream& out, const SockAddr& addr) {
	return out << addr._ip << ":" << addr._port << std::endl;
}

bool Socket::connect(const char* ip, unsigned short port) {
	SockAddr addr(ip, port);
	int r = ::connect(_fd, (SA*)&addr.get_sockaddr_in(), sizeof(SA));
	if (r < 0) {
		std::cout << "connect " << addr << " error" << std::endl;
		return false;
	}
	return true;
}