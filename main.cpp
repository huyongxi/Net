#include "server.hpp"


int main(void) {
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &act, NULL) == 0){
		cout << "SIGPIPE ignore" <<endl;
	}
	Server s("192.168.85.128",8000,4);
	thread net_thread(&Server::start, &s);
	string cmd = "";
	while( cmd != "q"){
		cin >> cmd;
	}
	s.stop = true;
	net_thread.join();

	return 0;
}