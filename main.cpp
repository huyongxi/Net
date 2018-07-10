#include "server.hpp"
using namespace std;
int main(void) {
#if (defined(WIN32) || defined(WIN64))
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	Socket s;
	s.bind("127.0.0.1",8888);
	s.listen();
	Socket clisock = s.accept();
	char buff[32];
	cout << clisock.fd() << endl;
	cout << clisock.readn(buff,32) << endl;
	cout << buff << endl;
	return 0;
}