#include "server.hpp"
int main(void) {
#if (defined(WIN32) || defined(WIN64))
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	Socket s;
	s.connect("61.135.169.125", 80);
	return 0;
}