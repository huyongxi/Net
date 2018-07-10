#include "server.hpp"






int main(void) {
#if (defined(WIN32) || defined(WIN64))
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
/*
	Socket s;
	s.bind("192.168.85.128",8888);
	s.listen();
	Socket clisock = s.accept();
	char buff[33] = {0};
	cout << clisock.fd() << endl;
	while(true){
		int r = clisock.readn(buff,32);
		cout << buff << endl;
	}
*/
	Server s("127.0.0.1",8000,4);
	s.start();
	sleep(20);
	return 0;
}